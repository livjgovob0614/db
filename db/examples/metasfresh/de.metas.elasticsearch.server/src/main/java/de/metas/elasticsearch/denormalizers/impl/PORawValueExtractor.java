package de.metas.elasticsearch.denormalizers.impl;

import org.adempiere.model.InterfaceWrapperHelper;
import org.compiere.model.PO;

import lombok.ToString;

/*
 * #%L
 * de.metas.elasticsearch.server
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 * #L%
 */

@ToString
final class PORawValueExtractor implements IESModelValueExtractor
{
	public static final transient PORawValueExtractor instance = new PORawValueExtractor();

	private PORawValueExtractor()
	{
	}

	@Override
	public Object extractValue(final Object model, final String columnName)
	{
		final PO po = InterfaceWrapperHelper.getPO(model);
		final Object value = po.get_Value(columnName);
		return value;
	}
}