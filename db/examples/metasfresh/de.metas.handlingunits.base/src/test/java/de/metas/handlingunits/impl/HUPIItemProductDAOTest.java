package de.metas.handlingunits.impl;

import static org.hamcrest.Matchers.is;
import static org.junit.Assert.assertThat;

import java.math.BigDecimal;
import java.time.LocalDate;
import java.time.Month;
import java.time.ZonedDateTime;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.List;

import org.adempiere.ad.model.util.ModelByIdComparator;
import org.adempiere.ad.trx.api.ITrx;
import org.adempiere.ad.wrapper.POJOWrapper;
import org.adempiere.model.InterfaceWrapperHelper;
import org.adempiere.test.AdempiereTestHelper;
import org.adempiere.test.AdempiereTestWatcher;
import org.compiere.model.I_C_BPartner;
import org.compiere.util.Env;
import org.compiere.util.TimeUtil;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestWatcher;

import de.metas.adempiere.model.I_M_Product;
import de.metas.bpartner.BPartnerId;
import de.metas.bpartner.service.IBPartnerDAO;
import de.metas.handlingunits.IHUPIItemProductDAO;
import de.metas.handlingunits.model.I_M_HU_PI;
import de.metas.handlingunits.model.I_M_HU_PI_Item;
import de.metas.handlingunits.model.I_M_HU_PI_Item_Product;
import de.metas.handlingunits.model.I_M_HU_PI_Version;
import de.metas.handlingunits.model.I_M_HU_PackingMaterial;
import de.metas.handlingunits.model.X_M_HU_PI_Item;
import de.metas.handlingunits.model.X_M_HU_PI_Version;
import de.metas.product.IProductDAO;
import de.metas.product.ProductId;
import de.metas.util.Services;
import de.metas.util.time.SystemTime;

public class HUPIItemProductDAOTest
{
	private HUPIItemProductDAO dao;
	private ProductId product1;
	private ProductId product2;

	private ProductId packagingProduct1;
	private ProductId packagingProduct2;

	private BPartnerId bpartner1;
	private BPartnerId bpartner2;
	private final BPartnerId bpartner_NULL = null;
	// private I_C_BPartner bpartner3;
	// private I_M_HU_PI_Item piItem1;
	private ZonedDateTime date1;
	private ZonedDateTime date2;
	private ZonedDateTime date3;
	private ZonedDateTime date4;

	/** Watches current test and dumps the database to console in case of failure */
	@Rule
	public final TestWatcher testWatcher = new AdempiereTestWatcher();

	@Before
	public void init()
	{
		AdempiereTestHelper.get().init();

		dao = (HUPIItemProductDAO)Services.get(IHUPIItemProductDAO.class);
		product1 = createProduct("p1");
		product2 = createProduct("p2");

		packagingProduct1 = createProduct("pp1");
		packagingProduct2 = createProduct("pp2");

		bpartner1 = BPartnerId.ofRepoId(createBPartner("bp1").getC_BPartner_ID());
		bpartner2 = BPartnerId.ofRepoId(createBPartner("bp2").getC_BPartner_ID());
		// bpartner3 = createBPartner("bp3");
		// piItem1 = createM_HU_PI_Item();
		date1 = LocalDate.of(2011, Month.OCTOBER, 01).atStartOfDay(SystemTime.zoneId());
		date2 = LocalDate.of(2012, Month.OCTOBER, 01).atStartOfDay(SystemTime.zoneId());
		date3 = LocalDate.of(2013, Month.OCTOBER, 01).atStartOfDay(SystemTime.zoneId());
		date4 = LocalDate.of(2014, Month.OCTOBER, 01).atStartOfDay(SystemTime.zoneId());
	}

	@Test
	public void test_createQueryOrderByBuilder()
	{
		final Comparator<Object> orderBy = dao.createQueryOrderByBuilder(null)
				.createQueryOrderBy()
				.getComparator();

		final List<I_M_HU_PI_Item_Product> itemProducts = Arrays.asList(
				createM_HU_PI_Item_Product(product1, null, date1, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit), createM_HU_PI_Item_Product(product1, bpartner1, date3, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit));

		final List<I_M_HU_PI_Item_Product> itemProductsOrdered = new ArrayList<>(itemProducts);
		Collections.sort(itemProductsOrdered, orderBy);

		assertEqualsByDescription("Invalid item at index=1", itemProducts.get(1), itemProductsOrdered.get(0));
		assertEqualsByDescription("Invalid item at index=2", itemProducts.get(0), itemProductsOrdered.get(1));
	}

	@Test
	public void test_retrieveMaterialItemProduct_product_anyBPartner_date()
	{
		final I_M_HU_PI_Item_Product[] itemProducts = new I_M_HU_PI_Item_Product[] {
				/* 0 */createM_HU_PI_Item_Product(product1, null, date1, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit),
				/* 1 */createM_HU_PI_Item_Product(product1, bpartner1, date3, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit),
		};

		test_retrieveMaterialItemProduct_product_bpartner_date(itemProducts[0], product1, bpartner1, date1, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
		test_retrieveMaterialItemProduct_product_bpartner_date(itemProducts[0], product1, bpartner1, date2, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
		test_retrieveMaterialItemProduct_product_bpartner_date(itemProducts[1], product1, bpartner1, date3, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);

		test_retrieveMaterialItemProduct_product_bpartner_date(itemProducts[0], product1, bpartner2, date1, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
		test_retrieveMaterialItemProduct_product_bpartner_date(itemProducts[0], product1, bpartner2, date2, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
		test_retrieveMaterialItemProduct_product_bpartner_date(itemProducts[0], product1, bpartner2, date3, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
	}

	@Test
	public void test_retrieveMaterialItemProduct_anyProduct_bpartner_date()
	{
		final I_M_HU_PI_Item_Product[] itemProducts = new I_M_HU_PI_Item_Product[] {
				/* 0 */createM_HU_PI_Item_Product(product1, null, date1, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit),
				/* 1 */createM_HU_PI_Item_Product(null, null, date3, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit),
		};

		test_retrieveMaterialItemProduct_product_bpartner_date(itemProducts[0], product1, bpartner1, date1, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
		test_retrieveMaterialItemProduct_product_bpartner_date(itemProducts[0], product1, bpartner1, date2, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
		test_retrieveMaterialItemProduct_product_bpartner_date(itemProducts[0], product1, bpartner1, date3, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);

		test_retrieveMaterialItemProduct_product_bpartner_date(null, product2, bpartner1, date1, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
		test_retrieveMaterialItemProduct_product_bpartner_date(null, product2, bpartner1, date2, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
		test_retrieveMaterialItemProduct_product_bpartner_date(null, product2, bpartner1, date3, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit); // 06566: don't pull the "anyProduct"
		// I_M_HU_PI_Item_Product, because product2 has no
		// assignment
		test_retrieveMaterialItemProduct_product_bpartner_date(null, product2, bpartner1, date4, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit); // 06566: don't pull the "anyProduct"
	}

	/**
	 * Tests {@link HUPIItemProductDAO#retrieveMaterialItemProduct(org.compiere.model.I_M_Product, I_C_BPartner, Date, String, boolean, org.compiere.model.I_M_Product)}, i.e that one that also takes the packagacking product into account.
	 *
	 * @task https://metasfresh.atlassian.net/browse/FRESH-386
	 */
	@Test
	public void test_retrieveMaterialItemProductbyPackagingProduct()
	{
		final String huUnitType = X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit;

		final I_M_HU_PI_Item_Product piip1 = createM_HU_PI_Item_Product(product1, bpartner1, date1, huUnitType);
		assertThat(piip1.getM_HU_PI_Item().getItemType(), is(X_M_HU_PI_Item.ITEMTYPE_Material)); // guard
		addPackingmaterialToItem(packagingProduct1, piip1.getM_HU_PI_Item());
		POJOWrapper.setInstanceName(piip1, "piip1");

		final I_M_HU_PI_Item_Product piip2 = createM_HU_PI_Item_Product(product1, bpartner1, date1, huUnitType);
		assertThat(piip1.getM_HU_PI_Item().getItemType(), is(X_M_HU_PI_Item.ITEMTYPE_Material)); // guard
		addPackingmaterialToItem(packagingProduct2, piip2.getM_HU_PI_Item());
		POJOWrapper.setInstanceName(piip2, "piip2");

		final boolean allowInfiniteCapacity = true;

		assertThat(dao.retrieveMaterialItemProduct(product1, bpartner1, date1, huUnitType, allowInfiniteCapacity, packagingProduct1),
				is(piip1));
		assertThat(dao.retrieveMaterialItemProduct(product1, bpartner1, date1, huUnitType, allowInfiniteCapacity, packagingProduct2),
				is(piip2));
	}

	private void test_retrieveMaterialItemProduct_product_bpartner_date(
			final I_M_HU_PI_Item_Product expected,
			final ProductId productId,
			final BPartnerId bpartnerId,
			final ZonedDateTime date,
			final String huUnitType)
	{
		final boolean allowInfiniteCapacity = true;
		final I_M_HU_PI_Item_Product actual = dao.retrieveMaterialItemProduct(productId, bpartnerId, date, huUnitType, allowInfiniteCapacity);
		final String message = "Invalid for product=" + productId + ", bpartner=" + bpartnerId + ", date=" + date;
		assertEqualsByDescription(message, expected, actual);
	}

	private void assertEqualsByDescription(final String message, final I_M_HU_PI_Item_Product expected, final I_M_HU_PI_Item_Product actual)
	{
		final String expectedDescription = expected == null ? null : expected.getDescription();
		final String actualDescription = actual == null ? null : actual.getDescription();
		Assert.assertEquals(message, expectedDescription, actualDescription);
	}

	private ProductId createProduct(final String value)
	{
		final I_M_Product p = InterfaceWrapperHelper.create(Env.getCtx(), I_M_Product.class, ITrx.TRXNAME_None);
		p.setValue(value);
		p.setName(value);
		InterfaceWrapperHelper.save(p);

		return ProductId.ofRepoId(p.getM_Product_ID());
	}

	private I_C_BPartner createBPartner(final String value)
	{
		final I_C_BPartner bp = InterfaceWrapperHelper.create(Env.getCtx(), I_C_BPartner.class, ITrx.TRXNAME_None);
		bp.setValue(value);
		bp.setName(value);
		InterfaceWrapperHelper.save(bp);

		return bp;
	}

	private I_M_HU_PI_Version createM_HU_PI_Version(final String huUnitType)
	{
		final I_M_HU_PI pi = InterfaceWrapperHelper.create(Env.getCtx(), I_M_HU_PI.class, ITrx.TRXNAME_None);
		InterfaceWrapperHelper.save(pi);

		final I_M_HU_PI_Version piVersion = InterfaceWrapperHelper.create(Env.getCtx(), I_M_HU_PI_Version.class, ITrx.TRXNAME_None);
		piVersion.setM_HU_PI(pi);
		piVersion.setHU_UnitType(huUnitType);
		piVersion.setIsCurrent(true);
		piVersion.setName("M_HU_PI_ID=" + pi.getM_HU_PI_ID() + "_Current");
		InterfaceWrapperHelper.save(piVersion);
		return piVersion;
	}

	private I_M_HU_PI_Item createMaterialM_HU_PI_Item(final String huUnitType)
	{
		final I_M_HU_PI_Version piVersion = createM_HU_PI_Version(huUnitType);

		final I_M_HU_PI_Item piItem = InterfaceWrapperHelper.create(Env.getCtx(), I_M_HU_PI_Item.class, ITrx.TRXNAME_None);
		piItem.setM_HU_PI_Version(piVersion);
		piItem.setItemType(X_M_HU_PI_Item.ITEMTYPE_Material);
		InterfaceWrapperHelper.save(piItem);
		return piItem;
	}

	private void addPackingmaterialToItem(final ProductId packingProductId, final I_M_HU_PI_Item materialPiItem)
	{
		final I_M_HU_PackingMaterial packingMaterial = InterfaceWrapperHelper.create(Env.getCtx(), I_M_HU_PackingMaterial.class, ITrx.TRXNAME_None);
		packingMaterial.setM_Product_ID(packingProductId.getRepoId());
		InterfaceWrapperHelper.save(packingMaterial);

		final I_M_HU_PI_Item packingMaterialPiItem = InterfaceWrapperHelper.create(Env.getCtx(), I_M_HU_PI_Item.class, ITrx.TRXNAME_None);
		packingMaterialPiItem.setM_HU_PI_Version(materialPiItem.getM_HU_PI_Version());

		packingMaterialPiItem.setItemType(X_M_HU_PI_Item.ITEMTYPE_PackingMaterial);
		packingMaterialPiItem.setM_HU_PackingMaterial(packingMaterial);
		packingMaterialPiItem.setQty(BigDecimal.ONE);

		InterfaceWrapperHelper.save(packingMaterialPiItem);
	}

	private I_M_HU_PI_Item_Product createM_HU_PI_Item_Product(final ProductId productId, final BPartnerId bpartnerId, final ZonedDateTime validFrom, final String huUnitType)
	{
		final I_M_HU_PI_Item_Product piItemProduct = InterfaceWrapperHelper.create(Env.getCtx(), I_M_HU_PI_Item_Product.class, ITrx.TRXNAME_None);

		piItemProduct.setM_Product_ID(ProductId.toRepoId(productId));
		if (productId == null)
		{
			piItemProduct.setIsAllowAnyProduct(true);
		}

		piItemProduct.setIsInfiniteCapacity(false);

		final I_M_HU_PI_Item huPiItem = createMaterialM_HU_PI_Item(huUnitType);
		piItemProduct.setM_HU_PI_Item(huPiItem);

		piItemProduct.setC_BPartner_ID(BPartnerId.toRepoId(bpartnerId));
		piItemProduct.setValidFrom(TimeUtil.asTimestamp(validFrom));

		InterfaceWrapperHelper.save(piItemProduct);
		piItemProduct.setDescription(toString(piItemProduct));

		InterfaceWrapperHelper.save(piItemProduct);

		return piItemProduct;
	}

	private String toString(final I_M_HU_PI_Item_Product piItemProduct)
	{
		final StringBuilder sb = new StringBuilder();

		final BPartnerId bpartnerId = BPartnerId.ofRepoIdOrNull(piItemProduct.getC_BPartner_ID());
		if (bpartnerId != null)
		{
			if (sb.length() > 0)
			{
				sb.append(", ");
			}

			final I_C_BPartner bpartner = Services.get(IBPartnerDAO.class).getById(bpartnerId);
			sb.append("BP=").append(bpartner.getValue());
		}

		final ProductId productId = ProductId.ofRepoIdOrNull(piItemProduct.getM_Product_ID());
		if (productId != null)
		{
			if (sb.length() > 0)
			{
				sb.append(", ");
			}

			final String productValue = Services.get(IProductDAO.class).retrieveProductValueByProductId(productId);
			sb.append("Product=").append(productValue);
		}

		if (piItemProduct.isAllowAnyProduct())
		{
			if (sb.length() > 0)
			{
				sb.append(", ");
			}
			sb.append("IsAllowAnyProduct");
		}
		if (piItemProduct.getValidFrom() != null)
		{
			if (sb.length() > 0)
			{
				sb.append(", ");
			}
			sb.append("ValidFrom=").append(piItemProduct.getValidFrom());
		}

		final I_M_HU_PI_Item piItem = piItemProduct.getM_HU_PI_Item();
		final I_M_HU_PI_Version piVersion = piItem == null ? null : piItem.getM_HU_PI_Version();
		if (piVersion != null && piVersion.getHU_UnitType() != null)
		{
			if (sb.length() > 0)
			{
				sb.append(", ");
			}
			sb.append("huUnitType=").append(piVersion.getHU_UnitType());
		}

		final String instanceName = sb.toString();
		POJOWrapper.setInstanceName(piItemProduct, instanceName);

		return instanceName;
	}

	/**
	 * @see http://dewiki908/mediawiki/index.php/08868_Wareneingang_POS_does_not_suggest_customer_defined_TU_%28109729999780%29
	 */
	@Test
	public void test_retrieveTUs_case08868()
	{
		final I_M_HU_PI_Item_Product pip1 = createM_HU_PI_Item_Product(product1, bpartner_NULL, date1, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
		final I_M_HU_PI_Item_Product pip2 = createM_HU_PI_Item_Product(product1, bpartner1, date1, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
		@SuppressWarnings("unused")
		final I_M_HU_PI_Item_Product pip3 = createM_HU_PI_Item_Product(product1, bpartner2, date1, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);
		final I_M_HU_PI_Item_Product pip4 = createM_HU_PI_Item_Product(product1, bpartner1, date1, X_M_HU_PI_Version.HU_UNITTYPE_TransportUnit);

		final List<I_M_HU_PI_Item_Product> pipsExpected = Arrays.asList(pip1, pip2, pip4);
		final List<I_M_HU_PI_Item_Product> pipsActual = dao.retrieveTUs(
				Env.getCtx(),
				product1,
				bpartner1);

		Collections.sort(pipsActual, ModelByIdComparator.getInstance());
		Assert.assertEquals(
				pipsExpected,
				pipsActual);
	}
}
