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
package it.eng.spagobi.analiticalmodel.document.service;

import it.eng.spagobi.analiticalmodel.document.dao.IBIObjectDAO;
import it.eng.spagobi.commons.dao.DAOFactory;
import it.eng.spagobi.commons.serializer.SerializerFactory;
import it.eng.spagobi.commons.services.AbstractSpagoBIAction;
import it.eng.spagobi.utilities.exceptions.SpagoBIServiceException;
import it.eng.spagobi.utilities.service.JSONSuccess;
import org.apache.log4j.Logger;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.List;
import java.util.Locale;

public class ManageBIObjects extends AbstractSpagoBIAction {

    // logger component
    private static Logger logger = Logger.getLogger(ManageBIObjects.class);
    private final String MESSAGE_DET = "MESSAGE_DET";
    // type of service
    private final String BIOBJECTS_LIST = "BIOBJECTS_LIST";
    private final String BIOBJECT_INSERT = "BIOBJECT_INSERT";
    private final String BIOBJECT_DELETE = "BIOBJECT_DELETE";

    public static String START = "start";
    public static String LIMIT = "limit";
    public static Integer START_DEFAULT = 0;
    public static Integer LIMIT_DEFAULT = 16;

    @Override
    public void doService() {
        logger.debug("IN");
        IBIObjectDAO boDao = DAOFactory.getBIObjectDAO();
        boDao.setUserProfile(getUserProfile());

        Locale locale = getLocale();

        String serviceType = this.getAttributeAsString(MESSAGE_DET);
        logger.debug("Service type " + serviceType);
        if (serviceType != null && serviceType.equalsIgnoreCase(BIOBJECTS_LIST)) {

            try {

                Integer start = getAttributeAsInteger(START);
                Integer limit = getAttributeAsInteger(LIMIT);

                if (start == null) {
                    start = START_DEFAULT;
                }
                if (limit == null) {
                    limit = LIMIT_DEFAULT;
                }

                Integer totalItemsNum = boDao.countBIObjects(null, null);
                List items = boDao.loadPagedObjectsList(start, limit);
                logger.debug("Loaded items list");
                JSONArray itemsJSON = (JSONArray) SerializerFactory.getSerializer("application/json").serialize(items, locale);
                JSONObject responseJSON = createJSONResponse(itemsJSON, totalItemsNum);

                writeBackToClient(new JSONSuccess(responseJSON));

            } catch (Throwable e) {
                logger.error("Exception occurred while retrieving items", e);
                throw new SpagoBIServiceException(SERVICE_NAME,
                        "Exception occurred while retrieving items", e);
            }
        } else if (serviceType != null && serviceType.equalsIgnoreCase(BIOBJECT_INSERT)) {
            //TODO
        } else if (serviceType != null && serviceType.equalsIgnoreCase(BIOBJECT_DELETE)) {
            //TODO
        } else if (serviceType == null) {
            //TODO
        }
        logger.debug("OUT");
    }

    /**
     * Creates a json array with children users informations
     *
     * @param rows
     * @return
     * @throws JSONException
     */
    private JSONObject createJSONResponse(JSONArray rows, Integer totalResNumber)
            throws JSONException {
        JSONObject results;

        results = new JSONObject();
        results.put("total", totalResNumber);
        results.put("title", "BIObjects");
        results.put("rows", rows);
        return results;
    }
}