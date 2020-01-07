package de.metas.banking.payment.paymentallocation.service;

/*
 * #%L
 * de.metas.banking.swingui
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 * #L%
 */

import java.util.Collection;

import com.google.common.collect.ImmutableList;

import de.metas.i18n.ITranslatableString;
import de.metas.i18n.TranslatableStringBuilder;
import de.metas.i18n.TranslatableStrings;

/**
 * Exception thrown by {@link PaymentAllocationBuilder} when are too many documents assigned on vendor side
 *
 * @author cg
 *
 */
public class MultipleVendorDocumentsException extends PaymentAllocationException
{
	private static final long serialVersionUID = 1L;
	private static final String MSG = "PaymentAllocation.CannotAllocateMultipleDocumentsException";

	private final Collection<IPaymentDocument> payments;
	private final Collection<PayableDocument> payableDocs;

	MultipleVendorDocumentsException(final Collection<IPaymentDocument> payments, final Collection<PayableDocument> payableDocs)
	{
		super("");
		this.payments = ImmutableList.copyOf(payments);
		this.payableDocs = ImmutableList.copyOf(payableDocs);
	}

	@Override
	protected ITranslatableString buildMessage()
	{
		final TranslatableStringBuilder message = TranslatableStrings.builder();

		if (payments != null && !payments.isEmpty())
		{
			for (final IPaymentDocument payment : payments)
			{
				if (payment == null)
				{
					continue;
				}

				if (!message.isEmpty())
				{
					message.append(", ");
				}
				message.append(payment.getDocumentNo());
			}
		}

		if (payableDocs != null && !payableDocs.isEmpty())
		{
			for (final PayableDocument doc : payableDocs)
			{
				if (doc == null)
				{
					continue;
				}

				if (!message.isEmpty())
				{
					message.append(", ");
				}
				message.append(doc.getDocumentNo());
			}
		}

		message.insertFirstADMessage(MSG);
		return message.build();
	}

}
