package de.metas.contracts.process;

import org.compiere.Adempiere;

import com.google.common.collect.ImmutableSet;

import de.metas.bpartner.BPartnerId;
import de.metas.bpartner.service.IBPartnerDAO;
import de.metas.contracts.impl.CustomerRetentionRepository;
import de.metas.process.JavaProcess;
import de.metas.util.Services;

/*
 * #%L
 * de.metas.contracts
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

public class C_Customer_Retention_CreateMissing extends JavaProcess
{

	final CustomerRetentionRepository customerRetentionRepo = Adempiere.getBean(CustomerRetentionRepository.class);

	final IBPartnerDAO bpartnerDAO = Services.get(IBPartnerDAO.class);

	@Override
	protected String doIt() throws Exception
	{

		final ImmutableSet<BPartnerId> customers = bpartnerDAO.retrieveAllCustomerIDs();

		customers.stream()
				.filter(customerId -> !customerRetentionRepo.hasCustomerRetention(customerId))
				.forEach(customerId -> customerRetentionRepo.createNewCustomerRetention(customerId));

		customers.stream()
				.filter(customerId -> !customerRetentionRepo.isNewCustomer(customerId))
				.forEach(customerId -> customerRetentionRepo.updateCustomerRetention(customerId));

		return MSG_OK;
	}

}
