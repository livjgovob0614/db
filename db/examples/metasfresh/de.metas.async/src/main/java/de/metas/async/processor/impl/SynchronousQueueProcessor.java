package de.metas.async.processor.impl;

/*
 * #%L
 * de.metas.async
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


import java.util.UUID;

import de.metas.async.api.IWorkPackageQueue;

class SynchronousQueueProcessor extends AbstractQueueProcessor
{
	private final String name;
	private boolean running;

	public SynchronousQueueProcessor(final IWorkPackageQueue queue)
	{
		super(queue);
		this.name = "SynchronousQueueProcessor_" + UUID.randomUUID();
		this.running = true;

		setQueuePollingTimeout(IWorkPackageQueue.TIMEOUT_OneTimeOnly);
	}

	@Override
	public String getName()
	{
		return name;
	}

	@Override
	protected void executeTask(final WorkpackageProcessorTask task)
	{
		task.run();
	}

	@Override
	public void shutdown()
	{
		running = false;
		// nothing
	}

	@Override
	protected boolean isRunning()
	{
		return running;
	}
}
