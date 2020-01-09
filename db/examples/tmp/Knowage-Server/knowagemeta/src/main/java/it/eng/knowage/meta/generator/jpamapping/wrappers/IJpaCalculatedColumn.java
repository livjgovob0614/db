/*
 * Knowage, Open Source Business Intelligence suite
 * Copyright (C) 2016 Engineering Ingegneria Informatica S.p.A.
 *
 * Knowage is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Knowage is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package it.eng.knowage.meta.generator.jpamapping.wrappers;

public interface IJpaCalculatedColumn {

	public static final String PUBLIC_SCOPE = "public"; //$NON-NLS-1$
	public static final String PROTECTED_SCOPE = "protected"; //$NON-NLS-1$
	public static final String PRIVATE_SCOPE = "private"; //$NON-NLS-1$

	/**
	 *
	 * @return the parent table
	 */
	IJpaTable getJpaTable();

	String getName();

	String getDescription();

	String getNature();

	String getDataType();

	String getExpression();

	/**
	 * @return the value of the specified column attribute
	 */
	public String getAttribute(String name);

}