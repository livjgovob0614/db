/*
Knowage, Open Source Business Intelligence suite
Copyright (C) 2016 Engineering Ingegneria Informatica S.p.A.

Knowage is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

Knowage is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
angular
	.module('cockpitModule')
	.controller('mapWidgetEditControllerFunction',mapWidgetEditControllerFunction)

function mapWidgetEditControllerFunction(
		$scope,
		finishEdit,
		model,
		sbiModule_translate,
		sbiModule_restServices,
		sbiModule_config,
		cockpitModule_datasetServices,
		cockpitModule_generalServices,
		$mdDialog,
		mdPanelRef,
		$location,
		knModule_fontIconsService,
		$mdToast
		){
	$scope.translate=sbiModule_translate;
	$scope.newModel = angular.copy(model);
	$scope.availableAggregationFunctions = ['SUM','AVG','MIN','MAX','COUNT'];
	$scope.availableBackgroundLayers = [];
	$scope.availableOperators = [{'label':'==','value':'=='},{'label':'!=','value':'!='},{'label':'<','value':'<','range':true},{'label':'>','value':'>','range':true},{'label':'<=','value':'<=','range':true},{'label':'>=','value':'>=','range':true}];
	$scope.visualizationTypes = [{"name":"markers","enabled":true,"class":"markers"},{"name":"clusters","enabled":true,"class":"clusters"},{"name":"heatmap","enabled":true,"class":"heatmap"},{"name":"choropleth","enabled":true,"class":"choropleth"}];
	$scope.uploadImg = {};
	$scope.widgetSpinner = false;
  	$scope.getTemplateUrl = function(template){
  		return cockpitModule_generalServices.getTemplateUrl('mapWidget',template);
  	}

	$scope.setTargetLayer = function(layer){
		for(var t in $scope.newModel.content.layers){
			if($scope.newModel.content.layers[t].targetDefault && $scope.newModel.content.layers[t].dsId != layer.dsId){
				$scope.newModel.content.layers[t].targetDefault = false;
			}
		}
	}

  	$scope.setMarkersVisualizationType = function(layer,type){
  		layer.visualizationType = type;
  		if(layer.clusterConf && layer.clusterConf.enabled) layer.clusterConf.enabled = false;
  		if(layer.heatmapConf && layer.heatmapConf.enabled) layer.heatmapConf.enabled = false;
  		if(type == 'clusters'){
  			layer.clusterConf = layer.clusterConf || {};
  			layer.clusterConf.enabled = true;
  		}
  		if(type == 'heatmap'){
  			layer.heatmapConf = layer.heatmapConf || {};
  			layer.heatmapConf.enabled = true;
  		}
  	}

  	$scope.expandRow = function(layer,content){
  		if(content == 'metadata'){
  			$scope.confrontationDs = cockpitModule_datasetServices.getDatasetById(layer.dsId);
  			$scope.confrontationDsList = angular.copy($scope.confrontationDs.metadata.fieldsMeta);
  			for(var i in $scope.confrontationDsList){
  				if($scope.confrontationDsList[i].fieldType == 'SPATIAL_ATTRIBUTE'){
  					$scope.confrontationDsList.splice(i,1);
  				}
  			}
  		}
  		for(var t in $scope.newModel.content.layers){
  			if($scope.newModel.content.layers[t].expanded != content || $scope.newModel.content.layers[t].dsId != layer.dsId){
  				delete $scope.newModel.content.layers[t].expanded;
  			}
  			if($scope.newModel.content.layers[t].dsId==layer.dsId && $scope.newModel.content.layers[t].expanded != content){
  				$scope.newModel.content.layers[t].expanded = content;
	  		}else {
	  			if($scope.newModel.content.layers[t].dsId==layer.dsId && $scope.newModel.content.layers[t].expanded == content){
	  				delete $scope.newModel.content.layers[t].expanded;
		  		}
	  		}
  		}
  	}

  	$scope.addField = function(columns){
  		columns.unshift({});
  	}

  	$scope.checkDs = function(column){
  		for(var i in $scope.confrontationDs.metadata.fieldsMeta){
  			var confrontField = $scope.confrontationDs.metadata.fieldsMeta[i];
  			if(confrontField.name == column.name && confrontField.alias == column.alias && confrontField.fieldType == column.fieldType){
  				for(var k in $scope.confrontationDsList){
  					if($scope.confrontationDsList[k].name == column.name){
  						$scope.confrontationDsList.splice(k,1);
  						break;
  					}
  				}
  				return true
  			}
  		}
  		return false;
  	}

  	$scope.updateDsList = function(column, dsId){
  		for(var j in $scope.newModel.content.columnSelectedOfDataset[dsId]){
  			if($scope.newModel.content.columnSelectedOfDataset[dsId][j].name == column.name){
  				for(var k in $scope.confrontationDsList){
  					if($scope.confrontationDsList[k].name == column.name){
  						$scope.confrontationDsList.splice(k,1);
  						break;
  					}
  				}
  		  		for(var i in $scope.confrontationDs.metadata.fieldsMeta){
  		  			if(column.name == $scope.confrontationDs.metadata.fieldsMeta[i].name){
  		  				angular.merge($scope.newModel.content.columnSelectedOfDataset[dsId][j],$scope.confrontationDs.metadata.fieldsMeta[i]);
  		  				if(!$scope.newModel.content.columnSelectedOfDataset[dsId][j].aliasToShow){
  		  					$scope.newModel.content.columnSelectedOfDataset[dsId][j].aliasToShow = $scope.newModel.content.columnSelectedOfDataset[dsId][j].alias;
  		  				}
  		  				break;
  		  			}
  		  		}
  		  		break;
  			}
  		}
  	}

  	$scope.hasShownDetails = function(layer){
  		layer.hasShownDetails = false;
  		var columnsList = $scope.newModel.content.columnSelectedOfDataset[layer.dsId];
  		for(var i in columnsList){
  			if(columnsList[i].properties.showDetails){
  				layer.hasShownDetails = true;
  				return;
  			}
  		}

  	}

	$scope.setModalColumn = function(column, layer){
		var columnsList = $scope.newModel.content.columnSelectedOfDataset[layer.dsId];
		layer.modalSelectionColumn = (column.properties.modal) ? column.alias : undefined;

		for(var i in columnsList){
			if(columnsList[i].alias !== column.alias){
				columnsList[i].properties.modal = false;
			}
		}

	}

  	$scope.deleteColumn = function(layer,column){
  		layer.splice(layer.indexOf(column),1);
  	}

	$scope.deleteLayer = function(layer){
		var index=$scope.newModel.content.layers.indexOf(layer);
		$scope.newModel.content.layers.splice(index,1);
		var nextItem=$scope.newModel.content.layers[index];
		delete $scope.newModel.content.columnSelectedOfDataset[layer.dsId];

		$scope.reorder();
	}

	$scope.move = function(e,current,direction){
		var currentIdx = $scope.newModel.content.layers.indexOf(current);
		var switchToIdx = null;
		var switchTo = null;
		if (direction=='up') {
			switchToIdx = currentIdx - 1;
		} else {
			switchToIdx = currentIdx + 1;
		}
		$scope.newModel.content.layers[currentIdx] = $scope.newModel.content.layers[switchToIdx];
		$scope.newModel.content.layers[switchToIdx] = current;

		$scope.reorder();
	};

	$scope.addLayer = function(ev) {
		$scope.myLayersId = [];
		for(var m in $scope.newModel.content.layers){
			$scope.myLayersId.push($scope.newModel.content.layers[m].dsId);
		}
		$mdDialog.show({
			controller: function ($scope,$mdDialog) {
				$scope.widgetSpinner = true;
				$scope.availableSpatialLayers = [];

				// Datasets need to be added on cockpit
				var datasetList = cockpitModule_datasetServices.getAvaiableDatasets();
				for(var i in datasetList){
					var dataset = datasetList[i];
					if(dataset.hasSpatialAttributes && $scope.myLayersId.indexOf(dataset.id.dsId)==-1){
						$scope.availableSpatialLayers.push(dataset);
					}
				}
				$scope.widgetSpinner = false;

				//Add the layers to the newModel
				$scope.add = function(){
					if (!$scope.newModel.content.layers) $scope.newModel.content.layers = [];
					for(var k in $scope.availableSpatialLayers){
						if($scope.availableSpatialLayers[k].selected){
							var tempLayer = $scope.availableSpatialLayers[k];
							var columnSelected = [];
							var newLayer =  {
								"type": "DATASET",
								"dsId": tempLayer.id.dsId,
								"alias": tempLayer.label,
								"name": tempLayer.name,
								"defaultVisible": true
							}
							for(var i in tempLayer.metadata.fieldsMeta){
								tempLayer.metadata.fieldsMeta[i].aliasToShow = tempLayer.metadata.fieldsMeta[i].alias;
								columnSelected.push(tempLayer.metadata.fieldsMeta[i]);
							}
							$scope.newModel.content.layers.push(newLayer);
							var availableDatasets = cockpitModule_datasetServices.getAvaiableDatasets();
							var exists = false;
							for(var i in availableDatasets){
								if(availableDatasets[i].id.dsId == tempLayer.id.dsId) {
									exists = true;
									break;
								};
							}
							if(!exists) cockpitModule_datasetServices.addAvaiableDataset(tempLayer);
							if(!$scope.newModel.content.columnSelectedOfDataset) $scope.newModel.content.columnSelectedOfDataset = {};
							$scope.newModel.content.columnSelectedOfDataset[tempLayer.id.dsId] = columnSelected;
						}
					}

					$scope.reorder();
					$mdDialog.hide();
				}

				$scope.selectLayer = function(layer, e){
					e.preventDefault();
					e.stopImmediatePropagation();
					layer.selected = !layer.selected;
				}

				//Exit the dialog without adding
				$scope.cancel = function(){
					$scope.widgetSpinner = false;
					$mdDialog.cancel();
				}
			},
			scope: $scope,
			preserveScope:true,
	      templateUrl: $scope.getTemplateUrl('mapWidgetAddLayerDialog'),
	      targetEvent: ev,
	      clickOutsideToClose:true,
	      locals: {  }
	    })
//	    $scope.hideWidgetSpinner();
	    $scope.widgetSpinner = false;
//		$scope.safeApply();
  	}
//  	$scope.colorPickerOptions = {format:'hex'};
  	$scope.colorPickerOptions = {format:'rgb'};
  	$scope.setIconType = function(layer,type) {
  		if (!layer.markerConf) layer.markerConf={};
  		layer.markerConf.type = type;
  	}

  	$scope.getDimensionFromRadius = function(radius){
  		return (radius * 2) + 'px';
  	}

  	$scope.chooseIcon = function(ev, layer) {

  		$mdDialog.show({
			controller: function ($scope,$mdDialog) {
				$scope.availableIcons = knModule_fontIconsService.icons;

				$scope.activeLayer = {};
				angular.copy(layer,$scope.activeLayer);

				$scope.setIcon = function(family,icon){
					if(!$scope.activeLayer.markerConf) $scope.activeLayer.markerConf = {};
					$scope.activeLayer.markerConf.icon = icon;
					$scope.activeLayer.markerConf.icon.family = family.name;
					$scope.choose();
				}
				$scope.choose = function(){
					angular.copy($scope.activeLayer,layer);
					$mdDialog.hide();
				}
				$scope.cancel = function(){
					$mdDialog.cancel();
				}
			},
			scope: $scope,
			preserveScope:true,
	      templateUrl: $scope.getTemplateUrl('mapWidgetAddIconDialog'),
	      targetEvent: ev,
	      clickOutsideToClose:true,
	      locals: {  }
	    })
  	}

  	$scope.filterIcon = function (item) {
  	    return item.className.startsWith('fa');
  	};

  	$scope.chooseImg = function(ev, layer) {

  		$mdDialog.show({
			controller: function ($scope,$mdDialog) {

				$scope.imagesList = [];
				$scope.selectedImg = '';

				$scope.activeLayer = {};
				angular.copy(layer,$scope.activeLayer);

				$scope.getImagesList = function (){
					sbiModule_restServices.restToRootProject();
					sbiModule_restServices.get("1.0/images", 'listImages').then(
						function(result){
							$scope.imagesList = result.data.data;
						});
				}
				$scope.getImagesList();

				$scope.getImageUrl = function(img){
					return sbiModule_config.externalBasePath + "/restful-services/1.0/images/getImage?IMAGES_ID=" + img.imgId;
				}

				$scope.selectImg = function(img){
					$scope.selectedImg = img;
					$scope.activeLayer.markerConf.img = $scope.getImageUrl(img);
				}

				$scope.setIcon = function(family,icon){
					if(!$scope.activeLayer.markerConf) $scope.activeLayer.markerConf = {};
					$scope.activeLayer.markerConf.icon = icon;
					$scope.activeLayer.markerConf.icon.family = family.name;
				}

				$scope.upload = function(ev,layer){
					if($scope.uploadImg.fileName == "" || $scope.uploadImg.fileName == undefined){
						$mdToast.show($mdToast.simple().content(sbiModule_translate.load('sbi.cockpit.widgets.image.missinguploadfile')).position('top').action(
						'OK').highlightAction(false).hideDelay(5000));
					}else{
						var fd = new FormData();
						fd.append('uploadedImage', $scope.uploadImg.file);
						sbiModule_restServices.restToRootProject();
						sbiModule_restServices.post("1.0/images", 'addImage', fd, {transformRequest: angular.identity,headers: {'Content-Type': undefined}})
						.then(function(response) {
							if(response.data.success){
								$scope.getImagesList();
							}else{
								$mdToast.show($mdToast.simple().content(sbiModule_translate.load(response.data.msg)).position('top').action(
								'OK').highlightAction(false).hideDelay(5000));
							}
						});
					}
				};

				$scope.erase = function(ev,img){
					sbiModule_restServices.restToRootProject();
						var imageId = 'imageId='+img.imgId;
						sbiModule_restServices.get("1.0/images", 'deleteImage', imageId)
						.then(function(response) {
							$mdToast.show($mdToast.simple().content(sbiModule_translate.load(response.data.msg)).position('top').action(
							'OK').highlightAction(false).hideDelay(5000));
							if(response.data.success){
								$scope.getImagesList();
							}
						});
				};

				$scope.choose = function(){
					angular.copy($scope.activeLayer,layer);
					$mdDialog.hide();
				}
				$scope.cancel = function(){
					$mdDialog.cancel();
				}
			},
			scope: $scope,
			preserveScope:true,
	      templateUrl: $scope.getTemplateUrl('mapWidgetImagesLibrary'),
	      targetEvent: ev,
	      clickOutsideToClose:true,
	      locals: {  }
	    })
  	}


  	$scope.addToDatasets = function() {
  		$scope.newModel.dataset = {};
  		if ($scope.newModel.content.layers.length > 1) {
  			// $scope.newModel.dataset.dsId = [];
  			for (var k in $scope.newModel.content.layers){
  				// $scope.newModel.dataset.dsId.push($scope.newModel.content.layers[k].dsId);
  				if($scope.newModel.content.layers[k].targetDefault){
  					$scope.newModel.dataset.dsId = $scope.newModel.content.layers[k].dsId;
  				}
  			}
  		} else {
  			$scope.newModel.dataset.dsId = $scope.newModel.content.layers[0].dsId;
  		}
  	}


  	$scope.getThresholds = function(ev, measure) {

  		$mdDialog.show({
			controller: function ($scope,$mdDialog) {

				$scope.activeMeasure = {};

				angular.copy(measure,$scope.activeMeasure);

				$scope.addThreshold = function() {
					if(!$scope.activeMeasure.properties.thresholds){
						$scope.activeMeasure.properties.thresholds = [];
					}
					$scope.activeMeasure.properties.thresholds.push({});
				}

				$scope.deleteThreshold = function(threshold){
					$scope.activeMeasure.properties.thresholds.splice($scope.activeMeasure.properties.thresholds.indexOf(threshold),1);
				}

				$scope.set = function(){
					angular.copy($scope.activeMeasure,measure);
					$mdDialog.hide();
				}
				$scope.cancel = function(){
					$mdDialog.cancel();
				}
			},
			scope: $scope,
			preserveScope:true,
	      templateUrl: $scope.getTemplateUrl('mapWidgetMeasureThresholds'),
	      targetEvent: ev,
	      clickOutsideToClose:true,
	      locals: {  }
	    })
  	}


  	//MAIN DIALOG BUTTONS
	$scope.saveConfiguration=function(){
		for(var c in $scope.newModel.content.layers){
			$scope.hasShownDetails($scope.newModel.content.layers[c])
			if($scope.newModel.content.layers[c].expanded) delete $scope.newModel.content.layers[c].expanded;
		}

		if($scope.newModel.content.layers.length == 1){ // force target if only one layer is defined
			$scope.newModel.content.layers[0].targetDefault = true;
		}

		$scope.addToDatasets();
		mdPanelRef.close();
		angular.copy($scope.newModel,model);
		finishEdit.resolve();
  	}

	$scope.cancelConfiguration=function(){
  		mdPanelRef.close();
  		finishEdit.reject();
  	}

	$scope.reorder = function() {
		for(var idx in $scope.newModel.content.layers) {
			$scope.newModel.content.layers[idx].order = 1 + Number(idx);
		}
	}

	$scope.setStatic = function(newLayer) {
		if (newLayer.isStatic) {
			newLayer.defaultVisible = true;
			newLayer.targetDefault = false;
		}
	}

	function loadAvailableLayers() {
		sbiModule_restServices.restToRootProject();
		sbiModule_restServices.get(".", "layers")
			.then(function(response) {
				var root = response.data.root;

				root.forEach(function(el) {
						// Currently, only geojson is supported
						if (el.type == 'File') {
							$scope.availableBackgroundLayers
								.push({
										key: el.layerId,
										value: el.name+"-"+el.label
									}
								);
						}
					}
				);

			}, function(response) {
				console.log("Error loading available background layers");
			});

	}

	loadAvailableLayers();
}