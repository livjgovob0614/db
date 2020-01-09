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

//var sbiM=angular.module('sbiModule',['toastr']);
var sbiM=angular.module('sbiModule',[]);


sbiM.config(function($mdThemingProvider) {
	$mdThemingProvider.theme('default').primaryPalette('indigo').accentPalette('blue-grey');
});

sbiM.factory('sbiModule_user',function(){

	var user={};

	return user;
});

sbiM.service('sbiModule_logger',function(){
	this.exec=true;
	this.log = function(val1,val2,val3){
		if(this.exec){
			console.log("[LOG] ",val1,(val2 || ""),(val3|| ""));
		}
	};

	this.trace = function(val1,val2,val3){
		if(this.exec){
			console.log("[TRACE] ",val1,(val2 || ""),(val3|| ""));
		}
	};

});

sbiM.service('sbiModule_messaging',function(toastr){
	this.showErrorMessage = function(msg,title){
		
		toastr.error(msg,title, {
			  closeButton: true
		});
	};
	this.showWarningMessage = function(msg,title){
		
		toastr.warning(msg,title, {
			  closeButton: true
		});
	};
	this.showInfoMessage = function(msg,title){
		
		toastr.info(msg,title, {
			  closeButton: true
		});
	};
	this.showSuccessMessage = function(msg,title){
		
		toastr.success(msg,title, {
			  closeButton: true
			});
	};
	
});

sbiM.service('sbiModule_translate', function() {
	this.addMessageFile = function(file){
		messageResource.load([file,"messages"], function(){});
	};

	this.load = function(key,sourceFile) {
		var sf= sourceFile == undefined? 'messages' : sourceFile;
		return messageResource.get(key, sf);
	};
});

sbiM.service('sbiModule_restServices', function($http, $q, sbiModule_config,sbiModule_logger,$mdDialog) {
	var alteredContextPath=null;

	this.alterContextPath=function(cpat){
		alteredContextPath=cpat;
	}

	function getBaseUrl(endP_path) {
		var burl= alteredContextPath==null? sbiModule_config.contextName +'/api/'+ endP_path+"/"  : alteredContextPath+ "" + endP_path+"/" 
				alteredContextPath=null;
		return burl ;
	};

	this.get = function(endP_path, req_Path, item, conf) {
		(item == undefined || item==null) ? item = "" : item = "?" + encodeURIComponent(item).replace(/'/g,"%27").replace(/"/g,"%22").replace(/%3D/g,"=").replace(/%26/g,"&");
		sbiModule_logger.trace("GET: " +endP_path+"/"+ req_Path + "" + item,conf);
		return $http.get(getBaseUrl(endP_path) + "" + req_Path + "" + item, conf);
	};

	this.remove = function(endP_path, req_Path, item, conf) {
		item == undefined ? item = "" : item = "?" + item;
		sbiModule_logger.trace("REMOVE: "+endP_path+"/"+req_Path + "" + item,conf);
		return $http.post(getBaseUrl(endP_path) + "" + req_Path + "" + item, conf);
	};

	this.post = function(endP_path, req_Path, item, conf) {
		sbiModule_logger.trace("POST: "+endP_path+"/"+ req_Path,item,conf);
		return $http.post(getBaseUrl(endP_path) + "" + req_Path, item, conf);
	};

	this.delete = function(endP_path, req_Path, item, conf) {
		(item == undefined || item==null) ? item = "" : item = "?" + encodeURIComponent(item).replace(/'/g,"%27").replace(/"/g,"%22").replace(/%3D/g,"=").replace(/%26/g,"&");
		sbiModule_logger.trace("PUT:" +endP_path+"/"+req_Path+ "" + item,conf);
		return $http.delete(getBaseUrl(endP_path) + "" + req_Path, conf);
	};

	this.put = function(endP_path, req_Path, item, conf) {
		sbiModule_logger.trace("PUT: "+endP_path+"/"+req_Path,item,conf);
		return $http.put(getBaseUrl(endP_path) + "" + req_Path, item, conf);
	};
	
	this.errorHandler=function(text,title){
		var titleFin=title || "";
		var textFin=text;
		if(angular.isObject(text)){
			if(text.hasOwnProperty("errors")){
				textFin="";
				for(var i=0;i<text.errors.length;i++){
					textFin+=text.errors[i].message+" <br> ";
				}
			}else{
				textFin=JSON.stringify(text)
			}
		}
		
		var alert = $mdDialog.alert()
		.title(titleFin)
		.content(textFin)
		.ariaLabel('error') 
		.ok('OK') 
		return $mdDialog.show(alert); //can use the finally function
	};
	
	/*
	NEW METHODS
	*/
	var genericErrorHandling = function(data, status, headers, config, deferred) {
  		deferred.reject(data, status, headers, config);
	};
	
	var handleResponse = function(data, status, headers, config, deferred) {
		if(data.data != null){
			if ( data.data.hasOwnProperty("errors")) {
				
				genericErrorHandling(data, status, headers, config, deferred);
			} else {
				deferred.resolve(data, status, headers, config);
			}	
		}else{
			if ( data.status == 201) {
				deferred.resolve(data, status, headers, config);
				
			} else {
				genericErrorHandling(data, status, headers, config, deferred);
			}	
			
		}
		
	};
	// SAMPLE METHOD, this will be the implementation
	this.promiseGet = function(endP_path, req_Path, item, conf) {
		var deferred = $q.defer();
		
		// Required for passing JSON on a GET request
		if (item == undefined || item==null) {
			item = "";
		}else {
			item = "?" + 
				encodeURIComponent(item)
				.replace(/'/g,"%27")
				.replace(/"/g,"%22")
				.replace(/%3D/g,"=")
				.replace(/%26/g,"&");
		}
		
		sbiModule_logger.trace("GET: " +endP_path+"/"+ req_Path + "" + item, conf);
		
		deferred.notify('About to call async function');

		$http.get(getBaseUrl(endP_path) + "" + req_Path + "" + item, conf)
			.then(
					function successCallback(data, status, headers, config) {
						handleResponse(data, status, headers, config, deferred);
				  	}, 
				  	function errorCallback(data, status, headers, config) {
				  		genericErrorHandling(data, status, headers, config, deferred);
				  	}
			);

		return deferred.promise;
	};
	
	this.promisePost = function(endP_path, req_Path, item, conf) {
		var deferred = $q.defer();
		
		sbiModule_logger.trace("POST: " +endP_path+"/"+ req_Path + "" + item, conf);
		
		deferred.notify('About to call async function');

		$http.post(getBaseUrl(endP_path) + "" + req_Path , item, conf)
			.then(
					function successCallback(data, status, headers, config) {
						handleResponse(data, status, headers, config, deferred);
				  	}, 
				  	function errorCallback(data, status, headers, config) {
				  		genericErrorHandling(data, status, headers, config, deferred);
				  	}
			);

		return deferred.promise;
	};
	
	this.promisePut = function(endP_path, req_Path, item, conf) {
		var deferred = $q.defer();
		
		sbiModule_logger.trace("PUT: " +endP_path+"/"+ req_Path + "" + item, conf);
		
		deferred.notify('About to call async function');

		$http.put(getBaseUrl(endP_path) + "" + req_Path , item, conf)
			.then(
					function successCallback(data, status, headers, config) {
						handleResponse(data, status, headers, config, deferred);
				  	}, 
				  	function errorCallback(data, status, headers, config) {
				  		genericErrorHandling(data, status, headers, config, deferred);
				  	}
			);

		return deferred.promise;
	};
	
	this.promiseDelete = function(endP_path, req_Path, item, conf) {
		var deferred = $q.defer();
		
		sbiModule_logger.trace("DELETE: " +endP_path+"/"+ req_Path + "" + item, conf);
		
		deferred.notify('About to call async function');
		(item == undefined || item==null) ? item = "" : item = "?" + encodeURIComponent(item).replace(/'/g,"%27").replace(/"/g,"%22").replace(/%3D/g,"=").replace(/%26/g,"&");
		
		$http.delete(getBaseUrl(endP_path) + "" + req_Path+""+item, conf)
			.then(
					function successCallback(data, status, headers, config) {
						handleResponse(data, status, headers, config, deferred);
				  	}, 
				  	function errorCallback(data, status, headers, config) {
				  		genericErrorHandling(data, status, headers, config, deferred);
				  	}
			);

		return deferred.promise;
	};

});