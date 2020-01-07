package de.metas.dpd.model;

/*
 * #%L
 * de.metas.swat.base
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


import java.sql.ResultSet;
import java.util.Properties;

import org.adempiere.util.proxy.Cached;
import org.compiere.model.Query;

import de.metas.cache.annotation.CacheIgnore;

/**
 *
 * @author ts
 * @see "<a href='http://dewiki908/mediawiki/index.php/Transportverpackung_%282009_0022_G61%29'>(2009_0022_G61)</a>"
 */
public class MDPDDepot extends X_DPD_Depot
{

	/**
	 *
	 */
	private static final long serialVersionUID = 7350231553001692690L;

	public MDPDDepot(Properties ctx, int DPD_Depot_ID, String trxName)
	{
		super(ctx, DPD_Depot_ID, trxName);
	}

	public MDPDDepot(Properties ctx, ResultSet rs, String trxName)
	{
		super(ctx, rs, trxName);
	}

	@Cached
	public static MDPDDepot retrieve(final Properties ctx,
			final String depotNumber, final int depotFileInfoId,
			@CacheIgnore final String trxName)
	{

		final String whereClause = COLUMNNAME_GeoPostDepotNumber + "=? AND "
				+ COLUMNNAME_DPD_FileInfo_ID + "=?";

		final Object[] parameters = new Object[] { depotNumber, depotFileInfoId };

		return new Query(ctx, Table_Name, whereClause, trxName).setParameters(
				parameters).setClient_ID().setOnlyActiveRecords(true)
				.firstOnly();
	}

	@Override
	public String toString()
	{

		final StringBuffer sb = new StringBuffer();
		sb.append("MDPDDepot[Id=");
		sb.append(get_ID());
		sb.append(", GeoPostDepotNumber=");
		sb.append(getGeoPostDepotNumber());
		sb.append(", CountryCode=");
		sb.append(getCountryCode());
		sb.append(", Name=");
		sb.append(getName());
		sb.append("]");

		return sb.toString();
	}
}
