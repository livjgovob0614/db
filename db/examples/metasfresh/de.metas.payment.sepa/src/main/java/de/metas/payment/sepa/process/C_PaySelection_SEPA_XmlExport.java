package de.metas.payment.sepa.process;

/*
 * #%L
 * de.metas.payment.sepa
 * %%
 * Copyright (C) 2015 metas GmbH
 * %%
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 * #L%
 */

import java.io.File;

import org.adempiere.exceptions.AdempiereException;
import org.adempiere.model.InterfaceWrapperHelper;

import de.metas.document.engine.IDocumentBL;
import de.metas.i18n.IMsgBL;
import de.metas.payment.sepa.api.IPaymentBL;
import de.metas.payment.sepa.api.ISEPADocumentBL;
import de.metas.payment.sepa.interfaces.I_C_PaySelection;
import de.metas.payment.sepa.model.I_SEPA_Export;
import de.metas.process.IProcessPrecondition;
import de.metas.process.IProcessPreconditionsContext;
import de.metas.process.JavaProcess;
import de.metas.process.ProcessInfoParameter;
import de.metas.process.ProcessPreconditionsResolution;
import de.metas.util.Check;
import de.metas.util.Services;
import de.metas.util.time.SystemTime;

/**
 * Process that creates SEPA xmls in 3 steps:
 *
 * Creates SEPA_Export/SEPA_Export_Lines from C_PaySelection/C_PaySelectionLines Create SEPADocument/lines from the export Marshals the lines into an XML
 *
 * @author ad
 *
 */
public class C_PaySelection_SEPA_XmlExport
		extends JavaProcess
		implements IProcessPrecondition
{

	private static final String MSG_NO_SELECTION = "de.metas.payment.sepa.noPaySelection";

	private final String PARAM_FileName = "SaveFileName";
	private String p_Filename = null;

	//
	// services
	private final IPaymentBL paymentBL = Services.get(IPaymentBL.class);
	private final IMsgBL msgBL = Services.get(IMsgBL.class);
	private final ISEPADocumentBL sepaDocumentBL = Services.get(ISEPADocumentBL.class);

	@Override
	protected void prepare()
	{
		for (final ProcessInfoParameter para : getParametersAsArray())
		{
			if (para.getParameter() == null)
			{
				continue;
			}
			else if (PARAM_FileName.equals(para.getParameterName()) && para.getParameter() != null)
			{
				p_Filename = para.getParameter().toString();
			}
		}
	}

	@Override
	protected String doIt() throws Exception
	{
		final int recordId = getRecord_ID();
		if (recordId <= 0)
		{
			throw new AdempiereException(msgBL.getMsg(getCtx(), MSG_NO_SELECTION));
		}

		final I_C_PaySelection paySelection = InterfaceWrapperHelper.create(getCtx(), recordId, I_C_PaySelection.class, getTrxName());

		final String fileNameToUse;

		// 08267: allow the user to omit the file name from the dialog, and instead use a default
		// also, allow the user to just provide a folder
		if (Check.isEmpty(p_Filename, true))
		{
			fileNameToUse = sepaDocumentBL.createDefaultSepaExportFileName(getCtx(), paySelection.getName(), this);
		}
		else
		{
			final File f = new File(p_Filename);
			if (f.isDirectory())
			{
				fileNameToUse = p_Filename + File.separator + paySelection.getName().replaceAll("\\W+", "");
			}
			else
			{
				fileNameToUse = p_Filename;
			}
		}

		//
		// First, generate the SEPA export as an intermediary step, to use the old framework.
		final I_SEPA_Export sepaExport = paymentBL.createSEPAExport(paySelection);

		//
		// After the export and lines have been created, marshal the document.
		sepaDocumentBL.marshalXMLCreditFile(fileNameToUse, sepaExport, this);

		paySelection.setLastExport(SystemTime.asTimestamp());
		paySelection.setLastExportBy_ID(getAD_User_ID());
		InterfaceWrapperHelper.save(paySelection);

		return MSG_OK;
	}

	@Override
	public ProcessPreconditionsResolution checkPreconditionsApplicable(final IProcessPreconditionsContext context)
	{
		if (context == null)
		{
			return ProcessPreconditionsResolution.rejectWithInternalReason("Process " + SEPA_Export_GenerateXML.class + "only works with context != null");
		}

		final String tableName = context.getTableName();
		if (!I_C_PaySelection.Table_Name.equals(tableName))
		{
			return ProcessPreconditionsResolution.rejectWithInternalReason("Process " + SEPA_Export_GenerateXML.class + " only works for C_PaySelection");
		}

		final I_C_PaySelection paySelectionHeader = context.getSelectedModel(I_C_PaySelection.class);
		if (!Services.get(IDocumentBL.class).isDocumentCompletedOrClosed(paySelectionHeader))
		{
			return ProcessPreconditionsResolution.rejectWithInternalReason(
					"Process " + SEPA_Export_GenerateXML.class + " only works for completed or closed C_PaySelections; C_PaySelection_ID=" + paySelectionHeader.getC_PaySelection_ID());
		}
		return ProcessPreconditionsResolution.accept();
	}
}
