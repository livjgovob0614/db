package de.metas.material.planning.pporder;

import java.util.List;

import org.compiere.model.I_M_Product;
import org.eevolution.model.I_PP_Order;
import org.eevolution.model.I_PP_Order_BOM;
import org.eevolution.model.I_PP_Order_BOMLine;

import de.metas.util.ISingletonService;

public interface IPPOrderBOMDAO extends ISingletonService
{
	I_PP_Order_BOMLine getOrderBOMLineById(PPOrderBOMLineId orderBOMLineId);

	/**
	 * Retrieve (active) BOM Lines for a specific PP Order
	 * 
	 * @param order
	 * @return
	 */
	List<I_PP_Order_BOMLine> retrieveOrderBOMLines(I_PP_Order order);

	/**
	 * Retrieve (active) BOM Lines for a specific PP Order
	 * 
	 * @param order
	 * @param orderBOMLineClass
	 * @return
	 */
	<T extends I_PP_Order_BOMLine> List<T> retrieveOrderBOMLines(I_PP_Order order, Class<T> orderBOMLineClass);

	List<I_PP_Order_BOMLine> retrieveOrderBOMLines(PPOrderId orderId);

	<T extends I_PP_Order_BOMLine> List<T> retrieveOrderBOMLines(PPOrderId orderId, Class<T> orderBOMLineClass);

	I_PP_Order_BOM getByOrderIdOrNull(PPOrderId orderId);

	int retrieveNextLineNo(PPOrderId orderId);

	I_PP_Order_BOMLine retrieveOrderBOMLine(I_PP_Order ppOrder, I_M_Product product);

	void save(I_PP_Order_BOM orderBOM);

	void save(I_PP_Order_BOMLine orderBOMLine);

	void deleteByOrderId(PPOrderId orderId);

	void deleteOrderBOMLinesByOrderId(PPOrderId orderId);

	void markBOMLinesAsProcessed(PPOrderId orderId);
	
	void markBOMLinesAsNotProcessed(PPOrderId orderId);
}
