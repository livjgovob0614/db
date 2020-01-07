package org.adempiere.archive.spi;

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

import org.adempiere.archive.api.IArchiveEventManager;
import org.compiere.model.I_AD_Archive;
import de.metas.email.EMailAddress;
import de.metas.email.mailboxes.UserEMailConfig;
import de.metas.user.UserId;

/**
 * Implementors can be registered to {@link IArchiveEventManager#registerArchiveEventListener(IArchiveEventListener)} and can then be fired using that manager.
 *
 * @author metas-dev <dev@metasfresh.com>
 *
 */
public interface IArchiveEventListener
{
	default void onPdfUpdate(I_AD_Archive archive, UserId userId, String action)
	{
		// nothing
	}

	default void onEmailSent(I_AD_Archive archive, String action, UserEMailConfig user, EMailAddress from, EMailAddress to, EMailAddress cc, EMailAddress bcc, String status)
	{
		// nothing
	}

	default void onPrintOut(I_AD_Archive archive, UserId userId, String printerName, int copies, String status)
	{
		// nothing
	}

	default void onVoidDocument(I_AD_Archive archive)
	{
		// nothing
	}
}
