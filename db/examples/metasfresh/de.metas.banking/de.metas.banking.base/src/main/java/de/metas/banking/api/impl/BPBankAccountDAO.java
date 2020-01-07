package de.metas.banking.api.impl;

import static org.adempiere.model.InterfaceWrapperHelper.loadOutOfTrx;

import java.util.Collection;
import java.util.List;
import java.util.Optional;
import java.util.Properties;

import org.adempiere.ad.dao.ICompositeQueryUpdater;
import org.adempiere.ad.dao.IQueryBL;
import org.adempiere.ad.dao.IQueryBuilder;
import org.adempiere.ad.dao.IQueryOrderBy.Direction;
import org.adempiere.ad.dao.IQueryOrderBy.Nulls;
import org.adempiere.ad.trx.api.ITrx;
import org.compiere.model.I_C_BP_BankAccount;

import com.google.common.collect.ImmutableListMultimap;

import de.metas.banking.api.BankAccountId;
import de.metas.banking.api.IBPBankAccountDAO;
import de.metas.bpartner.BPartnerBankAccountId;
import de.metas.bpartner.BPartnerId;
import de.metas.money.CurrencyId;
import de.metas.util.Check;
import de.metas.util.Services;
import lombok.NonNull;

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

public class BPBankAccountDAO implements IBPBankAccountDAO
{
	private final IQueryBL queryBL = Services.get(IQueryBL.class);

	@Override
	public I_C_BP_BankAccount getById(final int bpBankAccountId)
	{
		Check.assumeGreaterThanZero(bpBankAccountId, "bpBankAccountId");
		return loadOutOfTrx(bpBankAccountId, I_C_BP_BankAccount.class);
	}

	@Override
	public ImmutableListMultimap<BPartnerId, I_C_BP_BankAccount> getByBPartnerIds(@NonNull final Collection<BPartnerId> bpartnerIds)
	{
		if (bpartnerIds.isEmpty())
		{
			return ImmutableListMultimap.of();
		}

		return queryBL.createQueryBuilderOutOfTrx(I_C_BP_BankAccount.class)
				.addInArrayFilter(I_C_BP_BankAccount.COLUMNNAME_C_BPartner_ID, bpartnerIds)
				.create()
				.stream()
				.collect(ImmutableListMultimap.toImmutableListMultimap(
						record -> BPartnerId.ofRepoId(record.getC_BPartner_ID()),
						record -> record));
	}

	@Override
	public List<I_C_BP_BankAccount> retrieveBankAccountsForPartnerAndCurrency(Properties ctx, int partnerID, int currencyID)
	{
		final IQueryBuilder<I_C_BP_BankAccount> qb = queryBL
				.createQueryBuilder(I_C_BP_BankAccount.class, ctx, ITrx.TRXNAME_None)
				.addEqualsFilter(I_C_BP_BankAccount.COLUMN_C_BPartner_ID, partnerID);

		if (currencyID > 0)
		{
			qb.addEqualsFilter(I_C_BP_BankAccount.COLUMN_C_Currency_ID, currencyID);
		}

		final List<I_C_BP_BankAccount> bpBankAccounts = qb.addOnlyActiveRecordsFilter()
				.orderBy()
				.addColumn(de.metas.banking.model.I_C_BP_BankAccount.COLUMNNAME_IsDefault, Direction.Descending, Nulls.Last) // DESC (Y, then N)
				.addColumn(I_C_BP_BankAccount.COLUMNNAME_C_BP_BankAccount_ID)
				.endOrderBy()
				.create()
				.list();

		return bpBankAccounts;
		// return LegacyAdapters.convertToPOArray(bpBankAccounts, MBPBankAccount.class);
	}    // getOfBPartner

	@Override
	public Optional<BankAccountId> retrieveBankAccountByBPartnerAndCurrencyAndIBAN(@NonNull final BPartnerId bPartnerId, @NonNull final CurrencyId currencyId, @NonNull final String iban)
	{
		final BankAccountId bankAccountId = queryBL.createQueryBuilder(I_C_BP_BankAccount.class)
				.addEqualsFilter(I_C_BP_BankAccount.COLUMN_C_BPartner_ID, bPartnerId)
				.addEqualsFilter(I_C_BP_BankAccount.COLUMN_C_Currency_ID, currencyId)
				.addEqualsFilter(I_C_BP_BankAccount.COLUMNNAME_IBAN, iban)
				.addOnlyActiveRecordsFilter()
				.create()
				.firstIdOnly(BankAccountId::ofRepoIdOrNull);

		return Optional.ofNullable(bankAccountId);
	}

	@Override
	public void deactivateByBPartnerExcept(
			@NonNull final BPartnerId bpartnerId,
			@NonNull final Collection<BPartnerBankAccountId> exceptIds)
	{
		final ICompositeQueryUpdater<I_C_BP_BankAccount> columnUpdater = queryBL
				.createCompositeQueryUpdater(I_C_BP_BankAccount.class)
				.addSetColumnValue(I_C_BP_BankAccount.COLUMNNAME_IsActive, false);

		queryBL.createQueryBuilder(I_C_BP_BankAccount.class)
				.addOnlyActiveRecordsFilter()
				.addEqualsFilter(I_C_BP_BankAccount.COLUMN_C_BPartner_ID, bpartnerId)
				.addNotInArrayFilter(I_C_BP_BankAccount.COLUMN_C_BP_BankAccount_ID, exceptIds)
				.create()
				.update(columnUpdater);
	}
}
