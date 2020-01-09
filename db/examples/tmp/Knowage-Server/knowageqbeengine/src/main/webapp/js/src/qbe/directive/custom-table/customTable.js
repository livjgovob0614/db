/**
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
(function() {
	var scripts = document.getElementsByTagName("script");
	var currentScriptPath = scripts[scripts.length - 1].src;
	currentScriptPath = currentScriptPath.substring(0, currentScriptPath.lastIndexOf('/') + 1);

angular.module('qbe_custom_table', ['ngDraggable','exportModule','angularUtils.directives.dirPagination','formatModule'])
.directive('qbeCustomTable', function() {
    return {
        restrict: 'E',
        controller: qbeCustomTable,
        scope: {
            ngModel: '=',
        	expression: '=',
        	advancedFilters:'=',
        	distinct:'=',
            filters: '=',
            isTemporal: '='

        },
        templateUrl: currentScriptPath + 'custom-table.html',
        replace: true,
        transclude:true,
        link: function link(scope, element, attrs) {
        }
    };
})
.filter("orderedById",function(){
    return function(input,idIndex) {
        var ordered = [];
		for (var key in idIndex) {
			for(var obj in input){
				if(idIndex[key]==input[obj].id){
					ordered.push(input[obj]);
				}
			}

		}
        return ordered;
    };
})
.filter('format', function(formatter) {



	return function(item,field,row) {


		return formatter.format(item,field.format,row.dateFormatJava)

	};
});

function qbeCustomTable($scope, $rootScope, $mdDialog, sbiModule_translate, sbiModule_config, $mdPanel, query_service, $q, sbiModule_action,filters_service,expression_service,formatter){

	//$scope.smartPreview = query_service.smartView;
	$scope.query_service = query_service;
	$scope.completeResult = false;

	$scope.completeResultsColumns = [];

	$scope.previewModel = [];

	$scope.pageChanged = function(newPageNumber){
		$rootScope.$broadcast('start',{"itemsPerPage":$scope.itemsPerPage, "currentPageNumber":newPageNumber});
		}
	$scope.start = 0;

	$scope.itemsPerPage = 25;

	$scope.firstExecution = true;

	$scope.$watch('query_service.smartView',function(newValue,oldValue){
		if(newValue !==oldValue){
			//query_service.setSmartView(newValue);
			$rootScope.$emit('smartView', $scope.ngModel);
		}

	},true)

	$scope.translate = sbiModule_translate;

	$scope.selectedVisualization = 'previewData';

	$scope.orderAsc = true;

	$scope.openMenu = function($mdOpenMenu, ev) {
		originatorEv = ev;
		$mdOpenMenu(ev);
	};

	$scope.aggFunctions = [ "NONE", "SUM", "MIN", "MAX", "AVG", "COUNT", "COUNT_DISTINCT" ];
	$scope.tmpFunctions = ["YTD", "LAST_YEAR", "PARALLEL_YEAR", "MTD", "LAST_MONTH"];

	$scope.deleteAllFilters = function(){
		for(var i = 0;i< $scope.filters.length;i++){
			filters_service.deleteFilter($scope.filters,$scope.filters[i],$scope.expression,$scope.advancedFilters);
			i--;
		}

		expression_service.generateExpressions($scope.filters,$scope.expression,$scope.advancedFilters)
	}

	$scope.moveRight = function(currentOrder, column) {

		var newOrder = currentOrder + 1;
		var index = $scope.ngModel.indexOf(column);
		var indexOfNext = index + 1;

		if(index!=undefined && indexOfNext!=-1 && newOrder <= $scope.ngModel.length){
			$scope.ngModel[index] = $scope.ngModel[indexOfNext];
			$scope.ngModel[index].order = currentOrder;

			$scope.ngModel[indexOfNext] = column;
			$scope.ngModel[indexOfNext].order = newOrder;
		}

		$rootScope.$broadcast('move', {index:index,direction:+1});

	};

	$scope.moveLeft = function(currentOrder, column) {

		var newOrder = currentOrder - 1;
		var index = $scope.ngModel.indexOf(column);
		var indexOfBefore = index - 1;

		if(index!=undefined && indexOfBefore!=undefined && indexOfBefore!=-1){

			$scope.ngModel[index] = $scope.ngModel[indexOfBefore];
			$scope.ngModel[index].order = currentOrder;

			$scope.ngModel[indexOfBefore] = column;
			$scope.ngModel[indexOfBefore].order = newOrder;
		}

		$rootScope.$broadcast('move', {index:index,direction:-1});

	};

	$scope.changeAlias = function(field){
		$mdDialog.show({
            controller: function ($scope, $mdDialog) {

            	$scope.alias = field.alias;

                $scope.ok= function(){

                	field.index = $scope.ngModel.indexOf(field);
                	field.alias = $scope.alias;
                    $mdDialog.hide();
                }

                $scope.cancel = function(){
                	$mdDialog.hide();
                }
            },
            scope: $scope,
            locals :{field:field},
            preserveScope:true,
            templateUrl:  sbiModule_config.dynamicResourcesEnginePath +'/qbe/templates/alias.html',

            clickOutsideToClose:true
        })

	}

	$scope.applyFuntion = function(funct, id, entity) {
		$rootScope.$emit('applyFunction', {
			"funct" : funct,
			"fieldId" : id,
			"entity" : entity
		});
	};

	$scope.addTemporalParameter = function (field) {
		$rootScope.$broadcast('addTemporalParameter', field);
	}

	$scope.groupChanged = function(field) {
		if(field.group){
			field.funct = "NONE";
		}
	};
	$scope.modifyCalculatedField = function (row) {
		$rootScope.$broadcast('showCalculatedField',row);
	}

	$scope.removeColumns = function(fields) {
		var toRemove = [];
		for(var i in fields){
			toRemove.push({"id" : fields[i].id,"entity" : fields[i].entity
			})
		}
		$rootScope.$emit('removeColumns', toRemove);
	};

	$scope.toggleOrder = function (data) {

		 switch(data.ordering) {
		 	case "NONE":
		    	data.ordering = "ASC";
		        break;
		    case "ASC":
		    	data.ordering = "DESC";
		        break;
		    case "DESC":
		    	data.ordering = "ASC";
		        break;
		    default:
		    	data.ordering = "NONE";
		}

	}

	$scope.openFiltersAdvanced = function (){
		$rootScope.$broadcast('openFiltersAdvanced', true);
	}

	$scope.executeRequest = function () {
		$scope.firstExecution = true;
		$rootScope.$broadcast('executeQuery', {"start":$scope.start, "itemsPerPage":$scope.itemsPerPage/*, "fields": $scope.ngModel*/});
	}

	$scope.$on('queryExecuted', function (event, data) {
		$scope.completeResult = true;
		angular.copy(data.columns, $scope.completeResultsColumns);
		angular.copy(data.data, $scope.previewModel);
		$scope.results = data.results;
		if($scope.firstExecution&& !query_service.smartView){
			$scope.openPreviewTemplate(true, $scope.completeResultsColumns, $scope.previewModel, data.results);
			$scope.firstExecution = false;
		}
	});

	$scope.$on('start', function (event, data) {
		var start = 0;
		if(data.currentPageNumber>1){
			start = (data.currentPageNumber - 1) * data.itemsPerPage;
		}
		$rootScope.$broadcast('executeQuery', {"start":start, "itemsPerPage":data.itemsPerPage});
	});

	$scope.openPreviewTemplate = function (completeResult,completeResultsColumns,previewModel,totalNumberOfItems){

		var finishEdit=$q.defer();
		var config = {
				attachTo:  angular.element(document.body),
				templateUrl: sbiModule_config.dynamicResourcesEnginePath +'/qbe/templates/datasetPreviewDialogTemplate.html',
				position: $mdPanel.newPanelPosition().absolute().center().top("50%"),
				panelClass :"layout-row",
				fullscreen :true,
				controller: function($scope,mdPanelRef,sbiModule_translate,$mdDateLocale){


					var gridOptions = {
					        enableColResize: true,
					        enableSorting: false,
						    enableFilter: false,
						    pagination: false,
						    resizable: true,
						    onGridSizeChanged: resizeColumns,
						    suppressDragLeaveHidesColumns : true,
					        columnDefs :getColumns(completeResultsColumns),
					    	rowData: previewModel
						};



					    function getColumns(fields) {
							var columns = [];
							for(var f in fields){
								if(typeof fields[f] != 'object') continue;
								var tempCol = {
										"headerName":fields[f].label,
										"field":fields[f].name,
										"tooltipField":fields[f].name,
										"dataType":fields[f].dataType,
										"format":fields[f].format,
										"dateFormatJava":fields[f].dateFormatJava,
										"valueFormatter":function(params){
											return formatter.format(params.value,params.colDef.format,params.colDef.dateFormatJava)
											},
										};

								columns.push(tempCol);
							}
							return columns;
						}

					    function resizeColumns(){
							gridOptions.api.sizeColumnsToFit();
						}
					$scope.model ={"gridOptions":gridOptions, "completeresult": completeResult, "completeResultsColumns": completeResultsColumns, "previewModel": previewModel, "totalNumberOfItems": totalNumberOfItems, "mdPanelRef":mdPanelRef};
					$scope.$watch('model.previewModel',function(){
						console.log($scope.model)
						gridOptions.api.setRowData($scope.model.previewModel);
					},true)
					$scope.itemsPerPage = 20;
					$scope.currentPageNumber = 0;
					$scope.changeDatasetPage=function(itemsPerPage,currentPageNumber){

							$rootScope.$broadcast('start',{"itemsPerPage":itemsPerPage, "currentPageNumber":currentPageNumber});

					}

					$scope.changeDatasetPage($scope.itemsPerPage,$scope.currentPageNumber)
					$scope.closePanel = function () {

						angular.copy(null,$scope.changeDatasetPage)
						mdPanelRef.close();
						mdPanelRef.destroy();
					}

					$scope.$on("$destroy",function(){
						mdPanelRef.close();
						mdPanelRef.destroy();
						})
					$scope.translate = sbiModule_translate;
				},
				locals: {completeresult: completeResult, completeResultsColumns: completeResultsColumns, previewModel: previewModel, totalNumberOfItems: totalNumberOfItems},
				hasBackdrop: true,
				clickOutsideToClose: false,
				escapeToClose: false,
				focusOnOpen: true,
				preserveScope: true,
		};
		$mdPanel.open(config);
		return finishEdit.promise;

	}

	$scope.openHavings = function(field) {
		$rootScope.$broadcast('openHavings', field);
	}

	$scope.openFilters = function (field){
		$rootScope.$broadcast('openFilters', {"field":field});
	}

	$scope.checkDescription = function (field){
		var desc = 0;

		for (var i = 0; i < $scope.filters.length; i++) {
			if($scope.filters[i].leftOperandDescription == field.entity+" : "+field.name){
				field.filters.push($scope.filters[i]);
				desc++;
			}
		}
		if(desc == 0) {
			return "No filters";
		} else {
			return desc + " filters";
		}
	}

    $scope.countFilters = function (field) {
    	var filt = 0;
    	var hav = 0;
    	for (var i = 0; i < field.filters.length; i++) {
			if(field.filters[i].leftOperandDescription == field.entity+" : "+field.name){
				filt++;
			}
		}

    	for (var i = 0; i < field.havings.length; i++) {
			if(field.havings[i].leftOperandDescription == field.entity+" : "+field.name){
				hav++;
			}
		}

    	var total = filt + hav;
    	if(total == 0) {
    		return "";
    	} else {
    		return total + " filter/s";
    	}
    }

	$scope.openDialogForParams = function (model){
		$rootScope.$broadcast('openDialogForParams');
	}

	$scope.openDialogJoinDefinitions = function (model){
		$rootScope.$broadcast('openDialogJoinDefinitions');
	}

	$scope.$watch('ngModel',function(newValue,oldValue){
		if(newValue[0]){
			$scope.isChecked = newValue[0].distinct;

		}
	},true);


	$scope.showHiddenColumns = function () {
		for ( var field in $scope.ngModel) {
			$scope.ngModel[field].visible = true;
		}
	}

	$scope.idIndex = Array.apply(null, {length: 25}).map(Number.call, Number);


	$scope.basicViewColumns = [
								{
	                            	"label":$scope.translate.load("kn.qbe.custom.table.entity"),
	                            	"name":"entity"
	                        	},
	                        	{
	                        		"label":$scope.translate.load("kn.qbe.general.field"),
	                            	"name":"name"
	                        	},{
	                        		"label":$scope.translate.load("kn.qbe.general.alias"),
	                            	"name":"alias",
	                            	hideTooltip:true,
	                            	transformer: function() {
	                   	    		 return '<md-input-container class="md-block" style="margin:0"><input  ng-model="row.alias" ng-click="scopeFunctions.setAlias(row)"></md-input-container>';
	                   	    	 }
	                        	},
	                        	{
	                        		"label":$scope.translate.load("kn.qbe.custom.table.group"),
	                            	"name":"group",
	                    			hideTooltip:true,
	                            	transformer: function() {
	                            		return '<md-checkbox  ng-model="row.group"  ng-change="scopeFunctions.groupChanged(row)" aria-label="Checkbox"></md-checkbox>';
	                            	}
	                        	},
	                        	{
	                        		"label":$scope.translate.load("kn.qbe.selectgridpanel.headers.order"),
	                            	"name":"ordering",
	                            	hideTooltip:true,
	                            	transformer: function() {
	                            		return '<md-select  ng-model=row.ordering class="noMargin" ><md-option ng-repeat="col in scopeFunctions.orderingValues" value="{{col}}">{{col}}</md-option></md-select>';
	                            	}
	                        	},
	                        	{
	                        		"label":$scope.translate.load("kn.qbe.custom.table.aggregation"),
	                            	"name":"function",
	                            	hideTooltip:true,
	                            	transformer: function() {
	                            		return '<md-select ng-disabled="row.group" ng-model=row.funct class="noMargin" ><md-option ng-repeat="col in scopeFunctions.filterAggreagtionFunctions(row)" value="{{col}}">{{col}}</md-option></md-select>';
	                            	}
	                        	},
	                        	{
	                        		"label":$scope.translate.load("kn.qbe.custom.table.show.field"),
	                            	"name":"visible",
	                    			hideTooltip:true,
	                            	transformer: function() {
	                            		return '<md-checkbox  ng-model="row.visible"  aria-label="Checkbox"></md-checkbox>';
	                            	}
	                        	}
	]

	$scope.$watch('isTemporal',function(newValue,oldValue){
		if($scope.isTemporal){
			$scope.basicViewColumns.splice(5, 0, {
	    		"label":$scope.translate.load("kn.qbe.custom.table.function.temporal"),
	        	"name":"temporalOperand",
	        	hideTooltip:true,
	        	transformer: function() {
	        		return '<md-select ng-show="row.iconCls==measure" ng-model=row.temporalOperand class="noMargin" ><md-option ng-repeat="col in scopeFunctions.temporalFunctions" value="{{col}}">{{col}}</md-option></md-select>';
	        	}
	    	});
		}
	},true)

	$scope.treeSpeedMenu= [

        {
            label:sbiModule_translate.load("kn.qbe.custom.table.move.up"),
            icon:'fa fa-angle-up',
            color:'#a3a5a6',
            action:function(row,event){

            	$scope.basicViewScopeFunctions.moveUp(row)
            }
         },
         {
             label:sbiModule_translate.load("kn.qbe.custom.table.move.down"),
             icon:'fa fa-angle-down',
             color:'#a3a5a6',
             action:function(row,event){

            	 $scope.basicViewScopeFunctions.moveDown(row)
             }
          }
          ,
          {
      		"label":$scope.translate.load("kn.qbe.general.filters"),
      		icon:'fa fa-filter',
      		color:'#a3a5a6',
      		action:function(row,event){

      			$scope.basicViewScopeFunctions.openFilters(row);
            }
      	},
      	{
      		"label":$scope.translate.load("kn.qbe.general.havings"),
      		icon:'fa fa-check-square',
      		color:'#a3a5a6',
      		action:function(row,event){

      			$scope.basicViewScopeFunctions.openHavings(row);
            },
            visible: function (item) {

            	return item.funct && item.func != '' && item.funct !="NONE" ? true : false;
            }

      	},
      	{
    		"label": sbiModule_translate.load("kn.qbe.custom.table.modified.field"),
    		"icon": "fa fa-calculator",
    		"visible": function (item){
    			if(item.id.alias && item.id.expressionSimple) return true;
    			else return false
    		},
    		"action": function(item, event) {
    			$scope.basicViewScopeFunctions.modifyCalculatedField(item);
    		}
    	},
        {
            label:sbiModule_translate.load("kn.qbe.custom.table.delete.field"),
            icon:'fa fa-trash',
            color:'#a3a5a6',
            action:function(row,event){

           	 $scope.basicViewScopeFunctions.deleteField(row)
            }
         }
       ];

	$scope.basicViewScopeFunctions = {
		filterAggreagtionFunctions: function (row){

			if(row.fieldType=='attribute') return [ "NONE", "MIN", "MAX", "COUNT", "COUNT_DISTINCT" ];
			else return [ "NONE", "SUM", "MIN", "MAX", "AVG", "COUNT", "COUNT_DISTINCT" ];
		},
		aggregationFunctions: $scope.aggFunctions,
		orderingValues: ["NONE", "ASC", "DESC"],
		temporalFunctions: $scope.tmpFunctions,
		deleteField : function (row) {
			$scope.removeColumns([row]);
		},
		moveUp : function (row) {
			$scope.moveLeft(row.order, row);

		},
		moveDown : function (row) {
			$scope.moveRight(row.order, row);

		},
		openFilters : function (row) {
			$scope.openFilters(row);
		},
		openHavings : function (row) {
			$scope.openHavings(row);
		},
		isGrouped : function (row){
			for (var i = 0; i < $scope.ngModel.length; i++) {
				if($scope.ngModel[i].id==row.id && $scope.ngModel[i].group==true){
					return true;
				}
			}
		},
		groupChanged: function(row){
			$scope.groupChanged(row)
		},
		modifyCalculatedField : function (row){
			$scope.modifyCalculatedField(row);
		},
		setAlias : function (row){
			$scope.changeAlias(row);
		}
	};

	$scope.filtersListColumns = [
    	{"label": $scope.translate.load("kn.qbe.filters.condition"), "name": "operator"},
    	{"label": $scope.translate.load("kn.qbe.filters.target"), "name": "rightOperandDescription"}
    ]

	$scope.allFilters = [];

	$scope.showFilters = function(field) {

		$scope.allFilters = [];
		$scope.field = field;
		for (var i = 0; i < field.filters.length; i++) {
			$scope.filterObject = {
					"operator": field.filters[i].operator,
					"rightOperandDescription": field.filters[i].rightOperandDescription
			}

				$scope.allFilters.push($scope.filterObject);

		}

		for (var i = 0; i < field.havings.length; i++) {
			$scope.havingObject = {
				"operator": field.havings[i].operator,
				"rightOperandDescription": field.havings[i].rightOperandAggregator + "(" + field.havings[i].rightOperandDescription + ")"
			}
 			$scope.allFilters.push($scope.havingObject);
 		}

        if($scope.allFilters.length > 0) {
	    	$mdDialog.show({
	            controller: function ($scope, $mdDialog) {

	                $scope.ok= function(){
	                	console.log($scope)
	                    $mdDialog.hide();
	                }
	            },
	            scope: $scope,
	            preserveScope:true,
	            templateUrl:  sbiModule_config.dynamicResourcesEnginePath +'/qbe/templates/filtersInfo.html',

	            clickOutsideToClose:true
	        })
        } else {
        	$mdDialog.show(
        		$mdDialog.alert()
        		     .clickOutsideToClose(true)
        		     .title(field.entity + " : " + field.name)
        		     .textContent($scope.translate.load("kn.qbe.alert.nofilters"))
        		     .ok($scope.translate.load("kn.qbe.general.ok"))
            );
        }
	};

	$scope.showSQLQuery = function () {
		$rootScope.$broadcast('showSQLQuery', true);
	};

	$scope.showCalculatedField = function () {
		$rootScope.$broadcast('showCalculatedField');
	}

}
})();