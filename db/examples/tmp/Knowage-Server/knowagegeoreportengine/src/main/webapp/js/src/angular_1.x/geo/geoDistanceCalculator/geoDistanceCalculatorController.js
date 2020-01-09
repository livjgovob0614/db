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

/**
 * @authors Giovanni Luca Ulivo (GiovanniLuca.Ulivo@eng.it)
 *
 */

angular.module('geoModule')
.directive('geoDistanceCalculator',function(sbiModule_config){
	return{
		restrict: "E",
		templateUrl: sbiModule_config.contextName+'/js/src/angular_1.x/geo/geoDistanceCalculator/templates/geoDistanceCalculatorTemplate.jspf',
		controller: geoDistanceCalculatorControllerFunction,
		require: "^geoMap",
		scope: {
			id:"@"
		},
		disableParentScroll:true,
	}
})


function geoDistanceCalculatorControllerFunction($scope,$map,geo_interaction,sbiModule_translate){	
	$scope.showDC=false;
	$scope.measureList=[{label:sbiModule_translate.load("gisengine.geometry.line"),value:"LineString"},{label:sbiModule_translate.load("gisengine.geometry.polygon"),value:"Polygon"}];
	$scope.measureType="LineString";
	$scope.TypeM="LineString";
	$scope.disableButtons=false;
	$scope.mouseMoveListener;
	$scope.translate= sbiModule_translate;


	$scope.listOverlay=[];

	$scope.wgs84Sphere = new ol.Sphere(6378137);
	$scope.wgs84Ellipsoid = new ol.Ellipsoid(6378137, 1 / 298.257223563);
	$scope.sketch;
	$scope.helpTooltipElement;
	$scope.helpTooltip;
	$scope.measureTooltipElement;
	$scope.measureTooltip;
	$scope.continuePolygonMsg = sbiModule_translate.load("gisengine.geoDistanceCalculator.continueDrawPoligon");
	$scope.continueLineMsg = sbiModule_translate.load("gisengine.geoDistanceCalculator.continueDrawLine");
	$scope.draw;

	$scope.source = new ol.source.Vector();
	$scope.vector = new ol.layer.Vector({
		source: $scope.source,
		style: new ol.style.Style({
			fill: new ol.style.Fill({
				color: 'rgba(255, 255, 255, 0.2)'
			}),
			stroke: new ol.style.Stroke({
				color: '#ffcc33',
				width: 2
			}),
			image: new ol.style.Circle({
				radius: 7,
				fill: new ol.style.Fill({
					color: '#ffcc33'
				})
			})
		})
	});
	$map.addLayer($scope.vector);


	$scope.pointerMoveHandler=function(evt){

		if (evt.dragging) {
			return;
		}

		var helpMsg =  sbiModule_translate.load("gisengine.geoDistanceCalculator.startDrawing");
		var tooltipCoord = evt.coordinate;

		if ($scope.sketch) {

			var output;
			var geom = ($scope.sketch.getGeometry());
			if (geom instanceof ol.geom.Polygon) {
				output = $scope.formatArea(/** @type {ol.geom.Polygon} */ (geom));
				helpMsg = 	$scope.continuePolygonMsg;
				tooltipCoord = geom.getInteriorPoint().getCoordinates();
			} else if (geom instanceof ol.geom.LineString) {
				output = $scope.formatLength( /** @type {ol.geom.LineString} */ (geom));
				helpMsg = 	$scope.continueLineMsg;
				tooltipCoord = geom.getLastCoordinate();
			}
			$scope.measureTooltipElement.innerHTML = output;
			$scope.measureTooltip.setPosition(tooltipCoord);

		}
		if( $scope.helpTooltipElement){
			$scope.helpTooltipElement.innerHTML = helpMsg;
			$scope.helpTooltip.setPosition(evt.coordinate);
		}
	}



	$scope.addInteraction=function(loadMeasTool){
		console.log("$scope.measureType",$scope.TypeM)
		$scope.draw = new ol.interaction.Draw({
			source: $scope.source,
			type: /** @type {ol.geom.GeometryType} */ ($scope.TypeM),
			style: new ol.style.Style({
				fill: new ol.style.Fill({
					color: 'rgba(255, 255, 255, 0.2)'
				}),
				stroke: new ol.style.Stroke({
					color: 'rgba(0, 0, 0, 0.5)',
					lineDash: [10, 10],
					width: 2
				}),
				image: new ol.style.Circle({
					radius: 5,
					stroke: new ol.style.Stroke({
						color: 'rgba(0, 0, 0, 0.7)'
					}),
					fill: new ol.style.Fill({
						color: 'rgba(255, 255, 255, 0.2)'
					})
				})
			})
		});
		$map.addInteraction($scope.draw);

		if(loadMeasTool!=false){
			$scope.createMeasureTooltip();
		}
		$scope.createHelpTooltip();

		$scope.draw.on('drawstart',
				function(evt) {
			// set sketch
			$scope.sketch = evt.feature;
			$scope.disableButtons=true;
			$scope.appl();
		}, this);

		$scope.draw.on('drawend',
				function(evt) {
			$scope.measureTooltipElement.className = 'tooltip tooltip-static';
			$scope.measureTooltip.setOffset([0, -7]);
//			// unset sketch
			$scope.sketch = null;
			$scope.disableButtons=false;
			$scope.appl();
//			// unset tooltip so that a new one can be created
			$scope.measureTooltipElement = null;
			$scope.createMeasureTooltip();
		
		}, this);


	};



	/**
	 * Creates a new help tooltip
	 */
	$scope.createHelpTooltip=function() {
		if ($scope.helpTooltipElement) {
			$scope.helpTooltipElement.parentNode.removeChild($scope.helpTooltipElement);
		}
		$scope.helpTooltipElement = document.createElement('div');
		$scope.helpTooltipElement.className = 'tooltip';
		$scope.helpTooltip = new ol.Overlay({
			element: $scope.helpTooltipElement,
			offset: [15, 0],
			positioning: 'center-left'
		});

		$map.addOverlay($scope.helpTooltip);
	}


	/**
	 * Creates a new measure tooltip
	 */
	$scope.createMeasureTooltip=function() {
		if ($scope.measureTooltipElement) {
			$scope.measureTooltipElement.parentNode.removeChild($scope.measureTooltipElement);
		}
		$scope.measureTooltipElement = document.createElement('div');
		$scope.measureTooltipElement.className = 'tooltip tooltip-measure';
		$scope.measureTooltip = new ol.Overlay({
			element: $scope.measureTooltipElement,
			offset: [0, -15],
			positioning: 'bottom-center'
		});
		$scope.listOverlay.push($scope.measureTooltip);
		$map.addOverlay($scope.measureTooltip);

		$scope.appl();

	}

	$scope.appl=function(){
		if ($scope.$root.$$phase != '$apply') {
			$scope.$apply();
		}
	}
	$scope.changeMeasureType=function(meas){
		$scope.TypeM=meas;
		$map.removeInteraction($scope.draw);
		$scope.addInteraction();
	}



	/**
	 * format length output
	 * @param {ol.geom.LineString} line
	 * @return {string}
	 */
	$scope.formatLength = function(line) {
		var length;

		var coordinates = line.getCoordinates();
		length = 0;
		var sourceProj = $map.getView().getProjection();
		for (var i = 0, ii = coordinates.length - 1; i < ii; ++i) {
			var c1 = ol.proj.transform(coordinates[i], sourceProj, 'EPSG:4326');
			var c2 = ol.proj.transform(coordinates[i + 1], sourceProj, 'EPSG:4326');
//			length += $scope.wgs84Sphere.haversineDistance(c1, c2);
			length += $scope.wgs84Ellipsoid.vincentyDistance(c1, c2);

		}

		var output;
		if (length > 100) {
			output = (Math.round(length / 1000 * 100) / 100) +
			' ' + 'km';
		} else {
			output = (Math.round(length * 100) / 100) +
			' ' + 'm';
		}

		return output;

	};



	/**
	 * format area output
	 * @param {ol.geom.Polygon} polygon
	 * @return {string}
	 */
	$scope.formatArea = function(polygon) {
		var area;
		var sourceProj = $map.getView().getProjection();
		var geom = /** @type {ol.geom.Polygon} */(polygon.clone().transform(
				sourceProj, 'EPSG:4326'));
		var coordinates = geom.getLinearRing(0).getCoordinates();
		area = Math.abs($scope.wgs84Sphere.geodesicArea(coordinates));

		var output;
		if (area > 10000) {
			output = (Math.round(area / 1000000 * 100) / 100) +
			' ' + 'km<sup>2</sup>';
		} else {
			output = (Math.round(area * 100) / 100) +
			' ' + 'm<sup>2</sup>';
		}
		return output;
	};

	$scope.clearMeasure=function(){
		console.log("$scope.listOverlay",$scope.listOverlay)
		$scope.source.clear(); 

		for(var i=0;i< $scope.listOverlay.length;i++){
			$map.removeOverlay( $scope.listOverlay[i]);
		}
		$scope.listOverlay=[];
		$scope.createMeasureTooltip();
	}

	$scope.toggleDistanceCalculator=function(){
		if ($scope.sketch){
			return;
		}

		$scope.showDC=!$scope.showDC;

		//chiudo
		if (!$scope.showDC && $scope.helpTooltipElement) {
			$scope.helpTooltipElement.parentNode.removeChild($scope.helpTooltipElement);
			$scope.helpTooltipElement=null;
			$map.removeOverlay($scope.helpTooltip);
			$map.removeInteraction($scope.draw);
			$map.unByKey($scope.mouseMoveListener);/* Disabilito */
			geo_interaction.distance_calculator=false;
		}else{
			//apro
			$scope.mouseMoveListener=  $map.on('pointermove', $scope.pointerMoveHandler); /* Abilito */
			geo_interaction.distance_calculator=true;
			var x=($scope.listOverlay.length!=1);
			$scope.addInteraction(x);
		}
	}

	$scope.isEnabled=function(){
		if ($scope.sketch){
			return true;
		}
		return false;
	}



}