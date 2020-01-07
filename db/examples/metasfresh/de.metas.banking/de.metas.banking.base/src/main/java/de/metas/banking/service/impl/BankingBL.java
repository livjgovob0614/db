package de.metas.banking.service.impl;

import java.math.BigDecimal;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.List;

import org.adempiere.banking.model.I_C_Invoice;
import org.adempiere.exceptions.AdempiereException;
import org.adempiere.invoice.service.IInvoiceBL;
import org.adempiere.model.InterfaceWrapperHelper;
import org.adempiere.util.Constants;
import org.compiere.model.MInvoice;
import org.compiere.model.MInvoiceLine;
import org.compiere.model.MRecurrentPaymentHistory;
import org.compiere.model.MRecurrentPaymentLine;
import org.slf4j.Logger;

import de.metas.banking.model.I_C_RecurrentPayment;
import de.metas.banking.model.X_C_RecurrentPaymentLine;
import de.metas.banking.service.IBankingBL;
import de.metas.currency.CurrencyCode;
import de.metas.currency.ICurrencyDAO;
import de.metas.document.engine.IDocument;
import de.metas.logging.LogManager;
import de.metas.money.CurrencyId;
import de.metas.payment.PaymentRule;
import de.metas.util.Check;
import de.metas.util.Services;
import de.metas.util.time.SystemTime;

public class BankingBL implements IBankingBL
{
	private static final Logger logger = LogManager.getLogger(BankingBL.class);

	/**
	 * Create missing due invoices for active recurrent payments.
	 */
	@Override
	public List<I_C_Invoice> createInvoicesForRecurrentPayments(final String trxName)
	{
		final List<MRecurrentPaymentLine> dueLines = MRecurrentPaymentLine.getAllDueRecurrentPaymentLines(trxName);

		final long actualTime = System.currentTimeMillis();

		final List<I_C_Invoice> result = new ArrayList<>();

		for (final MRecurrentPaymentLine line : dueLines)
		{
			Timestamp lastDate = MRecurrentPaymentLine.getLatestHistoryDateInvoicedForLine(line.get_ID(), trxName);

			if (lastDate == null)
			{
				// create the first invoice
				result.add(createInvoiceForRecurrentPaymentLine(line));

				lastDate = line.getDateFrom();
			}

			logger.debug("lastDateInvoiced: " + lastDate.toString() + " line: " + line.get_ID());

			final GregorianCalendar cal = new GregorianCalendar();
			cal.setTimeInMillis(lastDate.getTime());

			while (true)
			{
				addToCalendar(line, cal);

				final long calTime = cal.getTimeInMillis();

				if (calTime < actualTime)
				{
					result.add(createInvoiceForRecurrentPaymentLine(line));
				}
				else
				{
					break;
				}
			}
		}
		return result;
	}

	private void addToCalendar(final MRecurrentPaymentLine line, final GregorianCalendar cal)
	{
		final int frequency = line.getFrequency();

		final String frequencyType = line.getFrequencyType();

		if (X_C_RecurrentPaymentLine.FREQUENCYTYPE_Month.equals(frequencyType))
		{
			cal.add(Calendar.MONTH, frequency);
			logger.debug(frequency + " month added");
		}
		else if (X_C_RecurrentPaymentLine.FREQUENCYTYPE_Day
				.equals(frequencyType))
		{
			cal.add(Calendar.DAY_OF_MONTH, frequency);
			logger.debug(frequency + " days added");
		}
		else
		{
			throw new AdempiereException("Unimplemented frequency type: " + frequencyType);
		}
	}

	/**
	 * Create an recurrent payment invoice for the given recurrent payment line.
	 *
	 * @param line
	 * @param dateInvoiced
	 */
	public I_C_Invoice createInvoiceForRecurrentPaymentLine(final MRecurrentPaymentLine line)
	{
		final I_C_RecurrentPayment header = line.getC_RecurrentPayment();

		final MInvoice invoice = new MInvoice(line.getCtx(), 0, line.get_TrxName());
		invoice.setAD_Org_ID(line.getAD_Org_ID());

		final I_C_Invoice extInvoice = InterfaceWrapperHelper.create(invoice, I_C_Invoice.class);
		extInvoice.setC_RecurrentPaymentLine_ID(line.get_ID());

		final Timestamp dateInvoiced = SystemTime.asTimestamp();
		invoice.setDateInvoiced(dateInvoiced);
		invoice.setDateAcct(dateInvoiced);

		invoice.setC_BPartner_ID(header.getC_BPartner_ID());
		invoice.setC_BPartner_Location_ID(header.getC_BPartner_Location_ID());

		invoice.setC_Currency_ID(line.getC_Currency_ID());
		invoice.setAD_User_ID(line.getAD_User_ID());
		invoice.setSalesRep_ID(line.getSalesRep_ID());
		invoice.setC_PaymentTerm_ID(line.getC_PaymentTerm_ID());
		invoice.setIsSOTrx(false); // always vendor invoice
		
		// FRESH-488: Set the payment rule 
		final PaymentRule paymentRule = Services.get(IInvoiceBL.class).getDefaultPaymentRule();
		invoice.setPaymentRule(paymentRule.getCode());

		Services.get(IInvoiceBL.class).setDocTypeTargetId(invoice, Constants.DOCBASETYPE_AVIinvoice);
		invoice.saveEx();

		final MInvoiceLine invoiceLine = new MInvoiceLine(invoice);
		invoiceLine.setC_Charge_ID(line.getC_Charge_ID());
		invoiceLine.setDescription(line.getDescription());
		invoiceLine.setQty(BigDecimal.ONE);
		invoiceLine.setPrice(line.getPayAmt());
		invoiceLine.setTax();
		invoiceLine.saveEx();

		// complete the invoice
		final boolean completed = invoice.processIt(IDocument.ACTION_Complete);
		if (completed)
		{
			invoice.saveEx();
		}
		else
		{
			throw new AdempiereException("Unable to complete invoice " + invoice.getDocumentNo());
		}

		// create history entry
		final MRecurrentPaymentHistory history = new MRecurrentPaymentHistory(line.getCtx(), 0, line.get_TrxName());
		history.setC_Invoice_ID(invoice.get_ID());
		history.setC_RecurrentPaymentLine_ID(line.get_ID());
		history.saveEx();

		return extInvoice;
	}

	@Override
	public String createBankAccountName(final org.compiere.model.I_C_BP_BankAccount bankAccount)
	{
		final StringBuilder name = new StringBuilder();
		
		if (bankAccount.getC_Bank_ID() > 0)
		{
			name.append(bankAccount.getC_Bank().getName());
		}
		else if (!Check.isBlank(bankAccount.getA_Name()))
		{
			// fallback
			name.append(bankAccount.getA_Name());
		}
		
		final CurrencyId currencyId = CurrencyId.ofRepoIdOrNull(bankAccount.getC_Currency_ID());
		if (currencyId != null)
		{
			if (name.length() > 0)
			{
				name.append("_");
			}
			
			final CurrencyCode currencyCode = Services.get(ICurrencyDAO.class).getCurrencyCodeById(currencyId);
			name.append(currencyCode.toThreeLetterCode());
		}
		return name.toString();
	}
}
