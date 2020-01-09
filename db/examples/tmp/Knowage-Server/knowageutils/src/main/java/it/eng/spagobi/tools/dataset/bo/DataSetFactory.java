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
package it.eng.spagobi.tools.dataset.bo;

import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Constructor;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import javax.servlet.http.HttpSession;

import org.apache.log4j.Logger;

import it.eng.spagobi.commons.constants.SpagoBIConstants;
import it.eng.spagobi.services.common.EnginConf;
import it.eng.spagobi.services.common.EnginConf.MissingClassException;
import it.eng.spagobi.services.dataset.bo.SpagoBiDataSet;
import it.eng.spagobi.services.proxy.MetamodelServiceProxy;
import it.eng.spagobi.tools.datasource.bo.DataSourceFactory;
import it.eng.spagobi.tools.datasource.bo.IDataSource;
import it.eng.spagobi.utilities.assertion.Assert;
import it.eng.spagobi.utilities.exceptions.SpagoBIRuntimeException;
import it.eng.spagobi.utilities.sql.SqlUtils;

/**
 * @author Andrea Gioia (andrea.gioia@eng.it)
 */
public class DataSetFactory {

	private static transient Logger logger = Logger.getLogger(DataSetFactory.class);

	public static IDataSet getDataSet(SpagoBiDataSet dataSetConfig, String userId) {
		return getDataSet(dataSetConfig, userId, null);
	}

	/**
	 * This method returns a dataset according to his configuration
	 *
	 * @param dataSetConfig
	 *            dataset configuration
	 * @param userId
	 *            used in QBE dataset
	 * @param session
	 *            sd in QBE dataset
	 * @return a dataset correctly configured
	 */
	public static IDataSet getDataSet(SpagoBiDataSet dataSetConfig, String userId, HttpSession session) {
		IDataSet dataSet = null;

		if (dataSetConfig == null) {
			throw new IllegalArgumentException("dataset-config parameter cannot be null");
		}

		InputStream source = DataSetFactory.class.getResourceAsStream("/datasetTypes.properties");
		Properties p = new Properties();
		try {
			p.load(source);
		} catch (IOException e) {
			throw new SpagoBIRuntimeException("Cannot load configuration from datasetTypes.properties file", e);
		}
		String dsType = dataSetConfig.getType();
		String className = p.getProperty(dsType);

		logger.debug("Dataset type: " + dsType);
		if (className.equals(JDBCDataSet.class.getName())) {
			try {
				IDataSource ds = DataSourceFactory.getDataSource(dataSetConfig.getDataSource());
				String dialectToLowerCase = ds.getHibDialectClass().toLowerCase();
				if (SqlUtils.isHiveLikeDialect(dialectToLowerCase)) {
					className = JDBCHiveDataSet.class.getName();
				} else if (dialectToLowerCase.contains("mongo")) {
					className = MongoDataSet.class.getName();
				}

			} catch (Exception e) {
				throw new RuntimeException("Missing right exstension", e);
			}
		}

		logger.debug("Dataset class: " + className);
		if (className == null) {
			throw new SpagoBIRuntimeException("No dataset class found for dataset type [" + dsType + "]");
		}
		Constructor c = null;
		Object object = null;
		if (className.endsWith("JDBCDataSet")) {
			String dialectToLowerCase = dataSetConfig.getDataSource().getHibDialectClass().toLowerCase();
			if (SqlUtils.isHiveLikeDialect(dialectToLowerCase)) {
				className = JDBCHiveDataSet.class.getName();
			} else if (dialectToLowerCase.contains("orient")) {
				className = JDBCOrientDbDataSet.class.getName();
			} else if (dialectToLowerCase.contains("vertica")) {
				className = JDBCVerticaDataSet.class.getName();
			}
		}
		try {

			if (className.toLowerCase().contains("federated")) {
				c = Class.forName(className).getConstructor(SpagoBiDataSet.class, HttpSession.class);
				object = c.newInstance(dataSetConfig, session);
			} else {
				c = Class.forName(className).getConstructor(SpagoBiDataSet.class);
				object = c.newInstance(dataSetConfig);
			}

			dataSet = (IDataSet) object;

			logger.debug("Check if the dataset type is a JDBC dataset");
			if (className.equals(JDBCDataSet.class.getName())) {
				dataSet.setParameters(dataSetConfig.getParameters());
			}

			logger.debug("Check if the dataset type is a Qbe dataset");

			if (className.endsWith("FederatedDataSet")) {
				Map parameters = dataSet.getParamsMap();
				if (parameters == null) {
					parameters = new HashMap();
					dataSet.setParamsMap(parameters);
				}
				dataSet.getParamsMap().put(SpagoBIConstants.USER_ID, userId);
			} else if (className.endsWith("QbeDataSet")) {

				logger.debug("The current dataset is a Qbe dataset. Looking for correct datamart retriever");
				String datamartRetrieverClass = p.getProperty("SbiQbeDatamartRetriever");

				logger.debug("Datamart retriver class name is: [" + datamartRetrieverClass + "]");

				// we need to differentiate the datamart retriever:
				// - when we are in SpagoBICore
				// - when we are in a SpagoBIEngine
				try {

					// config-engine.xml is present, the we are in a SpagoBIEngine
					logger.debug("Test if we are in the SpagoBI Core or in a SpagoBI Engine");
					EnginConf engineConfTest = EnginConf.getInstanceCheckingMissingClass(true);

					logger.debug("No missing class exception thrown. We are in a SpagoBI Engine");

					Assert.assertNotNull(session, "Impossible to find a valid HTTP Session");

					MetamodelServiceProxy proxy = new MetamodelServiceProxy(userId, session);
					Constructor cDatamartRetriver = Class.forName(datamartRetrieverClass).getConstructor(MetamodelServiceProxy.class);
					Map parameters = dataSet.getParamsMap();
					if (parameters == null) {
						parameters = new HashMap();
						dataSet.setParamsMap(parameters);
					}
					dataSet.getParamsMap().put(SpagoBIConstants.DATAMART_RETRIEVER, cDatamartRetriver.newInstance(proxy));
					logger.debug("Datamart retriever correctly added to Qbe dataset");

				} catch (MissingClassException e) {
					// if this exception is thrown, then we are in SpagoBICore
					logger.debug("Missing class exception thrown. We are in the SpagoBI Core");
					Constructor cDatamartRetriver = Class.forName(datamartRetrieverClass).getConstructor();
					Map parameters = dataSet.getParamsMap();
					if (parameters == null) {
						parameters = new HashMap();
						dataSet.setParamsMap(parameters);
					}
					dataSet.getParamsMap().put(SpagoBIConstants.DATAMART_RETRIEVER, cDatamartRetriver.newInstance());
					logger.debug("Datamart retriever correctly added to Qbe dataset");
				}

			}
		} catch (Exception e) {
			throw new SpagoBIRuntimeException("Error while instantiating dataset type [" + dsType + "], class [" + className + "]", e);
		}

		// if custom data set type try instantiate the referred class
		IDataSet customDataset = dataSet;
		if (CustomDataSet.DS_TYPE.equals((dataSetConfig.getType())) && customDataset instanceof CustomDataSet) {
			try {
				dataSet = ((CustomDataSet) customDataset).instantiate();
			} catch (Exception e) {
				logger.error("Cannot instantiate class " + ((CustomDataSet) customDataset).getJavaClassName() + ": go on with CustomDatasetClass");
			}
		}

		// if ( ScriptDataSet.DS_TYPE.equals( dataSetConfig.getType() ) ) {
		// dataSet = new ScriptDataSet( dataSetConfig );
		// } else if ( JDBCDataSet.DS_TYPE.equals( dataSetConfig.getType() ) ) {
		// dataSet = new JDBCDataSet( dataSetConfig );
		// } else if ( JavaClassDataSet.DS_TYPE.equals( dataSetConfig.getType()
		// ) ) {
		// dataSet = new JavaClassDataSet( dataSetConfig );
		// } else if ( WebServiceDataSet.DS_TYPE.equals( dataSetConfig.getType()
		// ) ) {
		// dataSet = new WebServiceDataSet( dataSetConfig );
		// } else if ( FileDataSet.DS_TYPE.equals( dataSetConfig.getType() ) ) {
		// dataSet = new FileDataSet( dataSetConfig );
		// } else {
		// logger.error("Invalid dataset type [" + dataSetConfig.getType() +
		// "]");
		// throw new
		// IllegalArgumentException("dataset type in dataset-config cannot be equal to ["
		// + dataSetConfig.getType() + "]");
		// }

		// retrieve persisting and scheduling informations
		dataSet.setPersisted(dataSetConfig.isPersisted());
		dataSet.setPersistedHDFS(dataSetConfig.isPersistedHDFS());
		dataSet.setScheduled(dataSetConfig.isScheduled());
		dataSet.setPersistTableName(dataSetConfig.getPersistTableName());
		dataSet.setStartDateField(dataSetConfig.getStartDateField());
		dataSet.setEndDateField(dataSetConfig.getEndDateField());
		dataSet.setSchedulingCronLine(dataSetConfig.getSchedulingCronLine());

		return dataSet;
	}
}
