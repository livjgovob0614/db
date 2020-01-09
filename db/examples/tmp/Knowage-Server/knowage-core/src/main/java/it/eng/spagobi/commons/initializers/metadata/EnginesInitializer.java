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
package it.eng.spagobi.commons.initializers.metadata;

import it.eng.spago.base.SourceBean;
import it.eng.spagobi.commons.metadata.SbiDomains;
import it.eng.spagobi.engines.config.metadata.SbiEngines;
import it.eng.spagobi.utilities.exceptions.SpagoBIRuntimeException;

import java.util.Iterator;
import java.util.List;

import org.apache.log4j.Logger;
import org.hibernate.Query;
import org.hibernate.Session;

/**
 * @author Andrea Gioia (andrea.gioia@eng.it)
 *
 */
public class EnginesInitializer extends SpagoBIInitializer {

	static private Logger logger = Logger.getLogger(EnginesInitializer.class);

	public EnginesInitializer() {
		targetComponentName = "Engines";
		configurationFileName = "it/eng/spagobi/commons/initializers/metadata/config/engines.xml";
	}

	/*
	 * public void init(SourceBean config, Session hibernateSession) { logger.debug("IN"); try { init(config, hibernateSession);
	 * 
	 * } catch(Throwable t){ logger.error("Impossible to init EnginesInitializer", t); }finally { logger.debug("OUT"); } }
	 */

	@Override
	public void init(SourceBean config, Session hibernateSession) {
		logger.debug("IN");
		try {
			String hql = "from SbiEngines";
			Query hqlQuery = hibernateSession.createQuery(hql);
			List engines = hqlQuery.list();
			if (engines.isEmpty()) {
				logger.info("No engines. Starting populating predefined engines...");
				writeEngines(hibernateSession);
			} else {
				logger.debug("Engines table is already populated, only engines will be populated");
				writeMissingEngines(hibernateSession);
			}
		} catch (Throwable t) {
			throw new SpagoBIRuntimeException("Ab unexpected error occured while initializeng Engines", t);
		} finally {
			logger.debug("OUT");
		}
	}

	private void writeMissingEngines(Session hibernateSession) throws Exception {
		logger.debug("IN");
		SourceBean enginesSB = getConfiguration();
		if (enginesSB == null) {
			logger.info("Configuration file for predefined engines not found");
			return;
		}
		List enginesList = enginesSB.getAttributeAsList("ENGINE");
		if (enginesList == null || enginesList.isEmpty()) {
			logger.info("No predefined engines available from configuration file");
			return;
		}
		Iterator it = enginesList.iterator();
		while (it.hasNext()) {
			SourceBean anEngineSB = (SourceBean) it.next();
			String label = (String) anEngineSB.getAttribute("label");
			if (label == null || label.equals("")) {
				logger.error("No predefined engine label found!!!");
				throw new Exception("No predefined engine label found!!!");
			}
			// Retrieving the engine in the DB with the specified label
			logger.debug("Retrieving the engine in the DB with the specified label");
			SbiEngines engine = findEngine(hibernateSession, label);

			if (engine == null) {
				SbiEngines anEngine = new SbiEngines();
				anEngine.setName((String) anEngineSB.getAttribute("name"));
				anEngine.setDescr((String) anEngineSB.getAttribute("descr"));
				anEngine.setMainUrl((String) anEngineSB.getAttribute("mainUrl"));
				anEngine.setDriverNm((String) anEngineSB.getAttribute("driverNm"));
				anEngine.setLabel((String) anEngineSB.getAttribute("label"));
				anEngine.setClassNm((String) anEngineSB.getAttribute("classNm"));
				anEngine.setUseDataSet(new Boolean((String) anEngineSB.getAttribute("useDataSet")));
				anEngine.setUseDataSource(new Boolean((String) anEngineSB.getAttribute("useDataSource")));
				anEngine.setEncrypt(new Short((String) anEngineSB.getAttribute("encrypt")));
				anEngine.setObjUplDir((String) anEngineSB.getAttribute("objUplDir"));
				anEngine.setObjUseDir((String) anEngineSB.getAttribute("objUseDir"));
				anEngine.setSecnUrl((String) anEngineSB.getAttribute("secnUrl"));

				String engineTypeCd = (String) anEngineSB.getAttribute("engineTypeCd");
				SbiDomains domainEngineType = findDomain(hibernateSession, engineTypeCd, "ENGINE_TYPE");
				anEngine.setEngineType(domainEngineType);

				String biobjTypeCd = (String) anEngineSB.getAttribute("biobjTypeCd");
				SbiDomains domainBiobjectType = findDomain(hibernateSession, biobjTypeCd, "BIOBJ_TYPE");
				anEngine.setBiobjType(domainBiobjectType);

				Integer engineId = (Integer) hibernateSession.save(anEngine);

				logger.debug("Inserting Engine with label = [" + anEngineSB.getAttribute("label") + "] ...");
			}
		}
		logger.debug("OUT");
	}

	private void writeEngines(Session aSession) throws Exception {
		logger.debug("IN");
		SourceBean enginesSB = getConfiguration();
		if (enginesSB == null) {
			logger.info("Configuration file for predefined engines not found");
			return;
		}
		List enginesList = enginesSB.getAttributeAsList("ENGINE");
		if (enginesList == null || enginesList.isEmpty()) {
			logger.info("No predefined engines available from configuration file");
			return;
		}
		Iterator it = enginesList.iterator();
		while (it.hasNext()) {
			SourceBean anEngineSB = (SourceBean) it.next();
			SbiEngines anEngine = new SbiEngines();
			anEngine.setName((String) anEngineSB.getAttribute("name"));
			anEngine.setDescr((String) anEngineSB.getAttribute("descr"));
			anEngine.setMainUrl((String) anEngineSB.getAttribute("mainUrl"));
			anEngine.setDriverNm((String) anEngineSB.getAttribute("driverNm"));
			anEngine.setLabel((String) anEngineSB.getAttribute("label"));
			anEngine.setClassNm((String) anEngineSB.getAttribute("classNm"));
			anEngine.setUseDataSet(new Boolean((String) anEngineSB.getAttribute("useDataSet")));
			anEngine.setUseDataSource(new Boolean((String) anEngineSB.getAttribute("useDataSource")));
			anEngine.setEncrypt(new Short((String) anEngineSB.getAttribute("encrypt")));
			anEngine.setObjUplDir((String) anEngineSB.getAttribute("objUplDir"));
			anEngine.setObjUseDir((String) anEngineSB.getAttribute("objUseDir"));
			anEngine.setSecnUrl((String) anEngineSB.getAttribute("secnUrl"));

			String engineTypeCd = (String) anEngineSB.getAttribute("engineTypeCd");
			SbiDomains domainEngineType = findDomain(aSession, engineTypeCd, "ENGINE_TYPE");
			anEngine.setEngineType(domainEngineType);

			String biobjTypeCd = (String) anEngineSB.getAttribute("biobjTypeCd");
			SbiDomains domainBiobjectType = findDomain(aSession, biobjTypeCd, "BIOBJ_TYPE");
			anEngine.setBiobjType(domainBiobjectType);

			Integer engineId = (Integer) aSession.save(anEngine);

			logger.debug("Inserting Engine with label = [" + anEngineSB.getAttribute("label") + "] ...");
		}
		logger.debug("OUT");
	}

	// private SbiTenant getDefaultTenant(Session hibernateSession) throws Exception {
	// logger.debug("IN");
	// SbiTenant aTenant = new SbiTenant();
	// String hql = "from SbiTenant t where t.name =?";
	// Query hqlQuery = hibernateSession.createQuery(hql);
	// hqlQuery.setString(0, "SPAGOBI");
	// SbiTenant defaultenant = (SbiTenant) hqlQuery.uniqueResult();
	// logger.debug("OUT");
	// return defaultenant;
	//
	// }
}
