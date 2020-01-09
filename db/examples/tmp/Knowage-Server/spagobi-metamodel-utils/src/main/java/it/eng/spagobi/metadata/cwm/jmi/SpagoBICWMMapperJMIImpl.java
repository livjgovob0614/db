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
package it.eng.spagobi.metadata.cwm.jmi;

import it.eng.spagobi.meta.cwm.jmi.spagobi.meta.core.CwmClassifier;
import it.eng.spagobi.meta.cwm.jmi.spagobi.meta.core.CwmNamespace;
import it.eng.spagobi.meta.cwm.jmi.spagobi.meta.keysindexes.CwmUniqueKey;
import it.eng.spagobi.meta.cwm.jmi.spagobi.meta.relational.CwmCatalog;
import it.eng.spagobi.meta.cwm.jmi.spagobi.meta.relational.CwmColumn;
import it.eng.spagobi.meta.cwm.jmi.spagobi.meta.relational.CwmForeignKey;
import it.eng.spagobi.meta.cwm.jmi.spagobi.meta.relational.CwmPrimaryKey;
import it.eng.spagobi.meta.cwm.jmi.spagobi.meta.relational.CwmSchema;
import it.eng.spagobi.meta.cwm.jmi.spagobi.meta.relational.CwmSqlsimpleType;
import it.eng.spagobi.meta.cwm.jmi.spagobi.meta.relational.CwmTable;
import it.eng.spagobi.meta.model.physical.PhysicalColumn;
import it.eng.spagobi.meta.model.physical.PhysicalForeignKey;
import it.eng.spagobi.meta.model.physical.PhysicalModel;
import it.eng.spagobi.meta.model.physical.PhysicalModelFactory;
import it.eng.spagobi.meta.model.physical.PhysicalPrimaryKey;
import it.eng.spagobi.meta.model.physical.PhysicalTable;
import it.eng.spagobi.metadata.cwm.ICWM;
import it.eng.spagobi.metadata.cwm.ICWMMapper;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @author Marco Cortella (marco.cortella@eng.it)
 * @author Andrea Gioia (andrea.gioia@eng.it)
 *
 */
public class SpagoBICWMMapperJMIImpl extends ICWMMapper {

	static public PhysicalModelFactory FACTORY = PhysicalModelFactory.eINSTANCE;

	// -----------------------------------------------------------------------------
	// DECODE - From CWM Metamodel to SpagoBI Metamodel
	// -----------------------------------------------------------------------------

	@Override
	public PhysicalModel decodeICWM(ICWM cwm) {
		return decodeModel((SpagoBICWMJMIImpl) cwm);
	}

	public PhysicalModel decodeModel(SpagoBICWMJMIImpl cwm) {
		PhysicalModel model = FACTORY.createPhysicalModel();
		// set model name
		model.setName(cwm.getName());
		// set catalog and schema
		model.setCatalog(cwm.getCatalog().getName());
		model.setSchema(cwm.getSchema().getName());

		// Table creation
		Collection<CwmTable> tables = cwm.getTables();
		for (CwmTable table : tables) {
			// create a physical table
			PhysicalTable physicalTable = FACTORY.createPhysicalTable();
			physicalTable.setModel(model);
			physicalTable.setName(table.getName());
			// add columns of this table
			addColumns(cwm, table, model, physicalTable);
		}

		// Primary Keys creation
		Collection<CwmPrimaryKey> primaryKeys = cwm.getPrimaryKeys();
		for (CwmPrimaryKey primaryKey : primaryKeys) {
			PhysicalPrimaryKey physicalPK = FACTORY.createPhysicalPrimaryKey();
			String name = primaryKey.getName();
			physicalPK.setName(name);
			List features = primaryKey.getFeature();
			CwmNamespace parent = primaryKey.getNamespace();
			if (parent instanceof CwmTable) {
				CwmTable cwmTable = (CwmTable) parent;
				PhysicalTable physicalTable = model.getTable(cwmTable.getName());
				if (physicalTable != null) {
					// set the physical table of the pk
					physicalPK.setTable(physicalTable);

					// find the columns part of the pk
					for (Object feature : features) {
						if (feature instanceof CwmColumn) {
							CwmColumn cwmColumn = (CwmColumn) feature;
							PhysicalColumn physicalColumn = physicalTable.getColumn(cwmColumn.getName());
							if (physicalColumn != null) {
								// add column to pk
								physicalPK.getColumns().add(physicalColumn);
							}
						}
					}
				}
			}
			model.getPrimaryKeys().add(physicalPK);

		}

		// Foreign Keys creation
		Collection<CwmForeignKey> foreignKeys = cwm.getForeignKeys();
		for (CwmForeignKey foreignKey : foreignKeys) {
			PhysicalForeignKey physicalForeignKey = FACTORY.createPhysicalForeignKey();
			physicalForeignKey.setName(foreignKey.getName());
			physicalForeignKey.setSourceName(foreignKey.getName());
			CwmNamespace cwmNamespace = foreignKey.getNamespace();
			// set source table of fk
			if (cwmNamespace instanceof CwmTable) {
				CwmTable cwmSourceTable = (CwmTable) cwmNamespace;
				PhysicalTable physicalTable = model.getTable(cwmSourceTable.getName());
				if (physicalTable != null) {
					physicalForeignKey.setSourceTable(physicalTable);
				}
				// get source columns
				List features = foreignKey.getFeature();
				for (Object feature : features) {
					if (feature instanceof CwmColumn) {
						CwmColumn cwmColumn = (CwmColumn) feature;
						PhysicalColumn physicalColumn = physicalTable.getColumn(cwmColumn.getName());
						if (physicalColumn != null) {
							physicalForeignKey.getSourceColumns().add(physicalColumn);
						}
					}
				}
			}

			// set target table of fk
			CwmUniqueKey cwmUniqueKey = foreignKey.getUniqueKey();
			if (cwmUniqueKey instanceof CwmPrimaryKey) {
				CwmPrimaryKey cwmPrimaryKey = (CwmPrimaryKey) cwmUniqueKey;
				physicalForeignKey.setDestinationName(cwmPrimaryKey.getName());
				CwmNamespace cwmNamespacePK = cwmPrimaryKey.getNamespace();
				if (cwmNamespacePK instanceof CwmTable) {
					CwmTable cwmTable = (CwmTable) cwmNamespacePK;
					PhysicalTable pkPhysicalTable = model.getTable(cwmTable.getName());
					physicalForeignKey.setDestinationTable(pkPhysicalTable);
					// set target columns
					List features = cwmPrimaryKey.getFeature();
					for (Object feature : features) {
						if (feature instanceof CwmColumn) {
							CwmColumn cwmColumn = (CwmColumn) feature;
							PhysicalColumn pkPhysicalColumn = pkPhysicalTable.getColumn(cwmColumn.getName());
							if (pkPhysicalColumn != null) {
								physicalForeignKey.getDestinationColumns().add(pkPhysicalColumn);
							}
						}
					}
				}
				model.getForeignKeys().add(physicalForeignKey);
			}
		}

		return model;
	}

	public void addColumns(SpagoBICWMJMIImpl cwm, CwmTable cwmTable, PhysicalModel model, PhysicalTable physicalTable) {
		Collection<CwmColumn> cwmColumns = cwm.getColumns(cwmTable);
		for (CwmColumn cwmColumn : cwmColumns) {
			PhysicalColumn physicalColumn = FACTORY.createPhysicalColumn();
			physicalColumn.setName(cwmColumn.getName());
			physicalColumn.setSize(cwmColumn.getLength());
			CwmClassifier cwmClassifer = cwmColumn.getType();
			if (cwmClassifer instanceof CwmSqlsimpleType) {
				CwmSqlsimpleType cwmType = ((CwmSqlsimpleType) cwmClassifer);
				String typeName = cwmType.getName();
				physicalColumn.setDataType(typeName);
			}

			// add column to physical table
			physicalTable.getColumns().add(physicalColumn);
		}
	}

	// -----------------------------------------------------------------------------
	// ENCODE - From SpagoBI Metamodel to CWM Metamodel
	// -----------------------------------------------------------------------------

	@Override
	public SpagoBICWMJMIImpl encodeICWM(PhysicalModel model) {

		SpagoBICWMJMIImpl cwm = new SpagoBICWMJMIImpl(model.getName());
		// Create Catalog
		CwmCatalog catalog = cwm.createCatalog(model.getCatalog());
		// Create Schema and attach to catalog
		CwmSchema schema = cwm.createSchema(model.getSchema());
		catalog.getOwnedElement().add(schema);
		schema.setNamespace(catalog);

		// Create tables and attach them to the schema
		List<PhysicalTable> tables = model.getTables();
		CwmTable cwmTable;
		Collection<CwmTable> ts = schema.getOwnedElement();
		// Map used to keep track of pk and fk for each table
		Map<PhysicalTable, CwmPrimaryKey> addedCwmPrimaryKeys = new HashMap<PhysicalTable, CwmPrimaryKey>();
		Map<PhysicalTable, List<CwmForeignKey>> addedCwmForeignKeys = new HashMap<PhysicalTable, List<CwmForeignKey>>();

		for (int i = 0; i < tables.size(); i++) {
			cwmTable = encodeTable(cwm, tables.get(i));
			ts.add(cwmTable);
			cwmTable.setNamespace(schema);
			// Check and Create primary keys
			PhysicalPrimaryKey physicalPrimaryKey = model.getPrimaryKey(tables.get(i));
			CwmPrimaryKey cwmPrimaryKey = null;
			List<PhysicalColumn> primaryKeyColumns = null;
			if (physicalPrimaryKey != null) {
				cwmPrimaryKey = encodePrimaryKey(cwm, physicalPrimaryKey.getName());
				addedCwmPrimaryKeys.put(tables.get(i), cwmPrimaryKey);
				primaryKeyColumns = physicalPrimaryKey.getColumns();
			}
			// Check and create foreign keys

			List<PhysicalForeignKey> physicalForeignKeys = model.getForeignKeys(tables.get(i));
			List<CwmForeignKey> cwmForeignKeys = new ArrayList<CwmForeignKey>();
			for (PhysicalForeignKey physicalForeignKey : physicalForeignKeys) {
				CwmForeignKey cwmForeignKey = encodeForeignKey(cwm, physicalForeignKey.getName());
				cwmForeignKey.setNamespace(cwmTable);
				cwmTable.getOwnedElement().add(cwmForeignKey);
				cwmForeignKeys.add(cwmForeignKey);
				addedCwmForeignKeys.put(tables.get(i), cwmForeignKeys);
			}

			// Create columns and attach to table
			List<PhysicalColumn> columns = tables.get(i).getColumns();
			for (PhysicalColumn column : columns) {
				CwmColumn cwmColumn = encodeColumn(cwm, column);
				cwmColumn.setLength(column.getSize());
				cwmColumn.setType(encodeSQLSimpleType(cwm, column.getDataType()));
				cwmColumn.setOwner(cwmTable);
				cwmTable.getFeature().add(cwmColumn);

				if (physicalPrimaryKey != null) {
					if (primaryKeyColumns.contains(column)) {
						// column is part of a pk, add it to the cwmPrimaryKey
						cwmPrimaryKey.getFeature().add(cwmColumn);
					}
				}

				// check if column is used in a foreign key (source columns)
				for (PhysicalForeignKey physicalForeignKey : physicalForeignKeys) {
					List<PhysicalColumn> fkSourceColumns = physicalForeignKey.getSourceColumns();
					if (fkSourceColumns.contains(column)) {
						for (CwmForeignKey cwmForeignKey : cwmForeignKeys) {
							if (cwmForeignKey.getName().equals(physicalForeignKey.getName())) {
								cwmForeignKey.getFeature().add(cwmColumn);
							}
						}
					}
				}
			}
			if (physicalPrimaryKey != null) {
				cwmTable.getOwnedElement().add(cwmPrimaryKey);
				cwmPrimaryKey.setNamespace(cwmTable);
			}
		}

		// Set the the referenced primary keys of the foreign keys
		// scan all the tables
		for (PhysicalTable physicalTable : tables) {
			// get foreign keys of specific table
			List<PhysicalForeignKey> physicalForeignKeys = model.getForeignKeys(physicalTable);
			// scan all foreign keys
			for (PhysicalForeignKey physicalForeignKey : physicalForeignKeys) {
				PhysicalTable destinationPhysicalTable = physicalForeignKey.getDestinationTable();
				// get corresponding cwmTable
				for (CwmTable destinationCwmTable : ts) {
					if (destinationCwmTable.getName().equals(destinationPhysicalTable.getName())) {
						// retrieve primary key of destination table
						CwmPrimaryKey destTableCwmPk = addedCwmPrimaryKeys.get(destinationPhysicalTable);
						if (destTableCwmPk != null) {
							List<CwmForeignKey> cwmForeignKeys = addedCwmForeignKeys.get(physicalTable);
							// search specific fk
							if (cwmForeignKeys != null) {
								for (CwmForeignKey cwmForeignKey : cwmForeignKeys) {
									if (cwmForeignKey.getName().equals(physicalForeignKey.getName())) {
										// this pk will be added to this fk
										cwmForeignKey.setUniqueKey(destTableCwmPk);
										destTableCwmPk.getKeyRelationship().add(cwmForeignKey);
									}
								}
							}
						}

					}
				}
			}
		}

		return cwm;
	}

	public CwmTable encodeTable(SpagoBICWMJMIImpl cwm, PhysicalTable table) {
		CwmTable t;
		t = cwm.createTable(table.getName());
		return t;
	}

	public CwmColumn encodeColumn(SpagoBICWMJMIImpl cwm, PhysicalColumn column) {
		CwmColumn c;
		c = cwm.createColumn(column.getName());

		return c;
	}

	public CwmSqlsimpleType encodeSQLSimpleType(SpagoBICWMJMIImpl cwm, String name) {
		CwmSqlsimpleType t;
		t = cwm.createSQLSimpleType(name);
		return t;
	}

	public CwmPrimaryKey encodePrimaryKey(SpagoBICWMJMIImpl cwm, String name) {
		CwmPrimaryKey pk;
		pk = cwm.createPrimaryKey(name);
		return pk;
	}

	public CwmForeignKey encodeForeignKey(SpagoBICWMJMIImpl cwm, String name) {
		CwmForeignKey fk;
		fk = cwm.createForeignKey(name);
		return fk;
	}

}
