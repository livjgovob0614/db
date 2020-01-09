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
package it.eng.spagobi.services.event.service;

import java.util.List;

import org.apache.log4j.Logger;

import com.jamonapi.Monitor;
import com.jamonapi.MonitorFactory;

import it.eng.spagobi.commons.bo.UserProfile;
import it.eng.spagobi.commons.utilities.UserUtilities;
import it.eng.spagobi.engines.drivers.handlers.IRolesHandler;
import it.eng.spagobi.events.EventsManager;
import it.eng.spagobi.events.bo.EventType;
import it.eng.spagobi.services.common.AbstractServiceImpl;
import it.eng.spagobi.services.security.exceptions.SecurityException;

public class EventServiceImpl extends AbstractServiceImpl {

	static private Logger logger = Logger.getLogger(EventServiceImpl.class);

	/**
	 * Fire event.
	 *
	 * @param token
	 *            the token
	 * @param user
	 *            the user
	 * @param description
	 *            the description
	 * @param parameters
	 *            the parameters
	 * @param rolesHandler
	 *            the roles handler
	 * @param type
	 *            the type
	 *
	 * @return the string
	 */
	public String fireEvent(String token, String user, String description, String parameters, String rolesHandler, String type) {
		logger.debug("IN");
		Monitor monitor = MonitorFactory.start("spagobi.service.event.fireEvent");
		try {
			validateTicket(token, user);
			this.setTenantByUserId(user);
			UserProfile profile = (UserProfile) UserUtilities.getUserProfile(user);
			return fireEvent(profile, description, parameters, rolesHandler, type);
		} catch (SecurityException e) {
			logger.error("SecurityException", e);
			return null;
		} catch (Exception e) {
			logger.error("Error while firing event", e);
			return null;
		} finally {
			this.unsetTenant();
			monitor.stop();
			logger.debug("OUT");
		}

	}

	private String fireEvent(UserProfile profile, String description, String parameters, String rolesHandler, String type) {
		logger.debug("IN");
		String returnValue = null;
		try {
			if (profile != null) {
				IRolesHandler rolesHandlerClass = (IRolesHandler) Class.forName(rolesHandler).newInstance();
				List roles = rolesHandlerClass.calculateRoles(parameters);
				Integer id = EventsManager.getInstance().registerEvent(profile.getUserId().toString(), description, parameters, roles, EventType.valueOf(type));
				returnValue = id.toString();
				logger.debug("Service executed succesfully");
			} else {
				logger.warn("User is NULL");
			}
			return returnValue;
		} catch (Exception e) {
			logger.error("Impossible to fire event [" + description + "]", e);
		} finally {
			logger.debug("OUT");
		}
		return null;
	}

}
