/******************************************************************************
 * Product: Adempiere ERP & CRM Smart Business Solution                       *
 * Copyright (C) 1999-2006 ComPiere, Inc. All Rights Reserved.                *
 * This program is free software; you can redistribute it and/or modify it    *
 * under the terms version 2 of the GNU General Public License as published   *
 * by the Free Software Foundation. This program is distributed in the hope   *
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.           *
 * See the GNU General Public License for more details.                       *
 * You should have received a copy of the GNU General Public License along    *
 * with this program; if not, write to the Free Software Foundation, Inc.,    *
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.                     *
 * For the text or an alternative of this public license, you may reach us    *
 * ComPiere, Inc., 2620 Augustine Dr. #245, Santa Clara, CA 95054, USA        *
 * or via info@compiere.org or http://www.compiere.org/license.html           *
 *****************************************************************************/
package org.compiere.process;

import java.io.File;

import org.compiere.Adempiere;
import org.compiere.model.I_AD_User;
import org.compiere.model.I_C_BPartner;
import org.compiere.model.MClient;
import org.compiere.model.MDunningLevel;
import org.compiere.model.MDunningRun;
import org.compiere.model.MDunningRunEntry;
import org.compiere.model.MQuery;
import org.compiere.model.MQuery.Operator;
import org.compiere.model.MUserMail;
import org.compiere.model.PrintInfo;
import org.compiere.print.MPrintFormat;
import org.compiere.print.ReportEngine;
import org.compiere.util.AdempiereUserError;

import de.metas.bpartner.service.IBPartnerDAO;
import de.metas.email.EMail;
import de.metas.email.EMailAddress;
import de.metas.email.EMailSentStatus;
import de.metas.email.MailService;
import de.metas.email.templates.MailTemplateId;
import de.metas.email.templates.MailTextBuilder;
import de.metas.process.JavaProcess;
import de.metas.process.ProcessInfoParameter;
import de.metas.util.Services;

/**
 *	Dunning Letter Print
 *	
 *  @author Jorg Janke
 *  @version $Id: DunningPrint.java,v 1.2 2006/07/30 00:51:02 jjanke Exp $
 *  
 *  FR 2872010 - Dunning Run for a complete Dunning (not just level) - Developer: Carlos Ruiz - globalqss - Sponsor: Metas
 *  
 *  @deprecated This class is not working anymore, not used and we consider to drop it
 */
@Deprecated
public class DunningPrint extends JavaProcess
{
	private final MailService mailService = Adempiere.getBean(MailService.class);
	
	/**	Mail PDF				*/
	private boolean		p_EMailPDF = false;
	/** Mail Template			*/
	private int			p_R_MailText_ID = 0;
	/** Dunning Run				*/
	private int			p_C_DunningRun_ID = 0;
	/** Print only Outstanding	*/
	private boolean		p_IsOnlyIfBPBalance = true;
	/** Print only unprocessed lines */
	private boolean p_PrintUnprocessedOnly = true;
	
	
	/**
	 *  Prepare - e.g., get Parameters.
	 */
	@Override
	protected void prepare()
	{
		ProcessInfoParameter[] para = getParametersAsArray();
		for (ProcessInfoParameter element : para)
		{
			String name = element.getParameterName();
			if (element.getParameter() == null)
			{
				
			}
			else if (name.equals("EMailPDF"))
			{
				p_EMailPDF = "Y".equals(element.getParameter());
			}
			else if (name.equals("R_MailText_ID"))
			{
				p_R_MailText_ID = element.getParameterAsInt();
			}
			else if (name.equals("C_DunningRun_ID"))
			{
				p_C_DunningRun_ID = element.getParameterAsInt();
			}
			else if (name.equals("IsOnlyIfBPBalance"))
			{
				p_IsOnlyIfBPBalance = "Y".equals(element.getParameter());
			}
			else if (name.equals("PrintUnprocessedOnly"))
			{
				p_PrintUnprocessedOnly = "Y".equals(element.getParameter());
			}
			else
			{
				log.error("Unknown Parameter: " + name);
			}
		}
	}	//	prepare

	/**
	 * Pocess
	 * @return info
	 * @throws Exception
	 */
	@Override
	protected String doIt () throws Exception
	{
		log.info("C_DunningRun_ID=" + p_C_DunningRun_ID + ",R_MailText_ID=" + p_R_MailText_ID 
			+ ", EmailPDF=" + p_EMailPDF + ",IsOnlyIfBPBalance=" + p_IsOnlyIfBPBalance 
			+ ",PrintUnprocessedOnly=" + p_PrintUnprocessedOnly);
		
		//	Need to have Template
		if (p_EMailPDF && p_R_MailText_ID == 0)
		{
			throw new AdempiereUserError ("@NotFound@: @R_MailText_ID@");
		}
		MailTextBuilder mText = null;
		if (p_EMailPDF)
		{
			mText = mailService.newMailTextBuilder(MailTemplateId.ofRepoId(p_R_MailText_ID));
		}
		//
		MDunningRun run = new MDunningRun (getCtx(), p_C_DunningRun_ID, get_TrxName());
		if (run.get_ID() == 0)
		{
			throw new AdempiereUserError ("@NotFound@: @C_DunningRun_ID@ - " + p_C_DunningRun_ID);
		}
		MClient client = MClient.get(getCtx());
		
		int count = 0;
		int errors = 0;
		MDunningRunEntry[] entries = run.getEntries(false);
		for (MDunningRunEntry entry : entries)
		{
			//	Print Format on Dunning Level
			MDunningLevel level = new MDunningLevel (getCtx(), entry.getC_DunningLevel_ID(), get_TrxName());
			MPrintFormat format = null;
			if (level.getDunning_PrintFormat_ID() > 0)
			{
				format = MPrintFormat.get (getCtx(), level.getDunning_PrintFormat_ID(), false);
			}
			
			if (p_IsOnlyIfBPBalance && entry.getAmt().signum() <= 0)
			{
				continue;
			}
			if (p_PrintUnprocessedOnly && entry.isProcessed())
			{
				continue;
			}
			//	To BPartner
			I_C_BPartner bp = Services.get(IBPartnerDAO.class).getById(entry.getC_BPartner_ID());
			//	To User
			I_AD_User to = entry.getAD_User();
			if (p_EMailPDF)
			{
				if (to.getAD_User_ID() <= 0)
				{
					addLog (entry.get_ID(), null, null, "@NotFound@: @AD_User_ID@ - " + bp.getName());
					errors++;
					continue;
				}
				else if (to.getEMail() == null || to.getEMail().length() == 0)
				{
					addLog (entry.get_ID(), null, null, "@NotFound@: @EMail@ - " + to.getName());
					errors++;
					continue;
				}
			}
			//	query
			MQuery query = new MQuery("C_Dunning_Header_v");
			query.addRestriction("C_DunningRunEntry_ID", Operator.EQUAL, 
				new Integer(entry.getC_DunningRunEntry_ID()));

			//	Engine
			PrintInfo info = new PrintInfo(
				bp.getName(),
				MDunningRunEntry.Table_ID,
				entry.getC_DunningRunEntry_ID(),
				entry.getC_BPartner_ID());
			info.setDescription(bp.getName() + ", Amt=" + entry.getAmt());
			ReportEngine re = null;
			if (format != null)
			{
				re = new ReportEngine(getCtx(), format, query, info);
			}
			boolean printed = false;
			if (p_EMailPDF)
			{
				final EMail email = client.createEMail(
						EMailAddress.ofString(to.getEMail()),
						(String)null, // subject
						(String)null); // message
				if (!email.isValid())
				{
					addLog (entry.get_ID(), null, null, 
						"@RequestActionEMailError@ Invalid EMail: " + to);
					errors++;
					continue;
				}
				mText.bpartnerContact(to);	//	variable context
				mText.bpartner(bp);
				mText.record(entry);
				String message = mText.getFullMailText();
				if (mText.isHtml())
				{
					email.setMessageHTML(mText.getMailHeader(), message);
				}
				else
				{
					email.setSubject (mText.getMailHeader());
					email.setMessageText (message);
				}
				//
				if (re != null) {
					File attachment = re.getPDF(File.createTempFile("Dunning", ".pdf"));
					log.debug(to + " - " + attachment);
					email.addAttachment(attachment);
				}
				//
				final EMailSentStatus emailSentStatus = email.send();
				MUserMail um = new MUserMail(
						getCtx(),
						mText.getMailTemplateId().getRepoId(), 
						entry.getAD_User_ID(), 
						email, 
						emailSentStatus);
				um.save();
				if (emailSentStatus.isSentOK())
				{
					addLog (entry.get_ID(), null, null,
						bp.getName() + " @RequestActionEMailOK@");
					count++;
					printed = true;
				}
				else
				{
					addLog (entry.get_ID(), null, null,
						bp.getName() + " @RequestActionEMailError@ " + emailSentStatus.getSentMsg());
					errors++;
				}
			}
			else
			{
				// metas: Abfrage ob PrintFormat mit Jasper oder nicht.
				if (re.getPrintFormat() != null
					&& re.getPrintFormat().getJasperProcess_ID() > 0)
				{
					// ReportCtl.startDocumentPrint(ReportEngine.DUNNING, entry.get_ID(), null, -1, true);
					// TODO: implement if needed
					throw new UnsupportedOperationException();
				}
				else
				{
					re.print();
				}
				// metas end
				count++;
				printed = true;
			}
			if (printed)
			{
				entry.setProcessed (true);
				entry.save ();
			}

		}	//	for all dunning letters
		if (errors==0) {
			run.setProcessed(true);
			run.save();
		}
		if (p_EMailPDF)
		{
			return "@Sent@=" + count + " - @Errors@=" + errors;
		}
		return "@Printed@=" + count;
	}	//	doIt
	
}	//	DunningPrint
