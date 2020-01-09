package it.eng.spagobi.tools.alert;

import it.eng.spago.error.EMFUserError;
import it.eng.spago.security.IEngUserProfile;
import it.eng.spagobi.commons.dao.DAOFactory;
import it.eng.spagobi.services.rest.annotations.ManageAuthorization;
import it.eng.spagobi.services.serialization.JsonConverter;
import it.eng.spagobi.tools.alert.bo.Alert;
import it.eng.spagobi.tools.alert.bo.AlertAction;
import it.eng.spagobi.tools.alert.bo.AlertListener;
import it.eng.spagobi.tools.alert.dao.IAlertDAO;
import it.eng.spagobi.tools.scheduler.bo.TriggerPaused;
import it.eng.spagobi.tools.scheduler.dao.ISchedulerDAO;
import it.eng.spagobi.utilities.JSError;
import it.eng.spagobi.utilities.rest.RestUtilities;

import java.io.IOException;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.DELETE;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.Response;

import org.apache.log4j.Logger;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * @authors Salvatore Lupo (Salvatore.Lupo@eng.it)
 * 
 */
@Path("/1.0/alert")
@ManageAuthorization
public class AlertService {

	private static Logger logger = Logger.getLogger(AlertService.class);
	private static final String ALERT_JOB_GROUP = "ALERT_JOB_GROUP";

	@GET
	@Path("/{id}/suspend")
	public Response suspend(@PathParam("id") Integer id, @Context HttpServletRequest req) throws EMFUserError {
		ISchedulerDAO schedulerDAO = DAOFactory.getSchedulerDAO();
		String name = "" + id;
		if (!schedulerDAO.isTriggerPaused(ALERT_JOB_GROUP, name, ALERT_JOB_GROUP, name)) {
			schedulerDAO.pauseTrigger(ALERT_JOB_GROUP, name, ALERT_JOB_GROUP, name);
		}
		return Response.ok().build();
	}

	@GET
	@Path("/{id}/resume")
	public Response resume(@PathParam("id") Integer id, @Context HttpServletRequest req) throws EMFUserError {
		ISchedulerDAO schedulerDAO = DAOFactory.getSchedulerDAO();
		String name = "" + id;
		if (schedulerDAO.isTriggerPaused(ALERT_JOB_GROUP, name, ALERT_JOB_GROUP, name)) {
			TriggerPaused triggerPaused = new TriggerPaused();
			triggerPaused.setJobGroup(ALERT_JOB_GROUP);
			triggerPaused.setJobName(name);
			triggerPaused.setTriggerGroup(ALERT_JOB_GROUP);
			triggerPaused.setTriggerName(name);
			schedulerDAO.resumeTrigger(ALERT_JOB_GROUP, name, ALERT_JOB_GROUP, name);
		}
		return Response.ok().build();
	}

	@GET
	@Path("/listListener")
	public Response listListener(@Context HttpServletRequest req) throws EMFUserError {
		IAlertDAO dao = getDao(req);
		List<AlertListener> listeners = dao.listListener();
		return Response.ok(JsonConverter.objectToJson(listeners, listeners.getClass())).build();
	}

	@GET
	@Path("/listAction")
	public Response listAction(@Context HttpServletRequest req) throws EMFUserError {
		IAlertDAO dao = getDao(req);
		List<AlertAction> actions = dao.listAction();
		return Response.ok(JsonConverter.objectToJson(actions, actions.getClass())).build();
	}

	@GET
	@Path("/listAlert")
	public Response listAlert(@Context HttpServletRequest req) throws EMFUserError {
		IAlertDAO dao = getDao(req);
		List<Alert> alert = dao.listAlert();
		return Response.ok(JsonConverter.objectToJson(alert, alert.getClass())).build();
	}

	@GET
	@Path("/{id}/load")
	public Response load(@PathParam("id") Integer id, @Context HttpServletRequest req) throws EMFUserError {
		IAlertDAO dao = getDao(req);
		Alert alert = dao.loadAlert(id);
		return Response.ok(JsonConverter.objectToJson(alert, alert.getClass())).build();
	}

	@POST
	@Path("/save")
	public Response save(@Context HttpServletRequest req) throws EMFUserError {
		try {
			String str = RestUtilities.readBodyAsJSONObject(req).toString();
			Alert alert = (Alert) JsonConverter.jsonToObject(str, Alert.class);
			JSError jsError = new JSError();
			check(alert, jsError);
			if (!jsError.hasErrors()) {
				IAlertDAO dao = getDao(req);
				Integer id = alert.getId();
				if (id == null) {
					id = dao.insert(alert);
				} else {
					dao.update(alert);
				}
				return Response.ok(new JSONObject().put("id", id).toString()).build();
			} else {
				return Response.ok(jsError.toString()).build();
			}
		} catch (IOException | JSONException e) {
			logger.error(req.getPathInfo(), e);
		}
		return Response.ok().build();
	}

	@DELETE
	@Path("/{id}/delete")
	public Response delete(@PathParam("id") Integer id, @Context HttpServletRequest req) throws EMFUserError {
		IAlertDAO dao = getDao(req);
		dao.remove(id);
		return Response.ok().build();
	}

	private void check(Alert alert, JSError jsError) {
		AlertListener alertListener = alert.getAlertListener();
		if (alertListener == null) {
			jsError.addError("Listener is mandatory");
		} else {
			if (alertListener.getClassName() == null) {
				jsError.addError("Listener error");
				logger.error("Listener name[" + alertListener.getName() + "] has null class");
			} else {
				try {
					Class.forName(alertListener.getClassName());
				} catch (ClassNotFoundException e) {
					logger.error("Listener name[" + alertListener.getName() + "] has not valid class", e);
					jsError.addError("Listener error");
				}
			}
		}

	}

	private IAlertDAO getDao(HttpServletRequest req) throws EMFUserError {
		IAlertDAO dao = DAOFactory.getAlertDAO();
		dao.setUserProfile((IEngUserProfile) req.getSession().getAttribute(IEngUserProfile.ENG_USER_PROFILE));
		return dao;
	}

}
