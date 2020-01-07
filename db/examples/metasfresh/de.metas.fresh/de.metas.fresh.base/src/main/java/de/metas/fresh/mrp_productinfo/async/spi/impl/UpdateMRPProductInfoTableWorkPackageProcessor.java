package de.metas.fresh.mrp_productinfo.async.spi.impl;

import java.util.Map;
import java.util.Properties;

import org.adempiere.model.InterfaceWrapperHelper;
import org.adempiere.model.PlainContextAware;
import org.adempiere.util.api.IParams;

import de.metas.async.api.IWorkpackageParamDAO;
import de.metas.async.model.I_C_Queue_WorkPackage;
import de.metas.async.spi.WorkpackageProcessorAdapter;
import de.metas.async.spi.WorkpackagesOnCommitSchedulerTemplate;
import de.metas.fresh.mrp_productinfo.IMRPProductInfoBL;
import de.metas.fresh.mrp_productinfo.IMRPProductInfoSelector;
import de.metas.fresh.mrp_productinfo.IMRPProductInfoSelectorFactory;
import de.metas.util.Services;

/*
 * #%L
 * de.metas.fresh.base
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

public class UpdateMRPProductInfoTableWorkPackageProcessor extends WorkpackageProcessorAdapter
{

	private static final WorkpackagesOnCommitSchedulerTemplate<IMRPProductInfoSelector> SCHEDULER = new WorkpackagesOnCommitSchedulerTemplate<IMRPProductInfoSelector>(UpdateMRPProductInfoTableWorkPackageProcessor.class)
	{
		@Override
		protected Properties extractCtxFromItem(final IMRPProductInfoSelector item)
		{
			return item.getCtx();
		}

		@Override
		protected String extractTrxNameFromItem(final IMRPProductInfoSelector item)
		{
			return item.getTrxName();
		}

		/** @return null
		 */
		@Override
		protected Object extractModelToEnqueueFromItem(final Collector collector, final IMRPProductInfoSelector item)
		{
			return null;
		}

		@Override
		protected Map<String, Object> extractParametersFromItem(final IMRPProductInfoSelector item)
		{
			return item.asMap();
		}

		/** @return {@code true} because we don't enqueue elements, just parameters. */
		@Override
		protected boolean isEnqueueWorkpackageWhenNoModelsEnqueued()
		{
			return true;
		}
	};

	public static void schedule(final Object item)
	{
		final IMRPProductInfoSelectorFactory mrpProductInfoSelectorFactory = Services.get(IMRPProductInfoSelectorFactory.class);

		final IMRPProductInfoSelector itemToEnqueue = mrpProductInfoSelectorFactory.createOrNullForModel(item);
		if (itemToEnqueue == null)
		{
			return; // nothing to do
		}
		SCHEDULER.schedule(itemToEnqueue);
	}

	@Override
	public Result processWorkPackage(
			final I_C_Queue_WorkPackage workpackage,
			final String localTrxName)
	{
		final IWorkpackageParamDAO workpackageParamDAO = Services.get(IWorkpackageParamDAO.class);
		final IMRPProductInfoBL mrpProductInfoBL = Services.get(IMRPProductInfoBL.class);
		final IParams params = workpackageParamDAO.retrieveWorkpackageParams(workpackage);

		final Properties ctx = InterfaceWrapperHelper.getCtx(workpackage);

		mrpProductInfoBL.updateItems(
				PlainContextAware.newWithTrxName(ctx, localTrxName),
				params);

		return Result.SUCCESS;
	}
}
