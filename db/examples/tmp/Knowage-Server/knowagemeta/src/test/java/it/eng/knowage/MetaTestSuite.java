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
package it.eng.knowage;

import it.eng.knowage.initializer.ModelInitializationTestSuite;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

/**
 * @author Antonella Giachino (antonella.giachino@eng.it)
 * 
 */
public class MetaTestSuite extends TestCase {
	static public Test suite() {
		TestSuite suite = new TestSuite("One test suite to bring them all");
		suite.addTest(ModelInitializationTestSuite.suite());
		// suite.addTest(ModelEditingTestSuite.suite());
		// suite.addTest(ModelSerializationTestSuite.suite());
		// suite.addTest(JpaMappingGenerationTestSuite.suite());
		// suite.addTest(ModelQueryTestSuite.suite());

		return suite;
	}
}