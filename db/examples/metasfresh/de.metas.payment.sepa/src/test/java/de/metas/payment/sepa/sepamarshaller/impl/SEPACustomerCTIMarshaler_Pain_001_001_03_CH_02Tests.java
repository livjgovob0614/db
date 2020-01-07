package de.metas.payment.sepa.sepamarshaller.impl;

import static org.adempiere.model.InterfaceWrapperHelper.newInstance;
import static org.adempiere.model.InterfaceWrapperHelper.save;
import static org.assertj.core.api.Assertions.assertThat;

import java.math.BigDecimal;

import org.adempiere.test.AdempiereTestHelper;
import org.junit.Before;
import org.junit.Test;

import de.metas.currency.CurrencyCode;
import de.metas.currency.impl.PlainCurrencyDAO;
import de.metas.money.CurrencyId;
import de.metas.payment.esr.model.I_C_BP_BankAccount;
import de.metas.payment.sepa.jaxb.sct.pain_001_001_03_ch_02.Document;
import de.metas.payment.sepa.model.I_SEPA_Export;
import de.metas.payment.sepa.model.I_SEPA_Export_Line;

public class SEPACustomerCTIMarshaler_Pain_001_001_03_CH_02Tests
{
	private SEPACustomerCTIMarshaler_Pain_001_001_03_CH_02 xmlGenerator;
	private Document xmlDocument;

	private CurrencyId eur;
	private CurrencyId chf;

	@Before
	public void beforeTest()
	{
		AdempiereTestHelper.get().init();

		this.xmlGenerator = new SEPACustomerCTIMarshaler_Pain_001_001_03_CH_02();
		this.xmlDocument = null;

		eur = PlainCurrencyDAO.createCurrencyId(CurrencyCode.EUR);
		chf = PlainCurrencyDAO.createCurrencyId(CurrencyCode.CHF);
	}

	@Test
	public void createDocument_batch() throws Exception
	{
		final I_SEPA_Export sepaExport = createSEPAExport(
				"org", // SEPA_CreditorIdentifier
				"INGBNL2A" // bic
		);
		createSEPAExportLine(sepaExport, "001",// SEPA_MandateRefNo
				"NL31INGB0000000044",// IBAN
				"INGBNL2A", // BIC
				new BigDecimal("100"), // amount
				eur);
		createSEPAExportLine(sepaExport, "002", // SEPA_MandateRefNo
				"NL31INGB0000000044", // IBAN
				"INGBNL2A",// BIC
				new BigDecimal("30"), // amount
				eur);

		createSEPAExportLine(sepaExport, "002", // SEPA_MandateRefNo
				"NL31INGB0000000044", // IBAN
				"INGBNL2A",// BIC
				new BigDecimal("40"), // amount
				chf);

		xmlDocument = xmlGenerator.createDocument(sepaExport);

		assertThat(xmlDocument.getCstmrCdtTrfInitn().getGrpHdr().getCtrlSum()).isEqualByComparingTo("170");
		assertThat(xmlDocument.getCstmrCdtTrfInitn().getGrpHdr().getNbOfTxs()).isEqualTo("3"); // needs to be 3, no matter wheter we do batch or not.
		assertThat(xmlDocument.getCstmrCdtTrfInitn().getGrpHdr().getInitgPty().getNm()).isEqualTo(sepaExport.getSEPA_CreditorIdentifier());

		// if no batch, it would be 3..
		// assertThat(xmlDocument.getCstmrCdtTrfInitn().getPmtInf()).hasSize(3);

		assertThat(xmlDocument.getCstmrCdtTrfInitn().getPmtInf()).hasSize(2);
		assertThat(xmlDocument.getCstmrCdtTrfInitn().getPmtInf()).allSatisfy(pmtInf -> assertThat(pmtInf.isBtchBookg()).isTrue());

		assertThat(xmlDocument.getCstmrCdtTrfInitn().getPmtInf()).hasSize(2);
	}

	private I_SEPA_Export createSEPAExport(
			final String SEPA_CreditorIdentifier,
			final String bic)
	{
		final I_SEPA_Export sepaExport = newInstance(I_SEPA_Export.class);
		sepaExport.setSEPA_CreditorIdentifier(SEPA_CreditorIdentifier);
		sepaExport.setSwiftCode(bic);
		sepaExport.setIsExportBatchBookings(true);
		save(sepaExport);

		return sepaExport;
	}

	private I_SEPA_Export_Line createSEPAExportLine(
			final I_SEPA_Export sepaExport,
			final String SEPA_MandateRefNo,
			final String iban,
			final String bic,
			final BigDecimal amt,
			final CurrencyId currencyId)
	{

		final I_C_BP_BankAccount bankAccount = newInstance(I_C_BP_BankAccount.class);
		bankAccount.setC_Currency_ID(currencyId.getRepoId());
		bankAccount.setIBAN(iban);
		bankAccount.setIsEsrAccount(true);
		bankAccount.setA_Name("bankAccount.A_Name");
		save(bankAccount);

		final I_SEPA_Export_Line line = newInstance(I_SEPA_Export_Line.class);
		line.setIBAN(iban);
		line.setSwiftCode(bic);
		line.setAmt(amt);
		line.setC_Currency_ID(currencyId.getRepoId());
		line.setSEPA_MandateRefNo(SEPA_MandateRefNo);

		line.setC_BP_BankAccount(bankAccount);
		line.setSEPA_Export(sepaExport);
		line.setIsActive(true);
		line.setIsError(false);
		save(line);

		return line;
	}

	@Test
	public void testReplaceForbiddenChars()
	{
		assertReplaceForbiddenCharsWorks("(1020739<-) | (1026@313<-)", "(1020739<-) _ (1026@313<-)");
		assertReplaceForbiddenCharsWorks("(1020739&lt;-) | (1026313&lt;-)", "(1020739&lt;-) _ (1026313&lt;-)");
		assertReplaceForbiddenCharsWorks("(1020739&lt;-) - (1026313&lt;-)", "(1020739&lt;-) - (1026313&lt;-)");
	}

	private void assertReplaceForbiddenCharsWorks(String input, String expected)
	{
		String output = SEPACustomerCTIMarshaler_Pain_001_001_03_CH_02.replaceForbiddenChars(input);
		assertThat(output).isEqualTo(expected);
	}

}
