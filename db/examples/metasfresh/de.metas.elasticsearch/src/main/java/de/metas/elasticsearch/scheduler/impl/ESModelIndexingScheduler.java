package de.metas.elasticsearch.scheduler.impl;

import java.util.List;
import java.util.Properties;

import org.adempiere.util.lang.ITableRecordReference;
import org.adempiere.util.lang.impl.TableRecordReference;
import org.compiere.util.Env;

import com.google.common.annotations.VisibleForTesting;

import de.metas.async.processor.IWorkPackageQueueFactory;
import de.metas.elasticsearch.config.ESModelIndexerId;
import de.metas.elasticsearch.scheduler.IESModelIndexingScheduler;
import de.metas.util.Services;
import lombok.NonNull;

/*
 * #%L
 * de.metas.elasticsearch
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

public class ESModelIndexingScheduler implements IESModelIndexingScheduler
{
	@VisibleForTesting
	static final String CLASSNAME_AddToIndexWorkpackageProcessor = "de.metas.elasticsearch.scheduler.async.AsyncAddToIndexProcessor";
	@VisibleForTesting
	static final String CLASSNAME_RemoveFromIndexWorkpackageProcessor = "de.metas.elasticsearch.scheduler.async.AsyncRemoveFromIndexProcessor";

	public static final String PARAMETERNAME_ModelIndexerId = "ModelIndexerId";

	@Override
	public final void addToIndex(final ESModelIndexerId modelIndexerId, final String modelTableName, final List<Integer> modelIds)
	{
		final List<TableRecordReference> models = TableRecordReference.ofRecordIds(modelTableName, modelIds);
		schedule(CLASSNAME_AddToIndexWorkpackageProcessor, modelIndexerId, models);
	}

	@Override
	public final void removeToIndex(final ESModelIndexerId modelIndexerId, final String modelTableName, final List<Integer> modelIds)
	{
		final List<TableRecordReference> models = TableRecordReference.ofRecordIds(modelTableName, modelIds);
		schedule(CLASSNAME_RemoveFromIndexWorkpackageProcessor, modelIndexerId, models);
	}

	private final void schedule(
			@NonNull final String workpackageProcessorClassname,
			@NonNull final ESModelIndexerId modelIndexerId,
			final List<? extends ITableRecordReference> models)
	{
		final Properties ctx = Env.getCtx();

		Services.get(IWorkPackageQueueFactory.class)
				.getQueueForEnqueuing(ctx, workpackageProcessorClassname)
				.newBlock()
				.newWorkpackage()
				.bindToThreadInheritedTrx()
				.addElements(models)
				.parameter(PARAMETERNAME_ModelIndexerId, modelIndexerId.toJson())
				.build();
	}
}
