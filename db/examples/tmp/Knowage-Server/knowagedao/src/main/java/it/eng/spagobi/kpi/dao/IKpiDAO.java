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
package it.eng.spagobi.kpi.dao;

import it.eng.spago.security.IEngUserProfile;
import it.eng.spagobi.commons.bo.Domain;
import it.eng.spagobi.commons.dao.ISpagoBIDao;
import it.eng.spagobi.kpi.bo.Alias;
import it.eng.spagobi.kpi.bo.Cardinality;
import it.eng.spagobi.kpi.bo.Kpi;
import it.eng.spagobi.kpi.bo.KpiExecution;
import it.eng.spagobi.kpi.bo.KpiScheduler;
import it.eng.spagobi.kpi.bo.KpiValue;
import it.eng.spagobi.kpi.bo.KpiValueExecLog;
import it.eng.spagobi.kpi.bo.Placeholder;
import it.eng.spagobi.kpi.bo.Rule;
import it.eng.spagobi.kpi.bo.RuleOutput;
import it.eng.spagobi.kpi.bo.Scorecard;
import it.eng.spagobi.kpi.bo.ScorecardStatus;
import it.eng.spagobi.kpi.bo.Target;
import it.eng.spagobi.kpi.bo.TargetValue;
import it.eng.spagobi.kpi.bo.Threshold;
import it.eng.spagobi.kpi.dao.KpiDAOImpl.STATUS;
import it.eng.spagobi.kpi.metadata.SbiKpiThresholdValue;
import it.eng.spagobi.utilities.exceptions.SpagoBIException;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.hibernate.Session;

public interface IKpiDAO extends ISpagoBIDao {

	/**
	 * Given a list of measures it builds a cardinality matrix (measures/attributes)
	 *
	 * @param measures
	 * @return a list of Cardinality
	 */
	public List<Cardinality> buildCardinality(final List<String> measures);

	/**
	 * Validate a Rule returning errors if any
	 *
	 * @param rule
	 * @return a map of {error key: list of alias}
	 */
	public Map<String, List<String>> aliasValidation(Rule rule);

	/**
	 * Retrieves all rule output filtered by type (MEASURE, ATTRIBUTE, TEMPORAL_ATTRIBUTE) and status (only active / only not active / all records)
	 *
	 * @param type
	 *            (see SbiDomains)
	 * @param status
	 * @return rule output list
	 */
	public List<RuleOutput> listRuleOutputByType(String type, STATUS status);

	public List<Target> listTargetByKpi(Integer kpiId, Integer kpiVersion);

	public List<Target> listTargetByKpi(Integer kpiId, Integer kpiVersion, boolean full);

	/**
	 * Checks if given measure names are really existing on db
	 *
	 * @param measure
	 *            names
	 * @return true if all measures are existing false otherwise
	 */
	public Boolean existsMeasureNames(String... names);

	/**
	 * Retrieves all kpi that are using a threshold
	 *
	 * @param threshold
	 *            id
	 * @return a list of kpi id
	 */
	public List<Integer> listKpiByThreshold(Integer thresholdId);

	/**
	 * Return 'true' if a threshold is used by kpi other then the one with id = kpiId
	 *
	 * @param kpiId
	 * @param thresholdId
	 * @return
	 */
	public boolean isThresholdUsedByOtherKpi(Integer kpiId, Integer thresholdId);

	/**
	 * Return a map of Kpis (id, version, name, active)/measureNames owned by this rule
	 *
	 * @param ruleId
	 * @param ruleVersion
	 * @return
	 */
	public Map<Kpi, List<String>> listKpisLinkedToRule(Integer ruleId, Integer ruleVersion, boolean onlyActiveKpis);

	/**
	 * Saves a new Rule and returns its id
	 *
	 * @param rule
	 * @return rule id
	 * @throws SpagoBIException
	 */
	public Rule insertRule(Rule rule) throws SpagoBIException;

	public Rule importRule(Rule rule, boolean overwriteMode, Session session) throws SpagoBIException;

	public Rule insertNewVersionRule(Rule rule) throws SpagoBIException;

	public Kpi insertNewVersionKpi(Session session, Kpi kpi) throws SpagoBIException;

	public void updateRule(Rule rule) throws SpagoBIException;

	public void removeRule(Integer id, Integer version, boolean toBeVersioned);

	public Rule loadRule(Integer id, Integer version);

	public Rule loadLastActiveRule(Integer id);

	public Rule loadLastActiveRule(Integer id, Session session);

	public Integer getRuleIdByName(String name);

	public Integer getRuleIdByName(String name, Session session);

	public Integer getRuleIdByName(String name, boolean activeOnly);

	public Integer getRuleIdByName(String name, boolean activeOnly, Session session);

	public Integer createDomainIfNotExists(Domain domain);

	/**
	 * Retrieves all kpi filtered by status (only active / only not active / all records)
	 *
	 * @param status
	 * @return
	 */
	public List<Kpi> listKpi(STATUS status, IEngUserProfile profile);

	public List<KpiExecution> listKpiWithResult();

	/**
	 * Retrieves all the KPI values satisfying the specified criteria
	 *
	 * @param kpiId
	 *            The id of the KPI
	 * @param kpiVersion
	 *            The version of the KPI
	 * @param computedAfter
	 *            The datetime after which the KPI values must have been computed (optional, it can be null)
	 * @param includeComputedAfter
	 *            If true, computedAfter is checked for equals (optional)
	 * @param computedBefore
	 *            The datetime before which the KPI values must have been computed (optional, it can be null)
	 * @param includeComputedBefore
	 *            If true, computedBefore is checked for equals (optional)
	 * @param attributesValues
	 *            The KPI attributes name-value pairs
	 * @return The list of KPI values
	 */
	public List<KpiValue> findKpiValues(Integer kpiId, Integer kpiVersion, Date computedAfter, Boolean includeComputedAfter, Date computedBefore,
			Boolean includeComputedBefore, Map<String, String> attributesValues);

	public List<KpiValue> findKpiValues(Integer kpiId, Integer kpiVersion, Date computedAfter, Date computedBefore, Map<String, String> attributesValues);

	public void insertKpiValueExecLog(KpiValueExecLog kpiValueExecLog);

	/**
	 * Insert a new kpi
	 *
	 * @param kpi
	 * @return new generated kpi id
	 */
	public Kpi insertKpi(Kpi kpi);

	/**
	 * Insert or update a KPI
	 *
	 * @param kpi
	 *            The KPI
	 * @param overwriteMode
	 *            Whether or not an existing KPI should be updated (either directly or by creating a new version)
	 * @return The KPI
	 */
	public Kpi importKpi(final Kpi kpi, boolean overwriteMode, Session session);

	/**
	 * Update an existing kpi
	 *
	 * @param kpi
	 * @return
	 */
	public void updateKpi(Kpi kpi);

	public void removeKpi(Integer id, Integer version);

	public Kpi loadKpi(Integer id, Integer version);

	/**
	 * Retrieves a KPI id, searching by its name
	 *
	 * @param the
	 *            KPI name
	 * @return the KPI id, null if not found
	 */
	public Integer getKpiIdByName(String name);

	public Integer getKpiIdByName(String name, Session session);

	public List<Alias> listAlias();

	/**
	 * Retrieve all aliases not currently used as measure in all rules excluding the one with specific id and version
	 *
	 * @param ruleId
	 * @param ruleVersion
	 * @return a list of Alias
	 */
	public List<Alias> listAliasNotInMeasure(Integer ruleId, Integer ruleVersion);

	public List<Alias> listAliasInMeasure(Integer ruleId, Integer ruleVersion);

	/**
	 * Retrieve all aliases not currently used as measure in all rules
	 *
	 * @return a list of Alias
	 */
	public List<Alias> listAliasNotInMeasure();

	public Alias loadAlias(String name);

	public List<Placeholder> listPlaceholder();

	public List<Threshold> listThreshold();

	public Threshold loadThreshold(Integer id);

	public Threshold loadThreshold(Integer id, Session session);

	public Integer getThresholdIdByName(String name);

	public Integer getThresholdIdByName(String name, Session session);

	/**
	 * Given a list of measures it retrieves a list of placeholder related to that measures
	 *
	 * @param measureList
	 * @return a list of placeholder name
	 */
	public List<String> listPlaceholderByMeasures(List<String> measureList);

	/**
	 * Given a kpi id and version, it retrieves a list of placeholder related to its measures
	 *
	 * @param kpi
	 *            id
	 * @param kpi
	 *            version
	 * @return a list of placeholder name
	 */
	public Map<Kpi, List<String>> listPlaceholderByKpiList(List<Kpi> kpis);

	public List<Target> listOverlappingTargets(Integer targetId, Date startDate, Date endDate, Set<Kpi> kpis);

	public List<Target> listTarget();

	public Target loadTarget(Integer id);

	public Target loadTarget(Integer id, Session session);

	public Target loadTargetByName(String name);

	public Target loadTargetByName(String name, Session session);

	public Integer insertTarget(Target target);

	public Integer insertTarget(Target target, Session session);

	public void updateTarget(Target target);

	public void updateTarget(Target target, Session session);

	public void removeTarget(Integer id);

	public List<TargetValue> listKpiWithTarget(Integer targetId);

	public List<KpiScheduler> listKpiScheduler();

	public void removeKpiScheduler(Integer id);

	public KpiScheduler loadKpiScheduler(Integer id);

	public List<Scorecard> listScorecard();

	public List<Scorecard> listScorecardByKpi(Integer kpiId, Integer kpiVersion);

	public List<Scorecard> listScorecardByKpi(Integer kpiId, Integer kpiVersion, boolean full);

	public Scorecard loadScorecard(Integer id);

	public Integer insertScorecard(Scorecard scorecard);

	public void updateScorecard(Scorecard scorecard);

	public void removeScorecard(Integer id);

	public Integer insertScheduler(KpiScheduler scheduler) throws SpagoBIException;

	public Integer updateScheduler(KpiScheduler scheduler) throws SpagoBIException;

	public String valueTargetbyKpi(Kpi kpi);

	/**
	 * Gets a criterion id (ie a domain) and a list of ScorecardStatus and returns a status
	 *
	 * @param scorecardId
	 * @param scorecardStatusLst
	 * @return status
	 */
	public String evaluateScorecardStatus(Integer criterionId, List<ScorecardStatus> scorecardStatusLst);

	public void editKpiValue(Integer id, double value, String comment);

	public List<KpiScheduler> listSchedulerByKpi(Integer kpiId, Integer kpiVersion);

	public List<KpiScheduler> listSchedulerAndFiltersByKpi(Integer kpiId, Integer kpiVersion, boolean showFilters);

	public Kpi loadLastActiveKpi(Integer id);

	public Kpi loadLastActiveKpi(Integer id, Session session);

	public ArrayList<KpiValueExecLog> loadKpiValueExecLog(final Integer id, final Integer number);

	public KpiValueExecLog loadlogExecutionListOutputContent(Integer id);

	/**
	 * Gets the last time run date with given kpiId
	 *
	 * @param kpiId
	 * @return timeRun
	 */
	public Date loadLastKpiValueTimeRunByKpiId(final Integer kpiId);

	public List<SbiKpiThresholdValue> listThresholdValueByIds(Collection<Integer> thresholdValueIds);

	public Scorecard loadScorecard(Integer id, Map<String, String> attributesValues);

	public Kpi loadLastActiveKpiByName(String name);

	public Scorecard loadScorecardByName(String name);

	public Scorecard loadScorecardByName(String name, Map<String, String> attributesValues);
}
