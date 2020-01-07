package de.metas.contracts.spi;

import java.util.List;

import javax.annotation.OverridingMethodsMustInvokeSuper;

import org.adempiere.exceptions.AdempiereException;
import org.adempiere.model.InterfaceWrapperHelper;

import de.metas.contracts.IFlatrateDAO;
import de.metas.contracts.IFlatrateTermEventService;
import de.metas.contracts.model.I_C_Flatrate_DataEntry;
import de.metas.contracts.model.I_C_Flatrate_Term;
import de.metas.contracts.model.I_C_Invoice_Clearing_Alloc;
import de.metas.contracts.model.X_C_Flatrate_DataEntry;
import de.metas.invoicecandidate.api.IInvoiceCandDAO;
import de.metas.util.Services;
import lombok.NonNull;

/*
 * #%L
 * de.metas.contracts
 * %%
 * Copyright (C) 2016 metas GmbH
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

/**
 * This one is invoked by {@link IFlatrateTermEventService} if no other listener was registered for a particular conditions type.
 *
 * @author metas-dev <dev@metasfresh.com>
 *
 */
public class FallbackFlatrateTermEventListener implements IFlatrateTermEventListener
{
	private static final String MSG_TERM_ERROR_ENTRY_ALREADY_CO_2P = "Term_Error_Entry_Already_CO";

	@Override
	@OverridingMethodsMustInvokeSuper
	public void beforeFlatrateTermReactivate(final I_C_Flatrate_Term term)
	{
		deleteFlatrateTermDataEntriesOnReactivate(term);
		deleteC_Invoice_Clearing_AllocsOnReactivate(term);
		deleteInvoiceCandidates(term);
	}

	/**
	 * Deletes {@link I_C_Flatrate_DataEntry} records for given term.
	 *
	 * @throws AdempiereException if any data entry record is completed
	 */
	protected void deleteFlatrateTermDataEntriesOnReactivate(final I_C_Flatrate_Term term)
	{
		final IFlatrateDAO flatrateDAO = Services.get(IFlatrateDAO.class);
		final List<I_C_Flatrate_DataEntry> entries = flatrateDAO.retrieveDataEntries(term, null, null);
		for (final I_C_Flatrate_DataEntry entry : entries)
		{
			// note: The system will prevent the deletion of a completed entry
			// However, we want to give a user-friendly explanation to the user.
			if (X_C_Flatrate_DataEntry.DOCSTATUS_Completed.equals(entry.getDocStatus()))
			{
				throw new AdempiereException(
						MSG_TERM_ERROR_ENTRY_ALREADY_CO_2P,
						new Object[] { entry.getC_UOM().getName(), entry.getC_Period().getName() });
			}
			InterfaceWrapperHelper.delete(entry);
		}
	}

	/**
	 * Deletes {@link I_C_Invoice_Clearing_Alloc}s for given term.
	 *
	 * @param term
	 */
	protected void deleteC_Invoice_Clearing_AllocsOnReactivate(final I_C_Flatrate_Term term)
	{
		// note: we assume that invoice candidate validator will take care of the invoice candidate's IsToClear flag
		final IFlatrateDAO flatrateDAO = Services.get(IFlatrateDAO.class);
		final List<I_C_Invoice_Clearing_Alloc> icas = flatrateDAO.retrieveClearingAllocs(term);
		for (final I_C_Invoice_Clearing_Alloc ica : icas)
		{
			InterfaceWrapperHelper.delete(ica);
		}
	}

	/**
	 * When a term is reactivated, its invoice candidate needs to be deleted.
	 * Note that we assume the deletion will fail with a meaningful error message if the invoice candidate has already been invoiced.
	 */
	public void deleteInvoiceCandidates(@NonNull final I_C_Flatrate_Term term)
	{
		Services.get(IInvoiceCandDAO.class).deleteAllReferencingInvoiceCandidates(term);
	}

	/**
	 * Does nothing; Feel free to override.
	 */
	@Override
	public void afterSaveOfNextTermForPredecessor(I_C_Flatrate_Term next, I_C_Flatrate_Term predecessor)
	{
		// nothing
	}

	/**
	 * Does nothing; Feel free to override.
	 */
	@Override
	public void afterFlatrateTermEnded(I_C_Flatrate_Term term)
	{
		// nothing
	}

	/**
	 * Does nothing; Feel free to override.
	 */
	@Override
	public void beforeSaveOfNextTermForPredecessor(I_C_Flatrate_Term next, I_C_Flatrate_Term predecessor)
	{
		// nothing
	}
}
