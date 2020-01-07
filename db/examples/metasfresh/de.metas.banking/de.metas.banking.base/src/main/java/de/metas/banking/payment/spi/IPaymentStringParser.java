package de.metas.banking.payment.spi;

/*
 * #%L
 * de.metas.banking.base
 * %%
 * Copyright (C) 2015 metas GmbH
 * %%
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 * #L%
 */


import java.util.Properties;

import de.metas.banking.payment.IPaymentString;

public interface IPaymentStringParser
{
	/**
	 * @param ctx
	 * @param paymentText
	 * @return {@link IPaymentString} implementation filled with relevant data from the <code>paymentString</code>
	 *
	 * @throws IndexOutOfBoundsException if the paymentText has invalid length
	 */
	IPaymentString parse(Properties ctx, String paymentText) throws IndexOutOfBoundsException;
}
