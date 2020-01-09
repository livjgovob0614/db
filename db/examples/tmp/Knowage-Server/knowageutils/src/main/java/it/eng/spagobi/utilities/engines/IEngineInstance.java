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
package it.eng.spagobi.utilities.engines;

import it.eng.spagobi.engines.drivers.IEngineDriver;

import java.util.Map;

/**
 * @author Andrea Gioia (andrea.gioia@eng.it)
 * 
 */
public interface IEngineInstance {
	public static final String COUNTRY = IEngineDriver.COUNTRY;
	public static final String LANGUAGE = IEngineDriver.LANGUAGE;

	String getId();

	Map getEnv();

	void setEnv(Map env);

	void validate() throws SpagoBIEngineException;

	EngineAnalysisMetadata getAnalysisMetadata();

	void setAnalysisMetadata(EngineAnalysisMetadata analysisMetadata);

	IEngineAnalysisState getAnalysisState();

	void setAnalysisState(IEngineAnalysisState analysisState);
}
