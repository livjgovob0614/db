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
package it.eng.knowage.meta.model.olap.util;

import it.eng.knowage.meta.model.ModelObject;
import it.eng.knowage.meta.model.olap.*;

import java.util.List;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * The <b>Switch</b> for the model's inheritance hierarchy.
 * It supports the call {@link #doSwitch(EObject) doSwitch(object)}
 * to invoke the <code>caseXXX</code> method for each class of the model,
 * starting with the actual class of the object
 * and proceeding up the inheritance hierarchy
 * until a non-null result is returned,
 * which is the result of the switch.
 * <!-- end-user-doc -->
 * @see it.eng.knowage.meta.model.olap.OlapModelPackage
 * @generated
 */
public class OlapModelSwitch<T> {
	/**
	 * The cached model package
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	protected static OlapModelPackage modelPackage;

	/**
	 * Creates an instance of the switch.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public OlapModelSwitch() {
		if (modelPackage == null) {
			modelPackage = OlapModelPackage.eINSTANCE;
		}
	}

	/**
	 * Calls <code>caseXXX</code> for each class of the model until one returns a non null result; it yields that result.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the first non-null result returned by a <code>caseXXX</code> call.
	 * @generated
	 */
	public T doSwitch(EObject theEObject) {
		return doSwitch(theEObject.eClass(), theEObject);
	}

	/**
	 * Calls <code>caseXXX</code> for each class of the model until one returns a non null result; it yields that result.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the first non-null result returned by a <code>caseXXX</code> call.
	 * @generated
	 */
	protected T doSwitch(EClass theEClass, EObject theEObject) {
		if (theEClass.eContainer() == modelPackage) {
			return doSwitch(theEClass.getClassifierID(), theEObject);
		}
		else {
			List<EClass> eSuperTypes = theEClass.getESuperTypes();
			return
				eSuperTypes.isEmpty() ?
					defaultCase(theEObject) :
					doSwitch(eSuperTypes.get(0), theEObject);
		}
	}

	/**
	 * Calls <code>caseXXX</code> for each class of the model until one returns a non null result; it yields that result.
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @return the first non-null result returned by a <code>caseXXX</code> call.
	 * @generated
	 */
	protected T doSwitch(int classifierID, EObject theEObject) {
		switch (classifierID) {
			case OlapModelPackage.OLAP_MODEL: {
				OlapModel olapModel = (OlapModel)theEObject;
				T result = caseOlapModel(olapModel);
				if (result == null) result = caseModelObject(olapModel);
				if (result == null) result = defaultCase(theEObject);
				return result;
			}
			case OlapModelPackage.CUBE: {
				Cube cube = (Cube)theEObject;
				T result = caseCube(cube);
				if (result == null) result = caseModelObject(cube);
				if (result == null) result = defaultCase(theEObject);
				return result;
			}
			case OlapModelPackage.DIMENSION: {
				Dimension dimension = (Dimension)theEObject;
				T result = caseDimension(dimension);
				if (result == null) result = caseModelObject(dimension);
				if (result == null) result = defaultCase(theEObject);
				return result;
			}
			case OlapModelPackage.HIERARCHY: {
				Hierarchy hierarchy = (Hierarchy)theEObject;
				T result = caseHierarchy(hierarchy);
				if (result == null) result = caseModelObject(hierarchy);
				if (result == null) result = defaultCase(theEObject);
				return result;
			}
			case OlapModelPackage.LEVEL: {
				Level level = (Level)theEObject;
				T result = caseLevel(level);
				if (result == null) result = caseModelObject(level);
				if (result == null) result = defaultCase(theEObject);
				return result;
			}
			case OlapModelPackage.MEASURE: {
				Measure measure = (Measure)theEObject;
				T result = caseMeasure(measure);
				if (result == null) result = caseModelObject(measure);
				if (result == null) result = defaultCase(theEObject);
				return result;
			}
			case OlapModelPackage.CALCULATED_MEMBER: {
				CalculatedMember calculatedMember = (CalculatedMember)theEObject;
				T result = caseCalculatedMember(calculatedMember);
				if (result == null) result = caseModelObject(calculatedMember);
				if (result == null) result = defaultCase(theEObject);
				return result;
			}
			case OlapModelPackage.NAMED_SET: {
				NamedSet namedSet = (NamedSet)theEObject;
				T result = caseNamedSet(namedSet);
				if (result == null) result = caseModelObject(namedSet);
				if (result == null) result = defaultCase(theEObject);
				return result;
			}
			case OlapModelPackage.VIRTUAL_CUBE: {
				VirtualCube virtualCube = (VirtualCube)theEObject;
				T result = caseVirtualCube(virtualCube);
				if (result == null) result = caseModelObject(virtualCube);
				if (result == null) result = defaultCase(theEObject);
				return result;
			}
			case OlapModelPackage.VIRTUAL_CUBE_DIMENSION: {
				VirtualCubeDimension virtualCubeDimension = (VirtualCubeDimension)theEObject;
				T result = caseVirtualCubeDimension(virtualCubeDimension);
				if (result == null) result = caseModelObject(virtualCubeDimension);
				if (result == null) result = defaultCase(theEObject);
				return result;
			}
			case OlapModelPackage.VIRTUAL_CUBE_MEASURE: {
				VirtualCubeMeasure virtualCubeMeasure = (VirtualCubeMeasure)theEObject;
				T result = caseVirtualCubeMeasure(virtualCubeMeasure);
				if (result == null) result = caseModelObject(virtualCubeMeasure);
				if (result == null) result = defaultCase(theEObject);
				return result;
			}
			default: return defaultCase(theEObject);
		}
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Olap Model</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Olap Model</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseOlapModel(OlapModel object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Cube</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Cube</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseCube(Cube object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Dimension</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Dimension</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseDimension(Dimension object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Hierarchy</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Hierarchy</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseHierarchy(Hierarchy object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Level</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Level</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseLevel(Level object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Measure</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Measure</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseMeasure(Measure object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Calculated Member</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Calculated Member</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseCalculatedMember(CalculatedMember object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Named Set</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Named Set</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseNamedSet(NamedSet object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Virtual Cube</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Virtual Cube</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseVirtualCube(VirtualCube object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Virtual Cube Dimension</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Virtual Cube Dimension</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseVirtualCubeDimension(VirtualCubeDimension object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Virtual Cube Measure</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Virtual Cube Measure</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseVirtualCubeMeasure(VirtualCubeMeasure object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>Object</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>Object</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject) doSwitch(EObject)
	 * @generated
	 */
	public T caseModelObject(ModelObject object) {
		return null;
	}

	/**
	 * Returns the result of interpreting the object as an instance of '<em>EObject</em>'.
	 * <!-- begin-user-doc -->
	 * This implementation returns null;
	 * returning a non-null result will terminate the switch, but this is the last case anyway.
	 * <!-- end-user-doc -->
	 * @param object the target of the switch.
	 * @return the result of interpreting the object as an instance of '<em>EObject</em>'.
	 * @see #doSwitch(org.eclipse.emf.ecore.EObject)
	 * @generated
	 */
	public T defaultCase(EObject object) {
		return null;
	}

} //OlapModelSwitch
