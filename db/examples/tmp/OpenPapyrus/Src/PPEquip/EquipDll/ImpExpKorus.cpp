// IMPEXPKORUS.CPP
// ���������� ��� �������/�������� ���������� � xml
//
//
// �������
// ������� �������� �� Papyrus
//		��������:
//		����� �������� ������� ��� ��� ��������������
//		���� �������� ������� ��� ��� ��������������
//		����� ������ - Bill.Code
//		���� ��������� - Bill.Date
//		GLN ���������� - Bill.GLN
//		GLN ���������� - Bill.AgentGLN / Bill.MainGLN
//		GLN ����� �������� - Bill.DlvrAddrCode / Bill.LocCode
//		���� �������� - Bill.DueDate/Bill.Date
//
//		�������������� ��������:
//			GLN ����������� - Bill.Obj2GLN / Bill.AgentGLN / Bill.MainGLN

//		�������� ������:
//		�������� ������ - BRow.Barcode
//		���������� ���������� - BRow.Quantity
//		������� ��������� - BRow.UnitName (����� ��� ��)
//		���� ������ � ��� - BRow.Cost
//		������ ��� - BRow.VatRate
//
//		�������������� ��������:
//			������� ������ - BRow.ArCode
//			������������ ������ - BRow.GoodsName
//
// �������� ��������� � Papyrus
//		����� EnumExpReceipt() ��������� Sdr_DllImpExpReceipt (�� ������ � Papyrus � ��������������� ��� GUID � ������� EDI)
//
//	������
//  �������/�������� ��������� � ������� �� ������:
//		�� Papyrus - ��� �������� (ORDRSP ��� APERAK)
//					 ������, �� ������� ���� ��������� ����������/�������. ���� ��� �� �������, ������� EDI
//						������ �� ������� � ��������� ������ ��� �������
//		�� Dll	   - � ������ ��������� ������������� - ����������� ��� ��������� ��������� Sdr_Bill
//					 � ������ ��������� ������� - ����������� ��������� ���� � Sdr_Bill
//		�� Papyrus - ��������� � ������� ��������� ������, �� ������� �������� �������������/������. ����� ReplyImportObjStatus, ��������� Sdr_DllImpObjStatus
//		���� ����� ����� � Papyrus ����:
//			�� Dll - � ������ ��������� ������������� ����������� ��������� Sdr_BRow
//
//	�������� ��������� � Papyrus
//	��������� ������
//		��������:
//			���� ��������� - Bill.Date
//			����� ������, �� ������� ������ �������������  - Bill.OrderBillNo
//			GLN ���������� - Bill.GLN
//			GLN ���������� - Bill.Obj2GLN
//			GLN ����� �������� - pBill.DlvrAddrCode
//			GLN ����������� - Bill.Obj2GLN
//
//		��������������� ��������:
//			�������������� ���� �������� - Bill.DueDate
//			���� �������� ������ - Bill.OrderDate
//			����� ��������� � ��� - Bill.Amount
//
//		�������� ������:
//			�������� ������ - BRow.Barcode
//			�������������� ���������� - BRow.Quantity
//			������� ��������� - BRow.UnitName
//
//		�������������� ��������:
//			������� ������ � ������� ����������� - BRow.ArCode
//			������������ ������ - BRow.GoodsName
//			���������� � �������� - BRow.UnitPerPack
//			���� ������ � ��� - BRow.Cost
//
//	��������� �������
//		����� �����, �� ������� ������ ������ - Bill.OrderBillNo
//		GLN ����� �������� - Bill.DlvrAddrCode / Bill.LocCode
//		GLN ���������� - Bill.MainGLN
//		GLN ���������� - Bill.GLN
//		���� �������� ������ - Bill.OrderDate
//
//	������� ��������� �� Papyrus
//		������ ������ � Papyrus - 1 - ���� ����� �����, 2 - ��� ������ ������. ����� ReplyImportObjStatus, ��������� Sdr_DllImpObjStatus.
//
//
#include <slib.h>
#include <sxml.h>
#include <ppedi.h>
#include <ppsoapclient.h>
#include <Korus\korusEDIWebServiceSoapProxy.h>

#define EXPORT	extern "C" __declspec (dllexport)
#define THROWERR(expr,val) { if(!(expr)){ SetError(val, ""); goto __scatch; } }
#define THROWERR_STR(expr,val,str) { if(!(expr)){ SetError(val, str); goto __scatch; } }

#define UNIT_NAME_KG			"��"
#define UNIT_NAME_PIECE			"�����"
#define WEBSYS_TECH_GLN			"9999000000001"	// ��������� GLN �������, � �������� ������������ ���������
#define EMPTY_LISTMB_RESP		"<mailbox-response></mailbox-response>"
//
// ����� ��������� xml-������� web-�������
//
#define ELEMENT_NAME_RELATION		"relation"
#define ELEMENT_NAME_PARTNER_ILN	"partner-iln"		// ������������� ��������
//
// �������� ���������-�������������� xml-������� � �������� web-�������
//
#define ELEMENT_CODE_DIRECTION_IN	"IN"		// �������� ��������
#define ELEMENT_CODE_DIRECTION_OUT	"OUT"		// ��������� ��������
#define ELEMENT_CODE_TYPE_ORDRSP	"ORDRSP"	// ������������� ������
#define ELEMENT_CODE_TYPE_APERAK	"APERAK"	// ������ ���������
#define ELEMENT_CODE_TYPE_DESADV	"DESADV"	// ����������� �� ��������

// ����� ��������� ���������
#define ELEMENT_NAME_SG3		"SG3"
#define ELEMENT_NAME_CUX		"CUX"
#define ELEMENT_NAME_C504		"C504"
#define ELEMENT_NAME_E6347		"E6347"
#define ELEMENT_NAME_E6345		"E6345"
#define ELEMENT_NAME_E6343		"E6343"
#define ELEMENT_NAME_E7077		"E7077"
#define ELEMENT_NAME_C273		"C273"
#define ELEMENT_NAME_E7008		"E7008"
#define ELEMENT_NAME_E3039		"E3039"
#define ELEMENT_NAME_SG26		"SG26"
#define ELEMENT_NAME_SG17		"SG17"
#define ELEMENT_NAME_E1229		"E1229"
#define ELEMENT_NAME_C186		"C186"
#define ELEMENT_NAME_E6063		"E6063"
#define ELEMENT_NAME_E6060		"E6060"
#define ELEMENT_NAME_E6411		"E6411"
#define ELEMENT_NAME_E5153		"E5153"
#define ELEMENT_NAME_E5278		"E5278"
#define ELEMENT_NAME_E5283		"E5283"
#define ELEMENT_NAME_C243		"C243"
#define ELEMENT_NAME_E5025		"E5025"
#define ELEMENT_NAME_E5004		"E5004"
//
// �������� ���������-��������������
//
#define ELEMENT_CODE_E2005_17		"17"		// ����/����� �������� (ORDRSP/DESADV)	(�������� ��� �������������)
#define ELEMENT_CODE_E2005_358		"358"		// ����/����� �������� (DESADV)	(�������� ��� �������������)
#define ELEMENT_CODE_E2005_137		"137"		// ����/����� ���������/���������
#define ELEMENT_CODE_E6345_RUB		"RUB"		// �����
#define ELEMENT_CODE_E7077_F		"F"			// ��� ������� �������� ������ - �����
//#define ELEMENT_CODE_E6063_21		"21"		// ������������� ����������� ����������
#define ELEMENT_CODE_E6063_194		"194"		// ������������� ��������� ����������
//#define ELEMENT_CODE_E6063_59		"59"		// ������������� ���������� ������ � ��������
//#define ELEMENT_CODE_E6063_113		"113"		// ������������� ��������������� ���������� (������������� ����� ����� �� ������������)
//#define ELEMENT_CODE_E6063_170		"170"		// ������������� ��������������� ���������� (������������� ����� ����� �� ������������)
//#define ELEMENT_CODE_E6063_12		"12"		// ������������� ������������ ���������� (DESADV)
#define ELEMENT_CODE_E6411_PCE		"PCE"		// ������� ��������� - ��������� ��������
#define ELEMENT_CODE_E6411_KGM		"KGM"		// ������� ��������� - ����������
#define ELEMENT_CODE_E5025_XB5		"XB5"		// ������������� ���� ������ � ��� (DESADV)
#define ELEMENT_CODE_E4451_AAO		"AAO"		// ������������� ���� �������� �������
#define ELEMENT_CODE_E2005_171		"171"		// ������������� ���� ��������� (� APERAK)
#define ELEMENT_CODE_E1153_CT		"CT"		// ������������� ������ �������� �� ��������
//
// ���� ������ � ���������
//
#define IEERR_SYMBNOTFOUND			1			// ������ �� ������ (������ ���� ������� �������/��������)
#define IEERR_NODATA				2			// ��� ������
#define IEERR_NOSESS				3			// ������ � ����� ������� �� ����������
#define IEERR_ONLYBILLS				4			// ������ DLL ����� �������� ������ � �����������
#define IEERR_NOOBJID				5			// ������� � ����� ��������������� ���
#define IEERR_IMPEXPCLSNOTINTD		6			// ������ ��� �������/�������� �� ���������������
#define IEERR_WEBSERV�ERR			7			// ������ ���-�������. � ���� ������ ��������� �������� ���� WebSrevcErr
#define IEERR_SOAP					8			// ������ soap-���������. � ���� ������ �������� ������ ���������� � SoapError
#define IEERR_IMPFILENOTFOUND		9			// ���� ������� �� ������: %s
#define IEERR_INVMESSAGEYTYPE		10			// �������� ��� ���������. ��������� %S
#define IEERR_NOCFGFORGLN			11			// ��� ������ ���� �������� ��� �������� ������������ ��� ������������ � GLN %s
#define IEERR_NULLWRIEXMLPTR		12			// ������� xmlWriter
#define IEERR_NULLREADXMLPTR		13			// ������� xmlReader
#define IEERR_XMLWRITE				14			// ������ ������ � xml-����
#define IEERR_XMLREAD				15			// ������ ������ �� xml-�����
#define IEERR_TOOMUCHGOODS			16			// ����� �������, ���������� � ����� �������, ������ ��������� �� ����������
#define IEERR_MSGSYMBNOTFOUND		17			// ������ �� ������ (������ ���� �������� �������/��������)
#define IEERR_ONLYIMPMSGTYPES		18			// �������� ������� �������� ������ � ������ ������ ORDRSP, DESADV � APERAK
#define IEERR_NOCONFIG				19			// �� ������� ������������ ��� ������� ���� ��������
#define IEERR_NOEXPPATH				20			// �� ���������� ���������� ��� ����� ��������
#define IEERR_ONLYEXPMSGTYPES		21			// �������� �������� �������� ������ � ������ ������ ORDER � RECADV
#define IEERR_NOMEM                 22          // �� ���������� ������
//
// ���� ������ ���-�������
//
#define IEWEBERR_AUTH				1			// ������ ��������������
#define IEWEBERR_CORRELATION		2			// ������ �� �����������
#define IEWEBERR_EXTERNAL			3			// ������� ������
#define IEWEBERR_SERVER				4			// ��������� ������ �������
#define IEWEBERR_TIMELIMIT			5			// �������� ������� �� ���������� ������
#define IEWEBERR_WEBERR				6			// ������ Web
#define IEWEBERR_PARAMS				7			// ������������ ���������
//
// ���� ������ ��������� ��� ���-�����
//
#define SYSLOG_INITEXPORT				"INITEXPORT"
#define SYSLOG_SETEXPORTOBJ				"SETEXPORTOBJ"
#define SYSLOG_INITEXPORTOBJITER		"INITEXPORTOBJITER"
#define SYSLOG_NEXTEXPORTOBJITER		"NEXTEXPORTOBJITER"
#define SYSLOG_ENUMEXPRECEIPT			"ENUMEXPRECEIPT"
#define SYSLOG_INITIMPORT				"INITIMPORT"
#define SYSLOG_GETIMPORTOBJ				"GETIMPORTOBJ"
#define SYSLOG_INITIMPORTOBJITER		"INITIMPORTOBJITER"
#define SYSLOG_NEXTIMPORTOBJITER		"NEXTIMPORTOBJITER"
#define SYSLOG_FINISHIMPEXP				"FINISHIMPEXP"
#define SYSLOG_REPLYIMPORTOBJSTATUS		"REPLYIMPORTOBJSTATUS"
#define SYSLOG_RELATIONSHIPS			"in Relationships()"
#define SYSLOG_PARSERELATIONRESPONSE	"in ParseRlnResponse()"
#define SYSLOG_ORDERHEADER				"in OrderHeader()"
#define SYSLOG_RECADVHEADER				"in RecadvHeader()"
#define SYSLOG_DOCPARTIES				"in DocPartiesAndCurrency()"
//#define SYSLOG_BEGINORDER				"in BeginOrder()"
//#define SYSLOG_GOODSORDER				"in GoodsOrder()"
#define SYSLOG_GOODSLINES				"in GoodsLines()"
//#define SYSLOG_ENDORDER					"in EndOrder()"
#define SYSLOG_ENDDOC					"in EndDoc()"
#define SYSLOG_SENDDOC					"in SendDoc()"
#define SYSLOG_RECEIVEDOC				"in ReceiveDoc()"
#define SYSLOG_PARSELISTMBRESP			"in ParseListMBResp()"
#define SYSLOG_PARSEFORDOCDATA			"in ParseForDocData()"
#define SYSLOG_PARSEFORGOODDATA			"in ParseForGoodData()"
#define SYSLOG_SETNEWSTATUS				"in SetNewStatus()"
#define SYSLOG_LISTMESSAGEBOX			"in ListMessageBox()"
#define SYSLOG_PARSEAPERAKRESP			"in ParseAperakResp()"

#define LOG_NOINCOMDOC					"��� �������� ���������"
#define LOG_NEWSTATERR					"���� ��� ����������� ������� NEW ��� ����������:"

class ExportCls;
class ImportCls;

int    ErrorCode = 0;
int    WebServcErrorCode = 0;
SString StrError = "";
SString LogName = "";
SString SysLogName = "";
static ExportCls * P_ExportCls = 0;
static ImportCls * P_ImportCls = 0;

int GetObjTypeBySymb(const char * pSymb, uint & rType);
int Relationships();
void ProcessError(EDIWebServiceSoapProxy & rProxy);

int SetError(int errCode, const char * pStr = "") 
{ 
	ErrorCode = errCode;
	StrError = pStr; 
	return 1; 
}

int SetWebServcError(int errCode) { WebServcErrorCode = errCode; return 1; }
void LogMessage(const char * pMsg);
void SysLogMessage(const char * pMsg);
// ������� �� ������ ������ ������ �������
// ��� ����� 4607806659997EC_1, � ������ ���� 4607806659997��
void FormatLoginToLogin(const char * login, SString & rStr);
//
// ��������� ��� ��������� ������ � �����������. ����������� ��� ������ Relationships()
//
struct StRlnConfig {
	void Clear()
	{
		EdiDocType = 0;
		SuppGLN = 0;
		Direction = 0;
		//DocType = 0;
		DocVersion = 0;
		DocStandard = 0;
		DocTest = 0;
	}
	int    EdiDocType;
	SString SuppGLN;		// GLN ����������, � �������� ��������� ������ ���������
	SString Direction;		// ��������� ��� �������� ��������
	SString DocVersion;		// ������ ������������
	SString DocStandard;	// �������� ���������
	SString DocTest;		// ������ ���������
};

struct ErrMessage {
	uint   Id;
	const char * P_Msg;
};

struct WebServcErrMessage {
	uint   Id;
	const  char * P_Msg;
};

struct ObjectTypeSymbols {
	char * P_Symb;
	uint Type;
};

struct MessageTypeSymbols {
	char * P_Symb;
	uint Type;
};

enum ObjectType {
	objGood = 1,
	objBill,
	objCheck,
	objCashSess,
	objPriceList,
	objLot,
	objPhoneList,
	objCliBnkData
	// � �.�.
};

enum ImpObjStatus {
	docStatNoSuchDoc = 0,
	docStatIsSuchDoc
};

ObjectTypeSymbols Symbols[] = {
	{"GOODS",       objGood},
	{"BILLS",		objBill},
	{"CHECKS",		objCheck},
	{"CSESS",		objCashSess},
	{"PRICELIST",	objPriceList},
	{"LOTS",		objLot},
	{"PHONELIST",	objPhoneList},
	{"CLIBANKDATA",	objCliBnkData}
};

ErrMessage ErrMsg[] = {
	{IEERR_SYMBNOTFOUND,		"������ �� ������"},
	{IEERR_NODATA,				"������ �� ��������"},
	{IEERR_NOSESS,				"������ � ����� ������� ���"},
	{IEERR_ONLYBILLS,			"Dll ����� �������� ������ � �����������"},
	{IEERR_NOOBJID,				"������� � ����� ��������������� ���"},
	{IEERR_IMPEXPCLSNOTINTD,	"������ ��� �������/�������� �� ���������������"},
	{IEERR_WEBSERV�ERR,			"������ Web-�������: "},
	{IEERR_SOAP,                "������ SOAP: "},
	{IEERR_IMPFILENOTFOUND,		"���� ������� �� ������: "},
	{IEERR_INVMESSAGEYTYPE,		"�������� ��� ���������. ��������� "},
	{IEERR_NOCFGFORGLN,			"��� ������ ���� �������� ��� �������� ������������ ��� ������������ � GLN "},
	{IEERR_NULLWRIEXMLPTR,		"������� xmlWriter"},
	{IEERR_NULLREADXMLPTR,		"������� xmlReader"},
	{IEERR_XMLWRITE,			"������ ������ � xml-����"},
	{IEERR_XMLREAD,				"������ ������ �� xml-�����"},
	{IEERR_TOOMUCHGOODS,		"����� �������, ���������� � ����� �������, ������ ��������� �� ����������"},
	{IEERR_MSGSYMBNOTFOUND,		"������ �� ������ (������ ���� �������� �������/��������)"},
	{IEERR_ONLYIMPMSGTYPES,		"�������� ������� �������� ������ � ������ ������ ORDRSP, DESADV � APERAK"},
	{IEERR_NOCONFIG,			"�� ������� ������������ ��� ������� ���� ��������"},
	{IEERR_NOEXPPATH,			"�� ��������� ������� ��� ����� ��������"},
	{IEERR_ONLYEXPMSGTYPES,		"�������� �������� �������� ������ � ������ ������ ORDER � RECADV"},
	{IEERR_NOMEM,          		"�� ���������� ������"},
};

WebServcErrMessage WebServcErrMsg[] = {
	{IEWEBERR_AUTH,			"������ ��������������"},
	{IEWEBERR_CORRELATION,	"������ �� �����������"},
	{IEWEBERR_EXTERNAL,		"������� ������"},
	{IEWEBERR_SERVER,		"���������� ������ �������"},
	{IEWEBERR_TIMELIMIT,	"�������� ������� �� ���������� ������"},
	{IEWEBERR_WEBERR,		"������ Web"},
	{IEWEBERR_PARAMS,		"������������ ���������"}
};

void LogMessage(const char * pMsg)
{
	SFile file(LogName, SFile::mAppend);
	if(file.IsValid()) {
		SString str;
        file.WriteLine(str.Z().Cat(getcurdatetime_(), DATF_GERMAN|DATF_CENTURY, TIMF_HMS).Tab().Cat(pMsg).CR());
	}
}

void SysLogMessage(const char * pMsg)
{
	SFile  file(SysLogName, SFile::mAppend);
	if(file.IsValid()) {
		SString str;
        file.WriteLine(str.Z().Cat(getcurdatetime_(), DATF_GERMAN|DATF_CENTURY, TIMF_HMS).Tab().Cat(pMsg).CR());
	}
}

void GetErrorMsg(SString & rMsg)
{
	SString str = "";
	for(size_t i = 0; i < SIZEOFARRAY(ErrMsg); i++) {
		if(ErrMsg[i].Id == ErrorCode) {
			str.Cat(ErrMsg[i].P_Msg);
			break;
		}
	}
	if(ErrorCode == IEERR_WEBSERV�ERR) {
		for(size_t i = 0; i < SIZEOFARRAY(ErrMsg); i++) {
			if(WebServcErrMsg[i].Id == WebServcErrorCode) {
				str.Cat(WebServcErrMsg[i].P_Msg);
				break;
			}
		}
	}
	else if(oneof4(ErrorCode, IEERR_SOAP, IEERR_IMPFILENOTFOUND, IEERR_INVMESSAGEYTYPE, IEERR_NOCFGFORGLN))
		str.Cat(StrError);
	rMsg = str;
}
//
// Returns:
//		0 - ������ �� ������
//		1 - ������ ������
//
int GetObjTypeBySymb(const char * pSymb, uint & rType)
{
	for(size_t i = 0; i < SIZEOFARRAY(Symbols); i++) {
		if(strcmp(Symbols[i].P_Symb, pSymb) == 0) {
			rType = Symbols[i].Type;
			return 1;
		}
	}
	return 0;
}
//
//
//
MessageTypeSymbols MsgSymbols[] = {
	{"ORDERS", PPEDIOP_ORDER},
	{"ORDRSP", PPEDIOP_ORDERRSP},
	{"APERAK", PPEDIOP_APERAK},
	{"DESADV", PPEDIOP_DESADV},
	{"RECADV", PPEDIOP_RECADV},

	{"ORDER",  PPEDIOP_ORDER} // �������������� �������� - ������ ��������� ����� {"ORDERS", PPEDIOP_ORDER}
};
//
// Returns:
//		0 - ������ �� ������
//		1 - ������ ������
//
int GetMsgTypeBySymb(const char * pSymb, int & rType)
{
	for(size_t i = 0; i < SIZEOFARRAY(MsgSymbols); i++) {
		if(_stricmp(MsgSymbols[i].P_Symb, pSymb) == 0) {
			rType = MsgSymbols[i].Type;
			return 1;
		}
	}
	return 0;
}

int GetMsgTypeSymb(uint msgType, SString & rSymb)
{
	rSymb.Z();
	for(size_t i = 0; i < SIZEOFARRAY(MsgSymbols); i++) {
		if(MsgSymbols[i].Type == msgType) {
			rSymb = MsgSymbols[i].P_Symb;
			return 1;
		}
	}
	return 0;
}
//
// ������� �� ������ ������ ������ �������
// ��� ����� 4607806659997EC_1, � ������ ���� 4607806659997��
//
void FormatLoginToLogin(const char * login, SString & rStr)
{
	uint   exit_while = 0;
	char   low_strip = '_';
	rStr.Z();
	if(login) {
		while(!exit_while) {
			if((*login == 0) || (*login == low_strip))
				exit_while = 1;
			else
				rStr.CatChar(*login);
			login++;
		}
	}
}

class Iterator {
public:
	void Init()
	{
		Count = 0;
	}
	uint GetCount() const
	{
		return Count;
	}
	void Next()
	{
		Count++;
	}
private:
	uint   Count;
};
//
// �����, �� �������� ����������� ImportCls � ExportCls. �������� ����� ������, ��� ���� �������
//
class ImportExportCls {
public:
	ImportExportCls();
	~ImportExportCls();
	void   CleanHeader();
	int    Relationships();
	int    ParseRlnResponse(const char * pResponse);

	Sdr_ImpExpHeader Header;
	TSArray <StRlnConfig> RlnCfgList; // ������ ������������ ������ ��� ���������� ���, � �������� �������� ������ ������
private:
};

ImportExportCls::ImportExportCls()
{
	RlnCfgList.clear();
}

ImportExportCls::~ImportExportCls()
{
	RlnCfgList.freeAll();
}

void ImportExportCls::CleanHeader()
{
	Header.CurDate = ZERODATE;
	Header.CurTime = ZEROTIME;
	Header.PeriodLow = ZERODATE;
	Header.PeriodUpp = ZEROTIME;
	memzero(Header.SrcSystemName, sizeof(Header.SrcSystemName));
	memzero(Header.SrcSystemVer, sizeof(Header.SrcSystemVer));
	memzero(Header.EdiLogin, sizeof(Header.EdiLogin));
	memzero(Header.EdiPassword, sizeof(Header.EdiPassword));
}

int ImportExportCls::Relationships()
{
	int    ok = 1;
	int    r = 0;
	SString login;
	_ns1__Relationships param;
	_ns1__RelationshipsResponse resp;
	EDIWebServiceSoapProxy proxy(SOAP_XML_INDENT);
	gSoapClientInit(&proxy, 0, 0);
	FormatLoginToLogin(Header.EdiLogin, login.Z()); // �� ������������
	param.Name = (char *)login.cptr(); // @badcast
	//param.Name = Header.EdiLogin;			// �� ������������
	param.Password = Header.EdiPassword;	// ������
	param.Timeout = 5000;					// ������� �� ���������� ������ ������ (��)
	if((r = proxy.Relationships(&param, &resp)) == SOAP_OK) {
		if(atoi(resp.RelationshipsResult->Res) == 0) {
			//
			// ��������� ���������� ����� � ��������� ����� RlnCfgList
			//
			ParseRlnResponse(resp.RelationshipsResult->Cnt);
			ok = 1;
		}
		else {
			SetError(IEERR_WEBSERV�ERR);
			SetWebServcError(atoi((const char *)resp.RelationshipsResult->Res));
			ok = 0;
		}
	}
	else {
		ProcessError(proxy);
		ok = 0;
	}
	if(!ok)
		SysLogMessage(SYSLOG_RELATIONSHIPS);
	return ok;
}

int ImportExportCls::ParseRlnResponse(const char * pResp)
{
	int    ok = 1;
	SString str;
	StRlnConfig * p_rln_cfg = 0;
	xmlTextReader * p_xml_ptr;
	xmlParserInputBuffer * p_input = 0;
	xmlNode * p_node;
	RlnCfgList.freeAll();
	if(pResp) {
		p_input = xmlParserInputBufferCreateMem(pResp, sstrlen(pResp), XML_CHAR_ENCODING_NONE);
		THROWERR((p_xml_ptr = xmlNewTextReader(p_input, NULL)), IEERR_NULLREADXMLPTR);
		//
		// xmlTextReaderSetup �����, ������ ��� ����� ������������ ���������� ��������� (������ �����) ��� �������� p_node->children
		// �������, ���� ��� �� ���. � ����������, ��� ���������� ��������� �� �����������.
		//
		THROWERR(xmlTextReaderSetup(p_xml_ptr, p_input, NULL, NULL, XML_PARSE_SAX1) == 0, IEERR_NULLREADXMLPTR);
		while(xmlTextReaderRead(p_xml_ptr)) {
			p_node = xmlTextReaderCurrentNode(p_xml_ptr);
			if(p_node && p_node->children) {
				const char * p_nn = (const char *)p_node->name;
				if(strcmp(p_nn, ELEMENT_NAME_RELATION) == 0) {
					p_rln_cfg = new StRlnConfig;
					p_rln_cfg->Clear();
				}
				else if(p_rln_cfg) {
					if(strcmp(p_nn, ELEMENT_NAME_PARTNER_ILN) == 0) {
						p_rln_cfg->SuppGLN.Set(p_node->children->content);
					}
					else if(strcmp(p_nn, "direction") == 0) { // �������� ��� ��������� ��������
						p_rln_cfg->Direction.Set(p_node->children->content);
					}
					else if(strcmp(p_nn, "document-type") == 0) { // ��� ���������
						GetMsgTypeBySymb((const char *)p_node->children->content, p_rln_cfg->EdiDocType);
					}
					else if(strcmp(p_nn, "document-version") == 0) { // ������ ������������
						p_rln_cfg->DocVersion.Set(p_node->children->content);
					}
					else if(strcmp(p_nn, "document-standard") == 0) { // �������� ���������
						p_rln_cfg->DocStandard.Set(p_node->children->content);
					}
					else if(strcmp(p_nn, "document-test") == 0) { // ������ ���������
						p_rln_cfg->DocTest.Set(p_node->children->content);
					}
				}
			}
			else if(SXml::IsName(p_node, ELEMENT_NAME_RELATION) && !p_node->children && p_rln_cfg) {
				//
				// ���� ����������� ��� <relation>
				// ������������� ���������, ����� p_node->name != 0, ��� � ������ ���������� ���� ������ ����
				// <description><bla-bla-bla></description>
				//
				RlnCfgList.insert(p_rln_cfg);
				p_rln_cfg = 0;
			}
		}
	}
	CATCH
		SysLogMessage(SYSLOG_PARSERELATIONRESPONSE);
		GetErrorMsg(str.Z());
		SysLogMessage(str);
		ok = 0;
	ENDCATCH;
	xmlFreeParserInputBuffer(p_input);
	//if(p_xml_ptr)
	//	xmlFreeTextReader(p_xml_ptr);

	// @vmiller ��� �������� {
	/*if(file.IsValid()) {
		for(int rel_pos = 0; rel_pos < RlnCfgList.getCount(); rel_pos++) {
			file_str.Z().Cat("Partner ILN = ").Cat(RlnCfgList.at(rel_pos).SuppGLN).CR();
			file.WriteLine(file_str);
			file_str.Z().Cat("Direction = ").Cat(RlnCfgList.at(rel_pos).Direction).CR();
			file.WriteLine(file_str);
			file_str.Z().Cat("DocType = ").Cat(RlnCfgList.at(rel_pos).DocType).CR();
			file.WriteLine(file_str);
		}
	}*/
	// }
	return ok;
}
//
//
//
EXPORT int FinishImpExp();

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch(dwReason) {
		case DLL_PROCESS_ATTACH:
			{
				SString product_name;
				(product_name = "Papyrus-ImpExpKorus");
				SLS.Init(product_name/*, (HINSTANCE)hModule*/);
			}
			break;
#ifdef _MT
		case DLL_THREAD_ATTACH:
			SLS.InitThread();
			break;
		case DLL_THREAD_DETACH:
			SLS.ReleaseThread();
			break;
#endif
		case DLL_PROCESS_DETACH:
			FinishImpExp(); // @vmiller
			break;
	}
	return TRUE;
}
//
// �������
//
class ExportCls : public ImportExportCls {
public:
	ExportCls();
	~ExportCls();
	void   CreateFileName(uint num)
	{
		ExpFileName.Z().Cat(PathStruct.Drv).CatChar(':').Cat(PathStruct.Dir).Cat(PathStruct.Nam).Cat(num).Dot().Cat(PathStruct.Ext);
	}
	int    OrderHeader();
	int    RecadvHeader();
	int    DocPartiesAndCurrency();
	int    GoodsLines(Sdr_BRow * pBRow);
	int    EndDoc();
	int    SendDoc();

	uint   Id;                  // ��, ������� Papyrus ����� ������������ ��� �� ������ ��������
	uint   ObjId;               // �� ��������������� ������� (��������, ��� ������ ���������� ������� ��������� ������������� ���� ��)
	uint   ObjType;             // ��� �������������� ��������
	uint   Inited;
	uint   SegNum;              // ���������� ��������� (���������) ���������
	uint   ReadReceiptNum;		// ���������� ����� ���������, ����������� �� ReceiptList
	int    MessageType;			// ��� ��������: ORDER, RECADV, DESADV
	double BillSumWithoutVat;	// ����� ��������� ��� ��� (�������� ������� ��������������, ��� � Sdr_Bill ���� ������� �� ����������)
	SString ExpFileName;
	SString LogFileName;		// ��� ��, ��� � ExpFileName
	SString TTN;				// ��� ��� RECADV
	xmlTextWriter * P_XmlWriter;
	Iterator Itr;
	SPathStruc PathStruct;
	Sdr_Bill Bill;
	TSArray <Sdr_DllImpExpReceipt> ReceiptList; // ������ ��������� �� ������������ ���������� // @todo TSArray-->TSCollection
};

ExportCls::ExportCls()
{
	Id = 0;
	ObjId = 0;
	ObjType = 0;
	Inited = 0;
	SegNum = 0;
	ReadReceiptNum = 0;
	BillSumWithoutVat = 0.0;
	ExpFileName = 0;
	LogName = 0;
	TTN = 0;
	P_XmlWriter = 0;
	ErrorCode = 0;
	WebServcErrorCode = 0;
	MessageType = 0;
}

ExportCls::~ExportCls()
{
	P_XmlWriter = 0;
}
//
// ������ � ��������� ���������� � ���������
//
// ��������� ������
//
int ExportCls::OrderHeader()
{
	int    ok = 1;
	LDATE  date;
	SString str, fmt;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	BillSumWithoutVat = 0.0;
	SegNum = 0;
	xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"ORDERS");
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"UNH");
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0062"); // �� ���������
				xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(ObjId).ucptr());
			xmlTextWriterEndElement(P_XmlWriter); //E0062
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"S009");
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0065"); // ��� ���������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"ORDERS");
				xmlTextWriterEndElement(P_XmlWriter); //E0065
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0052"); // ������ ���������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"D");
				xmlTextWriterEndElement(P_XmlWriter); //E0052
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0054"); // ������ �������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"01B");
				xmlTextWriterEndElement(P_XmlWriter); //E0054
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0051"); // ��� ������� �����������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"UN");
				xmlTextWriterEndElement(P_XmlWriter); //E0051
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0057"); // ���, ����������� ������� ������������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"EAN010");
				xmlTextWriterEndElement(P_XmlWriter); //E0057
			xmlTextWriterEndElement(P_XmlWriter); //S009
		xmlTextWriterEndElement(P_XmlWriter); //UNH
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"BGM");
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C002"); // ��� ���������/���������
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E1001"); // ��� ���������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"220"); // ����� (���� ������������� ������)
				xmlTextWriterEndElement(P_XmlWriter); //E1001
			xmlTextWriterEndElement(P_XmlWriter); //�002
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C106"); // ������������� ���������/���������
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E1004"); // ����� ������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.Code); // ������ ���� �������� 17 ��������
				xmlTextWriterEndElement(P_XmlWriter); //E1004
			xmlTextWriterEndElement(P_XmlWriter); //�106
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E1225"); // ��� ������� ���������
				xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"9"); // �������� (���� ��� �����, ��������������� �����, ������ � �.�.)
			xmlTextWriterEndElement(P_XmlWriter); //E1225
		xmlTextWriterEndElement(P_XmlWriter); //BGM
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"DTM"); // ���� ���������
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C507");
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2005"); // ������������ ������� ����-�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E2005_137); // ����/����� ���������/���������
				xmlTextWriterEndElement(P_XmlWriter); //E2005
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2380"); // ���� ��� �����, ��� ������
					xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(Bill.Date, DATF_YMD|DATF_CENTURY|DATF_NODIV).ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E2380
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2379"); // ������ ����/�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"102"); // CCYYMMDD
				xmlTextWriterEndElement(P_XmlWriter); //E2379
			xmlTextWriterEndElement(P_XmlWriter); //C507
		xmlTextWriterEndElement(P_XmlWriter); //DTM
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"DTM"); // ���� ��������
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C507");
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2005"); // ������������ ������� ����-�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"2"); // ����/����� ��������
				xmlTextWriterEndElement(P_XmlWriter); //E2005
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2380"); // ���� ��� �����, ��� ������
					date = (Bill.DueDate != ZERODATE) ? Bill.DueDate : Bill.Date;
					xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(date, DATF_YMD|DATF_CENTURY|DATF_NODIV).ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E2380
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2379"); // ������ ����/�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"102"); // CCYYMMDD
				xmlTextWriterEndElement(P_XmlWriter); //E2379
			xmlTextWriterEndElement(P_XmlWriter); //C507
		xmlTextWriterEndElement(P_XmlWriter); //DTM
	CATCH
		SysLogMessage(SYSLOG_ORDERHEADER);
		ok = 0;
	ENDCATCH;
	return ok;
}
//
// ��������� ����������� � �������
//
int ExportCls::RecadvHeader()
{
	int    ok = 1;
	SString str, fmt;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	BillSumWithoutVat = 0.0;
	SegNum = 0;
	xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"RECADV");
	{
		{
			SXml::WNode n_unh(P_XmlWriter, "UNH");
			{
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0062"); // �� ���������
					xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(ObjId).ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E0062
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"S009");
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0065"); // ��� ���������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"RECADV");
					xmlTextWriterEndElement(P_XmlWriter); //E0065
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0052"); // ������ ���������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"D");
					xmlTextWriterEndElement(P_XmlWriter); //E0052
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0054"); // ������ �������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"01B");
					xmlTextWriterEndElement(P_XmlWriter); //E0054
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0051"); // ��� ������� �����������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"UN");
					xmlTextWriterEndElement(P_XmlWriter); //E0051
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0057"); // ���, ����������� ������� ������������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"EAN005");
					xmlTextWriterEndElement(P_XmlWriter); //E0057
				xmlTextWriterEndElement(P_XmlWriter); //S009
			}
		}
		{
			SXml::WNode n_bgm(P_XmlWriter, "BGM");
			{
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C002"); // ��� ���������/���������
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E1001"); // ��� ���������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"632"); // RECADV (���� ������������� ������)
					xmlTextWriterEndElement(P_XmlWriter); //E1001
				xmlTextWriterEndElement(P_XmlWriter); //�002
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C106"); // ������������� ���������/���������
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E1004"); // ����� ���������
						xmlTextWriterWriteString(P_XmlWriter, (str = Bill.Code).ToUtf8().ucptr()); // ������ ���� �������� 17 ��������
					xmlTextWriterEndElement(P_XmlWriter); //E1004
				xmlTextWriterEndElement(P_XmlWriter); //�106
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E1225"); // ��� ������� ���������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"9"); // �������� (���� ��� �����, ��������������� �����, ������ � �.�.)
				xmlTextWriterEndElement(P_XmlWriter); //E1225
			}
		}
		{
			SXml::WNode n_dtm(P_XmlWriter, "DTM"); // ���� ���������
			{
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C507");
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2005"); // ������������ ������� ����-�������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E2005_137); // ����/����� ���������/���������
					xmlTextWriterEndElement(P_XmlWriter); //E2005
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2380"); // ���� ��� �����, ��� ������
						xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(Bill.Date, DATF_YMD|DATF_CENTURY|DATF_NODIV).ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E2380
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2379"); // ������ ����/�������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"102"); // CCYYMMDD
					xmlTextWriterEndElement(P_XmlWriter); //E2379
				xmlTextWriterEndElement(P_XmlWriter); //C507
			}
		}
		{
			SXml::WNode n_dtm(P_XmlWriter, "DTM"); // ���� �������
			{
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C507");
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2005"); // ������������ ������� ����-�������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"50"); // ����/����� �������
					xmlTextWriterEndElement(P_XmlWriter); //E2005
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2380"); // ���� ��� �����, ��� ������
						xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(Bill.Date, DATF_YMD|DATF_CENTURY|DATF_NODIV).ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E2380
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2379"); // ������ ����/�������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"102"); // CCYYMMDD
					xmlTextWriterEndElement(P_XmlWriter); //E2379
				xmlTextWriterEndElement(P_XmlWriter); //C507
			}
		}
		{
			SXml::WNode n_sg1(P_XmlWriter, "SG1"); // ����� ����������� �� �������� (���)
			{
				SXml::WNode n_ref(P_XmlWriter, "RFF");
				{
					SXml::WNode n_s(P_XmlWriter, "C506");
					n_s.PutInner("E1153", "AAK"); // ������������� ����������� �� ��������
					n_s.PutInner("E1154", (str = Bill.DesadvBillNo).Transf(CTRANSF_INNER_TO_UTF8)); // ����� ����������� �� ��������
				}
			}
		}
		if(!isempty(Bill.OrderBillNo)) {
			SXml::WNode n_sg1(P_XmlWriter, "SG1"); // ����� ������
			{
				SXml::WNode n_ref(P_XmlWriter, "RFF");
				{
					SXml::WNode n_s(P_XmlWriter, "C506");
					n_s.PutInner("E1153", "ON"); // ������������� ��������� ������
					n_s.PutInner("E1154", (str = Bill.OrderBillNo).ToUtf8()); // ����� ������
				}
			}
		}
	}
	CATCH
		SysLogMessage(SYSLOG_RECADVHEADER);
		ok = 0;
	ENDCATCH;
	return ok;
}
//
// ��������� ������ � ������
//
int ExportCls::DocPartiesAndCurrency()
{
	int    ok = 1;
	SString str, fmt;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	BillSumWithoutVat = 0.0;
	SegNum = 0;
	const char * p_sg = 0;
	if(MessageType == PPEDIOP_ORDER)
		p_sg = "SG2";
	else if(MessageType == PPEDIOP_RECADV)
		p_sg = "SG4";
	if(p_sg) {
		{
			SXml::WNode n_sg2(P_XmlWriter, p_sg); // GLN ����������
			{
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"NAD"); // ������������ � �����
					SegNum++;
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E3035"); // ������������ �������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"SU"); // ���������
					xmlTextWriterEndElement(P_XmlWriter); //E3035
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C082"); // ������ �������
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3039); // GLN �������
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.GLN);
						xmlTextWriterEndElement(P_XmlWriter); //E3039
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E3055"); // ��� ������� �����������
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"9"); // EAN (������������� ���������� �������� ���������)
						xmlTextWriterEndElement(P_XmlWriter); //E3055
					xmlTextWriterEndElement(P_XmlWriter); //C082
				xmlTextWriterEndElement(P_XmlWriter); //NAD
				if(MessageType == PPEDIOP_ORDER && !isempty(Bill.CntractCode)) {
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG3); // ����� �������� �� ��������
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"RFF");
							xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C506");
								xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E1153"); // �������������
									xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E1153_CT); // ����� ��������
								xmlTextWriterEndElement(P_XmlWriter); //E1153
								xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E1154");
									str.Z().Cat(Bill.CntractCode).Transf(CTRANSF_INNER_TO_UTF8);
									xmlTextWriterWriteString(P_XmlWriter, str.ucptr()); // ��� ����� ��������
								xmlTextWriterEndElement(P_XmlWriter); //E1154
							xmlTextWriterEndElement(P_XmlWriter); //C506
						xmlTextWriterEndElement(P_XmlWriter); //RFF
					xmlTextWriterEndElement(P_XmlWriter); //SG3
				}
			}
		}
		{
			SXml::WNode n_sg2(P_XmlWriter, p_sg); // GLN ����������
			{
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"NAD"); // ������������ � �����
					SegNum++;
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E3035"); // ������������ �������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"BY"); // ����������
					xmlTextWriterEndElement(P_XmlWriter); //E3035
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C082"); // ������ �������
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3039); // GLN �������
							// ���� GLN ������ �����, �� ����� GLN ������� �����������
							if(!isempty(Bill.AgentGLN))
								xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.AgentGLN);
							else
								xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.MainGLN);
						xmlTextWriterEndElement(P_XmlWriter); //E3039
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E3055"); // ��� ������� �����������
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"9"); // EAN (������������� ���������� �������� ���������)
						xmlTextWriterEndElement(P_XmlWriter); //E3055
					xmlTextWriterEndElement(P_XmlWriter); //C082
				xmlTextWriterEndElement(P_XmlWriter); //NAD
			}
		}
		{
			SXml::WNode n_sg2(P_XmlWriter, p_sg); // GLN ����� ��������
			{
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"NAD"); // ������������ � �����
					SegNum++;
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E3035"); // ������������ �������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"DP"); // �������� ����� ��������
					xmlTextWriterEndElement(P_XmlWriter); //E3035
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C082"); // ������ �������
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3039); // GLN �������
							if(!isempty(Bill.DlvrAddrCode))
								str.Z().Cat(Bill.DlvrAddrCode);
							else
								str.Z().Cat(Bill.LocCode);
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); //E3039
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E3055"); // ��� ������� �����������
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"9"); // EAN (������������� ���������� �������� ���������)
						xmlTextWriterEndElement(P_XmlWriter); //E3055
					xmlTextWriterEndElement(P_XmlWriter); //C082
				xmlTextWriterEndElement(P_XmlWriter); //NAD
			}
		}
	}
	// ��������� ���� ������ ��� � �������� RECADV
	if(MessageType == PPEDIOP_ORDER) {
		// �������������� ��������
		if(!isempty(Bill.Obj2GLN) || !isempty(Bill.AgentGLN) || !isempty(Bill.MainGLN)) {
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"SG2"); // GLN �����������
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"NAD"); // ������������ � �����
					SegNum++;
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E3035"); // ������������ �������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"IV"); // GLN �����������
					xmlTextWriterEndElement(P_XmlWriter); //E3035
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C082"); // ������ �������
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3039); // GLN �������
							if(!isempty(Bill.Obj2GLN))
								xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.Obj2GLN);
							else if(!isempty(Bill.AgentGLN))
								xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.AgentGLN);
							else
								xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.MainGLN);
						xmlTextWriterEndElement(P_XmlWriter); //E3039
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E3055"); // ��� ������� �����������
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"9"); // EAN (������������� ���������� �������� ���������)
						xmlTextWriterEndElement(P_XmlWriter); //E3055
					xmlTextWriterEndElement(P_XmlWriter); //C082
				xmlTextWriterEndElement(P_XmlWriter); //NAD
			xmlTextWriterEndElement(P_XmlWriter); //SG2
		}
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"SG7");
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_CUX); // ������
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C504); // ������
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6347); // ������������ ������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"2"); // ��������� ������
					xmlTextWriterEndElement(P_XmlWriter); //E6347
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6345); // ������������� ������ �� ISO 4217
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6345_RUB); // �����
					xmlTextWriterEndElement(P_XmlWriter); //E6345
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6343); // ������������ ���� ������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"9"); // ������ ������
					xmlTextWriterEndElement(P_XmlWriter); //E6343
				xmlTextWriterEndElement(P_XmlWriter); //C504
			xmlTextWriterEndElement(P_XmlWriter); //CUX
		xmlTextWriterEndElement(P_XmlWriter); //SG7
	}
	CATCH
		SysLogMessage(SYSLOG_DOCPARTIES);
		ok = 0;
	ENDCATCH;
	return ok;
}
//
// �������� �������
//
int ExportCls::GoodsLines(Sdr_BRow * pBRow)
{
	int    ok = 1;
	SString str;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	THROWERR(pBRow, IEERR_NODATA);
	//
	// ����� � ���������� ������ ����� ���� ���.
	// ��� � ���� ����� ����������, ��� ������ �� ������ DESADV
	//
	if(MessageType == PPEDIOP_RECADV && pBRow->LineNo == 1) {
		TTN = pBRow->TTN;
		THROW(P_ExportCls->RecadvHeader());
		THROW(P_ExportCls->DocPartiesAndCurrency());
	}
	if(MessageType == PPEDIOP_ORDER) {
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"SG28"); // ���� � �������
	}
	else if(MessageType == PPEDIOP_RECADV) {
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"SG16"); // ���� � �������
	}
	//
	// ������ ��� RECADV
	// ���������� ������� ��������
	//
	if(MessageType == PPEDIOP_RECADV) {
		{
			SXml::WNode n_cps(P_XmlWriter, "CPS");
			{
				SXml::WNode n(P_XmlWriter, "E7164", "1"); // ����� �������� �� ��������� - 1
			}
		}
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"SG22");
	}
	{
		SXml::WNode n_lin(P_XmlWriter, "LIN");
		SegNum++;
		{
			{
				SXml::WNode n(P_XmlWriter, "E1082", str.Z().Cat(Itr.GetCount() + 1)); // ����� ������ ������
			}
			{
				SXml::WNode n_212(P_XmlWriter, "C212"); // �������� ������������� ������
				{
					SXml::WNode n(P_XmlWriter, "E7140", str.Z().Cat(pBRow->Barcode)); // �����-��� ������
				}
				{
					SXml::WNode n(P_XmlWriter, "E7143", "SRV"); // ��� ��������� EAN.UCC
				}
			}
		}
	}
	//
	// �������������� ��������
	//
	if(!isempty(pBRow->ArCode)) {
		SXml::WNode n_pia(P_XmlWriter, "PIA"); // �������������� ������������� ������
		{
			SXml::WNode n(P_XmlWriter, "E4347", "1"); // ��� ���� �������������� ������ (�������������� �������������)
		}
		{
			SXml::WNode n_212(P_XmlWriter, "C212");
			{
				SXml::WNode n(P_XmlWriter, "E7140", (str = pBRow->ArCode).Transf(CTRANSF_INNER_TO_UTF8)); // �������
			}
			{
				SXml::WNode n(P_XmlWriter, "E7143", "SA"); // ������������� ���� ��������
			}
		}
	}
	// ������ ��� ORDERS
	if(MessageType == PPEDIOP_ORDER) {
		// �������������� ��������
		if(!isempty(pBRow->GoodsName)) {
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"IMD"); // �������� ������
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E7077); // ��� ������� ��������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E7077_F); // �����
				xmlTextWriterEndElement(P_XmlWriter); //E7077
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C273); // ��������
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E7008); // ��������
					{
						SString str1;
						str1.Z().Cat(pBRow->GoodsName).ToUtf8(); // ��������� ���������� ��� ���������
						str.Z().Cat("<![CDATA[").Cat(str1).Cat("]]>"); // ������ ����������� <![CDATA[�����-�� ������]]>, ��� ��������� ����� ���� ������� �������������� �������� ��� �������
						xmlTextWriterWriteString(P_XmlWriter, str.ucptr()); // ������������ ������
					}
					xmlTextWriterEndElement(P_XmlWriter); //E7008
				xmlTextWriterEndElement(P_XmlWriter); //C273
			xmlTextWriterEndElement(P_XmlWriter); //IMD
		}
	}
	xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"QTY"); // ���������� ������
		SegNum++;
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C186); // �����������
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6063); // ������������ ���� ����������
				if(MessageType == PPEDIOP_ORDER)
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"21"); // ���������� ���������� ������
				else if(MessageType == PPEDIOP_RECADV)
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6063_194); // �������� ���������� ������
			xmlTextWriterEndElement(P_XmlWriter); //E6063
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6060); // ����������
				str.Z().Cat(pBRow->Quantity);
				xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
			xmlTextWriterEndElement(P_XmlWriter); //E6060
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6411); // ������� ���������
				(str = pBRow->UnitName).ToUpper1251();
				if(str.CmpNC(UNIT_NAME_KG) == 0)
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6411_KGM); // ����������
				else
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6411_PCE); // ��������� ��������
			xmlTextWriterEndElement(P_XmlWriter); //E6411
		xmlTextWriterEndElement(P_XmlWriter); //C186
	xmlTextWriterEndElement(P_XmlWriter); //QTY
	// ������ ��� ORDER
	if(MessageType == PPEDIOP_ORDER) {
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"MOA"); // ����� �������� ������� � ���
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C516");
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5025); // ������������ ����� �������� �������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"128"); //	������������� ����� �������� ������� � ���
				xmlTextWriterEndElement(P_XmlWriter); //E5025
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5004); // �����
					xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(pBRow->Cost * pBRow->Quantity).ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E5004
			xmlTextWriterEndElement(P_XmlWriter); //C516
		xmlTextWriterEndElement(P_XmlWriter); //MOA
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"MOA"); // ����� �������� ������� ��� ���
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C516");
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5025); // ������������ ����� �������� �������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"203"); //	������������� ����� �������� ������� ��� ���
				xmlTextWriterEndElement(P_XmlWriter); //E5025
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5004); // �����
					//str.Z().Cat((pBRow->Cost - (pBRow->Cost / (pBRow->VatRate + 100) * 100)) * pBRow->Quantity);
					str.Z().Cat(((pBRow->Cost / (pBRow->VatRate + 100)) * 100) * pBRow->Quantity);
					BillSumWithoutVat += str.ToReal();
					xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E5004
			xmlTextWriterEndElement(P_XmlWriter); //C516
		xmlTextWriterEndElement(P_XmlWriter); //MOA
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"SG32"); // ���� ������ � ���
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"PRI"); // ������� ����������
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C509");
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E5125"); // ������������ ����
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"AAE"); // ���� ��� ������ � ��������, �� � �������
					xmlTextWriterEndElement(P_XmlWriter); //E5125
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E5118"); // ����
						xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(pBRow->Cost).ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E5118
				xmlTextWriterEndElement(P_XmlWriter); //C509
			xmlTextWriterEndElement(P_XmlWriter); //PRI
		xmlTextWriterEndElement(P_XmlWriter); //SG32
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"SG32"); // ���� ������ ��� ���
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"PRI"); // ������� ����������
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C509");
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E5125"); // ������������ ����
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"AAA"); // ������ ���� ��� �������
					xmlTextWriterEndElement(P_XmlWriter); //E5125
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E5118"); // ����
						//str.Z().Cat(pBRow->Cost - (pBRow->Cost / (pBRow->VatRate + 100) * 100));
						str.Z().Cat((pBRow->Cost / (pBRow->VatRate + 100)) * 100);
						xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E5118
				xmlTextWriterEndElement(P_XmlWriter); //C509
			xmlTextWriterEndElement(P_XmlWriter); //PRI
		xmlTextWriterEndElement(P_XmlWriter); //SG32
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"SG38"); // ������ ���
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"TAX");
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5283); // ������������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"7"); // ������������� ���������� ����������
				xmlTextWriterEndElement(P_XmlWriter); //E5283
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C241");
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5153); // ������������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"VAT"); // ������������� ������ ���
					xmlTextWriterEndElement(P_XmlWriter); //E5153
				xmlTextWriterEndElement(P_XmlWriter); //C241
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C243);
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5278);
						xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(pBRow->VatRate).ucptr()); // ������ ���
					xmlTextWriterEndElement(P_XmlWriter); //E5278
				xmlTextWriterEndElement(P_XmlWriter); //C243
			xmlTextWriterEndElement(P_XmlWriter); //TAX
		xmlTextWriterEndElement(P_XmlWriter); //SG38
	}
	if(MessageType == PPEDIOP_RECADV)
		xmlTextWriterEndElement(P_XmlWriter); //SG22
	xmlTextWriterEndElement(P_XmlWriter); //SG28/SG16
	CATCH
		SysLogMessage(SYSLOG_GOODSLINES);
		ok = 0;
	ENDCATCH;
	return ok;
}
//
// ��������� ������������ ���������
//
int ExportCls::EndDoc()
{
	int    ok = 1;
	SString str;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
		// ������ ��� ORDER
		if(MessageType == PPEDIOP_ORDER) {
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"UNS"); // ����������� ���
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0081"); // ������������� ������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"S"); // ���� �������� ����������
				xmlTextWriterEndElement(P_XmlWriter); //E0081
			xmlTextWriterEndElement(P_XmlWriter); //UNS
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"MOA"); // ����� ������ � ���
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C516");
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5025); // ������������ �����
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"9"); // ����� ��������� � ���
					xmlTextWriterEndElement(P_XmlWriter); //E5025
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5004); // �����
						str.Z().Cat(Bill.Amount);
						xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E5004
				xmlTextWriterEndElement(P_XmlWriter); //�516
			xmlTextWriterEndElement(P_XmlWriter); //MOA
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"MOA"); // ����� ������ ��� ���
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C516");
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5025); // ������������ �����
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"98"); // ����� ��������� ��� ���
					xmlTextWriterEndElement(P_XmlWriter); //E5025
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5004); // �����
						str.Z().Cat(BillSumWithoutVat);
						xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E5004
				xmlTextWriterEndElement(P_XmlWriter); //�516
			xmlTextWriterEndElement(P_XmlWriter); //MOA
		}
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"CNT"); // �������� ����������
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C270");
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E6069"); // ������������ ���� �������� ����������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"2"); // ���������� �������� ������� � ���������
				xmlTextWriterEndElement(P_XmlWriter); //E6069
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E6066"); // ��������
					str.Z().Cat(Itr.GetCount());
					xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E6066
			xmlTextWriterEndElement(P_XmlWriter); //C270
		xmlTextWriterEndElement(P_XmlWriter); //CNT
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"UNT"); // ��������� ���������
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0074"); // ����� ����� ��������� � ���������
				str.Z().Cat(SegNum);
				xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
			xmlTextWriterEndElement(P_XmlWriter); //E0074
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E0062"); // ����� ������������ ��������� (��������� � ��������� � ���������)
				str.Z().Cat(ObjId);
				xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
			xmlTextWriterEndElement(P_XmlWriter); //E0062
		xmlTextWriterEndElement(P_XmlWriter); //UNT
	xmlTextWriterEndElement(P_XmlWriter); //ORDERS

	CATCH
		SysLogMessage(SYSLOG_ENDDOC);
		ok = 0;
	ENDCATCH;
	TTN = 0;
	return ok;
}
//
// ������ � soap-����������
//
// ���������� ��������
//
int ExportCls::SendDoc()
{
	int    ok = 1;
	int    r = 0;
	int64  file_size = 0;
	size_t pos = 0;
	char * buf = 0;
	SFile  file(ExpFileName, SFile::mRead);
	_ns1__Send param;
	_ns1__SendResponse resp;
	Sdr_DllImpExpReceipt exp_rcpt;
	SString str, login, edi_doc_type_symb;

	EDIWebServiceSoapProxy proxy(SOAP_XML_INDENT);
	gSoapClientInit(&proxy, 0, 0);
	file.CalcSize(&file_size);
	memzero(&exp_rcpt, sizeof(Sdr_DllImpExpReceipt));
	//
	// ���� ������������ ������ ��� ����������� ��������
	//
	if(MessageType == PPEDIOP_ORDER) { // �����
		for(pos = 0; pos < RlnCfgList.getCount(); pos++) {
			const StRlnConfig & r_item = RlnCfgList.at(pos);
			if(!r_item.SuppGLN.CmpNC(Bill.GLN) && !r_item.Direction.CmpNC(ELEMENT_CODE_DIRECTION_OUT) && r_item.EdiDocType == MessageType)
				break;
		}
	}
	else if(MessageType == PPEDIOP_RECADV) { // ����������� � �������
		for(pos = 0; pos < RlnCfgList.getCount(); pos++) {
			const StRlnConfig & r_item = RlnCfgList.at(pos);
			if(!r_item.SuppGLN.CmpNC(Bill.GLN) && !r_item.Direction.CmpNC(ELEMENT_CODE_DIRECTION_OUT) && r_item.EdiDocType == MessageType)
				break;
		}
	}
	if(pos < RlnCfgList.getCount()) {
		const StRlnConfig & r_item = RlnCfgList.at(pos);
		FormatLoginToLogin(Header.EdiLogin, login.Z()); // �� ������������
		param.Name = (char *)(const char *)login;
		//param.Name = Header.EdiLogin;			// �� ������������
		param.Password = Header.EdiPassword;	// ������
		param.PartnerIln = Bill.GLN;			// �� ��������, �������� ���������� ��������
		GetMsgTypeSymb(r_item.EdiDocType, edi_doc_type_symb);
		param.DocumentType = (char *)(const char *)edi_doc_type_symb;      // ��� ���������
		param.DocumentVersion = (char *)(const char *)r_item.DocVersion;   // ������ ������������
		param.DocumentStandard = (char *)(const char *)r_item.DocStandard; // �������� ���������
		param.DocumentTest = (char *)(const char *)r_item.DocTest;         // ������ ���������
		param.ControlNumber = (char *)Bill.Code; // ����������� ����� ��������� (������ ����� ���������)
		buf = new char[(size_t)file_size + 1];
		memzero(buf, (size_t)file_size + 1);
		file.ReadLine(str.Z()); // ���������� ������ ������ <?xml version="1.0" encoding="UTF-8" ?>
		file.Seek((long)str.Len());
		file.ReadV(buf, (size_t)file_size + 1);
		param.DocumentContent = buf; // ���������� ���������
		param.Timeout = 5000;		// ������� �� ���������� ������ ������ (��) (����� ����� �� �������� ������� web-�������)
		if((r = proxy.Send(&param, &resp)) == SOAP_OK) {
			if(atoi(resp.SendResult->Res) == 0) {
				exp_rcpt.ID = atol(Bill.ID);
				STRNSCPY(exp_rcpt.ReceiptNumber, resp.SendResult->Cnt); // �� ��������� � ������� ����������. �-�� ����� ���������
				ReceiptList.insert(&exp_rcpt);
			}
			else {
				SetError(IEERR_WEBSERV�ERR);
				SetWebServcError(atoi((const char *)resp.SendResult->Res));
				ok = 0;
			}
		}
		else {
			ProcessError(proxy);
			ok = 0;
		}
	}
	else {
		str.Z().Cat(Bill.GLN).CR().Cat("�������� ").Cat(Bill.Code);
		SetError(IEERR_NOCFGFORGLN, str);

		SString str1;
		(str1 = "�� ������ GLN ").Cat(str);
		LogMessage(str1);
		ok = -1;
	}
	if(!ok) {
		SysLogMessage(SYSLOG_SENDDOC);
	}
	ZDELETE(buf);
	return ok;
}
//
// ������� ������� ��������
//
EXPORT int InitExport(void * pExpHeader, const char * pOutFileName, int * pId)
{
	int    ok = 1;
	SFile log_file;
	SPathStruc log_path;
	SPathStruc rel_path; // @vmiller ��� ��������
	SString str;
	if(!P_ExportCls) {
		P_ExportCls = new ExportCls;
	}
	if(P_ExportCls && !P_ExportCls->Inited) {
		if(pExpHeader)
			P_ExportCls->Header = *static_cast<const Sdr_ImpExpHeader *>(pExpHeader);
		if(!isempty(pOutFileName)) {
			P_ExportCls->PathStruct.Split(pOutFileName);
			P_ExportCls->PathStruct.Ext = "xml";
			if(P_ExportCls->PathStruct.Nam.Empty())
				P_ExportCls->PathStruct.Nam = "export_";
		}
		else {
			SLS.Init("Papyrus");
			str = SLS.GetExePath();
			P_ExportCls->PathStruct.Split(str);
			P_ExportCls->PathStruct.Dir.ReplaceStr("\\bin", "\\out", 1);
			P_ExportCls->PathStruct.Nam = "export_";
			P_ExportCls->PathStruct.Ext = "xml";
		}
		log_path.Copy(&P_ExportCls->PathStruct, SPathStruc::fDrv | SPathStruc::fDir | SPathStruc::fNam | SPathStruc::fExt);
		log_path.Nam = "export_log";
		log_path.Ext = "txt";
		log_path.Merge(LogName);
		log_path.Nam = "system_log";
		log_path.Merge(SysLogName);
		P_ExportCls->Id = 1;
		*pId = P_ExportCls->Id; // �� ������ ��������
		//
		// �������� �� ���������� ������������ ������
		//
		THROW(P_ExportCls->Relationships());
		P_ExportCls->Inited = 1;
	}
	THROWERR(P_ExportCls, IEERR_IMPEXPCLSNOTINTD);
	CATCH
		SysLogMessage(SYSLOG_INITEXPORT);
		GetErrorMsg(str.Z());
		SysLogMessage(str);
		LogMessage(str);
		ok = 0;
	ENDCATCH;
	return ok;
}

EXPORT int SetExportObj(uint idSess, const char * pObjTypeSymb, void * pObjData, int * pObjId, const char * pMsgType = 0)
{
	int    ok = 1;
	SString str;
	THROWERR(P_ExportCls, IEERR_IMPEXPCLSNOTINTD);
	THROWERR(idSess == P_ExportCls->Id, IEERR_NOSESS);
	THROWERR(pObjData, IEERR_NODATA);
	THROWERR(GetObjTypeBySymb(pObjTypeSymb, P_ExportCls->ObjType), IEERR_SYMBNOTFOUND);
	THROWERR(P_ExportCls->ObjType == objBill, IEERR_ONLYBILLS);
	THROWERR_STR(GetMsgTypeBySymb(pMsgType, P_ExportCls->MessageType), IEERR_MSGSYMBNOTFOUND, pMsgType);
	THROWERR(oneof2(P_ExportCls->MessageType, PPEDIOP_ORDER, PPEDIOP_RECADV), IEERR_ONLYEXPMSGTYPES);
	// ��������� ���������� ��������
	if(P_ExportCls->P_XmlWriter) {
		THROW(P_ExportCls->EndDoc());
		xmlTextWriterEndDocument(P_ExportCls->P_XmlWriter);
		xmlFreeTextWriter(P_ExportCls->P_XmlWriter);
		P_ExportCls->P_XmlWriter = 0;
		// ���������� ����, �������������� � ������� ���. ��� ����� � ����� ��������� ��� �� ������ ������� ����� ��������
		//THROW(P_ExportCls->SendDoc());
		if(!P_ExportCls->SendDoc()) {
			SysLogMessage(SYSLOG_SETEXPORTOBJ);
			GetErrorMsg(str.Z());
			SysLogMessage(str);
			LogMessage(str);
			ok = 0;
		}
	}
	P_ExportCls->ObjId++;
	*pObjId = P_ExportCls->ObjId;
	P_ExportCls->CreateFileName(P_ExportCls->ObjId);
	P_ExportCls->P_XmlWriter = xmlNewTextWriterFilename(P_ExportCls->ExpFileName, 0);
	THROWERR(P_ExportCls->P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	// @v9.7.10 xmlTextWriterSetIndent(P_ExportCls->P_XmlWriter, 1); // @v9.7.10
	xmlTextWriterSetIndentString(P_ExportCls->P_XmlWriter, reinterpret_cast<const xmlChar *>("\t"));
	// UTF-8 - �� ���������� ����������
	xmlTextWriterStartDocument(P_ExportCls->P_XmlWriter, 0, "UTF-8", 0);
	P_ExportCls->Bill = *(Sdr_Bill *)pObjData;
	P_ExportCls->Bill.GLN[13] = 0; // ��� ������, ������, ���������� ����� ������ �������
	if(P_ExportCls->MessageType == PPEDIOP_ORDER) {
		THROW(P_ExportCls->OrderHeader());
		THROW(P_ExportCls->DocPartiesAndCurrency());
	}
	CATCH
		SysLogMessage(SYSLOG_SETEXPORTOBJ);
		GetErrorMsg(str.Z());
		SysLogMessage(str);
		LogMessage(str);
		ok = 0;
	ENDCATCH;
	return ok;
}

EXPORT int InitExportObjIter(uint idSess, uint objId)
{
	int    ok = 1;
	SString str;
	THROWERR(P_ExportCls, IEERR_IMPEXPCLSNOTINTD);
	THROWERR(idSess == P_ExportCls->Id, IEERR_NOSESS);
	THROWERR(objId == P_ExportCls->ObjId, IEERR_NOOBJID);
	P_ExportCls->Itr.Init();
	CATCH
		SysLogMessage(SYSLOG_INITEXPORTOBJITER);
		GetErrorMsg(str.Z());
		SysLogMessage(str);
		LogMessage(str);
		ok = 0;
	ENDCATCH;
	return ok;
}

EXPORT int NextExportObjIter(uint idSess, uint objId, void * pRow)
{
	int    ok = 1;
	SString str;
	THROWERR(P_ExportCls, IEERR_IMPEXPCLSNOTINTD);
	THROWERR(pRow, IEERR_NODATA);
	THROWERR(idSess == P_ExportCls->Id, IEERR_NOSESS);
	THROWERR(objId == P_ExportCls->ObjId, IEERR_NOOBJID);
	THROW(P_ExportCls->GoodsLines((Sdr_BRow *)pRow));
	P_ExportCls->Itr.Next();
	CATCH
		SysLogMessage(SYSLOG_NEXTEXPORTOBJITER);
		GetErrorMsg(str.Z());
		SysLogMessage(str);
		LogMessage(str);
		ok = 0;
	ENDCATCH;
	return ok;
}

EXPORT int EnumExpReceipt(void * pReceipt)
{
	int    ok = -1;
	SString str;
	if(P_ExportCls && pReceipt) {
		// ���� ��������� �������� ��� �� ��� ������������ � ���������, ������� ��� ������
		if(P_ExportCls->P_XmlWriter) {
			THROW(P_ExportCls->EndDoc());
			xmlTextWriterEndDocument(P_ExportCls->P_XmlWriter);
			xmlFreeTextWriter(P_ExportCls->P_XmlWriter);
			P_ExportCls->P_XmlWriter = 0;
			//THROW(P_ExportCls->SendDoc());
			if(!P_ExportCls->SendDoc()) {
				SysLogMessage(SYSLOG_ENUMEXPRECEIPT);
				GetErrorMsg(str.Z());
				SysLogMessage(str);
				LogMessage(str);
				ok = 0;
			}
		}
		// ��������� ���������� �������� ����������
		if(P_ExportCls->ReadReceiptNum < P_ExportCls->ReceiptList.getCount()) {
			*static_cast<Sdr_DllImpExpReceipt *>(pReceipt) = P_ExportCls->ReceiptList.at(P_ExportCls->ReadReceiptNum++);
			ok = 1;
		}
	}
	CATCH
		SysLogMessage(SYSLOG_ENUMEXPRECEIPT);
		GetErrorMsg(str.Z());
		SysLogMessage(str);
		LogMessage(str);
		ok = 0;
	ENDCATCH;
	return ok;
}
//
// ������
//
struct AperakInfoSt {
	AperakInfoSt()
	{
		Clear();
	}
	void Clear()
	{
		DocNum.Z();
		Code.Z();
		Msg.Z();
		AddedMsg.Z();
		SupplGLN.Z();
		BuyerGLN.Z();
		AddrGLN.Z();
		DocDate = ZERODATE;
	}
	SString DocNum;		// ����� ��������� ������
	SString Code;		// ��� �������
	SString Msg;		// �������� �������
	SString AddedMsg;	// �������������� ���������
	SString SupplGLN;	// GLN ����������
	SString BuyerGLN;	// GLN ����������
	SString AddrGLN;		// GLN ������ ��������
	LDATE	DocDate;	// ���� ���������
};

class ImportCls : public ImportExportCls {
public:
	/*
		<partner-iln>4607180239990</partner-iln>
		<tracking-id>{20fb142f-0a0a-0a0a-0a0a-0a0a0a0a0a0a}</tracking-id>
		<document-type>ORDRSP</document-type>
		<document-version>D01B</document-version>
		<document-standard>XML</document-standard>
		<document-test>P</document-test>
		<document-status>N</document-status>
		<document-number>УА000002116</document-number>
		<document-date>2015-02-24</document-date>
		<document-control-number>560731568</document-control-number>
		<receive-date>2015-02-24 16:28:20</receive-date>
	*/
	/*
	struct MessageInfoBlock {
		MessageInfoBlock()
		{
			TrackingUUID.SetZero();
			EdiDocType = 0;
			DocDate = ZERODATE;
			ReceiveDate = ZERODATE;
			CheckNumber = 0;
		}
		SString PartnerILN;
		S_GUID TrackingUUID;
		uint   EdiDocType;
        SString DocVer;
        SString DocStd;
        SString DocStatus;
        SString DocNumber;
        LDATE   DocDate;
        LDATE   ReceiveDate;
        uint32  CheckNumber;
	};
	*/
	//TSCollection <MessageInfoBlock> MsgInfoList;
	PPEdiMessageList MsgList;

	ImportCls() : GoodsCount(0), Id(0), ObjId(0), ObjType(0), MessageType_(0), Inited(0), IncomMessagesCounter(0),
		BillSumWithoutVat(0.0)
	{
		ErrorCode = 0;
		WebServcErrorCode = 0;
	}
	~ImportCls()
	{
	}
	void   CreateFileName(uint num)
	{
		ImpFileName.Z().Cat(PathStruct.Drv).CatChar(':').Cat(PathStruct.Dir).Cat(PathStruct.Nam).Cat(num).Dot().Cat(PathStruct.Ext);
	}
	int    ReceiveDoc(uint messageType);
	int    ListMessageBox(uint messageType);
	int    ParseForDocData(uint messageType, Sdr_Bill * pBill);
	int    ParseForGoodsData(uint messageType, Sdr_BRow * pBRow);
	int    ParseAperakResp(const char * pResp);
	int    SetNewStatus(SString & rErrTrackIdList);

	int    GoodsCount;          // ����� ������� � ���������
	uint   Id;                  // ��, ������� Papyrus ����� ������������ ��� �� ������ ��������
	uint   ObjId;               // �� �������������� ������� (��������, ���� ��� ����� ����������, �� � ������� ��������� ���� ��)
	uint   ObjType;             // ��� ������������� ��������
	uint   MessageType_;        // ��� ��������: ORDRESP, APERAK, DESADV
	uint   Inited;
	uint   IncomMessagesCounter;	// ������� �������� ���������. ����� � ������, ���� ���� �������� ���������, �� ��� ������������� ��� ������ ���������.
		// � ���� ������ ReceiveDoc() �� ������ ��������� LOG_NOINCOMDOC
	double BillSumWithoutVat;	// � Sdr_Bill ���� ������� �� ����������, ������� �������� ����� ��������� Papyrus'� ��� ��������
	SString DocNum;				// ����� �������������� ���������, ������� �� ����� ������ � �������� ��� ��� DESADV
	SString ImpFileName;
	SString LogFileName;		// ��� ��, ��� � ImpFileName
	SString LastTrackId;		// GUID ���������� ������������ ��������� � ������� EDI
	Iterator Itr;
	SPathStruc PathStruct;
	AperakInfoSt AperakInfo;
	StrAssocArray TrackIds;		// ������ GUID ����������� ���������� � ������� EDI, ������� ������� ���� �������� (�� ���� ������������� ��� ������� ��������)
};

EXPORT int InitImport(void * pImpHeader, const char * pInputFileName, int * pId)
{
	int    ok = 1;
	SPathStruc log_path;
	SString str;
	ZDELETE(P_ImportCls);
	P_ImportCls = new ImportCls;
	if(P_ImportCls && !P_ImportCls->Inited) {
		if(pImpHeader)
			P_ImportCls->Header = *static_cast<const Sdr_ImpExpHeader *>(pImpHeader);
		if(!isempty(pInputFileName)) {
			P_ImportCls->PathStruct.Split(pInputFileName);
			if(P_ImportCls->PathStruct.Nam.Empty())
				(P_ImportCls->PathStruct.Nam = "korus_import_").Cat(P_ImportCls->ObjId);
			if(P_ImportCls->PathStruct.Ext.Empty())
				P_ImportCls->PathStruct.Ext = "xml";
		}
		else {
			//char   fname[256];
			//GetModuleFileName(NULL, fname, sizeof(fname));
			SString module_file_name;
			SSystem::SGetModuleFileName(0, module_file_name);
			P_ImportCls->PathStruct.Split(module_file_name);
			P_ImportCls->PathStruct.Dir.ReplaceStr("\\bin", "\\in", 1);
			(P_ImportCls->PathStruct.Nam = "import_").Cat(P_ImportCls->ObjId);
			P_ImportCls->PathStruct.Ext = "xml";
		}
		log_path.Copy(&P_ImportCls->PathStruct, SPathStruc::fDrv | SPathStruc::fDir | SPathStruc::fNam | SPathStruc::fExt);
		log_path.Nam = "import_log";
		log_path.Ext = "txt";
		log_path.Merge(LogName);
		log_path.Nam = "system_log";
		log_path.Merge(SysLogName);
		P_ImportCls->Id = 1;
		*pId = P_ImportCls->Id; // �� ������ �������
		//
		// �������� ������������ ������
		//
		THROW(P_ImportCls->Relationships());
		if(!isempty(P_ImportCls->Header.EdiDocType)) {
			int    message_type = 0;
			THROWERR(GetMsgTypeBySymb(P_ImportCls->Header.EdiDocType, message_type), IEERR_MSGSYMBNOTFOUND);
			THROW(P_ImportCls->ListMessageBox(message_type));
			P_ImportCls->MessageType_ = message_type;
		}
		else {
			P_ImportCls->MessageType_ = 0;
		}
		P_ImportCls->Inited = 1;
	}
	THROWERR(P_ImportCls, IEERR_IMPEXPCLSNOTINTD);
	CATCH
		SysLogMessage(SYSLOG_INITIMPORT);
		GetErrorMsg(str.Z());
		SysLogMessage(str);
		LogMessage(str);
		ok =0;
	ENDCATCH;
	return ok;
}

EXPORT int GetImportObj(uint idSess, const char * pObjTypeSymb, void * pObjData, int * pObjId, const char * pMsgType_)
{
	int    ok = 1, r = 0;
	SString str;
	THROWERR(P_ImportCls, IEERR_IMPEXPCLSNOTINTD);
	THROWERR(idSess == P_ImportCls->Id, IEERR_NOSESS);
	THROWERR(pObjData, IEERR_NODATA);
	THROWERR(GetObjTypeBySymb(pObjTypeSymb, P_ImportCls->ObjType), IEERR_SYMBNOTFOUND);
	P_ImportCls->ObjId++;
	*pObjId = P_ImportCls->ObjId;
	THROWERR(P_ImportCls->ObjType == objBill, IEERR_ONLYBILLS);
	{
		int    message_type = 0;
		SString message_type_symb = pMsgType_ ? pMsgType_ : P_ImportCls->Header.EdiDocType;
		//
		// ������� ��� ���������
		//
		THROWERR(message_type_symb.NotEmptyS(), IEERR_MSGSYMBNOTFOUND);
		THROWERR(GetMsgTypeBySymb(message_type_symb, message_type), IEERR_MSGSYMBNOTFOUND);
		THROWERR(oneof3(/*P_ImportCls->MessageType*/message_type, PPEDIOP_ORDERRSP, PPEDIOP_APERAK, PPEDIOP_DESADV), IEERR_ONLYIMPMSGTYPES);
		// �������� ��������
		P_ImportCls->CreateFileName(P_ImportCls->ObjId);
		THROW(r = P_ImportCls->ReceiveDoc(message_type));
		ok = 1; // @vmiller ��� ��������
		if(r == -1)
			ok = -1;
		else {
			if(oneof2(message_type, PPEDIOP_ORDERRSP, PPEDIOP_DESADV)) {
				THROW(P_ImportCls->ParseForDocData(message_type, (Sdr_Bill *)pObjData)); // ������ �������� � ��������� Sdr_Bill
			}
			else if(message_type == PPEDIOP_APERAK) {
				//
				// ����� ��������� pObjData �� ��������� P_ImportCls->AperakInfo
				// ��� ����� ������� ����� ��������� � GLN ������ ��������, ���� ���������. ����� ���������� ��� ����������� �������������� ������� ����������� ��������
				// GLN ����������
				//
				P_ImportCls->AperakInfo.DocNum.CopyTo(((Sdr_Bill *)pObjData)->OrderBillNo, sizeof(((Sdr_Bill *)pObjData)->OrderBillNo));
				P_ImportCls->AperakInfo.AddrGLN.CopyTo(((Sdr_Bill *)pObjData)->DlvrAddrCode, sizeof((Sdr_Bill *)pObjData)->DlvrAddrCode);
				P_ImportCls->AperakInfo.AddrGLN.CopyTo(((Sdr_Bill *)pObjData)->LocCode, sizeof((Sdr_Bill *)pObjData)->LocCode);
				P_ImportCls->AperakInfo.BuyerGLN.CopyTo(((Sdr_Bill *)pObjData)->MainGLN, sizeof((Sdr_Bill *)pObjData)->MainGLN);
				P_ImportCls->AperakInfo.SupplGLN.CopyTo(((Sdr_Bill *)pObjData)->GLN, sizeof((Sdr_Bill *)pObjData)->GLN);
				((Sdr_Bill *)pObjData)->OrderDate = P_ImportCls->AperakInfo.DocDate;
			}
		}
	}
	CATCH
		SysLogMessage(SYSLOG_GETIMPORTOBJ);
		GetErrorMsg(str.Z());
		LogMessage(str);
		SysLogMessage(str);
		ok = 0;
	ENDCATCH;
	return ok;
}

EXPORT int InitImportObjIter(uint idSess, uint objId)
{
	int    ok = 1;
	SString str;
	THROWERR(P_ImportCls, IEERR_IMPEXPCLSNOTINTD);
	THROWERR(idSess == P_ImportCls->Id, IEERR_NOSESS);
	THROWERR(objId == P_ImportCls->ObjId, IEERR_NOOBJID);
	P_ImportCls->Itr.Init();
	CATCH
		SysLogMessage(SYSLOG_INITIMPORTOBJITER);
		GetErrorMsg(str.Z());
		LogMessage(str);
		SysLogMessage(str);
		ok = 0;
	ENDCATCH;
	return ok;
}
//
// Returns:
//		-1 - ������� ��� �������� ������. ������, ���� � ��������� � ���� ������ ���������� �������� �����,
//			������� ��������� ����� �������� �������, �� ������ ������� �� ���������
//		 0 - ������
//		 1 - ������� ��������� �������� ������
//
EXPORT int NextImportObjIter(uint idSess, uint objId, void * pRow)
{
	int    ok = 1, r = 0;
	SString str;
	THROWERR(P_ImportCls, IEERR_IMPEXPCLSNOTINTD);
	THROWERR(pRow, IEERR_NODATA);
	THROWERR(idSess == P_ImportCls->Id, IEERR_NOSESS);
	THROWERR(objId == P_ImportCls->ObjId, IEERR_NOOBJID);
	// ��������� ���������� ����� �������� � ����������� Sdr_BRow
	THROW(r = P_ImportCls->ParseForGoodsData(P_ImportCls->MessageType_, (Sdr_BRow *)pRow));
	if(r == -1)
		ok = -1;
	P_ImportCls->Itr.Next();
	CATCH
		SysLogMessage(SYSLOG_NEXTIMPORTOBJITER);
		GetErrorMsg(str.Z());
		LogMessage(str);
		SysLogMessage(str);
		ok = 0;
	ENDCATCH;
	return ok;
}
//
// ���������� Papyrus'�� ����� ������� GetImportObj. ����������� Papyrus'� ������� ���� ����� ��� �������
//
EXPORT int ReplyImportObjStatus(uint idSess, uint objId, void * pObjStatus)
{
	int    ok = 1;
	if(P_ImportCls) {
		size_t pos = 0;
		SString str;
		const  uint message_type = P_ImportCls->MessageType_;
		Sdr_DllImpObjStatus * p_obj_status = (Sdr_DllImpObjStatus *)pObjStatus;
		//
		// ���� � Papyrus ���� �����, �� ������� �������� ������������� ��� ������
		//
		if(p_obj_status->DocStatus == docStatIsSuchDoc) {
			if(message_type == PPEDIOP_APERAK) {
				// ���-������ ������ � ���� ��������
				str.Z().Cat(P_ImportCls->AperakInfo.DocNum).CatDiv(':', 2).Cat(P_ImportCls->AperakInfo.Msg).Utf8ToChar();
				LogMessage(str);
			}
			// ����� ������ �� ������
		}
		//
		// ���� � Papyrus ������ ������ ���
		//
		else if(p_obj_status->DocStatus == docStatNoSuchDoc) {
			P_ImportCls->IncomMessagesCounter--;
			if(oneof2(message_type, PPEDIOP_ORDERRSP, PPEDIOP_DESADV)) {
				// ������� ��������� ����� xml-��������
				SFile::Remove(P_ImportCls->ImpFileName);
			}
			//
			// � ��� ������������, � ��� ������� ������ � ����� ��������� ������ � ���������/������� "�����",
			// ����� ��, ���� �� ������������, ������ ��� ��������.
			//
			pos = P_ImportCls->TrackIds.getCount();
			P_ImportCls->TrackIds.Add(pos, P_ImportCls->LastTrackId);
		}
	}
	else
		ok = -1;
	return ok;
}
//
// Descr: �������� ��������. � ������ ������, ���������� ��� � ����.
//
// Returns:
//		-1 - ��� �������� ���������
//		 0 - ������
//		 1 - ��������� ��������
//
int ImportCls::ReceiveDoc(uint messageType)
{
	int    ok = -1, r = 0;
	_ns1__Receive param;
	_ns1__ReceiveResponse resp;
	SString partner_iln, track_id, str, login;
	SString edi_doc_type_symb;
	SFile  file;
	EDIWebServiceSoapProxy proxy(SOAP_XML_INDENT);
	gSoapClientInit(&proxy, 0, 0);
	assert(messageType != 0);
	/*
	const uint _pos = MsgList.getPointer();
	if(_pos < MsgList.getCount()) {
		const PPEdiMessageEntry & r_eme = MsgList.at(_pos);
		MsgList.incPointer();
	*/
	//while(MsgInfoList.getPointer() < MsgInfoList.getCount()) {
	while(MsgList.getPointer() < MsgList.getCount()) {
		//const MessageInfoBlock * p_info_blk = MsgInfoList.at(MsgInfoList.getPointer());
		//MsgInfoList.incPointer();
		const PPEdiMessageEntry & r_eme = MsgList.at(MsgList.getPointer());
		MsgList.incPointer();
		//if(p_info_blk && p_info_blk->EdiDocType == messageType) {
		if(r_eme.EdiOp == messageType) {
			//partner_iln = p_info_blk->PartnerILN;
			//p_info_blk->TrackingUUID.ToStr(S_GUID::fmtIDL, track_id);
			partner_iln = r_eme.SenderCode;
			r_eme.Uuid.ToStr(S_GUID::fmtIDL, track_id);
			//
			// ���� ������������ ������� ��� ����������� ����������� � ���� ��������
			//
			const uint rcl_c = RlnCfgList.getCount();
			uint  pos_ = 0;
			for(uint i = 0; !pos_ && i < rcl_c; i++) {
				const StRlnConfig & r_item = RlnCfgList.at(i);
				if(!r_item.SuppGLN.CmpNC(partner_iln) && r_item.EdiDocType == messageType && !r_item.Direction.CmpNC(ELEMENT_CODE_DIRECTION_IN)) {
					pos_ = i+1;
				}
			}
			if(pos_ > 0 && pos_ <= rcl_c) {
				const StRlnConfig & r_item = RlnCfgList.at(pos_-1);
				FormatLoginToLogin(Header.EdiLogin, login.Z()); // �� ������������
				param.Name = (char *)(const char *)login;
				//param.Name = Header.EdiLogin;			// �� ������������ � �������
				param.Password = Header.EdiPassword;	// ������
				param.PartnerIln = (char *)(const char *)partner_iln; // �� ��������, �������� ��� ������ �������� (��� ORDRSP ��� ����� GLN ����������,
					// � ��� APERAK ��� ����� 9999000000001 - ��������� GLN �������, � �������� ������������ ���������)
				GetMsgTypeSymb(r_item.EdiDocType, edi_doc_type_symb);
				param.DocumentType = (char *)(const char *)edi_doc_type_symb; // ��� ���������
				param.TrackingId = (char *)(const char *)track_id; // �� ��������� � �������
				param.DocumentStandard = (char *)(const char *)r_item.DocStandard;	// �������� ���������
				param.ChangeDocumentStatus = "R"; // ����� ������ ��������� ����� ���������� ������ (read)
				param.Timeout = 10000;		// ������� �� ���������� ������ ������ (��) (����� ����� �� �������� ������� web-�������)
				LastTrackId = track_id;
				if(proxy.Receive(&param, &resp) == SOAP_OK) {
					if(atoi(resp.ReceiveResult->Res) == 0) {
						if(oneof2(messageType, PPEDIOP_ORDERRSP, PPEDIOP_DESADV)) {
							file.Open(ImpFileName, SFile::mWrite);
							file.WriteLine(resp.ReceiveResult->Cnt);
							IncomMessagesCounter++;
						}
						else if(messageType == PPEDIOP_APERAK) {
							// ����� �������� �����
							file.Open(ImpFileName, SFile::mWrite);
							file.WriteLine(resp.ReceiveResult->Cnt);
							THROW(ParseAperakResp(resp.ReceiveResult->Cnt));
							IncomMessagesCounter++;
						}
						ok = 1;
					}
					else {
						SetError(IEERR_WEBSERV�ERR);
						SetWebServcError(atoi((const char *)resp.ReceiveResult->Res));
						ok = 0;
					}
				}
				else {
					ProcessError(proxy);
					ok = 0;
				}
			}
			else {
				SetError(IEERR_NOCFGFORGLN, str.Z().Cat(partner_iln));
				ok = -1;
			}
			break;
		}
	}
	CATCHZOK
	if(!ok)
		SysLogMessage(SYSLOG_RECEIVEDOC);
	return ok;
}
//
// Descr: ������ ������ ������������ ��������� � New
//
int ImportCls::SetNewStatus(SString & rErrTrackIdList)
{
	int    ok = 1;
	SString str, login;
	char   track_id_buf[256];
	_ns1__ChangeDocumentStatus param;
	_ns1__ChangeDocumentStatusResponse resp;
	EDIWebServiceSoapProxy proxy(SOAP_XML_INDENT);
	gSoapClientInit(&proxy, 0, 0);
	rErrTrackIdList = 0;
	for(size_t pos = 0; pos < TrackIds.getCount(); pos++) {
		STRNSCPY(track_id_buf, TrackIds.Get(pos).Txt);
		FormatLoginToLogin(Header.EdiLogin, login.Z()); // �� ������������
		param.Name = (char *)login.cptr(); // @badcast
		//param.Name = Header.EdiLogin;			// �� ������������ � �������
		param.Password = Header.EdiPassword;	// ������
		param.TrackingId = track_id_buf; // �� ��������� � �������
		param.Status = "N"; // ����� ������ ��������� (new)
		if(proxy.ChangeDocumentStatus(&param, &resp) == SOAP_OK) {
			if(atoi(resp.ChangeDocumentStatusResult->Res) != 0) {
				SetError(IEERR_WEBSERV�ERR);
				SetWebServcError(atoi((const char *)resp.ChangeDocumentStatusResult->Res));
				if(rErrTrackIdList.Empty())
					rErrTrackIdList = track_id_buf;
				else
					rErrTrackIdList.Comma().Cat(track_id_buf);
				ok = 0;
			}
		}
		else {
			ProcessError(proxy);
			if(rErrTrackIdList.Empty())
				rErrTrackIdList = track_id_buf;
			else
				rErrTrackIdList.Comma().Cat(track_id_buf);
			ok = 0;
		}
	}
	if(!ok)
		SysLogMessage(SYSLOG_SETNEWSTATUS);
	return ok;
}
//
// Returns:
//		-1 - ��� ���������
//		 0 - ������
//		 1 - ���� ���������
//
int ImportCls::ListMessageBox(uint messageType)
{
	MsgList.Clear();
	int    ok = -1;
	uint   pos = 0;
	xmlTextReader * p_xml_ptr = 0;
	xmlParserInputBuffer * p_input = 0;
	//MessageInfoBlock * p_info_blk = 0;
	SString fmt, low, upp, str, login;
	SString edi_doc_type_symb;
	_ns1__ListMBEx param;
	_ns1__ListMBExResponse resp;
	EDIWebServiceSoapProxy proxy(SOAP_XML_INDENT);
	gSoapClientInit(&proxy, 0, 0);
	//
	// ������ ��������� ������ �������� ����������/�������� �����.
	// 1. ������� ������ ������������ ��� �������� ����������. ���������������, ��� ����� ������ ���������.
	// 2. ���������, ���� �� �� ���� �������� ���������.
	// 3.1. ���� ����, ���������� ������ ���� � ��������� � ������� �� ����� � ������������� �����������
	// 3.2. ���� �������� ��������� �� ���������, �� ������� ��������� ������������, �� ���� ���������� ����������
	// 4. ���� ���� �������� ���� ��� �������� ���� �����������, �� ������� �� ����� � ������� �����������
	//
	THROWERR(RlnCfgList.getCount(), IEERR_NOCONFIG);
	for(pos = 0; pos < RlnCfgList.getCount(); pos ++) {
		const StRlnConfig & r_item = RlnCfgList.at(pos);
		if(r_item.Direction.CmpNC(ELEMENT_CODE_DIRECTION_IN) == 0 && r_item.EdiDocType == messageType) {
			FormatLoginToLogin(Header.EdiLogin, login.Z()); // �� ������������
			param.Name = (char *)login.cptr(); // @badcast
			//param.Name = Header.EdiLogin;			// �� ������������
			param.Password = Header.EdiPassword;	// ������ ������������
			param.PartnerIln = (char *)(const char *)r_item.SuppGLN;           // �� ��������, �� �������� ��� ������� ��������
			GetMsgTypeSymb(r_item.EdiDocType, edi_doc_type_symb);
			param.DocumentType = (char *)(const char *)edi_doc_type_symb;      // ��� ���������
			param.DocumentVersion = (char *)(const char *)r_item.DocVersion;   // ������ ������������
			param.DocumentStandard = (char *)(const char *)r_item.DocStandard; // �������� ���������
			param.DocumentTest = (char *)(const char *)r_item.DocTest;         // ������ ���������
			param.DocumentStatus = "N";	// ������ ���������� ���������� (�������������, ��������) (������ �����) (N - ������ �����, A - ���)
			param.Timeout = 10000;		// ������� �� ���������� ������ ������ (��) (����� ����� �� �������� ������� web-�������)
			if((Header.PeriodLow != ZERODATE) && (Header.PeriodUpp != ZERODATE)) {
				low.Z().Cat(Header.PeriodLow.year());
				if(fmt.Z().Cat(Header.PeriodLow.month()).Len() == 1)
					fmt.PadLeft(1, '0');
				low.CatChar('-').Cat(fmt);
				if(fmt.Z().Cat(Header.PeriodLow.day()).Len() == 1)
					fmt.PadLeft(1, '0');
				low.CatChar('-').Cat(fmt);
				upp.Z().Cat(Header.PeriodUpp.year());
				if(fmt.Z().Cat(Header.PeriodUpp.month()).Len() == 1)
					fmt.PadLeft(1, '0');
				upp.CatChar('-').Cat(fmt);
				if(fmt.Z().Cat(Header.PeriodUpp.day()).Len() == 1)
					fmt.PadLeft(1, '0');
				upp.CatChar('-').Cat(fmt);
				param.DateFrom = (char *)(const char *)low;
				param.DateTo = (char *)(const char *)upp;
			}
			if(proxy.ListMBEx(&param, &resp) == SOAP_OK) {
				if(atoi(resp.ListMBExResult->Res) == 0) {
					if(strcmp(resp.ListMBExResult->Cnt, EMPTY_LISTMB_RESP) != 0) {
						SString xml_input = resp.ListMBExResult->Cnt;
						SString str, cname;
						p_input = xmlParserInputBufferCreateMem(xml_input, xml_input.Len(), XML_CHAR_ENCODING_NONE);
						THROWERR((p_xml_ptr = xmlNewTextReader(p_input, NULL)), IEERR_NULLREADXMLPTR);
						while(xmlTextReaderRead(p_xml_ptr)) {
							xmlNode * p_node = xmlTextReaderCurrentNode(p_xml_ptr);
							if(p_node && p_node->children && sstreqi_ascii((const char *)p_node->name, "document-info")) {
								//THROWERR(p_info_blk = new MessageInfoBlock, IEERR_NOMEM);
								PPEdiMessageEntry eme;
								for(xmlNode * p_doc_child = p_node->children; p_doc_child != 0; p_doc_child = p_doc_child->next) {
									if(p_doc_child->children) {
										cname.Set(p_doc_child->name);
										if(cname.CmpNC("tracking-id") == 0) { // �� ��������� � �������
											//p_info_blk->TrackingUUID.FromStr((const char *)p_doc_child->children->content);
											eme.Uuid.FromStr((const char *)p_doc_child->children->content);
										}
										else if(cname.CmpNC(ELEMENT_NAME_PARTNER_ILN) == 0) {
											//p_info_blk->PartnerILN.Set(p_doc_child->children->content);
											STRNSCPY(eme.SenderCode, (const char *)p_doc_child->children->content);
										}
										else if(cname.CmpNC("document-type") == 0) {
											GetMsgTypeBySymb((const char *)p_doc_child->children->content, eme.EdiOp);
										}
										else if(cname.CmpNC("document-version") == 0) {
											//p_info_blk->DocVer.Set(p_doc_child->children->content);
										}
										else if(cname.CmpNC("document-standard") == 0) {
											//p_info_blk->DocStd.Set(p_doc_child->children->content);
										}
										else if(cname.CmpNC("document-status") == 0) {
											//p_info_blk->DocStatus.Set(p_doc_child->children->content);
										}
										else if(cname.CmpNC("document-number") == 0) {
											//p_info_blk->DocNumber.Set(p_doc_child->children->content);
											STRNSCPY(eme.Code, p_doc_child->children->content);
										}
										else if(cname.CmpNC("document-date") == 0) {
											//strtodate((const char *)p_doc_child->children->content, DATF_YMD|DATF_CENTURY, &p_info_blk->DocDate);
											strtodate((const char *)p_doc_child->children->content, DATF_YMD|DATF_CENTURY, &eme.Dtm.d);
										}
										else if(cname.CmpNC("receive-date") == 0) {
											//strtodate((const char *)p_doc_child->children->content, DATF_YMD|DATF_CENTURY, &p_info_blk->ReceiveDate);
										}
										else if(cname.CmpNC("document-control-number") == 0) {
											//p_info_blk->CheckNumber = strtoul((const char *)p_doc_child->children->content, 0, 10);
										}
									}
								}
								//MsgInfoList.insert(p_info_blk);
								MsgList.Add(eme);
								//p_info_blk = 0;
							}
						}
						ok = 1;
						break;
					}
					else
						ok = -1;
				}
				else {
					SetWebServcError(atoi((const char *)resp.ListMBExResult->Res));
					THROWERR(0, IEERR_WEBSERV�ERR);
				}
			}
			else {
				ProcessError(proxy);
				break;
			}
		}
	}
	CATCH
		SysLogMessage(SYSLOG_LISTMESSAGEBOX);
		ok = 0;
	ENDCATCH
	xmlFreeParserInputBuffer(p_input);
	xmlFreeTextReader(p_xml_ptr);
	//delete p_info_blk;
	return ok;
}
//
// ��������� ���������� �������� � ��������� Sdr_Bill
//
int ImportCls::ParseForDocData(uint messageType, Sdr_Bill * pBill)
{
	int    ok = 0, exit_while = 0;
	SString str, temp_buf;
	xmlDoc * p_doc = 0;
	THROWERR_STR(fileExists(ImpFileName), IEERR_IMPFILENOTFOUND, ImpFileName);
	THROWERR(pBill, IEERR_NODATA);
	memzero(pBill, sizeof(Sdr_Bill));
	THROWERR((p_doc = xmlReadFile(ImpFileName, NULL, XML_PARSE_NOENT)), IEERR_NULLREADXMLPTR);
	xmlNode * p_node = 0;
	xmlNode * p_root = xmlDocGetRootElement(p_doc);
	THROWERR(p_root, IEERR_XMLREAD);
	// @v8.5.6 {
	STRNSCPY(pBill->EdiOpSymb, p_root->name);
	pBill->EdiOp = messageType;
	// } @v8.5.6
	p_node = p_root->children;
	while(p_node && p_node->type == XML_ELEMENT_NODE) {
		if(p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT))
			p_node = p_node->children;
		else if(p_node->next)
			p_node = p_node->next;
		else {
			xmlNode * p_node_2 = 0;
			while(p_node && p_node->P_ParentNode && !exit_while) {
				p_node_2 = p_node->P_ParentNode->next;
				if(p_node_2) {
					p_node = p_node_2;
					exit_while = 1;
				}
				else
					p_node = p_node->P_ParentNode;
			}
		}
		exit_while = 0;
		if(p_node && (p_node->type == XML_ELEMENT_NODE)) {
			if(SXml::IsName(p_node, "E0065") && p_node->children) {
				if(messageType == PPEDIOP_ORDERRSP) {
					THROWERR_STR(SXml::IsContent(p_node->children, "ORDRSP"), IEERR_INVMESSAGEYTYPE, "ORDRSP")
				}
				else if(messageType == PPEDIOP_DESADV) {
					THROWERR_STR(SXml::IsContent(p_node->children, "DESADV"), IEERR_INVMESSAGEYTYPE, "DESADV");
				}
			}
			else if(SXml::IsName(p_node, "E1004") && p_node->children) {
				str.Set(p_node->children->content).Utf8ToChar(); // ����� ����� ��������� � Papyrus
				str.CopyTo(pBill->Code, sizeof(pBill->Code));
				DocNum = str;
				ok = 1;
				//strcpy(pBill->Code, (const char *)p_node->children->content);
				//DocNum.CopyFrom((const char *)p_node->children->content);
				//ok = 1;
			}
			else if(SXml::IsName(p_node, "C506") && p_node->children && p_node->children->type == XML_READER_TYPE_ELEMENT) {
				p_node = p_node->children; // <E1153>
				if(SXml::IsName(p_node, "E1153") && p_node->children) {
					if(SXml::IsContent(p_node->children, "ON")) {
						if(p_node->next) {
							p_node = p_node->next; // <E1154>
							if(SXml::IsName(p_node, "E1154") && p_node->children) {
								// ����� ������, �� ������� ������ �������������
								STRNSCPY(pBill->OrderBillNo, p_node->children->content);
								ok = 1;
							}
						}
					}
				}
			}
			else if(SXml::IsName(p_node, "C507") && p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT)) {
				p_node = p_node->children; // <E2005>
				if(p_node && SXml::IsName(p_node, "E2005") && p_node->children) {
					str.Set(p_node->children->content); // �������� �������� ��������
					if(p_node->next) {
						p_node = p_node->next; // <E2380>
						if(p_node && SXml::IsName(p_node, "E2380") && p_node->children) {
							//
							// Note: ������ � ����� ����� ������: YYYYMMDD. ��� ���� �������� ������� ��������� �����.
							//   ��-�� ����� �������������� �������� ������ ���� �� 8 �������� (������� strtodate ��������� ���������
							//   ���� ��� ������������ ������ ��� ������������� ������� ������ � 8 ��������).
							//
							if(str.CmpNC(ELEMENT_CODE_E2005_137) == 0) {
								temp_buf.Set(p_node->children->content).Trim(8); // ���� ���������
								strtodate(temp_buf, DATF_YMD|DATF_CENTURY, &pBill->Date);
								ok = 1;
							}
							else if((str.CmpNC(ELEMENT_CODE_E2005_17) == 0) || (str.CmpNC("2") == 0) || (str.CmpNC(ELEMENT_CODE_E2005_358) == 0)) {
								temp_buf.Set(p_node->children->content).Trim(8); // ���� �������� (���� ���������� ���������)
								strtodate(temp_buf, DATF_YMD|DATF_CENTURY, &pBill->DueDate);
								ok = 1;
							}
							else if(str.CmpNC(ELEMENT_CODE_E2005_171) == 0) {
								temp_buf.Set(p_node->children->content).Trim(8); // ���� ������
								strtodate(temp_buf, DATF_YMD|DATF_CENTURY, &pBill->OrderDate);
								ok = 1;
							}
						}
					}
				}
			}
			else if(SXml::IsName(p_node, "E3035") && p_node->children) {
				str.Set(p_node->children->content); // �������� �������� ��������
				if(p_node->next) {
					p_node = p_node->next; // <C082>
					if(SXml::IsName(p_node, "C082") && p_node->children) {
						p_node = p_node->children; // <E3039>
						if(SXml::IsName(p_node, ELEMENT_NAME_E3039) && p_node->children) {
							if(str.CmpNC("BY") == 0) { // GLN ����������
								STRNSCPY(pBill->MainGLN, (const char *)p_node->children->content);
								STRNSCPY(pBill->AgentGLN, (const char *)p_node->children->content);
								ok = 1;
							}
							else if(str.CmpNC("SU") == 0) { // GLN ����������
								STRNSCPY(pBill->GLN, p_node->children->content);
								ok = 1;
							}
							else if(str.CmpNC("DP") == 0) { // GLN ������ ��������
								STRNSCPY(pBill->DlvrAddrCode, p_node->children->content);
								ok = 1;
							}
							else if(str.CmpNC("IV") == 0) { // GLN �����������
								STRNSCPY(pBill->Obj2GLN, p_node->children->content);
								ok = 1;
							}
						}
					}
				}
			}
			else if(SXml::IsName(p_node, ELEMENT_NAME_E1229) && p_node->children) { // ��� �������� (���������, ������� ��� ���������, �� �������)
				str.Set(p_node->children->content); // @vmiller
				ok = 1;
			}
			else if(SXml::IsName(p_node, "C516") && p_node->children && p_node->children->type == XML_READER_TYPE_ELEMENT) {
				p_node = p_node->children; // <E5025>
				if(p_node && SXml::IsName(p_node, ELEMENT_NAME_E5025) && p_node->children) {
					str.Set(p_node->children->content); // �������� �������� �������� ��������
					if(p_node->next) {
						p_node = p_node->next; // <E5004>
						if(p_node && SXml::IsName(p_node, ELEMENT_NAME_E5004) && p_node->children) {
							if(str == "9") { // ����� ��������� � ���
								pBill->Amount = atof((const char *)p_node->children->content);
								ok = 1;
							}
							else if(str == "98") { // ����� ��������� ��� ���
								str.Set(p_node->children->content); // @vmiller
								ok = 1;
							}
						}
					}
				}
			}
			else if(SXml::IsName(p_node, "E6069") && p_node->children) {
				if(SXml::IsContent(p_node->children, "2")) {
					if(p_node->next) {
						p_node = p_node->next; // <E6060>
						if(SXml::IsName(p_node, "E6066") && p_node->children) {
							// ������� ���������� �������� ������� � ���������
							GoodsCount = atoi((const char *)p_node->children->content);
							ok = 1;
						}
					}
				}
			}
		}
	}
	CATCH
		SysLogMessage(SYSLOG_PARSEFORDOCDATA);
		ok = 0;
	ENDCATCH;
	if(p_doc)
		xmlFreeDoc(p_doc);
	return ok;
}
//
// ��������� ���������� �������� � ��������� Sdr_BRow
// Returns:
//		-1 - ������� ��� �������� ������. ������, ���� � ��������� � ���� ������ ���������� �������� �����,
//			������� ��������� ����� �������� �������, �� ������ ������� �� ���������
//		 0 - ������
//		 1 - ������� ��������� �������� ������
//
int ImportCls::ParseForGoodsData(uint messageType, Sdr_BRow * pBRow)
{
	int    ok = 1, index = 1, sg26_end = 0, exit_while = 0;
	SString str, goods_segment;
	xmlDoc * p_doc = 0;

	THROWERR_STR(fileExists(ImpFileName), IEERR_IMPFILENOTFOUND, ImpFileName);
	THROWERR(pBRow, IEERR_NODATA);
	memzero(pBRow, sizeof(Sdr_BRow));
	THROWERR((p_doc = xmlReadFile(ImpFileName, NULL, XML_PARSE_NOENT)), IEERR_NULLREADXMLPTR);
	xmlNode * p_node = 0;
	xmlNode * p_root = xmlDocGetRootElement(p_doc);
	THROWERR(p_root, IEERR_XMLREAD);
	if(Itr.GetCount() < (uint)GoodsCount) {
		p_node = p_root->children;
		if(messageType == PPEDIOP_ORDERRSP)
			goods_segment = ELEMENT_NAME_SG26;
		else if(messageType == PPEDIOP_DESADV)
			goods_segment = ELEMENT_NAME_SG17;
		while(p_node && !sg26_end) {
			if(p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT))
				p_node = p_node->children;
			else if(p_node->next)
				p_node = p_node->next;
			else {
				xmlNode * p_node_2 = 0;
				while(p_node && p_node->P_ParentNode && !exit_while) {
					p_node_2 = p_node->P_ParentNode->next;
					if(p_node_2) {
						p_node = p_node_2;
						exit_while = 1;
					}
					else
						p_node = p_node->P_ParentNode;
				}
			}
			exit_while = 0;
			// ��������� ������� ��������� ������ �������� �������
			if(p_node && p_node->type == XML_READER_TYPE_ELEMENT) {
				if(SXml::IsName(p_node, goods_segment) && p_node->children) {
					if(index == (Itr.GetCount() + 1)) {
						while(p_node && !sg26_end) {
							exit_while = 0;
							if(p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT))
								p_node = p_node->children;
							else if(p_node->next)
								p_node = p_node->next;
							else {
								xmlNode * p_node_2 = 0;
								while(p_node && p_node->P_ParentNode && !exit_while) {
									p_node_2 = p_node->P_ParentNode->next;
									if(p_node_2) {
										p_node = p_node_2;
										exit_while = 1;
									}
									else
										p_node = p_node->P_ParentNode;
								}
							}
							if(p_node) {
								if(p_node->type == XML_DOCUMENT_NODE || SXml::IsName(p_node, goods_segment)) // ������ ������� ��������� ��� ��������� �������� �������. ���� ����� �� ������, �� ���� ������ ������ �������� ������.
									sg26_end = 1;
								else if(SXml::IsName(p_node, "LIN") && p_node->children && p_node->children->type == XML_READER_TYPE_ELEMENT) {
									p_node = p_node->children; // <E1082>
									if(SXml::IsName(p_node, "E1082") && p_node->children) { // ����� �������� �������
										str.Set(p_node->children->content); // @vmiller
										if(p_node->next) {
											p_node = p_node->next; // <E1229>
											if(SXml::IsName(p_node, ELEMENT_NAME_E1229) && p_node->children) { // ������ �������� �������
												str.Set(p_node->children->content); // @vmiller
											}
										}
									}
								}
								else if(SXml::IsName(p_node, "C212") && p_node->children && p_node->children->type == XML_READER_TYPE_ELEMENT) {
									p_node = p_node->children; // <E7140>
									if(SXml::IsName(p_node, "E7140") && p_node->children) {
										str.Set(p_node->children->content); // �������� �������� �������� �������� (��������)
										if(p_node->next) {
											p_node = p_node->next; // <E7143>
											if(SXml::IsName(p_node, "E7143") && p_node->children) {
												if(SXml::IsContent(p_node->children, "SRV")) // �������� ������
													str.CopyTo(pBRow->Barcode, sizeof(pBRow->Barcode));
											}
										}
									}
								}
								else if(SXml::IsName(p_node, "E4347") && p_node->children) {
									if(SXml::IsContent(p_node->children, "1")) {
										//xmlTextReaderRead(p_xml_ptr); // text
										//xmlTextReaderRead(p_xml_ptr); // </E4347>
										//xmlTextReaderRead(p_xml_ptr); // <C212>
										//xmlTextReaderRead(p_xml_ptr); // <E7140>
										//p_node = xmlTextReaderCurrentNode(p_xml_ptr);
										if(p_node->next && p_node->next->children && p_node->next->children->type == XML_READER_TYPE_ELEMENT) {
											p_node = p_node->next->children; // <E7140> (��������� <C212>)
											if(SXml::IsName(p_node, "E7140") && p_node->children) {
												str.Set(p_node->children->content); // �������� �������� �������� �������� (������� ������ � ����������)
												if(p_node->next) {
													p_node = p_node->next; // <E7143>
													if(SXml::IsName(p_node, "E7143") && p_node->children) {
														if(SXml::IsContent(p_node->children, "SA")) // ������� ����������
															str.CopyTo(pBRow->ArCode, sizeof(pBRow->ArCode));
													}
												}
											}
										}
									}
								}
								else if(SXml::IsName(p_node, ELEMENT_NAME_E7008) && p_node->children) {
									str.Set(p_node->children->content).Utf8ToOem(); // ������������ ������
									STRNSCPY(pBRow->GoodsName, str);
								}
								else if(SXml::IsName(p_node, ELEMENT_NAME_C186) && p_node->children && p_node->children->type == XML_READER_TYPE_ELEMENT) {
									p_node = p_node->children; // <E6063>
									if(SXml::IsName(p_node, ELEMENT_NAME_E6063) && p_node->children) {
										str.Set(p_node->children->content); // �������� �������� �������� ��������
										if(p_node->next) {
											p_node = p_node->next; // <E6060>
											if(SXml::IsName(p_node, ELEMENT_NAME_E6060) && p_node->children) {
												if(str == "21")
													str.Set(p_node->children->content); // @vmiller ���������� ���������� (ORDRSP/DESADV)
												else if(str == "113" || str == "170")
													pBRow->Quantity = atof((const char *)p_node->children->content); // �������������� ���������� (ORDRSP)
												else if(str == "12")
													pBRow->Quantity = atof((const char *)p_node->children->content); // ����������� ���������� (DESADV)
												else if(str == "59")
													pBRow->UnitPerPack = atof((const char *)p_node->children->content); // ���������� ������ � ��������
											}
										}
									}
								}
								else if(SXml::IsName(p_node, ELEMENT_NAME_E6411) && p_node->children) { // ������� ��������� ������
									if(SXml::IsContent(p_node->children, ELEMENT_CODE_E6411_KGM))
										STRNSCPY(pBRow->UnitName, UNIT_NAME_KG);
									else
										STRNSCPY(pBRow->UnitName, UNIT_NAME_PIECE);
								}
								else if(SXml::IsName(p_node, "C516") && p_node->children && p_node->children->type == XML_READER_TYPE_ELEMENT) {
									p_node = p_node->children; // <E5025>
									if(SXml::IsName(p_node, ELEMENT_NAME_E5025) && p_node->children) {
										str.Set(p_node->children->content); // �������� �������� �������� ��������
										if(p_node->next) {
											p_node = p_node->next; // <E5004>
											if(SXml::IsName(p_node, ELEMENT_NAME_E5004) && p_node->children) {
												if(str == "203")
													str.Set(p_node->children->content); // @vmiller // ����� �������� ������� ��� ���
												else if(str == "79")
													str.Set(p_node->children->content); // @vmiller // ����� �������� ������� � ���
												else if(messageType == PPEDIOP_DESADV && str.CmpNC(ELEMENT_CODE_E5025_XB5) == 0)
													pBRow->Cost = atof((const char *)p_node->children->content); // ���� ������ � ��� ��� DESADV
											}
										}
									}
								}
								else if(SXml::IsName(p_node, "C509") && p_node->children && p_node->children->type == XML_READER_TYPE_ELEMENT) {
									p_node = p_node->children; // <E5125>
									if(SXml::IsName(p_node, "E5125") && p_node->children) {
										str.Set(p_node->children->content); // �������� �������� �������� ��������
										if(p_node->next) {
											p_node = p_node->next; // <E5118>
											if(SXml::IsName(p_node, "E5118") && p_node->children) {
												if(str.CmpNC("AAA") == 0)
													str.Set(p_node->children->content); // @vmiller // ���� ������ ��� ���
												else if(str.CmpNC("AAE") == 0)
													pBRow->Cost = atof((const char *)p_node->children->content); // ���� ������ � ���
											}
										}
									}
								}
							}
							DocNum.CopyTo(pBRow->TTN, DocNum.Len() + 1);
						}
					}
					index++;
				}
			}
		}
	}
	else {
		ok = -1;
	}
	CATCH
		SysLogMessage(SYSLOG_PARSEFORGOODDATA);
		ok = 0;
	ENDCATCH;
	if(p_doc)
		xmlFreeDoc(p_doc);
	return ok;
}
//
// Descr: ������ ����������, ����������� � ������� Receive() ��� APERAK
// ARG(pResp		 IN): ������ ��� �������, ���������� � Receive()
// Retruns:
//		-1 - �� ���� �������� �������� ���������
//		 0 - ������
//		 1 - ��� ��������� �������
//
int ImportCls::ParseAperakResp(const char * pResp)
{
	int    ok = 1;
	SString str;
	xmlTextReader * p_xml_ptr;
	xmlParserInputBuffer * p_input = 0;
	xmlNode * p_node;
	AperakInfo.Clear();
	if(pResp) {
		p_input = xmlParserInputBufferCreateMem(pResp, sstrlen(pResp), XML_CHAR_ENCODING_NONE);
		THROWERR((p_xml_ptr = xmlNewTextReader(p_input, NULL)), IEERR_NULLREADXMLPTR);
		while(xmlTextReaderRead(p_xml_ptr)) {
			p_node = xmlTextReaderCurrentNode(p_xml_ptr);
			if(p_node) {
				if(SXml::IsName(p_node, "E0065") && p_node->children) {
					THROWERR_STR(SXml::IsContent(p_node->children, "APERAK"), IEERR_INVMESSAGEYTYPE, "APERAK");
				}
				else if(p_node && SXml::IsName(p_node, "E1153") && p_node->children) {
					if(SXml::IsContent(p_node->children, "ON")) {
						// �������� ����� ��������� ������
						if(p_node->next)
							if(SXml::IsName(p_node->next, "E1154") && p_node->next->children)
								AperakInfo.DocNum = (const char *)p_node->next->children->content;
					}
				}
				else if(p_node && SXml::IsName(p_node, "E2005") && p_node->children) {
					if(SXml::IsContent(p_node->children, ELEMENT_CODE_E2005_171)) { // ���� ��������� ������
						if(p_node->next) {
							if(SXml::IsName(p_node->next, "E2380") && p_node->next->children) {
								strtodate((const char *)p_node->next->children->content, DATF_YMD|DATF_CENTURY, &AperakInfo.DocDate);
							}
						}
					}
				}
				else if(p_node && SXml::IsName(p_node, "E3035") && p_node->children) {
					str.Set(p_node->children->content); // �������� �������� ��������
					if(p_node->next) {
						p_node = p_node->next;
						if(SXml::IsName(p_node, "C082") && p_node->children) {
							if(SXml::IsName(p_node->children, ELEMENT_NAME_E3039)) {
								if(str.CmpNC("BY") == 0)
									AperakInfo.BuyerGLN = (const char *)p_node->children->children->content; // GLN ����������
								else if(str.CmpNC("SU") == 0)
									AperakInfo.SupplGLN = (const char *)p_node->children->children->content; // GLN ����������
								else if(str.CmpNC("DP") == 0)
									AperakInfo.AddrGLN = (const char *)p_node->children->children->content; // GLN ������ ��������
							}
						}
					}
				}
				else if(p_node && SXml::IsName(p_node, "E9321") && p_node->children) {
					AperakInfo.Code.Set(p_node->children->content); // ��� �������
				}
				else if(p_node && SXml::IsName(p_node, "E4451") && p_node->children) {
					if(SXml::IsContent(p_node->children, ELEMENT_CODE_E4451_AAO)) {
						if(p_node->next) {
							if(p_node->next->children) {
								if(SXml::IsName(p_node->next->children, "E4440") && p_node->next->children->children)
									AperakInfo.Msg.Set(p_node->next->children->children->content); // �������� �������
							}
						}
					}
				}
				// ������ �� ���������� ���� ��������
				//else if(p_node && (strcmp((const char *)p_node->name, "E4441") == 0) && p_node->children) {
				//	// �������� �������������� ���������
				//	AperakInfo.AddedMsg = (const char *)p_node->children->content;
				//}
			}
		}
	}
	else
		ok = -1;
	CATCH
		SysLogMessage(SYSLOG_PARSEAPERAKRESP);
		ok = 0;
	ENDCATCH;
	xmlFreeParserInputBuffer(p_input);
	xmlFreeTextReader(p_xml_ptr);
	return ok;
}
//
// ����� ������� ��� �������/��������
//
EXPORT int FinishImpExp()
{
	int    ok = 1;
	SString str, err_track_id_list;
	if(P_ExportCls) {
		// ���� EnumExpReceipt() �� ��� ��������
		if(P_ExportCls->P_XmlWriter) {
			THROW(P_ExportCls->EndDoc());
			xmlTextWriterEndDocument(P_ExportCls->P_XmlWriter);
			xmlFreeTextWriter(P_ExportCls->P_XmlWriter);
			P_ExportCls->P_XmlWriter = 0;
			THROW(P_ExportCls->SendDoc());
		}
	}
	if(P_ImportCls) {
		THROW(P_ImportCls->SetNewStatus(err_track_id_list.Z()));
		if(!P_ImportCls->IncomMessagesCounter) {
			SysLogMessage(LOG_NOINCOMDOC);
			LogMessage(LOG_NOINCOMDOC);
		}
	}
	CATCH
		if(err_track_id_list.NotEmpty()) {
			(str = LOG_NEWSTATERR).Cat(err_track_id_list);
			SysLogMessage(str);
			LogMessage(str);
		}
		SysLogMessage(SYSLOG_FINISHIMPEXP);
		GetErrorMsg(str.Z());
		SysLogMessage(str);
		LogMessage(str);
		ok = 0;
	ENDCATCH;
	ZDELETE(P_ExportCls);
	ZDELETE(P_ImportCls);
	return ok;
}

EXPORT int GetErrorMessage(char * pMsg, uint bufLen)
{
	SString str;
	GetErrorMsg(str);
	memzero(pMsg, bufLen);
	if(str.NotEmpty() && pMsg)
		str.CopyTo(pMsg, bufLen < (str.Len() + 1) ? bufLen : (str.Len() + 1));
	ErrorCode = 0;
	WebServcErrorCode = 0;
	StrError = "";
	return 1;
}

void ProcessError(EDIWebServiceSoapProxy & rProxy)
{
	char   temp_err_buf[1024];
	SString temp_buf;
	ErrorCode = IEERR_SOAP;
	rProxy.soap_sprint_fault(temp_err_buf, sizeof(temp_err_buf));
	temp_buf = temp_err_buf;
	temp_buf.ReplaceChar('\xA', ' ');
	temp_buf.ReplaceChar('\xD', ' ');
	temp_buf.ReplaceStr("  ", " ", 0);
	StrError = temp_buf;
}
