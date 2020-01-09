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
package it.eng.spagobi.analiticalmodel.execution.service;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.log4j.Logger;
import org.json.JSONArray;
import org.json.JSONObject;

import it.eng.spago.error.EMFUserError;
import it.eng.spago.security.IEngUserProfile;
import it.eng.spagobi.analiticalmodel.document.AnalyticalModelDocumentManagementAPI;
import it.eng.spagobi.analiticalmodel.document.DocumentTemplateBuilder;
import it.eng.spagobi.analiticalmodel.document.bo.BIObject;
import it.eng.spagobi.analiticalmodel.document.bo.ObjTemplate;
import it.eng.spagobi.analiticalmodel.document.dao.IBIObjectDAO;
import it.eng.spagobi.analiticalmodel.document.handlers.ExecutionInstance;
import it.eng.spagobi.analiticalmodel.functionalitytree.bo.LowFunctionality;
import it.eng.spagobi.analiticalmodel.functionalitytree.dao.ILowFunctionalityDAO;
import it.eng.spagobi.commons.bo.Domain;
import it.eng.spagobi.commons.bo.UserProfile;
import it.eng.spagobi.commons.constants.SpagoBIConstants;
import it.eng.spagobi.commons.dao.DAOFactory;
import it.eng.spagobi.commons.services.AbstractSpagoBIAction;
import it.eng.spagobi.commons.utilities.StringUtilities;
import it.eng.spagobi.commons.utilities.UserUtilities;
import it.eng.spagobi.engines.config.bo.Engine;
import it.eng.spagobi.engines.config.dao.IEngineDAO;
import it.eng.spagobi.metadata.metadata.SbiMetaObjDs;
import it.eng.spagobi.metadata.metadata.SbiMetaObjDsId;
import it.eng.spagobi.tools.dataset.bo.IDataSet;
import it.eng.spagobi.tools.dataset.bo.VersionedDataSet;
import it.eng.spagobi.utilities.assertion.Assert;
import it.eng.spagobi.utilities.exceptions.SpagoBIRuntimeException;
import it.eng.spagobi.utilities.exceptions.SpagoBIServiceException;
import it.eng.spagobi.utilities.service.JSONSuccess;

public class SaveDocumentAction extends AbstractSpagoBIAction {

	private static final long serialVersionUID = 1L;

	// logger component
	private static Logger logger = Logger.getLogger(SaveDocumentAction.class);

	private final String MESSAGE_DET = "MESSAGE_DET";
	// type of service
	private final String DOC_SAVE = "DOC_SAVE";
	private final String DOC_UPDATE = "DOC_UPDATE";
	private final String MODIFY_GEOREPORT = "MODIFY_GEOREPORT";
	private final String MODIFY_COCKPIT = "MODIFY_COCKPIT";

	// RES detail
	private final String ID = "id";
	private final String OBJ_ID = "obj_id";
	private final String NAME = "name";
	private final String LABEL = "label";
	private final String DESCRIPTION = "description";
	private final String ENGINE = "engineid";
	private final String TYPE = "typeid";
	private final String TEMPLATE = "template";
	private final String FUNCTS = "functs";
	private final String PREVIEW_FILE = "previewFile";
	private final String BUSINESS_METADATA = "business_metadata";
	private final String OBJECT_WK_DEFINITION = "wk_definition";
	private final String OBJECT_QUERY = "query";
	private final String FORMVALUES = "formValues";
	private final String VISIBILITY = "visibility";
	private final String COMMUNITY = "communityId";

	public static final String OBJ_DATASET_ID = "dataSetId";
	public static final String OBJ_DATASET_LABEL = "dataset_label";

	private AnalyticalModelDocumentManagementAPI documentManagementAPI;

	// default type
	public static final String BIOBJ_TYPE_DOMAIN_CD = "BIOBJ_TYPE";

	// default for parameters
	public static final Integer REQUIRED = 0;
	public static final Integer MODIFIABLE = 1;
	public static final Integer MULTIVALUE = 0;
	public static final Integer VISIBLE = 1;

	// TODO move away from global
	String action;

	@Override
	public void doService() {

		logger.debug("IN");

		try {
			documentManagementAPI = new AnalyticalModelDocumentManagementAPI(getUserProfile());
		} catch (Throwable t) {
			throw new SpagoBIServiceException(SERVICE_NAME, "sbi.document.dao.instatiation.error", t);
		}

		JSONObject request = parseRequest();
		action = request.optString("action");
		logger.debug("Action type is equal to [" + action + "]");

		try {

			if (DOC_SAVE.equalsIgnoreCase(action)) {
				doInsertDocument(request);
			} else if (DOC_UPDATE.equalsIgnoreCase(action)) {
				logger.error("DOC_UPDATE action is no more supported");
				throw new SpagoBIServiceException(SERVICE_NAME, "sbi.document.unsupported.udpateaction");
			} else if (MODIFY_GEOREPORT.equalsIgnoreCase(action) || MODIFY_COCKPIT.equalsIgnoreCase(action)) {
				doModifyDocument(request);
			} else {
				throw new SpagoBIServiceException(SERVICE_NAME, "sbi.document.unsupported.action");
			}

			JSONObject attributesResponseSuccessJSON = new JSONObject();
			attributesResponseSuccessJSON.put("success", true);
			attributesResponseSuccessJSON.put("responseText", "Operation succeded");
			writeBackToClient(new JSONSuccess(attributesResponseSuccessJSON));
		} catch (SpagoBIServiceException e) {
			throw e;
		} catch (Exception e) {
			throw new SpagoBIServiceException(SERVICE_NAME, "sbi.document.saveError", e);
		} finally {
			logger.debug("OUT");
		}

	}

	private void doInsertDocument(JSONObject request) {

		logger.debug("IN");

		try {

			JSONObject documentJSON = request.optJSONObject("document");
			Assert.assertNotNull(StringUtilities.isNotEmpty(documentJSON.optString("name")), "Document's name cannot be null or empty");
			Assert.assertNotNull(StringUtilities.isNotEmpty(documentJSON.optString("label")), "Document's label cannot be null or empty");
			Assert.assertNotNull(StringUtilities.isNotEmpty(documentJSON.optString("type")), "Document's type cannot be null or empty");

			JSONArray foldersJSON = request.optJSONArray("folders");

			if (documentManagementAPI.getDocument(documentJSON.getString("label")) != null) {
				throw new SpagoBIServiceException(SERVICE_NAME, "sbi.document.labelAlreadyExistent");
			}

			String type = documentJSON.getString("type");
			if ("MAP".equalsIgnoreCase(type)) {
				insertGeoreportDocument(request);
			} else if ("DOCUMENT_COMPOSITE".equalsIgnoreCase(type)) {
				insertCockpitDocument(request);
			} else {
				throw new SpagoBIServiceException(SERVICE_NAME, "Impossible to create a document of type [" + type + "]");
			}
		} catch (SpagoBIServiceException e) {
			throw e;
		} catch (Exception e) {
			throw new SpagoBIServiceException(SERVICE_NAME, "An unexpected error occured while creating document", e);
		} finally {
			logger.debug("OUT");
		}
	}

	// private void modifyGeoreportDocument(JSONObject request){
	private void doModifyDocument(JSONObject request) {
		logger.debug("IN");
		try {
			JSONObject documentJSON = request.optJSONObject("document");
			JSONObject customDataJSON = request.optJSONObject("customData");

			Assert.assertNotNull(customDataJSON, "Custom data object cannot be null");

			// Load existing document
			IBIObjectDAO biObjectDao = DAOFactory.getBIObjectDAO();
			String documentLabel = documentJSON.getString("label");
			BIObject document = biObjectDao.loadBIObjectByLabel(documentLabel);
			JSONArray filteredFoldersJSON = new JSONArray();
			if (request.optJSONArray("folders") == null || request.optJSONArray("folders").length() == 0) {
				IEngUserProfile profile = getUserProfile();
				// add personal folder for default
				LowFunctionality userFunc = null;
				try {
					ILowFunctionalityDAO functionalitiesDAO = DAOFactory.getLowFunctionalityDAO();
					userFunc = functionalitiesDAO.loadLowFunctionalityByPath("/" + ((UserProfile) profile).getUserId(), false);
				} catch (Exception e) {
					logger.error("Error on insertion of the document.. Impossible to get the id of the personal folder ", e);
					throw new SpagoBIRuntimeException("Error on insertion of the document.. Impossible to get the id of the personal folder ", e);
				}
				filteredFoldersJSON.put(userFunc.getId());
			} else {
				filteredFoldersJSON = filterFolders(request.optJSONArray("folders"));
			}

			// update document informations
			document = syncronizeDocument(document, filteredFoldersJSON);

			String tempalteName = (MODIFY_GEOREPORT.equalsIgnoreCase(action)) ? "template.georeport" : "template.sbicockpit";
			String templateContent = customDataJSON.optString("templateContent");
			ObjTemplate template = buildDocumentTemplate(tempalteName, templateContent, document, null, null, null);
			documentManagementAPI.saveDocument(document, template);

			// save relations between document and datasets (only for cockpit engine)
			if (MODIFY_COCKPIT.equalsIgnoreCase(action)) {
				insertCockpitRelationsWithDataset(customDataJSON, document);
			}

		} catch (SpagoBIServiceException e) {
			throw e;
		} catch (Exception e) {
			throw new SpagoBIServiceException(SERVICE_NAME, "An unexpected error occured while creating geo document", e);
		} finally {
			logger.debug("OUT");
		}
	}

	private BIObject syncronizeDocument(BIObject obj, JSONArray folders) {
		logger.debug("IN");
		try {
			BIObject toReturn = obj;
			String name = getAttributeAsString("name");
			String description = getAttributeAsString("description");
			String previewFile = getAttributeAsString("previewFile");

			toReturn.setName(name);
			toReturn.setDescription(description);
			if (previewFile != null && !previewFile.equals(""))
				toReturn.setPreviewFile(previewFile.replace("\"", ""));

			// syncronize the folders
			toReturn = setFolders(obj, folders);

			return toReturn;
		} catch (SpagoBIServiceException e) {
			throw e;
		} catch (Exception e) {
			throw new SpagoBIServiceException(SERVICE_NAME, "An unexpected error occured while creating document", e);
		} finally {
			logger.debug("OUT");
		}
	}

	private void insertGeoreportDocument(JSONObject request) {

		logger.debug("IN");

		try {
			String sourceModelName = getAttributeAsString("model_name");
			JSONObject documentJSON = request.optJSONObject("document");
			JSONArray filteredFoldersJSON = new JSONArray();
			if (request.optJSONArray("folders") == null || request.optJSONArray("folders").length() == 0) {
				IEngUserProfile profile = getUserProfile();
				// add personal folder for default
				LowFunctionality userFunc = null;
				try {
					// ILowFunctionalityDAO functionalitiesDAO =
					// DAOFactory.getLowFunctionalityDAO();
					// userFunc =
					// functionalitiesDAO.loadLowFunctionalityByPath("/"+profile.getUserUniqueIdentifier(),false);
					userFunc = UserUtilities.loadUserFunctionalityRoot((UserProfile) profile, true);
				} catch (Exception e) {
					logger.error("Error on insertion of the document.. Impossible to get the id of the personal folder ", e);
					throw new SpagoBIRuntimeException("Error on insertion of the document.. Impossible to get the id of the personal folder ", e);
				}
				filteredFoldersJSON.put(userFunc.getId());
			} else {
				filteredFoldersJSON = filterFolders(request.optJSONArray("folders"));
			}
			JSONObject customDataJSON = request.optJSONObject("customData");
			Assert.assertNotNull(customDataJSON, "Custom data object cannot be null");

			if (request.has("sourceDataset")) {
				JSONObject sourceDatasetJSON = request.getJSONObject("sourceDataset");
				insertGeoReportDocumentCreatedOnDataset(sourceDatasetJSON, documentJSON, customDataJSON, filteredFoldersJSON);
			} else if (sourceModelName != null) {
				throw new SpagoBIServiceException(SERVICE_NAME, "Impossible to create geo document defined on a metamodel");
			} else {
				insertGeoReportDocumentCreatedOnDataset(null, documentJSON, customDataJSON, filteredFoldersJSON);
				// throw new SpagoBIServiceException(SERVICE_NAME,
				// "Impossible to create geo document because both sourceModel and sourceDataset are null");
			}
		} catch (SpagoBIServiceException e) {
			throw e;
		} catch (Exception e) {
			throw new SpagoBIServiceException(SERVICE_NAME, "An unexpected error occured while creating geo document", e);
		} finally {
			logger.debug("OUT");
		}
	}

	private void insertCockpitDocument(JSONObject request) {

		logger.debug("IN");

		try {
			JSONObject documentJSON = request.optJSONObject("document");
			JSONArray filteredFoldersJSON = new JSONArray();
			if (request.optJSONArray("folders") == null || request.optJSONArray("folders").length() == 0) {
				IEngUserProfile profile = getUserProfile();
				// add personal folder for default
				LowFunctionality userFunc = null;
				try {
					userFunc = UserUtilities.loadUserFunctionalityRoot((UserProfile) profile, true);
				} catch (Exception e) {
					logger.error("Error on insertion of the document.. Impossible to get the id of the personal folder ", e);
					throw new SpagoBIRuntimeException("Error on insertion of the document.. Impossible to get the id of the personal folder ", e);
				}
				filteredFoldersJSON.put(userFunc.getId());
			} else {
				filteredFoldersJSON = filterFolders(request.optJSONArray("folders"));
			}
			JSONObject customDataJSON = request.optJSONObject("customData");
			Assert.assertNotNull(customDataJSON, "Custom data object cannot be null");

			BIObject document = createBaseDocument(documentJSON, null, filteredFoldersJSON);
			ObjTemplate template = buildDocumentTemplate("template.sbicockpit", customDataJSON, null);

			documentManagementAPI.saveDocument(document, template);

			// insert relations between document and datasets
			insertCockpitRelationsWithDataset(customDataJSON, document);

		} catch (SpagoBIServiceException e) {
			throw e;
		} catch (Exception e) {
			throw new SpagoBIServiceException(SERVICE_NAME, "An unexpected error occured while creating cockpit document", e);
		} finally {
			logger.debug("OUT");
		}
	}

	private void insertGeoReportDocumentCreatedOnDataset(JSONObject sourceDatasetJSON, JSONObject documentJSON, JSONObject customDataJSON,
			JSONArray foldersJSON) {

		logger.debug("IN");

		String sourceDatasetLabel = null;
		try {
			BIObject document = createBaseDocument(documentJSON, null, foldersJSON);
			ObjTemplate template = buildDocumentTemplate("template.sbigeoreport", customDataJSON, null);

			IDataSet sourceDataset = null;
			if (sourceDatasetJSON != null) {
				sourceDatasetLabel = sourceDatasetJSON.optString("label");
				Assert.assertNotNull(StringUtilities.isNotEmpty(sourceDatasetLabel), "Source dataset's label cannot be null or empty");

				try {
					sourceDataset = DAOFactory.getDataSetDAO().loadDataSetByLabel(sourceDatasetLabel);
				} catch (Throwable t) {
					throw new SpagoBIServiceException(SERVICE_NAME, "Impossible to load source datset [" + sourceDatasetLabel + "]");
				}
				if (sourceDataset == null) {
					throw new SpagoBIServiceException(SERVICE_NAME, "Source datset [" + sourceDatasetLabel + "] does not exist");
				}
				document.setDataSetId(sourceDataset.getId());
			}

			documentManagementAPI.saveDocument(document, template);
			if (sourceDataset != null) {
				documentManagementAPI.propagateDatasetParameters(sourceDataset, document);
			}

			JSONArray metadataJSON = documentJSON.optJSONArray("metadata");
			if (metadataJSON != null) {
				documentManagementAPI.saveDocumentMetadataProperties(document, null, metadataJSON);
			}
		} catch (SpagoBIServiceException e) {
			throw e;
		} catch (Throwable e) {
			throw new SpagoBIServiceException(SERVICE_NAME,
					"An unexpected error occured while inserting geo document created on datset [" + sourceDatasetLabel + "]", e);
		} finally {
			logger.debug("OUT");
		}
	}

	// TODO consolidate the following 2 methods
	private BIObject createBaseDocument(JSONObject documentJSON, JSONObject sourceDocumentJSON, JSONArray folderJSON) {
		BIObject sourceDocument = null;
		String visibility = "true"; // default value
		String previewFile = "";

		try {
			if (sourceDocumentJSON != null) {
				String sourceDocumentId = sourceDocumentJSON.getString("id").trim();
				sourceDocument = documentManagementAPI.getDocument(new Integer(sourceDocumentId));
			}
			if (documentJSON.optString("visibility") != null && !documentJSON.optString("visibility").equals("")) {
				visibility = documentJSON.getString("visibility");// overriding
																	// default
																	// value
			}
			if (documentJSON.optString("previewFile") != null && !documentJSON.optString("previewFile").equals("")) {
				previewFile = documentJSON.getString("previewFile");// overriding
																	// default
																	// value
			}
			return createBaseDocument(documentJSON.getString("label"), documentJSON.getString("name"), documentJSON.getString("description"), visibility,
					previewFile, documentJSON.getString("type"), documentJSON.optString("engineId"), sourceDocument, folderJSON);
		} catch (Throwable t) {
			throw new SpagoBIServiceException(SERVICE_NAME, "An unaxpected error occured while creating base object", t);
		}
	}

	private BIObject createBaseDocument(String label, String name, String description, String visibility, String previewFile, String type, String engineId,
			BIObject sourceDocument, JSONArray foldersJSON) {

		BIObject document = new BIObject();

		document.setLabel(label);
		document.setName(name);
		document.setDescription(description);
		if (previewFile != null) {
			document.setPreviewFile(previewFile.replace("\"", ""));
		}

		if ("DOCUMENT_COMPOSITE".equalsIgnoreCase(type)) {
			// gets correct type of the engine for DOCUMENT_COMPOSITION (it's
			// cockpit and it uses the EXTERNAL engine)
			Engine engine = null;
			Domain engineType = null;
			try {
				List<Engine> engines = DAOFactory.getEngineDAO().loadAllEnginesForBIObjectTypeAndTenant(type);
				if (engines != null && !engines.isEmpty()) {
					if ("DOCUMENT_COMPOSITE".equalsIgnoreCase(type)) {
						for (Engine e : engines) {
							try {
								engineType = DAOFactory.getDomainDAO().loadDomainById(e.getEngineTypeId());
							} catch (EMFUserError ex) {
								throw new SpagoBIServiceException("Impossible to load engine type domain", ex);
							}

							if ("EXT".equalsIgnoreCase(engineType.getValueCd())) {
								engine = e;
							}
						}
					}
				}
			} catch (Throwable t) {
				throw new SpagoBIServiceException(SERVICE_NAME, "An unexpected error occured while loading the engine", t);
			}
			document.setBiObjectTypeID(engine.getBiobjTypeId());
			document.setBiObjectTypeCode(type);
			document.setEngine(engine);
		} else {
			setDocumentEngine(document, type, engineId);
		}
		Boolean isVisible = Boolean.parseBoolean(visibility);
		document.setVisible(isVisible);

		if (sourceDocument != null) {
			setDatasource(document, sourceDocument);
			setDataset(document, sourceDocument);
		}

		setDocumentState(document);
		setFolders(document, foldersJSON);
		setCreationUser(document);

		return document;
	}

	private BIObject setDocumentEngine(BIObject document, String type, String engineId) {
		Engine engine = null;

		try {

			Domain objType = DAOFactory.getDomainDAO().loadDomainByCodeAndValue(SpagoBIConstants.BIOBJ_TYPE, type);
			Integer biObjectTypeID = objType.getValueId();
			document.setBiObjectTypeID(biObjectTypeID);
			document.setBiObjectTypeCode(objType.getValueCd());

			if (StringUtilities.isNotEmpty(engineId)) {
				engine = DAOFactory.getEngineDAO().loadEngineByID(new Integer(engineId));
				if (engine == null) {
					throw new SpagoBIServiceException(SERVICE_NAME, "Impossible to load engine with id equals to[" + engineId + "]");
				}
			} else {
				IEngineDAO enginedao = DAOFactory.getEngineDAO();
				enginedao.setUserProfile(getUserProfile());
				List<Engine> engines = enginedao.loadAllEnginesForBIObjectTypeAndTenant(type);
				if (engines != null && !engines.isEmpty()) {
					engine = engines.get(0);
					if ("MAP".equalsIgnoreCase(type)) {
						for (Engine e : engines) {
							if (e.getLabel().equals("SpagoBIGisEngine")) {
								engine = e;
							}
						}
					}
				} else {
					throw new SpagoBIServiceException(SERVICE_NAME, "No suitable engine found for document type [" + type + "]");
				}
			}
			document.setEngine(engine);
		} catch (Throwable t) {
			throw new SpagoBIServiceException(SERVICE_NAME, "An unexpected error occured while loading the engine", t);
		}

		return document;
	}

	private BIObject setDatasource(BIObject document, BIObject sourceDocument) {

		document.setDataSourceId(sourceDocument.getDataSourceId());

		return document;
	}

	private BIObject setDataset(BIObject document, BIObject sourceDocument) {

		Integer dataSetId = sourceDocument.getDataSetId();
		document.setDataSetId(dataSetId);

		return document;
	}

	private BIObject setDocumentState(BIObject document) {
		try {
			Domain objState = DAOFactory.getDomainDAO().loadDomainByCodeAndValue(SpagoBIConstants.DOC_STATE, SpagoBIConstants.DOC_STATE_REL);
			Integer stateID = objState.getValueId();
			document.setStateID(stateID);
			document.setStateCode(objState.getValueCd());
		} catch (Throwable t) {
			throw new SpagoBIServiceException(SERVICE_NAME, "An unexpected error occured while setting document [" + document.getId() + "] state");
		}

		return document;
	}

	private BIObject setFolders(BIObject document, JSONArray functsArrayJSon) {

		List<Integer> folders;

		folders = new ArrayList<Integer>();

		try {
			for (int i = 0; i < functsArrayJSon.length(); i++) {
				// with Jackson library isn't necessary to convert the string
				// into integer because the value is already a number!
				// String folderId = functsArrayJSon.getString(i);
				// Integer id = new Integer(folderId);
				Integer id = new Integer(functsArrayJSon.getInt(i));
				if (id.intValue() == -1) {
					// -1 stands for personal folder: check is it exists
					// load personal folder to get its id: in case it does not
					// exist, create it
					LowFunctionality folder = UserUtilities.loadUserFunctionalityRoot((UserProfile) getUserProfile(), true);
					id = folder.getId();
				}
				folders.add(id);
			}

			document.setFunctionalities(folders);
		} catch (Throwable t) {
			throw new SpagoBIServiceException(SERVICE_NAME, "Impossible to set folders to document [" + document.getId() + "]", t);
		}

		return document;
	}

	private BIObject setCreationUser(BIObject document) {
		UserProfile userProfile = (UserProfile) this.getUserProfile();
		String creationUser = userProfile.getUserId().toString();
		document.setCreationUser(creationUser);
		return document;
	}

	private ObjTemplate buildDocumentTemplate(String templateName, JSONObject customDataJSON, BIObject sourceDocument) {
		String templateContent = customDataJSON.optString("templateContent");
		JSONObject smartFilterData = customDataJSON.optJSONObject("smartFilter");
		String query = customDataJSON.optString("query");
		String modelName = customDataJSON.optString("modelName");
		return buildDocumentTemplate(templateName, templateContent, sourceDocument, query, smartFilterData, modelName);
	}

	private ObjTemplate buildDocumentTemplate(String templateName, String templateContent, BIObject sourceDocument, String query, JSONObject smartFilterData,
			String modelName) {

		ObjTemplate template = null;

		DocumentTemplateBuilder documentTemplateBuilder = new DocumentTemplateBuilder();

		try {

			UserProfile userProfile = (UserProfile) this.getUserProfile();
			String templateAuthor = userProfile.getUserId().toString();

			if (StringUtilities.isNotEmpty(templateContent)) {
				template = documentTemplateBuilder.buildDocumentTemplate(templateName, templateAuthor, templateContent);
			} else if (smartFilterData != null) {
				ExecutionInstance executionInstance = getContext().getExecutionInstance(ExecutionInstance.class.getName());
				BIObject parentQbeDocument = executionInstance.getBIObject();
				template = documentTemplateBuilder.buildSmartFilterDocumentTemplate(templateName, templateAuthor, parentQbeDocument, query, smartFilterData,
						modelName);

			} else {
				throw new SpagoBIServiceException(SERVICE_NAME, "sbi.document.saveError");
			}

		} catch (Throwable t) {
			throw new SpagoBIServiceException(SERVICE_NAME, "An unexpected error occured while building document template", t);
		}
		return template;
	}

	/**
	 * @return a JSON object representing the input request to the service with the following structure: <code>
	 * 		{
	 * 			action: STRING
	 * 			, sourceDataset: {
	 * 				label: STRING
	 * 			}
	 * 			, sourceDocument: {
	 * 				id: NUMBER
	 * 			}
	 * 			, document: {
	 * 				id: NUMBER
	 * 				label: STRING
	 * 				name: STRING
	 * 				description: STRING
	 * 				type: STRING
	 * 				engineId: NUMBER
	 * 				metadata: [JSON, ..., JSON]
	 * 			}
	 * 			, customData: {
	 * 				query: [STRING]
	 * 				workseheet: [JSON]
	 * 				smartfilter:  [JSON]
	 * 			}
	 * 			, folders: [STRING, ... , STRING]
	 * 		}
	 * 	</code>
	 **/

	public JSONObject parseRequest() {

		logger.debug("OUT");

		try {
			JSONObject request = new JSONObject();

			String action = this.getAttributeAsString(MESSAGE_DET);
			request.put("action", action);

			// sourceDatasetLabel
			String sourceDatasetLabel = getAttributeAsString(OBJ_DATASET_LABEL);

			if (StringUtilities.isNotEmpty(sourceDatasetLabel)) {
				JSONObject sourceDataset = new JSONObject();
				sourceDataset.put("label", sourceDatasetLabel);
				request.put("sourceDataset", sourceDataset);
			}

			// sourceDocumentId
			String sourceDocumentId = getAttributeAsString(OBJ_ID);

			if (StringUtilities.isNotEmpty(sourceDocumentId)) {
				JSONObject sourceDocument = new JSONObject();
				sourceDocument.put("id", sourceDocumentId);
				request.put("sourceDocument", sourceDocument);
			}

			// document
			JSONObject document = new JSONObject();

			String documentId = getAttributeAsString(ID);
			if (documentId != null)
				document.put("id", documentId);

			String label = getAttributeAsString(LABEL);
			if (label != null)
				document.put("label", label);

			String name = getAttributeAsString(NAME);
			if (name != null)
				document.put("name", name);

			String description = getAttributeAsString(DESCRIPTION);
			if (description != null)
				document.put("description", description);

			String visibility = getAttributeAsString(VISIBILITY);
			if (visibility != null)
				document.put("visibility", visibility);

			String type = getAttributeAsString(TYPE);
			if (type != null)
				document.put("type", type);

			String engineId = getAttributeAsString(ENGINE);
			if (engineId != null)
				document.put("engineId", engineId);

			// preview file
			String previewFile = getAttributeAsString(PREVIEW_FILE);
			if (previewFile != null)
				document.put("previewFile", previewFile);

			String businessMetadata = getAttributeAsString(BUSINESS_METADATA);
			if (StringUtilities.isNotEmpty(businessMetadata)) {
				JSONObject businessMetadataJSON = new JSONObject(businessMetadata);
				JSONArray metaProperties = new JSONArray();
				JSONArray names = businessMetadataJSON.names();
				for (int i = 0; i < names.length(); i++) {
					String key = names.getString(i);
					String value = businessMetadataJSON.getString(key);
					JSONObject metaProperty = new JSONObject();
					metaProperty.put("meta_name", key);
					metaProperty.put("meta_content", value);
					metaProperties.put(metaProperty);
				}
				document.put("metadata", metaProperties);
			}

			request.put("document", document);

			// customData
			JSONObject customData = new JSONObject();

			if (requestContainsAttribute(FORMVALUES) && StringUtilities.isNotEmpty(getAttributeAsString(FORMVALUES))) {
				JSONObject smartFilterData = getAttributeAsJSONObject(FORMVALUES);
				if (smartFilterData != null)
					customData.put("smartFilter", smartFilterData);
			}

			String query = getAttributeAsString(OBJECT_QUERY);
			if (query != null)
				customData.put("query", query);

			// String templateContent = getAttributeAsString(TEMPLATE);
			JSONObject templateContent = getAttributeAsJSONObject(TEMPLATE);
			if (templateContent != null)
				customData.put("templateContent", templateContent);

			request.put("customData", customData);

			// folders
			JSONArray foldersJSON = new JSONArray();
			if (requestContainsAttribute(FUNCTS) && StringUtilities.isNotEmpty(getAttributeAsString(FUNCTS))) {
				foldersJSON = getAttributeAsJSONArray(FUNCTS);
				if (foldersJSON != null)
					request.put("folders", foldersJSON);
			}
			// COMMUNITY
			String communityFCode = getAttributeAsString(COMMUNITY);
			if (communityFCode != null && !"".equalsIgnoreCase(communityFCode)) {
				if (communityFCode.startsWith("-1")) {
					// clean the community
					String realCode = communityFCode.substring(communityFCode.indexOf("__") + 2);
					if (!realCode.equals("")) {
						for (int i = 0; i < foldersJSON.length(); i++) {
							if (foldersJSON.get(i).equals(Integer.valueOf(realCode))) {
								foldersJSON.remove(i);
								break;
							}
						}
					}
				} else {
					// add community folder to functionalities community folder
					LowFunctionality commF = DAOFactory.getLowFunctionalityDAO().loadLowFunctionalityByCode(communityFCode, false);
					Integer commFId = commF.getId();
					foldersJSON.put(commFId);
				}
			}

			logger.debug("Request succesfully parsed: " + request.toString(3));

			return request;
		} catch (SpagoBIServiceException e) {
			throw e;
		} catch (Exception e) {
			throw new SpagoBIServiceException(SERVICE_NAME, "An unexpected error occured while parsing the request", e);
		} finally {
			logger.debug("OUT");
		}
	}

	// ==============================================================
	// TODO refactor the following code...
	// ==============================================================

	// protected byte[] getTemplateContent() throws Exception {
	// logger.debug("OUT");
	// String wkDefinition = getAttributeAsString(OBJECT_WK_DEFINITION);
	// String query = getAttributeAsString(OBJECT_QUERY);
	// JSONObject smartFilterValues = getAttributeAsJSONObject(FORMVALUES);
	// String smartFilterValuesString = null;
	// if (smartFilterValues != null) {
	// smartFilterValuesString = smartFilterValues.toString();
	// }
	// logger.debug("Base query definition : " + query);
	// logger.debug("Smart filter values : " + smartFilterValues);
	// ExecutionInstance executionInstance = getContext().getExecutionInstance(ExecutionInstance.class.getName());
	// BIObject biobj = executionInstance.getBIObject();
	// ObjTemplate qbETemplate = biobj.getActiveTemplate();
	// String templCont = new String(qbETemplate.getContent());
	// logger.debug("OUT");
	// return content;
	// }
	//
	// protected ObjTemplate createNewTemplate(byte[] content) {
	// logger.debug("IN");
	// ObjTemplate objTemp = new ObjTemplate();
	// objTemp.setContent(content);
	// UserProfile userProfile = (UserProfile) this.getUserProfile();
	// objTemp.setCreationUser(userProfile.getUserId().toString());
	// objTemp.setDimension(Long.toString(content.length / 1000) + " KByte");
	// logger.debug("OUT");
	// return objTemp;
	// }

	private JSONArray filterFolders(JSONArray foldersJSON) {
		JSONArray toReturn = new JSONArray();
		try {
			Set<Integer> folderIds = new HashSet<Integer>();
			for (int i = 0; i < foldersJSON.length(); i++) {
				int id = foldersJSON.getInt(i);
				Integer folderId = new Integer(id);
				if (!folderIds.contains(folderId)) {
					toReturn.put(id);
					folderIds.add(new Integer(folderId));
				} else {
					logger.debug("Folder filtered out because duplicate: [" + id + "]");
				}
			}
		} catch (Exception e) {
			throw new SpagoBIServiceException(SERVICE_NAME, "An error occured while defines folders list", e);
		}
		return toReturn;
	}

	private static void insertCockpitRelationsWithDataset(JSONObject configJSON, BIObject obj) throws Exception {

		// 0. Get the engine. Only engines with 1:N relation with datasets are managed.
		// 09.05.2016 : for the moment only the cockpit engine is multidatasets.
		Engine engineObj = obj.getEngine();
		if (!engineObj.getLabel().toLowerCase().contains("cockpit")) {
			logger.debug("The engine [" + engineObj.getLabel() + "] cannot use multiple datasets.");
			return;
		}

		// 1. search used datasets
		JSONObject storeConfJSON = configJSON.getJSONObject("templateContent").getJSONObject("storesConf");
		if (storeConfJSON != null) {
			try {
				JSONArray lstStoresJSON = storeConfJSON.getJSONArray("stores");
				HashMap<Integer, Boolean> lstDsInsertedForObj = new HashMap<Integer, Boolean>();
				// 2. delete all relations between document and datasets if exist
				DAOFactory.getSbiObjDsDAO().deleteObjDsbyObjId(obj.getId());
				// 3. insert the new relations between document and datasets
				for (int i = 0; i < lstStoresJSON.length(); i++) {
					JSONObject storeJSON = lstStoresJSON.getJSONObject(i);
					String dsLabel = storeJSON.getString("storeId");
					logger.debug("Insert relation for dataset with label [" + dsLabel + "]");
					VersionedDataSet ds = ((VersionedDataSet) DAOFactory.getDataSetDAO().loadDataSetByLabel(dsLabel));

					// insert only relations with new ds
					if (lstDsInsertedForObj.get(ds.getId()) != null) {
						continue;
					}

					String dsOrganization = ds.getOrganization();
					logger.debug("Dataset organization used for insert relation is: " + dsOrganization);
					Integer dsVersion = ds.getVersionNum();
					logger.debug("Dataset version used for insert relation is: " + dsVersion);

					// creating relation object
					SbiMetaObjDs relObjDs = new SbiMetaObjDs();
					SbiMetaObjDsId relObjDsId = new SbiMetaObjDsId();
					relObjDsId.setDsId(ds.getId());
					relObjDsId.setOrganization(dsOrganization);
					relObjDsId.setVersionNum(dsVersion);
					relObjDsId.setObjId(obj.getId());
					relObjDs.setId(relObjDsId);

					DAOFactory.getSbiObjDsDAO().insertObjDs(relObjDs);
					lstDsInsertedForObj.put(ds.getId(), true);
				}
			} catch (Exception e) {
				throw new Exception(e);
			}
		} else {
			logger.debug("The document doesn't use any dataset! ");
		}
	}

}
