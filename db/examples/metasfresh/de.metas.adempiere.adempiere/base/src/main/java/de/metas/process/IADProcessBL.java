package de.metas.process;

import org.compiere.model.I_AD_Process;
import org.compiere.model.I_AD_Table;

import de.metas.util.ISingletonService;

/*
 * #%L
 * de.metas.adempiere.adempiere.base
 * %%
 * Copyright (C) 2017 metas GmbH
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

public interface IADProcessBL extends ISingletonService
{

	/**
	 * Create a document specific process with a specific workflow for the given document. Link the process to the document
	 * 
	 * @param document
	 * @return
	 */
	I_AD_Process createAndLinkDocumentSpecificProcess(I_AD_Table document);

}
