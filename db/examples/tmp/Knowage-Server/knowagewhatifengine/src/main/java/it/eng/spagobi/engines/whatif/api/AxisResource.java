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
package it.eng.spagobi.engines.whatif.api;

import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;

import org.apache.log4j.Logger;

import org.json.JSONObject;
import org.olap4j.Axis;
import org.olap4j.metadata.Hierarchy;
import org.olap4j.metadata.Member;
import org.pivot4j.PivotModel;
import org.pivot4j.transform.SwapAxes;

import it.eng.spagobi.engines.whatif.WhatIfEngineInstance;
import it.eng.spagobi.engines.whatif.axis.AxisDimensionManager;
import it.eng.spagobi.engines.whatif.common.AbstractWhatIfEngineService;
import it.eng.spagobi.engines.whatif.model.ModelConfig;
import it.eng.spagobi.engines.whatif.model.SpagoBIPivotModel;
import it.eng.spagobi.services.rest.annotations.ManageAuthorization;
import it.eng.spagobi.utilities.rest.RestUtilities;

@Path("/1.0/axis")
@ManageAuthorization

public class AxisResource extends AbstractWhatIfEngineService {

	public static transient Logger logger = Logger.getLogger(AxisResource.class);

	private AxisDimensionManager axisBusiness;

	private AxisDimensionManager getAxisBusiness() {
		WhatIfEngineInstance ei = getWhatIfEngineInstance();

		if (axisBusiness == null) {
			axisBusiness = new AxisDimensionManager(ei.getPivotModel());
		}
		return axisBusiness;
	}

	/**
	 * Service to swap the axis
	 *
	 * @return the rendered pivot table
	 */
	@POST
	@Path("/swap")
	@Produces("text/html; charset=UTF-8")
	
	public String swapAxis() {
		logger.debug("IN");
		WhatIfEngineInstance ei = getWhatIfEngineInstance();
		SpagoBIPivotModel model = (SpagoBIPivotModel) ei.getPivotModel();
		model.removeSubset();
		SwapAxes transform = model.getTransform(SwapAxes.class);
		if (transform.isSwapAxes()) {
			transform.setSwapAxes(false);

		} else {
			transform.setSwapAxes(true);

		}

		String table = renderModel(model);
		logger.debug("OUT");
		return table;

	}

	/**
	 * Service to move an hierarchy from an axis to another
	 *
	 * @param req
	 *            the HttpServletRequest
	 * @param fromAxisPos
	 *            the source axis(0 for rows, 1 for columns, -1 for filters)
	 * @param toAxisPos
	 *            the destination axis(0 for rows, 1 for columns, -1 for
	 *            filters)
	 * @param hierarchyName
	 *            the unique name of the hierarchy to move
	 * @return the rendered pivot table
	 */
	@POST
	@Path("/moveDimensionToOtherAxis")
	@Produces("text/html; charset=UTF-8")
	
	public String placeHierarchyOnAxis(@javax.ws.rs.core.Context HttpServletRequest req) {

		int fromAxisPos = 0;
		int toAxisPos = 0;
		String hierarchyName = null;
		try {
			JSONObject paramsObj = RestUtilities.readBodyAsJSONObject(req);

			fromAxisPos = paramsObj.getInt("fromAxis");
			toAxisPos = paramsObj.getInt("toAxis");
			hierarchyName = paramsObj.getString("hierarchy");
		} catch (Exception e) {
			logger.error("Error reading body", e);
		}

		WhatIfEngineInstance ei = getWhatIfEngineInstance();
		SpagoBIPivotModel model = (SpagoBIPivotModel) ei.getPivotModel();
		getAxisBusiness().moveDimensionToOtherAxis(fromAxisPos, toAxisPos, hierarchyName);

		model.removeSubset();

		model.removeOrder(Axis.ROWS);
		model.removeOrder(Axis.COLUMNS);

		ModelConfig modelConfig = getWhatIfEngineInstance().getModelConfig();
		modelConfig.setRowCount(model.getCellSet().getAxes().get(Axis.ROWS.axisOrdinal()).getPositionCount());
		modelConfig.setColumnCount(model.getCellSet().getAxes().get(Axis.COLUMNS.axisOrdinal()).getPositionCount());

		return renderModel(getPivotModel());
	}

	/**
	 * Service to move a hierarchy in the axis
	 *
	 * @param req
	 *            the HttpServletRequest
	 * @param axisPos
	 *            the destination axis(0 for rows, 1 for columns, -1 for
	 *            filters)
	 * @param hierarchyUniqueName
	 *            the unique name of the hierarchy to move
	 * @param newPosition
	 *            the new position of the hierarchy
	 * @param direction
	 *            the direction of the movement (-1 up, +1 down)
	 * @return the rendered pivot table
	 */
	@POST
	@Path("/moveHierarchy")
	@Produces("text/html; charset=UTF-8")
	
	public String moveHierarchies(@javax.ws.rs.core.Context HttpServletRequest req) {
		WhatIfEngineInstance ei = getWhatIfEngineInstance();
		SpagoBIPivotModel model = (SpagoBIPivotModel) ei.getPivotModel();

		String hierarchyUniqueName = null;
		int newPosition = -2;
		int direction = -2;
		int axisPos = -2;
		try {
			JSONObject paramsObj = RestUtilities.readBodyAsJSONObject(req);

			hierarchyUniqueName = paramsObj.getString("hierarchy");
			newPosition = paramsObj.getInt("newPosition");
			direction = paramsObj.getInt("direction");
			axisPos = paramsObj.getInt("axis");
		} catch (Exception e) {
			logger.error("Error reading body", e);
		}

		getAxisBusiness().moveHierarchy(axisPos, hierarchyUniqueName, newPosition, direction);

		model.setSorting(false);

		model.removeSubset();
		model.removeOrder(Axis.ROWS);
		model.removeOrder(Axis.COLUMNS);

		String table = renderModel(model);
		return table;
	}

	/**
	 * Removes the oldHierarchy from the axis and adds the new newHierarchy in
	 * the same position
	 *
	 * @param axisPos
	 *            the axis that contains the old hierarchy
	 * @param newHierarchyUniqueName
	 *            the unique name of the new hierarchy
	 * @param oldHierarchyUniqueName
	 *            the unique name of the old hierarchy
	 * @param hierarchyPosition
	 *            the position of the old hierarchy
	 */
	@POST
	@Path("/updateHierarchyOnDimension")
	@Produces("text/html; charset=UTF-8")
	
	public String updateHierarchyOnDimension(@javax.ws.rs.core.Context HttpServletRequest req) {
		int axisPos = -2;
		String newHierarchyUniqueName = null;
		String oldHierarchyUniqueName = null;
		int hierarchyPosition = -1;

		try {
			JSONObject paramsObj = RestUtilities.readBodyAsJSONObject(req);

			newHierarchyUniqueName = paramsObj.getString("newHierarchyUniqueName");
			oldHierarchyUniqueName = paramsObj.getString("oldHierarchyUniqueName");
			hierarchyPosition = paramsObj.getInt("hierarchyPosition");
			axisPos = paramsObj.getInt("axis");
		} catch (Exception e) {
			logger.error("Error reading body", e);
		}

		Hierarchy h = getAxisBusiness().updateHierarchyOnAxis(axisPos, newHierarchyUniqueName, oldHierarchyUniqueName, hierarchyPosition);

		getModelConfig().setDimensionHierarchy(h.getDimension().getUniqueName(), newHierarchyUniqueName);
		PivotModel model = getWhatIfEngineInstance().getPivotModel();
		ModelConfig modelConfig = getWhatIfEngineInstance().getModelConfig();
		modelConfig.setRowCount(model.getCellSet().getAxes().get(Axis.ROWS.axisOrdinal()).getPositionCount());
		modelConfig.setColumnCount(model.getCellSet().getAxes().get(Axis.COLUMNS.axisOrdinal()).getPositionCount());
		return renderModel(getPivotModel());
	}

	/**
	 * Service to change the visibility of the members of a hierarchy. It takes
	 * a hierarchy, removes all the members and shows only the ones passed in
	 * the body of the request
	 *
	 * @param req
	 *            the HttpServletRequest
	 * @param axisPos
	 *            the source axis(0 for rows, 1 for columns, -1 for filters)
	 * @return the rendered pivot table
	 */
	@POST
	@Path("/{axis}/placeMembersOnAxis")
	@Produces("text/html; charset=UTF-8")
	
	public String placeMembersOnAxis(@javax.ws.rs.core.Context HttpServletRequest req, @PathParam("axis") int axisPos) {

		List<Member> members = getMembersFromBody();

		if (members.size() > 0) {
			getAxisBusiness().updateAxisHierarchyMembers(members.get(0).getHierarchy(), members);
		}
		PivotModel model = getWhatIfEngineInstance().getPivotModel();
		ModelConfig modelConfig = getWhatIfEngineInstance().getModelConfig();
		modelConfig.setRowCount(model.getCellSet().getAxes().get(Axis.ROWS.axisOrdinal()).getPositionCount());
		modelConfig.setColumnCount(model.getCellSet().getAxes().get(Axis.COLUMNS.axisOrdinal()).getPositionCount());
		return renderModel(getPivotModel());
	}

}
