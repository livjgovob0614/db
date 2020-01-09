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
import it.eng.spagobi.metadata.metadata.SbiMetaJob;
import it.eng.spagobi.metadata.metadata.SbiMetaJobTable;
import it.eng.spagobi.metadata.metadata.SbiMetaTable;

import java.util.List;

import org.hibernate.Session;

/**
 * @author Antonella Giachino (antonella.giachino@eng.it)
 *
 */
public interface ISbiJobTableDAO extends ISpagoBIDao {

	public List<SbiMetaJob> loadJobsByTableId(Integer tableId) throws EMFUserError;

	public List<SbiMetaTable> loadTablesByJobId(Integer jobId) throws EMFUserError;

	public SbiMetaJobTable loadJobTable(Session session, Integer jobId, Integer tableId) throws EMFUserError;

	public void modifyJobTable(SbiMetaJobTable aMetaJobTable) throws EMFUserError;

	public void insertJobTable(SbiMetaJobTable aMetaJobTable) throws EMFUserError;

	public void insertJobTable(Session session, SbiMetaJobTable aMetaJobTable) throws EMFUserError;

	public void deleteJobTable(SbiMetaJobTable aMetaJobTable) throws EMFUserError;

}
