package de.metas.fresh.mrp_productinfo.impl;

import java.time.ZonedDateTime;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.stream.Collectors;

import org.adempiere.ad.trx.api.ITrx;
import org.adempiere.mm.attributes.api.IAttributeSetInstanceAware;
import org.adempiere.mm.attributes.api.IAttributeSetInstanceAwareFactoryService;
import org.adempiere.model.InterfaceWrapperHelper;
import org.adempiere.util.api.IParams;
import org.adempiere.util.lang.ITableRecordReference;
import org.adempiere.util.lang.ObjectUtils;
import org.adempiere.util.lang.impl.TableRecordReference;
import org.apache.commons.lang3.builder.CompareToBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;
import org.compiere.model.I_C_Order;
import org.compiere.model.I_C_OrderLine;
import org.compiere.model.I_M_InOutLine;
import org.compiere.model.I_M_MovementLine;
import org.compiere.model.I_M_Transaction;
import org.compiere.util.DB;
import org.compiere.util.TimeUtil;

import com.google.common.annotations.VisibleForTesting;
import com.google.common.collect.ImmutableMap;

import de.metas.fresh.model.I_Fresh_QtyOnHand;
import de.metas.fresh.model.I_Fresh_QtyOnHand_Line;
import de.metas.fresh.model.I_X_MRP_ProductInfo_Detail_MV;
import de.metas.fresh.mrp_productinfo.IMRPProductInfoSelector;
import de.metas.fresh.mrp_productinfo.IMRPProductInfoSelectorFactory;
import de.metas.inoutcandidate.api.IShipmentScheduleEffectiveBL;
import de.metas.inoutcandidate.api.IShipmentSchedulePA;
import de.metas.inoutcandidate.model.I_M_ShipmentSchedule;
import de.metas.order.OrderLineId;
import de.metas.procurement.base.model.I_PMM_PurchaseCandidate;
import de.metas.util.Check;
import de.metas.util.Services;
import lombok.Getter;
import lombok.NonNull;

/*
 * #%L
 * de.metas.fresh.base
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

public class MRPProductInfoSelectorFactory implements IMRPProductInfoSelectorFactory
{

	static final String DATE_PARAM_SUFFIX = "Date";
	static final String ASI_PARAM_SUFFIX = "M_AttributeSetInstance_ID";
	static final String PRODUCT_PARAM_SUFFIX = "M_Product_ID";

	@Override
	public IMRPProductInfoSelector createOrNullForModel(final Object model)
	{
		if (model == null)
		{
			return null;
		}

		final IAttributeSetInstanceAwareFactoryService attributeSetInstanceAwareFactory = Services.get(IAttributeSetInstanceAwareFactoryService.class);

		if (InterfaceWrapperHelper.isInstanceOf(model, I_C_OrderLine.class))
		{
			// note: we are interested in any DocStatus, because e.g. the order might have been reactivated
			final I_C_OrderLine orderLine = InterfaceWrapperHelper.create(model, I_C_OrderLine.class);
			final IAttributeSetInstanceAware asiAware = attributeSetInstanceAwareFactory.createOrNull(model);

			final ZonedDateTime date = getDateForOrderLine(orderLine);
			return new MRPProductInfoSelector(asiAware.getM_Product_ID(), asiAware.getM_AttributeSetInstance_ID(), date, model, null);
		}
		else if (InterfaceWrapperHelper.isInstanceOf(model, I_M_MovementLine.class))
		{
			final I_M_MovementLine movementLine = InterfaceWrapperHelper.create(model, I_M_MovementLine.class);
			final IAttributeSetInstanceAware asiAware = attributeSetInstanceAwareFactory.createOrNull(model);

			final ZonedDateTime date = TimeUtil.asZonedDateTime(movementLine.getM_Movement().getMovementDate());
			return new MRPProductInfoSelector(asiAware.getM_Product_ID(), asiAware.getM_AttributeSetInstance_ID(), date, model, null);
		}
		else if (InterfaceWrapperHelper.isInstanceOf(model, I_M_InOutLine.class))
		{
			final I_M_InOutLine inOutLine = InterfaceWrapperHelper.create(model, I_M_InOutLine.class);
			final IAttributeSetInstanceAware asiAware = attributeSetInstanceAwareFactory.createOrNull(model);

			final ZonedDateTime date = TimeUtil.asZonedDateTime(inOutLine.getM_InOut().getMovementDate());
			return new MRPProductInfoSelector(asiAware.getM_Product_ID(), asiAware.getM_AttributeSetInstance_ID(), date, model, null);
		}
		else if (InterfaceWrapperHelper.isInstanceOf(model, I_Fresh_QtyOnHand_Line.class))
		{
			final I_Fresh_QtyOnHand_Line onHandLine = InterfaceWrapperHelper.create(model, I_Fresh_QtyOnHand_Line.class);
			if (!onHandLine.isActive())
			{
				return null;
			}
			final I_Fresh_QtyOnHand qtyOnHand = onHandLine.getFresh_QtyOnHand();
			if (!qtyOnHand.isActive())
			{
				return null;
			}
			final IAttributeSetInstanceAware asiAware = attributeSetInstanceAwareFactory.createOrNull(model);

			final ZonedDateTime date = TimeUtil.asZonedDateTime(qtyOnHand.getDateDoc());
			return new MRPProductInfoSelector(asiAware.getM_Product_ID(), asiAware.getM_AttributeSetInstance_ID(), date, model, null);
		}
		else if (InterfaceWrapperHelper.isInstanceOf(model, I_PMM_PurchaseCandidate.class))
		{
			final I_PMM_PurchaseCandidate purchaseCandidate = InterfaceWrapperHelper.create(model, I_PMM_PurchaseCandidate.class);
			final IAttributeSetInstanceAware asiAware = attributeSetInstanceAwareFactory.createOrNull(model);

			final ZonedDateTime date = TimeUtil.asZonedDateTime(purchaseCandidate.getDatePromised());
			return new MRPProductInfoSelector(asiAware.getM_Product_ID(), asiAware.getM_AttributeSetInstance_ID(), date, model, null);
		}
		else if (InterfaceWrapperHelper.isInstanceOf(model, I_M_Transaction.class))
		{
			final I_M_Transaction transaction = InterfaceWrapperHelper.create(model, I_M_Transaction.class);
			final ZonedDateTime date = TimeUtil.asZonedDateTime(transaction.getMovementDate());

			return new MRPProductInfoSelector(transaction.getM_Product_ID(), transaction.getM_AttributeSetInstance_ID(), date, model, null);
		}

		if (InterfaceWrapperHelper.isInstanceOf(model, I_X_MRP_ProductInfo_Detail_MV.class))
		{
			final I_X_MRP_ProductInfo_Detail_MV detail = InterfaceWrapperHelper.create(model, I_X_MRP_ProductInfo_Detail_MV.class);
			final ZonedDateTime date = TimeUtil.asZonedDateTime(detail.getDateGeneral());
			
			return new MRPProductInfoSelector(detail.getM_Product_ID(), detail.getM_AttributeSetInstance_ID(), date, model, null);
		}

		return null;
	}

	private ZonedDateTime getDateForOrderLine(final I_C_OrderLine orderLine)
	{
		final OrderLineId orderLineId = OrderLineId.ofRepoId(orderLine.getC_OrderLine_ID());
		final I_M_ShipmentSchedule schedForOrderLine = Services.get(IShipmentSchedulePA.class).getByOrderLineId(orderLineId);
		if (schedForOrderLine != null)
		{
			final ZonedDateTime date = Services.get(IShipmentScheduleEffectiveBL.class).getPreparationDate(schedForOrderLine);
			if (date != null)
			{
				return date;
			}
		}

		final ZonedDateTime date;
		final I_C_Order order = orderLine.getC_Order();
		if (order.getPreparationDate() != null)
		{
			date = TimeUtil.asZonedDateTime(order.getPreparationDate());
		}
		else
		{
			date = TimeUtil.asZonedDateTime(order.getDatePromised());
		}
		Check.errorIf(date == null, "Unable to obtain a date for order line {}", orderLine);
		return date;
	}

	@Override
	public List<IMRPProductInfoSelector> createForParams(final IParams params)
	{
		// we could to this in one stream, but i think this way it's easier to debug
		final List<String> paramPrefices = params.getParameterNames().stream()
				.map(name -> getPrefix(name))
				.sorted()
				.distinct()
				.collect(Collectors.toList());

		return paramPrefices.stream()
				.map(paramPrefix -> createSingleForParams(params, paramPrefix))
				.collect(Collectors.toList());
	}

	@VisibleForTesting
	String getPrefix(final String parameterName)
	{
		Check.assumeNotEmpty(parameterName, "Param 'parameterName' is not empty");
		if (parameterName.endsWith(PRODUCT_PARAM_SUFFIX))
		{
			return parameterName.substring(0, parameterName.length() - PRODUCT_PARAM_SUFFIX.length());
		}
		else if (parameterName.endsWith(ASI_PARAM_SUFFIX))
		{
			return parameterName.substring(0, parameterName.length() - ASI_PARAM_SUFFIX.length());
		}
		else if (parameterName.endsWith(DATE_PARAM_SUFFIX))
		{
			return parameterName.substring(0, parameterName.length() - DATE_PARAM_SUFFIX.length());
		}

		Check.errorIf(true, "parameterName={} needs to end with one of our prefices", parameterName);
		return null;
	}

	@VisibleForTesting
	IMRPProductInfoSelector createSingleForParams(final IParams params, final String paramPrefix)
	{
		Check.assumeNotNull(params, "param 'params' is not null");

		Check.errorUnless(params.hasParameter(paramPrefix + PRODUCT_PARAM_SUFFIX), "Missing parameter {} in params={}", paramPrefix + PRODUCT_PARAM_SUFFIX, params);
		final int productID = params.getParameterAsInt(paramPrefix + PRODUCT_PARAM_SUFFIX, -1);
		Check.errorIf(productID <= 0, "Params={} has {} <= 0", params, paramPrefix + PRODUCT_PARAM_SUFFIX);

		Check.errorUnless(params.hasParameter(paramPrefix + ASI_PARAM_SUFFIX), "Missing parameter {} in params={}", paramPrefix + ASI_PARAM_SUFFIX, params);
		final int asiID = params.getParameterAsInt(paramPrefix + ASI_PARAM_SUFFIX, -1);
		Check.errorIf(asiID < 0, "Params={} has {} < 0", params, paramPrefix + ASI_PARAM_SUFFIX);

		Check.errorUnless(params.hasParameter(paramPrefix + DATE_PARAM_SUFFIX), "Missing parameter {} in params={}", paramPrefix + DATE_PARAM_SUFFIX, params);
		final ZonedDateTime date = params.getParameterAsZonedDateTime(paramPrefix + DATE_PARAM_SUFFIX);
		Check.errorIf(date == null, "Params={} has {} == null", params, paramPrefix + DATE_PARAM_SUFFIX);

		// if all values are OK, then use them. Otherwise they might not be consistent with each other
		return new MRPProductInfoSelector(productID, asiID, date, null, paramPrefix);
	}

	/**
	 * Important: check out the {@link #hashCode()} and {@link #equals(Object)} implementation before using instances in hashsets etc.
	 *
	 * @author metas-dev <dev@metasfresh.com>
	 *
	 */
	static class MRPProductInfoSelector implements IMRPProductInfoSelector
	{
		@Getter
		private final int M_Product_ID;

		@Getter
		private final int M_AttributeSetInstance_ID;

		private String asiKey;

		@Getter
		private final ZonedDateTime date;

		private final String paramPrefix;

		private final Map<String, Object> parametersMap;

		@Getter
		private final Properties ctx;

		@Getter
		private final String trxName;

		/**
		 *
		 * @param M_Product_ID
		 * @param M_AttributeSetInstance_ID
		 * @param date
		 * @param model
		 * @param paramPrefix ignored, unless the given {@code model} is {@code null}.
		 */
		private MRPProductInfoSelector(
				final int M_Product_ID,
				final int M_AttributeSetInstance_ID,
				@NonNull final ZonedDateTime date,
				final Object model,
				final String paramPrefix)
		{
			Check.assume(M_Product_ID > 0, "Param 'M_Product_ID' > 0");
			Check.assume(M_AttributeSetInstance_ID >= 0, "Param 'M_AttributeSetInstance_ID' >= 0");

			this.M_Product_ID = M_Product_ID;
			this.M_AttributeSetInstance_ID = M_AttributeSetInstance_ID;
			this.date = date;

			if (model == null)
			{
				this.paramPrefix = paramPrefix;
				this.ctx = null;
				this.trxName = null;
			}
			else
			{
				this.paramPrefix = createParamPrefix(model);
				this.ctx = InterfaceWrapperHelper.getCtx(model);
				this.trxName = InterfaceWrapperHelper.getTrxName(model);
			}
			this.parametersMap = ImmutableMap.<String, Object> of(
					mkProductParamKey(this.paramPrefix), getM_Product_ID(),
					mkAttributeSetInstanceParamKey(this.paramPrefix), getM_AttributeSetInstance_ID(),
					mkDateParamKey(this.paramPrefix), getDate());
		}

		@Override
		public int compareTo(final IMRPProductInfoSelector o)
		{
			final CompareToBuilder append = new CompareToBuilder()
					.append(o.getM_Product_ID(), getM_Product_ID())
					.append(o.getM_AttributeSetInstance_ID(), getM_AttributeSetInstance_ID())
					.append(o.getDate(), getDate());
			return append.build();
		}

		@Override
		public Map<String, Object> asMap()
		{
			return parametersMap;
		}

		private static String createParamPrefix(@NonNull final Object model)
		{
			final ITableRecordReference record = TableRecordReference.of(model);

			final String prefix = record.getTableName() + "_" + record.getRecord_ID() + "_";
			return prefix;
		}

		private static String mkProductParamKey(@NonNull final String paramPrefix)
		{
			return paramPrefix + PRODUCT_PARAM_SUFFIX;
		}

		private static String mkAttributeSetInstanceParamKey(@NonNull final String paramPrefix)
		{
			return paramPrefix + ASI_PARAM_SUFFIX;
		}

		private static String mkDateParamKey(@NonNull final String paramPrefix)
		{
			return paramPrefix + DATE_PARAM_SUFFIX;
		}

		@Override
		public String toString()
		{
			return toStringForDebugging();
		}

		private String toStringForDebugging()
		{
			return ObjectUtils.toString(this);
		}

		@Override
		public String toStringForRegularLogging()
		{
			return getParamPrefix()
					+ "[Date=" + getDate()
					+ ",M_Product_ID=" + getM_Product_ID()
					+ ",M_AttributeSetInstance_ID=" + getM_AttributeSetInstance_ID()
					+ "]";
		}

		@Override
		public int hashCode()
		{
			return new HashCodeBuilder().append(date).append(M_Product_ID).append(getASIKey()).build();
		}

		@Override
		public boolean equals(Object obj)
		{
			return obj instanceof MRPProductInfoSelector && hashCode() == obj.hashCode();
		}

		@Override
		public String getASIKey()
		{
			if (asiKey == null)
			{
				final int attributeSetInstanceID = getM_AttributeSetInstance_ID();
				asiKey = DB.getSQLValueString(ITrx.TRXNAME_None,
						"SELECT GenerateHUStorageASIKey(?, '')",               // important to get an empty string instead of NULL
						attributeSetInstanceID);
			}
			return asiKey;
		}

		@Override
		public String getParamPrefix()
		{
			return paramPrefix == null ? "<null>" : paramPrefix;
		}

	}
}
