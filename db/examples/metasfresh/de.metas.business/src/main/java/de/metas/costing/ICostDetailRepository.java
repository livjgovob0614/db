package de.metas.costing;

import java.util.List;
import java.util.stream.Stream;

import de.metas.acct.api.AcctSchemaId;
import de.metas.product.ProductId;

/*
 * #%L
 * de.metas.business
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

public interface ICostDetailRepository
{
	CostDetail create(CostDetail.CostDetailBuilder costDetailBuilder);

	void delete(CostDetail costDetail);

	CostDetail getCostDetailOrNull(CostDetailQuery query);

	CostAmount getCostDetailAmtOrNull(CostDetailQuery query);

	List<CostDetail> getAllForDocument(CostingDocumentRef documentRef);

	List<CostDetail> getAllForDocumentAndAcctSchemaId(CostingDocumentRef documentRef, AcctSchemaId acctSchemaId);

	boolean hasCostDetailsForProductId(ProductId productId);

	Stream<CostDetail> streamOrderedById(CostDetailQuery query);
}
