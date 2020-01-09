<%-- this is part of sbiModule.js --%>
<script>
var sbiM=angular.module('sbiModule');

sbiM.factory('sbiModule_config',function(){
	return {
		protocol: '<%= request.getScheme()%>' ,
		host: '<%= request.getServerName()%>',
	    port: '<%= request.getServerPort()%>',
	    contextName: '/<%= request.getContextPath().startsWith("/")||request.getContextPath().startsWith("\\")?request.getContextPath().substring(1): request.getContextPath()%>',
	    controllerPath: null ,// no cotroller just servlets   
	    docLabel :"<%=docLabel%>",
		 docVersion : "<%=docVersion%>",
		 userId : "<%=userId%>",
		 docAuthor :"<%=docAuthor%>",
		 docName :"<%=docName.replace('\n', ' ')%>",
		 docDescription: "<%=docDescription.replace('\n', ' ')%>",
		 docIsPublic: "<%=docIsPublic%>",
		 docIsVisible: "<%=docIsVisible%>",
		 docPreviewFile: "<%=docPreviewFile%>",
		 docCommunities: "<%=docCommunity%>",
		 docFunctionalities: "<%=docFunctionalities%>",
		 docDatasetLabel: "<%=docDatasetLabel%>",
		 docDatasetName: "<%=docDatasetName%>",
		 visibleDataSet: "<%=visibleDataSet%>",
		 externalBasePath:"<%=request.getParameter(SpagoBIConstants.SBI_CONTEXT)%>/"
	};
});
</script>