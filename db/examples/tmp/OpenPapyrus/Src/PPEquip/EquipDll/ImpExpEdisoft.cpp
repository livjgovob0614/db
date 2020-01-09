// IMPEXPEDISOFT.CPP
// ���������� ��� �������/�������� ���������� � xml ����� web-������ EDISoft
//
//
//
// �������
// (Order, Recadv)
// ������� �������� �� Papyrus
//		��������:
//		����� ����� ��������� ��� ���. ��������������. (Order/Recadv)
//		����� ���������� �����������/��������� ������. ��������������. (Order/Recadv)
//		�������� ���� �������� - Bill.DueDate (Order)
//		���� ��������� - Bill.Date (Order, Recadv)
//		����������� ���� ������ ������ - Bill.Date (Recadv)
//		����� ������ - Bill.OrderBillNo (Recadv) (������, ��� ����� �� �����������)
//		GLN ����������� � GLN ���������� - Bill.AgentGLN / Bill.MainGLN (Order, Recadv)
//		GLN ���������� � GLN �������� - Bill.GLN (Order, Recadv)
//		GLN ����� �������� - Bill.DlvrAddrCode / Bill.LocCode (Order, Recadv)
//		����� ����� �������� - Bill.DlvrAddr (Order, Recadv) (�����������)
//		����� ����� ������ � ��� - Bill.Amount (Order, Recadv)

//		�������� ������:
//		����� ��� - BRow.TTN (Recadv) (������������ � ��������� ���������)
//		�������� ������ - BRow.Barcode (Order, Recadv)
//		������� ������ - BRow.ArCode (Order)
//		�������� ������ - BRow.GoodsName (Order, Recadv)
//		���������� �����������/���������� ������ - BRow.Quantity (Order/Recadv)
//		���������� � �������� - BRow.UnitPerPack (Order, Recadv)
//		���� � ��� - BRow.Cost (Order, Recadv)
//		������ ��� - BRow.VatRate (Order, Recadv)
//		������� ��������� - BRow.UnitName (Order, Recadv)
//
// �������� ��������� � Papyrus
//		����� EnumExpReceipt() ��������� Sdr_DllImpExpReceipt (�� ������ � Papyrus � ��������������� ��� GUID � ������� EDI)
//
//
//	������
//  �������/�������� ��������� � ������� �� ������:
//		�� Papyrus - ��� �������� (ORDRSP, DESADV ��� APERAK)
//					 ������, �� ������� ���� ��������� ����������/�������. ���� ��� �� �������, ������� EDI
//						������ �� ������� � ��������� ������ ��� �������
//		�� Dll	   - � ������ ��������� ORDRSP ��� DESADV - ����������� ��� ��������� ��������� Sdr_Bill
//					 � ������ ��������� ������� - ����������� ��������� ���� � Sdr_Bill
//		�� Papyrus - ��������� � ������� ��������� ������, �� ������� �������� �������������/�����������/������. ����� ReplyImportObjStatus, ��������� Sdr_DllImpObjStatus
//		���� ����� ����� � Papyrus ����:
//			�� Dll - � ������ ��������� ORDRSP ��� DESADV ����������� ��������� Sdr_BRow
//
//	�������� ��������� � Papyrus
//	��������� ORDRSP, DESADV (�������������, �����������)
//		��������:
//			����� ��������� - Bill.Code (Ordrsp, Desadv)
//			���� ��������� - Bill.Date (Ordrsp, Desadv)
//			��������� ���� �������� - Bill.DueDate (Ordrsp, Desadv)
//			���� ������, �� ������� ������ ������ ����� - Bill.OrderDate (Ordrsp, Desadv)
//			����� ������, �� ������� ������ ������ ����� - Bill.OrderBillNo (Ordrsp, Desadv)
//			GLN ���������� - Bill.MainGLN (Ordrsp, Desadv)
//			GLN ���������� - Bill.MainGLN (Ordrsp, Desadv)
//			GLN ����������� - Bill.GLN (Ordrsp, Desadv)
//			GLN ����� �������� - Bill.DlvrAddrCode (Ordrsp, Desadv)
//			����� ��������� � ��� - Bill.Amount (Ordrsp, Desadv)
//
//		�������� ������:
//			������������/����������� ���������� ������ - BRow.Quantity (Ordrsp/Desadv)
//			���������� ������ � �������� - BRow.UnitPerPack (Ordrsp, Desadv)
//			���� � ��� - BRow.Cost (Ordrsp, Desadv)
//			��� �������������. ��� �������� - BRow.ManufINN (Desadv)
//			��� ����������� ��������� (���) - BRow.GoodKindCode (Desadv)
//			�������� ������ - BRow.Barcode (Ordrsp, Desadv)
//			�������� ������ - BRow.GoodsName (Ordrsp, Desadv)
//			������� ��������� - BRow.UnitName (Ordrsp, Desadv)
//			������ ��� � % - BRow.VatRate (Ordrsp, Desadv)
//			����� ��� - BRow.TTN (Desadv)
//
//	��������� �������
//		����� ��������� ������ - Bill.OrderBillNo
//		���� ��������� ������ - Bill.OrderDate
//		GLN ���������� - Bill.MainGLN
//		GLN ���������� - Bill.GLN
//		GLN ����� �������� - Bill.DlvrAddrCode
//		GLN ����� �������� - Bill.LocCode
//
//	������� ��������� �� Papyrus
//		������ ������ � Papyrus - 0 - ��� ������ ������, 1 - ���� ����� �����. ����� ReplyImportObjStatus, ��������� Sdr_DllImpObjStatus.
//
//
#include <slib.h>
#include <sxml.h>
#include <ppedi.h>
#include <ppsoapclient.h>
#include <Edisoft\edisoftEDIServiceSoapProxy.h>

#define EXPORT	extern "C" __declspec (dllexport)
#define THROWERR(expr,val) { if(!(expr)){ SetError(val, ""); goto __scatch; } }
#define THROWERR_STR(expr,val,str) { if(!(expr)){ SetError(val, str); goto __scatch; } }

#define UNIT_NAME_KG			"��"
#define UNIT_NAME_PIECE			"�����"
#define EMPTY_LISTMB_RESP		"<mailbox-response></mailbox-response>"

// ����� ��������� xml-������� web-�������
#define WEB_ELEMENT_NAME_TRACKID		"tracking-id"		// �� ��������� � �������
#define WEB_ELEMENT_NAME_PARTNER_ILN	"partner-iln"		// ������������� ��������
#define WEB_ELEMENT_NAME_DIRECTION		"direction"			// �������� ��� ��������� ��������
#define WEB_ELEMENT_NAME_DOCTYPE		"document-type"		// ��� ���������
#define WEB_ELEMENT_NAME_DOCVERSION		"document-version"	// ������ ������������
#define WEB_ELEMENT_NAME_DOCSTANDART	"document-standard"	// �������� ���������
#define WEB_ELEMENT_NAME_DOCTEST		"document-test"		// ������ ���������

// �������� ���������-�������������� xml-������� � �������� web-�������
#define WEB_ELEMENT_CODE_DIRECTION_IN	"IN"		// �������� ��������
#define WEB_ELEMENT_CODE_DIRECTION_OUT	"OUT"		// ��������� ��������
#define WEB_ELEMENT_CODE_TYPE_ORDERS	"ORDER" 	// �����
#define WEB_ELEMENT_CODE_TYPE_ORDRSP	"ORDRSP"	// ������������� ������
#define WEB_ELEMENT_CODE_TYPE_APERAK	"APERAK"	// ������ ���������
#define WEB_ELEMENT_CODE_TYPE_DESADV    "DESADV"    // ����������� �� �������� ������ ����������� (���������)
#define WEB_ELEMENT_CODE_TYPE_RECADV    "RECADV"    // ����������� � �������� ������
#define WEB_ELEMENT_CODE_STATUS_NEW		"N"			// ������ ��������� "�����"
#define WEB_ELEMENT_CODE_STATUS_READ	"R"			// ������ ��������� "��������"

// �������� ��������� ���������������
#define ELEMENT_NAME_DELIVERDATE        "ExpectedDeliveryDate"
#define ELEMENT_NAME_DOCNAMECODE        "DocumentNameCode"
#define ELEMENT_NAME_DOCPARTIES         "Document-Parties"
#define ELEMENT_NAME_ORDERPARTIES       "Order-Parties"
#define ELEMENT_NAME_BUYER              "Buyer"
#define ELEMENT_NAME_SELLER             "Seller"
#define ELEMENT_NAME_ORDERLINES         "Order-Lines"
#define ELEMENT_NAME_NAME               "Name"
#define ELEMENT_NAME_CITYNAME           "CityName"
#define ELEMENT_NAME_COUNTRY            "Country"
#define ELEMENT_NAME_TAXRATE            "TaxRate"
#define ELEMENT_NAME_ORDERSUMMARY       "Order-Summary"
#define ELEMENT_NAME_TOTALLINES         "TotalLines"
#define ELEMENT_NAME_TOTALNETAMOUNT     "TotalNetAmount"
#define ELEMENT_NAME_TOTALGROSSAMOUNT   "TotalGrossAmount"
#define ELEMENT_NAME_ORDRSPNUMBER       "OrderResponseNumber"
#define ELEMENT_NAME_ORDRSPDATE         "OrderResponseDate"
#define ELEMENT_NAME_BUYERORDERNUMBER   "BuyerOrderNumber"
#define ELEMENT_NAME_BUYERORDERDATE     "BuyerOrderDate"
#define ELEMENT_NAME_ITEMDISCR          "ItemDescription"
#define ELEMENT_NAME_ITEMSTATUS         "ItemStatus"
#define ELEMENT_NAME_ALLOCDELIVRD       "AllocatedDelivered"
#define ELEMENT_NAME_DESPATCHADVICEDATE "DespatchAdviceDate"
#define ELEMENT_NAME_QTTYDISPATCHED     "QuantityDespatched"
#define ELEMENT_NAME_ALCOCONTENT        "AlcoholContent"
#define ELEMENT_NAME_LICENSE            "License"
#define ELEMENT_NAME_SERIES             "Series"
#define ELEMENT_NAME_NUMBER             "Number"
#define ELEMENT_NAME_IISSUINAUTH        "IssuingAuthority"
#define ELEMENT_NAME_DATEOFISSUE        "DateOfIssue"
#define ELEMENT_NAME_EXPARITIONDATE     "ExpirationDate"
#define ELEMENT_NAME_PARTYTYPE          "Party-Type"
#define ELEMENT_NAME_TAXID              "TaxID"
#define ELEMENT_NAME_TAXRECREASONCODE   "TaxRegistrationReasonCode"
#define ELEMENT_NAME_CERTIFICATE        "Certificate"
#define ELEMENT_NAME_TYPE               "Type"
#define ELEMENT_NAME_DOCRECADV          "Document-ReceivingAdvice"
#define ELEMENT_NAME_DOCNUMBER          "DocumentNumber"
#define ELEMENT_NAME_DOCDATE            "DocumentDate"
#define ELEMENT_NAME_SYSTEMSGTEXT       "SystemMessageText"
#define ELEMENT_NAME_DOCUMENTID         "DocumentID"
#define ELEMENT_NAME_UNITPACKSZ         "UnitPacksize"
#define ELEMENT_NAME_UNITGROSSPRICE     "UnitGrossPrice"
#define ELEMENT_NAME_UNITNETPRICE		"UnitNetPrice"
#define ELEMENT_NAME_NETAMOUNT			"NetAmount"
#define ELEMENT_NAME_GROSSAMOUNT		"GrossAmount"

// �������� ��������� ���������������
#define ELEMENT_CODE_COUNTRY_RUS       "RU"     // ��� ������ - ������
#define ELEMENT_CODE_UNITOFMEASURE_PCE "PCE"    // ������� ��������� - �����
#define ELEMENT_CODE_UNITOFMEASURE_KGM "KGM"    // ������� ��������� - ���������
#define ELEMENT_CODE_UNITOFMEASURE_DPA "DPA"    // ������� ��������� - �����
#define ELEMENT_CODE_PARTYTYPE_MF      "MF"     // ��� ������� - �������������
#define ELEMENT_CODE_REFTYPE_YA1       "YA1"    // ��� ���� - ��� ���� ���������
#define ELEMENT_CODE_REFTYPE_ABT       "ABT"    // ��� ���� - ���
#define ELEMENT_CODE_REFTYPE_SER       "SER"    // ��� ���� - ����� (��� � �������������� ������ ����� �����)

// ���� ������ � ���������
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
#define IEERR_ONLYEXPMSGTYPES		19			// �������� �������� �������� ������ � ������ ������ ORDER � RECADV
#define IEERR_NOCONFIG				20			// �� ������� ������������ ��� ������� ���� ��������
#define IEERR_NOEXPPATH				21			// �� ���������� ���������� ��� ����� ��������
#define IEERR_PARSERESPONSEFAILD    22          // ������ ������� ������ �� ������� ����������
#define IEERR_TTNNEEDED				23			// �������� �� ������. ���������� ������� ���. ��������: %s// ������ ��� �� DESADV, � �� �� ��������� ��������� �������

// ���� ������ ���-�������
#define IEWEBERR_AUTH				1			// ������ ��������������
#define IEWEBERR_CORRELATION		2			// ������ �� �����������
#define IEWEBERR_EXTERNAL			3			// ������� ������
#define IEWEBERR_SERVER				4			// ��������� ������ �������
#define IEWEBERR_TIMELIMIT			5			// �������� ������� �� ���������� ������
#define IEWEBERR_WEBERR				6			// ������ Web
#define IEWEBERR_PARAMS				7			// ������������ ���������

// ���� ������ ��������� ��� ���-�����
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
#define SYSLOG_ORDERHEADER				"in OrderHeader()"
#define SYSLOG_RECADVHEADER				"in RecadvHeader()"
#define SYSLOG_DOCPARTIES				"in DocParties"
#define SYSLOG_GOODSLINES				"GoodsLines"
#define SYSLOG_ENDDOC					"EndDoc"
#define SYSLOG_SENDDOC					"in SendDoc()"
#define SYSLOG_RECEIVEDOC				"in ReceiveDoc()"
#define SYSLOG_PARSELISTMBRESP			"in ParseListMBResp()"
#define SYSLOG_PARSEFORDOCDATA			"in ParseForDocData()"
#define SYSLOG_PARSEFORGOODDATA			"in ParseForGoodData()"
#define SYSLOG_SETREADSTATUS			"in SetReadStatus()"
#define SYSLOG_SETNEWSTATUS				"in SetNewStatus()"
#define SYSLOG_LISTMESSAGEBOX			"in ListMessageBox()"
#define SYSLOG_PARSEAPERAKRESP			"in ParseAperakResp()"

#define LOG_NOINCOMDOC					"��� �������� ���������"
#define LOG_READSTATERR					"���� ��� ����������� ������� READ ��� ��������� "

class ExportCls;
class ImportCls;

int ErrorCode = 0;
int WebServcErrorCode = 0;
SString StrError = "";
SString LogName = "";
SString SysLogName = "";
static ExportCls * P_ExportCls = 0;
static ImportCls * P_ImportCls = 0;

int GetObjTypeBySymb(const char * pSymb, uint & rType);
void ProcessError(EDIServiceSoapProxy & rProxy);

int SetError(int errCode, const char * pStr = "")
{
	ErrorCode = errCode;
	StrError = pStr;
	return 0;
}

int SetWebServcError(int errCode, const char * pStrErr = 0)
{
	WebServcErrorCode = errCode;
	StrError = pStrErr;
	return 0;
}

void LogMessage(const char * pMsg);
void SysLogMessage(const char * pMsg);
void FormatLoginToGLN(const char * login, SString & rStr);
void FormatLoginToLogin(const char * login, SString & rStr);

struct ErrMessage {
	uint Id;
	const char * P_Msg;
};

struct WebServcErrMessage {
	uint Id;
	const char * P_Msg;
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

MessageTypeSymbols MsgSymbols[] = {
	{"ORDER",       PPEDIOP_ORDER},
	{"ORDRSP",      PPEDIOP_ORDERRSP},
	{"APERAK",      PPEDIOP_APERAK},
	{"DESADV",      PPEDIOP_DESADV},
	{"RECADV",      PPEDIOP_RECADV}
};

ErrMessage ErrMsg[] = {
	{IEERR_SYMBNOTFOUND,		"������ �� ������"},
	{IEERR_NODATA,				"������ �� ��������"},
	{IEERR_NOSESS,				"������ � ����� ������� ���"},
	{IEERR_ONLYBILLS,			"Dll ����� �������� ������ � �����������"},
	{IEERR_NOOBJID,				"������� � ����� ��������������� ���"},
	{IEERR_IMPEXPCLSNOTINTD,	"������ ��� �������/�������� �� ���������������"},
	{IEERR_WEBSERV�ERR,			"������ Web-�������: "},
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
	{IEERR_ONLYEXPMSGTYPES,		"�������� �������� �������� ������ � ������ ������ ORDER � RECADV"},
	{IEERR_NOCONFIG,			"�� ������� ������������ ��� ������� ���� ��������"},
	{IEERR_NOEXPPATH,			"�� ��������� ���������� ��� ����� ��������"},
	{IEERR_PARSERESPONSEFAILD,	"������ ������� ������ �� ������� ����������"},
	{IEERR_TTNNEEDED,			"�������� �� ������. ���������� ������� ���. ��������: "}
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
	SString str;
	int dd = 0, mm = 0, yy = 0, hh = 0, min = 0, ss = 0;
	LDATE date;
	LTIME time;
	if(file.IsValid()) {
		 getcurdate(&date);
		 getcurtime(&time);
		str.Z().Cat(date.day()).Dot().Cat(date.month()).Dot().Cat(date.year()).Space().Cat(time.hour()).
			CatChar(':').Cat(time.minut()).CatChar(':').Cat(time.sec()).Tab().Cat(pMsg).CR();
        file.WriteLine(str);
	}
}

void SysLogMessage(const char * pMsg)
{
	SFile file(SysLogName, SFile::mAppend);
	SString str;
	int dd = 0, mm = 0, yy = 0, hh = 0, min = 0, ss = 0;
	LDATE date;
	LTIME time;
	if(file.IsValid()) {
		 getcurdate(&date);
		 getcurtime(&time);
		str.Z().Cat(date.day()).Dot().Cat(date.month()).Dot().Cat(date.year()).Space().Cat(time.hour()).
			CatChar(':').Cat(time.minut()).CatChar(':').Cat(time.sec()).Tab().Cat(pMsg).CR();
        file.WriteLine(str);
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
	if((ErrorCode == IEERR_WEBSERV�ERR)) {
		for(size_t i = 0; i < SIZEOFARRAY(ErrMsg); i++) {
			if(WebServcErrMsg[i].Id == WebServcErrorCode) {
				(str).Cat(WebServcErrMsg[i].P_Msg).Space().Cat(StrError.Utf8ToChar());
				StrError.ToUtf8(); // ������� ���������, ��� ���� ���������� ������ ��� �������� ������, �� StrError ��� ����� ��������� � Char � � �������� ����� �����, � �� ���������
				break;
			}
		}
	}
	if((ErrorCode == IEERR_SOAP) || (ErrorCode == IEERR_IMPFILENOTFOUND) || (ErrorCode == IEERR_INVMESSAGEYTYPE) || (ErrorCode  == IEERR_NOCFGFORGLN))
		str.Cat(StrError);
	rMsg.Z().CopyFrom(str);
}

// ������������ ������ ������ � ������ GLN (�� ���� ��� EC �� �����)
// ��� ����� 4607806659997EC, � GLN ����������� ������ ���� 4607806659997
void FormatLoginToGLN(const char * login, SString & rStr)
{
	uint exit_while = 0;
	rStr.Z();
	if(login) {
		while(!exit_while) {
			if((*login == 0) || !isdec(*login))
				exit_while = 1;
			else {
				rStr.CatChar(*login);
				login ++;
			}
		}
	}
}

// ������� �� ������ ������ ������ �������
// ��� ����� 4607806659997EC_1, � ������ ���� 4607806659997��
void FormatLoginToLogin(const char * login, SString & rStr)
{
	uint exit_while = 0;
	char low_strip = '_';
	rStr.Z();
	if(login) {
		while(!exit_while) {
			if((*login == 0) || (*login == low_strip))
				exit_while = 1;
			else {
				rStr.CatChar(*login);
				login ++;
			}
		}
	}
}

// Returns:
//		0 - ������ �� ������
//		1 - ������ ������
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
// Returns:
//		0 - ������ �� ������
//		1 - ������ ������
int GetMsgTypeBySymb(const char * pSymb, uint & rType)
{
	for(size_t i = 0; i < SIZEOFARRAY(MsgSymbols); i++) {
		if(strcmp(MsgSymbols[i].P_Symb, pSymb) == 0) {
			rType = MsgSymbols[i].Type;
			return 1;
		}
	}
	return 0;
}

class Iterator {
public:
	void Init() { Count = 0; }
	const uint GetCount() { return Count; }
	void Next() { Count++; }
private:
	uint Count;
};
//
// �����, �� �������� ����������� ImportCls � ExportCls. �������� ����� ������, ��� ���� �������
//
class ImportExportCls {
public:
	ImportExportCls()
	{
	}
	~ImportExportCls()
	{
	}
	void CleanHeader()
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
	Sdr_ImpExpHeader Header;
	SString TTN;  // ��� RECADV. �������� �� ������ ���������� ������ ��������� ������������� ������� ������
					 // ��� DESADV. �������� �� ��������� ��������� � ������ �� ��� �������� ������
};

EXPORT int FinishImpExp();

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch(dwReason) {
		case DLL_PROCESS_ATTACH:
			{
				SString product_name;
				(product_name = "Papyrus-ImpExpEdisoft");
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
	void CreateFileName(uint num);
	int OrderHeader();
	int RecadvHeader();
	int DocParties();
	int GoodsLines(Sdr_BRow * pBRow);
	int EndDoc();
	int SendDoc();
	uint Id;					// ��, ������� Papyrus ����� ������������ ��� �� ������ ��������
	uint ObjId;					// �� ��������������� ������� (��������, ��� ������ ���������� ������� ��������� ������������� ���� ��)
	uint ObjType;				// ��� �������������� ��������
	uint MessageType;			// ��� ���������: ORDER, RECADV
	uint Inited;
	uint ReadReceiptNum;		// ���������� ����� ���������, ����������� �� ReceiptList
	double BillSumWithoutVat;	// ����� ��������� ��� ��� (�������� ������� ��������������, ��� � Sdr_Bill ���� ������� �� ����������)
	SString ExpFileName;
	xmlTextWriter * P_XmlWriter;
	Iterator Itr;
	SPathStruc PathStruct;
	Sdr_Bill Bill;
	// ������� �� ����� Sdr_DllImpExpReceipt
	TSCollection <Sdr_DllImpExpReceipt> ReceiptList; // ������ ��������� �� ������������ ����������
private:
	double TotalGoodsCount;     // � ���������� ���� ��������� ����� ���������� ������. �������� ������� ��������������
};

ExportCls::ExportCls() : Id(0), ObjId(0), ObjType(0), MessageType(0), Inited(0), ReadReceiptNum(0), BillSumWithoutVat(0.0), P_XmlWriter(0)
{
	LogName.Z();
	ErrorCode = 0;
	WebServcErrorCode = 0;
	TotalGoodsCount = 0;
	TTN.Z();
}

ExportCls::~ExportCls()
{
	P_XmlWriter = 0;
}

void ExportCls::CreateFileName(uint num)
{
	ExpFileName.Z().Cat(PathStruct.Drv).CatChar(':').Cat(PathStruct.Dir).Cat(PathStruct.Nam).Cat(num).Dot().Cat(PathStruct.Ext);
}
//
// ������ � ��������� ���������� � ���������
//
// ������ � ��������� ������
//
int ExportCls::OrderHeader()
{
	int    ok = 1;
	SString str, fmt;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	BillSumWithoutVat = 0.0;
	xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Document-Order");
	{
		SXml::WNode n_hdr(P_XmlWriter, "Order-Header");
		{
			n_hdr.PutInner("OrderNumber", (str = Bill.Code).ToUtf8()); // ����� ������
			{
				fmt.Z().Cat(Bill.Date.month());
				if(fmt.Len() == 1)
					fmt.PadLeft(1, '0');
				str.Z().Cat(Bill.Date.year()).CatChar('-').Cat(fmt);
				fmt.Z().Cat(Bill.Date.day());
				if(fmt.Len() == 1)
					fmt.PadLeft(1, '0');
                str.CatChar('-').Cat(fmt);
				n_hdr.PutInner("OrderDate", str); // ���� ������
			}
			n_hdr.PutInner("OrderCurrency", "RUR"); // ��� ������ (�����)
			{
				LDATE date;
				if(Bill.DueDate != ZERODATE)
					date = Bill.DueDate;
				else
					date = Bill.Date;
				fmt.Z().Cat(date.month());
				if(fmt.Len() == 1)
					fmt.PadLeft(1, '0');
				str.Z().Cat(date.year()).CatChar('-').Cat(fmt);
				fmt.Z().Cat(date.day());
				if(fmt.Len() == 1)
					fmt.PadLeft(1, '0');
				str.CatChar('-').Cat(fmt);
				n_hdr.PutInner(ELEMENT_NAME_DELIVERDATE, str); // �������� ���� �������� (�����������)
			}
			n_hdr.PutInner("DocumentFunctionCode", "9"); // ��� ������� ��������� (��������)
			n_hdr.PutInner(ELEMENT_NAME_DOCNAMECODE, "220"); // ��� ���� ��������� (�����)
		}
	}
	CATCH
		SysLogMessage(SYSLOG_ORDERHEADER);
		ok = 0;
	ENDCATCH;
	return ok;
}
//
// ������ � ��������� ��������� � ������� ������
//
int ExportCls::RecadvHeader()
{
	int    ok = 1;
	SString str, fmt;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	BillSumWithoutVat = 0.0;
	xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_DOCRECADV);
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"ReceivingAdvice-Header");
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"ReceivingAdviceNumber"); // ����� ��������� �����������
			//{
			//	SString str1;
			//	str1.Z().Cat(Bill.Code).ToUtf8();
			//	str.Z().Cat("<![CDATA[").Cat(str1).Cat("]]>"); // ������ ����������� <![CDATA[�����-�� ������]]>, ��� ��������� ����� ���� ������� �������������� �������� ��� �������
			//	xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
			//}
				xmlTextWriterWriteString(P_XmlWriter, (str = Bill.Code).ToUtf8().ucptr());
			xmlTextWriterEndElement(P_XmlWriter); // RECADVNUMBER
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"ReceivingAdviceDate"); // ���� �������� �����������
				fmt.Z().Cat(Bill.Date.month());
				if(fmt.Len() == 1)
					fmt.PadLeft(1, '0');
				str.Z().Cat(Bill.Date.year()).CatChar('-').Cat(fmt);
				fmt.Z().Cat(Bill.Date.day());
				if(fmt.Len() == 1)
					fmt.PadLeft(1, '0');
                str.CatChar('-').Cat(fmt);
				xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
			xmlTextWriterEndElement(P_XmlWriter); // RECADVDATE
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"GoodsReceiptDate"); // ����������� ���� ������ ������
				fmt.Z().Cat(Bill.Date.month());
				if(fmt.Len() == 1)
					fmt.PadLeft(1, '0');
				str.Z().Cat(Bill.Date.year()).CatChar('-').Cat(fmt);
				fmt.Z().Cat(Bill.Date.day());
				if(fmt.Len() == 1)
					fmt.PadLeft(1, '0');
                str.CatChar('-').Cat(fmt);
				xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
			xmlTextWriterEndElement(P_XmlWriter); // GOODSRECDATE
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_BUYERORDERNUMBER); // ����� ������
				xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.OrderBillNo); // ��� ����� ������ ��������
			xmlTextWriterEndElement(P_XmlWriter); // BUYERORDERNUMBER
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"DespatchNumber"); // ����� ���
				xmlTextWriterWriteString(P_XmlWriter, TTN.Transf(CTRANSF_INNER_TO_UTF8).ucptr());
			xmlTextWriterEndElement(P_XmlWriter); // DESPATCHNUMBER
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"DocumentFunctionCode"); // ��� ������� ���������
				xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"9"); // ��������
			xmlTextWriterEndElement(P_XmlWriter); // DOCFUNCTYPE
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_DOCNAMECODE); // ��� ���� ���������
				xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"351"); // ����������� � ��������
			xmlTextWriterEndElement(P_XmlWriter); // DOCNAMECODE
		xmlTextWriterEndElement(P_XmlWriter); // DOCORDERHDR

	CATCH
		SysLogMessage(SYSLOG_RECADVHEADER);
		ok = 0;
	ENDCATCH;
	return ok;
}

// �������� ���������� ������
// ����������: Sender = Buyer (����������)
//             Reciever = Seller (���������)
//
int ExportCls::DocParties()
{
	int    ok = 1;
	SString str, sender_gln, login;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
		{
			SXml::WNode n_dp(P_XmlWriter, ELEMENT_NAME_DOCPARTIES);
			{
				{
					SXml::WNode n_sender(P_XmlWriter, "Sender"); // �����������
					// � EDISoft GLN ����������� ��������� � ������� �� �������� ��������
					// ����� ����� �����, ����� � ���������� � �������� ������ ��� ������ �� ������, ��� � ��������
					// � GLN ����������� ������ GLN ������, �� ���� ����� ��������
					FormatLoginToGLN(Header.EdiLogin, sender_gln.Z());
					n_sender.PutInner("ILN", sender_gln); // GLN �����������
					//// ���� GLN ������ �����, �� ����� GLN ������� ����������� (�������� ��-�������, ��� ����� ����� �������� ��� ��������� �������� ����������)
					//if(!isempty(Bill.AgentGLN))
					//	xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.AgentGLN);
					//else
					//	xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.MainGLN);
				}
				{
					SXml::WNode n_rcvr(P_XmlWriter, "Receiver"); // ����������
					n_rcvr.PutInner("ILN", Bill.GLN); // GLN ����������
				}
			}
		}
		if(MessageType == PPEDIOP_ORDER)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_ORDERPARTIES);
		else if(MessageType == PPEDIOP_RECADV)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"ReceivingAdvice-Parties");
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_BUYER); // ����������
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"ILN"); // GLN ����������
					FormatLoginToGLN(Header.EdiLogin, login.Z());
					xmlTextWriterWriteString(P_XmlWriter, login.ucptr());
					//// ���� GLN ������ �����, �� ����� GLN ������� �����������
					//if(!isempty(Bill.AgentGLN))
					//	xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.AgentGLN);
					//else
					//	xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.MainGLN);
				xmlTextWriterEndElement(P_XmlWriter); // ILN
				//xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_NAME); // �������� ���������� (�����������)
				//xmlTextWriterEndElement(P_XmlWriter); // NAME
			xmlTextWriterEndElement(P_XmlWriter); // BUYER
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SELLER); // ��������
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"ILN"); // GLN ��������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.GLN);
				xmlTextWriterEndElement(P_XmlWriter); // ILN
				//xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_NAME); // �������� �������� (�����������)
				//	xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.ContragName); // ����� ����, ��� ����
				//xmlTextWriterEndElement(P_XmlWriter); // NAME
			xmlTextWriterEndElement(P_XmlWriter); // SELLER
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"DeliveryPoint"); // ����� ��������
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"ILN"); // GLN ����� ��������
					if(!isempty(Bill.DlvrAddrCode))
						str.Z().Cat(Bill.DlvrAddrCode);
					else
						str.Z().Cat(Bill.LocCode);
					xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
				xmlTextWriterEndElement(P_XmlWriter); // ILN
				//xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_NAME); // �������� ����� �������� (�����������)
				//xmlTextWriterEndElement(P_XmlWriter); // NAME
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"StreetAndNumber"); // ����� ����� �������� (����� � ���) (�����������)
					(str = Bill.DlvrAddr).ToUtf8();
					xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
				xmlTextWriterEndElement(P_XmlWriter); // ADDRESS
				//xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_CITYNAME); // ����� ����� �������� (�����������)
				//xmlTextWriterEndElement(P_XmlWriter); // CITYNAME
				//xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"PostalCode"); // �������� ������ ����� �������� (�����������)
				//xmlTextWriterEndElement(P_XmlWriter); // POSTALCODE
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_COUNTRY); // ������ ����� �������� (�����������)
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_COUNTRY_RUS); // ������
				xmlTextWriterEndElement(P_XmlWriter); // COUNTRY
			xmlTextWriterEndElement(P_XmlWriter); // DELIVERYPOINT
		xmlTextWriterEndElement(P_XmlWriter); // ORDERPARTIES/RECADVPARTIES
		if(MessageType == PPEDIOP_ORDER)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_ORDERLINES);
		else if(MessageType == PPEDIOP_RECADV)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"ReceivingAdvice-Lines");

	CATCH
		SysLogMessage(SYSLOG_DOCPARTIES);
		ok = 0;
	ENDCATCH;
	return ok;
}

int ExportCls::GoodsLines(Sdr_BRow * pBRow)
{
	int    ok = 1;
	SString str;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	THROWERR(pBRow, IEERR_NODATA);
	//
	// ��� RECADV
	// ���� ��� �� ������ ��� � ������ ������, �� �������� �� �������. ��� ������ ��������� ������ ����� ������ ����� ���������,
	// ������� � ���� ������� ������������ ������ ��� ������� ���
	//
	if(MessageType == PPEDIOP_RECADV)
		THROWERR_STR(!isempty(pBRow->TTN), IEERR_TTNNEEDED, Bill.Code);
	// ����� � ���������� ������ ����� ���� ���.
	// �� ������� pBRow->TTN ����� ����������, ��� ��� ���� ����������� �������, ��� ��� � ������� ����������
	// ���� �������� RECADV � DESADV ������ �� ����� ���������.
	// ��� � ���� ����� ����������, ��� ������ �� ������ DESADV
	if(MessageType == PPEDIOP_RECADV && TTN.Empty()) {
		TTN = pBRow->TTN;
		THROW(P_ExportCls->RecadvHeader());
		THROW(P_ExportCls->DocParties());
	}
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Line");
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Line-Item");
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"LineNumber"); // ���������� ����� ������ � ������
						xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(Itr.GetCount() + 1).ucptr());
					xmlTextWriterEndElement(P_XmlWriter); // LINENUMBER
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"EAN"); // �������� ������
						xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(pBRow->Barcode).ucptr());
					xmlTextWriterEndElement(P_XmlWriter); // EAN
					if(!isempty(pBRow->ArCode)) {
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"SupplierItemCode"); // ������� ������ ����������
							xmlTextWriterWriteString(P_XmlWriter, (str = pBRow->ArCode).Transf(CTRANSF_INNER_TO_UTF8).ucptr());
						xmlTextWriterEndElement(P_XmlWriter); // SUPPITEMCODE
					}
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_ITEMDISCR); // �������� ������ (���) (�����������)
						{
							str.Z().Cat(pBRow->GoodsName).ToUtf8(); // ��������� ������� ��� ���������
							xmlTextWriterWriteString(P_XmlWriter, SXml::WNode::CDATA(str).ucptr()); // ������������ ������
						}
					xmlTextWriterEndElement(P_XmlWriter); // ITEMDISCR
					if(MessageType == PPEDIOP_ORDER)
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"OrderedQuantity"); // ���������� ����������� ������
					else if(MessageType == PPEDIOP_RECADV)
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"QuantityReceived"); // ���������� ���������� ������
							// � ���� ���������������� ����� ��������� ���������� ��������, ��� ���� ��� ����� ������� ���������
							//if(pBRow->UnitPerPack) {
							//	double qtty_pack = pBRow->Quantity / pBRow->UnitPerPack;
							//	str.Z().Cat(qtty_pack);
							//	str.Printf("%.3f", qtty_pack); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
							//	TotalGoodsCount += qtty_pack;
							//}
							//else {
								str.Z().Cat(pBRow->Quantity);
								TotalGoodsCount += pBRow->Quantity;
							//}
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); // ORDEREDQTTY/RECEIVEDQTTY
					if(pBRow->UnitPerPack) {
						if(MessageType == PPEDIOP_ORDER)
							xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"OrderedUnitPacksize"); // ���������� � �������� (����������)
						else if(MessageType == PPEDIOP_RECADV)
							xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_UNITPACKSZ); // ���������� � �������� (���������)
							//str.Z().Cat(pBRow->UnitPerPack);
							//xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"1");
						xmlTextWriterEndElement(P_XmlWriter); // ORDEREDPACKSZ/UNITPACKSZ
					}

					// � ORDER � RECADV ���������� ������� ���������� ��������� ���� �����
					// ���� � ORDER ���� taxrate � ����� �� ������, � � RECADV - ���
					if(MessageType == PPEDIOP_ORDER) {
						{
							SXml::WNode n_uom(P_XmlWriter, "UnitOfMeasure"); // ������� ���������
							{
								(str = pBRow->UnitName).ToUpper1251();
								if(str.CmpNC(UNIT_NAME_KG) == 0)
									xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_UNITOFMEASURE_KGM); // ����������
								else {
									// ���� ����� � ���������, �� ����� ��������
									//if(pBRow->UnitPerPack)
									//	xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_UNITOFMEASURE_DPA); // ��������
									// ����� �����
									//else
										xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_UNITOFMEASURE_PCE); // ��������� ��������
								}
							}
						}
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"OrderedUnitNetPrice"); // ���� ��� ��� (�����)
							//double cost = pBRow->Cost - (pBRow->Cost / (pBRow->VatRate + 100) * 100);
							double cost = (pBRow->Cost / (pBRow->VatRate + 100) * 100);
							str.Z().Printf("%.3f", cost); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); // ORDEREDNETPRICE
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"OrderedUnitGrossPrice"); // ���� � ��� (�����)
							str.Z().Printf("%.3f", pBRow->Cost); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); // ORDEREDGROSSPRICE
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"OrderedNetAmount"); // ����� ��� ���
							//double amount = (pBRow->Cost - (pBRow->Cost / (pBRow->VatRate + 100) * 100)) * pBRow->Quantity;
							double amount = ((pBRow->Cost / (pBRow->VatRate + 100) * 100)) * pBRow->Quantity;
							str.Z().Printf("%.3f", amount); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
							BillSumWithoutVat += str.ToReal();
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); // ORDEREDNETAMOUNT
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"OrderedGrossAmount"); // ����� � ���
							str.Z().Printf("%.3f", pBRow->Cost * pBRow->Quantity); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); // ORDEREDGROSSAMOUNT
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_TAXRATE); // ��������� ������ % (�����������)
							str.Z().Printf("%.3f", pBRow->VatRate); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr()); // ������ ���
						xmlTextWriterEndElement(P_XmlWriter); // TAXRATE
					}
					else if(MessageType == PPEDIOP_RECADV) {
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_UNITGROSSPRICE); // ���� � ��� (������)
							str.Z().Printf("%.3f", pBRow->Cost); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); // UNITGROSSPRICE
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_UNITNETPRICE); // ���� ��� ��� (������)
							//double price = pBRow->Cost - (pBRow->Cost / (pBRow->VatRate + 100) * 100);
							double price = (pBRow->Cost / (pBRow->VatRate + 100) * 100);
							str.Z().Printf("%.3f", price); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); // UNITNETPRICE
						{
							SXml::WNode n_uom(P_XmlWriter, "UnitOfMeasure"); // ������� ���������
							{
								(str = pBRow->UnitName).Utf8ToChar().ToUpper().Transf(CTRANSF_INNER_TO_OUTER);
								if(str.CmpNC(UNIT_NAME_KG) == 0)
									xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_UNITOFMEASURE_KGM); // ����������
								else {
									// ���� ����� � ���������, �� ����� ��������
									//if(pBRow->UnitPerPack)
									//	xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_UNITOFMEASURE_DPA); // ��������
									// ����� �����
									//else
									xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_UNITOFMEASURE_PCE); // ��������� ��������
								}
							}
						}

						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_NETAMOUNT); // ����� ��� ���
							//double amount = (pBRow->Cost - (pBRow->Cost / (pBRow->VatRate + 100) * 100)) * pBRow->Quantity;
							double amount = ((pBRow->Cost / (pBRow->VatRate + 100) * 100)) * pBRow->Quantity;
							str.Z().Printf("%.3f", amount); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
							BillSumWithoutVat += str.ToReal();
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); // NETAMOUNT
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_GROSSAMOUNT); // ����� � ���
							str.Z().Printf("%.3f", pBRow->Cost * pBRow->Quantity); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); // GROSSAMOUNT
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_TAXRATE); // ��������� ������ % (�����������)
							str.Z().Printf("%.3f", pBRow->VatRate); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr()); // ������ ���
						xmlTextWriterEndElement(P_XmlWriter); // TAXRATE
					}
				xmlTextWriterEndElement(P_XmlWriter); // LINEITEM
				// @vmiller {
				// ��������� �������������� ����������
				//if(!isempty(pBRow->LotManuf)) {
				//	xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Line-Parties");
				//		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Line-Party");
				//			// �������������
				//			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_PARTYTYPE);
				//				xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_PARTYTYPE_MF); // ��� ���� - �������������
				//			xmlTextWriterEndElement(P_XmlWriter); // PARTYTYPE
				//			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_NAME);
				//				SString str1;
				//				str1.Z().Cat(pBRow->LotManuf).ToUtf8();
				//				str.Z().Cat("<![CDATA[").Cat(str1).Cat("]]>");
				//				xmlTextWriterWriteString(P_XmlWriter, str.ucptr()); // ������������ �������������
				//			xmlTextWriterEndElement(P_XmlWriter); // NAME
				//		xmlTextWriterEndElement(P_XmlWriter); // LINEPARTY
				//	xmlTextWriterEndElement(P_XmlWriter); // LINEPARTIES
				//}
				if(pBRow->GoodKindCode || !isempty(pBRow->CLB) || !isempty(pBRow->Serial)) {
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Line-Reference");
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Reference-Elements");
							// ��� ���� ���������
							if(pBRow->GoodKindCode) {
								SXml::WNode n_ref(P_XmlWriter, "Reference-Element");
								{
									xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Reference-Type");
										xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_REFTYPE_YA1); // ��� ���� - ��� ���� ���������
									xmlTextWriterEndElement(P_XmlWriter); // REFTYPE
									xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Reference-Id");
										xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(pBRow->GoodKindCode).ucptr()); // ��� ���� ���������
									xmlTextWriterEndElement(P_XmlWriter); // REFID
								}
							}
							// ���
							if(!isempty(pBRow->CLB)) {
								SXml::WNode n_ref(P_XmlWriter, "Reference-Element");
								{
									xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Reference-Type");
										xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_REFTYPE_ABT); // ��� ���� - ���
									xmlTextWriterEndElement(P_XmlWriter); // REFTYPE
									xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Reference-Id");
										xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(pBRow->CLB).ucptr()); // ���
									xmlTextWriterEndElement(P_XmlWriter); // REFID
								}
							}
							// �����
							if(!isempty(pBRow->Serial)) {
								SXml::WNode n_ref(P_XmlWriter, "Reference-Element");
								{
									xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Reference-Type");
										xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_REFTYPE_SER); // ��� ���� - �����
									xmlTextWriterEndElement(P_XmlWriter); // REFTYPE
									xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"Reference-Id");
										xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(pBRow->Serial).ucptr()); // �����
									xmlTextWriterEndElement(P_XmlWriter); // REFID
								}
							}
						xmlTextWriterEndElement(P_XmlWriter); // REFELEMENTS
					xmlTextWriterEndElement(P_XmlWriter); // LINEREFERENCE
				}
				// } @vmiller
			xmlTextWriterEndElement(P_XmlWriter); // LINE


	CATCH
		SysLogMessage(SYSLOG_GOODSLINES);
		ok = 0;
		if((MessageType == PPEDIOP_RECADV) && (!pBRow || isempty(pBRow->TTN))) // ����� ��������� ��������� ��������� � ��������� ���������� ��� ���������
			P_XmlWriter = 0;
	ENDCATCH;
	return ok;
}

// ��������� ������������ ��������� (������ ��� ��������� � ������� ������)
int ExportCls::EndDoc()
{
	int    ok = 1;
	SString str;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
		xmlTextWriterEndElement(P_XmlWriter); // ORDERLINES/RECADVPARTIES
		if(MessageType == PPEDIOP_ORDER)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_ORDERSUMMARY);
		else if(MessageType == PPEDIOP_RECADV)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"ReceivingAdvice-Summary");
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_TOTALLINES); // ����� ���������� �������� �����
				xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(Itr.GetCount()).ucptr());
			xmlTextWriterEndElement(P_XmlWriter); // TOTALLINES
			if(MessageType == PPEDIOP_ORDER)
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"TotalOrderedAmount"); // ����� ���������� ����������� ������
			else if(MessageType == PPEDIOP_RECADV)
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"TotalGoodsReceiptAmount"); // ����� ���������� ��������� ������
			xmlTextWriterWriteString(P_XmlWriter, str.Z().Printf("%.3f", TotalGoodsCount).ucptr()); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
			xmlTextWriterEndElement(P_XmlWriter); // TOTALORDEREDAMT/TOTALGOODSRECAMT

			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_TOTALNETAMOUNT); // ����� ����� ������ ��� ���
				xmlTextWriterWriteString(P_XmlWriter, str.Z().Printf("%.3f", BillSumWithoutVat).ucptr()); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
			xmlTextWriterEndElement(P_XmlWriter); // TOTALNETAMOUNT
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_TOTALGROSSAMOUNT); // ����� ����� ������ � ���
				str.Z().Printf("%.3f", Bill.Amount); // ����� ������ ����� ������� ������ ���� �� ������ 3-�
				xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
			xmlTextWriterEndElement(P_XmlWriter); // TOTALGROSSAMOUNT
		xmlTextWriterEndElement(P_XmlWriter); // ORDERSUMMARY/RECADVSUMMARY
	xmlTextWriterEndElement(P_XmlWriter); // DOCORDERS/DOCRECADV

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
	Sdr_DllImpExpReceipt * p_exp_rcpt = 0;
	SString str, login;
	EDIServiceSoapProxy proxy(SOAP_XML_INDENT|SOAP_XML_IGNORENS);
	gSoapClientInit(&proxy, 0, 0);
	file.CalcSize(&file_size);

	// ���� ������������ ������ ��� ����������� �������� � ���� ���������
	/*if(MessageType == PPEDIOP_ORDER) {
		for(pos = 0; pos < RlnCfgList.getCount(); pos++) {
			if(!RlnCfgList.at(pos).SuppGLN.CmpNC(Bill.GLN) &&
				!RlnCfgList.at(pos).Direction.CmpNC(WEB_ELEMENT_CODE_DIRECTION_OUT) &&
				!RlnCfgList.at(pos).DocType.CmpNC(WEB_ELEMENT_CODE_TYPE_ORDERS))
				break;
		}
	}
	else if(MessageType == PPEDIOP_RECADV) {
		for(pos = 0; pos < RlnCfgList.getCount(); pos++) {
			if(!RlnCfgList.at(pos).SuppGLN.CmpNC(Bill.GLN) &&
				!RlnCfgList.at(pos).Direction.CmpNC(WEB_ELEMENT_CODE_DIRECTION_OUT) &&
				!RlnCfgList.at(pos).DocType.CmpNC(WEB_ELEMENT_CODE_TYPE_RECADV))
				break;
		}
	}*/
	//if(pos < RlnCfgList.getCount()) { // @vmiller comment
		FormatLoginToLogin(Header.EdiLogin, login.Z()); // �� ������������
		param.Name = (char *)(const char *)login;
		//param.Name = Header.EdiLogin;			// �� ������������
		param.Password = Header.EdiPassword;	// ������
		param.PartnerIln = Bill.GLN;			// �� ��������, �������� ���������� ��������
		//param.DocumentType = (char *)(const char *)RlnCfgList.at(pos).DocType;	// ��� ���������
		if(MessageType == PPEDIOP_ORDER) // ��� ���������
			param.DocumentType = WEB_ELEMENT_CODE_TYPE_ORDERS;
		else if(MessageType == PPEDIOP_RECADV)
			param.DocumentType = WEB_ELEMENT_CODE_TYPE_RECADV;
		//param.DocumentVersion = (char *)(const char *)RlnCfgList.at(pos).DocVersion;	// ������ ������������
		//param.DocumentStandard = (char *)(const char *)RlnCfgList.at(pos).DocStandard;	// �������� ���������
		//param.DocumentTest = (char *)(const char *)RlnCfgList.at(pos).DocTest;			// ������ ���������
		//param.ControlNumber = (char *)Bill.Code; // ����������� ����� ��������� (������ ����� ���������)
		buf = new char[(size_t)file_size + 1];
		memzero(buf, (size_t)file_size + 1);
		file.ReadLine(str.Z()); // ���������� ������ ������ <?xml version="1.0" encoding="UTF-8" ?>
		file.Seek((long)str.Len());
		file.ReadV(buf, (size_t)file_size + 1);
		param.DocumentContent = buf; // ���������� ���������
		param.Timeout = 10000;		// ������� �� ���������� ������ ������ (��) (�������� �������������)
		if((r = proxy.Send(&param, &resp)) == SOAP_OK) {
			if(atoi(resp.SendResult->Res) == 0) {
				p_exp_rcpt = new Sdr_DllImpExpReceipt;
				memzero(p_exp_rcpt, sizeof(Sdr_DllImpExpReceipt));
				p_exp_rcpt->ID = atol(Bill.ID);
				STRNSCPY(p_exp_rcpt->ReceiptNumber, resp.SendResult->Cnt); // �� ��������� � ������� ����������
				ReceiptList.insert(p_exp_rcpt);
			}
			else {
				SetError(IEERR_WEBSERV�ERR);
				SetWebServcError(atoi((const char *)resp.SendResult->Res), (const char *)resp.SendResult->Cnt);
				ok = 0;
			}
		}
		else {
			ProcessError(proxy);
			ok = 0;
		}
	// @vmiller comment
	/*}
	else {
		str.Z().Cat(Bill.GLN).CR().Cat("�������� ").Cat(Bill.Code);
		SetError(IEERR_NOCFGFORGLN, str);
		ok = -1;
	}*/
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
	SFile  log_file;
	SPathStruc log_path;
	SString str;
	SETIFZ(P_ExportCls, new ExportCls);
	if(P_ExportCls && !P_ExportCls->Inited) {
		if(pExpHeader)
			P_ExportCls->Header = *static_cast<const Sdr_ImpExpHeader *>(pExpHeader);
		if(!isempty(pOutFileName)) {
			P_ExportCls->PathStruct.Split(pOutFileName);
			P_ExportCls->PathStruct.Nam.SetIfEmpty("edisoft_export_");
			P_ExportCls->PathStruct.Ext.SetIfEmpty("xml");
		}
		else {
			SLS.Init("Papyrus");
			str = SLS.GetExePath();
			P_ExportCls->PathStruct.Split(str);
			P_ExportCls->PathStruct.Dir.ReplaceStr("\\bin", "\\out", 1);
			P_ExportCls->PathStruct.Nam = "edisoft_export_";
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
	//THROWERR(GetMsgTypeBySymb(pMsgType, P_ExportCls->MessageType), IEERR_MSGSYMBNOTFOUND);
	THROWERR_STR(GetMsgTypeBySymb(pMsgType, P_ExportCls->MessageType), IEERR_MSGSYMBNOTFOUND, pMsgType);
	THROWERR(oneof2(P_ExportCls->MessageType, PPEDIOP_ORDER, PPEDIOP_RECADV), IEERR_ONLYEXPMSGTYPES);
	THROWERR(GetObjTypeBySymb(pObjTypeSymb, P_ExportCls->ObjType), IEERR_SYMBNOTFOUND);
	THROWERR(P_ExportCls->ObjType == objBill, IEERR_ONLYBILLS);
	// ��������� ���������� ��������
	if(P_ExportCls->P_XmlWriter) {
		THROW(P_ExportCls->EndDoc());
		xmlTextWriterEndDocument(P_ExportCls->P_XmlWriter);
		xmlFreeTextWriter(P_ExportCls->P_XmlWriter);
		P_ExportCls->P_XmlWriter = 0;
		// ���������� ����, �������������� � ������� ���. ��� ����� � ����� ��������� ��� �� ������ ������� ����� ��������
		//THROW(P_ExportCls->SendDoc());
		// ������ ��� THROW, ����� �������� ��������� ���������� �� �����������, �� ������ ��� ����� ��������
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
	xmlTextWriterSetIndentString(P_ExportCls->P_XmlWriter, reinterpret_cast<const xmlChar *>("\t"));
	// UTF-8 - �� ���������� ����������
	xmlTextWriterStartDocument(P_ExportCls->P_XmlWriter, 0, "UTF-8", 0);
	P_ExportCls->Bill = *(Sdr_Bill *)pObjData;
	if(P_ExportCls->MessageType == PPEDIOP_ORDER) {
		THROW(P_ExportCls->OrderHeader())
		THROW(P_ExportCls->DocParties());
	}
	// @vmiller comment {
	// �������� ��� ����� � NextExportObjIter() ��-�� ����, ��� ���� ��� ���������� � ��������� ����� ����������, � �� �����
	// ����������, � � RECADV ���� �������� ����������� � ���������.
	//else if(P_ExportCls->MessageType == PPEDIOP_RECADV) {
	//	THROW(P_ExportCls->RecadvHeader());
	//	THROW(P_ExportCls->DocParties());
	// } @vmiller comment
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
			// ������ ��� THROW, ����� � ������������ ���������� ������������ ����, �� ������ ��������
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
			*static_cast<Sdr_DllImpExpReceipt *>(pReceipt) = *P_ExportCls->ReceiptList.at(P_ExportCls->ReadReceiptNum++);
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
//
// ������
//
//
struct AperakInfoSt {
	AperakInfoSt()
	{
		Clear();
	}
	void Clear()
	{
		DocID.Z();
		OrderNum.Z();
		Code.Z();
		Msg.Z();
		SupplGLN.Z();
		BuyerGLN.Z();
		AddrGLN.Z();
		OrderDate = ZERODATE;
	}
	SString DocID;      // �� ��������� ������� (����� ��������� ��� ������� read)
	SString OrderNum;     // ����� ��������� ������
	SString Code;		// ��� ������� (� ������ ������ - 27)
	SString Msg;        // �������� ������ ��� �������
	SString SupplGLN;   // GLN ����������
	SString BuyerGLN;   // GLN ����������
	SString AddrGLN;    // GLN ������ ��������
	LDATE	OrderDate;    // ���� ���������
};

class ImportCls : public ImportExportCls {
public:
	ImportCls();
	~ImportCls();
	void CreateFileName(uint num);
	int ReceiveDoc();
	int ListMessageBox(SString & rResp);
	int ParseForDocData(Sdr_Bill * pBill);
	int ParseForGoodsData(Sdr_BRow * pBRow);
	int ParseAperakResp(const char * pResp);
	int SetReadStatus(SString & trackID);
	int SetNewStatus(SString & rErrTrackIdList);
	int GoodsCount;				// ����� ������� � ���������
	uint Id;					// ��, ������� Papyrus ����� ������������ ��� �� ������ ��������
	uint ObjId;					// �� �������������� ������� (��������, ���� ��� ����� ����������, �� � ������� ��������� ���� ��)
	uint ObjType;				// ��� ������������� ��������
	uint MessageType;			// ��� ��������: ORDRESP, APERAK
	uint Inited;
	uint IncomMessagesCounter;	// ������� �������� ���������. ����� � ������, ���� ���� �������� ���������, �� ��� ������������� ��� ������ ���������.
								// � ���� ������ ReceiveDoc() �� ������ ��������� LOG_NOINCOMDOC
	double BillSumWithoutVat;	// � Sdr_Bill ���� ������� �� ����������, ������� �������� ����� ��������� Papyrus'� ��� ��������
	SString ImpFileName;
	SString LastTrackId;		// GUID ���������� ������������ ��������� � ������� EDI
	Iterator Itr;
	SPathStruc PathStruct;
	AperakInfoSt AperakInfo;
	StrAssocArray TrackIds;		// ������ GUID ����������� ���������� � ������� EDI, ������� ������� ���� �������� (�� ���� ������������� ��� ������� ��������)
private:
	int ParseListMBResp(const char * pResp, SString & rPartnerIln, SString & rDocId);

	int HasSellerCertificate;  // ����� ��� ������ �������� ����� �� ��������� ������ ��� ���� � �������� �� ������� �������� ��� ����������.
							   // ����������� ��� ������ �������� �����, ������ ��� ���������, ������� ���� ���������, ��������� � Sdr_BRow
};

ImportCls::ImportCls() : GoodsCount(0), Id(0), ObjId(0), ObjType(0), MessageType(0), Inited(0), IncomMessagesCounter(0), BillSumWithoutVat(0.0)
{
	ErrorCode = 0;
	WebServcErrorCode = 0;
	HasSellerCertificate = 0;
}

ImportCls::~ImportCls()
{
}

void ImportCls::CreateFileName(uint num)
{
	ImpFileName.Z().Cat(PathStruct.Drv).CatChar(':').Cat(PathStruct.Dir).Cat(PathStruct.Nam).Cat(num).Dot().Cat(PathStruct.Ext);
}

EXPORT int InitImport(void * pImpHeader, const char * pInputFileName, int * pId)
{
	int    ok = 1;
	SPathStruc log_path;
	SString str;
	SETIFZ(P_ImportCls, new ImportCls);
	if(P_ImportCls && !P_ImportCls->Inited) {
		if(pImpHeader)
			P_ImportCls->Header = *static_cast<const Sdr_ImpExpHeader *>(pImpHeader);
		if(!isempty(pInputFileName)) {
			P_ImportCls->PathStruct.Split(pInputFileName);
			if(P_ImportCls->PathStruct.Nam.Empty())
				(P_ImportCls->PathStruct.Nam = "edisoft_import_").Cat(P_ImportCls->ObjId);
			P_ImportCls->PathStruct.Ext.SetIfEmpty("xml");
		}
		else {
			//char fname[256];
			//GetModuleFileName(NULL, fname, sizeof(fname));
			SString module_file_name;
			SSystem::SGetModuleFileName(0, module_file_name);
			P_ImportCls->PathStruct.Split(module_file_name);
			P_ImportCls->PathStruct.Dir.ReplaceStr("\\bin", "\\in", 1);
			(P_ImportCls->PathStruct.Nam = "edisoft_import_").Cat(P_ImportCls->ObjId);
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

EXPORT int GetImportObj(uint idSess, const char * pObjTypeSymb, void * pObjData, int * pObjId, const char * pMsgType = 0)
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
	// ������� ��� ���������
	THROWERR(pMsgType, IEERR_MSGSYMBNOTFOUND);
	THROWERR(GetMsgTypeBySymb(pMsgType, P_ImportCls->MessageType), IEERR_MSGSYMBNOTFOUND);
	THROWERR(oneof3(P_ImportCls->MessageType, PPEDIOP_ORDERRSP, PPEDIOP_APERAK, PPEDIOP_DESADV), IEERR_ONLYIMPMSGTYPES);
	//
	// �������� ��������
	//
	P_ImportCls->CreateFileName(P_ImportCls->ObjId);
	THROW(r = P_ImportCls->ReceiveDoc());
	if(r == -1)
		ok = -1;
	else {
		if(((P_ImportCls->MessageType == PPEDIOP_ORDERRSP) || (P_ImportCls->MessageType == PPEDIOP_DESADV)) && (r == 1)) // ��� ���� r == -2, �� ��� �� ���� �������� ��������� ����, ��� ��� ���
			// ������ �������� � ��������� Sdr_Bill
			THROW(P_ImportCls->ParseForDocData((Sdr_Bill *)pObjData))
		else if(P_ImportCls->MessageType == PPEDIOP_APERAK) {
			// ����� ��������� pObjData �� ��������� P_ImportCls->AperakInfo
			// ��� ����� ������� ����� ��������� � GLN ������ ��������, ���� ���������. ����� ���������� ��� ����������� �������������� ������� ����������� ��������
			// GLN ����������
			P_ImportCls->AperakInfo.OrderNum.CopyTo(((Sdr_Bill *)pObjData)->OrderBillNo, sizeof(((Sdr_Bill *)pObjData)->OrderBillNo));
			P_ImportCls->AperakInfo.AddrGLN.CopyTo(((Sdr_Bill *)pObjData)->DlvrAddrCode, sizeof((Sdr_Bill *)pObjData)->DlvrAddrCode);
			P_ImportCls->AperakInfo.AddrGLN.CopyTo(((Sdr_Bill *)pObjData)->LocCode, sizeof((Sdr_Bill *)pObjData)->LocCode);
			P_ImportCls->AperakInfo.BuyerGLN.CopyTo(((Sdr_Bill *)pObjData)->MainGLN, sizeof((Sdr_Bill *)pObjData)->MainGLN);
			P_ImportCls->AperakInfo.SupplGLN.CopyTo(((Sdr_Bill *)pObjData)->GLN, sizeof((Sdr_Bill *)pObjData)->GLN);
			((Sdr_Bill *)pObjData)->OrderDate = P_ImportCls->AperakInfo.OrderDate;
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

// Returns:
//		-1 - ������� ��� �������� ������. ������, ���� � ��������� � ���� ������ ���������� �������� �����,
//			������� ��������� ����� �������� �������, �� ������ ������� �� ���������
//		 0 - ������
//		 1 - ������� ��������� �������� ������
EXPORT int NextImportObjIter(uint idSess, uint objId, void * pRow)
{
	int ok = 1, r = 0;
	SString str;
	THROWERR(P_ImportCls, IEERR_IMPEXPCLSNOTINTD);
	THROWERR(pRow, IEERR_NODATA);
	THROWERR(idSess == P_ImportCls->Id, IEERR_NOSESS);
	THROWERR(objId == P_ImportCls->ObjId, IEERR_NOOBJID);
	// ��������� ���������� ����� �������� � ����������� Sdr_BRow
	THROW(r = P_ImportCls->ParseForGoodsData((Sdr_BRow *)pRow));
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

// ���������� Papyrus'�� ����� ������� GetImportObj. ����������� Papyrus'� ������� ���� ����� ��� �������
EXPORT int ReplyImportObjStatus(uint idSess, uint objId, void * pObjStatus)
{
	int    ok = 1;
	size_t pos = 0;
	SString str;
	// ���� � Papyrus ���� �����, �� ������� �������� ������������� ��� ������
	if(((Sdr_DllImpObjStatus *)pObjStatus)->DocStatus == docStatIsSuchDoc) {
		if(P_ImportCls) {
			if(P_ImportCls->MessageType == PPEDIOP_APERAK) {
				// ���-������ ������ � ���� ��������
				str.Z().Cat(P_ImportCls->AperakInfo.OrderNum.Transf(CTRANSF_INNER_TO_OUTER)).CatDiv(':', 2).Cat(P_ImportCls->AperakInfo.Msg.Utf8ToChar());
				LogMessage(str);
			}
			// ����� ������ �� ������
		}
	}
	// ���� � Papyrus ������ ������ ���
	else if(((Sdr_DllImpObjStatus *)pObjStatus)->DocStatus == docStatNoSuchDoc) {
		if(P_ImportCls) {
			P_ImportCls->IncomMessagesCounter--;
			if((P_ImportCls->MessageType == PPEDIOP_ORDERRSP) || (P_ImportCls->MessageType == PPEDIOP_DESADV)) {
				// ������� ��������� ����� xml-��������
				SFile file;
				file.Remove(P_ImportCls->ImpFileName);
			}
			// � ��� ������������, � ��� ������� ������ � ����� ��������� ������ � ���������/������� "�����", ����� ��, ���� �� ������������, ������ ��� ��������.
			pos = P_ImportCls->TrackIds.getCount();
			P_ImportCls->TrackIds.Add(pos, P_ImportCls->LastTrackId);
		}
	}
	return ok;
}

// Descr: �������� ��������. � ������ ������, ���������� ��� � ����.
//
// Returns:
//		-1 - ��� �������� ���������
//		 0 - ������
//		 1 - ��������� ��������
int ImportCls::ReceiveDoc()
{
	int ok = 1, r = 0;
	uint pos = 0;
	_ns1__Receive param;
	_ns1__ReceiveResponse resp;
	SString listmb, partner_iln, track_id, str, login;
	SFile file;

	EDIServiceSoapProxy proxy(SOAP_XML_INDENT|SOAP_XML_IGNORENS);
	gSoapClientInit(&proxy, 0, 0);

	AperakInfo.Clear(); // ��� ���������� ����� ��-�� "���������" ��������� (��. ����), ����� ��� ��� ���������
						// � ������� �� ������������ ���� �� ����������� ����������� ����������� ���������

	// ��������� ������� �������� ���������
	THROW(r = ListMessageBox(listmb));
	if(r == 1) // ���� ����, ��� ���������, �� ���������
		THROW(r = ParseListMBResp(listmb, partner_iln, track_id));
	if(r == 1) {
		// ���� ������������ ������� ��� ����������� ����������� � ���� ��������
		//if(MessageType == PPEDIOP_ORDERRSP) {
		//	for(pos = 0; pos < RlnCfgList.getCount(); pos++) {
		//		if(!RlnCfgList.at(pos).SuppGLN.CmpNC(partner_iln) &&
		//			!RlnCfgList.at(pos).DocType.CmpNC(WEB_ELEMENT_CODE_TYPE_ORDRSP) &&
		//			!RlnCfgList.at(pos).Direction.CmpNC(WEB_ELEMENT_CODE_DIRECTION_IN))
		//			break;
		//	}
		//}
		//else if(MessageType == PPEDIOP_DESADV) {
		//	for(pos = 0; pos < RlnCfgList.getCount(); pos++) {
		//		if(!RlnCfgList.at(pos).SuppGLN.CmpNC(partner_iln) &&
		//			!RlnCfgList.at(pos).DocType.CmpNC(WEB_ELEMENT_CODE_TYPE_DESADV) &&
		//			!RlnCfgList.at(pos).Direction.CmpNC(WEB_ELEMENT_CODE_DIRECTION_IN))
		//			break;
		//	}
		//}
		//else if(MessageType == PPEDIOP_APERAK) {
		//	for(pos = 0; pos < RlnCfgList.getCount(); pos++) {
		//		if(!RlnCfgList.at(pos).SuppGLN.CmpNC(partner_iln) &&
		//			!RlnCfgList.at(pos).DocType.CmpNC(WEB_ELEMENT_CODE_TYPE_APERAK)/* &&
		//			!RlnCfgList.at(pos).Direction.CmpNC(WEB_ELEMENT_CODE_DIRECTION_IN)*/)
		//			break;
		//	}
		//}
		//if(pos < RlnCfgList.getCount()) { // @vmiller comment
			FormatLoginToLogin(Header.EdiLogin, login.Z()); // �� ������������
			param.Name = (char *)(const char *)login;
			//param.Name = Header.EdiLogin;			// �� ������������ � �������
			param.Password = Header.EdiPassword;	// ������
			param.PartnerIln = (char *)(const char *)/*RlnCfgList.at(pos).SuppGLN*/partner_iln;	// �� ��������, �������� ��� ������ �������� (��� ORDRSP � DESADV ��� ����� GLN ����������,
												// � ��� APERAK ��� ����� ??? - ��������� GLN �������, � �������� ������������ ���������)
			//param.DocumentType = (char *)(const char *)RlnCfgList.at(pos).DocType;	// ��� ���������
			if(MessageType == PPEDIOP_ORDERRSP)
				param.DocumentType = WEB_ELEMENT_CODE_TYPE_ORDRSP;
			else if(MessageType == PPEDIOP_DESADV)
				param.DocumentType = WEB_ELEMENT_CODE_TYPE_DESADV;
			else if(MessageType == PPEDIOP_APERAK)
				param.DocumentType = WEB_ELEMENT_CODE_TYPE_APERAK;
			param.TrackingId = (char *)(const char *)track_id; // �� ��������� � �������
			//param.DocumentStandard = (char *)(const char *)RlnCfgList.at(pos).DocStandard;	// �������� ���������
			param.Timeout = 10000;		// ������� �� ���������� ������ ������ (��) (�������� �������������)
			LastTrackId = track_id;

			if(proxy.Receive(&param, &resp) == SOAP_OK) {
				// ��� ��������� Aperak ������ resp.ReceiveResult �� ����������� � ��������� �������.
				// ������ ��� ����������, ���� ��� ��������� ��������� ������ ��� ����������.
				// ����� ��� ��������� ��������� ��������� ����� ��� �����������. ��� ������ � ������� ���.
				// ���� ����� �� ����, ������� �������� �� ���� � �������� ����� ��������� ������ Read
				if(resp.ReceiveResult) {
					if(atoi(resp.ReceiveResult->Res) == 0) {
						if((MessageType == PPEDIOP_ORDERRSP) || (MessageType == PPEDIOP_DESADV)) {
							// ��� �������� ��������� ����������� ��������� ������ ��� �������� �����
							(str = resp.ReceiveResult->Cnt).ReplaceCR();
							file.Open(ImpFileName, SFile::mWrite);
							file.WriteLine(/*resp.ReceiveResult->Cnt*/str);
							IncomMessagesCounter++;
						}
						else if(MessageType == PPEDIOP_APERAK) {
							// ��� �������� ��������� ����������� ��������� ������ ��� �������� �����
							(str = resp.ReceiveResult->Cnt).ReplaceCR().ReplaceStr("\n", "", 0);
							// ����� �������� �����
							file.Open(ImpFileName, SFile::mWrite);
							//file.Open("D:\\Papyrus\\ppy\\OUT\\Edisoft\\aperak.xml", SFile::mWrite);
							file.WriteLine(/*resp.ReceiveResult->Cnt*/str);
							file.Close();
							THROW(ParseAperakResp(resp.ReceiveResult->Cnt));
							IncomMessagesCounter++;
						}
						// ������ ������ ��������� - ��������
						SetReadStatus(track_id);
					}
					else {
						SetError(IEERR_WEBSERV�ERR);
						SetWebServcError(atoi((const char *)resp.ReceiveResult->Res));
						ok = 0;
					}
				}
				else {
					// ��������� �� "���������" ���������
					SetReadStatus(track_id);
					r = -2; // ���� �� ���-�� �������� ��� �������
				}
			} else {
				ProcessError(proxy);
				ok = 0;
			}
		// @vmiller comment
		/*}
		else {
			str.Z().Cat(partner_iln);
			SetError(IEERR_NOCFGFORGLN, str);
			ok = -1;
		}*/
	}
	else if(r == -1) {
		// �� ��������� "��� �������� ���������" �������� �������� IncomMessagesCounter � ������� FinishImpExp
		ok = -1;
	}

	CATCHZOK;
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
	SString login;
	char   track_id_buf[256];
	_ns1__ChangeDocumentStatus param;
	_ns1__ChangeDocumentStatusResponse resp;
	EDIServiceSoapProxy proxy(SOAP_XML_INDENT|SOAP_XML_IGNORENS);
	gSoapClientInit(&proxy, 0, 0);
	rErrTrackIdList.Z();
	for(size_t pos = 0; pos < TrackIds.getCount(); pos++) {
		STRNSCPY(track_id_buf, TrackIds.Get(pos).Txt);
		FormatLoginToLogin(Header.EdiLogin, login.Z()); // �� ������������
		param.Name = (char *)(const char *)login; // @badcast
		//param.Name = Header.EdiLogin;			// �� ������������ � �������
		param.Password = Header.EdiPassword;	// ������
		param.TrackingId = track_id_buf; // �� ��������� � �������
		param.Status = "N"; // ����� ������ ��������� (new)
		if(proxy.ChangeDocumentStatus(&param, &resp) == SOAP_OK) {
			if(atoi(resp.ChangeDocumentStatusResult->Res) != 0) {
				SetError(IEERR_WEBSERV�ERR);
				SetWebServcError(atoi(resp.ChangeDocumentStatusResult->Res));
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
// Descr: ������ ������ ������������ ��������� � Read
//
int ImportCls::SetReadStatus(SString & trackID)
{
	int    ok = 1;
	SString str, login;
	_ns1__ChangeDocumentStatus param;
	_ns1__ChangeDocumentStatusResponse resp;
	EDIServiceSoapProxy proxy(SOAP_XML_INDENT|SOAP_XML_IGNORENS);
	gSoapClientInit(&proxy, 0, 0);
	FormatLoginToLogin(Header.EdiLogin, login.Z()); // �� ������������
	param.Name = (char *)(const char *)login;
	//param.Name = Header.EdiLogin;			// �� ������������ � �������
	param.Password = Header.EdiPassword;	// ������
	param.TrackingId = (char *)(const char *)trackID; // �� ��������� � �������
	param.Status = "R"; // ����� ������ ��������� (read)
	if(proxy.ChangeDocumentStatus(&param, &resp) == SOAP_OK) {
		if(atoi(resp.ChangeDocumentStatusResult->Res) != 0) {
			SetError(IEERR_WEBSERV�ERR);
			SetWebServcError(atoi((const char *)resp.ChangeDocumentStatusResult->Res));
			ok = 0;
		}
	}
	else
		ok = 0;
	if(!ok) {
		SysLogMessage(SYSLOG_SETREADSTATUS);
		(str = LOG_READSTATERR).Cat(trackID);
		SysLogMessage(str);
		LogMessage(str);
	}
	return ok;
}
//
// Returns:
//		-1 - ��� ���������
//		 0 - ������
//		 1 - ���� ���������
//
int ImportCls::ListMessageBox(SString & rResp)
{
	int    ok = 0;
	uint   pos = 0;
	SString fmt, low, upp, str, login;
	_ns1__ListMBEx param;
	_ns1__ListMBExResponse resp;
	EDIServiceSoapProxy proxy(SOAP_XML_INDENT|SOAP_XML_IGNORENS);
	gSoapClientInit(&proxy, 0, 0);
	//
	// ������ ��������� ������ �������� ����������/�������� �����.
	// 1. ������� ������ ������������ ��� �������� ����������. ���������������, ��� ����� ������ ��������.
	// 2. ���������, ���� �� �� ���� �������� ���������.
	// 3.1. ���� ����, ���������� ������ ���� � ��������� � ������� �� ����� � ������������� �����������
	// 3.2. ���� �������� ��������� �� ���������, �� ������� ��������� ������������, �� ���� ���������� ����������
	// 4. ���� ���� �������� ���� ��� �������� ���� �����������, �� ������� �� ����� � ������� �����������
	//
	//for(pos = 0; pos < RlnCfgList.getCount(); pos ++) {

	//
	// ������ ������.
	// 1. ������� �������� ��������� ��� �������� ����
	// 2. ���� ���� �������� ���� ��� �������� ���� ����� ���������, �� ������� � -1

		//if((((!RlnCfgList.at(pos).DocType.CmpNC(WEB_ELEMENT_CODE_TYPE_ORDRSP) && (MessageType == PPEDIOP_ORDERRSP)) || // ���� ORDRSP
		//	(!RlnCfgList.at(pos).DocType.CmpNC(WEB_ELEMENT_CODE_TYPE_DESADV) && (MessageType == PPEDIOP_DESADV))) && // ���� DESADV
		//	!RlnCfgList.at(pos).Direction.CmpNC(WEB_ELEMENT_CODE_DIRECTION_IN)) ||
		//	(!RlnCfgList.at(pos).DocType.CmpNC(WEB_ELEMENT_CODE_TYPE_APERAK) && (MessageType == PPEDIOP_APERAK))) { // ���� APERAK
			FormatLoginToLogin(Header.EdiLogin, login.Z()); // �� ������������
			param.Name = (char *)(const char *)login;
			//param.Name = Header.EdiLogin;			// �� ������������
			param.Password = Header.EdiPassword;	// ������ ������������
		//	param.PartnerIln = (char *)(const char *)RlnCfgList.at(pos).SuppGLN;	// �� ��������, �� �������� ��� ������� ��������
		//	param.DocumentType = (char *)(const char *)RlnCfgList.at(pos).DocType;	// ��� ���������
			if(MessageType == PPEDIOP_ORDERRSP)
				param.DocumentType = WEB_ELEMENT_CODE_TYPE_ORDRSP;
			else if(MessageType == PPEDIOP_DESADV)
				param.DocumentType = WEB_ELEMENT_CODE_TYPE_DESADV;
			else if(MessageType == PPEDIOP_APERAK)
				param.DocumentType = WEB_ELEMENT_CODE_TYPE_APERAK;
		//	param.DocumentVersion = (char *)(const char *)RlnCfgList.at(pos).DocVersion;	// ������ ������������
		//	param.DocumentStandard = (char *)(const char *)RlnCfgList.at(pos).DocStandard;	// �������� ���������
		//	param.DocumentTest = (char *)(const char *)RlnCfgList.at(pos).DocTest;		// ������ ���������
			param.DocumentStatus = "N";	// ������ ���������� ���������� (�������������, ��������) (������ �����)
			param.Timeout = 10000;		// ������� �� ���������� ������ ������ (��) (�������� �������������)
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
					int len = strlen(resp.ListMBExResult->Cnt);
					if(strncmp(resp.ListMBExResult->Cnt, EMPTY_LISTMB_RESP, strlen(resp.ListMBExResult->Cnt)) != 0) {
						rResp.CopyFrom(resp.ListMBExResult->Cnt);
						ok = 1;
					}
					else ok = -1;
				}
				else {
					SetError(IEERR_WEBSERV�ERR);
					SetWebServcError(atoi((const char *)resp.ListMBExResult->Res));
				}
			}
			else {
				ProcessError(proxy);
			}
		//}

	//if(!ok) {
	//	SetError(IEERR_NOCONFIG);
	//	SysLogMessage(SYSLOG_LISTMESSAGEBOX);
	//}
	return ok;
}
//
// ������ ����������, ����������� � ������� ListMB()
// ����������, ����� �������� "�� ��������� � �������" � "ID ��������, �� �������� ��� ������� ��������"
// Retruns:
//		-1 - �� ���� �������� �������� ���������
//		 0 - ������
//		 1 - ��� ��������� �������
//
int ImportCls::ParseListMBResp(const char * pResp, SString & rPartnerIln, SString & rDocId)
{
	int    ok = -1, found = 0;
	SString str;
	xmlTextReader * p_xml_ptr;
	xmlNode * p_node;
	xmlParserInputBuffer * p_input = xmlParserInputBufferCreateMem(pResp, sstrlen(pResp), XML_CHAR_ENCODING_NONE);
	THROWERR((p_xml_ptr = xmlNewTextReader(p_input, NULL)), IEERR_NULLREADXMLPTR);
	while(xmlTextReaderRead(p_xml_ptr) && (found != 2)) {
		p_node = xmlTextReaderCurrentNode(p_xml_ptr);
		if(p_node && sstreq(p_node->name, WEB_ELEMENT_NAME_TRACKID) && p_node->children) {
			rDocId = (const char *)p_node->children->content;
			found++;
		}
		else if(p_node && sstreq(p_node->name, WEB_ELEMENT_NAME_PARTNER_ILN) && p_node->children) {
			rPartnerIln = (const char *)p_node->children->content;
			found++;
		}
	}
	if(found == 2)
		ok = 1;
	CATCH
		SysLogMessage(SYSLOG_PARSELISTMBRESP);
		ok = 0;
	ENDCATCH;
	xmlFreeParserInputBuffer(p_input);
	xmlFreeTextReader(p_xml_ptr);
	return ok;
}
//
// ��� ORDRESP � DESADV
// ��������� ���������� �������� � ��������� Sdr_Bill
//
// ����������: Sender = Seller
//             Reciever = Buyer
//
int ImportCls::ParseForDocData(Sdr_Bill * pBill)
{
	int    ok = 0, exit_while = 0, is_correct = 0;
	SString str;
	xmlDoc * p_doc = 0;
    HasSellerCertificate = 0;
	THROWERR_STR(fileExists(ImpFileName), IEERR_IMPFILENOTFOUND, ImpFileName);
	THROWERR(pBill, IEERR_NODATA);
	memzero(pBill, sizeof(Sdr_Bill));
	THROWERR((p_doc = xmlReadFile(ImpFileName, NULL, XML_PARSE_NOENT)), IEERR_NULLREADXMLPTR);
	xmlNode * p_node = xmlDocGetRootElement(p_doc);
	THROWERR(p_node, IEERR_XMLREAD);
	// @v8.5.6 {
	STRNSCPY(pBill->EdiOpSymb, p_node->name);
	pBill->EdiOp = MessageType;
	// } @v8.5.6
	if(((MessageType == PPEDIOP_ORDERRSP && SXml::IsName(p_node, "Document-OrderResponse")) || // �� ������� ���� ����� ������, ��� ��� �� ���
		(MessageType == PPEDIOP_DESADV && SXml::IsName(p_node, "Document-DespatchAdvice"))) && p_node->children)
		is_correct = 1;
	while(p_node && (p_node->type == XML_ELEMENT_NODE)) {
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
		if(p_node && p_node->type == XML_ELEMENT_NODE) {
			if(((MessageType == PPEDIOP_ORDER && SXml::IsName(p_node, "Document-OrderResponse")) ||
				(MessageType == PPEDIOP_DESADV && SXml::IsName(p_node, "Document-DespatchAdvice") == 0)) && p_node->children) {
				is_correct = 1; // ���������, ��� ���������, ��������� � ������������ �������, ������������� ��������� ���������
				p_node = p_node->children;
			}
			if(is_correct) {
				//
				// ��� ��������� ������ ��� ORDRSP
				//
				if(MessageType == PPEDIOP_ORDERRSP) {
					if(SXml::IsName(p_node, ELEMENT_NAME_ORDRSPNUMBER) && p_node->children) {
						// ����� ��������� ������
						str.Set(p_node->children->content).Utf8ToOem(); // @vmiller @TODO tooem -> tochar
						STRNSCPY(pBill->Code, str);
						ok = 1;
					}
					else if(SXml::IsName(p_node, ELEMENT_NAME_ORDRSPDATE) && p_node->children) {
						// ���� ��������� ������
						// YYYY-MM-DD
						str.Set(p_node->children->content);
						SString sub_str;
						str.Sub(0, 4, sub_str);
						pBill->Date.setyear((uint)sub_str.ToLong());
						str.Sub(5, 2, sub_str);
						pBill->Date.setmonth((uint)sub_str.ToLong());
						str.Sub(8, 2, sub_str);
						pBill->Date.setday((uint)sub_str.ToLong());
						ok = 1;
					}
					else if(SXml::IsName(p_node, ELEMENT_NAME_DELIVERDATE) && p_node->children) {
						// ��������� ���� ��������
						// YYYY-MM-DD
						str = (const char *)p_node->children->content;
						SString sub_str;
						str.Sub(0, 4, sub_str);
						pBill->DueDate.setyear((uint)sub_str.ToLong());
						str.Sub(5, 2, sub_str);
						pBill->DueDate.setmonth((uint)sub_str.ToLong());
						str.Sub(8, 2, sub_str);
						pBill->DueDate.setday((uint)sub_str.ToLong());
						ok = 1;
					}
				}
				//
				// ��� ��������� ������ � DESADV
				//
				if(MessageType == PPEDIOP_DESADV) {
					if(SXml::IsName(p_node, "DespatchAdviceNumber") && p_node->children) {
						// ����� ��������� �����������
						//strcpy(pBill->Code, (const char *)p_node->children->content);
						str.Set(p_node->children->content).Utf8ToChar(); // ����� ����� ��������� � Papyrus
						STRNSCPY(pBill->Code, str);
						(TTN = (const char *)p_node->children->content).Utf8ToOem();
						ok = 1;
					}
					else if(SXml::IsName(p_node, "EstimatedDeliveryDate") && p_node->children) {
						// ��������� ���� ��������
						// YYYY-MM-DD
						str.Set(p_node->children->content);
						SString sub_str;
						str.Sub(0, 4, sub_str);
						pBill->DueDate.setyear((uint)sub_str.ToLong());
						str.Sub(5, 2, sub_str);
						pBill->DueDate.setmonth((uint)sub_str.ToLong());
						str.Sub(8, 2, sub_str);
						pBill->DueDate.setday((uint)sub_str.ToLong());
						ok = 1;
					}
					else if(SXml::IsName(p_node, ELEMENT_NAME_DESPATCHADVICEDATE) && p_node->children) {
						// ���� ��������� �����������
						// YYYY-MM-DD
						str.Set(p_node->children->content);
						SString sub_str;
						str.Sub(0, 4, sub_str);
						pBill->Date.setyear((uint)sub_str.ToLong());
						str.Sub(5, 2, sub_str);
						pBill->Date.setmonth((uint)sub_str.ToLong());
						str.Sub(8, 2, sub_str);
						pBill->Date.setday((uint)sub_str.ToLong());
						ok = 1;
					}
				}
				//
				// ����� ���������
				//
				if(SXml::IsName(p_node, "DocumentFunctionCode") && p_node->children) {
					// ��� ������� ���������
				}
				else if(SXml::IsName(p_node, ELEMENT_NAME_BUYERORDERNUMBER) && p_node->children) {
					// ����� ������, �� ������� ������ ������ �����
					strcpy(pBill->OrderBillNo, (const char *)p_node->children->content);
					ok = 1;
				}
				else if(SXml::IsName(p_node, ELEMENT_NAME_BUYERORDERDATE) && p_node->children) {
					// ���� ������, �� ������� ������ ������ �����
					// YYYY-MM-DD
					str = (const char *)p_node->children->content;
					SString sub_str;
					str.Sub(0, 4, sub_str);
					pBill->OrderDate.setyear((uint)sub_str.ToLong());
					str.Sub(5, 2, sub_str);
					pBill->OrderDate.setmonth((uint)sub_str.ToLong());
					str.Sub(8, 2, sub_str);
					pBill->OrderDate.setday((uint)sub_str.ToLong());
					ok = 1;
				}
				else if(SXml::IsName(p_node, "Receiver") && p_node->children) {
					p_node = p_node->children;
					if(SXml::IsName(p_node, "ILN") && p_node->children) {
						// GLN ����������
						// ���������� GLN ����������
						strcpy(pBill->MainGLN, (const char *)p_node->children->content);
						ok = 1;
					}
				}
				else if(SXml::IsName(p_node, "Sender") && p_node->children) {
					p_node = p_node->children;
					if(SXml::IsName(p_node, "ILN") && p_node->children) { // GLN �����������
						strcpy(pBill->GLN, (const char *)p_node->children->content);
						ok = 1;
					}
				}
				else if(SXml::IsName(p_node, "DeliveryPoint") && p_node->children) {
					p_node = p_node->children;
					if(SXml::IsName(p_node, "ILN") && p_node->children) { // GLN ����� ��������
						STRNSCPY(pBill->DlvrAddrCode, p_node->children->content);
						ok = 1;
					}
				}
				else if(SXml::IsName(p_node, ELEMENT_NAME_TOTALLINES) && p_node->children) { // ���������� �������� �����
					GoodsCount = atoi((const char *)p_node->children->content);
					ok = 1;
				}
				else if(SXml::IsName(p_node, ELEMENT_NAME_TOTALGROSSAMOUNT) && p_node->children) { // ����� ��������� � ���
					pBill->Amount = atof((const char *)p_node->children->content);
					ok = 1;
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
// ��� ORDRESP � DESADV
// ��������� ���������� �������� � ��������� Sdr_BRow
// Returns:
//		-1 - ������� ��� �������� ������. ������, ���� � ��������� � ���� ������ ���������� �������� �����,
//			������� ��������� ����� �������� �������, �� ������ ������� �� ���������
//		 0 - ������
//		 1 - ������� ��������� �������� ������
//
int ImportCls::ParseForGoodsData(Sdr_BRow * pBRow)
{
	int    ok = 1, index = 1, line_end = 0, exit_while = 0;
	SString str;
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
		while(p_node && !line_end) {
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
			if(p_node && (p_node->type == XML_READER_TYPE_ELEMENT)) {
				if(SXml::IsName(p_node, "Line") && p_node->children) {
					if(index == (Itr.GetCount() + 1)) {
						while(p_node && !line_end) {
							exit_while = 0;
							if(p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT))
								p_node = p_node->children;
							else if(p_node->next)
								p_node = p_node->next;
							else {
								while(p_node && p_node->P_ParentNode && !exit_while) {
									xmlNode * p_node_2 = p_node->P_ParentNode->next;
									if(p_node_2) {
										p_node = p_node_2;
										exit_while = 1;
									}
									else
										p_node = p_node->P_ParentNode;
								}
							}
							if(p_node) {
								//
								// ������ ������� ��������� ��� ��������� �������� �������. ���� ����� �� ������,
								// �� ���� ������ ������ �������� ������.
								//
								if(p_node->type == XML_DOCUMENT_NODE || SXml::IsName(p_node, "Line"))
									line_end = 1;
								else {
									if(MessageType == PPEDIOP_ORDERRSP) {
										if(p_node->children) {
											if(sstreq(p_node->name, ELEMENT_NAME_ITEMSTATUS)) {
												// ������ �������� ������� (������, �������, �� ������)
											}
											else if(sstreq(p_node->name, ELEMENT_NAME_ALLOCDELIVRD)) {
												// ������������ ���������� ������
												pBRow->Quantity = atof((const char *)p_node->children->content);
											}
											else if(sstreq(p_node->name, "OrderedUnitPacksize")) {
												// ���������� ������ � ��������
												pBRow->UnitPerPack = atof((const char *)p_node->children->content);
											}
											else if(sstreq(p_node->name, "OrderedUnitGrossPrice")) {
												// ���� � ���
												pBRow->Cost = atof((const char *)p_node->children->content);
											}
										}
									}
									else if(MessageType == PPEDIOP_DESADV) {
										if(p_node->children) {
											if(sstreq(p_node->name, ELEMENT_NAME_QTTYDISPATCHED)) {
												// ����������� ����������
												pBRow->Quantity = atof((const char *)p_node->children->content);
											}
											else if(sstreq(p_node->name, ELEMENT_NAME_UNITPACKSZ)) {
												// ���������� ������ � ��������
												pBRow->UnitPerPack = atof((const char *)p_node->children->content);
											}
											else if(sstreq(p_node->name, ELEMENT_NAME_UNITGROSSPRICE)) {
												// ���� � ���
												pBRow->Cost = atof((const char *)p_node->children->content);
											}
											else if(sstreq(p_node->name, ELEMENT_NAME_ALCOCONTENT)) {
												// ���������� �������� � %
											}
											else if(sstreq(p_node->name, "Line-Party")) {
												// ������ ���� � �������������
												p_node = p_node->children;
												if(SXml::IsName(p_node, ELEMENT_NAME_PARTYTYPE) && p_node->children) {
													// ��� ���� - �������������?
													if(SXml::IsContent(p_node->children, ELEMENT_CODE_PARTYTYPE_MF)) {
														while(p_node->next) {
															p_node = p_node->next;
															if(p_node->children) {
																//if(sstreq(p_node->name, "ILN") && p_node->children) {
																//	// GLN �������������
																//	str = (const char *)p_node->children->content;
																//}
																if(SXml::IsName(p_node, ELEMENT_NAME_TAXID)) { // ��� �������������
																	STRNSCPY(pBRow->ManufINN, p_node->children->content);
																}
																else if(SXml::IsName(p_node, ELEMENT_NAME_TAXRECREASONCODE)) { // ��� �������������
																	STRNSCPY(pBRow->ManufKPP, p_node->children->content);
																}
																else if(SXml::IsName(p_node, ELEMENT_NAME_NAME)) { // ������������ �������������
																	str.Set(p_node->children->content).Utf8ToOem().CopyTo(pBRow->LotManuf, sizeof(pBRow->LotManuf));
																}
															}
														}
													}
												}
											}
											else if(SXml::IsName(p_node, "Reference-Elements")) {
												// ������ �������������� ���� � ������
												int is_next = 1;
												p_node = p_node->children;
												while(is_next) {
													if(SXml::IsName(p_node, "Reference-Element") && p_node->children) {
														p_node = p_node->children;
														if(SXml::IsName(p_node, "Reference-Type") && p_node->children) {
															// ���� ��� ���
															if(SXml::IsContent(p_node->children, ELEMENT_CODE_REFTYPE_ABT)) {
																p_node = p_node->next;
																if(SXml::IsName(p_node, "Reference-Id") && p_node->children) {
																	str.Set(p_node->children->content).CopyTo(pBRow->CLB, sizeof(pBRow->CLB));
																}
															}
															// ���� ��� �����
															else if(SXml::IsContent(p_node->children, ELEMENT_CODE_REFTYPE_SER)) {
																p_node = p_node->next;
																if(SXml::IsName(p_node, "Reference-Id") && p_node->children) {
																	str.Set(p_node->children->content).CopyTo(pBRow->Serial, sizeof(pBRow->Serial));
																}
															}
															// ���� ��� ��� ���� ���������
															else if(SXml::IsContent(p_node->children, ELEMENT_CODE_REFTYPE_YA1)) {
																p_node = p_node->next;
																if(SXml::IsName(p_node, "Reference-Id") && p_node->children) {
																	pBRow->GoodKindCode = str.Set(p_node->children->content).ToLong();
																}
															}
															p_node = p_node->P_ParentNode;
															if(p_node->next)
																p_node = p_node->next;
															else
																is_next = 0;
														}
													}
												}
											}
										}
									}
									// ����� ���������
									if(SXml::IsName(p_node, "LineNumber") && p_node->children) {
										// ����� ������ �������� �������
									}
									else if(SXml::IsName(p_node, "EAN") && p_node->children) {
										// �������� ������
										strcpy(pBRow->Barcode, (const char *)p_node->children->content);
									}
									else if(SXml::IsName(p_node, "SupplierItemCode") && p_node->children) {
										// ������� ������ � ����������
										strcpy(pBRow->ArCode, (const char *)p_node->children->content);
									}
									else if(SXml::IsName(p_node, ELEMENT_NAME_ITEMDISCR) && p_node->children) {
										// �������� ������ (��������)
										(str = (const char *)p_node->children->content).Utf8ToOem();
										STRNSCPY(pBRow->GoodsName, str);
									}
									else if(SXml::IsName(p_node, "UnitOfMeasure") && p_node->children) {
										// ������� ���������
										if(SXml::IsContent(p_node->children, ELEMENT_CODE_UNITOFMEASURE_KGM))
											strcpy(pBRow->UnitName, UNIT_NAME_KG);
										else // ���� � ������
											strcpy(pBRow->UnitName, UNIT_NAME_PIECE);
									}
									else if(SXml::IsName(p_node, ELEMENT_NAME_TAXRATE) && p_node->children) {
										// ������ ��� � %
										pBRow->VatRate = atof((const char *)p_node->children->content);
									}
								}
							}
						}
					}
					// ����� ���. �������� �� ��������� ���������
					strcpy(pBRow->TTN, TTN);
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

// Descr: ������ ����������, ����������� � ������� Receive() ��� APERAK
// ARG(pResp		 IN): ������ ��� �������, ���������� � Receive()
// Retruns:
//		-1 - �� ���� �������� �������� ���������
//		 0 - ������
//		 1 - ��� ��������� �������
int ImportCls::ParseAperakResp(const char * pResp)
{
	int ok = 1, is_correct = 0, exit_while = 0;
	SString str;
	xmlDoc * p_doc = 0;

	AperakInfo.Clear();
	if(pResp) {
		THROWERR_STR(fileExists(ImpFileName), IEERR_IMPFILENOTFOUND, ImpFileName);
		THROWERR((p_doc = xmlReadFile(ImpFileName, NULL, XML_PARSE_NOENT)), IEERR_NULLREADXMLPTR);
		xmlNode * p_node = xmlDocGetRootElement(p_doc);
		THROWERR(p_node, IEERR_XMLREAD);
		if(SXml::IsName(p_node, "Document-ApplicationMessage") && p_node->children) // �� ������� ���� ����� ������, ��� ��� Aperak
			is_correct = 1;
		while(is_correct && p_node && (p_node->type == XML_ELEMENT_NODE)) {
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
				if(is_correct) {
					if(SXml::IsName(p_node, ELEMENT_NAME_DOCNUMBER) && p_node->children) {
						// �������� ����� ��������� ������
						(str = (const char *)p_node->children->content).Utf8ToOem();
						AperakInfo.OrderNum = str;
					}
					else if(SXml::IsName(p_node, ELEMENT_NAME_DOCUMENTID) && p_node->children) {
						// ������ ID ��������� �������
						AperakInfo.DocID = (const char *)p_node->children->content;
					}
					else if(SXml::IsName(p_node, ELEMENT_NAME_DOCDATE) && p_node->children) {
						// ������� ���� ��������� ������
						SString sub;
						str = (const char *)p_node->children->content;
						str.Sub(0, 4, sub);
						AperakInfo.OrderDate.setyear((uint)sub.ToLong());
						str.Sub(5, 2, sub);
						AperakInfo.OrderDate.setmonth((uint)sub.ToLong());
						str.Sub(8, 2, sub);
						AperakInfo.OrderDate.setday((uint)sub.ToLong());
					}
					else if(SXml::IsName(p_node, ELEMENT_NAME_BUYER) && p_node->children) {
						p_node = p_node->children;
						if(SXml::IsName(p_node, "ILN") && p_node->children)
							// ������� GLN ����������
							AperakInfo.BuyerGLN = (const char *)p_node->children->content;
					}
					else if(p_node && sstreq(p_node->name, ELEMENT_NAME_SELLER) && p_node->children) {
						p_node = p_node->children;
						if(SXml::IsName(p_node, "ILN") && p_node->children)
							// ������� GLN ����������
							AperakInfo.SupplGLN = (const char *)p_node->children->content;
					}
					else if(p_node && sstreq(p_node->name, "DeliveryPoint") && p_node->children) {
						p_node = p_node->children;
						if(SXml::IsName(p_node, "ILN") && p_node->children)
							// ������� GLN ����� ��������
							AperakInfo.AddrGLN = (const char *)p_node->children->content;
					}
					else if(p_node && sstreq(p_node->name, "LineMessageCode") && p_node->children) {
						// �������� ��� �������
						AperakInfo.Code = (const char *)p_node->children->content;
					}
					else if(p_node && sstreq(p_node->name, ELEMENT_NAME_SYSTEMSGTEXT) && p_node->children) {
						// �������� �������� �������
						AperakInfo.Msg = (const char *)p_node->children->content;
					}
				}
			}
		}
	}
	else
		ok = -1;
	THROWERR_STR(is_correct, IEERR_INVMESSAGEYTYPE, "APERAK");
	CATCH
		SysLogMessage(SYSLOG_PARSEAPERAKRESP);
		ok = 0;
	ENDCATCH;
	if(p_doc)
		xmlFreeDoc(p_doc);
	return ok;
}
//
//
// ����� ������� ��� �������/��������
//
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
		if(!P_ImportCls->IncomMessagesCounter && ((P_ImportCls->MessageType == PPEDIOP_ORDERRSP) || (P_ImportCls->MessageType == PPEDIOP_DESADV))) {
			SysLogMessage(LOG_NOINCOMDOC);
			LogMessage(LOG_NOINCOMDOC);
		}
	}
	CATCH
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

void ProcessError(EDIServiceSoapProxy & rProxy)
{
	char   temp_err_buf[1024];
	SString temp_buf;
	ErrorCode = IEERR_SOAP;
	rProxy.soap_sprint_fault(temp_err_buf, sizeof(temp_err_buf));
	(temp_buf = temp_err_buf).Utf8ToChar();
	StrError = temp_err_buf;
}

