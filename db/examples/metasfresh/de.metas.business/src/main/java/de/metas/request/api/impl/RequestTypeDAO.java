package de.metas.request.api.impl;

import org.adempiere.ad.dao.IQueryBL;
import org.adempiere.exceptions.AdempiereException;
import org.compiere.model.I_R_RequestType;

import com.google.common.annotations.VisibleForTesting;

import de.metas.request.RequestTypeId;
import de.metas.request.api.IRequestTypeDAO;
import de.metas.util.Services;

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

public class RequestTypeDAO implements IRequestTypeDAO
{
	@VisibleForTesting
	static final String InternalName_CustomerComplaint = "A_CustomerComplaint";
	@VisibleForTesting
	static final String InternalName_VendorComplaint = "B_VendorComplaint";

	@Override
	public RequestTypeId retrieveVendorRequestTypeId()
	{
		return retrieveRequestTypeIdByInternalName(InternalName_VendorComplaint);
	}

	@Override
	public RequestTypeId retrieveCustomerRequestTypeId()
	{
		return retrieveRequestTypeIdByInternalName(InternalName_CustomerComplaint);
	}

	private RequestTypeId retrieveRequestTypeIdByInternalName(final String internalName)
	{
		final IQueryBL queryBL = Services.get(IQueryBL.class);

		final RequestTypeId requestTypeId = queryBL.createQueryBuilderOutOfTrx(I_R_RequestType.class)
				.addOnlyActiveRecordsFilter()
				.addEqualsFilter(I_R_RequestType.COLUMNNAME_InternalName, internalName)
				.create()
				.firstIdOnly(RequestTypeId::ofRepoIdOrNull); // covered by unique index

		if (requestTypeId == null)
		{
			throw new AdempiereException("@NotFound@ @M_RequestType_ID@ (@InternalName@: " + internalName);
		}

		return requestTypeId;
	}
}
