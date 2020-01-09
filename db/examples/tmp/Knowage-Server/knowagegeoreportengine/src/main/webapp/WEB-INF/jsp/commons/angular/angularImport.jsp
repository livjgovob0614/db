<%-- 
	This files must be use in every 
	SpagoBI${pageContext.request.contextPath}/Knowage page that 
	makes use of AngularJS  
--%>

<script>
/*${disable.console.logging}*/
</script>
<%
	String knowageCoreContext = request.getParameter(SpagoBIConstants.SBI_HOST)+request.getParameter(SpagoBIConstants.SBI_CONTEXT);
%>

<meta name="viewport" content="initial-scale=1, maximum-scale=1, user-scalable=no" />
	<meta name="viewport" content="width=device-width">
	
	<link rel="stylesheet" href="${pageContext.request.contextPath}/fonts/font-awesome-4.4.0/css/font-awesome.min.css">
	
	<!-- angular reference-->
	<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/angular_1.4/angular.js"></script>
	<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/angular_1.4/angular-animate.min.js"></script>
	<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/angular_1.4/angular-aria.min.js"></script>
	<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/angular_1.4/angular-sanitize.min.js"></script>
	
	<!-- angular-material-->
	<script type="text/javascript" src="<%=knowageCoreContext%>/js/lib/angular/angular-material_1.1.0/angular-material.min.js"></script>
	<link rel="stylesheet" href="<%=knowageCoreContext%>/js/lib/angular/angular-material_1.1.0/angular-material.min.css">
	
	
	<!-- context menu -->
	<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/contextmenu/ng-context-menu.min.js"></script>
	
	<!--pagination-->
	<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/pagination/dirPagination.js"></script>
<!-- expanderBox -->
<script type="text/javascript" src="<%=knowageCoreContext%>/js/lib/angular/expander-box/expanderBox.js"></script>

<!-- angular tree -->
	<link rel="stylesheet" 	href="${pageContext.request.contextPath}/js/lib/angular/angular-tree/angular-ui-tree.min.css">
	<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/angular-tree/angular-ui-tree.js"></script>
	
<!-- angular table 
	<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/angular-table/AngularTable.js"></script>
	<script type="text/javascript" src="<%=knowageCoreContext%>/js/src/angular_1.4/tools/commons/angular-table/AngularTable.js"></script>-->
	<script type="text/javascript" src="<%=urlBuilder.getResourcePath(knowageContext, "/js/src/angular_1.4/tools/commons/angular-table/AngularTable.js")%>"></script>

	
<!-- colorpicker -->
<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/color-picker/tinycolor-min.js"></script>
<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/color-picker/tinygradient.min.js"></script>

<!-- toastr -->
<link rel="stylesheet" type="text/css" href="${pageContext.request.contextPath}/css/angular-toastr.css">
<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/toastr/angular-toastr.tpls.js"></script>


<script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/color-picker/angularjs-color-picker.js"></script>
<link rel="stylesheet" href="${pageContext.request.contextPath}/js/lib/angular/color-picker/angularjs-color-picker.min.css">
<link rel="stylesheet" href="${pageContext.request.contextPath}/js/lib/angular/color-picker/mdColorPickerPersonalStyle.css">

<%-- <script type="text/javascript" src="${pageContext.request.contextPath}/js/lib/angular/color-picker/mdColorPicker.js"></script> --%>
<%-- <link rel="stylesheet" href="${pageContext.request.contextPath}/js/lib/angular/color-picker/mdColorPicker.min.css"> --%>

    <script type="text/javascript" src="${pageContext.request.contextPath}/js/commons/Settings.js"></script>
	<script type="text/javascript" src="${pageContext.request.contextPath}/js/src/angular_1.x/module/sbiModule.js"></script> 
	
	<%@include file="/WEB-INF/jsp/commons/angular/sbiModule.jsp"%>