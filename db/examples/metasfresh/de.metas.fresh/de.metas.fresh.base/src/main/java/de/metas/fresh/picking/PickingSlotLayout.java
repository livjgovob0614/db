/**
 * 
 */
package de.metas.fresh.picking;

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

import java.awt.Color;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import com.google.common.collect.ImmutableList;

import de.metas.adempiere.form.terminal.ITerminalKey;
import de.metas.adempiere.form.terminal.KeyLayout;
import de.metas.adempiere.form.terminal.context.ITerminalContext;
import de.metas.fresh.picking.form.SwingPackingTerminalPanel;

/**
 * @author cg
 * 
 */
public class PickingSlotLayout extends KeyLayout
{
	private static final Color DEFAULT_Color = Color.RED;

	public PickingSlotLayout(ITerminalContext tc)
	{
		super(tc);
		setColumns(3);
		setRows(4);
		setDefaultColor(DEFAULT_Color);
	}

	public int getSlotNo()
	{
		final List<ITerminalKey> keys = getKeysOrNull();
		if (keys == null || keys.isEmpty())
		{
			return 0;
		}
		return keys.size();
	}

	@Override
	public String getId()
	{
		return "PickingSlot#" + 101;
	}

	@Override
	protected List<ITerminalKey> createKeys()
	{
		final ImmutableList<PickingSlotKey> keys = getBasePanel().getAvailablePickingSlots();
		return ImmutableList.copyOf(keys);
	}

	public List<PickingSlotKey> getPickingSlotKeys()
	{
		final List<ITerminalKey> keys = getKeys();
		final List<PickingSlotKey> pickingSlotKeys = new ArrayList<>(keys.size());
		for (final ITerminalKey key : keys)
		{
			final PickingSlotKey pks = (PickingSlotKey)key;
			pickingSlotKeys.add(pks);
		}

		return pickingSlotKeys;
	}

	public List<PickingSlotKey> getPickingSlotKeys(final PickingSlotKeyGroup group)
	{
		final List<ITerminalKey> keys = getKeys();
		final List<PickingSlotKey> pickingSlotKeys = new ArrayList<>();
		for (final ITerminalKey key : keys)
		{
			final PickingSlotKey pickingSlotKey = (PickingSlotKey)key;
			final PickingSlotKeyGroup pickingSlotKeyGroup = pickingSlotKey.getPickingSlotKeyGroup();
			if (!Objects.equals(pickingSlotKeyGroup, group))
			{
				continue;
			}

			pickingSlotKeys.add(pickingSlotKey);
		}

		return pickingSlotKeys;
	}

	@Override
	public SwingPackingTerminalPanel getBasePanel()
	{
		return SwingPackingTerminalPanel.cast(super.getBasePanel());
	}

	@Override
	public boolean isNumeric()
	{
		return false;
	}

	@Override
	public boolean isText()
	{
		return false;
	}
}
