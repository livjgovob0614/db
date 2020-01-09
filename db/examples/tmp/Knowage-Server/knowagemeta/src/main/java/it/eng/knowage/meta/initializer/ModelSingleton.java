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
package it.eng.knowage.meta.initializer;

import it.eng.knowage.meta.model.Model;

import java.io.IOException;
import java.util.Properties;

import org.eclipse.emf.ecore.util.ECrossReferenceAdapter;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Singleton class for passing object references
 *
 * @author Marco Cortella (marco.cortella@eng.it)
 *
 */
public class ModelSingleton {

	private static ModelSingleton istance;
	private Model model;
	private Object editingDomain;
	private ECrossReferenceAdapter crossReferenceAdapter;

	private static Logger logger = LoggerFactory.getLogger(ModelSingleton.class);

	private ModelSingleton() {
	}

	public static ModelSingleton getInstance() {
		if (istance == null) {
			istance = new ModelSingleton();
		}

		return istance;
	}

	/**
	 * @return the model
	 */
	public Model getModel() {
		return model;
	}

	/**
	 * @param model
	 *            the model to set
	 */
	public void setModel(Model model) {
		this.model = model;
	}

	/**
	 * @return the editingDomain
	 */
	public Object getEditingDomain() {
		return editingDomain;
	}

	/**
	 * @param editingDomain
	 *            the editingDomain to set
	 */
	public void setEditingDomain(Object editingDomain) {
		this.editingDomain = editingDomain;
	}

	/**
	 * @return the crossReferenceAdapter
	 */
	public ECrossReferenceAdapter getCrossReferenceAdapter() {
		return crossReferenceAdapter;
	}

	/**
	 * @param crossReferenceAdapter
	 *            the crossReferenceAdapter to set
	 */
	public void setCrossReferenceAdapter(ECrossReferenceAdapter crossReferenceAdapter) {
		this.crossReferenceAdapter = crossReferenceAdapter;
	}

	public Properties getMetaProperties() {
		logger.debug("IN");
		Properties properties = new Properties();
		try {
			properties.load(this.getClass().getResourceAsStream("/it/eng/knowage/meta/initializer/config/config.properties"));
		} catch (IOException e) {
			logger.error("Error in reading properties file; using Meta settings as default");
		}
		logger.debug("OUT");
		return properties;
	}

	public static boolean readBooleanProperty(Properties properties, String propertyKey) {
		boolean tpreturn = false;
		String value = properties.getProperty(propertyKey, "false");
		if (value.equalsIgnoreCase("true")) {
			tpreturn = true;
		}
		return tpreturn;
	}

}
