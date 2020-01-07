package de.metas.marketing.base;

import java.util.List;
import java.util.Optional;

import org.springframework.stereotype.Service;

import com.google.common.base.Predicates;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;

import de.metas.marketing.base.spi.PlatformClientFactory;
import de.metas.util.Check;
import de.metas.util.GuavaCollectors;
import lombok.NonNull;

/*
 * #%L
 * marketing-base
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 * #L%
 */

@Service
public class PlatformClientFactoryRegistry
{
	private final ImmutableMap<String, PlatformClientFactory> clientFactoriesByGatewayId;

	public PlatformClientFactoryRegistry(@NonNull final Optional<List<PlatformClientFactory>> platformClientFactories)
	{
		clientFactoriesByGatewayId = platformClientFactories.orElse(ImmutableList.of())
				.stream()
				.filter(Predicates.notNull())
				.collect(GuavaCollectors.toImmutableMapByKey(PlatformClientFactory::getPlatformGatewayId));
	}

	public PlatformClientFactory getPlatformClientFactory(@NonNull final String platformGatewayId)
	{
		final PlatformClientFactory service = clientFactoriesByGatewayId.get(platformGatewayId);
		return Check.assumeNotNull(service, "service shall exist for platformGatewayId={}", platformGatewayId);
	}

	public boolean hasGatewaySupport(@NonNull final String platformGatewayId)
	{
		return clientFactoriesByGatewayId.containsKey(platformGatewayId);
	}
}
