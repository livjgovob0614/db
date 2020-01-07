package org.adempiere.archive.spi;

/*
 * #%L
 * de.metas.adempiere.adempiere.base
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


import java.io.InputStream;
import java.util.Properties;

import org.compiere.model.I_AD_Archive;

/**
 * Archive Storage (e.g. database, filesystem etc)
 * 
 * @author tsa
 * 
 */
public interface IArchiveStorage
{
	/**
	 * Initialize storage handler.
	 * 
	 * NOTE: don't call it directly, it's called by API
	 * 
	 * @param ctx
	 * @param adClientId
	 */
	void init(Properties ctx, int adClientId);

	I_AD_Archive newArchive(final Properties ctx, final String trxName);

	byte[] getBinaryData(I_AD_Archive archive);

	/**
	 * Get Data as Input Stream
	 * 
	 * @param archive
	 * @return input stream or null
	 */
	InputStream getBinaryDataAsStream(I_AD_Archive archive);

	void setBinaryData(I_AD_Archive archive, byte[] data);
}
