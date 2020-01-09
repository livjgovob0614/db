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
package it.eng.spagobi.tools.dataset.common.dataproxy;

import it.eng.spagobi.tools.dataset.common.datareader.IDataReader;

import java.util.Map;

public abstract class AbstractDataProxy implements IDataProxy {

	Map parameters;
	Map profile;
	int offset;
	int fetchSize;
	int maxResults;
	boolean calculateResultNumberOnLoad;
	String statement;
	String resPath;

	@Override
	public String getResPath() {
		return resPath;
	}

	@Override
	public void setResPath(String resPath) {
		this.resPath = resPath;
	}

	@Override
	public Map getParameters() {
		return parameters;
	}

	@Override
	public void setParameters(Map parameters) {
		this.parameters = parameters;
	}

	@Override
	public Map getProfile() {
		return profile;
	}

	@Override
	public void setProfile(Map profile) {
		this.profile = profile;
	}

	@Override
	public boolean isPaginationSupported() {
		return isOffsetSupported() && isMaxResultsSupported();
	}

	@Override
	public boolean isOffsetSupported() {
		return false;
	}

	@Override
	public int getOffset() {
		return offset;
	}

	@Override
	public void setOffset(int offset) {
		this.offset = offset;
	}

	@Override
	public boolean isFetchSizeSupported() {
		return false;
	}

	@Override
	public int getFetchSize() {
		return fetchSize;
	}

	@Override
	public void setFetchSize(int fetchSize) {
		this.fetchSize = fetchSize;
	}

	@Override
	public boolean isMaxResultsSupported() {
		return true;
	}

	@Override
	public int getMaxResults() {
		return maxResults;
	}

	@Override
	public void setMaxResults(int maxResults) {
		this.maxResults = maxResults;
	}

	@Override
	public boolean isCalculateResultNumberOnLoadEnabled() {
		return calculateResultNumberOnLoad;
	}

	@Override
	public void setCalculateResultNumberOnLoad(boolean enabled) {
		calculateResultNumberOnLoad = enabled;
	}

	@Override
	public long getResultNumber() {
		return -1;
	}

	@Override
	public String getStatement() {
		return statement;
	}

	@Override
	public void setStatement(String statement) {
		this.statement = statement;
	}

	@Override
	public Object getData(IDataReader dataReader, Object... resources) {
		throw new UnsupportedOperationException("This operation has to be overriden by subclasses in order to be used.");
	}

}
