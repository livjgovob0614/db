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
package it.eng.qbe;

import java.util.List;
import java.util.Set;

import org.apache.log4j.Logger;

import it.eng.qbe.datasource.IDataSource;
import it.eng.qbe.model.structure.IModelEntity;
import it.eng.qbe.model.structure.IModelStructure;
import it.eng.spagobi.tools.datasource.bo.DataSourceFactory;
import junit.framework.TestCase;

/**
 * @author Andrea Gioia (andrea.gioia@eng.it)
 *
 */
public abstract class AbstractQbeTestCase extends TestCase {

	protected it.eng.spagobi.tools.datasource.bo.IDataSource connection;
	protected IDataSource dataSource;
	protected ClassLoader classLoader;

	public static String CONNECTION_DIALECT = "org.hibernate.dialect.MySQLDialect";
	public static String CONNECTION_DRIVER = "com.mysql.jdbc.Driver";
	public static String CONNECTION_URL = "jdbc:mysql://localhost:3306/foodmart_key";
	public static String CONNECTION_USER = "root";
	public static String CONNECTION_PWD = "root";

	static protected Logger logger = Logger.getLogger(AbstractQbeTestCase.class);

	@Override
	protected void setUp() throws Exception {
		super.setUp();

		classLoader = Thread.currentThread().getContextClassLoader();

		connection = DataSourceFactory.getDataSource();
		connection.setLabel("foodmart");
		connection.setHibDialectClass(CONNECTION_DIALECT);
		connection.setDriver(CONNECTION_DRIVER);
		connection.setUrlConnection(CONNECTION_URL);
		connection.setUser(CONNECTION_USER);
		connection.setPwd(CONNECTION_PWD);

		setUpDataSource();
	}

	protected abstract void setUpDataSource();

	@Override
	protected void tearDown() throws Exception {
		super.tearDown();
		connection = null;
		tearDownDataSource();

		Thread.currentThread().setContextClassLoader(classLoader);
	}

	protected void tearDownDataSource() {
		dataSource.close();
		dataSource = null;
	}

	protected void dumpRootEntities(IModelStructure modelStructure) {
		Set<String> modelNames = modelStructure.getModelNames();
		for (String modelName : modelNames) {
			logger.debug(modelNames + " :");
			List<IModelEntity> rootEntities = modelStructure.getRootEntities(modelName);
			for (IModelEntity rootEntity : rootEntities) {
				logger.debug(" - " + rootEntity.getUniqueName());
			}
		}
	}
}
