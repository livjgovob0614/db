/**
 * @authors Giovanni Luca Ulivo (GiovanniLuca.Ulivo@eng.it)
 * 
 */
var scripts = document.getElementsByTagName("script")
var currentScriptPath = scripts[scripts.length-1].src;

angular.module('expander-box', [ 'ngMaterial'])
.directive('expanderBox',function() {
	return {
		transclude : true,
		template:"<md-toolbar ng-click=\"toggle($event)\">" +
		"<span class=\"md-toolbar-tools\">" +
		"<span id=\"customToolbarContent\"></span>"+
		"{{title}}" +
		"</span>" +
		"<span id=\"customToolbarActionContent\"></span>"+
		"<md-button style=\"position:absolute;right:5px;top:0px;\" class=\"md-icon-button\" aria-label=\"More\"><md-icon class=\"fa fa-chevron-{{expanded?'up':'down'}}\"></md-icon></md-button>"+
		"</md-toolbar><md-content class=\"animate-accordion\" layout-padding  ng-show=\"expanded\"><div ng-transclude style='padding: 0;'></div></md-content> " 
		,
		controller : boxExpanderControllerFunction,
		scope : {
			id : "@",
			color:"@?",
			backgroundColor:"@?",
			toolbarClass:"@?",
			title:"=",
			locals:"=?",
			expanded:"=?"

		},
		link: function(scope, element, attrs, ctrl, transclude) {
			if(!attrs.color){
				scope.color="white"
			}
			
			if(!attrs.backgroundColor){
				scope.backgroundColor="#DFDFDF";
			}
			
			if(attrs.toolbarClass){
				angular.element(element[0].querySelector("md-toolbar")).addClass(attrs.toolbarClass);
			}
			
			if(!attrs.expanded){
				scope.expanded=false;
			}

			angular.element(element[0].querySelector("md-toolbar")).css("background-color",scope.backgroundColor);
			angular.element(element[0].querySelector("md-toolbar")).css("color",scope.color);
			angular.element(element[0].querySelector("md-content")).css("border","1px solid "+scope.backgroundColor);
			element.addClass("md-whiteframe-2dp");
			
		}
	}})

.directive('customToolbar',
		function($compile) {
	return {
		template:'',
		replace:true,
		transclude : true,
		link: function(scope, element, attrs, ctrl, transclude) {
			transclude(scope,function(clone,scope) {
				var contElem=element.parent().parent().parent()[0].querySelector("#customToolbarContent");
				angular.element(contElem).append(clone);
//				$compile(contElem)(scope);
			}); 
		}
	}
})
.directive('customToolbarAction',
		function($compile) {
	return {
		template:'',
		replace:true,
		transclude : true,
		link: function(scope, element, attrs, ctrl, transclude) {
			transclude(scope,function(clone,scope) {
				var contElem=element.parent().parent().parent()[0].querySelector("#customToolbarActionContent");
				angular.element(contElem).append(clone);
//				$compile(contElem)(scope);
			}); 
		}
	}
})

function boxExpanderControllerFunction($scope){
	$scope.toggle=function(event){
		event.stopPropagation();
		$scope.expanded=!$scope.expanded;
	}
}