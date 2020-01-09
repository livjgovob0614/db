package it.eng.spagobi.tools.catalogue.metadata;

import java.util.List;

import it.eng.spagobi.behaviouralmodel.analyticaldriver.bo.AbstractDriver;

public interface IDrivableBIResource<T extends AbstractDriver> {

	/**
	 * Gets the resource parameters.
	 *
	 * @return the resource parameters.
	 */
	public List<? extends AbstractDriver> getDrivers();


	/**
	 * Sets the resource parameters.
	 *
	 * @param resource parameters the new resource parameters
	 */
	public void setDrivers(List<T> drivers);

}
