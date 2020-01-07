package de.metas.currency.impl;

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
import java.util.Properties;

import javax.annotation.Nullable;

import org.adempiere.service.ClientId;
import org.compiere.util.Env;

import de.metas.acct.api.AcctSchema;
import de.metas.acct.api.IAcctSchemaDAO;
import de.metas.currency.ConversionTypeMethod;
import de.metas.currency.Currency;
import de.metas.currency.CurrencyConversionContext;
import de.metas.currency.CurrencyConversionContext.CurrencyConversionContextBuilder;
import de.metas.currency.CurrencyConversionResult;
import de.metas.currency.CurrencyConversionResult.CurrencyConversionResultBuilder;
import de.metas.currency.CurrencyPrecision;
import de.metas.currency.CurrencyRate;
import de.metas.currency.ICurrencyBL;
import de.metas.currency.ICurrencyDAO;
import de.metas.currency.exceptions.NoCurrencyRateFoundException;
import de.metas.money.CurrencyConversionTypeId;
import de.metas.money.CurrencyId;
import de.metas.organization.OrgId;
import de.metas.util.Check;
import de.metas.util.Services;
import de.metas.util.time.SystemTime;
import lombok.NonNull;

public class CurrencyBL implements ICurrencyBL
{
	@Override
	public final Currency getBaseCurrency(final Properties ctx)
	{
		final ClientId adClientId = Env.getClientId(ctx);
		final OrgId adOrgId = Env.getOrgId(ctx);

		return getBaseCurrency(adClientId, adOrgId);
	}

	@Override
	public Currency getBaseCurrency(
			@NonNull final ClientId adClientId,
			@NonNull final OrgId adOrgId)
	{
		final CurrencyId currencyId = getBaseCurrencyId(adClientId, adOrgId);

		final ICurrencyDAO currenciesRepo = Services.get(ICurrencyDAO.class);
		return currenciesRepo.getById(currencyId);
	}

	@Override
	public CurrencyId getBaseCurrencyId(
			@NonNull final ClientId adClientId,
			@NonNull final OrgId adOrgId)
	{
		final AcctSchema as = Services.get(IAcctSchemaDAO.class).getByCliendAndOrg(adClientId, adOrgId);
		Check.assumeNotNull(as, "Missing C_AcctSchema for AD_Client_ID={} and AD_Org_ID={}", adClientId, adOrgId);

		return as.getCurrencyId();
	}

	@Override
	public BigDecimal convertBase(
			final BigDecimal amt,
			final CurrencyId currencyFromId,
			final LocalDate convDate,
			final CurrencyConversionTypeId conversionTypeId,
			@NonNull final ClientId clientId,
			@NonNull final OrgId orgId)
	{
		final CurrencyId currencyToId = getBaseCurrencyId(clientId, orgId);
		return convert(amt, currencyFromId, currencyToId, convDate, conversionTypeId, clientId, orgId);
	}

	@Override
	public final BigDecimal convert(
			final BigDecimal amt,
			final CurrencyId currencyFromId,
			final CurrencyId currencyToId,
			final LocalDate convDate,
			final CurrencyConversionTypeId conversionTypeId,
			@NonNull final ClientId clientId,
			@NonNull final OrgId orgId)
	{
		final CurrencyConversionContext conversionCtx = createCurrencyConversionContext(
				convDate,
				conversionTypeId,
				clientId,
				orgId);
		final CurrencyConversionResult conversionResult = convert(
				conversionCtx,
				amt,
				currencyFromId,
				currencyToId);
		if (conversionResult == null)
		{
			return null;
		}
		return conversionResult.getAmount();
	}

	@Override
	public final CurrencyConversionResult convert(
			@NonNull final CurrencyConversionContext conversionCtx,
			@NonNull final BigDecimal amt,
			@NonNull final CurrencyId currencyFromId,
			@NonNull final CurrencyId currencyToId)
	{
		final BigDecimal amtConv;
		final BigDecimal conversionRateBD;
		if (amt.signum() == 0)
		{
			amtConv = BigDecimal.ZERO;
			conversionRateBD = null; // N/A
		}
		// Same Currency
		else if (CurrencyId.equals(currencyFromId, currencyToId))
		{
			amtConv = amt;
			conversionRateBD = BigDecimal.ONE;
		}
		else
		{
			final CurrencyRate currencyRate = getCurrencyRateOrNull(conversionCtx, currencyFromId, currencyToId);
			if (currencyRate == null)
			{
				// TODO: evaluate if we can throw an exception here
				return null;
			}

			conversionRateBD = currencyRate.getConversionRate();

			//
			// Calculate the converted amount
			amtConv = currencyRate.convertAmount(amt);
		}

		return prepareCurrencyConversionResult(conversionCtx)
				.sourceAmount(amt)
				.sourceCurrencyId(currencyFromId)
				.amount(amtConv)
				.currencyId(currencyToId)
				.conversionRateOrNull(conversionRateBD)
				.build();
	}	// convert

	private CurrencyPrecision getStdPrecision(final CurrencyId currencyId)
	{
		return Services.get(ICurrencyDAO.class).getStdPrecision(currencyId);
	}

	@Override
	public final BigDecimal convert(
			final BigDecimal amt,
			final CurrencyId currencyFromId,
			final CurrencyId currencyToId,
			@NonNull final ClientId clientId,
			@NonNull final OrgId orgId)
	{
		final CurrencyConversionContext conversionCtx = createCurrencyConversionContext(
				(LocalDate)null, // convDate
				(CurrencyConversionTypeId)null, // C_ConversionType_ID,
				clientId,
				orgId);
		final CurrencyConversionResult conversionResult = convert(
				conversionCtx,
				amt,
				currencyFromId,
				currencyToId);
		if (conversionResult == null)
		{
			return null;
		}
		return conversionResult.getAmount();
	}

	@Override
	public final BigDecimal getRate(
			final CurrencyId currencyFromId,
			final CurrencyId currencyToId,
			final LocalDate convDate,
			final CurrencyConversionTypeId conversionTypeId,
			final ClientId clientId,
			final OrgId orgId)
	{
		final CurrencyConversionContext conversionCtx = createCurrencyConversionContext(
				convDate,
				conversionTypeId,
				clientId,
				orgId);
		return getRate(conversionCtx, currencyFromId, currencyToId);
	}

	@Override
	public final CurrencyConversionContext createCurrencyConversionContext(
			@Nullable final LocalDate convDate,
			@Nullable final CurrencyConversionTypeId conversionTypeId,
			@NonNull final ClientId clientId,
			@NonNull final OrgId orgId)
	{
		final CurrencyConversionContextBuilder conversionCtx = CurrencyConversionContext.builder()
				.clientId(clientId)
				.orgId(orgId);

		final LocalDate convDateToUse = convDate != null ? convDate : SystemTime.asLocalDate();

		// Conversion Type
		if (conversionTypeId != null)
		{
			conversionCtx.conversionTypeId(conversionTypeId);
		}
		else
		{
			final CurrencyConversionTypeId defaultConversionTypeId = getDefaultConversionTypeId(clientId, orgId, convDateToUse);
			conversionCtx.conversionTypeId(defaultConversionTypeId);
		}

		// Conversion Date
		conversionCtx.conversionDate(convDateToUse);

		return conversionCtx.build();
	}

	@Override
	public final CurrencyConversionContext createCurrencyConversionContext(
			@NonNull final LocalDate convDate,
			@Nullable final ConversionTypeMethod conversionType,
			@NonNull final ClientId clientId,
			@NonNull final OrgId orgId)
	{
		// Find C_ConversionType_ID
		final ICurrencyDAO currencyDAO = Services.get(ICurrencyDAO.class);
		final ConversionTypeMethod conversionTypeEffective = conversionType != null ? conversionType : ConversionTypeMethod.Spot;
		final CurrencyConversionTypeId conversionTypeId = currencyDAO.getConversionTypeId(conversionTypeEffective);

		return createCurrencyConversionContext(convDate, conversionTypeId, clientId, orgId);
	}

	private CurrencyConversionTypeId getDefaultConversionTypeId(
			final ClientId adClientId,
			final OrgId adOrgId,
			final LocalDate date)
	{
		final ICurrencyDAO currenciesRepo = Services.get(ICurrencyDAO.class);
		return currenciesRepo.getDefaultConversionTypeId(adClientId, adOrgId, date);
	}

	@Override
	public BigDecimal getRate(
			@NonNull final CurrencyConversionContext conversionCtx,
			@NonNull final CurrencyId currencyFromId,
			@NonNull final CurrencyId currencyToId)
	{
		final CurrencyRate currencyRate = getCurrencyRateOrNull(conversionCtx, currencyFromId, currencyToId);
		return currencyRate == null ? null : currencyRate.getConversionRate();
	}

	private CurrencyRate getCurrencyRateOrNull(
			@NonNull final CurrencyConversionContext conversionCtx,
			@NonNull final CurrencyId currencyFromId,
			@NonNull final CurrencyId currencyToId)
	{
		final CurrencyConversionTypeId conversionTypeId = conversionCtx.getConversionTypeId();
		final LocalDate conversionDate = conversionCtx.getConversionDate();

		final BigDecimal conversionRate;
		if (currencyFromId.equals(currencyToId))
		{
			conversionRate = BigDecimal.ONE;
		}
		else
		{
			conversionRate = Services.get(ICurrencyDAO.class).retrieveRateOrNull(conversionCtx, currencyFromId, currencyToId);
			if (conversionRate == null)
			{
				return null;
			}
		}

		final CurrencyPrecision currencyPrecision = conversionCtx.getPrecision()
				.orElseGet(() -> getStdPrecision(currencyToId));

		return CurrencyRate.builder()
				.conversionRate(conversionRate)
				.fromCurrencyId(currencyFromId)
				.toCurrencyId(currencyToId)
				.currencyPrecision(currencyPrecision)
				.conversionTypeId(conversionTypeId)
				.conversionDate(conversionDate)
				.build();
	}

	@Override
	public CurrencyRate getCurrencyRate(
			@NonNull final CurrencyConversionContext conversionCtx,
			@NonNull final CurrencyId currencyFromId,
			@NonNull final CurrencyId currencyToId)
	{
		final CurrencyRate currencyRate = getCurrencyRateOrNull(conversionCtx, currencyFromId, currencyToId);
		if (currencyRate == null)
		{
			throw new NoCurrencyRateFoundException(conversionCtx, currencyFromId, currencyToId);
		}
		return currencyRate;
	}

	private static final CurrencyConversionResultBuilder prepareCurrencyConversionResult(@NonNull final CurrencyConversionContext conversionCtx)
	{
		return CurrencyConversionResult.builder()
				.clientId(conversionCtx.getClientId())
				.orgId(conversionCtx.getOrgId())
				.conversionDate(conversionCtx.getConversionDate())
				.conversionTypeId(conversionCtx.getConversionTypeId());
	}

}
