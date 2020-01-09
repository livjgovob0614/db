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
package it.eng.spagobi.api.v2;

import static com.jayway.restassured.RestAssured.expect;
import static com.jayway.restassured.RestAssured.get;
import static com.jayway.restassured.RestAssured.given;
import static org.hamcrest.CoreMatchers.not;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.Assert.assertNotNull;
import it.eng.spagobi.api.common.AbstractV2BasicAuthTestCase;
import it.eng.spagobi.tools.catalogue.bo.MetaModel;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import org.junit.Before;
import org.junit.FixMethodOrder;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import com.jayway.restassured.http.ContentType;
import com.jayway.restassured.path.json.JsonPath;
import com.jayway.restassured.response.Response;

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
public class BusinessModelResourceTest extends AbstractV2BasicAuthTestCase {

	List<Integer> bmIds;
	List<Integer> vIds;

	String json;

	MetaModel businessModel = new MetaModel();
	static int id;

	@Override
	@Before
	public void setup() {
		super.setup();

		json = get("/businessmodels").asString();
		bmIds = JsonPath.from(json).get("id");

		Date date = new Date();
		SimpleDateFormat formatter = new SimpleDateFormat("dd-MM-yy H:mm:ss:SSS");

		businessModel.setName("Insert from test " + formatter.format(date));
		businessModel.setDescription("Inserting new model for JUnit testing");
		businessModel.setModelLocked(false);
	}

	/**
	 * Tests for business models
	 **/
	@Test
	public void getAllBusinessModels() {
		expect().contentType(ContentType.JSON).statusCode(200).when().get("/businessmodels");

		get("/businessmodels").then().body("name", (not(equalTo(""))));

	}

	@Test
	public void getBusinessModelById() {
		for (int i = 0; i < bmIds.size(); i++) {
			expect().contentType(ContentType.JSON).statusCode(200).when().get("/businessmodels/" + bmIds.get(i));
		}
	}

	@Test
	public void postBusinessModel() {
		Response response = given().contentType(ContentType.JSON).body(businessModel).when().post("/businessmodels").then().statusCode(200).extract()
				.response();

		String responseJson = response.body().asString();
		id = JsonPath.from(responseJson).get("id");
		assertNotNull(id);
	}

	@Test
	public void putBusinessModel() {
		businessModel.setId(id);
		businessModel.setDescription(businessModel.getDescription() + " edited");

		Response response = given().contentType(ContentType.JSON).body(businessModel).when().put("/businessmodels/" + businessModel.getId()).then()
				.statusCode(200).extract().response();

	}

	@Test
	public void removeBusinessModel() {
		expect().statusCode(200).when().delete("/" + id);
	}

	/**
	 * Tests for business model versions
	 **/
	@Test
	public void getVersionsOfBusinnesModel() {
		for (int i = 0; i < bmIds.size(); i++) {
			expect().contentType(ContentType.JSON).statusCode(200).when().get("/businessmodels/" + bmIds.get(i) + "/versions");
		}
	}

	@Test
	public void getVersionsById() {
		for (int i = 0; i < bmIds.size(); i++) {

			String versions = get("/businessmodels/" + bmIds.get(i) + "/versions").asString();
			vIds = JsonPath.from(versions).get("id");

			for (int j = 0; j < vIds.size(); j++) {
				expect().contentType(ContentType.JSON).statusCode(200).when().get("/businessmodels/" + bmIds.get(i) + "/versions/" + vIds.get(j));
			}
		}
	}

}
