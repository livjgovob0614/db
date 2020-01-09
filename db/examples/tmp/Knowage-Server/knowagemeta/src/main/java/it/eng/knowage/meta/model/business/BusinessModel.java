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
package it.eng.knowage.meta.model.business;

import it.eng.knowage.meta.model.Model;
import it.eng.knowage.meta.model.ModelObject;
import it.eng.knowage.meta.model.physical.PhysicalModel;
import it.eng.knowage.meta.model.physical.PhysicalTable;

import java.util.List;

import org.eclipse.emf.common.util.EList;

/**
 * <!-- begin-user-doc --> A representation of the model object '<em><b>Business Model</b></em>'. <!-- end-user-doc -->
 * 
 * <p>
 * The following features are supported:
 * <ul>
 * <li>{@link it.eng.knowage.meta.model.business.BusinessModel#getParentModel <em>Parent Model</em>}</li>
 * <li>{@link it.eng.knowage.meta.model.business.BusinessModel#getPhysicalModel <em>Physical Model</em>}</li>
 * <li>{@link it.eng.knowage.meta.model.business.BusinessModel#getTables <em>Tables</em>}</li>
 * <li>{@link it.eng.knowage.meta.model.business.BusinessModel#getRelationships <em>Relationships</em>}</li>
 * <li>{@link it.eng.knowage.meta.model.business.BusinessModel#getIdentifiers <em>Identifiers</em>}</li>
 * <li>{@link it.eng.knowage.meta.model.business.BusinessModel#getDomains <em>Domains</em>}</li>
 * <li>{@link it.eng.knowage.meta.model.business.BusinessModel#getJoinRelationships <em>Join Relationships</em>}</li>
 * </ul>
 * </p>
 * 
 * @see it.eng.knowage.meta.model.business.BusinessModelPackage#getBusinessModel()
 * @model
 * @generated
 */
public interface BusinessModel extends ModelObject {
	/**
	 * Returns the value of the '<em><b>Parent Model</b></em>' container reference. It is bidirectional and its opposite is '
	 * {@link it.eng.knowage.meta.model.Model#getBusinessModels <em>Business Models</em>}'. <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Parent Model</em>' reference isn't clear, there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * 
	 * @return the value of the '<em>Parent Model</em>' container reference.
	 * @see #setParentModel(Model)
	 * @see it.eng.knowage.meta.model.business.BusinessModelPackage#getBusinessModel_ParentModel()
	 * @see it.eng.knowage.meta.model.Model#getBusinessModels
	 * @model opposite="businessModels" required="true" transient="false"
	 * @generated
	 */
	Model getParentModel();

	/**
	 * Sets the value of the '{@link it.eng.knowage.meta.model.business.BusinessModel#getParentModel <em>Parent Model</em>}' container reference. <!--
	 * begin-user-doc --> <!-- end-user-doc -->
	 * 
	 * @param value
	 *            the new value of the '<em>Parent Model</em>' container reference.
	 * @see #getParentModel()
	 * @generated
	 */
	void setParentModel(Model value);

	/**
	 * Returns the value of the '<em><b>Physical Model</b></em>' reference. <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Physical Model</em>' reference isn't clear, there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * 
	 * @return the value of the '<em>Physical Model</em>' reference.
	 * @see #setPhysicalModel(PhysicalModel)
	 * @see it.eng.knowage.meta.model.business.BusinessModelPackage#getBusinessModel_PhysicalModel()
	 * @model required="true"
	 * @generated
	 */
	PhysicalModel getPhysicalModel();

	/**
	 * Sets the value of the '{@link it.eng.knowage.meta.model.business.BusinessModel#getPhysicalModel <em>Physical Model</em>}' reference. <!-- begin-user-doc
	 * --> <!-- end-user-doc -->
	 * 
	 * @param value
	 *            the new value of the '<em>Physical Model</em>' reference.
	 * @see #getPhysicalModel()
	 * @generated
	 */
	void setPhysicalModel(PhysicalModel value);

	/**
	 * Returns the value of the '<em><b>Tables</b></em>' containment reference list. The list contents are of type
	 * {@link it.eng.knowage.meta.model.business.BusinessColumnSet}. It is bidirectional and its opposite is '
	 * {@link it.eng.knowage.meta.model.business.BusinessColumnSet#getModel <em>Model</em>}'. <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Tables</em>' reference list isn't clear, there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * 
	 * @return the value of the '<em>Tables</em>' containment reference list.
	 * @see it.eng.knowage.meta.model.business.BusinessModelPackage#getBusinessModel_Tables()
	 * @see it.eng.knowage.meta.model.business.BusinessColumnSet#getModel
	 * @model opposite="model" containment="true"
	 * @generated
	 */
	EList<BusinessColumnSet> getTables();

	/**
	 * Returns the value of the '<em><b>Relationships</b></em>' containment reference list. The list contents are of type
	 * {@link it.eng.knowage.meta.model.business.BusinessRelationship}. It is bidirectional and its opposite is '
	 * {@link it.eng.knowage.meta.model.business.BusinessRelationship#getModel <em>Model</em>}'. <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Relationships</em>' reference list isn't clear, there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * 
	 * @return the value of the '<em>Relationships</em>' containment reference list.
	 * @see it.eng.knowage.meta.model.business.BusinessModelPackage#getBusinessModel_Relationships()
	 * @see it.eng.knowage.meta.model.business.BusinessRelationship#getModel
	 * @model opposite="model" containment="true"
	 * @generated
	 */
	EList<BusinessRelationship> getRelationships();

	/**
	 * Returns the value of the '<em><b>Identifiers</b></em>' containment reference list. The list contents are of type
	 * {@link it.eng.knowage.meta.model.business.BusinessIdentifier}. It is bidirectional and its opposite is '
	 * {@link it.eng.knowage.meta.model.business.BusinessIdentifier#getModel <em>Model</em>}'. <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Identifiers</em>' containment reference list isn't clear, there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * 
	 * @return the value of the '<em>Identifiers</em>' containment reference list.
	 * @see it.eng.knowage.meta.model.business.BusinessModelPackage#getBusinessModel_Identifiers()
	 * @see it.eng.knowage.meta.model.business.BusinessIdentifier#getModel
	 * @model opposite="model" containment="true"
	 * @generated
	 */
	EList<BusinessIdentifier> getIdentifiers();

	/**
	 * Returns the value of the '<em><b>Domains</b></em>' containment reference list. The list contents are of type
	 * {@link it.eng.knowage.meta.model.business.BusinessDomain}. It is bidirectional and its opposite is '
	 * {@link it.eng.knowage.meta.model.business.BusinessDomain#getModel <em>Model</em>}'. <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Domains</em>' containment reference list isn't clear, there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * 
	 * @return the value of the '<em>Domains</em>' containment reference list.
	 * @see it.eng.knowage.meta.model.business.BusinessModelPackage#getBusinessModel_Domains()
	 * @see it.eng.knowage.meta.model.business.BusinessDomain#getModel
	 * @model opposite="model" containment="true"
	 * @generated
	 */
	EList<BusinessDomain> getDomains();

	/**
	 * Returns the value of the '<em><b>Join Relationships</b></em>' containment reference list. The list contents are of type
	 * {@link it.eng.knowage.meta.model.business.BusinessViewInnerJoinRelationship}. It is bidirectional and its opposite is '
	 * {@link it.eng.knowage.meta.model.business.BusinessViewInnerJoinRelationship#getModel <em>Model</em>}'. <!-- begin-user-doc -->
	 * <p>
	 * If the meaning of the '<em>Join Relationships</em>' containment reference list isn't clear, there really should be more of a description here...
	 * </p>
	 * <!-- end-user-doc -->
	 * 
	 * @return the value of the '<em>Join Relationships</em>' containment reference list.
	 * @see it.eng.knowage.meta.model.business.BusinessModelPackage#getBusinessModel_JoinRelationships()
	 * @see it.eng.knowage.meta.model.business.BusinessViewInnerJoinRelationship#getModel
	 * @model opposite="model" containment="true"
	 * @generated
	 */
	EList<BusinessViewInnerJoinRelationship> getJoinRelationships();

	// =========================================================================
	// Utility methods
	// =========================================================================

	/**
	 * @return the identifier of the given column set (BusinessTable or BusinessView)
	 */
	BusinessIdentifier getIdentifier(BusinessColumnSet table);

	// =========================================
	// TABLES (BUSINESS TABLES + BUSINESS VIEWS)
	// =========================================

	/**
	 * Returns the tables (business table or business view) whose unique name is equal to uniqueName if any, null otherwise.
	 * 
	 * @param uniqueName
	 *            the unique name of the table to look for
	 * @return the table whose unique name is equal to uniqueName if any, null otherwise.
	 */
	BusinessColumnSet getTableByUniqueName(String uniqueName);

	/**
	 * @deprecated This method is conceptually wrong because to one table name can be associated multiple Tables. This method implementation return only the
	 *             first table whose name match the name parameter. Use getTableByUniqueName instead.
	 * 
	 * @param name
	 *            the name of the table to look for
	 * @return the first table whose name is equal to parameter name
	 */
	@Deprecated
	BusinessColumnSet getTable(String name);

	// =========================================
	// BUSINESS TABLES
	// =========================================

	/**
	 * @return all the the business tables contained in the model
	 */
	List<BusinessTable> getBusinessTables();

	/**
	 * Returns the business table (business view are not included in the search) whose unique name is equal to uniqueName if any, null otherwise.
	 * 
	 * @param uniqueName
	 *            the unique name of the business table to look for
	 * @return the business table whose unique name is equal to uniqueName if any, null otherwise.
	 */
	BusinessTable getBusinessTableByUniqueName(String uniqueName);

	/**
	 * Return a list of all business tables whose name is equal to '<em>name</em>'.
	 * 
	 * @param physicalTable
	 *            the name to search for
	 * @return a list of all business tables whose name is equal to '<em>name</em>'. Never return null. If there are no business tables associated with the
	 *         given physical '<em>name</em>' an empty list will be returned.
	 */
	List<BusinessTable> getBusinessTableByName(String name);

	/**
	 * Return a list of all business tables associated with the given physicalTable.
	 * 
	 * @param physicalTable
	 *            a physicalTable
	 * @return a list of all business tables associated with the given physicalTable. Never return null. If there are no business tables associated with the
	 *         given physical table an empty list will be returned.
	 */
	List<BusinessTable> getBusinessTableByPhysicalTable(PhysicalTable physicalTable);

	/**
	 * Return a list of all business tables associated with the the physical table whose name is equal to name.
	 * 
	 * @param name
	 *            the name of the target physical table
	 * @return a list of all business tables associated with the the physical table whose name is equal to name. Never return null. If there are no business
	 *         tables associated with the given physical table an empty list will be returned.
	 */
	List<BusinessTable> getBusinessTableByPhysicalTable(String name);

	/**
	 * Delete the businessTable whose unique name is equal to parameter uniqueName.
	 * 
	 * @param uniqueName
	 *            the unique name of the business table to delete
	 * @return true if a table with the given unique name exists false otherwise
	 */
	boolean deleteBusinessTableByUniqueName(String uniqueName);

	// =========================================
	// BUSINESS VIEWS
	// =========================================

	/**
	 * @return all the the business views contained in the model
	 */
	List<BusinessView> getBusinessViews();

	/**
	 * Return a list of all business view associated with the the physical table whose name is equal to name.
	 * 
	 * @param name
	 *            the name of the target physical table
	 * @return a list of all business tables associated with the the physical table whose name is equal to name. Never return null. If there are no business
	 *         tables associated with the given physical table an empty list will be returned.
	 */
	BusinessView getBusinessViewByUniqueName(String name);

	/**
	 * Delete the businessView whose unique name is equal to parameter uniqueName.
	 * 
	 * @param uniqueName
	 *            the unique name of the business table to delete
	 * @return true if a table with the given unique name exists false otherwise
	 */
	boolean deleteBusinessViewByUniqueName(String uniqueName);

	void addBusinessView(BusinessView businessView);

} // BusinessModel
