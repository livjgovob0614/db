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
package it.eng.spagobi.adhocreporting.services;

import it.eng.spago.security.IEngUserProfile;
import it.eng.spagobi.commons.dao.DAOFactory;
import it.eng.spagobi.commons.services.AbstractSpagoBIAction;
import it.eng.spagobi.tools.datasource.bo.IDataSource;
import it.eng.spagobi.tools.datasource.dao.IDataSourceDAO;
import it.eng.spagobi.utilities.exceptions.SpagoBIServiceException;
import it.eng.spagobi.utilities.service.JSONSuccess;
import it.eng.spagobi.utilities.sql.SqlUtils;
import org.apache.log4j.Logger;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.util.Iterator;
import java.util.List;

/**
 * @author Zerbetto Davide (davide.zerbetto@eng.it)
 * <p>
 * This action is intended for final users; it doesn't use standard serializer for datasources, since no sensitive information (URL, JDBC user and pwd)
 * should be sent to final users. It filters HBase or Hive datasource, since they cannot be used, at the moment, as datasources for dataset persistence.
 */
public class GetDatasourcesListUserAction extends AbstractSpagoBIAction {

    public static final String SERVICE_NAME = "GET_DATASOURCES_LIST_ACTION";

    public static final String ID = "id";
    public static final String LABEL = "label";
    public static final String DESCRIPTION = "description";

    // logger component
    private static Logger logger = Logger.getLogger(GetDatasourcesListUserAction.class);

    @Override
    public void doService() {
        logger.debug("IN");
        try {

            IDataSourceDAO dao = DAOFactory.getDataSourceDAO();
            IEngUserProfile profile = getUserProfile();

            dao.setUserProfile(profile);

            List<IDataSource> items = null;
            try {
                items = dao.loadAllDataSources();
            } catch (Exception e) {
                throw new SpagoBIServiceException("Error while getting datasources' list", e);
            }

            JSONObject responseJSON = null;
            try {
                JSONArray itemsJSON = new JSONArray();
                Iterator<IDataSource> it = items.iterator();
                while (it.hasNext()) {
                    IDataSource datasource = it.next();
                    String dialect = datasource.getHibDialectClass();
                    // HBase and Hive cannot be selected in order to persist a dataset, therefore we exclude them.
                    // TODO When implementing dataset persistence on those system, remove this filter.
                    if (dialect.contains("hbase") && SqlUtils.isHiveLikeDialect(dialect)) {
                        continue;
                    }
                    JSONObject obj = new JSONObject();
                    obj.put(ID, datasource.getDsId());
                    obj.put(LABEL, datasource.getLabel());
                    obj.put(DESCRIPTION, datasource.getDescr());
                    itemsJSON.put(obj);
                }
                responseJSON = createJSONResponse(itemsJSON, itemsJSON.length());
            } catch (Exception e) {
                throw new SpagoBIServiceException("Error while serializing data", e);
            }

            try {
                writeBackToClient(new JSONSuccess(responseJSON));
            } catch (IOException e) {
                throw new SpagoBIServiceException("Impossible to write back the responce to the client", e);
            }

        } finally {
            logger.debug("OUT");
        }
    }

    protected JSONObject createJSONResponse(JSONArray rows, Integer totalResNumber) throws JSONException {
        JSONObject results;
        results = new JSONObject();
        results.put("total", totalResNumber);
        results.put("title", "Datasources");
        results.put("rows", rows);
        return results;
    }

}
