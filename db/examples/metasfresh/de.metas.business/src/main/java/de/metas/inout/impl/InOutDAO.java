package de.metas.inout.impl;

import static org.adempiere.model.InterfaceWrapperHelper.load;
import static org.adempiere.model.InterfaceWrapperHelper.loadOutOfTrx;

import java.math.BigDecimal;
import java.time.LocalDate;
import java.util.Collection;

/*
 * #%L
 * de.metas.adempiere.adempiere.base
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

import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
import java.util.stream.Stream;

import com.google.common.collect.ImmutableList;
import de.metas.shipping.model.ShipperTransportationId;
import org.adempiere.ad.dao.IQueryBL;
import org.adempiere.ad.dao.IQueryBuilder;
import org.adempiere.ad.dao.impl.CompareQueryFilter.Operator;
import org.adempiere.ad.trx.api.ITrx;
import org.compiere.model.IQuery.Aggregate;
import org.compiere.model.I_C_OrderLine;
import org.compiere.model.I_M_InOut;
import org.compiere.model.I_M_InOutLine;

import com.google.common.collect.ImmutableSet;

import de.metas.bpartner.BPartnerId;
import de.metas.document.engine.IDocument;
import de.metas.inout.IInOutDAO;
import de.metas.inout.InOutAndLineId;
import de.metas.inout.InOutId;
import de.metas.inout.InOutLineId;
import de.metas.lang.SOTrx;
import de.metas.product.ProductId;
import de.metas.util.Check;
import de.metas.util.GuavaCollectors;
import de.metas.util.Services;
import de.metas.util.collections.CollectionUtils;
import lombok.NonNull;

import javax.annotation.Nullable;

public class InOutDAO implements IInOutDAO
{
	@Override
	public I_M_InOut getById(@NonNull final InOutId inoutId)
	{
		return load(inoutId, I_M_InOut.class);
	}

	@Nullable
	@Override
	public <T extends I_M_InOut> T getById(@NonNull final InOutId inoutId, @NonNull final Class<T> modelClass)
	{
		return load(inoutId, modelClass);
	}

	@Override
	public I_M_InOutLine getLineById(@NonNull final InOutLineId inoutLineId)
	{
		return load(inoutLineId, I_M_InOutLine.class);
	}

	@NonNull
	@Override
	public ImmutableList<InOutId> retrieveByShipperTransportation(@NonNull final ShipperTransportationId shipperTransportationId)
	{
		return Services.get(IQueryBL.class)
				.createQueryBuilder(de.metas.inout.model.I_M_InOut.class)
				.addOnlyActiveRecordsFilter()
				.addEqualsFilter(de.metas.inout.model.I_M_InOut.COLUMNNAME_M_ShipperTransportation, shipperTransportationId)
				.create()
				.listIds(InOutId::ofRepoId)
				.asList();
	}

	@Override
	public <T extends I_M_InOutLine> T getLineById(@NonNull final InOutLineId inoutLineId, final Class<T> modelClass)
	{
		@SuppressWarnings("UnnecessaryLocalVariable")
		final T inoutLine = loadOutOfTrx(inoutLineId.getRepoId(), modelClass);
		return inoutLine;
	}

	@Override
	public List<I_M_InOutLine> retrieveLines(final I_M_InOut inOut)
	{
		return retrieveLines(inOut, I_M_InOutLine.class);
	}

	@Override
	public List<I_M_InOutLine> retrieveAllLines(final I_M_InOut inOut)
	{
		final boolean retrieveAll = true;
		return retrieveLines(inOut, retrieveAll, I_M_InOutLine.class);
	}

	@Override
	public <T extends I_M_InOutLine> List<T> retrieveLines(final I_M_InOut inOut, final Class<T> inoutLineClass)
	{
		final boolean retrieveAll = false;
		return retrieveLines(inOut, retrieveAll, inoutLineClass);
	}

	private <T extends I_M_InOutLine> List<T> retrieveLines(
			@NonNull final I_M_InOut inOut,
			final boolean retrieveAll,
			@NonNull final Class<T> inoutLineClass)
	{
		final IQueryBuilder<I_M_InOutLine> queryBuilder = Services.get(IQueryBL.class).createQueryBuilder(I_M_InOutLine.class, inOut)
				.addEqualsFilter(I_M_InOutLine.COLUMN_M_InOut_ID, inOut.getM_InOut_ID());

		if (!retrieveAll)
		{
			queryBuilder.addOnlyActiveRecordsFilter();
		}

		queryBuilder.orderBy()
				.addColumn(I_M_InOutLine.COLUMNNAME_Line)
				.addColumn(I_M_InOutLine.COLUMNNAME_M_InOutLine_ID);

		final List<T> inoutLines = queryBuilder
				.create()
				.list(inoutLineClass);

		// Optimization: set M_InOut link
		for (final I_M_InOutLine inoutLine : inoutLines)
		{
			inoutLine.setM_InOut(inOut);
		}
		return inoutLines;
	}

	@Override
	public List<I_M_InOutLine> retrieveLinesForInOuts(final Collection<? extends I_M_InOut> inouts)
	{
		Check.assumeNotEmpty(inouts, "inouts is not empty");

		final Map<Integer, I_M_InOut> inoutsById = inouts.stream().collect(GuavaCollectors.toImmutableMapByKey(I_M_InOut::getM_InOut_ID));

		final List<I_M_InOutLine> inoutLines = Services.get(IQueryBL.class).createQueryBuilder(I_M_InOutLine.class)
				.addOnlyActiveRecordsFilter()
				.addInArrayFilter(I_M_InOutLine.COLUMN_M_InOut_ID, inoutsById.keySet())
				.orderBy(I_M_InOutLine.COLUMNNAME_M_InOut_ID)
				.orderBy(I_M_InOutLine.COLUMNNAME_Line)
				.orderBy(I_M_InOutLine.COLUMNNAME_M_InOutLine_ID)
				.create()
				.listImmutable(I_M_InOutLine.class);

		// Optimization: set M_InOut link
		inoutLines.forEach(inoutLine -> inoutLine.setM_InOut(inoutsById.get(inoutLine.getM_InOut_ID())));

		return inoutLines;
	}

	@Override
	public List<I_M_InOutLine> retrieveLinesForOrderLine(final I_C_OrderLine orderLine)
	{
		return retrieveLinesForOrderLine(orderLine, I_M_InOutLine.class);
	}

	@Override
	public <T extends I_M_InOutLine> List<T> retrieveLinesForOrderLine(final I_C_OrderLine orderLine, final Class<T> clazz)
	{
		final IQueryBuilder<I_M_InOutLine> queryBuilder = Services.get(IQueryBL.class).createQueryBuilder(I_M_InOutLine.class, orderLine)
				.addEqualsFilter(I_M_InOutLine.COLUMN_C_OrderLine_ID, orderLine.getC_OrderLine_ID())
				// .filterByClientId()
				.addOnlyActiveRecordsFilter();
		queryBuilder.orderBy()
				.addColumn(I_M_InOutLine.COLUMNNAME_M_InOutLine_ID);

		return queryBuilder.create()
				.list(clazz);
	}

	@Override
	public <T extends I_M_InOutLine> List<T> retrieveLinesWithoutOrderLine(final I_M_InOut inOut, final Class<T> clazz)
	{
		final IQueryBuilder<I_M_InOutLine> queryBuilder = Services.get(IQueryBL.class).createQueryBuilder(I_M_InOutLine.class, inOut)
				.addEqualsFilter(I_M_InOutLine.COLUMNNAME_M_InOut_ID, inOut.getM_InOut_ID())
				.addEqualsFilter(I_M_InOutLine.COLUMNNAME_C_OrderLine_ID, null)
				.addOnlyActiveRecordsFilter()
				.addOnlyContextClient();
		queryBuilder.orderBy()
				.addColumn(I_M_InOutLine.COLUMNNAME_M_InOutLine_ID);

		return queryBuilder.create()
				.list(clazz);
	}

	@Override
	public IQueryBuilder<I_M_InOutLine> createUnprocessedShipmentLinesQuery(final Properties ctx)
	{
		// + " AND io.DocStatus IN ('DR', 'IP','WC')"
		// + " AND io.IsSOTrx='Y'"
		// + " AND iol.AD_Client_ID=?";

		return Services.get(IQueryBL.class).createQueryBuilder(I_M_InOut.class, ctx, ITrx.TRXNAME_None)
				.addInArrayOrAllFilter(I_M_InOut.COLUMNNAME_DocStatus,
						IDocument.STATUS_Drafted,  // task: 07448: we also need to consider drafted shipments, because that's the customer workflow, and qty in a drafted InOut don'T couln'T at picked
						// anymore, because they are already in a shipper-transportation
						IDocument.STATUS_InProgress,
						IDocument.STATUS_WaitingConfirmation)
				.addEqualsFilter(I_M_InOut.COLUMNNAME_IsSOTrx, true)
				.addOnlyActiveRecordsFilter()
				.addOnlyContextClient()
				.andCollectChildren(I_M_InOutLine.COLUMN_M_InOut_ID, I_M_InOutLine.class);
	}

	@Override
	public IQueryBuilder<I_M_InOutLine> retrieveAllReferencingLinesBuilder(final I_M_InOutLine packingMaterialLine)
	{
		return Services.get(IQueryBL.class)
				.createQueryBuilder(I_M_InOutLine.class, packingMaterialLine)
				// .addOnlyActiveRecordsFilter() add all, also inactive ones
				.addEqualsFilter(de.metas.inout.model.I_M_InOutLine.COLUMNNAME_M_PackingMaterial_InOutLine_ID, packingMaterialLine.getM_InOutLine_ID())
				.orderBy()
				.addColumn(I_M_InOutLine.COLUMNNAME_Line)
				.addColumn(I_M_InOutLine.COLUMNNAME_M_InOutLine_ID).endOrderBy();

	}

	@Override
	public List<Integer> retrieveLineIdsWithQualityDiscount(final I_M_InOut inOut)
	{

		final IQueryBuilder<I_M_InOutLine> queryBuilder = createInDisputeQueryBuilder(inOut);
		return queryBuilder
				.create()
				.listIds();
	}

	@Override
	public I_M_InOutLine retrieveLineWithQualityDiscount(@NonNull final I_M_InOutLine originInOutLine)
	{
		final IQueryBuilder<I_M_InOutLine> queryBuilder = createInDisputeQueryBuilder(originInOutLine.getM_InOut());

		final int orderLineID = originInOutLine.getC_OrderLine_ID();

		return queryBuilder.addEqualsFilter(I_M_InOutLine.COLUMNNAME_C_OrderLine_ID, orderLineID)
				.create()
				.firstOnly(I_M_InOutLine.class);
	}

	private IQueryBuilder<I_M_InOutLine> createInDisputeQueryBuilder(final I_M_InOut inOut)
	{

		final IQueryBL queryBL = Services.get(IQueryBL.class);

		final IQueryBuilder<I_M_InOutLine> queryBuilder = queryBL
				.createQueryBuilder(I_M_InOutLine.class, inOut)
				.addOnlyActiveRecordsFilter()
				.addEqualsFilter(de.metas.inout.model.I_M_InOutLine.COLUMNNAME_M_InOut_ID, inOut.getM_InOut_ID())
				.addCompareFilter(de.metas.inout.model.I_M_InOutLine.COLUMNNAME_QualityDiscountPercent, Operator.GREATER, BigDecimal.ZERO);

		return queryBuilder;
	}

	@Override
	public LocalDate getLastInOutDate(@NonNull final BPartnerId bpartnerId, @NonNull final ProductId productId, @NonNull final SOTrx soTrx)
	{
		return Services.get(IQueryBL.class)
				.createQueryBuilder(I_M_InOutLine.class)
				.addEqualsFilter(I_M_InOutLine.COLUMN_M_Product_ID, productId.getRepoId())
				.andCollect(I_M_InOutLine.COLUMN_M_InOut_ID)
				.addEqualsFilter(I_M_InOut.COLUMNNAME_C_BPartner_ID, bpartnerId.getRepoId())
				.addOnlyActiveRecordsFilter()
				.addEqualsFilter(I_M_InOut.COLUMN_IsSOTrx, soTrx.toBoolean())
				.create()
				.aggregate(I_M_InOut.COLUMN_MovementDate, Aggregate.MAX, LocalDate.class);
	}

	@Override
	public I_M_InOut retrieveInOut(@NonNull final List<I_M_InOutLine> receiptLines)
	{
		final int inOutId = CollectionUtils.extractSingleElement(receiptLines, I_M_InOutLine::getM_InOut_ID);

		return load(inOutId, I_M_InOut.class);
	}

	@Override
	public Stream<InOutId> streamInOutIdsByBPartnerId(@NonNull final BPartnerId bpartnerId)
	{
		return Services.get(IQueryBL.class)
				.createQueryBuilder(I_M_InOut.class)
				.addEqualsFilter(I_M_InOut.COLUMNNAME_C_BPartner_ID, bpartnerId)
				.create()
				.listIds(InOutId::ofRepoId)
				.stream();
	}

	@Override
	public Set<InOutAndLineId> retrieveLinesForInOutId(final InOutId inOutId)
	{
		final I_M_InOut inOut = getById(inOutId);

		return retrieveLines(inOut)
				.stream()
				.map(this::extractInOutAndLineId)
				.collect(ImmutableSet.toImmutableSet());
	}

	private InOutAndLineId extractInOutAndLineId(final I_M_InOutLine line)
	{
		return InOutAndLineId.ofRepoId(line.getM_InOut_ID(), line.getM_InOutLine_ID());
	}
}
