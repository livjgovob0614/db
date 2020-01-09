/*
 * Knowage, Open Source Business Intelligence suite
 * Copyright (C) 2016 Engineering Ingegneria Informatica S.p.A.
 * 
 * Knowage is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Knowage is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package it.eng.spagobi.commons.initializers.caching;

import it.eng.spago.base.SourceBean;
import it.eng.spago.init.InitializerIFace;
import it.eng.spagobi.commons.SingletonConfig;
import it.eng.spagobi.tools.dataset.cache.CacheFactory;
import it.eng.spagobi.tools.dataset.cache.ICache;
import it.eng.spagobi.tools.dataset.cache.ICacheConfiguration;
import it.eng.spagobi.tools.dataset.cache.SpagoBICacheConfiguration;
import it.eng.spagobi.tools.dataset.persist.PersistedTableManager;
import it.eng.spagobi.tools.datasource.bo.IDataSource;
import org.apache.log4j.Logger;

public class CachingInitializer implements InitializerIFace {

	static private Logger logger = Logger.getLogger(CachingInitializer.class);
	private SourceBean _config;

	@Override
	public SourceBean getConfig() {
		return _config;
	}

	@Override
	public void init(SourceBean config) {
		logger.debug("IN");
		_config = config;

		ICacheConfiguration cacheConfiguration = null;
		try {
			cacheConfiguration = SpagoBICacheConfiguration.getInstance();
		ICache cache = CacheFactory.getCache(cacheConfiguration);
		cache.deleteAll();

		IDataSource dataSource = cacheConfiguration.getCacheDataSource();
		String prefix = SingletonConfig.getInstance().getConfigValue("SPAGOBI.CACHE.NAMEPREFIX");
		if (prefix != null && !prefix.isEmpty() && dataSource != null) {
			PersistedTableManager persistedTableManager = new PersistedTableManager();
			persistedTableManager.dropTablesWithPrefix(dataSource, prefix);
		}
		} catch (Exception e) {
			logger.error("Cannot initialize cache", e);
		} finally {
		logger.debug("OUT");
		}

	}

}
