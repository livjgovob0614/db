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
package it.eng.knowage.engine.cockpit;

import it.eng.spagobi.utilities.engines.SpagoBIEngineRuntimeException;

/**
 * The Class CockpitEngineRuntimeException.
 */
public class CockpitEngineRuntimeException extends SpagoBIEngineRuntimeException {

	private static final long serialVersionUID = 1L;

	/**
	 * The hints. List hints;
	 */

	CockpitEngineInstance engineInstance;

	/**
	 * Builds a <code>CockpitEngineRuntimeException</code>.
	 * 
	 * @param message
	 *            Text of the exception
	 */
	public CockpitEngineRuntimeException(String message) {
		super(message);
	}

	/**
	 * Builds a <code>CockpitEngineRuntimeException</code>.
	 * 
	 * @param message
	 *            Text of the exception
	 * @param ex
	 *            previous Exception object
	 */
	public CockpitEngineRuntimeException(String message, Exception ex) {
		super(message, ex);
	}

	@Override
	public CockpitEngineInstance getEngineInstance() {
		return engineInstance;
	}

	public void setEngineInstance(CockpitEngineInstance engineInstance) {
		this.engineInstance = engineInstance;
	}
}
