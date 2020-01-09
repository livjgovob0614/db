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
package it.eng.spagobi.commons.utilities;

import java.io.File;

import javax.naming.Context;

import com.mysql.jdbc.jdbc2.optional.MysqlConnectionPoolDataSource;

import it.eng.spagobi.commons.dao.DAOConfig;
import it.eng.spagobi.utilities.MockContext;
import it.eng.spagobi.utilities.MockFactory;

public class UtilitiesDAOForTest {

	public static void setUpDatabaseTestJNDI() throws Exception {
		DAOConfig.setHibernateConfigurationFileFile(new File("../knowage/src/hibernate.cfg.xml"));

		try {
			Class.forName("org.hsqldb.jdbcDriver");
		} catch (Exception e) {
			System.err.println("ERROR: failed to load HSQLDB JDBC driver.");
			e.printStackTrace();
			return;
		}

		// Create initial context
		System.setProperty(Context.INITIAL_CONTEXT_FACTORY, MockFactory.class.getName());

		/* Construct DataSources*/MysqlConnectionPoolDataSource knowageDs = new MysqlConnectionPoolDataSource();
		knowageDs.setURL("jdbc:mysql://localhost/knowage_master");
		knowageDs.setUser("root");
		knowageDs.setPassword("123456");

		MysqlConnectionPoolDataSource foodmartDs = new MysqlConnectionPoolDataSource();
		foodmartDs.setURL("jdbc:mysql://localhost/foodmart");
		foodmartDs.setUser("root");
		foodmartDs.setPassword("123456");

		Context ic = new MockContext();
		MockFactory.context = ic;
		ic.bind("java:/comp/env/jdbc/knowage", knowageDs);
		ic.bind("java:comp/env/jdbc/knowage", knowageDs);
		ic.bind("java:comp/env/jdbc/foodmart", foodmartDs);
	}
}
