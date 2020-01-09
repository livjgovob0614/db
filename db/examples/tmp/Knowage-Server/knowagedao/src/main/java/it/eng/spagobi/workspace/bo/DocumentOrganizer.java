/*
 * Knowage, Open Source Business Intelligence suite
 * Copyright (C) 2016 Engineering Ingegneria Informatica S.p.A.

 * Knowage is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Knowage is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package it.eng.spagobi.workspace.bo;

/*
 * Knowage, Open Source Business Intelligence suite
 * Copyright (C) 2016 Engineering Ingegneria Informatica S.p.A.

 * Knowage is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Knowage is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
import it.eng.spagobi.hotlink.rememberme.bo.HotLink;

public class DocumentOrganizer extends HotLink {

	private Integer functId;
	private Integer biObjId;

	/**
	 * Added for the tooltip for a document when searching through Organizer documents. The goal is to see from which folder the document that is hovered comes,
	 * to see the full path.
	 *
	 * NOTE: This is not really used on the client side, since this parameter provides the path that contains the code of the document, instead of its name.
	 * Since the name is displayed for the document, the user could not know for which folder which code corresponds. The DB table should be extended so to
	 * contain also a full path with the name of each folder's name in it.
	 *
	 * @author Danilo Ristovski (danristo, danilo.ristovski@mht.net)
	 */
	private String documentPath;

	public DocumentOrganizer() {
		super();
	}

	public DocumentOrganizer(Integer functId, Integer biObjId, String documentPath) {
		super();
		this.functId = functId;
		this.biObjId = biObjId;
		this.documentPath = documentPath;
	}

	public String getDocumentPath() {
		return documentPath;
	}

	public void setDocumentPath(String documentPath) {
		this.documentPath = documentPath;
	}

	public Integer getFunctId() {
		return functId;
	}

	public void setFunctId(Integer functId) {
		this.functId = functId;
	}

	public Integer getBiObjId() {
		return biObjId;
	}

	public void setBiObjId(Integer biObjId) {
		this.biObjId = biObjId;
	}

}
