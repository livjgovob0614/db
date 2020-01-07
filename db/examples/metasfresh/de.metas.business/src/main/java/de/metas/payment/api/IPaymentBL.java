/**
 * 
 */
package de.metas.payment.api;

/*
 * #%L
 * de.metas.adempiere.adempiere.base
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

import java.math.BigDecimal;
import java.util.Date;
import java.util.List;
import java.util.Set;

import org.compiere.model.I_C_AllocationHdr;
import org.compiere.model.I_C_BPartner;
import org.compiere.model.I_C_Invoice;
import org.compiere.model.I_C_Payment;

import de.metas.payment.PaymentId;
import de.metas.payment.PaymentRule;
import de.metas.util.ISingletonService;

public interface IPaymentBL extends ISingletonService
{
	I_C_Payment getById(PaymentId paymentId);

	List<I_C_Payment> getByIds(Set<PaymentId> paymentIds);

	DefaultPaymentBuilder newInboundReceiptBuilder();

	DefaultPaymentBuilder newOutboundPaymentBuilder();

	DefaultPaymentBuilder newBuilderOfInvoice(I_C_Invoice invoice);

	/**
	 * 
	 * @param payment
	 * @param colName source column name
	 * @param creditMemoAdjusted True if we want to get absolute values for Credit Memos
	 */
	void updateAmounts(final I_C_Payment payment, final String colName, boolean creditMemoAdjusted);

	/**
	 * updates amount when flag IsOverUnderPayment change
	 * 
	 * @param payment
	 * @param creditMemoAdjusted True if we want to get absolute values for Credit Memos
	 */
	void onIsOverUnderPaymentChange(final I_C_Payment payment, boolean creditMemoAdjusted);

	/**
	 * updates amounts when currency change
	 * 
	 * @param payment
	 */
	void onCurrencyChange(final I_C_Payment payment);

	/**
	 * updates amounts when PayAmt change
	 * 
	 * @param payment
	 * @param creditMemoAdjusted True if we want to get absolute values for Credit Memos
	 */
	void onPayAmtChange(final I_C_Payment payment, boolean creditMemoAdjusted);

	/**
	 * @return the payment rule for the BP. If none is set, gets the one of the BP group.
	 */
	PaymentRule getPaymentRuleForBPartner(I_C_BPartner bPartner);

	/**
	 * check if the invoice is allocated with the specified payment
	 * 
	 * @param payment
	 * @param invoice
	 * @return
	 */
	boolean isMatchInvoice(I_C_Payment payment, I_C_Invoice invoice);

	/**
	 * Test Allocation (and set allocated flag)
	 *
	 * @return true if updated
	 */
	boolean testAllocation(I_C_Payment payment);

	boolean isCashTrx(final I_C_Payment payment);

	/**
	 * WriteOff given payment.
	 * 
	 * NOTE: transaction is automatically handled (thread inherited transaction will be used or a new one will be created).
	 * 
	 * @param payment
	 * @param writeOffAmt amount to write-off
	 * @param date allocation date
	 * @return generated and completed allocation
	 */
	I_C_AllocationHdr paymentWriteOff(final I_C_Payment payment, final BigDecimal writeOffAmt, final Date date);
}
