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

var geoM = angular.module('geoModule', [ 'ngMaterial', 'ngAnimate','angular_table', 'sbiModule', 'color.picker', "expander-box","geo.settings" ]);

geoM.config(['$mdThemingProvider', function($mdThemingProvider) {
    $mdThemingProvider.theme('knowage')
    $mdThemingProvider.setDefaultTheme('knowage');
}]);

geoM.factory('geoModule_dataset', function() {
	var ds = {};
	return ds;
});

geoM.factory('geModule_datasetJoinColumnsItem', function() {
	var dsjc = {};
	return dsjc;
});

geoM.factory('geoModule_indicators', function() {
	var gi = [];
	return gi;
});

geoM.factory('geoModule_filters', function() {
	var gi = [];
	return gi;
});

geoM.factory('geoModule_templateLayerData', function() {
	var tld = {};
	return tld;
});

geoM.factory('$map', function() {
	var map = new ol.Map({
		target : 'map',
		layers : [],
		controls : [ new ol.control.Zoom() ],
		view : new ol.View({
			center : [ 1545862.460039, 4598265.489834 ],
			zoom : 5
		})
	});
	return map;
});

geoM.factory('baseLayer', function(geoModule_constant) {
	// todo thr following configuration should be loaded from a rest service
	// (from LayerCatalogue)
	var baseLayersConf = {};
	baseLayersConf[geoModule_constant.defaultBaseLayer] = {
			"OpenStreetMap" : {
				"type" : "TMS",
				"category" : "Default",
				"label" : "OpenStreetMap",
				"layerURL" : "http://tile.openstreetmap.org/",
				"layerOptions" : {
					"type" : "png",
					"displayOutsideMaxExtent" : true
				}
			}
//			,"OSM" : {
//				"type" : "OSM",
//				"category" : "Default",
//				"label" : "OSM"
//			}
	};

	return baseLayersConf;
});

geoM.service(
		'geoModule_layerServices',
		function($http, baseLayer, sbiModule_logger, $map,
				geoModule_thematizer, geo_interaction, crossNavigation,
				sbiModule_config, geoModule_template,
				sbiModule_restServices, geoModule_templateLayerData,$q) {

			var layerServ = this;
			var sFeatures;
			var select;
			var currentInteraction = {
					"type" : null,
					"obj" : null
			};
			this.selectedBaseLayer; // the selected base layer
			this.selectedBaseLayerOBJ;
			this.layerWithoutFilter=true;
			this.loadedLayer = {};
			this.loadedLayerOBJ = {};
			this.templateLayer = {};
			this.selectedFeatures = [];
			this.measure;
			this.filters={};
			this.filtersTemp={};

			// this.cachedFeatureStyles = {};

			this.setTemplateLayer = function(data) {
				//if is report without dataset, the template layer data is a array
				var tmpGeoModule_templateLayerData=data;
				geoModule_templateLayerData[data.layerName]=data;

				if(geoModule_template.noDatasetReport!=true){
					geoModule_thematizer.updateLegend(geoModule_template.analysisType);
				}

				var tmpTemplateLayer;

				if (tmpGeoModule_templateLayerData.type == "WMS") {
					var params = JSON.parse(tmpGeoModule_templateLayerData.layerParams);
					params.LAYERS = tmpGeoModule_templateLayerData.layerName;
					if(geoModule_template.noDatasetReport){
						var cqlfiler=geoModule_thematizer.loadCqlFilter(data);
						if(cqlfiler!="")
						params.CQL_FILTER=cqlfiler;
					}else{

						var sldBody = geoModule_thematizer.getWMSSlBody(tmpGeoModule_templateLayerData);
						params.SLD_BODY = sldBody;
					}


					tmpTemplateLayer = new ol.layer.Tile(
							{
								source : new ol.source.TileWMS(
										/** @type {olx.source.TileWMSOptions} */
										{
											url : sbiModule_config.externalBasePath+"restful-services/1.0/geo/getWMSlayer?layerURL="
											+ tmpGeoModule_templateLayerData.layerURL,
											params : params,
											options : JSON.parse(tmpGeoModule_templateLayerData.layerOptions)
										})
							});

				} else {
					//defines a source vector with the festures of the layer
					var vectorSource = new ol.source.Vector(
							{
								features : (new ol.format.GeoJSON()).readFeatures(tmpGeoModule_templateLayerData,
										{
									// dataProjection: 'EPSG:4326',
									featureProjection : 'EPSG:3857'
										})
							});
					//defines  a data vector with previous source vector + style
					var dataVector={source : vectorSource};

					if(geoModule_template.noDatasetReport!=true){
						dataVector.style =geoModule_thematizer.getStyle;
					}else{
						dataVector.style =geoModule_thematizer.fillStyleNoDataset(tmpGeoModule_templateLayerData.layerName);
					}
					tmpTemplateLayer = new ol.layer.Vector(dataVector);
				}

				tmpTemplateLayer.setZIndex(0);
				if(geoModule_template.hiddenTargetLayer.indexOf(tmpGeoModule_templateLayerData.layerName)!=-1){
					tmpTemplateLayer.setVisible(false);
				}
				layerServ.templateLayer[tmpGeoModule_templateLayerData.layerName]=tmpTemplateLayer;

				//adds the layer to the map
				$map.addLayer(tmpTemplateLayer);


				if(geoModule_template.currentView.center[0]==0 && geoModule_template.currentView.center[1]==0){
					//gets the initial position
					 if (tmpGeoModule_templateLayerData.type == "WMS"){
						$map.getView().setZoom(3);
					}else{
						if (tmpTemplateLayer.getSource().getFeatures().length>0 && tmpTemplateLayer.getSource().getFeatures()[0].getGeometry().getType() == 'Point')
							$map.getView().setCenter(tmpTemplateLayer.getSource().getFeatures()[0].getGeometry().getCoordinates());
						else
							$map.getView().setCenter(tmpTemplateLayer.getSource().getFeatures()[0].getGeometry().getCoordinates()[0][0][0]);

						if(tmpTemplateLayer.getSource().getFeatures().length>35){
							$map.getView().setZoom(4);
						}else{
							$map.getView().setZoom(5);
						}
					}
				}else{
					//gets the initial position from the template
					$map.getView().setCenter(geoModule_template.currentView.center);
					$map.getView().setZoom(geoModule_template.currentView.zoom);
				}


				// $map.getView().fit(layerServ.templateLayer.getProperties().source.getExtent(),$map.getSize());

				layerServ.addClickEvent();
			};

			this.overlay;
			var clickAdded=false;
			this.addClickEvent = function() {
				if(clickAdded){
					return;
				}
				clickAdded=true;
				var selectStyle = new ol.style.Style({
					stroke : new ol.style.Stroke({
						// color: '#000000',
						color : [ 0, 0, 0, 1 ],
						width : 2
					}),
					fill : new ol.style.Fill({
						color : "rgba(174, 206, 230, 0.78)"
					})
				});

				layerServ.overlay = new ol.Overlay(/** @type {olx.OverlayOptions} */
						({
							element : angular.element((document.querySelector('#popup')))[0],
						}));
				$map.addOverlay(layerServ.overlay);

				select = new ol.interaction.Select({
					condition : ol.events.condition.singleClick,
					style : selectStyle
				});
				$map.addInteraction(select);

				select.on('select',	function(evt) {

					var containWMS=false;
				 	for(var i in geoModule_templateLayerData){
						if(geoModule_templateLayerData[i].type == "WMS"){
							containWMS=true;
							break;
						}
					}


					if (geo_interaction.type == "identify"
						&& (evt.selected[0] == undefined || geo_interaction.distance_calculator)
						&& !containWMS) {
						layerServ.overlay.setPosition(undefined);
						return;
					}


					var findOne=false;
					var index=0;
					var checkedNumber=0;
					for(var i in geoModule_templateLayerData){
						if(!findOne){
							checkClickAction(geoModule_templateLayerData[i],layerServ.templateLayer[i],evt)
							.then(function(){
								checkedNumber++;
								findOne=true;
							},
							function(){
								checkedNumber++;
								if((Object.keys(geoModule_templateLayerData).length==checkedNumber) && !findOne){
									if(geo_interaction.type == "cross"){
										geo_interaction.setSelectedFeatures([]);
									}else{
										layerServ.overlay.setPosition(undefined);
									}
								}
							});
						}
						index++;
					}



				});
			}

			function checkClickAction(tmplLayerData,tmplLayer,evt){
				var deferredAction = $q.defer();
				// if is a WMS i must load the
				// properties from server
				if (tmplLayerData.type == "WMS") {
					var parser = new ol.format.WMSGetFeatureInfo();
					var infoFormat='application/json';
					if(geo_interaction.type == "cross"){
						infoFormat='application/vnd.ogc.gml';
					}

					var urlInfo = tmplLayer.getSource().getGetFeatureInfoUrl(
							evt.mapBrowserEvent.coordinate,
							$map.getView().getResolution(),
							'EPSG:3857',
							{
								'INFO_FORMAT' : infoFormat
							});
					$http.get(urlInfo,{transformResponse:function(data) {

						if(geo_interaction.type == "cross"){
							var features =	parser.readFeatures(data);
							data=JSON.stringify({features:features.length});

							if(features.length>0){
								//re-add all the deselected
								for(var i=0;i<evt.deselected.length;i++){
									evt.target.getFeatures().push(evt.deselected[i]);
								}
							}

							for(var i=0;i<features.length;i++){
								var contain=-1;
								for(var pk=0;pk<evt.target.getFeatures().getArray().length;pk++){
									if(angular.equals(evt.target.getFeatures().getArray()[pk].getId(),features[i].getId())){
										contain=pk;
										break;
									}
								}

								if(contain==-1){
									evt.target.getFeatures().push(features[i]);
								}else{
									evt.target.getFeatures().getArray().splice(contain,1);
								}
							}

							geo_interaction.selectedFeaturesCallbackFunctions[0]();

						}
						return JSON.parse(data);
					}}).success(function(data,status,headers,config) {
						if (data.hasOwnProperty("errors")) {
							sbiModule_logger.log("getGetFeatureInfoUrl non Ottenuti",data.errors);
						} else {
							if(geo_interaction.type == "cross"){
								if(data.features==0){
									deferredAction.reject();
								}else{
									deferredAction.resolve();
									layerServ.doClickAction(evt,{});
								}
							}else{
								if (data.features.length == 0) {
									deferredAction.reject();
								} else {
									deferredAction.resolve();
									layerServ.doClickAction(evt,data.features[0].properties);
								}
							}

						}
					})
					.error(function(data,status,headers,config) {
						deferredAction.reject();
						sbiModule_logger.log("getGetFeatureInfoUrl non Ottenuti ",status);
					});

				} else {
					var prop = evt.selected.length ? evt.selected[0].getProperties(): null;

					if(prop!=null){
						deferredAction.resolve();
						layerServ.doClickAction(evt,prop)
					}else{
						deferredAction.reject();

					}

				}

				return deferredAction.promise;
			}

			this.intersectFeature = function() {
				// select fetures with a box
				var sFeatures = select.getFeatures();
				var selectStyle = new ol.style.Style({
					stroke : new ol.style.Stroke({
						color : [ 0, 0, 0, 1 ],
						width : 2
					}),
					fill : new ol.style.Fill({
						color : "rgba(174, 206, 230, 0.78)"
					})
				});
				var dragBox = new ol.interaction.DragBox(
						{
							condition : ol.events.condition.platformModifierKeyOnly,
							style : selectStyle
						});
				currentInteraction.type = "intersect";
				currentInteraction.obj = dragBox;
				$map.addInteraction(dragBox);

				dragBox.on('boxend',function(e) {
					var selection = [];
					var extent = dragBox.getGeometry().getExtent();

					//multy template layers
					for(var i in layerServ.templateLayer){
						var vectorSource = layerServ.templateLayer[i].getSource();

						//not load the features of type wms
						if(geoModule_templateLayerData[i].type!="WMS"){
							vectorSource.forEachFeatureIntersectingExtent(extent,function(feature) {
								sFeatures.push(feature);
								selection.push(feature);
							});
						}else{
							sbiModule_logger.log("unable to load features from templateLayers of type wms");
						}

					}

					layerServ.selectedFeatures = selection;
					geo_interaction.setSelectedFeatures(layerServ.selectedFeatures);

				});

				// clear selection when drawing a new box and when
				// clicking on the map
				dragBox.on('boxstart', function(e) {
					layerServ.overlay.setPosition(undefined);
					sFeatures.clear();
				});

			}

			this.insideFeature = function() {
				var sFeatures = select.getFeatures();
				var selectStyle = new ol.style.Style({
					stroke : new ol.style.Stroke({
						color : [ 0, 0, 0, 1 ],
						width : 2
					}),
					fill : new ol.style.Fill({
						color : "rgba(174, 206, 230, 0.78)"
					})
				});
				var dragBox = new ol.interaction.DragBox(
						{
							condition : ol.events.condition.platformModifierKeyOnly,
							style : selectStyle
						});
				currentInteraction.obj = dragBox;
				currentInteraction.type = "inside";
				$map.addInteraction(dragBox);

				dragBox.on('boxend',function(e) {
					var selection = [];
					var extent = dragBox.getGeometry().getExtent();



					//multy template layers
					for(var i in layerServ.templateLayer){
						if(geoModule_templateLayerData[i].type!="WMS"){
							var vectorSource = layerServ.templateLayer[i].getSource();
							getInsideFeatures(vectorSource,extent,sFeatures,selection);
						}else{
							sbiModule_logger.log("unable to load features from templateLayers of type wms");
						}

					}


					layerServ.selectedFeatures = selection;
					geo_interaction.setSelectedFeatures(layerServ.selectedFeatures);


				});
				// clear selection when drawing a new box and when
				// clicking on the map
				dragBox.on('boxstart', function(e) {
					layerServ.overlay.setPosition(undefined);
					sFeatures.clear();
				});

			}

			function getInsideFeatures(vectorSource,extent,sFeatures,selection){
				vectorSource.forEachFeatureIntersectingExtent(extent,function(feature) {
					var geom = feature.getGeometry().getExtent();
					if (geom[0] > extent[0]) {
						if (geom[1] > extent[1]) {
							if (geom[2] < extent[2]) {
								if (geom[3] < extent[3]) {
									// it is inside the polygon
									sFeatures.push(feature);
									selection.push(feature);
								}
							}
						}
					}
				});
			}

			this.spy = function() {
				var element;

				var sFeatures = select.getFeatures();
				var imagery = new ol.layer.Tile({
					source : new ol.source.MapQuest({
						layer : 'osm',
						crossOriginKeyword: 'anonymous'
					}),
					opacity : 0.001
				});

				var container = document.getElementById('map');
				var coordinate;
				$map.addLayer(imagery);
				imagery.setZIndex(100000);
				var radius ;
				var ray ;

				document.addEventListener('keydown', function(evt) {

					if(evt.which === 17 && geo_interaction.selectedFilterType == "near"){
						//if press control select all features
						sFeatures.clear();
						layerServ.near($map.getCoordinateFromPixel(coordinate), ray);
						$map.removeLayer(imagery);
					}
				})

				// get the pixel position with every move
				var mousePosition = null;

				container.addEventListener('mousemove',function(event) {

					mousePosition = $map.getEventPixel(event);
					$map.render();
				});

				container.addEventListener('mouseout', function() {
					mousePosition = null;
					$map.removeLayer(imagery);
					$map.render();
				});

				// before rendering the layer, do some clipping
				imagery.on('precompose', function(event) {

					var ctx = event.context;
					var pixelRatio = event.frameState.pixelRatio;
					ctx.save();
					ctx.beginPath();
					radius = layerServ.calculateRay(layerServ.measure);
					ray = radius;
					if (mousePosition) {
						// only show a circle around the mouse

						ctx.arc(mousePosition[0] * pixelRatio,mousePosition[1] * pixelRatio, radius *  pixelRatio, 0, 2 * Math.PI);
						coordinate = [mousePosition[0], mousePosition[1]];
						var endCoordinate = [coordinate[0]+radius*  pixelRatio,coordinate[1]+radius*  pixelRatio];

						ray = layerServ.calculateDistance($map.getCoordinateFromPixel(coordinate),$map.getCoordinateFromPixel(endCoordinate));

						ctx.lineWidth = 5 * pixelRatio;
						ctx.strokeStyle = 'rgba(0,0,0,0.5)';
						ctx.stroke();
					}
					ctx.clip();
				});

				// after rendering the layer, restore the canvas context
				imagery.on('postcompose', function(event) {
					var ctx = event.context;
					ctx.restore();

				});


			}
			this.near = function(coordinate, ray) {

				var circleGeometry;
				var myCircle;
				var sFeatures = select.getFeatures();
				var selection = [];

				for(var i in layerServ.templateLayer){
					if(geoModule_templateLayerData[i].type!="WMS"){
						var features = layerServ.templateLayer[i].getSource().getFeatures();
						getNearFeatures(features,coordinate,ray,sFeatures,selection);
					}else{
						sbiModule_logger.log("unable to load features from templateLayers of type wms");
					}
				}



				if(selection.length>0){
					layerServ.selectedFeatures = selection;
					geo_interaction.setSelectedFeatures(layerServ.selectedFeatures);
				}

			}

			function getNearFeatures(features,coordinate,ray,sFeatures,selection){
				if (features.length > 0) {

					features.forEach(function(feature) {

						var geom = feature.getGeometry().getCoordinates();
						for (var i = 0; i < geom[0][0].length; i++) {
							var coordFeature = geom[0][0][i];
							var x=coordFeature[0];
							var y = coordFeature[1] ;


							if (layerServ.findIntersect(x, y , coordinate, ray)) {
								// ok
								selection.push(feature);
								sFeatures.push(feature);
								break;
							}
						}

					})

				}
			}

			this.findIntersect = function(x, y, center, ray) {
				//intersect inside a circle

				var x = Math.pow(x - center[0], 2);
				var y = Math.pow(y - center[1], 2);
				var difference = x + y;
				if (difference <= Math.pow(ray, 2)) {
					return true;
				} else {
					return false;
				}
			}




			this.setInteraction = function() {
				var type = geo_interaction.selectedFilterType;

				if (currentInteraction != {} && currentInteraction.type != type) {
					// remove $map
					$map.removeInteraction(currentInteraction.obj);

				}
				// set the tyoe of interaction

				currentInteraction.type = type;
				if (type == 'near') {
					layerServ.spy();

				} else if (type == 'intersect') {
					layerServ.intersectFeature();

				} else if (type == 'inside') {
					layerServ.insideFeature();
				}
			}
			this.doClickAction = function(evt, prop) {
				layerServ.selectedFeatures = evt.target.getFeatures().getArray();
				//adds features with shift+click
				geo_interaction.setSelectedFeatures(layerServ.selectedFeatures);
				var multiSelect = geoModule_template.crossNavigation  && geoModule_template.crossNavigationMultiselect ? geoModule_template.crossNavigationMultiselect : null;

				if (geo_interaction.type == "identify") {

					var coordinate = evt.mapBrowserEvent.coordinate;
					var hdms = ol.coordinate.toStringHDMS(ol.proj.transform(coordinate, 'EPSG:3857','EPSG:4326'));

					var txt = "";
					for ( var key in prop) {
						if (key != "geometry") {
							txt += "<p>" + key + " : " + prop[key] + "</p>";
						}
					}

					angular.element((document.querySelector('#popup-content')))[0].innerHTML = txt;
//					$map.getOverlays().getArray()[0].setPosition(coordinate);
					$map.getOverlays().getArray()[$map.getOverlays().getArray().length-1].setPosition(coordinate);

				}else if (multiSelect && geo_interaction.type == "cross" && geo_interaction.selectedFilterType == "near") {
					layerServ.spy();
				} else if (multiSelect && geo_interaction.type == "cross" && geo_interaction.selectedFilterType == "intersect") {
					layerServ.intersectFeature();
				} else if (multiSelect && geo_interaction.type == "cross" && geo_interaction.selectedFilterType == "inside") {
					layerServ.insideFeature();
				} else if (geo_interaction.type == "cross") {

					layerServ.overlay.setPosition(undefined); // hides eventual messages present on the map

//					var multiSelect = geoModule_template.crossNavigation  && geoModule_template.crossNavigationMultiselect ? geoModule_template.crossNavigationMultiselect : null;
					switch (multiSelect) {
					case (multiSelect !== undefined && true):
						/*
						 * Cross navigation with multiple selected features is handled in
						 * "geoCrossNavMultiselect" controller
						 */
						break;
					default:
						if (prop != null) {
							crossNavigation.navigateTo(layerServ.selectedFeatures[0]);
						}
					break;
					}
				}
			}



			this.calculateDistance = function(coord,endCoord){
				//calculate ray in km
				var wgs84Ellipsoid = new ol.Ellipsoid(6378137, 1 / 298.257223563);
				var sourceProj = $map.getView().getProjection();
				var c1 = ol.proj.transform(coord, sourceProj, 'EPSG:4326');
				var c2 = ol.proj.transform(endCoord, sourceProj, 'EPSG:4326');

				return wgs84Ellipsoid.vincentyDistance(c1,c2);
			}

			this.calculateRay = function(distance){
				//random coordinate
				var x = -12821852.872668605;
				var y = 5021607.010222939;
				var xF = x + distance;
				var yF = y + distance;

				var coord = $map.getPixelFromCoordinate([x,y]);
				var coordF =  $map.getPixelFromCoordinate([xF,yF]);
				return layerServ.calculateDistance(coord,coordF);

			}
			this.updateTemplateLayer = function(legendType) {
				 geoModule_thematizer.updateLegend(legendType);
				if (layerServ.templateLayer == undefined || Object.keys(layerServ.templateLayer).length == 0) {
					return;
				}
				for(var key in geoModule_templateLayerData){
					layerServ.updateTargetLayer(key);
				}
			};

			this.updateTargetLayer=function(key){
				if(geoModule_template.hiddenTargetLayer.indexOf(key)!=-1){
					layerServ.templateLayer[key].setVisible(false);
					return;
				}
				layerServ.templateLayer[key].setVisible(true);


				if (geoModule_templateLayerData[key].type == "WMS") {
					var params={};
					if(!geoModule_template.noDatasetReport){
						var sldBody = geoModule_thematizer.getWMSSlBody(geoModule_templateLayerData[key]);
						params.SLD_BODY = sldBody;
					}else{
						var cqlFilter=geoModule_thematizer.loadCqlFilter(geoModule_templateLayerData[key],!geoModule_template.noDatasetReport);
						if(cqlFilter!=""){
							params.CQL_FILTER=cqlFilter;
						}else{
							params.CQL_FILTER=null;
						}

					}

					layerServ.templateLayer[key].getSource().updateParams(params);
				} else {
					layerServ.templateLayer[key].changed();
				}
			}

			this.isSelectedBaseLayer = function(layer) {
				return angular.equals(this.selectedBaseLayerOBJ, layer);
			};

			this.layerIsLoaded = function(layer) {
				return (this.loadedLayerOBJ[layer.layerId] != undefined);
			};



			this.alterBaseLayer = function(layerConf) {
				var layer = this.createLayer(layerConf, true);
				if (layer != undefined) {
					$map.removeLayer(this.selectedBaseLayer);
					this.selectedBaseLayer = layer;
					if (this.selectedBaseLayerOBJ == undefined) {
						this.selectedBaseLayerOBJ = layerConf;
					}
					geoModule_template.selectedBaseLayer=layerConf.label;
					$map.addLayer(this.selectedBaseLayer);
					$map.updateSize();$map.render();
				}
			};



			this.toggleLayer = function(layerConf) {

				if (this.loadedLayer[layerConf.layerId] != undefined) {
					$map.removeLayer(this.loadedLayer[layerConf.layerId]);
					delete this.loadedLayer[layerConf.layerId];
					delete this.loadedLayerOBJ[layerConf.layerId];
					geoModule_template.layersLoaded[layerConf.label]=false;

				} else {
					var layer = this.createLayer(layerConf, false);
					if(layer.hasOwnProperty("$$state")){
						layer.then(function(tmpLayer) {
							layerServ.updateLayerLoaded(tmpLayer,layerConf)

						});
					}else{
						layerServ.updateLayerLoaded(layer,layerConf)
					}

				}
			};

			this.updateLayerLoaded=function(layer,layerConf){
				if (layer != undefined) {
					this.loadedLayer[layerConf.layerId] = layer;
					this.loadedLayerOBJ[layerConf.layerId] = layerConf;
					$map.addLayer(layer);
					geoModule_template.layersLoaded[layerConf.label]=true;
					$map.updateSize();$map.render();
				}
			}
			this.removeFromSelectedFilter = function(layerConf){
				//remove from selected filters the filters of removed layer

				var arr_tmp=[];
				for(var key in layerServ.filters){
					if(key==layerConf.layerId){

					}else{
						arr_tmp[key]=layerServ.filters[key];
					}
				}
				layerServ.filters = arr_tmp;
			}

			this.createLayer = function(layerConf, isBase) {
				var tmpLayer;
				var asyncCall;
				var name = layerConf.layerName;

				switch (layerConf.type) {
				case 'WMS':
					tmpLayer = new ol.layer.Tile({
						source : new ol.source.TileWMS(/** @type {olx.source.TileWMSOptions} */
								({
									url : layerConf.layerURL,
									params : JSON.parse(layerConf.layerParams),
									options : JSON.parse(layerConf.layerOptions)
								})),
								crossOriginKeyword: 'anonymous',
								name: name,
								type: 'WMS'
					});
					break;

				case 'WFS':
					var vectorSource = new ol.source.Vector({
						url : layerConf.layerURL,
						format : new ol.format.GeoJSON(),
						name: name,
						type: 'WFS'
							// options : JSON.parse(layerConf.layerOptions)
					});

					tmpLayer = new ol.layer.Vector({
						source : vectorSource,
						style: layerServ.applyFilter,
						name: name,
						type: 'WFS'
					});
					break;

				case 'TMS': // TODO check if work
					var options = (layerConf.layerOptions instanceof Object) ? layerConf.layerOptions : JSON.parse(layerConf.layerOptions);
					tmpLayer = new ol.layer.Tile({
						source : new ol.source.XYZ({
							tileUrlFunction : function(coordinate) {
								if (coordinate == null) {
									return "";
								}
								var z = coordinate[0];
								var x = coordinate[1];
								// var y = (1 << z) -coordinate[2] - 1;
								var y = -coordinate[2] - 1;
								return layerConf.layerURL + '' + z + '/' + x + '/' + y + '.' + options.type;
							},

						}),
						crossOriginKeyword: 'anonymous',
						name: name,
						type: 'TMS'
					});
					break;

				case 'OSM':
					tmpLayer = new ol.layer.Tile({
						source : new ol.source.MapQuest({
							layer : 'osm',
							crossOriginKeyword: 'anonymous'
						}),
						name: name,
						type: 'OSM'
					});
					break;

				case 'File':

					tmpLayer= this.getLayerFromFile(layerConf);
					tmpLayer.name= name,
					tmpLayer["type"]= 'File'
						break;

				default:
					console.error('Layer type [' + layerConf.type + '] not supported');
				break;

				}
				if (isBase) {
					tmpLayer.setZIndex(-1)
				} else {

				}

				return tmpLayer;

			};

			this.getLayerFromFile=function(layerConf){
				var deferredLayer = $q.defer();
				sbiModule_restServices.alterContextPath( sbiModule_config.externalBasePath+'restful-services/');
				sbiModule_restServices.post("2.0/analyticalDriversee", 'getFileLayer', { layerUrl:layerConf.pathFile })
				.then(
						function(response, status, headers, config) {

								var vectorSource = new ol.source.Vector(
										{
											features : (new ol.format.GeoJSON()).readFeatures(response.data,
													{
												featureProjection : 'EPSG:3857'
													})
										});

								var tmpLayer= new ol.layer.Vector({
									source : vectorSource,
									style: layerServ.applyFilter
								});
								deferredLayer.resolve(tmpLayer);

						},function(response, status, headers, config) {
							sbiModule_restServices.errorHandler(response.data,"Error while attempt to load file layer ");
							deferredLayer.reject()
						});

				return deferredLayer.promise;

			}
			this.removeSelectPopup=function(){
				layerServ.overlay.setPosition(undefined);
				select.getFeatures().clear();
			}

			this.applyFilter = function(feature, resolution){


				var image = new ol.style.Circle({
					radius: 5,
					fill: null,
					stroke: new ol.style.Stroke({color: 'red', width: 1})
				});

				var styles = {
						'Point': new ol.style.Style({
							image: image
						}),
						'LineString': new ol.style.Style({
							stroke: new ol.style.Stroke({
								color: 'green',
								width: 1
							})
						}),
						'MultiLineString': new ol.style.Style({
							stroke: new ol.style.Stroke({
								color: 'green',
								width: 1
							})
						}),
						'MultiPoint': new ol.style.Style({
							image: image
						}),
						'MultiPolygon': new ol.style.Style({
							stroke: new ol.style.Stroke({
								color: "#3399cc",
								width: 1
							})
						}),
						'Polygon': new ol.style.Style({
							stroke: new ol.style.Stroke({
								color: "#3399cc",
								width: 1
							})
						}),

						'GeometryCollection': new ol.style.Style({
							stroke: new ol.style.Stroke({
								color: 'magenta',
								width: 2
							}),
							fill: new ol.style.Fill({
								color: 'magenta'
							}),
							image: new ol.style.Circle({
								radius: 10,
								fill: null,
								stroke: new ol.style.Stroke({
									color: 'magenta'
								})
							})
						}),
						'Circle': new ol.style.Style({
							stroke: new ol.style.Stroke({
								color: 'red',
								width: 2
							}),
							fill: new ol.style.Fill({
								color: 'rgba(255,0,0,0.2)'
							})
						})
				};
				var layer_founded;
				var sameFeature = function(featureTemp){
					return (feature === featureTemp) ? true : false;
				};
				$map.getLayers().forEach(function(layer){
					var source = layer.getSource();
					if(source instanceof ol.source.Vector){
						var features = source.getFeatures();
						if(features.length > 0){
							var found = features.some(sameFeature);
							if(found){
								layer_founded = layer;
							}
						}
					}
				});


				/*	if(layerServ.layerWithoutFilter){
					return [styles[feature.getGeometry().getType()]];
				}*/


				var applFilter=undefined;

				var propertiesFeature = feature.getProperties();

				var cont = 0;
				var idLayer ;
				for(var layerCurr in layerServ.loadedLayer){
					if(layer_founded==layerServ.loadedLayer[layerCurr]){
						idLayer = layerCurr;
						break;
					}
				}
				for(var key in layerServ.filters){
					if(key==idLayer){
						for(var i =0;i<layerServ.filters[key].length;i++){

							var value = propertiesFeature[layerServ.filters[key][i].filter];
							if(layerServ.filters[key][i].model==""){
								cont ++;
								//applFilter=true;
							}
							var valuesInsert = layerServ.filters[key][i].model.split(",");

							for(var k=0;k<valuesInsert.length;k++){
								if(value==valuesInsert[k]){
									//if contains filter
									return  [styles[feature.getGeometry().getType()]];
								}
							}
						}
						if(cont == layerServ.filters[key].length){
							applFilter = false ;
						}else{
							applFilter = true;
						}
					}


				}
				if(applFilter==undefined){
					applFilter = false ;
				}

				if(applFilter){

					return null;
				}else{


					return  [styles[feature.getGeometry().getType()]];
				}


			}


			this.setLayerFilter= function(layerConf,filters) {
				layerServ.filters=filters;
			}



		});



geoM.factory('geoModule_constant', function(sbiModule_translate) {
	var cont = {
			analysisLayer : sbiModule_translate.load("gisengine.constant.analysisLayer"),
			templateLayer : sbiModule_translate.load("gisengine.constant.templateLayer"),
			noCategory : sbiModule_translate.load("gisengine.constant.noCategory"),
			defaultBaseLayer : sbiModule_translate.load("gisengine.constant.defaultBaseLayer")
	}
	return cont;
});


