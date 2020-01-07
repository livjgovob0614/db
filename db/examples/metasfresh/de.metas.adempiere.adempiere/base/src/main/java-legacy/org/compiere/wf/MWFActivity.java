/******************************************************************************
 * Product: Adempiere ERP & CRM Smart Business Solution *
 * Copyright (C) 1999-2006 ComPiere, Inc. All Rights Reserved. *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms version 2 of the GNU General Public License as published *
 * by the Free Software Foundation. This program is distributed in the hope *
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. *
 * See the GNU General Public License for more details. *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA. *
 * For the text or an alternative of this public license, you may reach us *
 * ComPiere, Inc., 2620 Augustine Dr. #245, Santa Clara, CA 95054, USA *
 * or via info@compiere.org or http://www.compiere.org/license.html *
 *****************************************************************************/
package org.compiere.wf;

import java.io.File;
import java.math.BigDecimal;
import java.sql.ResultSet;
import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Properties;
import java.util.Set;
import java.util.stream.Stream;

import org.adempiere.ad.persistence.TableModelLoader;
import org.adempiere.ad.service.IADReferenceDAO;
import org.adempiere.ad.table.api.IADTableDAO;
import org.adempiere.ad.trx.api.ITrx;
import org.adempiere.ad.trx.api.ITrxSavepoint;
import org.adempiere.exceptions.AdempiereException;
import org.adempiere.service.ClientId;
import org.compiere.SpringContextHolder;
import org.compiere.model.I_AD_Process_Para;
import org.compiere.model.I_AD_Role;
import org.compiere.model.I_AD_User;
import org.compiere.model.I_AD_WF_Node_Para;
import org.compiere.model.MClient;
import org.compiere.model.MColumn;
import org.compiere.model.MNote;
import org.compiere.model.PO;
import org.compiere.model.Query;
import org.compiere.model.X_AD_WF_Activity;
import org.compiere.print.ReportEngine;
import org.compiere.process.StateEngine;
import org.compiere.util.DisplayType;
import org.compiere.util.Env;
import org.compiere.util.Trace;
import org.compiere.util.Trx;
import org.compiere.util.Util;

import de.metas.attachments.AttachmentEntryService;
import de.metas.bpartner.BPartnerId;
import de.metas.bpartner.service.IBPartnerBL;
import de.metas.currency.ICurrencyBL;
import de.metas.document.engine.IDocument;
import de.metas.document.engine.IDocumentBL;
import de.metas.email.EMailAddress;
import de.metas.email.MailService;
import de.metas.email.templates.MailTemplateId;
import de.metas.email.templates.MailTextBuilder;
import de.metas.event.Topic;
import de.metas.event.Type;
import de.metas.i18n.IMsgBL;
import de.metas.money.CurrencyId;
import de.metas.notification.INotificationBL;
import de.metas.notification.UserNotificationRequest;
import de.metas.notification.UserNotificationRequest.TargetRecordAction;
import de.metas.organization.IOrgDAO;
import de.metas.organization.OrgId;
import de.metas.organization.OrgInfo;
import de.metas.process.ProcessInfo;
import de.metas.process.ProcessInfoParameter;
import de.metas.security.IRoleDAO;
import de.metas.security.IUserRolePermissions;
import de.metas.security.IUserRolePermissionsDAO;
import de.metas.security.RoleId;
import de.metas.security.permissions.DocumentApprovalConstraint;
import de.metas.user.UserId;
import de.metas.user.api.IUserDAO;
import de.metas.util.Check;
import de.metas.util.GuavaCollectors;
import de.metas.util.Services;

/**
 * Workflow Activity Model.
 * Controlled by WF Process:
 * set Node - startWork
 *
 * @author Jorg Janke
 * @version $Id: MWFActivity.java,v 1.4 2006/07/30 00:51:05 jjanke Exp $
 */
public class MWFActivity extends X_AD_WF_Activity implements Runnable
{
	private static final long serialVersionUID = 2987002047442429221L;

	private static final Topic USER_NOTIFICATIONS_TOPIC = Topic.of("de.metas.document.UserNotifications", Type.REMOTE);
	private static final String MSG_NotApproved = "NotApproved";

	/**
	 * Get Activities for table/record
	 *
	 * @param ctx context
	 * @param AD_Table_ID table
	 * @param Record_ID record
	 * @param activeOnly if true only not processed records are returned
	 * @return activity
	 */
	private static MWFActivity[] get(Properties ctx, int AD_Table_ID, int Record_ID, boolean activeOnly)
	{
		ArrayList<Object> params = new ArrayList<>();
		StringBuffer whereClause = new StringBuffer("AD_Table_ID=? AND Record_ID=?");
		params.add(AD_Table_ID);
		params.add(Record_ID);
		if (activeOnly)
		{
			whereClause.append(" AND Processed<>?");
			params.add(true);
		}
		List<MWFActivity> list = new Query(ctx, Table_Name, whereClause.toString(), null)
				.setParameters(params)
				.setOrderBy(COLUMNNAME_AD_WF_Activity_ID)
				.list(MWFActivity.class);

		MWFActivity[] retValue = new MWFActivity[list.size()];
		list.toArray(retValue);
		return retValue;
	}	// get

	/**
	 * Get Active Info
	 *
	 * @param ctx context
	 * @param AD_Table_ID table
	 * @param Record_ID record
	 * @return activity summary
	 */
	public static String getActiveInfo(Properties ctx, int AD_Table_ID, int Record_ID)
	{
		MWFActivity[] acts = get(ctx, AD_Table_ID, Record_ID, true);
		if (acts == null || acts.length == 0)
		{
			return null;
		}
		//
		StringBuffer sb = new StringBuffer();
		for (int i = 0; i < acts.length; i++)
		{
			if (i > 0)
			{
				sb.append("\n");
			}
			MWFActivity activity = acts[i];
			sb.append(activity.toStringX());
		}
		return sb.toString();
	}	// getActivityInfo

	/**************************************************************************
	 * Standard Constructor
	 *
	 * @param ctx context
	 * @param AD_WF_Activity_ID id
	 * @param trxName transaction
	 */
	public MWFActivity(Properties ctx, int AD_WF_Activity_ID, String trxName)
	{
		super(ctx, AD_WF_Activity_ID, trxName);
		if (AD_WF_Activity_ID == 0)
		{
			throw new IllegalArgumentException("Cannot create new WF Activity directly");
		}
		m_state = new StateEngine(getWFState());
	}	// MWFActivity

	/**
	 * Load Constructor
	 *
	 * @param ctx context
	 * @param rs result set
	 * @param trxName transaction
	 */
	public MWFActivity(Properties ctx, ResultSet rs, String trxName)
	{
		super(ctx, rs, trxName);
		m_state = new StateEngine(getWFState());
	}	// MWFActivity

	/**
	 * Parent Contructor
	 *
	 * @param process process
	 * @param AD_WF_Node_ID start node
	 */
	public MWFActivity(MWFProcess process, int AD_WF_Node_ID)
	{
		super(process.getCtx(), 0, process.get_TrxName());
		setAD_WF_Process_ID(process.getAD_WF_Process_ID());
		setPriority(process.getPriority());
		// Document Link
		setAD_Table_ID(process.getAD_Table_ID());
		setRecord_ID(process.getRecord_ID());
		// modified by Rob Klein
		setAD_Client_ID(process.getAD_Client_ID());
		setAD_Org_ID(process.getAD_Org_ID());
		// Status
		super.setWFState(WFSTATE_NotStarted);
		m_state = new StateEngine(getWFState());
		setProcessed(false);
		// Set Workflow Node
		setAD_Workflow_ID(process.getAD_Workflow_ID());
		setAD_WF_Node_ID(AD_WF_Node_ID);
		// Node Priority & End Duration
		MWFNode node = MWFNode.get(getCtx(), AD_WF_Node_ID);
		int priority = node.getPriority();
		if (priority != 0 && priority != getPriority())
		{
			setPriority(priority);
		}
		long limitMS = node.getLimitMS();
		if (limitMS != 0)
		{
			setEndWaitTime(new Timestamp(limitMS + System.currentTimeMillis()));
		}
		// Responsible
		setResponsible(process);
		save();
		//
		m_audit = new MWFEventAudit(this);
		m_audit.save();
		//
		m_process = process;
	}	// MWFActivity

	/**
	 * Parent Contructor
	 *
	 * @param process process
	 * @param AD_WF_Node_ID start node
	 * @param lastPO PO from the previously executed node
	 */
	public MWFActivity(MWFProcess process, int next_ID, PO lastPO)
	{
		this(process, next_ID);
		if (lastPO != null)
		{
			// Compare if the last PO is the same type and record needed here, if yes, use it
			if (lastPO.get_Table_ID() == getAD_Table_ID() && lastPO.get_ID() == getRecord_ID())
			{
				m_po = lastPO;
			}
		}
	}

	/** State Machine */
	private StateEngine m_state = null;
	/** Workflow Node */
	private MWFNode m_node = null;
	/** Transaction */
	// private Trx m_trx = null;
	/** Audit */
	private MWFEventAudit m_audit = null;
	/** Persistent Object */
	private PO m_po = null;
	/** Document Status */
	private String m_docStatus = null;
	/** New Value to save in audit */
	private String m_newValue = null;
	/** Process */
	private MWFProcess m_process = null;
	/** List of email recipients */
	private ArrayList<EMailAddress> m_emails = new ArrayList<>();

	/**************************************************************************
	 * Get State
	 *
	 * @return state
	 */
	public StateEngine getState()
	{
		return m_state;
	}	// getState

	/**
	 * Set Activity State.
	 * It also validates the new state and if is valid,
	 * then create event audit and call {@link MWFProcess#checkActivities(String, PO)}
	 *
	 * @param WFState
	 */
	@Override
	public void setWFState(String WFState)
	{
		if (m_state == null)
		{
			m_state = new StateEngine(getWFState());
		}
		if (m_state.isClosed())
		{
			return;
		}
		if (getWFState().equals(WFState))
		{
			return;
		}
		//
		if (m_state.isValidNewState(WFState))
		{
			String oldState = getWFState();
			log.debug(oldState + "->" + WFState + ", Msg=" + getTextMsg());
			super.setWFState(WFState);
			m_state = new StateEngine(getWFState());
			save();			// closed in MWFProcess.checkActivities()
			updateEventAudit();

			// Inform Process
			if (m_process == null)
			{
				m_process = new MWFProcess(getCtx(), getAD_WF_Process_ID(),
						this.get_TrxName());
			}
			m_process.checkActivities(this.get_TrxName(), m_po);
		}
		else
		{
			String msg = "Set WFState - Ignored Invalid Transformation - New="
					+ WFState + ", Current=" + getWFState();
			log.error(msg);
			Trace.printStack();
			setTextMsg(msg);
			save();
			// TODO: teo_sarca: throw exception ? please analyze the call hierarchy first
		}
	}	// setWFState

	/**
	 * Is Activity closed
	 *
	 * @return true if closed
	 */
	public boolean isClosed()
	{
		return m_state.isClosed();
	}	// isClosed

	/**************************************************************************
	 * Update Event Audit
	 */
	private void updateEventAudit()
	{
		// log.debug("");
		getEventAudit();
		m_audit.setTextMsg(getTextMsg());
		m_audit.setWFState(getWFState());
		if (m_newValue != null)
		{
			m_audit.setNewValue(m_newValue);
		}
		if (m_state.isClosed())
		{
			m_audit.setEventType(MWFEventAudit.EVENTTYPE_ProcessCompleted);
			long ms = System.currentTimeMillis() - m_audit.getCreated().getTime();
			m_audit.setElapsedTimeMS(new BigDecimal(ms));
		}
		else
		{
			m_audit.setEventType(MWFEventAudit.EVENTTYPE_StateChanged);
		}
		m_audit.save();
	}	// updateEventAudit

	/**
	 * Get/Create Event Audit
	 *
	 * @return event
	 */
	public MWFEventAudit getEventAudit()
	{
		if (m_audit != null)
		{
			return m_audit;
		}
		MWFEventAudit[] events = MWFEventAudit.get(getCtx(), getAD_WF_Process_ID(), getAD_WF_Node_ID(), get_TrxName());
		if (events == null || events.length == 0)
		{
			m_audit = new MWFEventAudit(this);
		}
		else {
			m_audit = events[events.length - 1];		// last event
		}
		return m_audit;
	}	// getEventAudit

	/**************************************************************************
	 * Get Persistent Object in Transaction
	 *
	 * @param trx transaction
	 * @return po
	 */
	private final PO getPO(final String trxName)
	{
		if (m_po != null)
		{
			if (trxName != null)
			{
				m_po.set_TrxName(trxName);
			}
			return m_po;
		}

		final String tableName = Services.get(IADTableDAO.class).retrieveTableName(getAD_Table_ID());
		m_po = TableModelLoader.instance.getPO(getCtx(), tableName, getRecord_ID(), trxName);
		return m_po;
	}	// getPO

	private final PO getPONoLoad()
	{
		return m_po;
	}

	/**
	 * Get Persistent Object
	 *
	 * @return po
	 */
	public PO getPO()
	{
		return getPO(get_TrxName());
	}	// getPO

	private IDocument getDocument()
	{
		return getDocument(ITrx.TRXNAME_ThreadInherited);
	}

	private IDocument getDocument(final String trxName)
	{
		final PO po = getPO(trxName);
		if (po == null)
		{
			throw new AdempiereException("Persistent Object not found - AD_Table_ID=" + getAD_Table_ID() + ", Record_ID=" + getRecord_ID());
		}

		return Services.get(IDocumentBL.class).getDocument(po);
	}

	private IDocument getDocumentOrNull(final String trxName)
	{
		final PO po = getPO(trxName);
		if (po == null)
		{
			throw new AdempiereException("Persistent Object not found - AD_Table_ID=" + getAD_Table_ID() + ", Record_ID=" + getRecord_ID());
		}

		return Services.get(IDocumentBL.class).getDocumentOrNull(po);
	}

	/**
	 * Get PO AD_Client_ID
	 *
	 * @return client of PO
	 */
	public int getPO_AD_Client_ID()
	{
		final PO po = getPO();
		return po != null ? po.getAD_Client_ID() : -1;
	}

	/**
	 * Get Attribute Value (based on Node) of PO
	 *
	 * @return Attribute Value or null
	 */
	public Object getAttributeValue()
	{
		MWFNode node = getNode();
		if (node == null)
		{
			return null;
		}
		int AD_Column_ID = node.getAD_Column_ID();
		if (AD_Column_ID == 0)
		{
			return null;
		}
		PO po = getPO();
		if (po.get_ID() == 0)
		{
			return null;
		}
		return po.get_ValueOfColumn(AD_Column_ID);
	}	// getAttributeValue

	/**
	 * Is SO Trx
	 *
	 * @return SO Trx or of not found true
	 */
	public boolean isSOTrx()
	{
		PO po = getPO();
		if (po.get_ID() == 0)
		{
			return true;
		}
		// Is there a Column?
		int index = po.get_ColumnIndex("IsSOTrx");
		if (index < 0)
		{
			if (po.get_TableName().startsWith("M_"))
			{
				return false;
			}
			return true;
		}
		// we have a column
		try
		{
			Boolean IsSOTrx = (Boolean)po.get_Value(index);
			return IsSOTrx.booleanValue();
		}
		catch (Exception e)
		{
			log.error("", e);
		}
		return true;
	}	// isSOTrx

	/**************************************************************************
	 * Set AD_WF_Node_ID.
	 * (Re)Set to Not Started
	 *
	 * @param AD_WF_Node_ID now node
	 */
	@Override
	public void setAD_WF_Node_ID(int AD_WF_Node_ID)
	{
		if (AD_WF_Node_ID == 0)
		{
			throw new IllegalArgumentException("Workflow Node is not defined");
		}
		super.setAD_WF_Node_ID(AD_WF_Node_ID);
		//
		if (!WFSTATE_NotStarted.equals(getWFState()))
		{
			super.setWFState(WFSTATE_NotStarted);
			m_state = new StateEngine(getWFState());
		}
		if (isProcessed())
		{
			setProcessed(false);
		}
	}	// setAD_WF_Node_ID

	/**
	 * Get WF Node
	 *
	 * @return node
	 */
	public MWFNode getNode()
	{
		if (m_node == null)
		{
			m_node = MWFNode.get(getCtx(), getAD_WF_Node_ID());
		}
		return m_node;
	}	// getNode

	/**
	 * Get WF Node Name
	 *
	 * @return translated node name
	 */
	public String getNodeName()
	{
		return getNode().getName(true);
	}	// getNodeName

	/**
	 * Get Node Description
	 *
	 * @return translated node description
	 */
	public String getNodeDescription()
	{
		return getNode().getDescription(true);
	}	// getNodeDescription

	/**
	 * Get Node Help
	 *
	 * @return translated node help
	 */
	public String getNodeHelp()
	{
		return getNode().getHelp(true);
	}	// getNodeHelp

	/**
	 * Is this an user Approval step?
	 *
	 * @return true if User Approval
	 */
	public boolean isUserApproval()
	{
		return getNode().isUserApproval();
	}	// isNodeApproval

	/**
	 * Is this a Manual user step?
	 *
	 * @return true if Window/Form/..
	 */
	public boolean isUserManual()
	{
		return getNode().isUserManual();
	}	// isUserManual

	/**
	 * Is this a user choice step?
	 *
	 * @return true if User Choice
	 */
	public boolean isUserChoice()
	{
		return getNode().isUserChoice();
	}	// isUserChoice

	/**
	 * Set Text Msg (add to existing)
	 *
	 * @param TextMsg
	 */
	@Override
	public void setTextMsg(String TextMsg)
	{
		if (TextMsg == null || TextMsg.length() == 0)
		{
			return;
		}
		String oldText = getTextMsg();
		if (oldText == null || oldText.length() == 0)
		{
			super.setTextMsg(Util.trimSize(TextMsg, 1000));
		}
		else if (TextMsg != null && TextMsg.length() > 0)
		{
			super.setTextMsg(Util.trimSize(oldText + "\n - " + TextMsg, 1000));
		}
	}	// setTextMsg

	/**
	 * Add to Text Msg
	 *
	 * @param obj some object
	 */
	public void addTextMsg(Object obj)
	{
		if (obj == null)
		{
			return;
		}
		//
		StringBuffer TextMsg = new StringBuffer();
		if (obj instanceof Exception)
		{
			Exception ex = (Exception)obj;
			if (ex.getMessage() != null && ex.getMessage().trim().length() > 0)
			{
				TextMsg.append(ex.toString());
			}
			else if (ex instanceof NullPointerException)
			{
				TextMsg.append(ex.getClass().getName());
			}
			while (ex != null)
			{
				StackTraceElement[] st = ex.getStackTrace();
				for (int i = 0; i < st.length; i++)
				{
					StackTraceElement ste = st[i];
					if (i == 0 || ste.getClassName().startsWith("org.compiere") || ste.getClassName().startsWith("org.adempiere"))
					{
						TextMsg.append(" (").append(i).append("): ")
								.append(ste.toString())
								.append("\n");
					}
				}
				if (ex.getCause() instanceof Exception)
				{
					ex = (Exception)ex.getCause();
				}
				else
				{
					ex = null;
				}
			}
		}
		else
		{
			TextMsg.append(obj.toString());
		}
		//
		String oldText = getTextMsg();
		if (oldText == null || oldText.length() == 0)
		{
			super.setTextMsg(Util.trimSize(TextMsg.toString(), 1000));
		}
		else if (TextMsg != null && TextMsg.length() > 0)
		{
			super.setTextMsg(Util.trimSize(oldText + "\n - " + TextMsg.toString(), 1000));
		}
	}	// setTextMsg

	/**
	 * Get WF State text
	 *
	 * @return state text
	 */
	public String getWFStateText()
	{
		return Services.get(IADReferenceDAO.class).retrieveListNameTrl(getCtx(), WFSTATE_AD_Reference_ID, getWFState());
	}	// getWFStateText

	/**
	 * Set Responsible and User from Process / Node
	 *
	 * @param process process
	 */
	private void setResponsible(MWFProcess process)
	{
		// Responsible
		int AD_WF_Responsible_ID = getNode().getAD_WF_Responsible_ID();
		if (AD_WF_Responsible_ID == 0)
		{
			AD_WF_Responsible_ID = process.getAD_WF_Responsible_ID();
		}
		setAD_WF_Responsible_ID(AD_WF_Responsible_ID);
		MWFResponsible resp = getResponsible();

		// User - Directly responsible
		int AD_User_ID = resp.getAD_User_ID();
		// Invoker - get Sales Rep or last updater of document
		if (AD_User_ID == 0 && resp.isInvoker())
		{
			AD_User_ID = process.getAD_User_ID();
		}
		//
		setAD_User_ID(AD_User_ID);
	}	// setResponsible

	/**
	 * Get Responsible
	 *
	 * @return responsible
	 */
	public MWFResponsible getResponsible()
	{
		MWFResponsible resp = MWFResponsible.get(getCtx(), getAD_WF_Responsible_ID());
		return resp;
	}	// isInvoker

	/**
	 * Is Invoker (no user & no role)
	 *
	 * @return true if invoker
	 */
	public boolean isInvoker()
	{
		return getResponsible().isInvoker();
	}	// isInvoker

	/**
	 * Get Approval User.
	 * If the returned user is the same, the document is approved.
	 *
	 * @param AD_User_ID starting User
	 * @param C_Currency_ID currency
	 * @param amount amount
	 * @param AD_Org_ID document organization
	 * @param ownDocument the document is owned by AD_User_ID
	 * @return AD_User_ID - if -1 no Approver
	 */
	public int getApprovalUser(final int AD_User_ID,
			final int C_Currency_ID, final BigDecimal amount,
			final int orgRepoId,
			boolean ownDocument)
	{
		// Nothing to approve
		if (amount == null || amount.signum() == 0)
		{
			return AD_User_ID;
		}

		// services
		final IUserDAO userDAO = Services.get(IUserDAO.class);
		final IUserRolePermissionsDAO userRolePermissionsDAO = Services.get(IUserRolePermissionsDAO.class);
		
		final OrgId orgId = OrgId.ofRepoIdOrAny(orgRepoId);

		// Starting user
		I_AD_User user = userDAO.retrieveUserOrNull(getCtx(), AD_User_ID);
		log.debug("For User=" + user
				+ ", Amt=" + amount
				+ ", Own=" + ownDocument);

		I_AD_User oldUser = null;
		while (user != null)
		{
			if (user.equals(oldUser))
			{
				log.debug("Loop - {}", user.getName());
				return -1;
			}
			oldUser = user;
			log.debug("User=" + user.getName());
			// Get Roles of User
			final List<IUserRolePermissions> roles = userRolePermissionsDAO.retrieveUserRolesPermissionsForUserWithOrgAccess(
					Env.getClientId(),
					orgId,
					UserId.ofRepoId(AD_User_ID),
					Env.getLocalDate());
			for (final IUserRolePermissions role : roles)
			{
				final DocumentApprovalConstraint docApprovalConstraints = role.getConstraint(DocumentApprovalConstraint.class)
						.or(DocumentApprovalConstraint.DEFAULT);
				if (ownDocument && !docApprovalConstraints.canApproveOwnDoc())
				{
					continue;	// find a role with allows them to approve own
				}

				BigDecimal amtApproval = docApprovalConstraints.getAmtApproval();
				if (amtApproval == null || amtApproval.signum() == 0)
				{
					continue;
				}

				//
				final int amtApprovalCurrencyId = docApprovalConstraints.getC_Currency_ID();
				if (C_Currency_ID != amtApprovalCurrencyId
						&& amtApprovalCurrencyId > 0)			// No currency = amt only
				{
					amtApproval = Services.get(ICurrencyBL.class).convert(// today & default rate
							amtApproval,
							CurrencyId.ofRepoId(amtApprovalCurrencyId),
							CurrencyId.ofRepoId(C_Currency_ID),
							ClientId.ofRepoId(getAD_Client_ID()),
							orgId);
					if (amtApproval == null || amtApproval.signum() == 0)
					{
						continue;
					}
				}
				boolean approved = amount.compareTo(amtApproval) <= 0;
				log.debug("Approved=" + approved
						+ " - User=" + user.getName() + ", Role=" + role.getName()
						+ ", ApprovalAmt=" + amtApproval);
				if (approved)
				{
					return user.getAD_User_ID();
				}
			}

			// **** Find next User
			// Get Supervisor
			if (user.getSupervisor_ID() != 0)
			{
				user = userDAO.getById(user.getSupervisor_ID());
				log.debug("Supervisor: {}", user);
			}
			else
			{
				log.debug("No Supervisor");
				IOrgDAO orgsRepo = Services.get(IOrgDAO.class);
				OrgInfo orgInfo = orgsRepo.getOrgInfoById(orgId);
				// Get Org Supervisor
				if (orgInfo.getSupervisorId() != null)
				{
					user = userDAO.getById(orgInfo.getSupervisorId());
				}
				else
				{
					log.debug("No Org Supervisor");
					// Get Parent Org Supervisor
					if (orgInfo.getParentOrgId() != null)
					{
						orgInfo = orgsRepo.getOrgInfoById(orgInfo.getParentOrgId());
						if (orgInfo.getSupervisorId() != null)
						{
							user = userDAO.getById(orgInfo.getSupervisorId());
							log.debug("Parent Org Supervisor: {}", user);
						}
					}
				}
			}	// No Supervisor
			// ownDocument should always be false for the next user
			ownDocument = false;
		}	// while there is a user to approve

		log.debug("No user found");
		return -1;
	}	// getApproval

	/**************************************************************************
	 * Execute Work.
	 * Called from MWFProcess.startNext
	 * Feedback to Process via setWFState -> checkActivities
	 */
	@Override
	public void run()
	{
		log.debug("Node={}", getNode());
		m_newValue = null;

		// m_trx = Trx.get(, true);
		Trx trx = null;
		boolean localTrx = false;
		if (get_TrxName() == null)
		{
			this.set_TrxName(Trx.createTrxName("WFA"));
			localTrx = true;
		}

		trx = Trx.get(get_TrxName(), true);

		ITrxSavepoint savepoint = null;

		//
		try
		{
			if (!localTrx)
			{
				savepoint = trx.createTrxSavepoint(null);
			}

			if (!m_state.isValidAction(StateEngine.ACTION_Start))
			{
				setTextMsg("State=" + getWFState() + " - cannot start");
				addTextMsg(new Exception(""));
				setWFState(StateEngine.STATE_Terminated);
				return;
			}
			//
			setWFState(StateEngine.STATE_Running);

			if (getNode().get_ID() == 0)
			{
				setTextMsg("Node not found - AD_WF_Node_ID=" + getAD_WF_Node_ID());
				setWFState(StateEngine.STATE_Aborted);
				return;
			}
			// Do Work
			/**** Trx Start ****/
			boolean done = performWork(Trx.get(get_TrxName(), false));

			/**** Trx End ****/
			// teo_sarca [ 1708835 ]
			// Reason: if the commit fails the document should be put in Invalid state
			if (localTrx)
			{
				try
				{
					trx.commit(true);
				}
				catch (Exception e)
				{
					// If we have a DocStatus, change it to Invalid, and throw the exception to the next level
					if (m_docStatus != null)
					{
						m_docStatus = IDocument.STATUS_Invalid;
					}
					throw e;
				}
			}

			setWFState(done ? StateEngine.STATE_Completed : StateEngine.STATE_Suspended);

			// NOTE: there is no need to postImmediate because DocumentEngine is handling this case (old code was removed from here)
		}
		catch (final Exception ex)
		{
			log.warn("{}", getNode(), ex);
			/**** Trx Rollback ****/
			if (localTrx)
			{
				trx.rollback();
			}
			else if (savepoint != null)
			{
				try
				{
					trx.rollback(savepoint);
					savepoint = null;
				}
				catch (Exception e1)
				{
					log.warn("Failed while rolling back to savepoint " + savepoint + ". Going forward...", e1);
				}
			}

			//
			if (ex.getCause() != null)
			{
				log.warn("Cause", ex.getCause());
			}

			String processMsg = ex.getLocalizedMessage();
			if (processMsg == null || processMsg.length() == 0)
			{
				processMsg = ex.getMessage();
			}
			setTextMsg(processMsg);
			addTextMsg(ex);
			setWFState(StateEngine.STATE_Terminated);	// unlocks

			// Set Document Status
			final PO po = getPONoLoad();
			final IDocument doc = po != null ? Services.get(IDocumentBL.class).getDocumentOrNull(po) : null;
			if (doc != null && m_docStatus != null)
			{
				po.load(get_TrxName());
				doc.setDocStatus(m_docStatus);
				m_po.save();
			}
		}
		finally
		{
			if (localTrx && trx != null)
			{
				trx.close();
			}
		}
	}	// run

	/**
	 * Perform Work.
	 * Set Text Msg.
	 *
	 * @param trx transaction
	 * @return true if completed, false otherwise
	 * @throws Exception if error
	 */
	private boolean performWork(final Trx trx) throws Exception
	{
		log.debug("Performing work for {} [{}]", m_node, trx);
		m_docStatus = null;

		if (m_node.getPriority() != 0)
		{
			setPriority(m_node.getPriority());
		}

		final String trxName = trx != null ? trx.getTrxName() : ITrx.TRXNAME_None;
		final String action = m_node.getAction();

		/****** Sleep (Start/End) ******/
		if (MWFNode.ACTION_WaitSleep.equals(action))
		{
			log.debug("Sleep:WaitTime=" + m_node.getWaitTime());
			if (m_node.getWaitingTime() == 0)
			 {
				return true;	// done
			}
			Calendar cal = Calendar.getInstance();
			cal.add(m_node.getDurationCalendarField(), m_node.getWaitTime());
			setEndWaitTime(new Timestamp(cal.getTimeInMillis()));
			return false;		// not done
		}

		/****** Document Action ******/
		else if (MWFNode.ACTION_DocumentAction.equals(action))
		{
			log.debug("DocumentAction={}", m_node.getDocAction());

			boolean success = false;
			String processMsg = null;
			final IDocument doc = getDocument(trxName);

			//
			try
			{
				success = doc.processIt(m_node.getDocAction());	// ** Do the work
				setTextMsg(doc.getSummary());
				processMsg = doc.getProcessMsg();
				m_docStatus = doc.getDocStatus();
			}
			catch (Exception e)
			{
				if (m_process != null)
				{
					m_process.setProcessMsg(e.getLocalizedMessage());
				}
				throw e;
			}

			// NOTE: there is no need to postImmediate because DocumentEngine is handling this case (old code was removed from here)

			//
			if (m_process != null)
			{
				m_process.setProcessMsg(processMsg);
			}

			//
			if (!m_po.save())
			{
				success = false;
				processMsg = "SaveError";
			}
			if (!success)
			{
				if (processMsg == null || processMsg.length() == 0)
				{
					processMsg = "PerformWork Error - " + m_node.toStringX();
					if (doc != null)
					{
						processMsg += " - DocStatus=" + doc.getDocStatus();
					}
				}
				throw new Exception(processMsg);
			}
			return success;
		}	// DocumentAction

		/****** Report ******/
		else if (MWFNode.ACTION_AppsReport.equals(action))
		{
			log.debug("Report: AD_Process_ID={}", m_node.getAD_Process_ID());
			final ProcessInfo pi = ProcessInfo.builder()
					.setCtx(getCtx())
					.setAD_Client_ID(getAD_Client_ID())
					.setAD_User_ID(getAD_User_ID())
					.setAD_Process_ID(m_node.getAD_Process_ID())
					.setTitle(m_node.getName(true))
					.setRecord(getAD_Table_ID(), getRecord_ID())
					.addParameters(createProcessInfoParameters(getPO(trxName)))
					.build();
			if (!pi.isReportingProcess())
			{
				throw new IllegalStateException("Not a Report AD_Process_ID=" + pi);
			}

			// Report
			ReportEngine re = ReportEngine.get(getCtx(), pi);
			if (re == null)
			{
				throw new IllegalStateException("Cannot create Report AD_Process_ID=" + m_node.getAD_Process_ID());
			}
			File report = re.getPDF();
			// Notice
			int AD_Message_ID = 753;		// HARDCODED WorkflowResult
			MNote note = new MNote(getCtx(), AD_Message_ID, getAD_User_ID(), trx.getTrxName());
			note.setTextMsg(m_node.getName(true));
			note.setDescription(m_node.getDescription(true));
			note.setRecord(getAD_Table_ID(), getRecord_ID());
			note.save();
			// Attachment

			final AttachmentEntryService attachmentEntryService = SpringContextHolder.instance.getBean(AttachmentEntryService.class);
			attachmentEntryService.createNewAttachment(note, report);
			return true;
		}

		/****** Process ******/
		else if (MWFNode.ACTION_AppsProcess.equals(action))
		{
			log.debug("Process: AD_Process_ID={}", m_node.getAD_Process_ID());
			ProcessInfo.builder()
					.setCtx(getCtx())
					.setAD_Client_ID(getAD_Client_ID())
					.setAD_User_ID(getAD_User_ID())
					.setAD_Process_ID(m_node.getAD_Process_ID())
					.setTitle(m_node.getName(true))
					.setRecord(getAD_Table_ID(), getRecord_ID())
					.addParameters(createProcessInfoParameters(getPO(trxName)))
					//
					.buildAndPrepareExecution()
					.onErrorThrowException()
					.executeSync();
			return true;
		}

		/******
		 * Start Task (Probably redundant;
		 * same can be achieved by attaching a Workflow node sequentially)
		 ******/
		/*
		 * else if (MWFNode.ACTION_AppsTask.equals(action))
		 * {
		 * log.warn("Task:AD_Task_ID=" + m_node.getAD_Task_ID());
		 * log.warn("Start Task is not implemented yet");
		 * }
		 */

		/****** EMail ******/
		else if (MWFNode.ACTION_EMail.equals(action))
		{
			log.debug("EMail:EMailRecipient={}", m_node.getEMailRecipient());

			final IDocument document = getDocumentOrNull(trxName);
			if (document != null)
			{
				m_emails = new ArrayList<>();
				sendEMail();
				setTextMsg(m_emails.toString());
			}
			else
			{
				final PO po = getPO(trxName);

				MailTemplateId mailTemplateId = MailTemplateId.ofRepoId(getNode().getR_MailText_ID());
				
				final MailService mailService = SpringContextHolder.instance.getBean(MailService.class);
				final MailTextBuilder mailTextBuilder = mailService.newMailTextBuilder(mailTemplateId)
						.recordAndUpdateBPartnerAndContact(po);

				// metas: tsa: check for null strings
				StringBuffer subject = new StringBuffer();
				if (!Check.isEmpty(getNode().getDescription(), true))
				{
					subject.append(getNode().getDescription());
				}
				if (!Check.isEmpty(mailTextBuilder.getMailHeader(), true))
				{
					if (subject.length() > 0)
					{
						subject.append(": ");
					}
					subject.append(mailTextBuilder.getMailHeader());
				}

				// metas: tsa: check for null strings
				StringBuffer message = new StringBuffer(mailTextBuilder.getFullMailText());
				if (!Check.isEmpty(getNodeHelp(), true))
				{
					message.append("\n-----\n").append(getNodeHelp());
				}
				
				final EMailAddress to = EMailAddress.ofString(getNode().getEMail());

				final MClient client = MClient.get(getCtx(), getAD_Client_ID());
				client.sendEMail(to, subject.toString(), message.toString(), null);
			}
			return true;	// done
		}	// EMail

		/****** Set Variable ******/
		else if (MWFNode.ACTION_SetVariable.equals(action))
		{
			String value = m_node.getAttributeValue();
			log.debug("SetVariable:AD_Column_ID={}", m_node.getAD_Column_ID()
					+ " to " + value);
			MColumn column = m_node.getColumn();
			int dt = column.getAD_Reference_ID();
			return setVariable(value, dt, null, trxName);
		}	// SetVariable

		/****** TODO Start WF Instance ******/
		else if (MWFNode.ACTION_SubWorkflow.equals(action))
		{
			log.warn("Workflow:AD_Workflow_ID={}", m_node.getAD_Workflow_ID());
			log.warn("Start WF Instance is not implemented yet");
		}

		/****** User Choice ******/
		else if (MWFNode.ACTION_UserChoice.equals(action))
		{
			log.debug("UserChoice:AD_Column_ID={}", m_node.getAD_Column_ID());
			// Approval
			final IDocument doc = Services.get(IDocumentBL.class).getDocumentOrNull(getPO(trxName));
			if (m_node.isUserApproval() && doc != null)
			{
				boolean autoApproval = false;
				// Approval Hierarchy
				if (isInvoker())
				{
					// Set Approver
					int startAD_User_ID = getAD_User_ID();
					if (startAD_User_ID == 0)
					{
						startAD_User_ID = doc.getDoc_User_ID();
					}
					int nextAD_User_ID = getApprovalUser(startAD_User_ID,
							doc.getC_Currency_ID(), doc.getApprovalAmt(),
							doc.getAD_Org_ID(),
							startAD_User_ID == doc.getDoc_User_ID());	// own doc
					// same user = approved
					autoApproval = startAD_User_ID == nextAD_User_ID;
					if (!autoApproval)
					{
						setAD_User_ID(nextAD_User_ID);
					}
				}
				else
				// fixed Approver
				{
					MWFResponsible resp = getResponsible();
					// MZ Goodwill
					// [ 1742751 ] Workflow: User Choice is not working
					if (resp.isHuman())
					{
						autoApproval = resp.getAD_User_ID() == m_process.getAD_User_ID();
						if (!autoApproval && resp.getAD_User_ID() != 0)
						{
							setAD_User_ID(resp.getAD_User_ID());
						}
					}
					else if (resp.isRole())
					{
						final RoleId roleId = RoleId.ofRepoId(resp.getAD_Role_ID());
						final Set<UserId> allRoleUserIds = Services.get(IRoleDAO.class).retrieveUserIdsForRoleId(roleId);
						if (allRoleUserIds.contains(UserId.ofRepoId(m_process.getAD_User_ID())))
						{
							autoApproval = true;
						}
					}
					else if (resp.isOrganization())
					{
						throw new AdempiereException("Support not implemented for " + resp);
					}
					else
					{
						throw new AdempiereException("@NotSupported@ " + resp);
					}
					// end MZ
				}
				if (autoApproval
						&& doc.processIt(IDocument.ACTION_Approve)
						&& doc.save())
				{
					return true;	// done
				}
			}	// approval
			return false;	// wait for user
		}
		/****** User Form ******/
		else if (MWFNode.ACTION_UserForm.equals(action))
		{
			log.debug("Form:AD_Form_ID=" + m_node.getAD_Form_ID());
			return false;
		}
		/****** User Window ******/
		else if (MWFNode.ACTION_UserWindow.equals(action))
		{
			log.debug("Window:AD_Window_ID=" + m_node.getAD_Window_ID());
			return false;
		}
		//
		throw new IllegalArgumentException("Invalid Action (Not Implemented) =" + action);
	}	// performWork

	/**
	 * Set Variable
	 *
	 * @param valueStr new Value
	 * @param displayType display type
	 * @param textMsg optional Message
	 * @return true if set
	 * @throws Exception if error
	 */
	private boolean setVariable(String valueStr, int displayType, String textMsg, String trxName)
	{
		m_newValue = null;
		final PO po = getPO(trxName);
		if (po == null)
		{
			throw new AdempiereException("Persistent Object not found - AD_Table_ID=" + getAD_Table_ID() + ", Record_ID=" + getRecord_ID());
		}

		// Set Value
		final Object dbValue;
		if (valueStr == null)
		{
			dbValue = null;
		}
		else if (displayType == DisplayType.YesNo)
		{
			dbValue = DisplayType.toBoolean(valueStr);
		}
		else if (DisplayType.isNumeric(displayType))
		{
			dbValue = new BigDecimal(valueStr);
		}
		else
		{
			dbValue = valueStr;
		}

		final String nodeColumnName = Services.get(IADTableDAO.class).retrieveColumnName(getNode().getAD_Column_ID());
		po.set_ValueOfColumn(nodeColumnName, dbValue);

		po.save();
		if (dbValue != null && !dbValue.equals(po.get_ValueOfColumn(getNode().getAD_Column_ID())))
		{
			throw new AdempiereException("Persistent Object not updated - AD_Table_ID="
					+ getAD_Table_ID() + ", Record_ID=" + getRecord_ID()
					+ " - Should=" + valueStr + ", Is=" + po.get_ValueOfColumn(m_node.getAD_Column_ID()));
		}
		// Info
		String msg = getNode().getAttributeName() + "=" + valueStr;
		if (textMsg != null && textMsg.length() > 0)
		{
			msg += " - " + textMsg;
		}
		setTextMsg(msg);
		m_newValue = valueStr;
		return true;
	}	// setVariable

	/**
	 * Set User Choice
	 *
	 * @param AD_User_ID user
	 * @param value new Value
	 * @param displayType display type
	 * @param textMsg optional Message
	 * @return true if set
	 * @throws Exception if error
	 */
	public boolean setUserChoice(int AD_User_ID, String value, int displayType, String textMsg)
	{
		setWFState(StateEngine.STATE_Running);
		setAD_User_ID(AD_User_ID);
		final String trxName = get_TrxName();
		boolean ok = setVariable(value, displayType, textMsg, trxName);
		if (!ok)
		{
			return false;
		}

		String newState = StateEngine.STATE_Completed;
		// Approval
		final IDocument doc = getDocumentOrNull(trxName);
		if (getNode().isUserApproval() && doc != null)
		{
			try
			{
				// Not approved
				if (!"Y".equals(value))
				{
					newState = StateEngine.STATE_Aborted;
					if (!(doc.processIt(IDocument.ACTION_Reject)))
					{
						setTextMsg("Cannot Reject - Document Status: " + doc.getDocStatus());
					}
				}
				else
				{
					if (isInvoker())
					{
						int startAD_User_ID = getAD_User_ID();
						if (startAD_User_ID == 0)
						{
							startAD_User_ID = doc.getDoc_User_ID();
						}
						int nextAD_User_ID = getApprovalUser(startAD_User_ID,
								doc.getC_Currency_ID(), doc.getApprovalAmt(),
								doc.getAD_Org_ID(),
								startAD_User_ID == doc.getDoc_User_ID());	// own doc
						// No Approver
						if (nextAD_User_ID <= 0)
						{
							newState = StateEngine.STATE_Aborted;
							setTextMsg("Cannot Approve - No Approver");
							doc.processIt(IDocument.ACTION_Reject);
						}
						else if (startAD_User_ID != nextAD_User_ID)
						{
							forwardTo(nextAD_User_ID, "Next Approver");
							newState = StateEngine.STATE_Suspended;
						}
						else
						// Approve
						{
							if (!(doc.processIt(IDocument.ACTION_Approve)))
							{
								newState = StateEngine.STATE_Aborted;
								setTextMsg("Cannot Approve - Document Status: " + doc.getDocStatus());
							}
						}
					}
					// No Invoker - Approve
					else if (!(doc.processIt(IDocument.ACTION_Approve)))
					{
						newState = StateEngine.STATE_Aborted;
						setTextMsg("Cannot Approve - Document Status: " + doc.getDocStatus());
					}
				}
				doc.save();
			}
			catch (Exception e)
			{
				newState = StateEngine.STATE_Terminated;
				setTextMsg("User Choice: " + AdempiereException.extractMessage(e));
				addTextMsg(e);
				log.warn("", e);
			}

			// Send Approval Notification
			if (newState.equals(StateEngine.STATE_Aborted) && doc.getDoc_User_ID() > 0)
			{
				final String docInfo = (doc.getSummary() != null ? doc.getSummary() + "\n" : "")
						+ (doc.getProcessMsg() != null ? doc.getProcessMsg() + "\n" : "")
						+ (getTextMsg() != null ? getTextMsg() : "");

				final INotificationBL notificationBL = Services.get(INotificationBL.class);
				notificationBL.sendAfterCommit(UserNotificationRequest.builder()
						.topic(USER_NOTIFICATIONS_TOPIC)
						.recipientUserId(UserId.ofRepoId(doc.getDoc_User_ID()))
						.contentADMessage(MSG_NotApproved)
						.contentADMessageParam(doc.toTableRecordReference())
						.contentADMessageParam(docInfo)
						.targetAction(TargetRecordAction.of(doc.toTableRecordReference()))
						.build());
			}
		}
		setWFState(newState);
		return ok;
	}	// setUserChoice

	/**
	 * Forward To
	 *
	 * @param AD_User_ID user
	 * @param textMsg text message
	 * @return true if forwarded
	 */
	public boolean forwardTo(int AD_User_ID, String textMsg)
	{
		if (AD_User_ID < 0)
		{
			log.warn("Does not exist - AD_User_ID=" + AD_User_ID);
		}
		if (AD_User_ID == getAD_User_ID())
		{
			log.warn("Same User - AD_User_ID=" + AD_User_ID);
			return false;
		}
		//
		final I_AD_User oldUser = getAD_User();
		final I_AD_User user = Services.get(IUserDAO.class).getById(AD_User_ID);
		// Update
		setAD_User_ID(user.getAD_User_ID());
		setTextMsg(textMsg);
		save();
		// Close up Old Event
		getEventAudit();
		m_audit.setAD_User_ID(oldUser.getAD_User_ID());
		m_audit.setTextMsg(getTextMsg());
		m_audit.setAttributeName("AD_User_ID");
		m_audit.setOldValue(oldUser.getName() + "(" + oldUser.getAD_User_ID() + ")");
		m_audit.setNewValue(user.getName() + "(" + user.getAD_User_ID() + ")");
		//
		m_audit.setWFState(getWFState());
		m_audit.setEventType(MWFEventAudit.EVENTTYPE_StateChanged);
		long ms = System.currentTimeMillis() - m_audit.getCreated().getTime();
		m_audit.setElapsedTimeMS(new BigDecimal(ms));
		m_audit.save();
		// Create new one
		m_audit = new MWFEventAudit(this);
		m_audit.save();
		return true;
	}	// forwardTo

	/**
	 * Set User Confirmation
	 *
	 * @param AD_User_ID user
	 * @param textMsg optional message
	 */
	public void setUserConfirmation(int AD_User_ID, String textMsg)
	{
		log.debug(textMsg);
		setWFState(StateEngine.STATE_Running);
		setAD_User_ID(AD_User_ID);
		if (textMsg != null)
		{
			setTextMsg(textMsg);
		}
		setWFState(StateEngine.STATE_Completed);
	}	// setUserConfirmation

	private List<ProcessInfoParameter> createProcessInfoParameters(final PO po)
	{
		return Stream.of(m_node.getParameters())
				.map(wfNodePara -> createProcessInfoParameter(wfNodePara, po))
				.filter(pip -> pip != null)
				.collect(GuavaCollectors.toImmutableList());
	}

	private final ProcessInfoParameter createProcessInfoParameter(final I_AD_WF_Node_Para nPara, final PO po)
	{
		final String attributeName = nPara.getAttributeName();
		final String attributeValue = nPara.getAttributeValue();
		log.debug("{} = {}", attributeName, attributeValue);

		// Value - Constant/Variable
		Object value = attributeValue;
		if (attributeValue == null || (attributeValue != null && attributeValue.length() == 0))
		{
			value = null;
		}
		else if (attributeValue.indexOf('@') != -1 && po != null)	// we have a variable
		{
			// Strip
			int index = attributeValue.indexOf('@');
			String columnName = attributeValue.substring(index + 1);
			index = columnName.indexOf('@');
			if (index == -1)
			{
				log.warn(attributeName + " - cannot evaluate=" + attributeValue);
				return null;
			}
			columnName = columnName.substring(0, index);
			index = po.get_ColumnIndex(columnName);
			if (index != -1)
			{
				value = po.get_Value(index);
			}
			else
			// not a column
			{
				// try Env
				String env = Env.getContext(getCtx(), columnName);
				if (env.length() == 0)
				{
					log.warn(attributeName + " - not column nor environment =" + columnName + "(" + attributeValue + ")");
					return null;
				}
				else
				{
					value = env;
				}
			}
		}	// @variable@

		final I_AD_Process_Para adProcessPara = nPara.getAD_Process_Para();

		// No Value
		if (value == null)
		{
			if (adProcessPara.isMandatory())
			{
				log.warn(attributeName + " - empty - mandatory!");
			}
			else
			{
				log.debug(attributeName + " - empty");
			}
			return null;
		}

		// Convert to Type
		try
		{
			final int displayType = adProcessPara.getAD_Reference_ID();
			if (DisplayType.isNumeric(displayType)
					|| DisplayType.isID(displayType))
			{
				BigDecimal bd = null;
				if (value instanceof BigDecimal)
				{
					bd = (BigDecimal)value;
				}
				else if (value instanceof Integer)
				{
					bd = new BigDecimal(((Integer)value).intValue());
				}
				else
				{
					bd = new BigDecimal(value.toString());
				}
				return ProcessInfoParameter.of(attributeName, bd);
			}
			else if (DisplayType.isDate(displayType))
			{
				Timestamp ts = null;
				if (value instanceof Timestamp)
				{
					ts = (Timestamp)value;
				}
				else
				{
					ts = Timestamp.valueOf(value.toString());
				}
				return ProcessInfoParameter.of(attributeName, ts);
			}
			else
			{
				return ProcessInfoParameter.of(attributeName, value.toString());
			}
		}
		catch (Exception e)
		{
			log.warn("Failed on {} = {} ({})", attributeName, attributeValue, value, e);
			return null;
		}
	}

	/*********************************
	 * Send EMail
	 */
	private void sendEMail()
	{
		final IDocument doc = getDocument();

		final MailService mailService = SpringContextHolder.instance.getBean(MailService.class);
		final MailTemplateId mailTemplateId = MailTemplateId.ofRepoId(m_node.getR_MailText_ID());
		final MailTextBuilder mailTextBuilder = mailService.newMailTextBuilder(mailTemplateId)
				.recordAndUpdateBPartnerAndContact(m_po);
		//
		String subject = doc.getDocumentInfo()
				+ ": " + mailTextBuilder.getMailHeader();
		String message = mailTextBuilder.getFullMailText()
				+ "\n-----\n" + doc.getDocumentInfo()
				+ "\n" + doc.getSummary();
		File pdf = doc.createPDF();
		//
		MClient client = MClient.get(doc.getCtx(), doc.getAD_Client_ID());

		// Explicit EMail
		sendEMail(
				client,
				(UserId)null,
				EMailAddress.ofNullableString(m_node.getEMail()),
				subject,
				message,
				pdf,
				mailTextBuilder.isHtml());
		// Recipient Type
		String recipient = m_node.getEMailRecipient();
		// email to document user
		if (recipient == null || recipient.length() == 0)
		{
			final UserId docUserId = UserId.ofRepoIdOrNull(doc.getDoc_User_ID());
			sendEMail(client, docUserId, null, subject, message, pdf, mailTextBuilder.isHtml());
		}
		else if (recipient.equals(MWFNode.EMAILRECIPIENT_DocumentBusinessPartner))
		{
			int index = m_po.get_ColumnIndex("AD_User_ID");
			if (index > 0)
			{
				Object oo = m_po.get_Value(index);
				if (oo instanceof Integer)
				{
					final UserId userId = UserId.ofRepoIdOrNull(((Integer)oo).intValue());
					if (userId != null)
					{
						sendEMail(client, userId, null, subject, message, pdf, mailTextBuilder.isHtml());
					}
					else
					{
						log.debug("No User in Document");
					}
				}
				else
				{
					log.debug("Empty User in Document");
				}
			}
			else
			{
				log.debug("No User Field in Document");
			}
		}
		else if (recipient.equals(MWFNode.EMAILRECIPIENT_DocumentOwner))
		{
			final UserId docUserId = UserId.ofRepoIdOrNull(doc.getDoc_User_ID());
			sendEMail(client, docUserId, null, subject, message, pdf, mailTextBuilder.isHtml());
		}
		else if (recipient.equals(MWFNode.EMAILRECIPIENT_WFResponsible))
		{
			final MWFResponsible resp = getResponsible();
			if (resp.isInvoker())
			{
				final UserId docUserId = UserId.ofRepoIdOrNull(doc.getDoc_User_ID());
				sendEMail(client, docUserId, null, subject, message, pdf, mailTextBuilder.isHtml());
			}
			else if (resp.isHuman())
			{
				final UserId docUserId = UserId.ofRepoIdOrNull(doc.getDoc_User_ID());
				sendEMail(client, docUserId, null, subject, message, pdf, mailTextBuilder.isHtml());
			}
			else if (resp.isRole())
			{
				final I_AD_Role role = resp.getRole();
				if (role != null)
				{
					final RoleId roleId = RoleId.ofRepoId(role.getAD_Role_ID());
					for (final UserId adUserId : Services.get(IRoleDAO.class).retrieveUserIdsForRoleId(roleId))
					{
						sendEMail(client, adUserId, null, subject, message, pdf, mailTextBuilder.isHtml());
					}
				}
			}
			else if (resp.isOrganization())
			{
				final OrgId orgId = OrgId.ofRepoIdOrAny(m_po.getAD_Org_ID());
				final OrgInfo org = Services.get(IOrgDAO.class).getOrgInfoById(orgId);
				if (org.getSupervisorId() == null)
				{
					log.debug("No Supervisor for AD_Org_ID={}", orgId);
				}
				else
				{
					sendEMail(client, org.getSupervisorId(), null, subject, message, pdf, mailTextBuilder.isHtml());
				}
			}
		}
	}	// sendEMail

	/**
	 * Send actual EMail
	 *
	 * @param client client
	 * @param AD_User_ID user
	 * @param email email string
	 * @param subject subject
	 * @param message message
	 * @param pdf attachment
	 * @param isHtml isHtml
	 */
	private void sendEMail(
			MClient client,
			UserId userId, 
			EMailAddress email,
			String subject, 
			String message, 
			File pdf, 
			boolean isHtml)
	{
		if (userId != null)
		{
			final I_AD_User user = Services.get(IUserDAO.class).getById(userId);
			email = EMailAddress.ofNullableString(user.getEMail());
			if (email != null)
			{
				if (!m_emails.contains(email))
				{
					client.sendEMail(null, user, subject, message, pdf, isHtml);
					m_emails.add(email);
				}
			}
			else
			{
				log.debug("No EMail for User {}", user.getName());
			}
		}
		else if (email != null)
		{
			if (!m_emails.contains(email))
			{
				client.sendEMail(email, subject, message, pdf, isHtml);
				m_emails.add(email);
			}
		}
		else
		{
			log.warn("No userId or email provided");
		}
	}	// sendEMail

	/**************************************************************************
	 * Get Process Activity (Event) History
	 *
	 * @return history
	 */
	public String getHistoryHTML()
	{
		SimpleDateFormat format = DisplayType.getDateFormat(DisplayType.DateTime);
		StringBuffer sb = new StringBuffer();
		MWFEventAudit[] events = MWFEventAudit.get(getCtx(), getAD_WF_Process_ID(), get_TrxName());
		for (MWFEventAudit audit : events)
		{
			// sb.append("<p style=\"width:400\">");
			sb.append("<p>");
			sb.append(format.format(audit.getCreated()))
					.append(" ")
					.append(getHTMLpart("b", audit.getNodeName()))
					.append(": ")
					.append(getHTMLpart(null, audit.getDescription()))
					.append(getHTMLpart("i", audit.getTextMsg()));
			sb.append("</p>");
		}
		return sb.toString();
	}	// getHistory

	/**
	 * Get HTML part
	 *
	 * @param tag HTML tag
	 * @param content content
	 * @return <tag>content</tag>
	 */
	private StringBuffer getHTMLpart(String tag, String content)
	{
		StringBuffer sb = new StringBuffer();
		if (content == null || content.length() == 0)
		{
			return sb;
		}
		if (tag != null && tag.length() > 0)
		{
			sb.append("<").append(tag).append(">");
		}
		sb.append(content);
		if (tag != null && tag.length() > 0)
		{
			sb.append("</").append(tag).append(">");
		}
		return sb;
	}	// getHTMLpart

	/**
	 * String Representation
	 *
	 * @return info
	 */
	@Override
	public String toString()
	{
		StringBuffer sb = new StringBuffer("MWFActivity[");
		sb.append(get_ID()).append(",Node=");
		if (m_node == null)
		{
			sb.append(getAD_WF_Node_ID());
		}
		else
		{
			sb.append(m_node.getName());
		}
		sb.append(",State=").append(getWFState())
				.append(",AD_User_ID=").append(getAD_User_ID())
				.append(",").append(getCreated())
				.append("]");
		return sb.toString();
	} 	// toString

	/**
	 * User String Representation.
	 * Suspended: Approve it (Joe)
	 *
	 * @return info
	 */
	public String toStringX()
	{
		StringBuffer sb = new StringBuffer();
		sb.append(getWFStateText())
				.append(": ").append(getNode().getName());
		if (getAD_User_ID() > 0)
		{
			final String userFullname = Services.get(IUserDAO.class).retrieveUserFullname(getAD_User_ID());
			sb.append(" (").append(userFullname).append(")");
		}
		return sb.toString();
	}	// toStringX

	/**
	 * Get Document Summary
	 *
	 * @return PO Summary
	 */
	public String getSummary()
	{
		PO po = getPO();
		if (po == null)
		{
			return null;
		}
		StringBuffer sb = new StringBuffer();
		String[] keyColumns = po.get_KeyColumns();
		if ((keyColumns != null) && (keyColumns.length > 0))
		{
			sb.append(Services.get(IMsgBL.class).translate(getCtx(), keyColumns[0])).append(" ");
		}
		int index = po.get_ColumnIndex("DocumentNo");
		if (index != -1)
		{
			sb.append(po.get_Value(index)).append(": ");
		}
		index = po.get_ColumnIndex("SalesRep_ID");
		Integer sr = null;
		if (index != -1)
		{
			sr = (Integer)po.get_Value(index);
		}
		else
		{
			index = po.get_ColumnIndex("AD_User_ID");
			if (index != -1)
			{
				sr = (Integer)po.get_Value(index);
			}
		}
		if (sr != null)
		{
			String userFullname = Services.get(IUserDAO.class).retrieveUserFullname(sr.intValue());
			if (!Check.isEmpty(userFullname))
			{
				sb.append(userFullname).append(" ");
			}
		}
		//
		index = po.get_ColumnIndex("C_BPartner_ID");
		if (index != -1)
		{
			final BPartnerId bpartnerId = BPartnerId.ofRepoIdOrNull(po.get_ValueAsInt(index));
			if (bpartnerId != null)
			{
				final String bpartnerName = Services.get(IBPartnerBL.class).getBPartnerName(bpartnerId);
				sb.append(bpartnerName).append(" ");
			}
		}
		return sb.toString();
	}	// getSummary

}	// MWFActivity
