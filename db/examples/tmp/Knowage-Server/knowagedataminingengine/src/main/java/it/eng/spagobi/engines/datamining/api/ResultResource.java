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
package it.eng.spagobi.engines.datamining.api;

import it.eng.spago.security.IEngUserProfile;
import it.eng.spagobi.engines.datamining.DataMiningEngineInstance;
import it.eng.spagobi.engines.datamining.bo.DataMiningResult;
import it.eng.spagobi.engines.datamining.common.AbstractDataMiningEngineResource;
import it.eng.spagobi.engines.datamining.compute.DataMiningPythonExecutor;
import it.eng.spagobi.engines.datamining.compute.DataMiningRExecutor;
import it.eng.spagobi.engines.datamining.compute.DataMiningUtils;
import it.eng.spagobi.engines.datamining.compute.IDataMiningExecutor;
import it.eng.spagobi.engines.datamining.model.DataMiningCommand;
import it.eng.spagobi.engines.datamining.model.Output;
import it.eng.spagobi.engines.datamining.serializer.SerializationException;
import it.eng.spagobi.services.rest.annotations.ManageAuthorization;
import it.eng.spagobi.utilities.engines.SpagoBIEngineRuntimeException;

import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import javax.ws.rs.GET;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.QueryParam;

import org.apache.log4j.Logger;

@Path("/1.0/result")
@ManageAuthorization
public class ResultResource extends AbstractDataMiningEngineResource {
	public static transient Logger logger = Logger.getLogger(ResultResource.class);

	/**
	 * Service to get Result
	 *
	 * @return
	 *
	 */
	@GET
	@Path("/{command}/{output}/{rerun}")
	@Produces("text/html; charset=UTF-8")
	public String getResult(@PathParam("command") String commandName, @PathParam("output") String outputName, @PathParam("rerun") Boolean rerun,
			@QueryParam("DOC_LABEL") String docLabel) {
		logger.debug("IN");

		String outputOfExecution = null;
		DataMiningResult result = getDataMiningResult(commandName, outputName, rerun, docLabel, null, getUserProfile());

		try {
			outputOfExecution = serialize(result);
		} catch (SerializationException e) {
			logger.error("Error serializing the result", e);
			throw new SpagoBIEngineRuntimeException("Error serializing the result", e);
		}

		if (!isNullOrEmpty(outputOfExecution)) {
			logger.debug("Returning result");
		} else {
			logger.debug("No result found");
		}

		logger.debug("OUT");
		return outputOfExecution;

	}

	public DataMiningResult getDataMiningResult(String commandName, String outputName, Boolean rerun, String docLabel,
			DataMiningEngineInstance dataMiningEngineInstance, IEngUserProfile profile) {
		logger.debug("IN");

		DataMiningResult result = null;
		dataMiningEngineInstance = dataMiningEngineInstance == null ? getDataMiningEngineInstance() : dataMiningEngineInstance;

		// Instantiate an R or a Python executor depending on LANGUAGE tag in xml file
		IDataMiningExecutor executor = null;
		// TODO Factory pattern: in case language is not supported, throw an Exception
		if (dataMiningEngineInstance.getLanguage() == null) {
			dataMiningEngineInstance.setLanguage("R");
		}
		if (dataMiningEngineInstance.getLanguage().equalsIgnoreCase("Python")) {
			executor = new DataMiningPythonExecutor(dataMiningEngineInstance, profile);
		} else if (dataMiningEngineInstance.getLanguage().equalsIgnoreCase("R")) {
			executor = new DataMiningRExecutor(dataMiningEngineInstance, profile);
		}

		List<DataMiningCommand> commands = null;
		if (dataMiningEngineInstance.getCommands() != null && !dataMiningEngineInstance.getCommands().isEmpty()) {
			commands = dataMiningEngineInstance.getCommands();
			for (Iterator it = commands.iterator(); it.hasNext();) {
				DataMiningCommand cmd = (DataMiningCommand) it.next();
				if (cmd.getName().equals(commandName)) {
					List<Output> outputs = cmd.getOutputs();
					if (outputs != null && !outputs.isEmpty()) {
						for (Iterator it2 = outputs.iterator(); it2.hasNext();) {
							Output output = (Output) it2.next();
							if (output.getOutputName().equals(outputName)) {
								// and starts execution!
								try {
									HashMap params = (HashMap) dataMiningEngineInstance.getAnalyticalDrivers();
									result = executor.execute(params, cmd, output, profile, rerun, docLabel);
								} catch (IOException e) {
									logger.error("Error executing script", e);
									throw new SpagoBIEngineRuntimeException("Error executing script", e);
								} catch (Exception e) {
									logger.error("Error in script code generation", e);
									throw new SpagoBIEngineRuntimeException("Error in script code generation", e);
								}
							}
						}
					}
				}
			}
		}

		if (result != null) {
			logger.debug("Returning result");
		} else {
			logger.debug("No result found");
		}

		logger.debug("OUT");
		return result;
	}

	/**
	 * Checks whether the result panel has to be displayed ad first execution
	 *
	 * @throws IOException
	 *
	 */
	@GET
	@Path("/needsResultAtForstExec")
	@Produces("text/html; charset=UTF-8")
	public String needsResultAtForstExec() throws IOException {
		logger.debug("IN");
		Boolean resNeeded = true;
		DataMiningEngineInstance dataMiningEngineInstance = getDataMiningEngineInstance();

		resNeeded = DataMiningUtils.areDatasetsProvided(dataMiningEngineInstance, getUserProfile());
		if (!resNeeded) {
			return getJsonKo();
		}
		logger.debug("OUT");
		return getJsonSuccess();

	}

}
