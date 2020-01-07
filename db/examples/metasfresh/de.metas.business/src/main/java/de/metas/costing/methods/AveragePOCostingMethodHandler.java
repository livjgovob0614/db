package de.metas.costing.methods;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.time.LocalDate;
import java.util.List;
import java.util.Optional;
import java.util.Properties;

import org.adempiere.ad.trx.api.ITrx;
import org.adempiere.exceptions.AdempiereException;
import org.adempiere.exceptions.DBException;
import org.adempiere.service.ClientId;
import org.compiere.model.I_C_InvoiceLine;
import org.compiere.model.I_M_InOutLine;
import org.compiere.model.I_M_MatchInv;
import org.compiere.util.DB;
import org.compiere.util.Env;
import org.compiere.util.TimeUtil;
import org.springframework.stereotype.Component;

import com.google.common.collect.ImmutableList;

import de.metas.acct.api.AcctSchema;
import de.metas.acct.api.IAcctSchemaDAO;
import de.metas.costing.CostAmount;
import de.metas.costing.CostDetail;
import de.metas.costing.CostDetailAdjustment;
import de.metas.costing.CostDetailCreateRequest;
import de.metas.costing.CostDetailCreateResult;
import de.metas.costing.CostDetailPreviousAmounts;
import de.metas.costing.CostDetailVoidRequest;
import de.metas.costing.CostPrice;
import de.metas.costing.CostSegment;
import de.metas.costing.CostingMethod;
import de.metas.costing.CurrentCost;
import de.metas.currency.CurrencyPrecision;
import de.metas.currency.ICurrencyBL;
import de.metas.inout.IInOutDAO;
import de.metas.inout.InOutLineId;
import de.metas.invoice.IMatchInvDAO;
import de.metas.money.CurrencyConversionTypeId;
import de.metas.money.CurrencyId;
import de.metas.order.IOrderLineBL;
import de.metas.order.OrderLineId;
import de.metas.organization.OrgId;
import de.metas.product.ProductPrice;
import de.metas.quantity.Quantity;
import de.metas.uom.IUOMConversionBL;
import de.metas.uom.UomId;
import de.metas.util.Services;
import lombok.NonNull;

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

@Component
public class AveragePOCostingMethodHandler extends CostingMethodHandlerTemplate
{
	public AveragePOCostingMethodHandler(@NonNull final CostingMethodHandlerUtils utils)
	{
		super(utils);
	}

	@Override
	public CostingMethod getCostingMethod()
	{
		return CostingMethod.AveragePO;
	}

	@Override
	protected CostDetailCreateResult createCostForMatchPO(final CostDetailCreateRequest request)
	{
		return createCostDetailAndAdjustCurrentCosts(request);
	}

	@Override
	protected CostDetailCreateResult createCostForMatchInvoice(final CostDetailCreateRequest request)
	{
		final Quantity qty = request.getQty();
		final UomId qtyUOMId = UomId.ofRepoId(qty.getUOMId());

		final int matchInvId = request.getDocumentRef().getRecordId();
		final CostAmount costPrice = getPOCostPriceForMatchInv(matchInvId)
				.map(price -> convertToUOM(price, qtyUOMId))
				.orElseThrow(() -> new AdempiereException("Cannot fetch PO cost price for " + request))
				.transform(CostAmount::ofProductPrice);
		final CostAmount amt = costPrice.multiply(qty);

		return utils.createCostDetailRecordNoCostsChanged(request.withAmount(amt));
	}

	private ProductPrice convertToUOM(final ProductPrice costPrice, final UomId uomId)
	{
		final IUOMConversionBL uomConversionsBL = Services.get(IUOMConversionBL.class);

		final CurrencyPrecision precision = utils.getCostingPrecision(costPrice.getCurrencyId());
		return uomConversionsBL.convertProductPriceToUom(costPrice, uomId, precision);
	}

	@Override
	protected CostDetailCreateResult createCostForMaterialReceipt(final CostDetailCreateRequest request)
	{
		final Quantity qty = request.getQty();
		final UomId qtyUOMId = qty.getUomId();

		final InOutLineId receiptInOutLineId = InOutLineId.ofRepoId(request.getDocumentRef().getRecordId());
		final CostAmount costPrice = getPOCostPriceForReceiptInOutLine(receiptInOutLineId)
				.map(price -> convertToUOM(price, qtyUOMId))
				.map(CostAmount::ofProductPrice)
				.orElseGet(() -> utils.getCurrentCostPrice(request).toCostAmount());
		final CostAmount amt = costPrice.multiply(qty);
		final CostAmount amtConv = utils.convertToAcctSchemaCurrency(amt, request);

		return utils.createCostDetailRecordNoCostsChanged(request.withAmount(amtConv));
	}

	@Override
	protected CostDetailCreateResult createOutboundCostDefaultImpl(final CostDetailCreateRequest request)
	{
		return createCostDetailAndAdjustCurrentCosts(request);
	}

	private CostDetailCreateResult createCostDetailAndAdjustCurrentCosts(final CostDetailCreateRequest request)
	{
		final Quantity qty = request.getQty();
		final boolean isInboundTrx = qty.signum() > 0;

		final CurrentCost currentCosts = utils.getCurrentCost(request);
		final CostDetailCreateResult result;
		if (isInboundTrx)
		{
			result = utils.createCostDetailRecordWithChangedCosts(request, currentCosts);

			currentCosts.addWeightedAverage(request.getAmt(), qty);
		}
		else
		{
			final CostPrice price = currentCosts.getCostPrice();
			final CostAmount amt = price.multiply(qty).roundToPrecisionIfNeeded(currentCosts.getPrecision());
			final CostDetailCreateRequest requestEffective = request.withAmount(amt);
			result = utils.createCostDetailRecordWithChangedCosts(requestEffective, currentCosts);

			currentCosts.addToCurrentQtyAndCumulate(qty, amt);
		}

		utils.saveCurrentCost(currentCosts);

		return result;
	}

	@Override
	public void voidCosts(final CostDetailVoidRequest request)
	{
		final Quantity qty = request.getQty();
		final boolean isInboundTrx = qty.signum() > 0;
		final CurrentCost currentCosts = utils.getCurrentCost(request.getCostSegmentAndElement());
		if (isInboundTrx)
		{
			currentCosts.addWeightedAverage(request.getAmt().negate(), qty.negate());
		}
		else
		{
			currentCosts.addToCurrentQtyAndCumulate(qty.negate(), request.getAmt().negate());
		}

		utils.saveCurrentCost(currentCosts);
	}

	private Optional<ProductPrice> getPOCostPriceForMatchInv(final int matchInvId)
	{
		final IMatchInvDAO matchInvoicesRepo = Services.get(IMatchInvDAO.class);
		final IOrderLineBL orderLineBL = Services.get(IOrderLineBL.class);

		final I_M_MatchInv matchInv = matchInvoicesRepo.getById(matchInvId);
		return Optional.of(matchInv)
				.map(I_M_MatchInv::getC_InvoiceLine)
				.map(I_C_InvoiceLine::getC_OrderLine)
				.map(orderLineBL::getCostPrice);
	}

	private Optional<ProductPrice> getPOCostPriceForReceiptInOutLine(final InOutLineId receiptInOutLineId)
	{
		final IInOutDAO inoutsRepo = Services.get(IInOutDAO.class);

		final I_M_InOutLine receiptLine = inoutsRepo.getLineById(receiptInOutLineId);
		return Optional.of(receiptLine)
				.map(I_M_InOutLine::getC_OrderLine)
				.map(Services.get(IOrderLineBL.class)::getCostPrice);
	}

	@Override
	public Optional<CostAmount> calculateSeedCosts(final CostSegment costSegment, final OrderLineId orderLineId_NOTUSED)
	{
		final ICurrencyBL currencyConversionBL = Services.get(ICurrencyBL.class);
		final Properties ctx = Env.getCtx();
		final int productId = costSegment.getProductId().getRepoId();
		final int AD_Org_ID = costSegment.getOrgId().getRepoId();
		final int M_AttributeSetInstance_ID = costSegment.getAttributeSetInstanceId().getRepoId();
		final AcctSchema acctSchema = Services.get(IAcctSchemaDAO.class).getById(costSegment.getAcctSchemaId());
		final CurrencyId acctCurencyId = acctSchema.getCurrencyId();
		final CurrencyPrecision costingPrecision = acctSchema.getCosting().getCostingPrecision();

		String sql = "SELECT t.MovementQty, mp.Qty, ol.QtyOrdered, ol.PriceCost, ol.PriceActual,"	// 1..5
				+ " o.C_Currency_ID, o.DateAcct, o.C_ConversionType_ID,"	// 6..8
				+ " o.AD_Client_ID, o.AD_Org_ID, t.M_Transaction_ID "		// 9..11
				+ "FROM M_Transaction t"
				+ " INNER JOIN M_MatchPO mp ON (t.M_InOutLine_ID=mp.M_InOutLine_ID)"
				+ " INNER JOIN C_OrderLine ol ON (mp.C_OrderLine_ID=ol.C_OrderLine_ID)"
				+ " INNER JOIN C_Order o ON (ol.C_Order_ID=o.C_Order_ID) "
				+ "WHERE t.M_Product_ID=?";
		if (AD_Org_ID != 0)
		{
			sql += " AND t.AD_Org_ID=?";
		}
		else if (M_AttributeSetInstance_ID != 0)
		{
			sql += " AND t.M_AttributeSetInstance_ID=?";
		}
		sql += " ORDER BY t.M_Transaction_ID";

		PreparedStatement pstmt = null;
		ResultSet rs = null;
		BigDecimal newStockQty = BigDecimal.ZERO;
		//
		BigDecimal newAverageAmt = BigDecimal.ZERO;
		final int oldTransaction_ID = 0;
		try
		{
			pstmt = DB.prepareStatement(sql, ITrx.TRXNAME_None);
			pstmt.setInt(1, productId);
			if (AD_Org_ID != 0)
			{
				pstmt.setInt(2, AD_Org_ID);
			}
			else if (M_AttributeSetInstance_ID != 0)
			{
				pstmt.setInt(2, M_AttributeSetInstance_ID);
			}
			rs = pstmt.executeQuery();
			while (rs.next())
			{
				final BigDecimal oldStockQty = newStockQty;
				final BigDecimal movementQty = rs.getBigDecimal(1);
				int M_Transaction_ID = rs.getInt(11);
				if (M_Transaction_ID != oldTransaction_ID)
				{
					newStockQty = oldStockQty.add(movementQty);
				}
				M_Transaction_ID = oldTransaction_ID;
				//
				final BigDecimal matchQty = rs.getBigDecimal(2);
				if (matchQty == null)
				{
					continue;
				}
				// Assumption: everything is matched
				BigDecimal price = rs.getBigDecimal(4);
				if (price == null || price.signum() == 0)
				{
					price = rs.getBigDecimal(5);			// Actual
				}
				final CurrencyId C_Currency_ID = CurrencyId.ofRepoId(rs.getInt(6));
				final LocalDate DateAcct = TimeUtil.asLocalDate(rs.getTimestamp(7));
				final CurrencyConversionTypeId C_ConversionType_ID = CurrencyConversionTypeId.ofRepoIdOrNull(rs.getInt(8));
				final ClientId Client_ID = ClientId.ofRepoId(rs.getInt(9));
				final OrgId Org_ID = OrgId.ofRepoId(rs.getInt(10));
				final BigDecimal cost = currencyConversionBL.convert(
						price,
						C_Currency_ID,
						acctCurencyId,
						DateAcct,
						C_ConversionType_ID,
						Client_ID,
						Org_ID);
				//
				final BigDecimal oldAverageAmt = newAverageAmt;
				final BigDecimal averageCurrent = oldStockQty.multiply(oldAverageAmt);
				final BigDecimal averageIncrease = matchQty.multiply(cost);
				BigDecimal newAmt = averageCurrent.add(averageIncrease);
				newAmt = newAmt.setScale(costingPrecision.toInt(), RoundingMode.HALF_UP);
				newAverageAmt = newAmt.divide(newStockQty, costingPrecision.toInt(), RoundingMode.HALF_UP);
			}
		}
		catch (final SQLException e)
		{
			throw new DBException(e, sql);
		}
		finally
		{
			DB.close(rs, pstmt);
			rs = null;
			pstmt = null;
		}
		//
		if (newAverageAmt != null && newAverageAmt.signum() != 0)
		{
			return Optional.of(CostAmount.of(newAverageAmt, acctCurencyId));
		}
		else
		{
			return Optional.empty();
		}
	}

	public void adjustInboundCostDetailAmount(
			@NonNull final CostDetail costDetail,
			@NonNull final CostAmount amount)
	{
		if (costDetail.getAmt().equals(amount))
		{
			return;
		}
		if (!costDetail.isInboundTrx())
		{
			throw new AdempiereException("Only inbound cost details can be adjusted: " + costDetail);
		}

		final CurrentCost currentCost = utils.getCurrentCost(costDetail);
		currentCost.setFrom(costDetail.getPreviousAmounts());

		currentCost.addWeightedAverage(amount, costDetail.getQty());

		final List<CostDetailAdjustment> nextCostDetailAdjustments = utils.streamAllCostDetailsAfter(costDetail)
				.map(nextCostDetail -> recalculateCostDetailAmount(nextCostDetail, currentCost))
				.collect(ImmutableList.toImmutableList());

		//
		// TODO: Create the final cost detail which is about posting the adjustment
		currentCost.getCostPrice();
		currentCost.getCurrentQty();
		currentCost.getCumulatedAmt();
		currentCost.getCumulatedQty();
	}

	private CostDetailAdjustment recalculateCostDetailAmount(final CostDetail costDetail, final CurrentCost currentCost)
	{
		final CostDetailPreviousAmounts previousAmounts = CostDetailPreviousAmounts.of(currentCost);
		final Quantity qty = costDetail.getQty();
		final CostAmount amt;

		//
		// Inbound
		if (costDetail.isInboundTrx())
		{
			amt = costDetail.getAmt();
			currentCost.addWeightedAverage(amt, qty);
		}
		//
		// Outbound
		else
		{
			amt = currentCost.getCostPrice()
					.multiply(qty)
					.roundToPrecisionIfNeeded(currentCost.getPrecision());

			currentCost.addToCurrentQtyAndCumulate(qty, amt);
		}

		//
		return CostDetailAdjustment.builder()
				.costDetailId(costDetail.getId())
				.amt(amt)
				.qty(qty)
				.previousAmounts(previousAmounts)
				.build();
	}
}
