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
package it.eng.spagobi.tools.dataset.dao;

import java.util.List;

import org.hibernate.Session;

import it.eng.spago.error.EMFUserError;
import it.eng.spago.security.IEngUserProfile;
import it.eng.spagobi.commons.dao.ISpagoBIDao;
import it.eng.spagobi.tools.dataset.metadata.SbiDataSet;

public interface ISbiDataSetDAO extends ISpagoBIDao {

	public SbiDataSet loadSbiDataSetByLabel(String label);

	public List<SbiDataSet> loadSbiDataSets();

	public List<SbiDataSet> loadDataSets(String owner, Boolean includeOwned, Boolean includePublic, String scope, String type, String category,
			String implementation, Boolean showDerivedDatasets);

	public List<SbiDataSet> loadPaginatedSearchSbiDataSet(String search, Integer page, Integer item_per_page, IEngUserProfile finalUserProfile,
			Boolean seeTechnical, Integer[] ids, boolean spatialOnly);

	public Integer countSbiDataSet(String search) throws EMFUserError;

	public Integer countSbiDataSet(String search, Integer[] ids) throws EMFUserError;

	public SbiDataSet loadSbiDataSetByIdAndOrganiz(Integer id, String organiz);

	public SbiDataSet loadSbiDataSetByIdAndOrganiz(Integer id, String organiz, Session session);

}
