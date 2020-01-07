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
import java.time.LocalDate;

import javax.annotation.Nullable;

import de.metas.banking.api.BankAccountId;
import org.adempiere.invoice.service.IInvoiceBL;
import org.adempiere.model.InterfaceWrapperHelper;
import org.compiere.model.I_C_Invoice;
import org.compiere.model.I_C_Payment;
import org.compiere.model.X_C_DocType;
import org.compiere.util.TimeUtil;

import de.metas.bpartner.BPartnerId;
import de.metas.document.DocTypeId;
import de.metas.document.DocTypeQuery;
import de.metas.document.IDocTypeDAO;
import de.metas.document.engine.DocStatus;
import de.metas.document.engine.IDocument;
import de.metas.document.engine.IDocumentBL;
import de.metas.invoice.InvoiceId;
import de.metas.money.CurrencyId;
import de.metas.organization.OrgId;
import de.metas.payment.TenderType;
import de.metas.util.Check;
import de.metas.util.Services;
import lombok.NonNull;

public class DefaultPaymentBuilder
{
	public static DefaultPaymentBuilder newInboundReceiptBuilder()
	{
		return new DefaultPaymentBuilder()
				.receipt(true);
	}

	public static DefaultPaymentBuilder newOutboundPaymentBuilder()
	{
		return new DefaultPaymentBuilder()
				.receipt(false);
	}

	public static DefaultPaymentBuilder newBuilderOfInvoice(@NonNull final I_C_Invoice invoice)
	{
		return new DefaultPaymentBuilder()
				.invoice(invoice);
	}

	private final transient IDocTypeDAO docTypesRepo = Services.get(IDocTypeDAO.class);

	private boolean _built = false;
	private final I_C_Payment payment;

	private DefaultPaymentBuilder()
	{
		payment = InterfaceWrapperHelper.newInstance(I_C_Payment.class);
		payment.setProcessed(false);
		payment.setDocStatus(DocStatus.Drafted.getCode());
		payment.setDocAction(IDocument.ACTION_Complete);
	}

	/**
	 * Creates and completes the payment
	 *
	 * @return payment
	 */
	public I_C_Payment createAndProcess()
	{
		return createAndProcess(IDocument.ACTION_Complete, IDocument.STATUS_Completed);
	}

	/**
	 * Creates and processes the payment.
	 *
	 * @param docAction
	 * @param expectedDocStatus
	 * @return payment
	 */
	private I_C_Payment createAndProcess(final String docAction, final String expectedDocStatus)
	{
		final I_C_Payment payment = createDraft();

		payment.setDocAction(docAction);
		Services.get(IDocumentBL.class).processEx(payment, docAction, expectedDocStatus);

		return payment;
	}

	/**
	 * Creates the draft payment.
	 *
	 * @return draft payment
	 */
	private I_C_Payment createDraft()
	{
		final I_C_Payment payment = createNoSave();
		InterfaceWrapperHelper.save(payment);
		return payment;
	}

	/**
	 * Creates the payment but it does not save it.
	 *
	 * @return payment (not saved!)
	 */
	public I_C_Payment createNoSave()
	{
		markAsBuilt();

		// note: the only reason why we are calling the "...OrNull" method is because some unit tests are failing.
		final DocTypeId docTypeId = getDocTypeIdOrNull();
		payment.setC_DocType_ID(DocTypeId.toRepoId(docTypeId));

		return payment;
	}

	private DocTypeId getDocTypeIdOrNull()
	{
		final String docBaseType = payment.isReceipt() ? X_C_DocType.DOCBASETYPE_ARReceipt : X_C_DocType.DOCBASETYPE_APPayment;

		return docTypesRepo.getDocTypeIdOrNull(DocTypeQuery.builder()
				.docBaseType(docBaseType)
				.docSubType(DocTypeQuery.DOCSUBTYPE_Any)
				.adClientId(payment.getAD_Client_ID())
				.adOrgId(payment.getAD_Org_ID())
				.build());
	}

	private final void assertNotBuilt()
	{
		Check.assume(!_built, "payment already built");
	}

	private final void markAsBuilt()
	{
		assertNotBuilt();
		_built = true;
	}

	public final DefaultPaymentBuilder adOrgId(@NonNull final OrgId adOrgId)
	{
		assertNotBuilt();
		payment.setAD_Org_ID(adOrgId.getRepoId());
		return this;
	}

	private final DefaultPaymentBuilder receipt(final boolean isReceipt)
	{
		assertNotBuilt();
		payment.setIsReceipt(isReceipt);
		return this;
	}

	public final DefaultPaymentBuilder bpBankAccountId(@Nullable final BankAccountId bpBankAccountId)
	{
		assertNotBuilt();
		if (bpBankAccountId == null)
		{
			payment.setC_BP_BankAccount_ID(0);
		}
		else
		{
			payment.setC_BP_BankAccount_ID(bpBankAccountId.getRepoId());
		}
		return this;
	}

	public final DefaultPaymentBuilder accountNo(final String accountNo)
	{
		assertNotBuilt();
		payment.setAccountNo(accountNo);
		return this;
	}

	public final DefaultPaymentBuilder dateAcct(@Nullable final LocalDate dateAcct)
	{
		assertNotBuilt();
		payment.setDateAcct(TimeUtil.asTimestamp(dateAcct));
		return this;
	}

	public final DefaultPaymentBuilder dateTrx(@Nullable final LocalDate dateTrx)
	{
		assertNotBuilt();
		payment.setDateTrx(TimeUtil.asTimestamp(dateTrx));
		return this;
	}

	public final DefaultPaymentBuilder bpartnerId(@NonNull final BPartnerId bpartnerId)
	{
		assertNotBuilt();
		payment.setC_BPartner_ID(bpartnerId.getRepoId());
		return this;
	}

	public final DefaultPaymentBuilder payAmt(final BigDecimal payAmt)
	{
		assertNotBuilt();
		payment.setPayAmt(payAmt);
		return this;
	}

	public final DefaultPaymentBuilder discountAmt(final BigDecimal discountAmt)
	{
		assertNotBuilt();
		payment.setDiscountAmt(discountAmt);
		return this;
	}

	public final DefaultPaymentBuilder writeoffAmt(final BigDecimal writeoffAmt)
	{
		assertNotBuilt();
		payment.setWriteOffAmt(writeoffAmt);
		return this;
	}

	public final DefaultPaymentBuilder currencyId(@NonNull final CurrencyId currencyId)
	{
		assertNotBuilt();
		payment.setC_Currency_ID(currencyId.getRepoId());
		return this;
	}

	public final DefaultPaymentBuilder tenderType(@NonNull final TenderType tenderType)
	{
		assertNotBuilt();
		payment.setTenderType(tenderType.getCode());
		return this;
	}

	/**
	 * Sets the following fields using the given <code>invoice</code>:
	 * <ul>
	 * <li>C_Invoice_ID
	 * <li>C_BPartner_ID
	 * <li>C_Currency_ID
	 * <li>IsReceipt: set from the invoice's <code>SOTrx</code> (negated if the invoice is a credit memo)
	 * </ul>
	 *
	 * @param invoice
	 * @return
	 */
	private final DefaultPaymentBuilder invoice(@NonNull final I_C_Invoice invoice)
	{
		adOrgId(OrgId.ofRepoId(invoice.getAD_Org_ID()));
		invoiceId(InvoiceId.ofRepoId(invoice.getC_Invoice_ID()));
		bpartnerId(BPartnerId.ofRepoId(invoice.getC_BPartner_ID()));
		currencyId(CurrencyId.ofRepoId(invoice.getC_Currency_ID()));
		receipt(computeIsReceiptFlag(invoice));

		return this;
	}

	public final DefaultPaymentBuilder invoiceId(@NonNull final InvoiceId invoiceId)
	{
		assertNotBuilt();
		payment.setC_Invoice_ID(invoiceId.getRepoId());
		return this;
	}

	private boolean computeIsReceiptFlag(@NonNull final I_C_Invoice invoice)
	{
		if (Services.get(IInvoiceBL.class).isCreditMemo(invoice))
		{
			// SOTrx=Y, but credit memo => receipt=N
			return !invoice.isSOTrx();
		}
		else
		{
			// SOTrx=Y => receipt=Y
			return invoice.isSOTrx();
		}
	}

	public final DefaultPaymentBuilder description(final String description)
	{
		assertNotBuilt();
		payment.setDescription(description);
		return this;
	}
}
