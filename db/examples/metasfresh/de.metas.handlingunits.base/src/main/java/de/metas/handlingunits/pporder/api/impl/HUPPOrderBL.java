package de.metas.handlingunits.pporder.api.impl;

import java.util.Collection;

import org.adempiere.ad.trx.api.ITrxManager;
import org.adempiere.model.InterfaceWrapperHelper;
import org.adempiere.warehouse.WarehouseId;
import org.eevolution.api.IPPOrderBL;
import org.eevolution.api.IPPOrderDAO;
import org.eevolution.api.PPOrderPlanningStatus;
import org.eevolution.model.I_PP_Order_BOMLine;

import com.google.common.base.Objects;
import com.google.common.collect.ImmutableMultimap;

import de.metas.handlingunits.IHUAssignmentBL;
import de.metas.handlingunits.IHUQueryBuilder;
import de.metas.handlingunits.IHandlingUnitsDAO;
import de.metas.handlingunits.allocation.IAllocationSource;
import de.metas.handlingunits.allocation.impl.GenericAllocationSourceDestination;
import de.metas.handlingunits.impl.DocumentLUTUConfigurationManager;
import de.metas.handlingunits.impl.IDocumentLUTUConfigurationManager;
import de.metas.handlingunits.model.I_M_HU;
import de.metas.handlingunits.model.I_PP_Order;
import de.metas.handlingunits.model.X_M_HU;
import de.metas.handlingunits.pporder.api.HUPPOrderIssueProducer;
import de.metas.handlingunits.pporder.api.HUPPOrderIssueReceiptCandidatesProcessor;
import de.metas.handlingunits.pporder.api.IHUPPOrderBL;
import de.metas.handlingunits.pporder.api.IPPOrderReceiptHUProducer;
import de.metas.material.planning.pporder.IPPOrderBOMDAO;
import de.metas.material.planning.pporder.PPOrderBOMLineId;
import de.metas.material.planning.pporder.PPOrderId;
import de.metas.product.ProductId;
import de.metas.util.Services;
import lombok.NonNull;

public class HUPPOrderBL implements IHUPPOrderBL
{
	@Override
	public I_PP_Order getById(@NonNull final PPOrderId ppOrderId)
	{
		return Services.get(IPPOrderDAO.class).getById(ppOrderId, I_PP_Order.class);
	}

	@Override
	public IDocumentLUTUConfigurationManager createReceiptLUTUConfigurationManager(@NonNull final org.eevolution.model.I_PP_Order ppOrder)
	{
		final de.metas.handlingunits.model.I_PP_Order documentLine = InterfaceWrapperHelper.create(ppOrder, de.metas.handlingunits.model.I_PP_Order.class);
		return new DocumentLUTUConfigurationManager<>(documentLine, PPOrderDocumentLUTUConfigurationHandler.instance);
	}

	@Override
	public IDocumentLUTUConfigurationManager createReceiptLUTUConfigurationManager(@NonNull final org.eevolution.model.I_PP_Order_BOMLine ppOrderBOMLine)
	{
		final de.metas.handlingunits.model.I_PP_Order_BOMLine documentLine = InterfaceWrapperHelper.create(ppOrderBOMLine, de.metas.handlingunits.model.I_PP_Order_BOMLine.class);
		return new DocumentLUTUConfigurationManager<>(documentLine, PPOrderBOMLineDocumentLUTUConfigurationHandler.instance);
	}

	@Override
	public IAllocationSource createAllocationSourceForPPOrder(final I_PP_Order ppOrder)
	{
		final PPOrderProductStorage ppOrderProductStorage = new PPOrderProductStorage(ppOrder);
		final IAllocationSource ppOrderAllocationSource = new GenericAllocationSourceDestination(
				ppOrderProductStorage,
				ppOrder // referenced model
		);
		return ppOrderAllocationSource;
	}

	@Override
	public HUPPOrderIssueProducer createIssueProducer(@NonNull final PPOrderId ppOrderId)
	{
		return new HUPPOrderIssueProducer(ppOrderId);
	}

	@Override
	public IPPOrderReceiptHUProducer receivingMainProduct(@NonNull final PPOrderId ppOrderId)
	{
		final I_PP_Order ppOrder = getById(ppOrderId);
		return new CostCollectorCandidateFinishedGoodsHUProducer(ppOrder);
	}

	@Override
	public IPPOrderReceiptHUProducer receivingByOrCoProduct(@NonNull final PPOrderBOMLineId orderBOMLineId)
	{
		final IPPOrderBOMDAO ppOrderBOMsRepo = Services.get(IPPOrderBOMDAO.class);
		final I_PP_Order_BOMLine orderBOMLine = ppOrderBOMsRepo.getOrderBOMLineById(orderBOMLineId);
		return new CostCollectorCandidateCoProductHUProducer(orderBOMLine);
	}

	@Override
	public IHUQueryBuilder createHUsAvailableToIssueQuery(@NonNull final I_PP_Order_BOMLine ppOrderBomLine)
	{
		final IHandlingUnitsDAO handlingUnitsDAO = Services.get(IHandlingUnitsDAO.class);
		return handlingUnitsDAO
				.createHUQueryBuilder()
				.addOnlyWithProductId(ProductId.ofRepoId(ppOrderBomLine.getM_Product_ID()))
				.addOnlyInWarehouseId(WarehouseId.ofRepoId(ppOrderBomLine.getM_Warehouse_ID()))
				.addHUStatusToInclude(X_M_HU.HUSTATUS_Active)
				.setExcludeReserved()
				.setOnlyTopLevelHUs()
				.onlyNotLocked();
	}

	private static final ImmutableMultimap<PPOrderPlanningStatus, PPOrderPlanningStatus> fromPlanningStatus2toPlanningStatusAllowed = ImmutableMultimap.<PPOrderPlanningStatus, PPOrderPlanningStatus> builder()
			.put(PPOrderPlanningStatus.PLANNING, PPOrderPlanningStatus.REVIEW)
			.put(PPOrderPlanningStatus.PLANNING, PPOrderPlanningStatus.COMPLETE)
			.put(PPOrderPlanningStatus.REVIEW, PPOrderPlanningStatus.PLANNING)
			.put(PPOrderPlanningStatus.REVIEW, PPOrderPlanningStatus.COMPLETE)
			// .put(PPOrderPlanningStatus.COMPLETE, PPOrderPlanningStatus.PLANNING) // don't allow this transition unless https://github.com/metasfresh/metasfresh/issues/2708 is done
			.build();

	@Override
	public boolean canChangePlanningStatus(final PPOrderPlanningStatus fromPlanningStatus, final PPOrderPlanningStatus toPlanningStatus)
	{
		return fromPlanningStatus2toPlanningStatusAllowed.get(fromPlanningStatus).contains(toPlanningStatus);
	}

	@Override
	public void processPlanning(@NonNull final PPOrderPlanningStatus targetPlanningStatus, @NonNull final PPOrderId ppOrderId)
	{
		Services.get(ITrxManager.class).assertThreadInheritedTrxExists();

		final I_PP_Order ppOrder = Services.get(IPPOrderDAO.class).getById(ppOrderId, I_PP_Order.class);
		final PPOrderPlanningStatus planningStatus = PPOrderPlanningStatus.ofCode(ppOrder.getPlanningStatus());
		if (Objects.equal(planningStatus, targetPlanningStatus))
		{
			throw new IllegalStateException("Already " + targetPlanningStatus);
		}
		if (!canChangePlanningStatus(planningStatus, targetPlanningStatus))
		{
			throw new IllegalStateException("Cannot change planning status from " + planningStatus + " to " + targetPlanningStatus);
		}

		if (PPOrderPlanningStatus.PLANNING.equals(targetPlanningStatus))
		{
			// nothing
		}
		else if (PPOrderPlanningStatus.REVIEW.equals(targetPlanningStatus))
		{
			// nothing
		}
		else if (PPOrderPlanningStatus.COMPLETE.equals(targetPlanningStatus))
		{
			HUPPOrderIssueReceiptCandidatesProcessor.newInstance()
					.setCandidatesToProcessByPPOrderId(ppOrderId)
					.process();
		}
		else
		{
			throw new IllegalArgumentException("Unknown target planning status: " + targetPlanningStatus);
		}

		//
		// Update ppOrder's planning status
		ppOrder.setPlanningStatus(targetPlanningStatus.getCode());
		InterfaceWrapperHelper.save(ppOrder);
	}

	@Override
	public void setAssignedHandlingUnits(@NonNull final I_PP_Order ppOrder, @NonNull final Collection<I_M_HU> hus)
	{
		final IHUAssignmentBL huAssignmentBL = Services.get(IHUAssignmentBL.class);
		huAssignmentBL.setAssignedHandlingUnits(ppOrder, hus);
	}

	@Override
	public void setAssignedHandlingUnits(@NonNull final I_PP_Order_BOMLine ppOrderBOMLine, @NonNull final Collection<I_M_HU> hus)
	{
		final IHUAssignmentBL huAssignmentBL = Services.get(IHUAssignmentBL.class);
		huAssignmentBL.setAssignedHandlingUnits(ppOrderBOMLine, hus);
	}

	@Override
	public void closeOrder(@NonNull final PPOrderId ppOrderId)
	{
		final IPPOrderBL ppOrdersService = Services.get(IPPOrderBL.class);
		ppOrdersService.closeOrder(ppOrderId);
	}

}
