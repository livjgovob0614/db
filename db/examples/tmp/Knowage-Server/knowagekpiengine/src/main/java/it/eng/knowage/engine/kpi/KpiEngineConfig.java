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
package it.eng.knowage.engine.kpi;

import it.eng.knowage.engine.kpi.model.conf.FullKpiConfig;
import it.eng.spago.base.SourceBean;
import it.eng.spago.configuration.ConfigSingleton;
import it.eng.spagobi.services.common.EnginConf;

import java.io.File;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.log4j.Logger;

/**
 * @author
 */
@SuppressWarnings("unchecked")
public class KpiEngineConfig {

	public static final String CACHE_NAME_PREFIX_CONFIG = "SPAGOBI.CACHE.NAMEPREFIX";
	public static final String CACHE_SPACE_AVAILABLE_CONFIG = "SPAGOBI.CACHE.SPACE_AVAILABLE";
	public static final String CACHE_LIMIT_FOR_CLEAN_CONFIG = "SPAGOBI.CACHE.LIMIT_FOR_CLEAN";

	private static Map<String, FullKpiConfig> chartLibConf = new HashMap<>();

	private static EnginConf engineConfig;

	private static transient Logger logger = Logger.getLogger(KpiEngineConfig.class);

	// -- singleton pattern --------------------------------------------
	private static KpiEngineConfig instance;

	public static KpiEngineConfig getInstance() {
		return instance;
	}

	static {
		logger.trace("IN");

		engineConfig = EnginConf.getInstance();

		instance = new KpiEngineConfig();

		SourceBean chartLibraries = (SourceBean) getConfigSourceBean().getAttribute("chartConfiguration");
		if (chartLibraries != null) {
			List<SourceBean> chartLibrariesItems = chartLibraries.getAttributeAsList("chart");
			for (SourceBean chart : chartLibrariesItems) {
				String type = (String) chart.getAttribute("type");
				String name = (String) chart.getAttribute("name");
				String vmPath = (String) chart.getAttribute("vmPath");
				String vmName = (String) chart.getAttribute("vmName");
				String libIniPath = (String) chart.getAttribute("libIniPath");
				String libIniName = (String) chart.getAttribute("libIniName");
				String enabledInCockpit = (String) chart.getAttribute("enabledInCockpit");
				chartLibConf.put(type, new FullKpiConfig(type, name, vmPath, vmName, libIniPath, libIniName, enabledInCockpit));
			}
		}
		logger.trace("OUT");
	}

	private KpiEngineConfig() {

	}

	// -- singleton pattern --------------------------------------------

	// -- ACCESSOR Methods -----------------------------------------------
	public static EnginConf getEngineConfig() {
		return engineConfig;
	}

	public static SourceBean getConfigSourceBean() {
		return getEngineConfig().getConfig();
	}

	public static Map<String, FullKpiConfig> getChartLibConf() {
		return chartLibConf;
	}

	public static String getEngineResourcePath() {
		String path = null;
		if (getEngineConfig().getResourcePath() != null) {
			path = getEngineConfig().getResourcePath() + File.separatorChar + "chart";
		} else {
			path = ConfigSingleton.getRootPath() + File.separatorChar + "resources" + File.separatorChar + "chart";
		}

		return path;
	}
}
