package org.eevolution.process;

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

import java.util.Collections;
import java.util.List;

import org.adempiere.ad.model.util.ModelByIdComparator;
import org.adempiere.exceptions.AdempiereException;
import org.compiere.model.ModelValidationEngine;
import org.compiere.model.ModelValidator;
import org.eevolution.api.CostCollectorType;
import org.eevolution.api.IPPCostCollectorDAO;
import org.eevolution.api.IPPOrderBL;
import org.eevolution.api.IPPOrderDAO;
import org.eevolution.model.I_PP_Cost_Collector;
import org.eevolution.model.I_PP_Order;
import org.eevolution.model.I_PP_Order_BOMLine;
import org.eevolution.model.X_PP_Order;

import de.metas.document.engine.DocStatus;
import de.metas.document.engine.IDocument;
import de.metas.document.engine.IDocumentBL;
import de.metas.material.planning.pporder.IPPOrderBOMBL;
import de.metas.material.planning.pporder.IPPOrderBOMDAO;
import de.metas.material.planning.pporder.PPOrderId;
import de.metas.process.IProcessPrecondition;
import de.metas.process.IProcessPreconditionsContext;
import de.metas.process.JavaProcess;
import de.metas.process.ProcessPreconditionsResolution;
import de.metas.util.Check;
import de.metas.util.Services;

/**
 * Unclose a manufacturing order.
 *
 * @author tsa
 * @task 08731
 */
public class PP_Order_UnClose extends JavaProcess implements IProcessPrecondition
{
	// services
	private final transient IDocumentBL docActionBL = Services.get(IDocumentBL.class);
	private final transient IPPOrderBL ppOrderBL = Services.get(IPPOrderBL.class);
	private final transient IPPOrderDAO ppOrdersRepo = Services.get(IPPOrderDAO.class);
	private final transient IPPOrderBOMBL ppOrderBOMBL = Services.get(IPPOrderBOMBL.class);
	private final transient IPPCostCollectorDAO ppCostCollectorDAO = Services.get(IPPCostCollectorDAO.class);

	@Override
	public ProcessPreconditionsResolution checkPreconditionsApplicable(final IProcessPreconditionsContext context)
	{
		final I_PP_Order ppOrder = context.getSelectedModel(I_PP_Order.class);
		return ProcessPreconditionsResolution.acceptIf(isEligible(ppOrder));
	}

	private boolean isEligible(I_PP_Order ppOrder)
	{
		if (!X_PP_Order.DOCSTATUS_Closed.equals(ppOrder.getDocStatus()))
		{
			return false;
		}

		return true;
	}

	@Override
	protected void prepare()
	{
		// nothing
	}

	@Override
	protected String doIt()
	{
		final PPOrderId ppOrderId = getPPOrderId();
		final I_PP_Order ppOrder = ppOrdersRepo.getById(ppOrderId);
		if (!isEligible(ppOrder))
		{
			throw new AdempiereException("@NotValid@ " + ppOrder);
		}

		unclose(ppOrder);

		return MSG_OK;
	}

	private PPOrderId getPPOrderId()
	{
		Check.assumeEquals(getTableName(), I_PP_Order.Table_Name, "TableName");
		return PPOrderId.ofRepoId(getRecord_ID());
	}

	private void unclose(final I_PP_Order ppOrder)
	{
		ModelValidationEngine.get().fireDocValidate(ppOrder, ModelValidator.TIMING_BEFORE_UNCLOSE);

		//
		// Unclose PP_Order's Qty
		ppOrderBL.uncloseQtyOrdered(ppOrder);
		ppOrdersRepo.save(ppOrder);

		//
		// Unclose PP_Order BOM Line's quantities
		final List<I_PP_Order_BOMLine> lines = Services.get(IPPOrderBOMDAO.class).retrieveOrderBOMLines(ppOrder);
		for (final I_PP_Order_BOMLine line : lines)
		{
			ppOrderBOMBL.unclose(line);
		}

		// firing this before having updated the docstatus. This is how the *real* DocActions like MInvoice do it too.
		ModelValidationEngine.get().fireDocValidate(ppOrder, ModelValidator.TIMING_AFTER_UNCLOSE);

		//
		// Update DocStatus
		ppOrder.setDocStatus(IDocument.STATUS_Completed);
		ppOrder.setDocAction(IDocument.ACTION_Close);
		ppOrdersRepo.save(ppOrder);

		//
		// Reverse ALL cost collectors
		final PPOrderId ppOrderId = PPOrderId.ofRepoId(ppOrder.getPP_Order_ID());
		reverseAllCostCollectors(ppOrderId);
	}

	private final void reverseAllCostCollectors(final PPOrderId ppOrderId)
	{
		final List<I_PP_Cost_Collector> costCollectors = ppCostCollectorDAO.getByOrderId(ppOrderId);

		// Sort the cost collectors in reverse order of their creation,
		// just to make sure we are reversing the effect from last one to first one.
		Collections.sort(costCollectors, ModelByIdComparator.getInstance().reversed());

		for (final I_PP_Cost_Collector cc : costCollectors)
		{
			if (docActionBL.isDocumentReversedOrVoided(cc))
			{
				continue;
			}

			// Reversing activity controls is not supported atm, so we are skipping them.
			final CostCollectorType costCollectorType = CostCollectorType.ofCode(cc.getCostCollectorType());
			if (costCollectorType.isActivityControl())
			{
				continue;
			}

			final DocStatus costCollectorDocStatus = DocStatus.ofNullableCodeOrUnknown(cc.getDocStatus());
			if (costCollectorDocStatus.isClosed())
			{
				cc.setDocStatus(DocStatus.Completed.getCode());
				Services.get(IPPCostCollectorDAO.class).save(cc);
			}

			docActionBL.processEx(cc, IDocument.ACTION_Reverse_Correct, DocStatus.Reversed.getCode());
		}
	}
}
