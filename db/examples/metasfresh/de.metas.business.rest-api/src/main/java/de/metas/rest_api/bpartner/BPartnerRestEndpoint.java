package de.metas.rest_api.bpartner;

import org.springframework.http.ResponseEntity;

import de.metas.rest_api.bpartner.request.JsonRequestBPartnerUpsert;
import de.metas.rest_api.bpartner.request.JsonRequestBankAccountsUpsert;
import de.metas.rest_api.bpartner.request.JsonRequestContactUpsert;
import de.metas.rest_api.bpartner.request.JsonRequestLocationUpsert;
import de.metas.rest_api.bpartner.response.JsonResponseComposite;
import de.metas.rest_api.bpartner.response.JsonResponseCompositeList;
import de.metas.rest_api.bpartner.response.JsonResponseContact;
import de.metas.rest_api.bpartner.response.JsonResponseLocation;
import de.metas.rest_api.bpartner.response.JsonResponseUpsert;

/*
 * #%L
 * de.metas.business.rest-api
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

public interface BPartnerRestEndpoint
{
	ResponseEntity<JsonResponseComposite> retrieveBPartner(String bpartnerIdentifier);

	ResponseEntity<JsonResponseLocation> retrieveBPartnerLocation(
			String bpartnerIdentifier,
			String locationIdentifier);

	ResponseEntity<JsonResponseContact> retrieveBPartnerContact(
			String bpartnerIdentifier,
			String contactIdentifier);

	ResponseEntity<JsonResponseCompositeList> retrieveBPartnersSince(
			Long epochTimestampMillis,
			String next);

	ResponseEntity<JsonResponseUpsert> createOrUpdateBPartner(JsonRequestBPartnerUpsert bpartners);

	ResponseEntity<JsonResponseUpsert> createOrUpdateLocation(
			String bpartnerIdentifier,
			JsonRequestLocationUpsert locations);

	ResponseEntity<JsonResponseUpsert> createOrUpdateContact(
			String bpartnerIdentifier,
			JsonRequestContactUpsert contacts);

	ResponseEntity<JsonResponseUpsert> createOrUpdateBankAccount(
			String bpartnerIdentifier,
			JsonRequestBankAccountsUpsert bankAccounts);
}
