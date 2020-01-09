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
package it.eng.spagobi.commons.services;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Properties;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpSession;

import org.apache.log4j.Logger;
import org.hibernate.HibernateException;
import org.hibernate.Session;

import it.eng.spago.base.Constants;
import it.eng.spago.base.RequestContainer;
import it.eng.spago.base.SessionContainer;
import it.eng.spago.base.SourceBean;
import it.eng.spago.dispatching.module.AbstractHttpModule;
import it.eng.spago.error.EMFErrorHandler;
import it.eng.spago.error.EMFErrorSeverity;
import it.eng.spago.error.EMFUserError;
import it.eng.spago.security.IEngUserProfile;
import it.eng.spagobi.analiticalmodel.document.bo.BIObject;
import it.eng.spagobi.commons.SingletonConfig;
import it.eng.spagobi.commons.bo.Config;
import it.eng.spagobi.commons.bo.Role;
import it.eng.spagobi.commons.bo.UserProfile;
import it.eng.spagobi.commons.constants.SpagoBIConstants;
import it.eng.spagobi.commons.dao.DAOFactory;
import it.eng.spagobi.commons.dao.IConfigDAO;
import it.eng.spagobi.commons.metadata.SbiExtRoles;
import it.eng.spagobi.commons.utilities.AuditLogUtilities;
import it.eng.spagobi.commons.utilities.GeneralUtilities;
import it.eng.spagobi.commons.utilities.HibernateSessionManager;
import it.eng.spagobi.commons.utilities.ObjectsAccessVerifier;
import it.eng.spagobi.commons.utilities.StringUtilities;
import it.eng.spagobi.commons.utilities.UserUtilities;
import it.eng.spagobi.commons.utilities.messages.MessageBuilder;
import it.eng.spagobi.profiling.bean.SbiUser;
import it.eng.spagobi.profiling.dao.ISbiUserDAO;
import it.eng.spagobi.services.common.SsoServiceFactory;
import it.eng.spagobi.services.common.SsoServiceInterface;
import it.eng.spagobi.services.security.bo.SpagoBIUserProfile;
import it.eng.spagobi.services.security.exceptions.SecurityException;
import it.eng.spagobi.services.security.service.ISecurityServiceSupplier;
import it.eng.spagobi.services.security.service.SecurityServiceSupplierFactory;
import it.eng.spagobi.tenant.Tenant;
import it.eng.spagobi.tenant.TenantManager;
import it.eng.spagobi.utilities.themes.ThemesManager;
import it.eng.spagobi.wapp.services.ChangeTheme;
import it.eng.spagobi.wapp.util.MenuUtilities;

public class LoginModule extends AbstractHttpModule {

	static Logger logger = Logger.getLogger(LoginModule.class);

	private static final String PROP_NODE = "changepwd.";
	public static final String LIST_MENU = "LIST_MENU";

	/** The format date to manage the data validation. */
	private static final String DATE_FORMAT = "yyyy-MM-dd";

	IEngUserProfile profile = null;
	EMFErrorHandler errorHandler = null;

	/**
	 * Service.
	 *
	 * @param request  the request
	 * @param response the response
	 *
	 * @throws Exception the exception
	 *
	 * @see it.eng.spago.dispatching.action.AbstractHttpAction#service(it.eng.spago.base.SourceBean, it.eng.spago.base.SourceBean)
	 */
	@Override
	public void service(SourceBean request, SourceBean response) throws Exception {
		logger.debug("IN");

		RequestContainer reqCont = RequestContainer.getRequestContainer();
		SessionContainer sessCont = reqCont.getSessionContainer();
		SessionContainer permSess = sessCont.getPermanentContainer();

		HttpServletRequest servletRequest = getHttpRequest();
		HttpSession httpSession = servletRequest.getSession();

		String currTheme = manageTheme(request, reqCont, permSess);

		manageLocale(permSess);

		boolean activeSoo = isSSOActive();

		String docLabel = (String) request.getAttribute(SpagoBIConstants.OBJECT_LABEL);
		boolean isPublicDoc = false;
		boolean isPublicUser = false;
		if (docLabel != null) {
			BIObject obj = DAOFactory.getBIObjectDAO().loadBIObjectByLabel(docLabel);
			isPublicDoc = ObjectsAccessVerifier.isObjectPublic(obj);
		}

		errorHandler = getErrorHandler();

		UserProfile previousProfile = (UserProfile) permSess.getAttribute(IEngUserProfile.ENG_USER_PROFILE);

		String userIdForDefaultProfile = null;

		String userId = null;
		if (!activeSoo) {
			if (isPublicDoc) {
				userId = SpagoBIConstants.PUBLIC_USER_ID;
				isPublicUser = true;
			} else {
				userId = (String) request.getAttribute("userID");
			}
			logger.debug("userID=" + userId);
			if (userId == null) {
				if (previousProfile != null) {
					profile = previousProfile;
					// user is authenticated, nothing to do
					logger.debug("User is authenticated");
					// fill response
					List lstMenu = MenuUtilities.getMenuItems(profile);
					String url = "/themes/" + currTheme + "/jsp/";
					if (UserUtilities.isTechnicalUser(profile)) {
						url += "adminHome.jsp";
					} else {
						url += "userHome.jsp";
					}
					servletRequest.getSession().setAttribute(LIST_MENU, lstMenu);
					getHttpRequest().getRequestDispatcher(url).forward(getHttpRequest(), getHttpResponse());
					// response.setAttribute(SpagoBIConstants.PUBLISHER_NAME, "userhome");
					return;
				} else {
					// user must authenticate
					logger.debug("User must authenticate");
					// set publisher name
					String url = "/themes/" + currTheme + "/jsp/login.jsp";
					getHttpRequest().setAttribute("start_url", url);
					getHttpRequest().getRequestDispatcher(url).forward(getHttpRequest(), getHttpResponse());
					// orig:
					// String url = GeneralUtilities.getSpagoBiHost() + servletRequest.getContextPath();
					// response.setAttribute("start_url", url);
					// response.setAttribute(SpagoBIConstants.PUBLISHER_NAME, "login");
					logger.debug("OUT");
					return;
				}
				// logger.error("User identifier not found. Cannot build user profile object");
				// throw new SecurityException("User identifier not found.");
			}
		} else {

			SsoServiceInterface userProxy = SsoServiceFactory.createProxyService();
			userId = userProxy.readUserIdentifier(servletRequest);
			logger.debug("OUT,userId:" + userId);
			// if we are in SSO and user has a previous profile keep it!
			if (previousProfile != null
			// && previousProfile.getUserUniqueIdentifier().equals(userId) REMOVE
			) {
				if (previousProfile != null) {
					profile = previousProfile;
					// user is authenticated, nothing to do
					logger.debug("User is authenticated");
					// fill response
					List lstMenu = MenuUtilities.getMenuItems(profile);
					// set publisher name
					String url = "/themes/" + currTheme + "/jsp/";
					if (UserUtilities.isTechnicalUser(profile)) {
						url += "adminHome.jsp";
					} else {
						url += "userHome.jsp";
					}
					servletRequest.getSession().setAttribute(LIST_MENU, lstMenu);
					getHttpRequest().getRequestDispatcher(url).forward(getHttpRequest(), getHttpResponse());

					return;
				}
			}

		}

		boolean isInternalSecurity = ("true".equalsIgnoreCase((String) request.getAttribute("isInternalSecurity"))) ? true : false;
		logger.debug("isInternalSecurity: " + isInternalSecurity);

		ISecurityServiceSupplier supplier = SecurityServiceSupplierFactory.createISecurityServiceSupplier();
		// If SSO is not active, check username and password, i.e. performs the authentication;
		// instead, if SSO is active, the authentication mechanism is provided by the SSO itself, so SpagoBI does not make
		// any authentication, just creates the user profile object and puts it into Spago permanent container
		if (!activeSoo && !isPublicUser) {
			String pwd = (String) request.getAttribute("password");
			try {

				SpagoBIUserProfile userProfile = null;
				userProfile = supplier.checkAuthentication(userId, pwd);
				if (userProfile == null) {
					logger.error("userName/pwd uncorrect");
					EMFUserError emfu = new EMFUserError(EMFErrorSeverity.ERROR, 501);
					errorHandler.addError(emfu);
					AuditLogUtilities.updateAudit(getHttpRequest(), profile, "SPAGOBI.Login", null, "KO");
					return;
				} else if (isInternalSecurity == true) {
					SbiUser user = DAOFactory.getSbiUserDAO().loadSbiUserByUserId(userId);
					if (user.getFlgPwdBlocked() != null && user.getFlgPwdBlocked()) {
						logger.error("userName/pwd uncorrect");
						EMFUserError emfu = new EMFUserError(EMFErrorSeverity.ERROR, 502);
						errorHandler.addError(emfu);
						AuditLogUtilities.updateAudit(getHttpRequest(), profile, "SPAGOBI.Login", null, "KO");
						return;
					}
				}

				userIdForDefaultProfile = userId;
				userId = userProfile.getUniqueIdentifier();

			} catch (Exception e) {
				logger.error("Reading user information... ERROR", e);
				throw new SecurityException("Reading user information... ERROR", e);
			}
			// if it's internal (SpagoBI) active pwd management and checks
			if (isInternalSecurity) {
				// gets the user bo
				ISbiUserDAO userDao = DAOFactory.getSbiUserDAO();
				SbiUser user = userDao.loadSbiUserByUserId(userId);

				// Checks if the input role is valid for SpagoBI.
				// Only if the configuration about this check returns true.
				String strAdminPatter = SingletonConfig.getInstance().getConfigValue("SPAGOBI.SECURITY.ROLE-TYPE-PATTERNS.ADMIN-PATTERN");
				int sbiUserId = -1;
				if (user != null)
					sbiUserId = user.getId();
				List lstRoles = userDao.loadSbiUserRolesById(sbiUserId);

				boolean isAdminUser = false;
				for (int i = 0; i < lstRoles.size(); i++) {
					SbiExtRoles tmpRole = (SbiExtRoles) lstRoles.get(i);
					Role role = DAOFactory.getRoleDAO().loadByID(tmpRole.getExtRoleId());
					if (role.getName().equals(strAdminPatter)) {
						isAdminUser = true;
						logger.debug("User is administrator. Checks on the password are not applied !");
						break;
					}
				}

				if (!isAdminUser) {
					// check validation of the password
					logger.debug("Validation password starting...");

					boolean goToChangePwd = checkPwd(user);
					if (goToChangePwd) {
						response.setAttribute("user_id", user.getUserId());
						String url = GeneralUtilities.getSpagoBiHost() + servletRequest.getContextPath();
						response.setAttribute("start_url", url);
						response.setAttribute(SpagoBIConstants.PUBLISHER_NAME, "ChangePwdPublisher");
						return;
					}

					logger.info("The pwd is active!");
					// update lastAccessDate on db with current date
					try {
						if (user != null) {
							user.setDtLastAccess(new Date());
							userDao.updateSbiUser(user, user.getId());
						}
					} catch (Exception e) {
						logger.error("Error while update user's dtLastAccess: " + e);
					}
				}
			}
		}

		try {
			httpSession = regenerateSession(servletRequest);
			// set default role
			SbiUser user = DAOFactory.getSbiUserDAO().loadSbiUserByUserId(userIdForDefaultProfile);
			Integer defaultRoleId = user.getDefaultRoleId();
			String defaultRole = null;
			if (defaultRoleId != null)
				defaultRole = DAOFactory.getRoleDAO().loadByID(defaultRoleId).getName();

			profile = UserUtilities.getUserProfile(userId, defaultRole);
			if (profile == null) {
				logger.error("user not created");
				EMFUserError emfu = new EMFUserError(EMFErrorSeverity.ERROR, 501);
				errorHandler.addError(emfu);
				AuditLogUtilities.updateAudit(getHttpRequest(), profile, "SPAGOBI.Login", null, "ERR");
				return;
			}

			// checks if the input role is valid with SpagoBI's role list
			boolean isRoleValid = true;
			String checkRoleLogin = SingletonConfig.getInstance().getConfigValue("SPAGOBI.SECURITY.CHECK_ROLE_LOGIN");
			if (("true").equals(checkRoleLogin)) {
				String roleToCheck = SingletonConfig.getInstance().getConfigValue("SPAGOBI.SECURITY.ROLE_LOGIN");
				String valueRoleToCheck = "";
				if (!("").equals(roleToCheck)) {
					valueRoleToCheck = (request.getAttribute(roleToCheck) != null) ? (String) request.getAttribute(roleToCheck) : "";
					if (!("").equals(valueRoleToCheck)) {
						Collection lstRoles = profile.getRoles();
						isRoleValid = false;
						Iterator iterRoles = lstRoles.iterator();
						while (iterRoles.hasNext()) {
							String iterRoleName = (String) iterRoles.next();
							if (iterRoleName.equals(valueRoleToCheck)) {
								isRoleValid = true;
								logger.debug("Role in input " + valueRoleToCheck + " is valid. ");
								break;
							}
						}
					} else {
						logger.debug("Role " + roleToCheck + " is not passed into the request. Check on the role is not applied. ");
					}
				}
				if (!isRoleValid) {
					logger.error("role uncorrect");
					EMFUserError emfu = new EMFUserError(EMFErrorSeverity.ERROR, 501);
					errorHandler.addError(emfu);
					AuditLogUtilities.updateAudit(getHttpRequest(), profile, "SPAGOBI.Login", null, "KO");
					// put user role into session
					httpSession.setAttribute(roleToCheck, valueRoleToCheck);
					response.setAttribute(roleToCheck, valueRoleToCheck);
					return;
				}
			}

			Boolean userHasChanged = Boolean.TRUE;
			// try to find if the user has changed: if so, the session parameters must be reset, see also homebis.jsp
			// check previous userId with current one: if they are equals, user has not changed
			if (previousProfile != null && previousProfile.getUserId().equals(((UserProfile) profile).getUserId())) {
				userHasChanged = Boolean.FALSE;
			}
			response.setAttribute("USER_HAS_CHANGED", userHasChanged);
			// put user profile into session
			permSess.setAttribute(IEngUserProfile.ENG_USER_PROFILE, profile);

		} catch (Exception e) {
			logger.error("Reading user information... ERROR");
			AuditLogUtilities.updateAudit(getHttpRequest(), profile, "SPAGOBI.Login", null, "ERR");
			throw new SecurityException("Reading user information... ERROR", e);
		}

		// String username = (String) profile.getUserUniqueIdentifier();
		String username = (String) ((UserProfile) profile).getUserId();

		// putting tenant id on thread local
		Tenant tenant = new Tenant(((UserProfile) profile).getOrganization());
		TenantManager.setTenant(tenant);

		try {

			// Start writing log in the DB
			Session aSession = null;
			try {
				aSession = HibernateSessionManager.getCurrentSession();
				AuditLogUtilities.updateAudit(getHttpRequest(), profile, "SPAGOBI.Login", null, "OK");
			} catch (HibernateException he) {
				throw new EMFUserError(EMFErrorSeverity.ERROR, 100);
			} finally {
				if (aSession != null) {
					if (aSession.isOpen())
						aSession.close();
				}
			}
			// End writing log in the DB

			List lstMenu = MenuUtilities.getMenuItems(profile);

			String url = "/themes/" + currTheme + "/jsp/";
			if (UserUtilities.isTechnicalUser(profile)) {
				url += "adminHome.jsp";
			} else {
				url += "userHome.jsp";
			}
			servletRequest.getSession().setAttribute(LIST_MENU, lstMenu);
			getHttpRequest().getRequestDispatcher(url).forward(getHttpRequest(), getHttpResponse());
		} finally {
			// since TenantManager uses a ThreadLocal, we must clean after request processed in each case
			TenantManager.unset();
		}

		logger.debug("OUT");
	}

	private boolean isSSOActive() {
		String activeStr = SingletonConfig.getInstance().getConfigValue("SPAGOBI_SSO.ACTIVE");
		boolean activeSoo = false;
		if (activeStr != null && activeStr.equalsIgnoreCase("true")) {
			activeSoo = true;
		}
		return activeSoo;
	}

	private void manageLocale(SessionContainer permSess) {
		// updates locale information on permanent container for Spago messages mechanism
		// search firstly if a default language is set on configuraiton file, else take browser from spago
		if (permSess.getAttribute(Constants.USER_LANGUAGE) == null || permSess.getAttribute(Constants.USER_COUNTRY) == null) {
			logger.debug("getting locale...");
			Locale locale = GeneralUtilities.getStartingDefaultLocale();
			if (locale == null) {
				locale = MessageBuilder.getBrowserLocaleFromSpago();
			} else {
				logger.debug("Locale " + locale.getLanguage() + " - " + locale.getCountry() + " taken as default from configuraiton file");
			}
			if (locale != null) {
				logger.debug("locale taken as default is " + locale.getLanguage() + " - " + locale.getCountry());
				permSess.setAttribute(Constants.USER_LANGUAGE, locale.getLanguage());
				permSess.setAttribute(Constants.USER_COUNTRY, locale.getCountry());
			}
		} else {
			logger.debug("locale already found in session");
		}
	}

	private String manageTheme(SourceBean request, RequestContainer reqCont, SessionContainer permSess) {
		String theme_name = (String) request.getAttribute(ChangeTheme.THEME_NAME);
		logger.debug("theme selected: " + theme_name);
		String currTheme = "";

		if (theme_name != null && theme_name.length() > 0) {
			permSess.setAttribute(SpagoBIConstants.THEME, theme_name);
			currTheme = theme_name;
		} else {
			currTheme = ThemesManager.getCurrentTheme(reqCont);
			if (currTheme == null)
				currTheme = ThemesManager.getDefaultTheme();
		}
		logger.debug("currTheme: " + currTheme);
		return currTheme;
	}

	private HttpSession regenerateSession(HttpServletRequest request) {

		HttpSession oldSession = request.getSession();

		Enumeration attrNames = oldSession.getAttributeNames();
		Properties props = new Properties();

		if (attrNames != null) {
			while (attrNames.hasMoreElements()) {
				String key = (String) attrNames.nextElement();
				props.put(key, oldSession.getAttribute(key));
			}
		}
		// Invalidating previous session
		oldSession.invalidate();
		// Generate new session
		HttpSession newSession = request.getSession(true);
		attrNames = props.keys();

		while (attrNames.hasMoreElements()) {
			String key = (String) attrNames.nextElement();
			newSession.setAttribute(key, props.get(key));
		}
		return newSession;
	}

	private boolean checkPwd(SbiUser user) throws Exception {
		logger.debug("IN");
		boolean toReturn = false;
		if (user == null)
			return toReturn;
		Date currentDate = new Date();

		// gets the active controls to applicate:
		IConfigDAO configDao = DAOFactory.getSbiConfigDAO();
		List lstConfigChecks = configDao.loadConfigParametersByProperties(PROP_NODE);
		logger.debug("checks found on db: " + lstConfigChecks.size());

		for (int i = 0; i < lstConfigChecks.size(); i++) {
			Config check = (Config) lstConfigChecks.get(i);
			if ((SpagoBIConstants.CHANGEPWD_CHANGE_FIRST).equals(check.getLabel()) && user.getDtLastAccess() == null) {
				// if dtLastAccess isn't enhanced it represents the first login, so is necessary change the pwd
				logger.info("The pwd needs to activate!");
				toReturn = true;
				break;
			}

			if ((SpagoBIConstants.CHANGEPWD_EXPIRED_TIME).equals(check.getLabel()) && user.getDtPwdEnd() != null
					&& currentDate.compareTo(user.getDtPwdEnd()) >= 0) {
				// check if the pwd is expiring, in this case it's locked.
				logger.info("The pwd is expiring... it should be changed");
				toReturn = true;
				break;
			}
			if ((SpagoBIConstants.CHANGEPWD_DISACTIVE_TIME).equals(check.getLabel())) {
				// defines the end date for uselessness
				Date tmpEndForUnused = null;
				if (user.getDtLastAccess() != null) {
					SimpleDateFormat sdf = new SimpleDateFormat(DATE_FORMAT);
					Calendar cal = Calendar.getInstance();
					cal.set(user.getDtLastAccess().getYear() + 1900, user.getDtLastAccess().getMonth(), user.getDtLastAccess().getDate());
					cal.add(Calendar.MONTH, 6);
					try {
						tmpEndForUnused = StringUtilities.stringToDate(sdf.format(cal.getTime()), DATE_FORMAT);
						logger.debug("End Date For Unused: " + tmpEndForUnused);
					} catch (Exception e) {
						logger.error("The control pwd goes on error: " + e);
					}
				}
				if (tmpEndForUnused != null && currentDate.compareTo(tmpEndForUnused) >= 0) {
					// check if the pwd is unused by 6 months, in this case it's locked.
					logger.info("The pwd is unused more than 6 months! It's locked!!");
					toReturn = true;
					break;
				}
			}
		} // for

		// general controls: check if the account is already blocked, otherwise update dtLastAccess field
		if (user.getFlgPwdBlocked() != null && user.getFlgPwdBlocked()) {
			// if flgPwdBlocked is true the user cannot goes on
			logger.info("The pwd needs to activate!");
			toReturn = true;
		}
		logger.debug("OUT");
		return toReturn;
	}

}
