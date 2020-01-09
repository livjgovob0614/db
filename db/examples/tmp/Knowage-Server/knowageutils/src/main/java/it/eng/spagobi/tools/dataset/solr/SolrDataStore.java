/*
 * Knowage, Open Source Business Intelligence suite
 * Copyright (C) 2018 Engineering Ingegneria Informatica S.p.A.
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
 *
 */

package it.eng.spagobi.tools.dataset.solr;

import it.eng.spagobi.tools.dataset.common.datastore.DataStore;
import it.eng.spagobi.tools.dataset.common.datastore.IDataStore;
import org.apache.log4j.Logger;

import java.util.HashMap;
import java.util.Map;

public class SolrDataStore extends DataStore {

    private static final Logger logger = Logger.getLogger(SolrDataStore.class);
    
    private Map<String, IDataStore> facets = new HashMap<>();

    public SolrDataStore(IDataStore documentDataStore) {
        records = documentDataStore.getRecords();
        metaData = documentDataStore.getMetaData();
    }

    public  Map<String, IDataStore> getFacets() {
        return facets;
    }

    public void setFacets(Map<String, IDataStore> facets) {
        this.facets = facets;
    }

    public void addFacetDataStore(String name, IDataStore dataStore) {
        facets.put(name, dataStore);
    }
}
