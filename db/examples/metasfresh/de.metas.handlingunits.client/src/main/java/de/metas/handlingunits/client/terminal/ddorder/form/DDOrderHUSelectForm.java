package de.metas.handlingunits.client.terminal.ddorder.form;

import org.adempiere.warehouse.WarehouseId;

/*
 * #%L
 * de.metas.handlingunits.client
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 * #L%
 */


import org.compiere.apps.form.FormFrame;
import org.eevolution.model.I_PP_Order;

import de.metas.handlingunits.client.terminal.ddorder.model.DDOrderHUSelectModel;
import de.metas.handlingunits.client.terminal.ddorder.view.DDOrderHUSelectFrame;
import de.metas.handlingunits.client.terminal.form.AbstractHUSelectForm;

public class DDOrderHUSelectForm extends AbstractHUSelectForm<DDOrderHUSelectModel>
{
	@Override
	protected DDOrderHUSelectFrame createFramePanel(final FormFrame frame)
	{
		final DDOrderHUSelectFrame framePanel = new DDOrderHUSelectFrame(frame, frame.getWindowNo());
		return framePanel;
	}

	/**
	 * Used when manually opening the DD order panel for a particular order.
	 *
	 * @param pp_Order
	 */
	public void setContextManufacturingOrder(final I_PP_Order pp_Order)
	{
		final DDOrderHUSelectFrame framePanel = (DDOrderHUSelectFrame)getFramePanel();
		framePanel.setContextManufacturingOrder(pp_Order);
	}

	public void setWarehouseOverrideId(final WarehouseId warehouseOverrideId)
	{
		final DDOrderHUSelectFrame framePanel = (DDOrderHUSelectFrame)getFramePanel();
		framePanel.setWarehouseOverrideId(warehouseOverrideId);
	}
}
