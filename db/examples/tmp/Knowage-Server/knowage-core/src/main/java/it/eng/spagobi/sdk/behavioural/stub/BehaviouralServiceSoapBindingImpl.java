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

package it.eng.spagobi.sdk.behavioural.stub;

import it.eng.spagobi.sdk.behavioural.impl.BehaviouralServiceImpl;

public class BehaviouralServiceSoapBindingImpl implements it.eng.spagobi.sdk.behavioural.stub.BehaviouralService {
	public it.eng.spagobi.sdk.behavioural.bo.SDKAttribute[] getAllAttributes(java.lang.String in0) throws java.rmi.RemoteException,
			it.eng.spagobi.sdk.exceptions.NotAllowedOperationException {
		BehaviouralServiceImpl impl = new BehaviouralServiceImpl();
		return impl.getAllAttributes(in0);
	}

	public it.eng.spagobi.sdk.behavioural.bo.SDKRole[] getRoles() throws java.rmi.RemoteException, it.eng.spagobi.sdk.exceptions.NotAllowedOperationException {
		BehaviouralServiceImpl impl = new BehaviouralServiceImpl();
		return impl.getRoles();

	}

	public it.eng.spagobi.sdk.behavioural.bo.SDKRole[] getRolesByUserId(java.lang.String in0) throws java.rmi.RemoteException,
			it.eng.spagobi.sdk.exceptions.NotAllowedOperationException {
		BehaviouralServiceImpl impl = new BehaviouralServiceImpl();
		return impl.getRolesByUserId(in0);

	}

}
