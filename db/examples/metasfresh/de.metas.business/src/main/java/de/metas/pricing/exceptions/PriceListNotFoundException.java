package de.metas.pricing.exceptions;

import org.adempiere.exceptions.AdempiereException;

import de.metas.lang.SOTrx;

/*
 * #%L
 * de.metas.business
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

/**
 * Exception thrown when price list could not be found for a given pricing system.
 * 
 * @author metas-dev <dev@metasfresh.com>
 *
 */
public class PriceListNotFoundException extends AdempiereException
{
	private static final long serialVersionUID = -6763638806735067636L;

	private static final String MSG_NO_PO_PRICELIST_FOUND = "NoPOPriceListFound";
	private static final String MSG_NO_SO_PRICELIST_FOUND = "NoSOPriceListFound";

	public PriceListNotFoundException(final String pricingSystemName, final SOTrx soTrx)
	{
		super(getADMessage(soTrx), new Object[] { pricingSystemName });
	}

	private static final String getADMessage(final SOTrx soTrx)
	{
		if (soTrx == null)
		{
			// shall not happen, but we have to return something, so we assume it's sales
			return MSG_NO_SO_PRICELIST_FOUND;
		}
		return soTrx.isSales() ? MSG_NO_SO_PRICELIST_FOUND : MSG_NO_PO_PRICELIST_FOUND;
	}
}
