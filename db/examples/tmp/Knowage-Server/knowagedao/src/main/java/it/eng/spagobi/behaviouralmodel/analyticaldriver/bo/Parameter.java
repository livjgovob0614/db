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
package it.eng.spagobi.behaviouralmodel.analyticaldriver.bo;

import java.io.Serializable;
import java.util.List;

import javax.validation.constraints.Max;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

import com.fasterxml.jackson.annotation.JsonProperty;

import it.eng.spagobi.behaviouralmodel.lov.bo.ModalitiesValue;
import it.eng.spagobi.services.validation.Xss;

/**
 * Defines a <code>Parameter</code> object.
 *
 * @author sulis
 */

public class Parameter implements Serializable {

	/**
	 *
	 */
	private static final long serialVersionUID = -5755926354664490196L;

	@Xss
	@NotNull
	@Max(value = 11)
	private Integer id;
	@Xss
	@Size(max = 160)
	private String description = "";
	@Xss
	@NotNull
	@Max(value = 6)
	private Integer length;
	@Xss
	@NotNull
	@Size(max = 20)
	private String label = "";
	@Xss
	@NotNull
	@Size(max = 40)
	private String name = "";
	@Xss
	@NotNull
	@Size(max = 20)
	private String type = "";
	@Xss
	@Size(max = 20)
	private String mask = "";
	@Xss
	@NotNull
	@Max(value = 11)
	private Integer typeId;
	private String modality = "";
	private boolean isFunctional;
	private boolean isTemporal;

	private ModalitiesValue modalityValue = null;
	private ModalitiesValue modalityValueForDefault = null;
	private ModalitiesValue modalityValueForMax = null;

	private String defaultFormula = "";

	private String valueSelection;
	private String selectedLayer = "";
	private String selectedLayerProp = "";

	private List checks = null;

	/**
	 * Gets the description.
	 *
	 * @return Returns the description.
	 */
	public String getDescription() {
		return description;
	}

	/**
	 * Sets the description.
	 *
	 * @param description
	 *            The description to set.
	 */
	public void setDescription(String description) {
		this.description = description;
	}

	/**
	 * Gets the id.
	 *
	 * @return Returns the id.
	 */
	public Integer getId() {
		return id;
	}

	/**
	 * Sets the id.
	 *
	 * @param id
	 *            The id to set.
	 */
	public void setId(Integer id) {
		this.id = id;
	}

	/**
	 * Gets the label.
	 *
	 * @return Returns the label.
	 */
	public String getLabel() {
		return label;
	}

	/**
	 * Sets the label.
	 *
	 * @param label
	 *            The label to set.
	 */
	public void setLabel(String label) {
		this.label = label;
	}

	/**
	 * Gets the length.
	 *
	 * @return Returns the length.
	 */
	public Integer getLength() {
		return length;
	}

	/**
	 * Sets the length.
	 *
	 * @param length
	 *            The length to set.
	 */
	public void setLength(Integer length) {
		this.length = length;
	}

	/**
	 * Gets the mask.
	 *
	 * @return Returns the mask.
	 */
	public String getMask() {
		return mask;
	}

	/**
	 * Sets the mask.
	 *
	 * @param mask
	 *            The mask to set.
	 */
	public void setMask(String mask) {
		this.mask = mask;
	}

	/**
	 * Gets the type.
	 *
	 * @return Returns the type.
	 */
	public String getType() {
		return type;
	}

	/**
	 * Sets the type.
	 *
	 * @param type
	 *            The type to set.
	 */
	public void setType(String type) {
		this.type = type;
	}

	/**
	 * Gets the type id.
	 *
	 * @return Returns the typeId.
	 */
	public Integer getTypeId() {
		return typeId;
	}

	/**
	 * Sets the type id.
	 *
	 * @param typeId
	 *            The typeId to set.
	 */
	public void setTypeId(Integer typeId) {
		this.typeId = typeId;
	}

	/**
	 * Gets the modality value.
	 *
	 * @return Returns the modalityValue.
	 */
	public ModalitiesValue getModalityValue() {
		return modalityValue;
	}

	/**
	 * Sets the modality value.
	 *
	 * @param modalityValue
	 *            The modalityValue to set.
	 */
	public void setModalityValue(ModalitiesValue modalityValue) {
		this.modalityValue = modalityValue;
	}

	/**
	 * Gets the modality.
	 *
	 * @return Returns the modality.
	 */
	public String getModality() {
		return modality;
	}

	/**
	 * Sets the modality.
	 *
	 * @param modality
	 *            The modality to set.
	 */
	public void setModality(String modality) {
		this.modality = modality;
	}

	/**
	 * Gets the checks.
	 *
	 * @return Returns the checks.
	 */
	public List getChecks() {
		return checks;
	}

	/**
	 * Sets the checks.
	 *
	 * @param checks
	 *            The checks to set.
	 */
	public void setChecks(List checks) {
		this.checks = checks;
	}

	/**
	 * Gets the name.
	 *
	 * @return the name
	 */
	public String getName() {
		return name;
	}

	/**
	 * Sets the name.
	 *
	 * @param name
	 *            the new name
	 */
	public void setName(String name) {
		this.name = name;
	}

	/**
	 * Checks if is functional.
	 *
	 * @return true, if is functional
	 */
	public boolean isFunctional() {
		return this.isFunctional;
	}

	/**
	 * Sets the checks if is functional.
	 *
	 * @param isFunctional
	 *            the new checks if is functional
	 */
	@JsonProperty(value = "functional")
	public void setIsFunctional(boolean isFunctional) {
		this.isFunctional = isFunctional;
	}

	/**
	 * Checks if the parameter is temporal.
	 *
	 * @return true if the parameter is temporal
	 */
	public boolean isTemporal() {
		return isTemporal;
	}

	/**
	 * Sets the checks if the parameter is temporal.
	 *
	 * @param isTemporal
	 *            the new checks if is temporal
	 */
	@JsonProperty(value = "temporal")
	public void setIsTemporal(boolean isTemporal) {
		this.isTemporal = isTemporal;
	}

	public ModalitiesValue getModalityValueForDefault() {
		return modalityValueForDefault;
	}

	public void setModalityValueForDefault(ModalitiesValue modalityValueForDefault) {
		this.modalityValueForDefault = modalityValueForDefault;
	}

	public ModalitiesValue getModalityValueForMax() {
		return modalityValueForMax;
	}

	public void setModalityValueForMax(ModalitiesValue modalityValueForMax) {
		this.modalityValueForMax = modalityValueForMax;
	}

	public String getDefaultFormula() {
		return this.defaultFormula;
	}

	public void setDefaultFormula(String defaultFormula) {
		this.defaultFormula = defaultFormula;
	}

	public String getValueSelection() {
		return valueSelection;
	}

	public void setValueSelection(String valueSelection) {
		this.valueSelection = valueSelection;
	}

	public String getSelectedLayer() {
		return selectedLayer;
	}

	public void setSelectedLayer(String selectedLayer) {
		this.selectedLayer = selectedLayer;
	}

	public String getSelectedLayerProp() {
		return selectedLayerProp;
	}

	public void setSelectedLayerProp(String selectedLayerProp) {
		this.selectedLayerProp = selectedLayerProp;
	}
	
	@Override
	  public boolean equals(Object v) {
	        boolean retVal = false;

	        if (v instanceof Parameter){
	        	Parameter ptr = (Parameter) v;
	            retVal = ptr.id.longValue() == this.id;
	        }

	     return retVal;
	  }

}