package de.metas.dpd.service.impl;

/*
 * #%L
 * de.metas.swat.base
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


import java.awt.Font;
import java.awt.FontFormatException;
import java.awt.GraphicsEnvironment;
import java.io.IOException;
import java.math.BigDecimal;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import javax.print.attribute.HashPrintRequestAttributeSet;
import javax.print.attribute.HashPrintServiceAttributeSet;
import javax.print.attribute.PrintRequestAttributeSet;
import javax.print.attribute.PrintServiceAttributeSet;
import javax.print.attribute.standard.Copies;
import javax.print.attribute.standard.PrinterName;

import org.adempiere.ad.trx.api.ITrx;
import org.adempiere.exceptions.AdempiereException;
import org.adempiere.model.I_M_PackageInfo;
import org.adempiere.model.I_M_PackagingContainer;
import org.adempiere.model.InterfaceWrapperHelper;
import org.compiere.model.I_C_Location;
import org.compiere.model.I_M_Package;
import org.compiere.model.I_M_Shipper;
import org.compiere.model.MSysConfig;
import org.compiere.model.MUOM;
import org.compiere.model.Query;
import org.compiere.util.Env;
import org.compiere.util.Trx;
import org.slf4j.Logger;

import de.metas.adempiere.service.IPrinterRoutingBL;
import de.metas.adempiere.service.IPrintingService;
import de.metas.document.sequence.IDocumentNoBuilderFactory;
import de.metas.dpd.exception.DPDException;
import de.metas.dpd.model.MDPDCountry;
import de.metas.dpd.model.MDPDDepot;
import de.metas.dpd.model.MDPDFileInfo;
import de.metas.dpd.model.MDPDRoute;
import de.metas.dpd.model.MDPDService;
import de.metas.dpd.model.MDPDServiceInfo;
import de.metas.dpd.service.IDPDRoutingService;
import de.metas.dpd.service.Iso7064;
import de.metas.dpd.service.RoutingQuery;
import de.metas.dpd.service.RoutingResult;
import de.metas.i18n.Msg;
import de.metas.inout.model.I_M_InOut;
import de.metas.logging.LogManager;
import de.metas.process.IADProcessDAO;
import de.metas.process.ProcessInfo;
import de.metas.report.jasper.client.JRClient;
import de.metas.shipping.model.I_M_ShippingPackage;
import de.metas.shipping.model.X_M_ShippingPackage;
import de.metas.util.Check;
import de.metas.util.Services;
import net.sf.jasperreports.engine.JRException;
import net.sf.jasperreports.engine.JRExporterParameter;
import net.sf.jasperreports.engine.JasperPrint;
import net.sf.jasperreports.engine.export.JRPrintServiceExporter;
import net.sf.jasperreports.engine.export.JRPrintServiceExporterParameter;

/**
 *
 * @author ts
 * @see "<a href='http://dewiki908/mediawiki/index.php/Transportverpackung_%282009_0022_G61%29'>(2009_0022_G61)</a>"
 */
public class DPDRoutingService implements IDPDRoutingService
{

	private static final String AD_SYSCONFIG_DPD_INDENTIFIER = "DPD-Indentifier";

	public static final String SEQ_PACKAGENO_TABLENAME = "DPD_PackageNumber";

	public static final String AD_SYSCONFIG_DPD_VERSANDDEPOT = "DPD-Versanddepot";

	public static final String AD_MESSAGE_DPDFILE = "de.metas.dpd.service.RoutingService.retrieveData";
	public static final String AD_MESSAGE_DPDDepot = "de.metas.dpd.service.RoutingService.retrieveData_DPDDepot";
	public static final String AD_MESSAGE_DPDService = "de.metas.dpd.service.RoutingService.retrieveData_DPDService";

	public static final String DPD_SERVICE_STANDARD = "101";
	public static final String DPD_SERVICE_KLEINPAKET = "136";

	// needs to excecute "de/metas/docs/sales/dpdlabel/report.jasper"
	public static final String JasperProcess_Label = "DPD_Label_InOut";
	public static final String JasperProcess_Package = "DPD_Label_Package";

	// public static final String REPORT_RESOURCE = "de/metas/docs/sales/dpdlabel/report.jasper";
	// public static final String REPORT_RESOURCE1 = "de/metas/docs/sales/dpdlabel/report1.jasper";

	private static final Logger logger = LogManager.getLogger(DPDRoutingService.class);

	static
	{

		// Load and register Crystal font
		try
		{
			final Font crystal = Font.createFont(Font.TRUETYPE_FONT,
					DPDRoutingService.class.getResourceAsStream("/CRYSRG__.TTF"));
			GraphicsEnvironment.getLocalGraphicsEnvironment().registerFont(
					crystal);
			logger.info("Registered font " + crystal);

		}
		catch (FontFormatException e1)
		{
			throw new AdempiereException(e1);
		}
		catch (IOException e1)
		{
			throw new AdempiereException(e1);
		}
	}

	@Override
	public RoutingResult retrieveData(final Properties ctx, final RoutingQuery query, final String trxName)
	{
		validateQuery(query);

		final MDPDFileInfo routeFileInfo =
				MDPDFileInfo.retrieve(ctx, "ROUTES", query.date, trxName);
		String msg = Msg.getMsg(ctx, AD_MESSAGE_DPDFILE, new Object[]{"ROUTES"});
		Check.assumeNotNull(routeFileInfo, msg, query.date);

		final MDPDRoute route =
				MDPDRoute.retrieve(ctx, query.dCountry, query.dPostCode, query.service, query.rDepot, routeFileInfo.get_ID(), query.date, trxName);

		msg = Msg.getMsg(ctx, AD_MESSAGE_DPDFILE, new Object[]{"ROUTE"});
		Check.assumeNotNull(route, msg, query.date);

		final MDPDFileInfo serviceFileInfo =
				MDPDFileInfo.retrieve(ctx, "SERVICE", query.date, trxName);
		msg = Msg.getMsg(ctx, AD_MESSAGE_DPDFILE, new Object[]{"SERVICE"});
		Check.assumeNotNull(serviceFileInfo, msg, query.date);

		final MDPDService service =	MDPDService.retrieve(ctx, query.service, serviceFileInfo.get_ID(), trxName);
		msg = Msg.getMsg(ctx, AD_MESSAGE_DPDService, new Object[]{query.service, serviceFileInfo.get_ID()});
		Check.assumeNotNull(service, msg, query.date);

		final MDPDFileInfo serviceInfoFileInfo =
				MDPDFileInfo.retrieve(ctx, "SERVICEINFO.DE", query.date, trxName);
		msg = Msg.getMsg(ctx, AD_MESSAGE_DPDFILE, new Object[]{"SERVICEINFO.DE"});
		Check.assumeNotNull(serviceInfoFileInfo, msg, query.date);

		final MDPDServiceInfo serviceInfo =
				MDPDServiceInfo.retrieve(ctx, query.service, serviceInfoFileInfo.get_ID(), trxName);

		final MDPDFileInfo depotsFileInfo =
				MDPDFileInfo.retrieve(ctx, "DEPOTS", query.date, trxName);
		msg = Msg.getMsg(ctx, AD_MESSAGE_DPDFILE, new Object[]{"DEPOTS"});
		Check.assumeNotNull(depotsFileInfo, msg, query.date);

		final MDPDDepot depot = MDPDDepot.retrieve(ctx, route.getD_Depot(), depotsFileInfo.get_ID(), trxName);
		msg = Msg.getMsg(ctx, AD_MESSAGE_DPDService, new Object[]{route.getD_Depot(), depotsFileInfo.get_ID()});
		Check.assumeNotNull(depot, msg, query.date);

		final String countryCode;
		if (depot.getCountryCode() == null)
		{
			countryCode = query.dCountry;
		}
		else
		{
			countryCode = depot.getCountryCode();
		}

		final MDPDFileInfo countryFileInfo = MDPDFileInfo.retrieve(ctx, "COUNTRY", query.date, trxName);
		msg = Msg.getMsg(ctx, AD_MESSAGE_DPDFILE, new Object[]{"COUNTRY"});
		Check.assumeNotNull(countryFileInfo, msg, query.date);

		final MDPDCountry country = MDPDCountry.retrieve(ctx, countryCode, countryFileInfo.get_ID(), trxName);

		final String groupingPriority = mkEmptyIfNull(route.getGroupingPriority());

		// we don't want the string "null" in DB
		final String lotDAttachment = mkEmptyIfNull(depot.getIATALikeCode()) + groupingPriority;

		final String dSort = route.getD_Sort();

		final String serviceMark = service.getServiceMark();

		final String serviceText = service.getServiceText();

		final String serviceFieldInfo;
		if (serviceInfo != null)
		{
			serviceFieldInfo = serviceInfo.getServiceFieldInfo();
		}
		else
		{
			serviceFieldInfo = null;
		}

		final String oSort = route.getO_Sort();

		return new RoutingResult(
				countryCode,
				serviceText,
				route.getD_Depot(),
				lotDAttachment,
				serviceMark,
				serviceFieldInfo,
				oSort,
				dSort,
				groupingPriority,
				route.getBarcodeID(),
				routeFileInfo.getVersion(),
				country.getNumCountryCode());
	}

	private String mkEmptyIfNull(final String str)
	{

		if (str == null)
		{
			return "";
		}
		return str;
	}

	private void validateQuery(final RoutingQuery query)
	{

		final Map<String, String> invalidFields = new HashMap<>();

		if (Check.isEmpty(query.dPostCode))
		{
			invalidFields.put("D-Postcode", query.dPostCode);
		}

		if (!invalidFields.isEmpty())
		{
			throw DPDException.invalidQuery(query, invalidFields);
		}

	}

	public RoutingQuery mkRoutingQuery(
			final Properties ctx,
			final I_M_InOut inOut,
			final I_M_Package pack,
			final String serviceOverride)
	{

		final I_C_Location loc = inOut.getC_BPartner_Location().getC_Location();

		final String rDepot = MSysConfig.getValue(
				AD_SYSCONFIG_DPD_VERSANDDEPOT, inOut.getAD_Client_ID(), inOut.getAD_Org_ID());

		final Timestamp date = Env.getContextAsDate(ctx, "#Date");

		final String dCountry = loc.getC_Country().getCountryCode();

		final String postCode = loc.getPostal();

		final String area = loc.getRegionName();

		final String city = loc.getCity();

		final String service;
		if (serviceOverride == null)
		{
			service = getServiceCode(pack);
		}
		else
		{
			service = serviceOverride;
		}
		return new RoutingQuery(rDepot, date, service, dCountry, postCode,				area, city);
	}

	private String getServiceCode(final I_M_Package pack)
	{
		final de.metas.shipping.interfaces.I_M_Package packExt = InterfaceWrapperHelper.create(pack, de.metas.shipping.interfaces.I_M_Package.class);
		final BigDecimal weight = packExt.getPackageWeight();

		if (weight == null || BigDecimal.ZERO.compareTo(weight) == 0)
		{
			// TODO -> AD_Message
			throw DPDException.invalidPackage(pack, "Gewicht fehlt");
		}
		if (new BigDecimal("3").compareTo(weight) < 0)
		{
			// package is heavier than 3 kg
			return DPD_SERVICE_STANDARD;
		}

		final int containerId = packExt.getM_PackagingContainer_ID();
		if (containerId <= 0)
		{
			throw DPDException.invalidPackage(pack, "Verpackungsabmessungen fehlen");
		}

		final I_M_PackagingContainer container = InterfaceWrapperHelper.newInstance(I_M_PackagingContainer.class);
		BigDecimal lengthAmt = container.getLength();
		BigDecimal widthAmt = container.getWidth();
		BigDecimal heightAmt = container.getHeight();

		final MUOM uom = (MUOM)container.getC_UOM_Length();
		if (uom.getX12DE355().equals("mm"))
		{
			lengthAmt = lengthAmt.divide(BigDecimal.TEN);
			widthAmt = widthAmt.divide(BigDecimal.TEN);
			heightAmt = heightAmt.divide(BigDecimal.TEN);

		}
		else
		{
			// TODO -> AD_Messsage
			throw DPDException.invalidPackage(pack,
					"Einheit der Paketabmessungen ist unbekannt: "
							+ uom.getX12DE355());
		}

		if (new BigDecimal("50").compareTo(lengthAmt) < 0)
		{
			// package is too long
			return DPD_SERVICE_STANDARD;
		}

		final BigDecimal combinedLengthAndGirth = widthAmt.add(heightAmt)
				.multiply(new BigDecimal("2")).add(lengthAmt);

		if (new BigDecimal("111").compareTo(combinedLengthAndGirth) < 0)
		{
			// package is too big
			return DPD_SERVICE_STANDARD;
		}
		return DPD_SERVICE_KLEINPAKET;
	}

	@Override
	public void createPackageInfo(
			final I_M_Package pack,
			final RoutingResult result,
			final RoutingQuery query)
	{

		final Properties ctx = Env.getCtx();
		final String trxName = ITrx.TRXNAME_ThreadInherited;

		final String trackingNo = mkTrackingNo(ctx, trxName);

		pack.setTrackingInfo(trackingNo + Iso7064.compute(trackingNo));
		InterfaceWrapperHelper.save(pack);

		final I_M_PackageInfo packInf =  InterfaceWrapperHelper.newInstance(I_M_PackageInfo.class);
		packInf.setM_Package_ID(pack.getM_Package_ID());

		final String barcodeInfo = mkBarcodeInfo(result, query, trackingNo);
		final char barcodeId = (char)Integer.parseInt(result.barcodeId);

		final StringBuffer routingtext = new StringBuffer();
		routingtext.append(result.country);
		routingtext.append("-");
		routingtext.append(result.dDepot);
		if (!Check.isEmpty(result.lotDAcheminement))
		{
			routingtext.append("-");
			routingtext.append(result.lotDAcheminement);
		}

		packInf.setServiceValue(query.service);
		packInf.setD_Sort(result.dSort);
		packInf.setO_Sort(result.oSort);
		packInf.setRoutingText(routingtext.toString());
		packInf.setServiceFieldInfo(result.serviceInfo);
		packInf.setServiceMark(result.serviceMark);
		packInf.setServiceText(result.serviceText);
		packInf.setBarcodeInfo(barcodeId + barcodeInfo
				+ Iso7064.compute(barcodeInfo));
		packInf.setVersion(result.routingVersion);
		InterfaceWrapperHelper.save(packInf);
	}

	private String mkBarcodeInfo(final RoutingResult result,
			final RoutingQuery query, final String trackingNo)
	{
		final StringBuffer barcodeInfoSB = new StringBuffer();

		for (int i = 0; i < 7 - query.dPostCode.length(); i++)
		{
			barcodeInfoSB.append('0');
		}
		barcodeInfoSB.append(query.dPostCode);
		barcodeInfoSB.append(' ');
		barcodeInfoSB.append(trackingNo);
		barcodeInfoSB.append(' ');
		barcodeInfoSB.append(query.service);
		barcodeInfoSB.append(' ');

		final String strCountryCode = Integer.toString(result.numCountryCode);
		for (int i = 0; i < 3 - strCountryCode.length(); i++)
		{
			barcodeInfoSB.append('0');
		}
		barcodeInfoSB.append(strCountryCode);
		barcodeInfoSB.append(' ');

		final String barcodeInfo = barcodeInfoSB.toString();
		return barcodeInfo;
	}

	private String mkTrackingNo(final Properties ctx, final String trxName)
	{

		final StringBuffer trackingNo = new StringBuffer();

		// DDDD
		final String routingDepot = MSysConfig.getValue(
				AD_SYSCONFIG_DPD_VERSANDDEPOT, Env.getAD_Client_ID(ctx), Env
						.getAD_Org_ID(ctx));

		trackingNo.append(routingDepot);

		trackingNo.append(' ');

		// DPD Identifier X1 and X2
		trackingNo.append(MSysConfig.getValue(AD_SYSCONFIG_DPD_INDENTIFIER, Env
				.getAD_Client_ID(ctx), Env.getAD_Org_ID(ctx)));

		// TT TTTT TT

		// FIXME: tsa: SEQ_PACKAGENO_TABLENAME table does not exist in !?!?
		final IDocumentNoBuilderFactory documentNoFactory = Services.get(IDocumentNoBuilderFactory.class);
		final String packageNo = documentNoFactory.forTableName(SEQ_PACKAGENO_TABLENAME, Env.getAD_Client_ID(ctx), Env.getAD_Org_ID(ctx))
				.build();
		assert packageNo.length() == 8;

		StringBuffer sb = new StringBuffer();
		sb.append(packageNo.substring(0, 2));
		sb.append(' ');
		sb.append(packageNo.substring(2, 6));
		sb.append(' ');
		sb.append(packageNo.substring(6));

		trackingNo.append(sb.toString());

		return trackingNo.toString();
	}

	/**
	 *
	 * @param inOut
	 * @param serviceOverride
	 * @param packg
	 *            this is the parmater from table M_Package; if the process is started from M_InOut, this will be null
	 */
	private void createPackageInfos(
			final Properties ctx,
			final I_M_InOut inOut,
			final String serviceOverride,
			final I_M_Package packg,
			final String trxName)
	{

		if (packg != null)
		{
			createPackageInfo(ctx, packg, inOut, serviceOverride, trxName);
			return;
		}

		final Collection<I_M_Package> packages = retrievePackagesForInOut(ctx, inOut, trxName);

		if (packages.isEmpty())
		{
			// TODO: -> AD_Message
			throw DPDException.invalidInOut(ctx, inOut, "Lieferschein hat keine Packstuecke");
		}

		for (final I_M_Package pack : packages)
		{
			createPackageInfo(ctx, pack, inOut, serviceOverride, trxName);
		}
	}

	@Override
	public void createPackageInfo(
			final Properties ctx,
			final I_M_Package pack,
			final I_M_InOut inOut,
			final String serviceOverride,
			final String trxName)
	{

		final String shipperName = pack.getM_Shipper().getName();
		if (!"DPD".equals(shipperName))
		{
			return;
		}

		final boolean existingInfo = !retrievePackageInfosForPackage(pack).isEmpty();
		if (existingInfo)
		{
			return;
		}
		final RoutingQuery query = mkRoutingQuery(ctx, inOut, pack, serviceOverride);

		final RoutingResult result = retrieveData(ctx, query, trxName);
		createPackageInfo(pack, result, query);
	}

	@Override
	public void createLabel(
			final Properties ctx,
			final I_M_InOut inOut,
			final String serviceOverride,
			I_M_Package pack,
			final String trxName)
	{

		createPackageInfos(ctx, inOut, serviceOverride, pack, trxName);
		try
		{
			Trx.get(trxName, false).commit(true);
		}
		catch (SQLException e)
		{
			throw new AdempiereException(e);
		}
	}

	@Override
	public boolean printLabel(
			final Properties ctx,
			final I_M_InOut inOut,
			I_M_Package pack,
			BigDecimal M_Shipper_ID,
			final String trxName)
	{
		return printPackageLabel(ctx, inOut, pack, M_Shipper_ID, trxName);
	}

	private JasperPrint retrieveJasperPrint(final Properties ctx, final String processValue, final int adTableId, final int recordId, final String trxName)
	{
		final ProcessInfo pi = ProcessInfo.builder()
				.setCtx(ctx)
				.setAD_ProcessByValue(processValue)
				.setRecord(adTableId, recordId)
				.build();
		return JRClient.get().createJasperPrint(pi);
	}

	public boolean printPackageLabel(
			final Properties ctx,
			final I_M_InOut inOut,
			final I_M_Package pack,
			final BigDecimal M_Shipper_ID,
			final String trxName) throws AdempiereException
	{
		final String shipperName;

		if (pack == null)
		{
			if (M_Shipper_ID == null)
			{
				shipperName = inOut.getM_Shipper().getName();
			}
			else
			{
				final I_M_Shipper shipper = InterfaceWrapperHelper.create(ctx, M_Shipper_ID.intValue(), I_M_Shipper.class, ITrx.TRXNAME_None);
				shipperName = shipper.getName();
			}
		}
		else
		{
			shipperName = pack.getM_Shipper().getName();
		}
		if (!"DPD".equals(shipperName))
		{
			return false;
		}

		final JasperPrint jasperPrint;
		final String jasperProcessValue;
		if (pack == null)
		{
			jasperProcessValue = JasperProcess_Label;
			final int adTableId = InterfaceWrapperHelper.getModelTableId(inOut);
			jasperPrint = retrieveJasperPrint(ctx, jasperProcessValue, adTableId, inOut.getM_InOut_ID(), trxName);
		}
		else
		{
			jasperProcessValue = JasperProcess_Package;
			final int adTableId = InterfaceWrapperHelper.getModelTableId(pack);
			jasperPrint = retrieveJasperPrint(ctx, jasperProcessValue, adTableId, pack.getM_Package_ID(), trxName);
		}

		if (jasperPrint == null)
		{
			throw new AdempiereException("Unable to retrieve DPD label");
		}

		final JRPrintServiceExporter printExporter = new JRPrintServiceExporter();
		printExporter.setParameter(JRExporterParameter.JASPER_PRINT, jasperPrint);

		final PrintRequestAttributeSet printRequestAttributeSet = new HashPrintRequestAttributeSet();
		printRequestAttributeSet.add(new Copies(1));

		printExporter.setParameter(JRPrintServiceExporterParameter.PRINT_REQUEST_ATTRIBUTE_SET, printRequestAttributeSet);

		final IPrintingService printingService = Services.get(IPrinterRoutingBL.class).findPrintingService(
				ctx,
				-1, // C_DocType_ID
				Services.get(IADProcessDAO.class).retriveProcessIdByValue(jasperProcessValue), // AD_Process_ID
				IPrinterRoutingBL.PRINTERTYPE_Label);

		final PrintServiceAttributeSet printServiceAttributeSet = new HashPrintServiceAttributeSet();
		printServiceAttributeSet.add(new PrinterName(printingService.getPrinterName(), null));

		printExporter.setParameter(JRPrintServiceExporterParameter.PRINT_SERVICE_ATTRIBUTE_SET, printServiceAttributeSet);
		printExporter.setParameter(JRPrintServiceExporterParameter.DISPLAY_PRINT_DIALOG, !printingService.isDirectPrint());
		printExporter.setParameter(JRPrintServiceExporterParameter.DISPLAY_PAGE_DIALOG, Boolean.FALSE);

		try
		{
			printExporter.exportReport();
		}
		catch (final JRException e)
		{
			final String msg = "Error printing the report - " + e.getMessage();
			throw new AdempiereException(msg, e);
		}

		return true;
	}

	@Override
	public void discardLabel(
			final Properties ctx,
			final I_M_InOut inOut, final I_M_Package packg, final BigDecimal M_Shipper_ID,
			final String trxName)
	{
		if (packg != null)
		{
			String docNo = fetchgetShipperTransportationDoc(packg);
			if (!Check.isEmpty(docNo, true))
			{
				String msg = Msg.getMsg(Env.getCtx(), "PackageWithOrder", new Object[] { docNo });
				throw new AdempiereException(msg);
			}
			packg.setTrackingInfo(null);
			if (M_Shipper_ID != null)
			{
				packg.setM_Shipper_ID(M_Shipper_ID.intValue());
			}
			InterfaceWrapperHelper.save(packg);

			for (final I_M_PackageInfo packageInfo : retrievePackageInfosForPackage(packg))
			{
				InterfaceWrapperHelper.delete(packageInfo);
			}
			return;
		}

		for (final I_M_Package pack : retrievePackagesForInOut(ctx, inOut, trxName))
		{
			String docNo = fetchgetShipperTransportationDoc(pack);
			if (!Check.isEmpty(docNo, true))
			{
				String msg = Msg.getMsg(Env.getCtx(), "PackageWithOrder", new Object[] { docNo });
				throw new AdempiereException(msg);
			}
			pack.setTrackingInfo(null);
			if (M_Shipper_ID != null)
			{
				pack.setM_Shipper_ID(M_Shipper_ID.intValue());
			}
			InterfaceWrapperHelper.save(pack);

			for (final I_M_PackageInfo packageInfo : retrievePackageInfosForPackage(pack))
			{
				InterfaceWrapperHelper.delete(packageInfo);
			}
		}
	}

	private String fetchgetShipperTransportationDoc(final I_M_Package pack)
	{
		String whereClause = I_M_ShippingPackage.COLUMNNAME_M_Package_ID + " = ? ";
		X_M_ShippingPackage sp = new Query(Env.getCtx(), I_M_ShippingPackage.Table_Name, whereClause, ITrx.TRXNAME_ThreadInherited)
									.setParameters(pack.getM_Package_ID())
									.setOrderBy(I_M_ShippingPackage.COLUMNNAME_M_InOut_ID)
									.first();
		if (sp != null)
			return sp.getM_ShipperTransportation().getDocumentNo();
		return null;
	}

	private List<I_M_Package> retrievePackagesForInOut(
			final Properties ctx,
			final I_M_InOut inOut,
			final String trxName)
	{
		final String whereClause = I_M_Package.COLUMNNAME_M_InOut_ID + "=?";

		return new Query(ctx, I_M_Package.Table_Name, whereClause, trxName)
				.setParameters(inOut.getM_InOut_ID())
				.setOnlyActiveRecords(true)
				.setClient_ID()
				.setOrderBy(I_M_Package.COLUMNNAME_M_Package_ID)
				.list(I_M_Package.class);
	}

	private static Collection<I_M_PackageInfo> retrievePackageInfosForPackage(final I_M_Package pack) {

		final String whereClause = I_M_PackageInfo.COLUMNNAME_M_Package_ID + "=?";
		final Object[] parameters = { pack.getM_Package_ID() };

		Properties ctx = InterfaceWrapperHelper.getCtx(pack);
		String trxName = InterfaceWrapperHelper.getTrxName(pack);
		return new Query(ctx, I_M_PackageInfo.Table_Name, whereClause, trxName).setParameters(parameters).list(I_M_PackageInfo.class);
	}

}
