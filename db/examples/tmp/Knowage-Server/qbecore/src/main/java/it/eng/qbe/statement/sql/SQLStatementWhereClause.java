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

package it.eng.qbe.statement.sql;

import java.util.Map;

import org.apache.log4j.Logger;

import it.eng.qbe.query.Query;
import it.eng.qbe.statement.AbstractStatementWhereClause;
import it.eng.qbe.statement.IConditionalOperator;
import it.eng.qbe.statement.IStatement;
import it.eng.qbe.statement.jpa.JPQLStatementWhereClause;

/**
 * @author Alberto Ghedin (alberto.ghedin@eng.it)
 */

public class SQLStatementWhereClause extends AbstractStatementWhereClause {

	public static final String WHERE = "WHERE";

	public static transient Logger logger = Logger.getLogger(JPQLStatementWhereClause.class);

	public static String build(SQLStatement parentStatement, Query query, Map<String, Map<String, String>> entityAliasesMaps) {
		SQLStatementWhereClause clause = new SQLStatementWhereClause(parentStatement);
		return clause.buildClause(query, entityAliasesMaps);
	}

	public SQLStatementWhereClause(SQLStatement statement) {
		parentStatement = statement;
	}

	@Override
	public IConditionalOperator getOperator(String operator) {
		return SQLStatementConditionalOperators.getOperator(operator);
	}

	public static String injectAutoJoins(IStatement parentStatement, String whereClause, Query query, Map<String, Map<String, String>> entityAliasesMaps) {
		return AbstractStatementWhereClause.injectAutoJoins(parentStatement, whereClause, query, entityAliasesMaps);
	}

}
