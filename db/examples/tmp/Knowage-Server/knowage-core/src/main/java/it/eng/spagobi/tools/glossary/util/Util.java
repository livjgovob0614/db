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
package it.eng.spagobi.tools.glossary.util;

import it.eng.spago.error.EMFUserError;
import it.eng.spagobi.analiticalmodel.document.bo.BIObject;
import it.eng.spagobi.analiticalmodel.document.metadata.SbiObjects;
import it.eng.spagobi.behaviouralmodel.analyticaldriver.bo.BIObjectParameter;
import it.eng.spagobi.commons.dao.DAOFactory;
import it.eng.spagobi.metadata.metadata.SbiMetaBc;
import it.eng.spagobi.metadata.metadata.SbiMetaTable;
import it.eng.spagobi.tools.dataset.bo.IDataSet;
import it.eng.spagobi.tools.dataset.metadata.SbiDataSet;
import it.eng.spagobi.tools.glossary.metadata.SbiGlContents;
import it.eng.spagobi.tools.glossary.metadata.SbiGlGlossary;
import it.eng.spagobi.tools.glossary.metadata.SbiGlReferences;
import it.eng.spagobi.tools.glossary.metadata.SbiGlWord;
import it.eng.spagobi.tools.udp.bo.Udp;
import it.eng.spagobi.tools.udp.metadata.SbiUdpValue;

import java.util.Iterator;
import java.util.List;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class Util {

	public static boolean isNumber(String str) {
		return str != null && str.matches("\\d+");
	}

	public static Integer getNumberOrNull(Object str) {
		if (str instanceof Integer)
			return (Integer) str;
		return isNumber("" + str) ? Integer.valueOf("" + str) : null;
	}

	public static JSONObject fromContentsLight(SbiGlContents sbiGlContents, boolean wordChild, boolean ContentsChild) throws JSONException {
		JSONObject ret = new JSONObject();
		ret.put("CONTENT_ID", sbiGlContents.getContentId());
		ret.put("CONTENT_NM", sbiGlContents.getContentNm());
		ret.put("HAVE_WORD_CHILD", wordChild);
		ret.put("HAVE_CONTENTS_CHILD", ContentsChild);

		return ret;
	}

	public static JSONObject fromDocumentLight(BIObject sbiob) throws JSONException {
		JSONObject ret = new JSONObject();
		ret.put("DOCUMENT_ID", sbiob.getId());
		ret.put("DOCUMENT_LABEL", sbiob.getLabel());
		ret.put("DOCUMENT_NAME", sbiob.getName());
		ret.put("DOCUMENT_DESCR", sbiob.getDescription());
		ret.put("DOCUMENT_AUTH", sbiob.getCreationUser());
		return ret;
	}

	public static JSONArray fromObjectParameterListLight(List<BIObjectParameter> objPar) throws JSONException {
		JSONArray ja = new JSONArray();
		for (BIObjectParameter par : objPar) {

			JSONObject ret = new JSONObject();
			ret.put("id", par.getId());
			ret.put("label", par.getLabel());
			ret.put("required", par.isRequired());
			ret.put("urlName", par.getParameterUrlName());
			ret.put("type", par.getParameter().getType());
			ja.put(ret);
		}
		return ja;
	}

	public static JSONObject fromDataSetLight(IDataSet datas) throws JSONException {
		JSONObject ret = new JSONObject();
		ret.put("DATASET_ID", datas.getId());
		ret.put("DATASET_NM", datas.getLabel());
		ret.put("DATASET_ORG", datas.getOrganization());
		return ret;
	}

	public static JSONObject fromDataSetLight(SbiDataSet sbidataset) throws JSONException {
		JSONObject jobj = new JSONObject();
		jobj.put("DATASET_ID", sbidataset.getId().getDsId());
		jobj.put("DATASET_NM", sbidataset.getLabel());
		jobj.put("DATASET_ORG", sbidataset.getId().getOrganization());
		return jobj;
	}

	public static JSONObject fromBnessClsLight(SbiMetaBc sbibnesscls) throws JSONException {
		JSONObject jobj = new JSONObject();
		jobj.put("BC_ID", sbibnesscls.getBcId());
		jobj.put("META_MODEL_NAME", sbibnesscls.getSbiMetaModel().getName());
		jobj.put("BC_NAME", sbibnesscls.getName());
		return jobj;
	}

	public static JSONObject fromTableLight(SbiMetaTable sbitable) throws JSONException {
		JSONObject jobj = new JSONObject();
		jobj.put("TABLE_ID", sbitable.getTableId());
		jobj.put("META_SOURCE_NAME", sbitable.getSbiMetaSource().getName());
		jobj.put("TABLE_NM", sbitable.getName());
		return jobj;
	}

	public static JSONObject fromDocumentLight(SbiObjects sbiob) throws JSONException {
		JSONObject ret = new JSONObject();
		ret.put("DOCUMENT_ID", sbiob.getBiobjId());
		ret.put("DOCUMENT_LABEL", sbiob.getLabel());
		return ret;
	}

	public static JSONObject fromWordLight(SbiGlWord sbiGlWord) throws JSONException {
		JSONObject jobj = new JSONObject();
		jobj.put("WORD_ID", sbiGlWord.getWordId());
		jobj.put("WORD", sbiGlWord.getWord());
		return jobj;
	}

	public static JSONObject fromUdpLight(Udp SbiUdp) throws JSONException {
		JSONObject jobj = new JSONObject();
		jobj.put("ATTRIBUTE_ID", SbiUdp.getUdpId());
		jobj.put("ATTRIBUTE_NM", SbiUdp.getLabel());
		return jobj;
	}

	public static JSONObject fromWord(SbiGlWord word) throws JSONException, EMFUserError {
		JSONObject obj = new JSONObject();

		obj.put("WORD_ID", word.getWordId());
		obj.put("WORD", word.getWord());
		obj.put("DESCR", word.getDescr());
		obj.put("FORMULA", word.getFormula());
		obj.put("STATE", word.getState_id());
		if (word.getState() != null) {
			obj.put("STATE_NM", word.getState().getValueNm());
		}
		obj.put("CATEGORY", word.getCategory_id());
		if (word.getCategory() != null) {
			obj.put("CATEGORY_NM", word.getCategory().getValueNm());
		}
		JSONArray links = new JSONArray();
		if (word.getReferences() != null) {
			for (Iterator<SbiGlReferences> iterator = word.getReferences().iterator(); iterator.hasNext();) {
				SbiGlReferences refWord = iterator.next();
				links.put(fromWordLight(refWord.getRefWord()));
			}
			obj.put("LINK", links);
		}
		JSONArray attrs = new JSONArray();
		if (word.getAttributes() != null) {
			for (Iterator<SbiUdpValue> iterator = word.getAttributes().iterator(); iterator.hasNext();) {
				SbiUdpValue attr = iterator.next();
				JSONObject jsonAttr = new JSONObject();
				jsonAttr.put("ATTRIBUTE_ID", attr.getSbiUdp().getUdpId());

				jsonAttr.put("ATTRIBUTE_NM", DAOFactory.getUdpDAO().loadById(attr.getSbiUdp().getUdpId()).getLabel());
				jsonAttr.put("VALUE", attr.getValue());
				attrs.put(jsonAttr);
			}
			obj.put("SBI_GL_WORD_ATTR", attrs);
		}
		return obj;
	}

	public static JSONObject fromGlossaryLight(SbiGlGlossary sbiGlGlossary)

	throws JSONException {
		JSONObject ret = new JSONObject();
		ret.put("GLOSSARY_ID", sbiGlGlossary.getGlossaryId());
		ret.put("GLOSSARY_NM", sbiGlGlossary.getGlossaryNm());
		return ret;
	}

	public static JSONObject fromGlossary(SbiGlGlossary sbiGlGlossary) throws JSONException {
		JSONObject ret = new JSONObject();
		ret.put("GLOSSARY_ID", sbiGlGlossary.getGlossaryId());
		ret.put("GLOSSARY_NM", sbiGlGlossary.getGlossaryNm());
		ret.put("GLOSSARY_CD", sbiGlGlossary.getGlossaryCd());
		ret.put("GLOSSARY_DS", sbiGlGlossary.getGlossaryDs());
		return ret;
	}

	public static JSONObject fromContent(SbiGlContents sbiGlContents) throws JSONException {
		JSONObject ret = new JSONObject();
		ret.put("CONTENT_ID", sbiGlContents.getGlossaryId());
		ret.put("CONTENT_NM", sbiGlContents.getContentNm());
		ret.put("CONTENT_CD", sbiGlContents.getContentCd());
		ret.put("CONTENT_DS", sbiGlContents.getContentDs());
		return ret;
	}
}
