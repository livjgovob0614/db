package org.eevolution.api;

import java.time.LocalDateTime;

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

import java.util.List;
import java.util.Set;
import java.util.stream.Stream;

import org.adempiere.warehouse.WarehouseId;
import org.compiere.model.I_M_Warehouse;
import org.eevolution.model.I_PP_Order;

import de.metas.material.planning.pporder.PPOrderId;
import de.metas.order.OrderLineId;
import de.metas.product.ResourceId;
import de.metas.util.ISingletonService;

public interface IPPOrderDAO extends ISingletonService
{
	I_PP_Order getById(PPOrderId ppOrderId);

	<T extends I_PP_Order> T getById(PPOrderId ppOrderId, Class<T> type);

	List<I_PP_Order> getByIds(Set<PPOrderId> orderIds);

	/**
	 * Gets released manufacturing orders based on {@link I_M_Warehouse}s.
	 * 
	 * @param warehouseId
	 * @return manufacturing orders
	 */
	List<I_PP_Order> retrieveReleasedManufacturingOrdersForWarehouse(WarehouseId warehouseId);

	/**
	 * @param orderLineId
	 * @return PP_Order_ID or -1 if not found.
	 */
	PPOrderId retrievePPOrderIdByOrderLineId(final OrderLineId orderLineId);

	void changeOrderScheduling(PPOrderId orderId, LocalDateTime scheduledStartDate, LocalDateTime scheduledFinishDate);

	Stream<I_PP_Order> streamOpenPPOrderIdsOrderedByDatePromised(ResourceId plantId);

	void save(I_PP_Order order);
}
