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

import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.Set;

import org.apache.log4j.Logger;
import org.hibernate.Query;
import org.hibernate.Session;
import org.hibernate.Transaction;
import org.xml.sax.InputSource;

import it.eng.spago.base.SourceBean;
import it.eng.spago.init.InitializerIFace;
import it.eng.spagobi.commons.dao.AbstractHibernateDAO;
import it.eng.spagobi.commons.metadata.SbiAuthorizations;
import it.eng.spagobi.commons.metadata.SbiAuthorizationsRoles;
import it.eng.spagobi.commons.metadata.SbiDomains;
import it.eng.spagobi.commons.metadata.SbiOrganizationProductType;
import it.eng.spagobi.commons.metadata.SbiProductType;
import it.eng.spagobi.commons.metadata.SbiProductTypeEngine;
import it.eng.spagobi.commons.metadata.SbiTenant;
import it.eng.spagobi.commons.metadata.SbiUserFunctionality;
import it.eng.spagobi.engines.config.metadata.SbiEngines;
import it.eng.spagobi.utilities.exceptions.SpagoBIRuntimeException;

/**
 * @author Andrea Gioia (andrea.gioia@eng.it)
 *
 */
public abstract class SpagoBIInitializer extends AbstractHibernateDAO implements InitializerIFace {

	/**
	 * The name of the component affected by the initialization procedure
	 */
	protected String targetComponentName;
	protected String configurationFileName;

	static private Logger logger = Logger.getLogger(SpagoBIInitializer.class);

	protected SpagoBIInitializer() {
		targetComponentName = "SpagoBI";
	}

	public String getTargetComponentName() {
		return targetComponentName;
	}

	@Override
	public SourceBean getConfig() {
		return null;
	}

	@Override
	public void init(SourceBean config) {
		Session hibernateSession;
		Transaction hibernateTransaction;
		long startTime;
		long endTime;

		logger.debug("IN");

		hibernateSession = null;
		hibernateTransaction = null;

		try {

			startTime = System.currentTimeMillis();

			hibernateSession = this.getSession();
			hibernateTransaction = hibernateSession.beginTransaction();

			init(config, hibernateSession);

			hibernateTransaction.commit();

			endTime = System.currentTimeMillis();

			logger.debug("[" + targetComponentName + "] succesfully initialized in " + (endTime - startTime) + " ms");
		} catch (Throwable t) {
			logger.error("An unexpected error occured while initializing [" + targetComponentName + "]", t);
			if (hibernateTransaction != null) {
				hibernateTransaction.rollback();
				logger.debug("[" + targetComponentName + "] initialization succesfully rolled back");
			}
		} finally {
			if (hibernateSession != null && hibernateSession.isOpen()) {
				hibernateSession.close();
			}
			logger.debug("OUT");
		}
	}

	public abstract void init(SourceBean config, Session hibernateSession);

	protected SourceBean getConfiguration() throws Exception {
		logger.debug("IN");
		InputStream is = null;
		SourceBean toReturn = null;
		try {
			Thread curThread = Thread.currentThread();
			ClassLoader classLoad = curThread.getContextClassLoader();
			is = classLoad.getResourceAsStream(configurationFileName);
			InputSource source = new InputSource(is);
			toReturn = SourceBean.fromXMLStream(source);
			logger.debug("Configuration successfully read from resource " + configurationFileName);
		} catch (Exception e) {
			logger.error("Error while reading configuration from resource " + configurationFileName, e);
			throw e;
		} finally {
			if (is != null)
				try {
					is.close();
				} catch (IOException e) {
					logger.error(e);
				}
			logger.debug("OUT");
		}
		return toReturn;
	}

	protected SbiDomains findDomain(Session aSession, String valueCode, String domainCode) {
		logger.debug("IN");
		String hql = "from SbiDomains where valueCd = ? and domainCd = ?";
		Query hqlQuery = aSession.createQuery(hql);
		hqlQuery.setParameter(0, valueCode);
		hqlQuery.setParameter(1, domainCode);
		SbiDomains domain = (SbiDomains) hqlQuery.uniqueResult();
		logger.debug("OUT");
		return domain;
	}

	protected SbiEngines findEngine(Session aSession, String label) {
		logger.debug("IN");
		String hql = "from SbiEngines e where e.label = :label";
		Query hqlQuery = aSession.createQuery(hql);
		hqlQuery.setParameter("label", label);
		SbiEngines engine = (SbiEngines) hqlQuery.uniqueResult();
		logger.debug("OUT");
		return engine;
	}

	protected SbiProductType findProductType(Session aSession, String label) {
		logger.debug("IN");
		String hql = "from SbiProductType e where e.label = :label";
		Query hqlQuery = aSession.createQuery(hql);
		hqlQuery.setParameter("label", label);
		SbiProductType productType = (SbiProductType) hqlQuery.uniqueResult();
		logger.debug("OUT");
		return productType;
	}

	protected List<SbiProductType> getProductTypes(Session aSession) {
		logger.debug("IN");
		String hql = "from SbiProductType";
		Query hqlQuery = aSession.createQuery(hql);
		List<SbiProductType> productTypes = hqlQuery.list();
		logger.debug("OUT");
		return productTypes;
	}

	protected SbiTenant findTenant(Session aSession, String name) {
		logger.debug("IN");
		String hql = "from SbiTenant t where t.name = :name";
		Query hqlQuery = aSession.createQuery(hql);
		hqlQuery.setParameter("name", name);
		SbiTenant tenant = (SbiTenant) hqlQuery.uniqueResult();
		logger.debug("OUT");
		return tenant;
	}


	protected SbiProductTypeEngine findProductEngineType(Session aSession, String engine, String productType) {
		logger.debug("IN");
		String hql = "from SbiProductTypeEngine p where p.sbiEngines.label = :engine and p.sbiProductType.label = :productLabel";
		Query hibQuery = aSession.createQuery(hql);
		hibQuery.setString("engine", engine);
		hibQuery.setString("productLabel", productType);
		SbiProductTypeEngine result = (SbiProductTypeEngine) hibQuery.uniqueResult();
		logger.debug("OUT");
		return result;
	}

	protected SbiOrganizationProductType findOrganizationProductType(Session aSession, String tenant, String productType) {
		logger.debug("IN");
		String hql = "from SbiOrganizationProductType p where p.sbiOrganizations.name = :tenantName and p.sbiProductType.label = :productLabel";
		Query hibQuery = aSession.createQuery(hql);
		hibQuery.setString("tenantName", tenant);
		hibQuery.setString("productLabel", productType);
		SbiOrganizationProductType result = (SbiOrganizationProductType) hibQuery.uniqueResult();
		logger.debug("OUT");
		return result;
	}

	protected void deleteProductType(Session hibernateSession, SbiProductType productType) {
		logger.debug("IN");
		try {
			Integer productTypeId = productType.getProductTypeId();
			Set<SbiProductTypeEngine> sbiProductTypeEngine = productType.getSbiProductTypeEngine();
			for (SbiProductTypeEngine pte : sbiProductTypeEngine) {
				deleteProductTypeEngine(hibernateSession, pte);
			}

			Set<SbiAuthorizations> sbiAuthorization = productType.getSbiAuthorizations();
			for (SbiAuthorizations auth : sbiAuthorization) {
				deleteAuthorization(hibernateSession, auth);
			}

			Set<SbiUserFunctionality> sbiUserFunc = productType.getSbiUserFunctionality();
			for (SbiUserFunctionality uf : sbiUserFunc) {
				deleteUserFunctionality(hibernateSession, uf);
			}

			Set<SbiOrganizationProductType> sbiOrganizationProductType = productType.getSbiOrganizationProductType();
			for (SbiOrganizationProductType opt : sbiOrganizationProductType) {
				deleteOrganizationProductType(hibernateSession, opt);
			}

			hibernateSession.delete(productType);
		} catch (Throwable t) {
			throw new SpagoBIRuntimeException(
					"An error occured while deletion of Product Type " + productType.getProductTypeId() + " in synchronization with Configuraion file", t);
		} finally {
			logger.debug("OUT");
		}
	}

	protected void deleteAuthorization(Session hibernateSession, SbiAuthorizations sbiAuthorization) {
		logger.debug("IN");
		try {
			// Doesn't work check hibernate mapping to use this code
			// Set<SbiAuthorizationsRoles> sbiAuthorizationsRoles = sbiAuthorization.getSbiAuthorizationsRoleses();
			// for (SbiAuthorizationsRoles authRole : sbiAuthorizationsRoles) {
			// deleteAuthorizationsRoles(hibernateSession, authRole);
			// }

			Integer authorizationId = sbiAuthorization.getId();

			String hql = "delete from SbiAuthorizationsRoles a where a.id.authorizationId = :authorizationId";
			Query query = hibernateSession.createQuery(hql);
			query.setInteger("authorizationId", authorizationId);
			query.executeUpdate();

			hibernateSession.delete(sbiAuthorization);
		} catch (Throwable t) {
			throw new SpagoBIRuntimeException(
					"An error occured while deletion of Authorizations " + sbiAuthorization.getId() + " in synchronization with Configuraion file", t);
		} finally {
			logger.debug("OUT");
		}
	}

	protected void deleteProductTypeEngine(Session hibernateSession, SbiProductTypeEngine sbiProductTypeEngine) {
		logger.debug("IN");
		try {
			hibernateSession.delete(sbiProductTypeEngine);
		} catch (Throwable t) {
			throw new SpagoBIRuntimeException(
					"An error occured while deletion of Engine " + sbiProductTypeEngine.getId() + " in synchronization with Configuraion file", t);
		} finally {
			logger.debug("OUT");
		}
	}

	protected void deleteUserFunctionality(Session hibernateSession, SbiUserFunctionality sbiUserFunc) {
		logger.debug("IN");
		try {
			// automatically delete the SbiRoleTypeUserFunc related to the SbiUserFunctionality
			hibernateSession.delete(sbiUserFunc);
		} catch (Throwable t) {
			throw new SpagoBIRuntimeException(
					"An error occured while deletion of User Functionality " + sbiUserFunc.getId() + " in synchronization with Configuraion file", t);
		} finally {
			logger.debug("OUT");
		}
	}

	protected void deleteAuthorizationsRoles(Session hibernateSession, SbiAuthorizationsRoles sbiAuthorizationsRoles) {
		logger.debug("IN");
		try {
			hibernateSession.delete(sbiAuthorizationsRoles);
		} catch (Throwable t) {
			throw new SpagoBIRuntimeException(
					"An error occured while deletion of Authorizations Roles " + sbiAuthorizationsRoles.getId() + " in synchronization with Configuraion file",
					t);
		} finally {
			logger.debug("OUT");
		}
	}

	private void deleteOrganizationProductType(Session hibernateSession, SbiOrganizationProductType opt) {
		logger.debug("IN");
		try {
			hibernateSession.delete(opt);
		} catch (Throwable t) {
			throw new SpagoBIRuntimeException(
					"An error occured while deletion of Organization Product Type " + opt.getId() + " in synchronization with Configuraion file", t);
		} finally {
			logger.debug("OUT");
		}
	}
}
