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
package it.eng.spagobi.metadata.dao;

import it.eng.spago.error.EMFUserError;
import it.eng.spagobi.commons.dao.ISpagoBIDao;
import it.eng.spagobi.metadata.metadata.SbiMetaBcAttribute;

import java.util.List;

import org.hibernate.Session;

/**
 * @author Antonella Giachino (antonella.giachino@eng.it)
 *
 */
public interface ISbiMetaBCAttributeDAO extends ISpagoBIDao {

	public SbiMetaBcAttribute loadBcAttributeByID(Integer id) throws EMFUserError;

	public SbiMetaBcAttribute loadBcAttributeByName(String name) throws EMFUserError;

	public void modifyBcAttribute(SbiMetaBcAttribute aMetaBCAttribute) throws EMFUserError;

	public Integer insertBcAttribute(SbiMetaBcAttribute aMetaBCAttribute) throws EMFUserError;

	public void deleteBcAttribute(SbiMetaBcAttribute aMetaBCAttribute) throws EMFUserError;

	public List<SbiMetaBcAttribute> loadAllBCAttributes() throws EMFUserError;

	public List<SbiMetaBcAttribute> loadAllBCAttributeFromTableColumn(int tableColumnId) throws EMFUserError;

	public List<SbiMetaBcAttribute> loadAllBCAttributeFromBC(int bcid) throws EMFUserError;

	// TRANSACTIONAL METHODS (the session is an input parameter):

	public SbiMetaBcAttribute loadBcAttributeByID(Session session, Integer id) throws EMFUserError;

	public SbiMetaBcAttribute loadBcAttributeByNameAndBc(Session session, String name, Integer bcId) throws EMFUserError;

	public void modifyBcAttribute(Session session, SbiMetaBcAttribute aMetaBCAttribute) throws EMFUserError;

	public Integer insertBcAttribute(Session session, SbiMetaBcAttribute aMetaBCAttribute) throws EMFUserError;

}
