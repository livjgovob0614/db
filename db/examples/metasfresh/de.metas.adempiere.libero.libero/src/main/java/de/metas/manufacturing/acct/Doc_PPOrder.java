package de.metas.manufacturing.acct;

/*
 * #%L
 * de.metas.adempiere.libero.libero
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

import java.math.BigDecimal;
import java.util.List;

import org.compiere.acct.Doc;
import org.compiere.acct.DocLine;
import org.compiere.acct.Fact;
import org.compiere.model.X_C_DocType;
import org.eevolution.model.I_PP_Order;

import com.google.common.collect.ImmutableList;

import de.metas.acct.api.AcctSchema;
import de.metas.acct.doc.AcctDocContext;

public class Doc_PPOrder extends Doc<DocLine<Doc_PPOrder>>
{

	public Doc_PPOrder(final AcctDocContext ctx)
	{
		super(ctx, X_C_DocType.DOCBASETYPE_ManufacturingOrder);

		final I_PP_Order ppOrder = getModel(I_PP_Order.class);
		setDateAcct(ppOrder.getDateOrdered());
	}

	@Override
	protected void loadDocumentDetails()
	{
		// nothing
	}

	@Override
	public BigDecimal getBalance()
	{
		return BigDecimal.ZERO;
	}

	@Override
	public List<Fact> createFacts(final AcctSchema as)
	{
		return ImmutableList.of();
	}

}
