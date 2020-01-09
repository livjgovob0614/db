package it.eng.spagobi.commons.serializer;

import java.util.Iterator;
import java.util.List;
import java.util.Locale;

import org.json.JSONArray;
import org.json.JSONObject;

import it.eng.spagobi.analiticalmodel.document.handlers.AbstractDriverRuntime;
import it.eng.spagobi.analiticalmodel.document.handlers.DocumentDriverRuntime;
import it.eng.spagobi.analiticalmodel.execution.bo.LovValue;
import it.eng.spagobi.analiticalmodel.execution.bo.defaultvalues.DefaultValuesList;
import it.eng.spagobi.behaviouralmodel.analyticaldriver.bo.AbstractParview;
import it.eng.spagobi.commons.utilities.messages.MessageBuilder;

public class DocumentParameterForExecutionJSONSerializer implements Serializer {

	@Override
	public Object serialize(Object o, Locale locale) throws SerializationException {
		JSONObject result = null;

		if (!(o instanceof DocumentDriverRuntime)) {
			throw new SerializationException("DocumentParameterForExecutionJSONSerializer is unable to serialize object of type: " + o.getClass().getName());
		}

		try {
			DocumentDriverRuntime parameter = (DocumentDriverRuntime) o;
			result = new JSONObject();
			result.put("id", parameter.getId());
			MessageBuilder msgBuild = new MessageBuilder();

			// String label=msgBuild.getUserMessage(parameter.getLabel(),null,
			// locale);
			String label = parameter.getLabel();
			// label = msgBuild.getI18nMessage(locale, label);

			result.put("label", label);
			result.put("type", parameter.getParType());
			result.put("selectionType", parameter.getSelectionType());
			result.put("allowInternalNodeSelection", parameter.getPar().getModalityValue().getLovProvider().contains("<LOVTYPE>treeinner</LOVTYPE>"));
			result.put("enableMaximizer", parameter.isEnableMaximizer());
			result.put("typeCode", parameter.getTypeCode());
			result.put("mandatory", parameter.isMandatory());
			result.put("colspan", parameter.getColspan());
			result.put("thickPerc", parameter.getThickPerc());
			result.put("multivalue", parameter.isMultivalue());
			result.put("visible", parameter.isVisible());
			result.put("valuesCount", parameter.getValuesCount());
			result.put("valueSelection", parameter.getValueSelection());
			result.put("selectedLayer", parameter.getSelectedLayer());
			result.put("selectedLayerProp", parameter.getSelectedLayerProp());

			if (parameter.getValuesCount() == 1) {
				result.put("value", parameter.getValue());
			}

			if (parameter.getObjParameterIds() != null) {
				JSONArray objParameterIds = new JSONArray();
				for (Iterator iterator = parameter.getObjParameterIds().iterator(); iterator.hasNext();) {
					Integer id = (Integer) iterator.next();
					objParameterIds.put(id);
				}
				result.put("objParameterIds", objParameterIds);
			}

			JSONArray dependencies = new JSONArray();
			Iterator it = parameter.getDependencies().keySet().iterator();
			while (it.hasNext()) {
				String paramUrlName = (String) it.next();
				JSONObject dependency = new JSONObject();
				dependency.put("urlName", paramUrlName);
				dependency.put("hasDataDependency", false);
				dependency.put("hasVisualDependency", false);
				JSONArray visualDependencyConditions = new JSONArray();
				dependency.put("visualDependencyConditions", visualDependencyConditions);

				List<DocumentDriverRuntime.DriverDependencyRuntime> parameterDependencies;
				parameterDependencies = parameter.getDependencies().get(paramUrlName);

				for (int i = 0; i < parameterDependencies.size(); i++) {
					Object pd = parameterDependencies.get(i);
					if (pd instanceof AbstractDriverRuntime.DataDependencyRuntime) {
						dependency.put("hasDataDependency", true);
					} else if (pd instanceof AbstractDriverRuntime.VisualDependencyRuntime) {
						AbstractParview visualCondition = ((DocumentDriverRuntime.VisualDependencyRuntime) pd).condition;
						dependency.put("hasVisualDependency", true);
						JSONObject visualDependencyCondition = new JSONObject();
						visualDependencyCondition.put("operation", visualCondition.getOperation());
						visualDependencyCondition.put("value", visualCondition.getCompareValue());
						String viewLabel = visualCondition.getViewLabel();
						// viewLabel = msgBuild.getI18nMessage(locale, viewLabel);
						visualDependencyCondition.put("label", viewLabel);
						visualDependencyConditions.put(visualDependencyCondition);
					}
				}

				dependencies.put(dependency);
			}
			result.put("dependencies", dependencies);
			result.put("parameterUseId", parameter.getParameterUseId());

			JSONArray defaultValues = new JSONArray();
			DefaultValuesList defaults = parameter.getDefaultValues();
			Iterator<LovValue> defaultsIt = defaults.iterator();
			while (defaultsIt.hasNext()) {
				LovValue aDefault = defaultsIt.next();
				JSONObject aDefaultJSON = new JSONObject();
				aDefaultJSON.put("value", aDefault.getValue());
				aDefaultJSON.put("description", aDefault.getDescription());
				defaultValues.put(aDefaultJSON);
			}
			result.put("defaultValues", defaultValues);
		} catch (Throwable t) {
			throw new SerializationException("An error occurred while serializing object: " + o, t);
		} finally {

		}

		return result;
	}

}
