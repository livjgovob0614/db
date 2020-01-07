package de.metas.bpartner;

import java.util.Objects;

import javax.annotation.Nullable;

import de.metas.util.Check;
import de.metas.util.lang.RepoIdAware;
import lombok.NonNull;
import lombok.Value;

/*
 * #%L
 * de.metas.business
 * %%
 * Copyright (C) 2018 metas GmbH
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

@Value
public class BPartnerLocationId implements RepoIdAware
{
	int repoId;

	@NonNull
	BPartnerId bpartnerId;

	public static BPartnerLocationId ofRepoId(@NonNull final BPartnerId bpartnerId, final int bpartnerLocationId)
	{
		return new BPartnerLocationId(bpartnerId, bpartnerLocationId);
	}

	public static BPartnerLocationId ofRepoId(final int bpartnerId, final int bpartnerLocationId)
	{
		return new BPartnerLocationId(BPartnerId.ofRepoId(bpartnerId), bpartnerLocationId);
	}

	public static BPartnerLocationId ofRepoIdOrNull(
			@Nullable final Integer bpartnerId,
			@Nullable final Integer bpartnerLocationId)
	{
		return bpartnerId != null && bpartnerId > 0 && bpartnerLocationId != null && bpartnerLocationId > 0
				? ofRepoId(bpartnerId, bpartnerLocationId)
				: null;
	}

	public static BPartnerLocationId ofRepoIdOrNull(
			@Nullable final BPartnerId bpartnerId,
			final int bpartnerLocationId)
	{
		return bpartnerId != null && bpartnerLocationId > 0 ? ofRepoId(bpartnerId, bpartnerLocationId) : null;
	}

	private BPartnerLocationId(@NonNull final BPartnerId bpartnerId, final int bpartnerLocationId)
	{
		this.repoId = Check.assumeGreaterThanZero(bpartnerLocationId, "bpartnerLocationId");
		this.bpartnerId = bpartnerId;
	}

	public static int toRepoId(final BPartnerLocationId bpLocationId)
	{
		return toRepoIdOr(bpLocationId, -1);
	}

	public static int toRepoIdOr(final BPartnerLocationId bpLocationId, final int defaultValue)
	{
		return bpLocationId != null ? bpLocationId.getRepoId() : defaultValue;
	}

	public static boolean equals(final BPartnerLocationId id1, final BPartnerLocationId id2)
	{
		return Objects.equals(id1, id2);
	}
}
