/**
 *
 */
package de.metas.handlingunits.receiptschedule;

import java.awt.image.BufferedImage;
import java.math.BigDecimal;

/*
 * #%L
 * de.metas.handlingunits.base
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

import java.util.Collection;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import javax.annotation.Nullable;

import org.adempiere.warehouse.LocatorId;

import de.metas.handlingunits.HuId;
import de.metas.handlingunits.IHUContext;
import de.metas.handlingunits.allocation.IAllocationRequest;
import de.metas.handlingunits.allocation.IAllocationSource;
import de.metas.handlingunits.attribute.HUAttributeConstants;
import de.metas.handlingunits.impl.IDocumentLUTUConfigurationManager;
import de.metas.handlingunits.model.I_M_ReceiptSchedule;
import de.metas.handlingunits.model.I_M_ReceiptSchedule_Alloc;
import de.metas.handlingunits.storage.IProductStorage;
import de.metas.inoutcandidate.api.IReceiptScheduleBL;
import de.metas.inoutcandidate.api.InOutGenerateResult;
import de.metas.util.ISingletonService;
import de.metas.util.Services;
import lombok.Builder;
import lombok.NonNull;
import lombok.Value;

/**
 * @author cg
 */
public interface IHUReceiptScheduleBL extends ISingletonService
{
	/**
	 * @param receiptSchedule
	 * @return amount of TUs which were planned to be received (i.e. amount of TUs ordered) or <code>null</code> in case there is no order line
	 */
	BigDecimal getQtyOrderedTUOrNull(I_M_ReceiptSchedule receiptSchedule);

	/**
	 * Same as {@link #getQtyOrderedTUOrNull(I_M_ReceiptSchedule)} but it will return zero instead of <code>null</code>.
	 */
	BigDecimal getQtyOrderedTUOrZero(I_M_ReceiptSchedule receiptSchedule);

	/**
	 *
	 * @param receiptSchedule
	 * @return qty ordered minus qty moved (TU)
	 */
	BigDecimal getQtyToMoveTU(I_M_ReceiptSchedule receiptSchedule);

	default BigDecimal getQtyToMoveCU(I_M_ReceiptSchedule receiptSchedule)
	{
		return Services.get(IReceiptScheduleBL.class).getQtyMoved(receiptSchedule);
	}

	// IInOutProducer createInOutProducerFromReceiptScheduleHU(CreateReceiptsParameters parameters);

	@Value
	@Builder
	public class CreateReceiptsParameters
	{
		@NonNull
		Properties ctx;

		/**
		 * <code>null</code> or a set of M_HU_IDs that shall be considered. If called with <code>null</code>, then all (planned?) HUs from the
		 * {@link de.metas.handlingunits.model.I_M_ReceiptSchedule_Alloc} will be assigned to the inOut.
		 */
		@Nullable
		Set<HuId> selectedHuIds;

		/**
		 * if <code>false</code> (the default), then a new InOut is created with the current date from {@link org.compiere.util.Env#getDate(Properties)}. Otherwise it is created with
		 * the DatePromised value of the receipt schedule's C_Order. To be used e.g. when doing migration work.
		 */
		boolean createReceiptWithDatePromised;

		/**
		 * If this is {@code true}, and if more than one receipt is created, then successfully created receipts won't be rolled back if other receipts fail.
		 * Intended to be {@code true} in unsupervised batch mode (where the log will be studied later!) and {@code false} in user-interactive mode (where there are not a lot of receipts created in one go).
		 */
		boolean commitEachReceiptIndividually;

		boolean printReceiptLabels;

		List<? extends de.metas.inoutcandidate.model.I_M_ReceiptSchedule> receiptSchedules;

		/**
		 * If the receipt was created and the good were not automatically moved to the quarantine warehouse,
		 * then the system can create a movement or distribution order (depends on product-planning master data) to this warehouse-locator.
		 *
		 * If {@code null}, then the respective receipt schedules', {@link I_M_ReceiptSchedule#getM_Warehouse_Dest_ID()}s' default locators will be used.
		 */
		@Nullable
		LocatorId destinationLocatorIdOrNull;
	}

	/**
	 * @return receipt generation result with receipts collected
	 */
	InOutGenerateResult processReceiptSchedules(CreateReceiptsParameters parameters);

	/**
	 * Mark LU and TU handling units of the allocations as destroyed, and unassign them, if the allocation does not already reference a receipt, if they are active and if they have the status
	 * "Planning".
	 *
	 * Also, the receipt schedule allocations of the destroyed HUs will be deactivated and saved.
	 *
	 * @param allocations
	 * @param trxName
	 */
	void destroyHandlingUnits(List<I_M_ReceiptSchedule_Alloc> allocations, String trxName);

	IProductStorage createProductStorage(de.metas.inoutcandidate.model.I_M_ReceiptSchedule rs);

	IAllocationSource createAllocationSource(I_M_ReceiptSchedule receiptSchedule);

	IDocumentLUTUConfigurationManager createLUTUConfigurationManager(I_M_ReceiptSchedule receiptSchedule);

	IDocumentLUTUConfigurationManager createLUTUConfigurationManager(List<I_M_ReceiptSchedule> receiptSchedules);

	/**
	 * Destroy the handling units from allocations in a huContext
	 *
	 * @param huContext
	 * @param allocs
	 */
	void destroyHandlingUnits(IHUContext huContext, List<I_M_ReceiptSchedule_Alloc> allocs);

	/**
	 * Set request's initial attribute values defaults to be used when new HUs are created.
	 *
	 * Mainly this method is setting the {@link HUAttributeConstants#ATTR_CostPrice}.
	 *
	 * @param request request to be updated
	 * @param receiptSchedules receipt schedule from where to extract the inital attributes
	 * @return updated request (could be the same, but it's not guaranteed)
	 */
	IAllocationRequest setInitialAttributeValueDefaults(IAllocationRequest request, Collection<? extends de.metas.inoutcandidate.model.I_M_ReceiptSchedule> receiptSchedules);

	/**
	 * @see #setInitialAttributeValueDefaults(IAllocationRequest, List)
	 */
	IAllocationRequest setInitialAttributeValueDefaults(IAllocationRequest request, de.metas.inoutcandidate.model.I_M_ReceiptSchedule receiptSchedule);

	void attachPhoto(I_M_ReceiptSchedule receiptSchedule, String filename, BufferedImage image);
}
