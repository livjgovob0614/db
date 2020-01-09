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
package it.eng.spagobi.commons.serializer;

import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

import org.apache.log4j.Logger;
import org.joda.time.DateTime;
import org.joda.time.format.DateTimeFormatter;
import org.joda.time.format.ISODateTimeFormat;
import org.json.JSONArray;
import org.json.JSONObject;

import it.eng.spagobi.commons.dao.DAOFactory;
import it.eng.spagobi.tools.scheduler.bo.Trigger;
import it.eng.spagobi.tools.scheduler.dao.ISchedulerDAO;

/**
 * @author Marco Cortella (marco.cortella@eng.it)
 *
 */
public class TriggerJSONSerializer implements Serializer {

	private static transient Logger logger = Logger.getLogger(TriggerJSONSerializer.class);

	public static final String JOB_NAME = "jobName";
	public static final String JOB_GROUP = "jobGroup";
	public static final String TRIGGER_NAME = "triggerName";
	public static final String TRIGGER_GROUP = "triggerGroup";
	public static final String TRIGGER_DESCRIPTION = "triggerDescription";
	public static final String TRIGGER_CALENDAR_NAME = "triggerCalendarName";
	/**
	 * @deprecated {@link #TRIGGER_ZONED_START_TIME} may be preferred
	 */
	@Deprecated
	public static final String TRIGGER_START_DATE = "triggerStartDate";
	/**
	 * @deprecated {@link #TRIGGER_ZONED_START_TIME} may be preferred
	 */
	@Deprecated
	public static final String TRIGGER_START_TIME = "triggerStartTime";
	/**
	 * @deprecated {@link #TRIGGER_ZONED_END_TIME} may be preferred
	 */
	@Deprecated
	public static final String TRIGGER_END_DATE = "triggerEndDate";
	/**
	 * @deprecated {@link #TRIGGER_ZONED_END_TIME} may be preferred
	 */
	@Deprecated
	public static final String TRIGGER_END_TIME = "triggerEndTime";
	public static final String TRIGGER_CHRON_STRING = "triggerChronString";
	public static final String TRIGGER_CHRON_TYPE = "triggerChronType";
	public static final String TRIGGER_IS_PAUSED = "triggerIsPaused";

	public static final String JOB_PARAMETERS = "jobParameters";

	public static final String TRIGGER_ZONED_START_TIME = "triggerZonedStartTime";
	public static final String TRIGGER_ZONED_END_TIME = "triggerZonedEndTime";

	@Override
	public Object serialize(Object o, Locale locale) throws SerializationException {
		JSONObject result = null;
		if (!(o instanceof Trigger)) {
			throw new SerializationException("TriggerJSONSerializer is unable to serialize object of type: " + o.getClass().getName());
		}
		try {
			Trigger trigger = (Trigger) o;
			result = new JSONObject();

			// Job Name and Job Group at which this Trigger belongs to
			String jobName = (trigger.getJob().getName() != null) ? trigger.getJob().getName() : "";
			String jobGroup = (trigger.getJob().getGroupName() != null) ? trigger.getJob().getGroupName() : "";
			result.put(JOB_NAME, jobName);
			result.put(JOB_GROUP, jobGroup);

			String triggerName = (trigger.getName() != null) ? trigger.getName() : "";
			String triggerGroup = (trigger.getGroupName() != null) ? trigger.getGroupName() : "";
			String triggerDescription = (trigger.getDescription() != null) ? trigger.getDescription() : "";
			result.put(TRIGGER_NAME, triggerName);
			result.put(TRIGGER_GROUP, triggerGroup);
			result.put(TRIGGER_DESCRIPTION, triggerDescription);

			Date triggerStartTime = trigger.getStartTime();
			String triggerStartDateSerialized = "";
			String triggerStartTimeSerialized = "";
			if (triggerStartTime != null) {
				triggerStartDateSerialized = serializeDate(triggerStartTime);
				triggerStartTimeSerialized = serializeTime(triggerStartTime);
			}
			Date triggerEndTime = trigger.getEndTime();
			String triggerEndDateSerialized = "";
			String triggerEndTimeSerialized = "";
			if (triggerEndTime != null) {
				triggerEndDateSerialized = serializeDate(triggerEndTime);
				triggerEndTimeSerialized = serializeTime(triggerEndTime);
			}
			result.put(TRIGGER_CALENDAR_NAME, triggerName);
			result.put(TRIGGER_START_DATE, triggerStartDateSerialized);
			result.put(TRIGGER_START_TIME, triggerStartTimeSerialized);
			result.put(TRIGGER_END_DATE, triggerEndDateSerialized);
			result.put(TRIGGER_END_TIME, triggerEndTimeSerialized);

			String triggerZonedStartTimeSerialized = "";
			if (triggerStartTime != null) {
				triggerZonedStartTimeSerialized = serializeZonedTime(triggerStartTime);
			}
			String triggerZonedEndTimeSerialized = "";
			if (triggerEndTime != null) {
				triggerZonedEndTimeSerialized = serializeZonedTime(triggerEndTime);
			}
			result.put(TRIGGER_ZONED_START_TIME, triggerZonedStartTimeSerialized);
			result.put(TRIGGER_ZONED_END_TIME, triggerZonedEndTimeSerialized);

			String triggerCronExpression = ((trigger.getChronExpression().getExpression()) != null) ? trigger.getChronExpression().getExpression() : "";
			result.put(TRIGGER_CHRON_STRING, triggerCronExpression);

			String triggerCronType = trigger.getChronType() != null ? trigger.getChronType() : "";
			result.put(TRIGGER_CHRON_TYPE, triggerCronType);

			ISchedulerDAO schedulerDAO = DAOFactory.getSchedulerDAO();
			boolean isTriggerPaused = schedulerDAO.isTriggerPaused(triggerGroup, triggerName, jobGroup, jobName);
			result.put(TRIGGER_IS_PAUSED, isTriggerPaused);

			// Job parameter for trigger details
			JSONArray parsListJSON = new JSONArray();

			Map<String, String> jobParameters = trigger.getJob().getParameters();
			Set<String> jobParametersName = jobParameters.keySet();
			for (String jobParameterName : jobParametersName) {
				String jobParameterValue = jobParameters.get(jobParameterName);
				if (jobParameterValue == null) {
					logger.warn("Job parameter [" + jobParameterName + "] has no value");
					jobParameterValue = "";
				}
				JSONObject jsonPar = new JSONObject();
				jsonPar.put("name", jobParameterName);
				jsonPar.put("value", jobParameterValue);
				parsListJSON.put(jsonPar);

			}
			result.put(JOB_PARAMETERS, parsListJSON);

		} catch (Throwable t) {
			throw new SerializationException("An error occurred while serializing object: " + o, t);
		}

		return result;
	}

	public String serializeTime(Date date) {
		String serializedTime;

		serializedTime = null;
		Calendar startCal = new GregorianCalendar();
		startCal.setTime(date);

		int hour = startCal.get(Calendar.HOUR_OF_DAY);
		int minute = startCal.get(Calendar.MINUTE);
		// hour format: hh:mm
		serializedTime = ((hour < 10) ? "0" : "") + hour + ":" + ((minute < 10) ? "0" : "") + minute;

		return serializedTime;
	}

	public String serializeDate(Date date) {
		String serializedDate;

		serializedDate = null;
		Calendar startCal = new GregorianCalendar();
		startCal.setTime(date);
		// date format: dd/mm/yyyy
		int day = startCal.get(Calendar.DAY_OF_MONTH);
		int month = startCal.get(Calendar.MONTH);
		int year = startCal.get(Calendar.YEAR);
		serializedDate = ((day < 10) ? "0" : "") + day + "/" + ((month + 1 < 10) ? "0" : "") + (month + 1) + "/" + year;

		return serializedDate;
	}

	/**
	 * Serialize {@link Date} to ISO 8601 format string.
	 */
	private String serializeZonedTime(Date triggerEndTime) {
		DateTimeFormatter formatter = ISODateTimeFormat.dateTime();
		DateTime dateTime = new DateTime(triggerEndTime);
		return formatter.print(dateTime);
	}

}
