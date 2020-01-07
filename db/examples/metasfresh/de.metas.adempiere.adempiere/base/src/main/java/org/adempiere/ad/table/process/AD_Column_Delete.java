package org.adempiere.ad.table.process;

import org.adempiere.ad.trx.api.ITrx;
import org.adempiere.ad.window.api.IADWindowDAO;
import org.adempiere.model.InterfaceWrapperHelper;
import org.compiere.model.I_AD_Column;
import org.compiere.util.DB;

import de.metas.process.JavaProcess;
import de.metas.process.Param;
import de.metas.util.Services;

/*
 * #%L
 * de.metas.adempiere.adempiere.base
 * %%
 * Copyright (C) 2017 metas GmbH
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

/**
 * Deletes the selected AD_Column.
 * If asked, it can also delete the AD_Field(s) and the database column.
 *
 * @author metas-dev <dev@metasfresh.com>
 *
 */
public class AD_Column_Delete extends JavaProcess
{
	// services
	private final IADWindowDAO adWindowsRepo = Services.get(IADWindowDAO.class);

	@Param(parameterName = "IsDropDBColumn")
	private boolean p_IsDropDBColumn;
	@Param(parameterName = "IsDeleteFields")
	private boolean p_IsDeleteFields;

	@Override
	protected String doIt() throws Exception
	{
		final I_AD_Column adColumn = getRecord(I_AD_Column.class);

		if (p_IsDeleteFields)
		{
			final int adColumnId = adColumn.getAD_Column_ID();
			adWindowsRepo.deleteFieldsByColumnId(adColumnId);
		}

		if (p_IsDropDBColumn)
		{
			dropDBColumn(adColumn);
		}

		InterfaceWrapperHelper.delete(adColumn);
		addLog("AD_Column Deleted {}", adColumn);

		return MSG_OK;
	}

	private void dropDBColumn(final I_AD_Column adColumn)
	{
		final String tableName = adColumn.getAD_Table().getTableName();
		final String columnName = adColumn.getColumnName();

		executeDDL("ALTER TABLE " + tableName + " DROP COLUMN IF EXISTS " + columnName);
	}

	private final void executeDDL(final String sql)
	{
		DB.executeUpdateEx(sql, ITrx.TRXNAME_ThreadInherited);
		addLog("DDL: " + sql);
	}
}
