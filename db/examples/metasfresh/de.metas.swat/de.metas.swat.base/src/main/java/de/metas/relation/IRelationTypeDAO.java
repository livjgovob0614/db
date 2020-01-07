package de.metas.relation;

import java.util.Properties;

import org.compiere.model.I_AD_RelationType;
import org.compiere.util.Env;

import de.metas.util.ISingletonService;

/*
 * #%L
 * de.metas.swat.base
 * %%
 * Copyright (C) 2016 metas GmbH
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

public interface IRelationTypeDAO extends ISingletonService
{

	I_AD_RelationType retrieveForInternalName(Properties ctx, String internalName);

	default I_AD_RelationType retrieveForInternalName(final String internalName)
	{
		return retrieveForInternalName(Env.getCtx(), internalName);
	}

}
