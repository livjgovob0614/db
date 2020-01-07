/* vetisamsApplicationManagementServiceBindingProxy.h
   Generated by gSOAP 2.8.8 from vetisams.h

Copyright(C) 2000-2012, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under one of the following licenses:
1) GPL or 2) Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#ifndef vetisamsApplicationManagementServiceBindingProxy_H
#define vetisamsApplicationManagementServiceBindingProxy_H
#include "vetisamsH.h"

class SOAP_CMAC ApplicationManagementServiceBindingProxy : public soap
{ public:
	const char *soap_endpoint; // Endpoint URL of service 'ApplicationManagementServiceBindingProxy' (change as needed)
	ApplicationManagementServiceBindingProxy(); // Constructor
	ApplicationManagementServiceBindingProxy(const struct soap&); // Construct from another engine state
	ApplicationManagementServiceBindingProxy(const char *url); // Constructor with endpoint URL
	ApplicationManagementServiceBindingProxy(soap_mode iomode); // Constructor with engine input+output mode control
	ApplicationManagementServiceBindingProxy(const char *url, soap_mode iomode); // Constructor with URL and input+output mode control
	ApplicationManagementServiceBindingProxy(soap_mode imode, soap_mode omode); // Constructor with engine input and output mode control
	virtual	~ApplicationManagementServiceBindingProxy(); // Destructor frees deserialized data
	virtual	void ApplicationManagementServiceBindingProxy_init(soap_mode imode, soap_mode omode); // Initializer used by constructors
	virtual	void destroy(); // Delete all deserialized data (uses soap_destroy and soap_end)
	virtual	void reset(); // Delete all deserialized data and reset to default
	virtual	void soap_noheader(); // Disables and removes SOAP Header from message
	virtual	const SOAP_ENV__Header *soap_header(); // Get SOAP Header structure (NULL when absent)
	virtual	const SOAP_ENV__Fault *soap_fault(); // Get SOAP Fault structure (NULL when absent)
	virtual	const char *soap_fault_string(); // Get SOAP Fault string (NULL when absent)
	virtual	const char *soap_fault_detail(); // Get SOAP Fault detail as string (NULL when absent)
	virtual	int soap_close_socket(); // Close connection (normally automatic, except for send_X ops)
	virtual	int soap_force_close_socket(); // Force close connection (can kill a thread blocked on IO)
	virtual	void soap_print_fault(FILE*); // Print fault
#ifndef WITH_LEAN
	/// Print fault to stream
#ifndef WITH_COMPAT
	virtual	void soap_stream_fault(std::ostream&);
#endif

	/// Put fault into buffer
	virtual	char *soap_sprint_fault(char *buf, size_t len);
#endif

	/// Web service operation 'submitApplicationRequest' (returns error code or SOAP_OK)
	virtual	int submitApplicationRequest(_ws__submitApplicationRequest *ws__submitApplicationRequest, _ws__submitApplicationResponse *ws__submitApplicationResponse) { return submitApplicationRequest(NULL, NULL, ws__submitApplicationRequest, ws__submitApplicationResponse); }
	virtual	int submitApplicationRequest(const char *endpoint, const char *soap_action, _ws__submitApplicationRequest *ws__submitApplicationRequest, _ws__submitApplicationResponse *ws__submitApplicationResponse);

	/// Web service operation 'receiveApplicationResult' (returns error code or SOAP_OK)
	virtual	int receiveApplicationResult(_ws__receiveApplicationResultRequest *ws__receiveApplicationResultRequest, _ws__receiveApplicationResultResponse *ws__receiveApplicationResultResponse) { return receiveApplicationResult(NULL, NULL, ws__receiveApplicationResultRequest, ws__receiveApplicationResultResponse); }
	virtual	int receiveApplicationResult(const char *endpoint, const char *soap_action, _ws__receiveApplicationResultRequest *ws__receiveApplicationResultRequest, _ws__receiveApplicationResultResponse *ws__receiveApplicationResultResponse);
};
#endif