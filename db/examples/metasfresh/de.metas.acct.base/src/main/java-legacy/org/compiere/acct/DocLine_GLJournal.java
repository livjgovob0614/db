package org.compiere.acct;

import org.adempiere.model.InterfaceWrapperHelper;
import org.adempiere.util.LegacyAdapters;
import org.compiere.model.I_C_ValidCombination;
import org.compiere.model.I_GL_JournalLine;
import org.compiere.model.MAccount;

import de.metas.acct.api.AcctSchemaId;

/*
 * #%L
 * de.metas.acct.base
 * %%
 * Copyright (C) 2018 metas GmbH
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

class DocLine_GLJournal extends DocLine<Doc_GLJournal>
{
	private AcctSchemaId acctSchemaId;
	private MAccount m_account = null;

	public DocLine_GLJournal(final I_GL_JournalLine glJournalLine, final Doc_GLJournal doc)
	{
		super(InterfaceWrapperHelper.getPO(glJournalLine), doc);
	}

	public final AcctSchemaId getAcctSchemaId()
	{
		return acctSchemaId;
	}

	public final void setAcctSchemaId(final AcctSchemaId acctSchemaId)
	{
		this.acctSchemaId = acctSchemaId;
	}

	public final void setAccount(final I_C_ValidCombination acct)
	{
		m_account = LegacyAdapters.convertToPO(acct);
	}

	public final MAccount getAccount()
	{
		return m_account;
	}   // getAccount

}
