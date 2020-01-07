package de.metas.costing;

import java.util.List;
import java.util.Set;

import org.adempiere.service.ClientId;

/*
 * #%L
 * de.metas.business
 * %%
 * Copyright (C) 2018 metas GmbH
 * %%
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 * #L%
 */

public interface ICostElementRepository
{
	CostElement getById(CostElementId costElementId);

	CostElement getOrCreateMaterialCostElement(ClientId adClientId, CostingMethod costingMethod);

	List<CostElement> getCostElementsWithCostingMethods(ClientId adClientId);

	List<CostElement> getMaterialCostingMethods(ClientId adClientId);

	List<CostElement> getNonCostingMethods(ClientId adClientId);

	List<CostElement> getByCostingMethod(CostingMethod costingMethod);

	Set<CostElementId> getActiveCostElementIds();

	Set<CostElementId> getIdsByCostingMethod(CostingMethod costingMethod);
}
