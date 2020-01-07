package de.metas.process;

import java.sql.CallableStatement;
import java.sql.ResultSet;
import java.util.List;
import java.util.Optional;
import java.util.Properties;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

import org.adempiere.ad.trx.api.ITrx;
import org.adempiere.ad.trx.api.ITrxManager;
import org.adempiere.ad.trx.api.OnTrxMissingPolicy;
import org.adempiere.exceptions.AdempiereException;
import org.adempiere.exceptions.DBException;
import org.adempiere.util.lang.IAutoCloseable;
import org.adempiere.util.lang.NullAutoCloseable;
import org.compiere.model.I_AD_Rule;
import org.compiere.model.X_AD_Rule;
import org.compiere.print.ReportCtl;
import org.compiere.util.DB;
import org.compiere.util.Env;
import org.compiere.util.Ini;
import org.compiere.util.TrxRunnableAdapter;
import org.compiere.wf.MWFProcess;
import org.compiere.wf.MWorkflow;
import org.slf4j.Logger;

import com.google.common.base.Stopwatch;

import de.metas.i18n.IMsgBL;
// import de.metas.adempiere.form.IClientUI;
import de.metas.logging.LogManager;
import de.metas.organization.OrgId;
import de.metas.script.IADRuleDAO;
import de.metas.script.ScriptEngineFactory;
import de.metas.script.ScriptExecutor;
import de.metas.security.IUserRolePermissions;
import de.metas.security.IUserRolePermissionsDAO;
import de.metas.security.RoleId;
import de.metas.session.jaxrs.IServerService;
import de.metas.util.Check;
import de.metas.util.Services;
import lombok.NonNull;

/**
 * Process executor: executes a process (sync or async) which was defined by given {@link ProcessInfo}.
 *
 * @author authors of earlier versions of this class are: Jorg Janke, Low Heng Sin, Teo Sarca
 * @author metas-dev <dev@metasfresh.com>
 */
public final class ProcessExecutor
{
	public static Builder builder(final ProcessInfo processInfo)
	{
		return new Builder(processInfo);
	}

	public static OrgId getCurrentOrgId()
	{
		final OrgId orgId = s_currentOrg_ID.get();
		return orgId != null ? orgId : OrgId.ANY;
	}

	public static AdProcessId getCurrentProcessIdOrNull()
	{
		return s_currentProcess_ID.get();
	}

	//
	// Thread locals
	private static final ThreadLocal<AdProcessId> s_currentProcess_ID = new ThreadLocal<>(); // metas: c.ghita@metas.ro
	private static final ThreadLocal<OrgId> s_currentOrg_ID = new ThreadLocal<>(); // metas: c.ghita@metas.ro

	// services
	private static final transient Logger logger = LogManager.getLogger(ProcessExecutor.class);
	private final transient IMsgBL msgBL = Services.get(IMsgBL.class);
	private final transient ITrxManager trxManager = Services.get(ITrxManager.class);
	private final transient IADPInstanceDAO adPInstanceDAO = Services.get(IADPInstanceDAO.class);

	private final IProcessExecutionListener listener;
	private final ProcessInfo pi;
	private final boolean switchContextWhenRunning;
	private final boolean onErrorThrowException;

	private Thread m_thread; // metas

	private ProcessExecutor(final Builder builder)
	{
		pi = builder.getProcessInfo();

		// gh #2092 verify that we have an AD_Role_ID; otherwise, the assertPermissions() call we are going to do will fail
		Check.errorIf(pi.getRoleId() == null, "Process info has AD_Role_ID={}; builder={}", pi.getRoleId(), builder);

		listener = builder.getListener();
		switchContextWhenRunning = builder.switchContextWhenRunning;
		onErrorThrowException = builder.onErrorThrowException;
	}

	private String buildThreadName()
	{
		return pi.getTitle() + "-" + PInstanceId.toRepoIdOr(pi.getPinstanceId(), 0);
	}

	/**
	 * Run this process asynchronously
	 */
	private void executeAsync()
	{
		Check.assumeNull(m_thread, "not already started");

		final Thread thread = new Thread(() -> executeSync());
		thread.setName(buildThreadName());

		thread.start();

		m_thread = thread;
	}

	/**
	 * Run this process synchronously
	 */
	private void executeSync()
	{
		//
		// Case: the process requires to be executed on server, but we are not running on server
		// => execute the process remotely
		if (pi.isServerProcess() && Ini.isSwingClient())
		{
			executeSync_Remote();
		}
		//
		// Case: our process has some parts that requires to be executed out of transaction
		// but we are currently running in a transaction.
		// => spawn a new thread, run the process there and wait for the thread to finish
		else if (pi.getProcessClassInfo().isRunOutOfTransaction()
				&& trxManager.hasThreadInheritedTrx())
		{
			final Thread thread = new Thread(() -> executeNow());
			thread.setName(buildThreadName());
			thread.start();

			try
			{
				thread.join();
			}
			catch (final InterruptedException ex)
			{
				throw AdempiereException.wrapIfNeeded(ex);
			}

			//
			// Propagate the error if asked
			if (onErrorThrowException)
			{
				pi.getResult().propagateErrorIfAny();
			}
		}
		//
		// Case: standard case
		// => run the process now
		else
		{
			executeNow();
		}
	}

	private void executeSync_Remote()
	{
		// Make sure process info is persisted
		adPInstanceDAO.saveProcessInfoOnly(pi);

		try
		{
			lock(false);

			final ProcessExecutionResult result = pi.getResult();

			final ProcessExecutionResult remoteResult = Services.get(IServerService.class).process(pi.getPinstanceId().getRepoId());
			result.updateFrom(remoteResult);
		}
		catch (final Exception e)
		{
			final Throwable cause = AdempiereException.extractCause(e);
			logger.warn("Got error", cause);

			final ProcessExecutionResult result = pi.getResult();
			result.markAsError(cause);
			result.markLogsAsStale();
		}
		finally
		{
			unlock(false);
		}
	}

	private void executeNow()
	{
		logger.debug("running: {}", pi);

		//
		// set up the processExecutor that we will run further down
		final TrxRunnableAdapter processExecutor = new TrxRunnableAdapter()
		{
			@Override
			public void run(final String localTrxName) throws Exception
			{
				//
				// Execute the process (workflow/java/db process)
				if (pi.getAD_Workflow_ID() > 0)
				{
					startWorkflow();
					return;
				}
				else if (!Check.isEmpty(pi.getClassName(), true))
				{
					startJavaOrScriptProcess();
				}
				else if (pi.getDBProcedureName().isPresent())
				{
					startDBProcess();
				}

				//
				// Prepare report
				final boolean isReport = pi.isReportingProcess();
				final boolean hasProcessClass = !Check.isEmpty(pi.getClassName());
				if (isReport && hasProcessClass)
				{
					// nothing do to, the Jasper process class implementation is responsible for triggering the report preview if any
					return;
				}
				else if (isReport)
				{
					ReportCtl.builder()
							.setProcessInfo(pi)
							.start();
					pi.getResult().setSummary("Report");
				}
			}

			@Override
			public boolean doCatch(final Throwable e)
			{
				logger.warn("Got error", e);
				final ProcessExecutionResult result = pi.getResult();
				result.markAsError(e);
				return ROLLBACK;
			}
		};

		//
		// now run the process executor
		final AdProcessId previousProcessId = s_currentProcess_ID.get();
		final OrgId previousOrgId = s_currentOrg_ID.get();
		Stopwatch duration = null;
		try (final IAutoCloseable contextRestorer = switchContextIfNeeded())
		{
			s_currentProcess_ID.set(pi.getAdProcessId());
			s_currentOrg_ID.set(pi.getOrgId());

			//
			// Check permissions
			assertPermissions();

			// Lock
			lock(true);

			duration = Stopwatch.createStarted();

			//
			// Execute
			if (pi.getProcessClassInfo().isRunOutOfTransaction())
			{
				trxManager.runOutOfTransaction(processExecutor);
			}
			else
			{
				trxManager.run(ITrx.TRXNAME_ThreadInherited, processExecutor);
			}
		}
		finally
		{
			//
			// Update statistics
			if (duration != null)
			{
				duration.stop();
				final IADProcessDAO adProcessDAO = Services.get(IADProcessDAO.class);
				adProcessDAO.addProcessStatistics(pi.getAdProcessId(), pi.getClientId(), duration.elapsed(TimeUnit.MILLISECONDS)); // never throws exception
			}

			// Unlock
			unlock(true); // never throws exception

			// Clear thread local AD_Org_ID/AD_Process_ID/etc
			s_currentOrg_ID.set(previousOrgId);
			s_currentProcess_ID.set(previousProcessId);
		}

		//
		// Propagate the error if asked
		if (onErrorThrowException)
		{
			pi.getResult().propagateErrorIfAny();
		}
	}

	private IAutoCloseable switchContextIfNeeded()
	{
		if (switchContextWhenRunning)
		{
			return Env.switchContext(pi.getCtx());
		}
		else
		{
			return NullAutoCloseable.instance;
		}
	}

	private void assertPermissions()
	{
		final IUserRolePermissions permissions = Services.get(IUserRolePermissionsDAO.class).getUserRolePermissions(
				pi.getRoleId(),
				pi.getUserId(),
				pi.getClientId(),
				Env.getLocalDate(pi.getCtx()));

		if (!permissions.getRoleId().isSystem())
		{
			final AdProcessId adProcessId = pi.getAdProcessId();
			final Boolean access = permissions.getProcessAccess(adProcessId.getRepoId());
			if (access == null || !access.booleanValue())
			{
				throw new AdempiereException("Cannot access Process " + adProcessId + " with role: " + permissions.getName());
			}
		}
	}

	/**
	 * Lock the process instance and notify the parent
	 *
	 * NOTE: it's OK to throw exceptions
	 */
	private void lock(final boolean runningLocally)
	{
		//
		// Database: lock the AD_PInstance
		if (runningLocally)
		{
			adPInstanceDAO.lock(pi.getPinstanceId());
		}

		//
		// Notify parent
		if (listener != null)
		{
			listener.lockUI(pi);
		}
	}

	/**
	 * Unlock the process instance and notify the parent.
	 *
	 * NOTE: it's very important this method to never throw exception.
	 */
	private void unlock(final boolean runningLocally)
	{
		final Properties ctx = pi.getCtx();
		final ProcessExecutionResult result = pi.getResult();

		//
		// Translate process summary if needed
		if (runningLocally)
		{
			final String summary = result.getSummary();
			if (summary != null && summary.indexOf('@') >= 0)
			{
				result.setSummary(msgBL.parseTranslation(ctx, summary));
			}
		}

		//
		// Notify parent
		try
		{
			if (listener != null)
			{
				listener.unlockUI(pi);
			}
		}
		catch (final Exception ex)
		{
			logger.warn("Failed notifying the listener {} to unlock {}", listener, pi, ex);
		}

		//
		// Database: unlock and save the result
		if (runningLocally)
		{
			try
			{
				adPInstanceDAO.unlockAndSaveResult(result);
			}
			catch (final Throwable e)
			{
				logger.error("Failed unlocking for {}", result, e);
			}
		}
	}

	/**
	 * Start Workflow.
	 *
	 * @param AD_Workflow_ID workflow
	 * @return true if started
	 */
	private boolean startWorkflow()
	{
		final int AD_Workflow_ID = pi.getAD_Workflow_ID();
		Check.assume(AD_Workflow_ID > 0, "AD_Workflow_ID > 0");
		logger.debug("startWorkflow: {} ({})", AD_Workflow_ID, pi);

		final MWorkflow wf = MWorkflow.get(pi.getCtx(), AD_Workflow_ID);

		// note: depending on a pi flag we also called wf.start(pi);, but that flag always had a constant value.
		final MWFProcess wfProcess = wf.startWait(pi);	// may return null

		final boolean started = wfProcess != null;
		logger.debug("startWorkflow finish: started={}, wfProcess={}", started, wfProcess);

		return started;
	}   // startWorkflow

	/**
	 * Start Java/Script process.
	 *
	 * @return true if success
	 * @throws Exception
	 */
	private void startJavaOrScriptProcess() throws Exception
	{
		logger.debug("startProcess: {}", pi);

		final Optional<String> ruleValue = ScriptEngineFactory.extractRuleValueFromClassname(pi.getClassName());
		if (ruleValue.isPresent())
		{
			startScriptProcess(ruleValue.get());
		}
		else
		{
			startJavaProcess();
		}
	}

	private void startScriptProcess(final String ruleValue)
	{
		final Properties ctx = pi.getCtx();
		final I_AD_Rule rule = Services.get(IADRuleDAO.class).retrieveByValue(ctx, ruleValue);
		if (rule == null)
		{
			throw new AdempiereException("@ScriptNotFound@: " + ruleValue);
		}
		if (!X_AD_Rule.EVENTTYPE_Process.equals(rule.getEventType()))
		{
			throw new AdempiereException("@ScriptNotFound@: " + ruleValue + " - eventType must be Process");
		}

		final ITrx trx = trxManager.getThreadInheritedTrx(OnTrxMissingPolicy.ReturnTrxNone);
		final String trxName = trx == null ? null : trx.getTrxName();

		final ScriptExecutor scriptExecutor = ScriptEngineFactory.get()
				.createExecutor(rule)
				.putContext(ctx, pi.getWindowNo())
				.putArgument("Trx", trx)
				.putArgument("TrxName", trxName)
				.putArgument("Table_ID", pi.getTable_ID())
				.putArgument("Record_ID", pi.getRecord_ID())
				.putArgument("AD_Client_ID", pi.getAD_Client_ID())
				.putArgument("AD_Org_ID", pi.getAD_Org_ID())
				.putArgument("AD_User_ID", pi.getAD_User_ID())
				.putArgument("AD_Role_ID", RoleId.toRepoId(pi.getRoleId()))
				.putArgument("AD_PInstance_ID", PInstanceId.toRepoId(pi.getPinstanceId()));

		final List<ProcessInfoParameter> parameters = pi.getParameter();
		if (parameters != null)
		{
			scriptExecutor.putArgument("Parameter", parameters.toArray(new ProcessInfoParameter[parameters.size()])); // put as array for backward compatibility
			for (final ProcessInfoParameter para : parameters)
			{
				final String name = para.getParameterName();
				if (para.getParameter_To() == null)
				{
					final Object value = para.getParameter();
					if (name.endsWith("_ID") && (value instanceof Number))
					{
						scriptExecutor.putProcessParameter(name, ((Number)value).intValue());
					}
					else
					{
						scriptExecutor.putProcessParameter(name, value);
					}
				}
				else
				{
					final Object value1 = para.getParameter();
					final Object value2 = para.getParameter_To();
					if (name.endsWith("_ID") && (value1 instanceof Number))
					{
						scriptExecutor.putProcessParameter(name + "1", ((Number)value1).intValue());
					}
					else
					{
						scriptExecutor.putProcessParameter(name + "1", value1);
					}
					if (name.endsWith("_ID") && (value2 instanceof Number))
					{
						scriptExecutor.putProcessParameter(name + "2", ((Number)value2).intValue());
					}
					else
					{
						scriptExecutor.putProcessParameter(name + "2", value2);
					}
				}
			}
		}
		scriptExecutor.putArgument("ProcessInfo", pi);

		final Object scriptResult = scriptExecutor.execute(rule.getScript());
		final String msg = scriptResult != null ? scriptResult.toString() : null;
		// transaction should rollback if there are error in process
		if ("@Error@".equals(msg))
		{
			throw new AdempiereException(msg);
		}

		// Update result
		final ProcessExecutionResult result = pi.getResult();
		result.setSummary(msgBL.parseTranslation(ctx, msg)); // Parse Variables
	}

	private void startJavaProcess() throws Exception
	{
		final ProcessInfo pi = this.pi;

		final JavaProcess process = pi.newProcessClassInstanceOrNull();
		if (process == null)
		{
			throw new AdempiereException("Cannot create process class instance for " + pi); // shall not happen
		}

		final ITrx trx = trxManager.getThreadInheritedTrx(OnTrxMissingPolicy.ReturnTrxNone);

		try (final IAutoCloseable currentInstanceRestorer = JavaProcess.temporaryChangeCurrentInstanceOverriding(process))
		{
			process.startProcess(pi, trx);
		}
	}

	/**
	 * Start Database Process
	 *
	 * @return true if success
	 */
	private void startDBProcess()
	{
		final String dbProcedureName = pi.getDBProcedureName().get();
		logger.debug("startDBProcess: {} ({})", dbProcedureName, pi);

		final String sql = "{call " + dbProcedureName + "(?)}";
		final Object[] sqlParams = new Object[] { pi.getPinstanceId() };
		try (final CallableStatement cstmt = DB.prepareCall(sql, ResultSet.CONCUR_UPDATABLE, ITrx.TRXNAME_ThreadInherited))
		{
			DB.setParameters(cstmt, sqlParams);
			cstmt.executeUpdate();

			final ProcessExecutionResult result = pi.getResult();
			adPInstanceDAO.loadResultSummary(result);
			result.markLogsAsStale();
		}
		catch (final Exception e)
		{
			throw new DBException(e, sql, sqlParams);
		}
	}

	/**
	 * In case the process is running asynchronously, wait until thread completes
	 */
	public void waitToComplete()
	{
		final Thread thread = m_thread;
		if (thread != null && thread.isAlive())
		{
			try
			{
				thread.join();
			}
			catch (final InterruptedException e)
			{
				// somebody stopped the thread by sending an INTERRUPT signal
				logger.info("Process thread interrupted", e);
			}
		}
	}

	public ProcessInfo getProcessInfo()
	{
		return pi;
	}

	public ProcessExecutionResult getResult()
	{
		return pi.getResult();
	}

	public static final class Builder
	{
		private final transient IADPInstanceDAO adPInstanceDAO = Services.get(IADPInstanceDAO.class);

		private final ProcessInfo processInfo;
		private IProcessExecutionListener listener = null;
		private boolean switchContextWhenRunning = false;
		private boolean onErrorThrowException = false;
		private Consumer<ProcessInfo> beforeCallback = null;

		private Builder(@NonNull final ProcessInfo processInfo)
		{
			this.processInfo = processInfo;
		}

		public void executeASync()
		{
			processInfo.setAsync(true); // #1160 advise the product info, that we want an asynchronous execution
			final ProcessExecutor worker = build();
			worker.executeAsync();
		}

		public ProcessExecutor executeSync()
		{
			processInfo.setAsync(false); // #1160 advise the process info, that we want a synchronous execution
			final ProcessExecutor worker = build();
			worker.executeSync();
			return worker;
		}

		private ProcessExecutor build()
		{
			try
			{
				prepareAD_PInstance(processInfo);
			}
			catch (final Throwable e)
			{
				final ProcessExecutionResult result = processInfo.getResult();
				result.markAsError(e);

				if (listener != null)
				{
					listener.onProcessInitError(processInfo);
				}
				else
				{
					throw AdempiereException.wrapIfNeeded(e);
				}
			}

			return new ProcessExecutor(this);
		}

		private void prepareAD_PInstance(final ProcessInfo pi)
		{
			//
			// Save process info to database, including parameters.
			adPInstanceDAO.saveProcessInfo(pi);

			//
			// Execute before call callback
			if (beforeCallback != null)
			{
				beforeCallback.accept(pi);
			}
		}

		private ProcessInfo getProcessInfo()
		{
			return processInfo;
		}

		public Builder setListener(final IProcessExecutionListener listener)
		{
			this.listener = listener;
			return this;
		}

		private IProcessExecutionListener getListener()
		{
			return listener;
		}

		/**
		 * Advice the executor to propagate the error in case the execution failed.
		 */
		public Builder onErrorThrowException()
		{
			this.onErrorThrowException = true;
			return this;
		}

		public Builder onErrorThrowException(final boolean onErrorThrowException)
		{
			this.onErrorThrowException = onErrorThrowException;
			return this;
		}

		/**
		 * Advice the executor to switch current context with process info's context.
		 *
		 * @see ProcessInfo#getCtx()
		 * @see Env#switchContext(Properties)
		 */
		public Builder switchContextWhenRunning()
		{
			this.switchContextWhenRunning = true;
			return this;
		}

		/**
		 * Sets the callback to be executed after AD_PInstance is created but before the actual process is started.
		 * If the callback fails, the exception is propagated, so the process will not be started.
		 *
		 * A common use case of <code>beforeCallback</code> is to create to selections which are linked to this process instance.
		 *
		 * @param beforeCallback
		 */
		public Builder callBefore(final Consumer<ProcessInfo> beforeCallback)
		{
			this.beforeCallback = beforeCallback;
			return this;
		}
	}
}	// ProcessCtl
