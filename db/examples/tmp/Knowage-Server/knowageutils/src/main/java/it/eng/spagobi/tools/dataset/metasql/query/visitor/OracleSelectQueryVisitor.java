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

package it.eng.spagobi.tools.dataset.metasql.query.visitor;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import it.eng.spagobi.tools.dataset.common.datawriter.CockpitJSONDataWriter;
import it.eng.spagobi.tools.dataset.common.query.AggregationFunctions;
import it.eng.spagobi.tools.dataset.common.query.IAggregationFunction;
import it.eng.spagobi.tools.dataset.metasql.query.item.InFilter;
import it.eng.spagobi.tools.dataset.metasql.query.item.Projection;
import it.eng.spagobi.tools.dataset.metasql.query.item.Sorting;
import it.eng.spagobi.utilities.database.IDataBase;

public class OracleSelectQueryVisitor extends AbstractSelectQueryVisitor {

	private static final int SQL_IN_CLAUSE_LIMIT = 999;
	private static final String DATE_FORMAT = "YYYY-MM-DD HH24:MI:SS";
	private static final String TIMESTAMP_FORMAT = DATE_FORMAT + ".FF";

	public OracleSelectQueryVisitor(IDataBase database) {
		super(database);
	}

	@Override
	protected void append(InFilter item) {
		List<Projection> projections = item.getProjections();
		String openBracket;
		if (projections.size() > 1) {
			if (projections.size() > SQL_IN_CLAUSE_LIMIT) {
				openBracket = "(1,";
			} else {
				openBracket = "(";
			}
		} else {
			openBracket = "";
		}
		String closeBracket = projections.size() > 1 ? ")" : "";

		queryBuilder.append(openBracket);

		append(projections.get(0), false);
		for (int i = 1; i < projections.size(); i++) {
			queryBuilder.append(",");
			append(projections.get(i), false);
		}

		queryBuilder.append(closeBracket);

		queryBuilder.append(" ");
		queryBuilder.append(item.getOperator());
		queryBuilder.append(" (");

		List<Object> operands = item.getOperands();
		for (int i = 0; i < operands.size(); i++) {
			if (i % projections.size() == 0) { // 1st item of tuple of values
				if (i >= projections.size()) { // starting from 2nd tuple of values
					queryBuilder.append(",");
				}
				queryBuilder.append(openBracket);
			}
			if (i % projections.size() != 0) {
				queryBuilder.append(",");
			}
			append(operands.get(i));
			if (i % projections.size() == projections.size() - 1) { // last item of tuple of values
				queryBuilder.append(closeBracket);
			}
		}

		queryBuilder.append(")");
	}

	@Override
	protected void append(Object operand) {
		if (operand == null) {
			queryBuilder.append("NULL");
		} else {
			if (buildPreparedStatement) {
				queryBuilder.append("?");
				queryParameters.add(operand);
			} else {
				if (operand.getClass().toString().toLowerCase().contains("timestamp")) {
					queryBuilder.append(getFormattedTimestamp(operand.toString()));
				} else if (Date.class.isAssignableFrom(operand.getClass())) {
					queryBuilder.append(getFormattedDate((Date) operand));
				} else if (String.class.isAssignableFrom(operand.getClass())) {
					queryBuilder.append("'");
					queryBuilder.append(((String) operand).replaceAll("'", "''"));
					queryBuilder.append("'");
				} else {
					queryBuilder.append(operand);
				}
			}
		}
	}

	public String getFormattedTimestamp(String timestamp) {
		StringBuilder sb = new StringBuilder();

		sb.append("TO_TIMESTAMP('");
		sb.append(timestamp);
		sb.append("','");
		sb.append(TIMESTAMP_FORMAT);
		sb.append("')");

		return sb.toString();
	}

	@Override
	public String getFormattedDate(Date date) {
		SimpleDateFormat dateFormat = new SimpleDateFormat(CockpitJSONDataWriter.CACHE_DATE_TIME_FORMAT);

		StringBuilder sb = new StringBuilder();
		sb.append("TO_DATE('");
		sb.append(dateFormat.format(date));
		sb.append("','");
		sb.append(DATE_FORMAT);
		sb.append("')");

		return sb.toString();
	}

	@Override
	protected void append(Sorting item) {
		String aliasDelimiter = database.getAliasDelimiter();
		Projection projection = item.getProjection();
		IAggregationFunction aggregationFunction = projection.getAggregationFunction();

		String name = aliasDelimiter + projection.getName() + aliasDelimiter;
		if (aggregationFunction == null || AggregationFunctions.NONE_FUNCTION.equals(aggregationFunction)) {
			queryBuilder.append(name);
		} else {
			String alias = projection.getAlias();
			if (alias != null && name.equals(aliasDelimiter + alias + aliasDelimiter)) {
				queryBuilder.append(aliasDelimiter + alias + aliasDelimiter);
			} else {
				queryBuilder.append(aggregationFunction.apply(name));
			}
		}

		queryBuilder.append(item.isAscending() ? " ASC" : " DESC");
	}

}
