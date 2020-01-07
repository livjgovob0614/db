package de.metas.currency.impl;

import java.time.LocalDate;
import java.util.Comparator;

import org.adempiere.service.ClientId;

import de.metas.money.CurrencyConversionTypeId;
import de.metas.organization.OrgId;
import lombok.Builder;
import lombok.NonNull;
import lombok.Value;

/*
 * #%L
 * de.metas.adempiere.adempiere.base
 * %%
 * Copyright (C) 2019 metas GmbH
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

@Value
@Builder
final class CurrencyConversionTypeRouting
{
	@NonNull
	ClientId clientId;

	@NonNull
	OrgId orgId;

	@NonNull
	LocalDate validFrom;

	@NonNull
	CurrencyConversionTypeId conversionTypeId;

	public boolean isMatching(
			@NonNull final ClientId clientId,
			@NonNull final OrgId orgId,
			@NonNull final LocalDate date)
	{
		return (this.clientId.isSystem() || ClientId.equals(this.clientId, clientId))
				&& (this.orgId.isAny() || OrgId.equals(this.orgId, orgId))
				&& this.validFrom.compareTo(date) <= 0;
	}

	public static Comparator<CurrencyConversionTypeRouting> moreSpecificFirstComparator()
	{
		return (routing1, routing2) -> routing1.isMoreSpecificThan(routing2) ? -1 : 0;
	}

	public boolean isMoreSpecificThan(@NonNull final CurrencyConversionTypeRouting other)
	{
		if (!ClientId.equals(this.getClientId(), other.getClientId()))
		{
			return this.getClientId().isRegular();
		}

		if (!OrgId.equals(this.getOrgId(), other.getOrgId()))
		{
			return this.getOrgId().isRegular();
		}

		return this.getValidFrom().compareTo(other.getValidFrom()) > 0;
	}
}
