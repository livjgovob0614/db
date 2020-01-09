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
package it.eng.spagobi.analiticalmodel.execution.bo.defaultvalues;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.log4j.Logger;

import it.eng.spagobi.analiticalmodel.execution.bo.LovValue;

public class DefaultValuesList extends ArrayList<LovValue> {

	private static Logger logger = Logger.getLogger(DefaultValuesList.class);

	/**
	 * Returns true if the default values' list contains the value specified in input. The input is compared with the default values' value property, i.e. it is
	 * not compared with default values' description properties.
	 */
	@Override
	public boolean contains(Object value) {
		Iterator<LovValue> it = this.iterator();
		while (it.hasNext()) {
			LovValue defaultValue = it.next();
			if (defaultValue.toString().equalsIgnoreCase("LovValue [value=, description=]") || defaultValue.getValue().equals(value)) {
				logger.debug("Value [" + value + "] is a default value");
				return true;
			}
		}
		return false;
	}

	/**
	 * Returns the default value object with value property equal to the value specified in input, or null if this object is not found
	 *
	 * @param value The value to look for: it is compared with default values' value property
	 * @return The LovValue object with value specified in input, or null if this object is not found
	 */
	public LovValue getDefaultValue(Object value) {
		Iterator<LovValue> it = this.iterator();
		while (it.hasNext()) {
			LovValue defaultValue = it.next();
			if (defaultValue.getValue().equals(value)) {
				logger.debug("Value [" + value + "] found in this default values' list");
				return defaultValue;
			}
		}
		logger.debug("Value [" + value + "] not found in this default values' list");
		return null;
	}

	public List getValuesAsList() {
		List toReturn = new ArrayList();
		Iterator<LovValue> it = this.iterator();
		while (it.hasNext()) {
			LovValue defaultValue = it.next();
			toReturn.add(defaultValue.getValue().toString());
		}
		return toReturn;
	}

	public List getDescriptionsAsList() {
		List toReturn = new ArrayList();
		Iterator<LovValue> it = this.iterator();
		while (it.hasNext()) {
			LovValue defaultValue = it.next();
			toReturn.add(defaultValue.getDescription().toString());
		}
		return toReturn;
	}

}
