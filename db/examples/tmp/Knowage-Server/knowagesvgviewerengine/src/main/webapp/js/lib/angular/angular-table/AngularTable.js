/**
 * @authors Giovanni Luca Ulivo (GiovanniLuca.Ulivo@eng.it)
 * 
 */
var scripts = document.getElementsByTagName("script")
var currentScriptPath = scripts[scripts.length-1].src;

angular.module('angular_table', [ 'ngMaterial','angularUtils.directives.dirPagination','ng-context-menu','ui.tree'])
.directive('angularTable',
		function($compile) {
	return {
		templateUrl: currentScriptPath.substring(0, currentScriptPath.lastIndexOf('/') + 1) + 'template/angular-table.html',
		transclude : true,
		controller : TableControllerFunction,
		priority:1000,
		scope : {
			ngModel : '=',
			id : "@",
			columns:"=?",//items to display. if not defined list all element ordering by name
			columnsSearch : "=?", // columns where search
			showSearchBar : '=', //default false
			searchFunction : '&',
			pageCangedFunction : "&",
			totalItemCount : "=?", //if not present, create a non sync pagination and page change function is not necessary
			currentPageNumber : "=?",
			noPagination : "=?", //not create pagination and  totalItemCount and pageCangedFunction are not necessary
			clickFunction : "&", //function to call when click into element list
			menuOption : "=?", //menu to open with right click
			speedMenuOption : "=?", //speed menu to open with  button at the end of item
			selectedItem : "=?", //optional to get the selected  item value
			highlightsSelectedItem : "=?",
			multiSelect : "=?",
			scopeFunctions : "=?",
			dragEnabled: "=?",
			dragDropOptions :"=?",
			enableClone:"=?",
			showEmptyPlaceholder :"=?",
			noDropEnabled:"=?"
		},
		compile: function (tElement, tAttrs, transclude) {
			 return {
			        pre: function preLink(scope, element, attrs, ctrl, transclud) { 
			        	if( attrs.dragEnabled && (attrs.dragEnabled==true || attrs.dragEnabled=="true")){
							var table=angular.element(element[0].querySelector("table"))
							table.attr('ui-tree',"dragDropOptions");
							table.attr('drag-delay',"600");
							table.attr('data-clone-enabled',"enableClone");
							table.attr('data-empty-placeholder-enabled',"showEmptyPlaceholder");
							table.attr('data-nodrop-enabled',"noDropEnabled");
						
							
							$compile(table)(scope)
							var body=angular.element(table[0].querySelector("tbody"));
							body.attr('ui-tree-nodes',"");
							body.attr('ng-model',"ngModel");
						}
			        },
			        post: function postLink(scope, element, attrs, ctrl, transclud) {

						//TODO ngModel obbligatorio


						var id="at";
						if(attrs.id){
							id=attrs.id;
						}
						var template=angular.element(element[0].querySelector("#angularTableTemplate"))
						template.addClass(id+"ItemBox");


						var table= angular.element(template[0].querySelector("table"));
						var thead= angular.element(table[0].querySelector("thead"));
						var tbody= angular.element(table[0].querySelector("tbody"));
						var footerBox= angular.element(template[0].querySelector("angular-table-footer"));

						//create the head
						thead.attr('angular-table-head',"");
						//create tbody
						tbody.attr('angular-table-body',"");

						//create the column of the table. If attrs.column is present load only this column, else load all the columns
						scope.tableColumns=[];
						
						
						if(attrs.multiSelect && (attrs.multiSelect==true || attrs.multiSelect=="true" || scope.multiSelect==true)){
							scope.tableColumns.push({label:"--MULTISELECT--",name:"--MULTISELECT--",size:"30px"});
							thead.attr('multi-select',true);
							tbody.attr('multi-select',true);
						}
						
						if(attrs.dragEnabled && (attrs.dragEnabled==true || attrs.dragEnabled=="true")){
							scope.tableColumns.push({label:"",name:"--DRAG_AND_DROP--",size:"1px"});
						}else{
							tbody.attr('no-drag-and-drop',true);
						}
						if(attrs.columnsSearch){
							var colSearch=scope.columnsSearch;
							if(Object.prototype.toString.call(colSearch)=="[object String]"){
								//if is a string, convert it to object
								scope.columnsSearch=JSON.parse(colSearch);
							}
						}
						
						if(attrs.columns){
							var col=scope.columns;
							if(Object.prototype.toString.call(col)=="[object String]"){
								//if is a string, convert it to object
								col=JSON.parse(col);
							}
							 
							for(var i=0;i<col.length;i++){
								var tmpColData={};
								if(Object.prototype.toString.call(col[i])=="[object Object]"){
									//json of parameter like name, label,size
									tmpColData.name=col[i].name || "---";
									tmpColData.label=col[i].label || tmpColData.name;
									tmpColData.size=col[i].size || "";
								}else{
									//only the col name
									tmpColData.label=col[i];
									tmpColData.name=col[i];
									tmpColData.size="";
								}

								scope.tableColumns.push(tmpColData);
							}
						}else{
							//load all
							var firstValue=scope.ngModel[0];
							if(firstValue!=undefined){
								for(var key in firstValue){
									var tmpColData={};
									tmpColData.label=key;
									tmpColData.name=key;
									tmpColData.size="";
									scope.tableColumns.push(tmpColData);
								}
							}
						}
						//add speed menu column at the end of the table
						if(attrs.speedMenuOption ){
							scope.tableColumns.push({label:"",name:"--SPEEDMENU--",size:"30px"});
							thead.attr('speed-menu-option',true);
							tbody.attr('speed-menu-option',true);
						}

						
						
						//check for pagination
						if(!attrs.totalItemCount){
							scope.currentPageNumber=undefined;
							scope.totalItemCount=undefined;
							var paginBox= angular.element(footerBox[0].querySelector("dir-pagination-controls")); 
							paginBox.removeAttr("on-page-change");
							tbody.attr('local-pagination',true);
						}
						if(attrs.noPagination){
							scope.itemsPerPage=999;
						}else{
							scope.itemsPerPage=3;
						}


					


						$compile(thead)(scope);
						$compile(tbody)(scope);

						//add search tab
						if(!attrs.showSearchBar || attrs.showSearchBar==false || attrs.showSearchBar=="false"){
							angular.element(template[0].querySelector(".tableSearchBar")).css("display","none");
						}

						var showAngularTableActions=false;
						if (attrs.showSearchBar && (attrs.showSearchBar==true || attrs.showSearchBar=="true")) {
							showAngularTableActions=true;
							if (!attrs.searchFunction) {
								scope.localSearch = true;
							}
						}

						if (attrs.multiSelect) {
							if (!attrs.selectedItem) {
								scope.selectedItem = [];
							}
						}

						if(!showAngularTableActions){
							angular.element(template[0].querySelector("angular-table-actions")).css("display","none");
						}
						
						


					
			        }
			      }
			
			
		},
		
	}
})
.directive('angularTableHead',
		function($compile) {
	return {
		templateUrl: 'headThTemplate.html',
		transclude : true,
		controller : TableHeaderControllerFunction,
		link: function(scope, element, attrs, ctrl, transclude) {


		}

	}
})
.directive('angularTableBody',
		function($compile) {
	return {
		templateUrl: 'bodyTemplate.html',
		transclude : true,
		controller : TableBodyControllerFunction,
		priority:100,
		compile: function (tElement, tAttrs, transclude) {
			if( tAttrs.localPagination=="true" || tAttrs.localPagination==true){
				angular.element(tElement.children()[0]).removeAttr('total-items');
				angular.element(tElement.children()[0]).removeAttr('current-page');
			}
			
			if( tAttrs.noDragAndDrop=="true" ){
				angular.element(tElement.children()[0]).removeAttr('ui-tree-node');
			}
			
		},
		link: function(scope, element, attrs, ctrl, transclude) {
			console.log("scope",scope)
		}
	}
})
.directive('angularTableFooter',
		function($compile) {
	return {
		template:"<div ng-transclude></div>",
		transclude : true,
		link: function(scope, element, attrs, ctrl, transclude) {
			console.log("inity footrer")
		}
	}
})
.directive('actionBackgroundColor',
		function($compile) {
	return {
		link: function(scope, element, attrs, ctrl, transclude) {
			element.css("background-color",attrs.actionBackgroundColor);
		}
	}
})
.directive('actionColor',
		function($compile) {
	return {
		link: function(scope, element, attrs, ctrl, transclude) {
			element.css("color",attrs.actionColor);
		}
	}
})
.directive('dynamichtml', function ($compile) {
	return {
		restrict: 'A',
		replace: true,
		link: function (scope, ele, attrs) {
			scope.$watch(attrs.dynamichtml, function(html) {
				ele.html(html);
				$compile(ele.contents())(scope);
			});
		}
	};
})
.filter('filterBySpecificColumns', function(){
	return function(items,columnsSearch,searchTerm,localSearch) { 
		if(searchTerm==undefined || searchTerm==""){
			return items;
		}
		var filtered = [];
		for(var item in items){
			if(columnsSearch!=undefined && columnsSearch.length!=0){
				for( var cols in columnsSearch){
					if (items[item][columnsSearch[cols]].toString().toUpperCase().indexOf(searchTerm.toUpperCase()) !== -1) {
						filtered.push(items[item]);
						break;
					}
				};
			}else{
				if (JSON.stringify(items[item]).toString().toUpperCase().indexOf(searchTerm.toUpperCase()) !== -1) {
					filtered.push(items[item]);
				}
			}
		};
		return filtered;
	}});


function TableControllerFunction($scope,$timeout){
	$scope.currentPageNumber = 1;
	$scope.tmpWordSearch = "";
	$scope.prevSearch = "";
	$scope.localSearch = false;
	$scope.searchFastVal = "";
	$scope.column_ordering;
	$scope.reverse_col_ord = false;
	$scope.internal_column_ordering;
	$scope.internal_reverse_col_ord = false;


	$scope.searchItem = function (searchVal) {
		if ($scope.localSearch) {
			$scope.searchFastVal = searchVal;
		} else {
			$scope.tmpWordSearch = searchVal;
			$timeout(function () {
				if ($scope.tmpWordSearch != searchVal || $scope.prevSearch == searchVal) {
					return;
				}
				$scope.prevSearch = searchVal;
				$scope.searchFunction({
					searchValue : searchVal,
					itemsPerPage : $scope.itemsPerPage,
					columnsSearch : $scope.columnsSearch,
					columnOrdering : $scope.column_ordering,
					reverseOrdering : $scope.reverse_col_ord
				});

			}, 1000);
		}
	}

	// pagination item
	$scope.changeWordItemPP=function(){
		if(angular.element(document.querySelector('#angularTableTemplate.'+$scope.id+'ItemBox'))[0]==undefined){
			return;
		}

		var box=angular.element(document.querySelector('#angularTableTemplate.'+$scope.id+'ItemBox'))[0]
		if(box==undefined){return;}

		var tableAction=angular.element(document.querySelector('#angularTableTemplate.'+$scope.id+'ItemBox angular-table-actions'))[0];
		var footerTab= angular.element(document.querySelector('#angularTableTemplate.'+$scope.id+'ItemBox angular-table-footer'))[0];
		var headButton =  angular.element(document.querySelector('#angularTableTemplate.'+$scope.id+'ItemBox table thead'))[0];
		var listItemTemplBox = angular.element(document.querySelector('#angularTableTemplate.'+$scope.id+'ItemBox table tbody tr'))[0];

		var boxHeight = box.offsetHeight;
		var tableActionHeight=  tableAction==undefined? 0 : tableAction.offsetHeight ;
		var footerTabHeigth= footerTab==undefined? 0 : footerTab.offsetHeight ;
		var headButtonHeight= headButton==undefined? 0 : headButton.offsetHeight ;
		var listItemTemplBoxHeight = listItemTemplBox==undefined? 0 : listItemTemplBox.offsetHeight;

		var nit = parseInt((boxHeight - tableActionHeight - footerTabHeigth -headButtonHeight) / listItemTemplBoxHeight);
		$scope.itemsPerPage = nit <= 0 ? 1 : nit;
		if(firstLoad){
			$scope.pageCangedFunction({itemsPerPage:$scope.itemsPerPage,newPageNumber:1,searchValue:''});
			firstLoad=false;
		}

	}
	var firstLoad=true;
	$timeout(function() {
		$scope.changeWordItemPP();
	},0)

	$scope.$watch(function() {
		var elem=angular.element(document.querySelector('#angularTableTemplate.'+$scope.id+'ItemBox'))[0];
		return elem==undefined? null:  elem.offsetHeight;
	}, function(newValue, oldValue) {

		if (newValue != oldValue) {
			$scope.changeWordItemPP();
		}
	}, true);

}

function TableBodyControllerFunction($scope){
	$scope.clickItem = function (row, cell,evt) {
		if ($scope.multiSelect) {
			$scope.toggleMultiSelect(row,evt);
		} else {
			$scope.selectedItem = row;
		}

		$scope.clickFunction({
			item : row,
			cell : cell,
			listId : $scope.id,
			row : row,
			column : cell
		});
	}

	$scope.toggleMultiSelect = function (row,evt) {
		evt.stopPropagation();
		var index = $scope.indexInList($scope.selectedItem, row);
		if (index > -1) {
			$scope.selectedItem.splice(index, 1)
		} else {
			$scope.selectedItem.push(row);
		}
	}

	$scope.indexInList = function (list, item) {
		for (var i = 0; i < list.length; i++) {
			if (angular.equals(list[i], item)) {
				return i;
			}
		}
		return -1;
	}

	$scope.isSelected = function (item) {
		if ($scope.multiSelect) {
			return $scope.indexInList($scope.selectedItem, item) > -1;

		} else {
			return angular.equals($scope.selectedItem, item);
		}

	}



}

function TableHeaderControllerFunction($scope,$timeout){
	$scope.multiSelectVal=false;
	$scope.selectAll=function(){
		$scope.multiSelectVal=!$scope.multiSelectVal;
		console.log("select all;")
		var template=angular.element(document.querySelector("#angularTableTemplate."+$scope.id+"ItemBox"));
		var table= angular.element(template[0].querySelector("table"));
		var tbody= angular.element(table[0].querySelector("tbody"));
		var rows=tbody[0].children;

		var j=0;
		for(var i=0;i<rows.length;i++){
			$timeout(function() {
				var checkCol=angular.element(rows[j].querySelector('td'));
				j++;
			
				if(checkCol[0].querySelector('md-checkbox').checked!=$scope.multiSelectVal){
					checkCol.triggerHandler('click');
				}
			}, 0);

		}
	}

	$scope.orderBy = function (column) {
		if ($scope.column_ordering == column) {
			$scope.reverse_col_ord = !$scope.reverse_col_ord;
		} else {
			$scope.column_ordering = column;
			$scope.reverse_col_ord = false;
		}

		if ($scope.localSearch) {
			$scope.internal_column_ordering = $scope.column_ordering;
			$scope.internal_reverse_col_ord = $scope.reverse_col_ord;
		} else {
			$scope.searchFunction({
				searchValue : $scope.prevSearch,
				itemsPerPage : $scope.itemsPerPage,
				columnsSearch : $scope.columnsSearch,
				columnOrdering : $scope.column_ordering,
				reverseOrdering : $scope.reverse_col_ord
			});
		}
	}
}