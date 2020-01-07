// IMPEXPKONTUR.CPP
// ���������� ��� �������/�������� ���������� � xml ����� ���������� Kontur
//
// �������
// (Order)
// ������� �������� �� Papyrus
//		��������:
//		����� ����� ��������� ��� ���. ��������������.
//		����� ���������� �����������/��������� ������. ��������������.
//		�������� ���� �������� - Bill.DueDate
//		���� ��������� - Bill.Date
//		GLN ����������� - Bill.AgentGLN / Bill.MainGLN
//		GLN ���������� - Bill.GLN
//		GLN ����� �������� - Bill.DlvrAddrCode / Bill.LocCode
//		����� ����� �������� - Bill.DlvrAddr
//		����� ����� ������ � ��� - Bill.Amount

//		�������� ������:
//		�������� ������ - BRow.Barcode
//		������� ������ - BRow.ArCode (���������)
//		�������� ������ - BRow.GoodsName
//		���������� ����������� ������ - BRow.Quantity
//		���� � ��� - BRow.Cost
//		������ ��� - BRow.VatRate
//		������� ��������� - BRow.UnitName
//
// �������� ��������� � Papyrus
//		����� EnumExpReceipt() ��������� Sdr_DllImpExpReceipt (�� ������ � Papyrus ������������ � ���� �����, ��� ������ ������
//      �� ���������� ����� ��������� � ����� �������)
//
//
//	������
//  �������/�������� ��������� � ������� �� ������:
//		�� Papyrus - ��� �������� (ORDRSP, DESADV ��� APERAK)
//		�� Dll	   - � ������ ��������� ORDRSP ��� DESADV - ����������� ��� ��������� ��������� Sdr_Bill
//					 � ������ ��������� ������� - ����������� ��������� ���� � Sdr_Bill
//		�� Papyrus - ��������� � ������� ��������� ������, �� ������� �������� �������������/�����������/������. ����� ReplyImportObjStatus, ��������� Sdr_DllImpObjStatus
//		���� ����� ����� � Papyrus ����:
//			�� Dll - � ������ ��������� ORDRSP ��� DESADV ����������� ��������� Sdr_BRow
//                   � ������ ��������� APERAK ��������� � ����
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
//			���� � ��� - BRow.Cost (Ordrsp, Desadv)
//			�������� ������ - BRow.Barcode (Ordrsp, Desadv)
//			�������� ������ - BRow.GoodsName (Ordrsp, Desadv)
//			������� ��������� - BRow.UnitName (Ordrsp, Desadv)
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
// ����������� �������:
//    1) �������� ����� FTP
//    2) ���� � ������������ ��������� ��������� ��������� ������ ��� �������� ��� ��������� ��������, �� ������ �� ��������� ���.
//    �������������� � �������� APERAK �� �� ��������� �� ����� ������, �� GLN ��������. �� ���� ���� APERAK �� ����� ������ ��
//    � ����� ��������, ������� dll ������ ������ ��� �� ������ �� FTP. ����� ������ � ��������� ������ ��������� ���������,
//    ���� ���������� � "�����������". ������ ���.
//
//
//
#include <ppedi.h>
#include <snet.h>
#include <winsock2.h>
#include <wininet.h>

#define EXPORT	extern "C" __declspec (dllexport)
#define THROWERR(expr,val) { if(!(expr)){ SetError(val, ""); goto __scatch; } }
#define THROWERR_STR(expr,val,str) { if(!(expr)){ SetError(val, str); goto __scatch; } }

#define UNIT_NAME_KG			"��"
#define UNIT_NAME_PIECE			"�����"
#define INBOX					"Inbox"
#define OUTBOX					"Outbox"
#define REPORTSBOX				"Reports"

// �������� ���������-��������������
#define ELEMENT_CODE_E0052_D		"D"			// ������ ���������
#define ELEMENT_CODE_E0054_01B		"01B"		// ������ �������
#define ELEMENT_CODE_E0051_UN		"UN"		// ��� ������� �����������
#define ELEMENT_CODE_E0057_EAN010	"EAN010"	// ���, ����������� ������� ������������
//#define ELEMENT_CODE_E1001_220		"220"		// ��� ��������� - �����
#define ELEMENT_CODE_E2379_102		"102"		// ������ ����/������� - CCYYMMDD
#define ELEMENT_CODE_E6345_RUB		"RUB"		// �����

// ���� ������ � ���������
#define IEERR_SYMBNOTFOUND			1			// ������ �� ������ (������ ���� ������� �������/��������)
#define IEERR_NODATA				2			// ��� ������
#define IEERR_NOSESS				3			// ������ � ����� ������� �� ����������
#define IEERR_ONLYBILLS				4			// ������ DLL ����� �������� ������ � �����������
#define IEERR_NOOBJID				5			// ������� � ����� ��������������� ���
#define IEERR_IMPEXPCLSNOTINTD		6			// ������ ��� �������/�������� �� ���������������
#define IEERR_FTP					7			// ������ FTP

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
#define IEERR_ONLYEXPMSGTYPES		19			// �������� �������� �������� ������ � ������ ������ ORDER
#define IEERR_NOEXPPATH				20			// �� ���������� ���������� ��� ����� ��������

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
#define SYSLOG_DOCPARTIES				"in DocPartiesAndCurrency()"
#define SYSLOG_GOODSLINES				"in GoodsLines()"
#define SYSLOG_ENDDOC					"in EndDoc()"
#define SYSLOG_SENDDOC					"in SendDoc()"
#define SYSLOG_RECEIVEDOC				"in ReceiveDoc()"
#define SYSLOG_PARSEFORDOCDATA			"in ParseForDocData()"
#define SYSLOG_PARSEFORGOODDATA			"in ParseForGoodData()"
#define SYSLOG_PARSEAPERAKRESP			"in ParseAperakResp()"

#define LOG_NOINCOMDOC					"��� �������� ���������"

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
void ProcessError(const char * pProcess);

int SetError(int errCode, const char * pStr = "")
{
	ErrorCode = errCode;
	StrError = pStr;
	return 0;
}

void LogMessage(const char * pMsg);
void SysLogMessage(const char * pMsg);
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
	uint   Type;
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
	{"DECLNORDER",  PPEDIOP_DECLINEORDER},
	{"RECADV",      PPEDIOP_RECADV},
	{"ALCODESADV",  PPEDIOP_ALCODESADV}
};

ErrMessage ErrMsg[] = {
	{IEERR_SYMBNOTFOUND,		"������ �� ������"},
	{IEERR_NODATA,				"������ �� ��������"},
	{IEERR_NOSESS,				"������ � ����� ������� ���"},
	{IEERR_ONLYBILLS,			"Dll ����� �������� ������ � �����������"},
	{IEERR_NOOBJID,				"������� � ����� ��������������� ���"},
	{IEERR_IMPEXPCLSNOTINTD,	"������ ��� �������/�������� �� ���������������"},
	{IEERR_FTP,					"������ FTP"},
	{IEERR_IMPFILENOTFOUND,		"���� ������� �� ������: "},
	{IEERR_INVMESSAGEYTYPE,		"�������� ��� ���������. ��������� "},
	{IEERR_NOCFGFORGLN,			"��� ������ ���� �������� ��� �������� ������������ ��� ������������ � GLN "},
	{IEERR_NULLWRIEXMLPTR,		"������� xmlWriter"},
	{IEERR_NULLREADXMLPTR,		"������� xmlReader"},
	{IEERR_XMLWRITE,			"������ ������ � xml-����"},
	{IEERR_XMLREAD,				"������ ������ �� xml-�����"},
	{IEERR_TOOMUCHGOODS,		"����� �������, ���������� � ����� �������, ������ ��������� �� ����������"},
	{IEERR_MSGSYMBNOTFOUND,		"������ �� ������ (������ ���� �������� �������/��������)"},
	{IEERR_ONLYIMPMSGTYPES,		"�������� ������� �������� ������ � ������ ������ ORDRSP, DESADV � APERAK, ALCODESADV"},
	{IEERR_ONLYEXPMSGTYPES,		"�������� �������� �������� ������ � ������ ������ ORDER"},
	{IEERR_NOEXPPATH,			"�� ���������� ���������� ��� ����� ��������"}
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
	rMsg.Z();
	for(size_t i = 0; i < SIZEOFARRAY(ErrMsg); i++) {
		if(ErrMsg[i].Id == ErrorCode) {
			rMsg.Cat(ErrMsg[i].P_Msg);
			break;
		}
	}
	if(oneof4(ErrorCode, IEERR_FTP, IEERR_IMPFILENOTFOUND, IEERR_INVMESSAGEYTYPE, IEERR_NOCFGFORGLN))
		rMsg.Space().Cat(StrError);
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
			else {
				rStr.CatChar(*login);
				login ++;
			}
		}
	}
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
// Returns:
//		0 - ������ �� ������
//		1 - ������ ������
//
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
	void   Init()
	{
		Count = 0;
	}
	const  uint GetCount() { return Count; }
	void   Next() { Count++; }
private:
	uint   Count;
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
	SString TTN;		// ��� DESADV. �������� �� ��������� ��������� � ������ �� ��� �������� ������
};

class FtpClient {
public:
	enum {
		stDisconnected = 0,
		stConnected
	};
	FtpClient(const char * pLogin, const char * pPass) : Status(stDisconnected), Login(pLogin), Password(pPass), HInternet(0), HFtpSession(0), HFtpFind(0)
	{
	}
	~FtpClient()
	{
		Disconnect();
	}
	int    Connect();
	void   Disconnect();
	int    PutFile(const char * pLocSrc, const char * pFtpDst);
	int    GetFile(const char * pFtpSrc, const char * pLocDst);
	int    NextFileName(const char * pFtpSrc, SString & rFileName);
	int    DeleteFile(const char * pFtpFile);

	SEnumImp * SLAPI Enum(const char * pWildcard); // SDirEntry
private:
	int    Status;
    SString Login;
	SString Password;
	HINTERNET HInternet;
    HINTERNET HFtpSession;
	HINTERNET HFtpFind;
};

int FtpClient::Connect()
{
	int    ok = 1;
	if(Status == stDisconnected) {
		THROW(HInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL,NULL, 0));
		THROW(HFtpSession = InternetConnect(HInternet, _T("ftp-edi.kontur.ru"), INTERNET_DEFAULT_FTP_PORT, 
			SUcSwitch(Login), SUcSwitch(Password), INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0));
		Status = stConnected;
	}
    CATCHZOK
	return ok;
}

void FtpClient::Disconnect()
{
	if(Status == stConnected) {
		InternetCloseHandle(HFtpSession);
		InternetCloseHandle(HInternet);
		HFtpSession = 0;
		HInternet = 0;
		HFtpFind = 0;
		Status = stDisconnected;
	}
}

int FtpClient::PutFile(const char * pLocSrc, const char * pFtpDst)
{
	int    ok = 1;
	if(Status == stDisconnected)
		THROW(Connect());
	THROW(FtpPutFile(HFtpSession, SUcSwitch(pLocSrc), SUcSwitch(pFtpDst), FTP_TRANSFER_TYPE_BINARY, 0));
	CATCHZOK
	return ok;
}

int FtpClient::GetFile(const char * pFtpSrc, const char * pLocDst)
{
	int    ok = 1;
	if(Status == stDisconnected)
		THROW(Connect());
	THROW(FtpGetFile(HFtpSession, SUcSwitch(pFtpSrc), SUcSwitch(pLocDst), 0, 0, 0, NULL));
	CATCHZOK
    return ok;
}
//
// Descr: ���������� ��� ����� � ����� � ���������� ��� ��������.
// ARG(pFtpSrc    IN): ����� �� ftp
// ARG(rFileName OUT): ��� ����� �� ftp
// Retutns:
//   -1 - ��� ������
//    0 - ������
//    1 - ��� ����� ��������
//
int FtpClient::NextFileName(const char * pFtpSrc, SString & rFileName)
{
	int    ok = 1;
	WIN32_FIND_DATA find_data;
	if(Status == stDisconnected)
		THROW(Connect());
	memzero(&find_data, sizeof(WIN32_FIND_DATA));
	if(!HFtpFind)
		THROW(HFtpFind = FtpFindFirstFile(HFtpSession, SUcSwitch(pFtpSrc), &find_data, 0, NULL))
	else
		InternetFindNextFile(HFtpFind, &find_data);
	if(!isempty(find_data.cFileName))
		rFileName.CopyFrom(SUcSwitch(find_data.cFileName));
	else
		ok = -1;
	CATCHZOK
    return ok;
}

SEnumImp * FtpClient::Enum(const char * pWildcard)
{
	class FtpEnum : public SEnumImp {
	public:
		FtpEnum(HINTERNET hSess, const char * pWildcard) : H(0), HSess(hSess), Wildcard(pWildcard)
		{
		}
		virtual int Next(void * pRec)
		{
			int    ok = 0;
			SDirEntry entry;
			MEMSZERO(entry);
			WIN32_FIND_DATA find_data;
			if(!H) {
				H = FtpFindFirstFile(HSess, SUcSwitch(Wildcard), &find_data, 0, 0);
				if(H)
					ok = 1;
			}
			else {
				if(InternetFindNextFile(H, &find_data))
					ok = 1;
			}
			if(ok) {
				entry = find_data;
			}
			else {
				if(GetLastError() == ERROR_NO_MORE_FILES)
					ok = -1;
			}
			ASSIGN_PTR((SDirEntry *)pRec, entry);
			return ok;
		}
	protected:
		HINTERNET H;
		HINTERNET HSess;
		SString Wildcard;
	};
	return new FtpEnum(HFtpSession, pWildcard);
}

int FtpClient::DeleteFile(const char * pFtpFile)
{
	int    ok = 1;
	if(Status == stDisconnected)
		THROW(Connect());
	THROW(FtpDeleteFile(HFtpSession, SUcSwitch(pFtpFile)));
	CATCHZOK
    return ok;
}

EXPORT int FinishImpExp();

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch(dwReason) {
		case DLL_PROCESS_ATTACH:
			{
				SString product_name;
				(product_name = "Papyrus-ImpExpKontur");
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
			FinishImpExp();
			break;
	}
	return TRUE;
}
//
// �������
//
class ExportCls : public ImportExportCls {
public:
	ExportCls() : ImportExportCls(), Id(0), ObjId(0), ObjType(0), MessageType(0), Inited(0), SegNum(0), ReadReceiptNum(0), BillSumWithoutVat(0.0),
		P_XmlWriter(0), TotalGoodsCount(0), Declined(0)
	{
		ErrorCode = 0;
		WebServcErrorCode = 0;
		LogName.Z();
	}
	~ExportCls()
	{
		P_XmlWriter = 0;
	}
	void   CreateFileName(uint num)
	{
		ExpFileName.Z().Cat(PathStruct.Drv).CatChar(':').Cat(PathStruct.Dir).Cat(PathStruct.Nam).Cat(num).Dot().Cat(PathStruct.Ext);
	}
	int    OrderHeader();
	int    DocPartiesAndCurrency();
	int    GoodsLines(Sdr_BRow * pBRow);
	int    EndDoc();
	int    SendDoc();
	int    RecadvHeader();

	uint   Id;                // ��, ������� Papyrus ����� ������������ ��� �� ������ ��������
	uint   ObjId;             // �� ��������������� ������� (��������, ��� ������ ���������� ������� ��������� ������������� ���� ��)
	uint   ObjType;           // ��� �������������� ��������
	uint   MessageType;       // ��� ���������: ORDER
	uint   Inited;
	uint   Declined;          // ������� ������ ��������� ������
	uint   SegNum;            // ���������� ��������� (���������) ���������
	uint   ReadReceiptNum;    // ���������� ����� ���������, ����������� �� ReceiptList
	double BillSumWithoutVat; // ����� ��������� ��� ��� (�������� ������� ��������������, ��� � Sdr_Bill ���� ������� �� ����������)
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
//
// ������ � ��������� ���������� � ���������
//
// �������� ����������� �����
//
int ExportCls::OrderHeader()
{
	int    ok = 1;
	SString str, fmt;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	BillSumWithoutVat = 0.0;
	SegNum = 0;
	xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"ORDERS");
		{
			SXml::WNode n_unh(P_XmlWriter, "UNH");
			{
				SegNum++;
				n_unh.PutInner("E0062", str.Z().Cat(ObjId)); // �� ���������
				{
					SXml::WNode n_s009(P_XmlWriter, "S009");
					n_s009.PutInner("E0065", "ORDERS"); // ��� ���������
					n_s009.PutInner("E0052", ELEMENT_CODE_E0052_D); // ������ ���������
					n_s009.PutInner("E0054", ELEMENT_CODE_E0054_01B); // ������ �������
					n_s009.PutInner("E0051", ELEMENT_CODE_E0051_UN); // ��� ������� �����������
					n_s009.PutInner("E0057", ELEMENT_CODE_E0057_EAN010); // ���, ����������� ������� ������������
				}
			}
		}
		{
			SXml::WNode n_bgm(P_XmlWriter, "BGM");
			{
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
					xmlTextWriterWriteString(P_XmlWriter, Declined ? (const xmlChar *)"1" : (const xmlChar *)"9"); // ������ | ��������
				xmlTextWriterEndElement(P_XmlWriter); //E1225
			}
		}
		{
			SXml::WNode n_dtm(P_XmlWriter, "DTM"); // ���� ���������
			{
				SegNum++;
				SXml::WNode n_s507(P_XmlWriter, "C507");
				n_s507.PutInner("E2005", "137"); // ������������ ������� ����-������� (����/����� ���������/���������)
				n_s507.PutInner("E2380", str.Z().Cat(Bill.Date, DATF_YMD|DATF_CENTURY|DATF_NODIV)); // ���� ��� �����, ��� ������
				n_s507.PutInner("E2379", ELEMENT_CODE_E2379_102); // ������ ����/������� (CCYYMMDD)
			}
		}
		{
			SXml::WNode n_dtm(P_XmlWriter, "DTM"); // ���� ��������
			{
				SegNum++;
				SXml::WNode n_s507(P_XmlWriter, "C507");
				n_s507.PutInner("E2005", "2"); // ������������ ������� ����-������� (����/����� ��������)
				n_s507.PutInner("E2380", str.Z().Cat(NZOR(Bill.DueDate, Bill.Date), DATF_YMD|DATF_CENTURY|DATF_NODIV)); // ���� ��� �����, ��� ������
				n_s507.PutInner("E2379", ELEMENT_CODE_E2379_102); // ������ ����/������� (CCYYMMDD)
			}
		}
	CATCH
		SysLogMessage(SYSLOG_ORDERHEADER);
		ok = 0;
	ENDCATCH;
	return ok;
}

int ExportCls::DocPartiesAndCurrency()
{
	int    ok = 1;
	SString str, sender_gln, login;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	const char * p_sg = 0;
	if(MessageType == PPEDIOP_ORDER)
		p_sg = "SG2";
	else if(MessageType == PPEDIOP_RECADV)
		p_sg = "SG4";
	if(p_sg) {
		{
			SXml::WNode n_sg2(P_XmlWriter, p_sg); // GLN ����������
			{
				SXml::WNode n_nad(P_XmlWriter, "NAD"); // ������������ � �����
				{
					SegNum++;
					n_nad.PutInner("E3035", "SU"); // ������������ ������� - ���������
					{
						SXml::WNode n_c082(P_XmlWriter, "C082"); // ������ �������
						n_c082.PutInner("E3039", Bill.GLN); // GLN �������
						n_c082.PutInner("E3055", "9"); // ��� ������� ����������� - EAN (������������� ���������� �������� ���������)
					}
				}
			}
		}
		{
			SXml::WNode n_sg2(P_XmlWriter, p_sg); // GLN ����������������
			{
				SXml::WNode n_nad(P_XmlWriter, "NAD"); // ������������ � �����
				{
					SegNum++;
					n_nad.PutInner("E3035", "CZ"); // ������������ ������� - ����������������
					{
						SXml::WNode n_c082(P_XmlWriter, "C082"); // ������ �������
						n_c082.PutInner("E3039", Bill.GLN); // GLN �������
						n_c082.PutInner("E3055", "9"); // ��� ������� ����������� - EAN (������������� ���������� �������� ���������)
					}
				}
			}
		}
		{
			SXml::WNode n_sg2(P_XmlWriter, p_sg); // GLN ����������
			{
				SXml::WNode n_nad(P_XmlWriter, "NAD"); // ������������ � �����
				{
					SegNum++;
					n_nad.PutInner("E3035", "BY"); // ������������ ������� - ����������
					{
						SXml::WNode n_c082(P_XmlWriter, "C082"); // ������ �������
						n_c082.PutInner("E3039", isempty(Bill.AgentGLN) ? Bill.MainGLN : Bill.AgentGLN); // GLN �������
						n_c082.PutInner("E3055", "9"); // ��� ������� ����������� - EAN (������������� ���������� �������� ���������)
					}
				}
			}
		}
		{
			SXml::WNode n_sg2(P_XmlWriter, p_sg); // GLN ����� ��������
			{
				SXml::WNode n_nad(P_XmlWriter, "NAD"); // ������������ � �����
				{
					SegNum++;
					n_nad.PutInner("E3035", "DP"); // ������������ ������� - �������� ����� ��������
					{
						SXml::WNode n_c082(P_XmlWriter, "C082"); // ������ �������
						n_c082.PutInner("E3039", isempty(Bill.DlvrAddrCode) ? Bill.LocCode : Bill.DlvrAddrCode); // GLN �������
						n_c082.PutInner("E3055", "9"); // ��� ������� ����������� - EAN (������������� ���������� �������� ���������)
					}
				}
			}
		}
		if(MessageType == PPEDIOP_ORDER) {
			// �������������� ��������
			if(!isempty(Bill.Obj2GLN) || !isempty(Bill.AgentGLN) || !isempty(Bill.MainGLN)) {
				SXml::WNode n_sg2(P_XmlWriter, p_sg); // GLN �����������
				{
					SXml::WNode n_nad(P_XmlWriter, "NAD"); // ������������ � �����
					{
						SegNum++;
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E3035"); // ������������ �������
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"IV"); // GLN �����������
						xmlTextWriterEndElement(P_XmlWriter); //E3035
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C082"); // ������ �������
							xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E3039"); // GLN �������
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
					}
				}
			}
			{
				SXml::WNode n_sg7(P_XmlWriter, "SG7");
				{
					SXml::WNode n_cux(P_XmlWriter, "CUX"); // ������
					{
						SegNum++;
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"C504"); // ������
							xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E6347"); // ������������ ������
								xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"2"); // ��������� ������
							xmlTextWriterEndElement(P_XmlWriter); //E6347
							xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E6345"); // ������������� ������ �� ISO 4217
								xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6345_RUB); // �����
							xmlTextWriterEndElement(P_XmlWriter); //E6345
							xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E6343"); // ������������ ���� ������
								xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"9"); // ������ ������
							xmlTextWriterEndElement(P_XmlWriter); //E6343
						xmlTextWriterEndElement(P_XmlWriter); //C504
					}
				}
			}
		}
	}
	CATCH
		SysLogMessage(SYSLOG_DOCPARTIES);
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
			SXml::WNode n_unh(P_XmlWriter, "UNH");
			{
				SegNum++;
				n_unh.PutInner("E0062", str.Z().Cat(ObjId)); // �� ���������
				{
					SXml::WNode n_s(P_XmlWriter, "S009");
					n_s.PutInner("E0065", "RECADV"); // ��� ���������
					n_s.PutInner("E0052", "D"); // ������ ���������
					n_s.PutInner("E0054", "01B"); // ������ �������
					n_s.PutInner("E0051", "UN"); // ��� ������� �����������
					n_s.PutInner("E0057", "EAN005"); // ���, ����������� ������� ������������
				}
			}
		}
		{
			SXml::WNode n_bgm(P_XmlWriter, "BGM");
			{
				SegNum++;
				{
					SXml::WNode n_c2(P_XmlWriter, "C002"); // ��� ���������/���������
					n_c2.PutInner("E1001", "632"); // ��� ��������� (RECADV (���� ������������� ������))
				}
				{
					SXml::WNode n_c106(P_XmlWriter, "C106"); // ������������� ���������/���������
					n_c106.PutInner("E1004", (str = Bill.Code).ToUtf8()); // ����� ��������� (������ ���� �������� 17 ��������)
				}
				n_bgm.PutInner("E1225", "9"); // ��� ������� ��������� (�������� (���� ��� �����, ��������������� �����, ������ � �.�.))
			}
		}
		{
			SXml::WNode n_dtm(P_XmlWriter, "DTM"); // ���� ���������
			{
				SegNum++;
				SXml::WNode n_s(P_XmlWriter, "C507");
				n_s.PutInner("E2005", "137"); // ������������ ������� ����-������� ���������/���������
				n_s.PutInner("E2380", str.Z().Cat(Bill.Date, DATF_YMD|DATF_CENTURY|DATF_NODIV)); // ���� ��� �����, ��� ������
				n_s.PutInner("E2379", "102"); // ������ ����/������� CCYYMMDD
			}
		}
		{
			SXml::WNode n_dtm(P_XmlWriter, "DTM"); // ���� �������
			{
				SegNum++;
				SXml::WNode n_s(P_XmlWriter, "C507");
				n_s.PutInner("E2005", "50"); // ������������ ������� ����-�������
				n_s.PutInner("E2380", str.Z().Cat(Bill.Date, DATF_YMD|DATF_CENTURY|DATF_NODIV)); // ���� ��� �����, ��� ������
				n_s.PutInner("E2379", "102"); // ������ ����/������� CCYYMMDD
			}
		}
		{
			SXml::WNode n_sg1(P_XmlWriter, "SG1"); // ����� ����������� �� �������� (DESADV)
			{
				{
					SXml::WNode n_ref(P_XmlWriter, "RFF");
					{
						SXml::WNode n_s(P_XmlWriter, "C506");
						n_s.PutInner("E1153", "AAK"); // ������������� ����������� �� ��������
						n_s.PutInner("E1154", (str = Bill.DesadvBillNo).ToUtf8()); // ����� ����������� �� ��������
					}
				}
				{
					SXml::WNode n_dtm(P_XmlWriter, "DTM"); // ���� DESADV
					{
						SXml::WNode n_s(P_XmlWriter, "C507");
						n_s.PutInner("E2005", "171"); // ������������ ������� ����-������� ��������� DESADV
						n_s.PutInner("E2380", str.Z().Cat(Bill.DesadvBillDt, DATF_YMD|DATF_CENTURY|DATF_NODIV)); // ���� ��� �����, ��� ������
						n_s.PutInner("E2379", "102"); // ������ ����/������� CCYYMMDD
					}
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
	CATCH
		SysLogMessage(SYSLOG_RECADVHEADER);
		ok = 0;
	ENDCATCH;
	return ok;
}
//
// ��������� � ����� ���� � �������
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
			n_cps.PutInner("E7164", "1"); // ����� �������� �� ��������� - 1
		}
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"SG22");
	}
		{
			SXml::WNode n_lin(P_XmlWriter, "LIN");
			SegNum++;
			{
				SXml::WNode n(P_XmlWriter, "E1082", str.Z().Cat(Itr.GetCount() + 1)); // ����� ������ ������
			}
			{
				SXml::WNode n_212(P_XmlWriter, "C212"); // �������� ������������� ������
				n_212.PutInner("E7140", str.Z().Cat(pBRow->Barcode)); // �����-��� ������
				n_212.PutInner("E7143", "SRV"); // ��� ��������� EAN.UCC
			}
		}
		// �������������� ��������
		if(!isempty(pBRow->ArCode)) {
			// @v8.5.6 ����������������� ������� ������� �� SXml
			SXml::WNode n_pia(P_XmlWriter, "PIA"); // �������������� ������������� ������
			n_pia.PutInner("E4347", "1"); // �������������� �������������
			{
				SXml::WNode n_c212(P_XmlWriter, "C212");
				n_c212.PutInner("E7140", str.Z().Cat(pBRow->ArCode).Transf(CTRANSF_INNER_TO_UTF8)); // �������
				n_c212.PutInner("E7143", "SA"); // ������������� �������� ����������
			}
		}
		// �������������� ��������
		if(!isempty(pBRow->GoodsName)) {
			SXml::WNode n_imd(P_XmlWriter, "IMD"); // �������� ������
			{
				SegNum++;
				n_imd.PutInner("E7077", "F"); // ��� ������� �������� (�����)
				{
					SXml::WNode n_c273(P_XmlWriter, "C273"); // ��������
					n_c273.PutInner("E7008", SXml::WNode::CDATA(str.Z().Cat(pBRow->GoodsName).ToUtf8()));
				}
			}
		}
		if(MessageType == PPEDIOP_RECADV) {
			SXml::WNode n_qty(P_XmlWriter, "QTY"); // �������� ����������
			{
				SegNum++;
				SXml::WNode n_c186(P_XmlWriter, "C186"); // �����������
				n_c186.PutInner("E6063", "194"); // ������������ ���� ���������� (�������� ���������� ������)
				n_c186.PutInner("E6060", str.Z().Cat(pBRow->Quantity)); // ����������
				(str = pBRow->UnitName).ToUpper1251();
				if(str.CmpNC(UNIT_NAME_KG) == 0)
					n_c186.PutInner("E6411", "KGM"); // ������� ��������� (����������)
				else
					n_c186.PutInner("E6411", "PCE"); // ������� ��������� (��������� ��������)
			}
		}
		else {
			SXml::WNode n_qty(P_XmlWriter, "QTY"); // ���������� ����������
			{
				SegNum++;
				SXml::WNode n_c186(P_XmlWriter, "C186"); // �����������
				n_c186.PutInner("E6063", "21"); // ������������ ���� ���������� (���������� ���������� ������)
				n_c186.PutInner("E6060", str.Z().Cat(pBRow->Quantity)); // ����������
				(str = pBRow->UnitName).ToUpper1251();
				if(str.CmpNC(UNIT_NAME_KG) == 0)
					n_c186.PutInner("E6411", "KGM"); // ������� ��������� (����������)
				else
					n_c186.PutInner("E6411", "PCE"); // ������� ��������� (��������� ��������)
			}
		}
		{
			SXml::WNode n_moa(P_XmlWriter, "MOA"); // ����� �������� ������� � ���
			{
				SegNum++;
				SXml::WNode n_c516(P_XmlWriter, "C516");
				n_c516.PutInner("E5025", "79"); // ������������ ����� �������� ������� (������������� ����� �������� ������� � ���)
				double sum = pBRow->Cost * pBRow->Quantity;
				n_c516.PutInner("E5004", str.Z().Printf("%.2f", sum)); // ����� (����� ������ ����� ������� - �� ������ 2)
			}
		}
		{
			SXml::WNode n_moa(P_XmlWriter, "MOA"); // ����� �������� ������� ��� ���
			{
				SegNum++;
				SXml::WNode n_c516(P_XmlWriter, "C516");
				n_c516.PutInner("E5025", "203"); // ������������ ����� �������� ������� (������������� ����� �������� ������� ��� ���)
				{
					double sum = (pBRow->Cost / (pBRow->VatRate + 100) * 100) * pBRow->Quantity;
					BillSumWithoutVat += sum;
					n_c516.PutInner("E5004", str.Z().Printf("%.2f", sum)); // ����� (����� ������ ����� ������� - �� ������ 2)
				}
			}
		}
		{
			SXml::WNode n_sg32(P_XmlWriter, "SG32"); // ���� ������ � ���
			{
				SXml::WNode n_pri(P_XmlWriter, "PRI"); // ������� ����������
				{
					SegNum++;
					SXml::WNode n_c509(P_XmlWriter, "C509");
					n_c509.PutInner("E5125", "AAE"); // ������������ ���� (���� ��� ������ � ��������, �� � �������)
					const double cost = pBRow->Cost;
					n_c509.PutInner("E5118", str.Z().Printf("%.2f", cost)); // ���� (����� ������ ����� ������� - �� ������ 2)
				}
			}
		}
		{
			SXml::WNode n_sg32(P_XmlWriter, "SG32"); // ���� ������ ��� ���
			{
				SXml::WNode n_pri(P_XmlWriter, "PRI"); // ������� ����������
				{
					SegNum++;
					SXml::WNode n_c509(P_XmlWriter, "C509");
					n_c509.PutInner("E5125", "AAA"); // ������������ ���� (������ ���� ��� �������)
					const double cost = pBRow->Cost / (pBRow->VatRate + 100) * 100;
					n_c509.PutInner("E5118", str.Z().Printf("%.2f", cost)); // ���� (����� ������ ����� ������� - �� ������ 2)
				}
			}
		}
		{
			SXml::WNode n_sg38(P_XmlWriter, "SG38"); // ������ ���
			{
				SXml::WNode n_tax(P_XmlWriter, "TAX");
				{
					n_tax.PutInner("E5283", "7"); // ������������ (������������� ���������� ����������)
					{
						SXml::WNode n_c241(P_XmlWriter, "C241");
						n_c241.PutInner("E5153", "VAT"); // ������������ (������������� ������ ���)
					}
					{
						SXml::WNode n_c243(P_XmlWriter, "C243");
						n_c243.PutInner("E5278", str.Z().Cat(pBRow->VatRate)); // ������ ���
					}
				}
			}
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
// ��������� ������������ ������
//
int ExportCls::EndDoc()
{
	int    ok = 1;
	SString str;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
#if 0 // {
	{
		SXml::WNode n(P_XmlWriter, "UNS"); // ����������� ���
		SegNum++;
		{ SXml::WNode(P_XmlWriter, "E0081", "S"); } // ������������� ������ (���� �������� ����������)
	}
	{
		SXml::WNode n(P_XmlWriter, "MOA"); // ����� ������ � ���
		SegNum++;
		{
			SXml::WNode n2(P_XmlWriter, "C516");
			{ SXml::WNode(P_XmlWriter, "E5025", "128"); } // ������������ ����� (����� ��������� � ���)
			{ SXml::WNode(P_XmlWriter, "E5004", str.Z().Cat(Bill.Amount, MKSFMTD(0, 2, 0))); } // �����
		}
	}
	{
		SXml::WNode n(P_XmlWriter, "MOA"); // ����� ������ ��� ���
		SegNum++;
		{
			SXml::WNode n2(P_XmlWriter, "C516");
			{ SXml::WNode(P_XmlWriter, "E5025", "98"); } // ������������ ����� (����� ��������� ��� ���)
			{ SXml::WNode(P_XmlWriter, "E5004", str.Z().Cat(BillSumWithoutVat, MKSFMTD(0, 2, 0))); } // �����
		}
	}
	{
		SXml::WNode n(P_XmlWriter, "CNT"); // �������� ����������
		SegNum++;
		{
			SXml::WNode n2(P_XmlWriter, "C270");
			{ SXml::WNode(P_XmlWriter, "E6069", "2"); } // ������������ ���� �������� ���������� (���������� �������� ������� � ���������)
			{ SXml::WNode(P_XmlWriter, "E6066", str.Z().Cat(Itr.GetCount())); } // ��������
		}
	}
	{
		SXml::WNode n(P_XmlWriter, "UNT"); // ��������� ���������
		SegNum++;
		{ SXml::WNode(P_XmlWriter, "E0074", str.Z().Cat(SegNum)); } // ����� ����� ��������� � ���������
		{ SXml::WNode(P_XmlWriter, "E0062", str.Z().Cat(ObjId)); } // // ����� ������������ ��������� (��������� � ��������� � ���������)
	}
	xmlTextWriterEndElement(P_XmlWriter); //ORDERS
#else // }{
		{
			SXml::WNode n_uns(P_XmlWriter, "UNS"); // ����������� ���
			SegNum++;
			n_uns.PutInner("E0081", "S"); // ������������� ������ (���� �������� ����������)
		}
		{
			SXml::WNode n_moa(P_XmlWriter, "MOA"); // ����� ������ � ���
			{
				SegNum++;
				SXml::WNode n_c516(P_XmlWriter, "C516");
				n_c516.PutInner("E5025", "128"); // ������������ ����� (����� ��������� � ���)
				n_c516.PutInner("E5004", str.Z().Printf("%.2f", Bill.Amount)); // ����� (����� ������ ����� ������� - �� ������ 2)
			}
		}
		{
			SXml::WNode n_moa(P_XmlWriter, "MOA"); // ����� ������ ��� ���
			{
				SegNum++;
				SXml::WNode n_c516(P_XmlWriter, "C516");
				n_c516.PutInner("E5025", "98"); // ������������ ����� (����� ��������� ��� ���)
				n_c516.PutInner("E5004", str.Z().Printf("%.2f", BillSumWithoutVat)); // ����� (����� ������ ����� ������� - �� ������ 2)
			}
		}
		{
			SXml::WNode n_cnt(P_XmlWriter, "CNT"); // �������� ����������
			{
				SegNum++;
				SXml::WNode n_c270(P_XmlWriter, "C270");
				n_c270.PutInner("E6069", "2"); // ������������ ���� �������� ���������� (���������� �������� ������� � ���������)
				n_c270.PutInner("E6066", str.Z().Cat(Itr.GetCount())); // ��������
			}
		}
		{
			SegNum++;
			SXml::WNode n_unt(P_XmlWriter, "UNT"); // ��������� ���������
			n_unt.PutInner("E0074", str.Z().Cat(SegNum)); // ����� ����� ��������� � ���������
			n_unt.PutInner("E0062", str.Z().Cat(ObjId)); // ����� ������������ ��������� (��������� � ��������� � ���������)
		}
	xmlTextWriterEndElement(P_XmlWriter); //ORDERS
#endif
	CATCH
		SysLogMessage(SYSLOG_ENDDOC);
		ok = 0;
	ENDCATCH;
	return ok;
}
//
// ������ � FTP
//
// ���������� ��������
//
int ExportCls::SendDoc()
{
	int    ok = 1;
	SString	inbox_filename;
	SPathStruc path_struct(ExpFileName);
	(inbox_filename = OUTBOX).CatChar('/').Cat(path_struct.Nam).Dot().Cat(path_struct.Ext);
	Sdr_DllImpExpReceipt * p_exp_rcpt = 0;
	FtpClient ftp_client(Header.EdiLogin, Header.EdiPassword);
	//
	// ������������ � ftp
	//
	if(ftp_client.Connect()) {
		//
		// ����������� ����
		//
		if(ftp_client.PutFile(ExpFileName, inbox_filename)) {
			p_exp_rcpt = new Sdr_DllImpExpReceipt;
			memzero(p_exp_rcpt, sizeof(Sdr_DllImpExpReceipt));
			p_exp_rcpt->ID = atol(Bill.ID);
			STRNSCPY(p_exp_rcpt->ReceiptNumber, Bill.ID); // ����� ������ �������� �� ��������� � ��������
			ReceiptList.insert(p_exp_rcpt);
		}
		else {
			ProcessError("FtpPutFile");
			ok = 0;
		}
	}
	else {
		ProcessError("FtpConnect");
		ok = 0;
	}
	if(!ok) {
		SysLogMessage(SYSLOG_SENDDOC);
	}
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
	SString temp_buf;
	SETIFZ(P_ExportCls, new ExportCls);
	if(P_ExportCls && !P_ExportCls->Inited) {
		if(pExpHeader) {
			P_ExportCls->Header = *static_cast<const Sdr_ImpExpHeader *>(pExpHeader);
			FormatLoginToLogin(P_ExportCls->Header.EdiLogin, temp_buf.Z());
			temp_buf.CopyTo(P_ExportCls->Header.EdiLogin, sizeof(P_ExportCls->Header.EdiLogin));
		}
		if(!isempty(pOutFileName)) {
			P_ExportCls->PathStruct.Split(pOutFileName);
			P_ExportCls->PathStruct.Ext = "xml";
			if(P_ExportCls->PathStruct.Nam.Empty())
				P_ExportCls->PathStruct.Nam = "export_";
		}
		else {
			SLS.Init("Papyrus");
			P_ExportCls->PathStruct.Split(temp_buf = SLS.GetExePath());
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
		P_ExportCls->Inited = 1;
	}
	THROWERR(P_ExportCls, IEERR_IMPEXPCLSNOTINTD);
	CATCH
		SysLogMessage(SYSLOG_INITEXPORT);
		GetErrorMsg(temp_buf.Z());
		SysLogMessage(temp_buf);
		LogMessage(temp_buf);
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
	THROWERR(GetMsgTypeBySymb(pMsgType, P_ExportCls->MessageType), IEERR_MSGSYMBNOTFOUND);
	if(P_ExportCls->MessageType == PPEDIOP_DECLINEORDER) {
		P_ExportCls->MessageType = PPEDIOP_ORDER;
		P_ExportCls->Declined = 1;
	}
	else
		P_ExportCls->Declined = 0;
	THROWERR(oneof2(P_ExportCls->MessageType, PPEDIOP_ORDER, PPEDIOP_RECADV), IEERR_ONLYEXPMSGTYPES);
	THROWERR(P_ExportCls->ObjType == objBill, IEERR_ONLYBILLS);
	// ��������� ���������� ��������
	if(P_ExportCls->P_XmlWriter) {
		THROW(P_ExportCls->EndDoc());
		xmlTextWriterEndDocument(P_ExportCls->P_XmlWriter);
		xmlFreeTextWriter(P_ExportCls->P_XmlWriter);
		P_ExportCls->P_XmlWriter = 0;
		// ���������� ����, �������������� � ������� ���. ��� ����� � ����� ��������� ��� �� ������ ������� ����� ��������
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
// ������
//
//
// ����� ������ ������� ���������� �� Korus � EdiSoft ��-�� ������� ����� ftp.
// 1) ������� ������ � ������� �������� ������ ������� ������� (������ ������� � ����� �����) � ������ ������ ����� �������,
//    � ����� ������ � ������� ������ ��� �������� �� ������ ftp
// 2) ����� Receive() ��������� ���� ������, ���� �� ����. ����������� ������ ����� ������ ������ ��������� ����
//    � ������ ��� �� ��������� ����
// 3) ����� ReplyImportObjStatus() ����� ���������� ��� ����� � ������ ��� ��������, ���� ������ ���� ������������ ���
//    ������� ��������
// 4) � FinishImpExp() �� ����� ftp ����� ������� �����, ��� ����� ����������� � ������� ��� ��������
//
struct AperakInfoSt {
	AperakInfoSt()
	{
		Clear();
	}
	void Clear()
	{
		OrderNum.Z();
		Code.Z();
		Msg.Z();
		AddedMsg.Z();
		SupplGLN.Z();
		BuyerGLN.Z();
		AddrGLN.Z();
		OrderDate = ZERODATE;
	}
	SString OrderNum;	// ����� ��������� ������
	SString Code;		// ��� �������
	SString Msg;		// �������� �������
	SString AddedMsg;	// �������������� ���������
	SString SupplGLN;	// GLN ����������
	SString BuyerGLN;	// GLN ����������
	SString AddrGLN;	// GLN ������ ��������
	LDATE	OrderDate;	// ���� ���������
};

class ImportCls : public ImportExportCls {
public:
	ImportCls() : P_FtpCli(0), GoodsCount(0), Id(0), ObjType(0), MessageType(0), State(0), BillSumWithoutVat(0.0)
	{
		ErrorCode = 0;
		WebServcErrorCode = 0;
		AperakInfo.Clear();
		FilesForDel.Z();
	}
	~ImportCls()
	{
		delete P_FtpCli;
	}
	int    ReceiveDoc(const char * pLocalPath, int * pResultId);
	int    ParseForDocData(Sdr_Bill * pBill);
	int    ParseForGoodsData(Sdr_BRow * pBRow);
	int    ParseAperakResp();
	int    GetMessageList(const char * pLocalPath);
	int    ParseFileName(const char * pFileName, PPEdiMessageEntry * pEntry) const;
	int    ReplyImportObjStatus(uint idSess, uint objId, void * pObjStatus);
	int    Finish()
	{
		if(!MsgList.getCount()) {
			SysLogMessage(LOG_NOINCOMDOC);
			LogMessage(LOG_NOINCOMDOC);
		}
		MsgList.Clear();
		State &= ~stMsgList;
		State &= ~stInit;
		return 1;
	}

	enum {
		stInit    = 0x0001,
		stMsgList = 0x0002
	};
	long   State;
	//uint   Inited;
	int    GoodsCount;			// ����� ������� � ���������
	uint   Id;					// ��, ������� Papyrus ����� ������������ ��� �� ������ ��������
	//uint   ObjId;				// �� �������������� ������� (��������, ���� ��� ����� ����������, �� � ������� ��������� ���� ��)
	uint   ObjType;				// ��� ������������� ��������
	uint   MessageType;			// ��� ��������: ORDRESP, APERAK
	//uint   InboxReadIndex;      // ������ ������ �� InboxFiles
	double BillSumWithoutVat;	// � Sdr_Bill ���� ������� �� ����������, ������� �������� ����� ��������� Papyrus'� ��� ��������
	SString ImpFileName;
	SString LogFileName;		// ��� ��, ��� � ImpFileName
	SString TempPath;
	Iterator Itr;
	//SPathStruc PathStruct;
	AperakInfoSt AperakInfo;
	//StrAssocArray InboxFiles;   // ������ � ������� �������� ������ ������� ����
	StrAssocArray FilesForDel;  // ������ � ������� ������, ������� ��������� ������� �� ����� �� ftp
private:
	int    ParseListMBResp(const char * pResp, SString & rPartnerIln, SString & rDocId);
	SString & GetFtpPath(const PPEdiMessageEntry & rEntry, SString & rBuf) const;
	SString & GetTempPath(const PPEdiMessageEntry & rEntry, SString & rBuf) const;
	FtpClient * P_FtpCli;
	PPEdiMessageList MsgList;
};
//
// Descr: �������� ��������. � ������ ������, ���������� ��� � ����.
//
// Returns:
//		-1 - ��� �������� ���������
//		 0 - ������
//		 1 - ��������� ��������
//
int ImportCls::ReceiveDoc(const char * pLocalPath, int * pResultId)
{
	int    ok = 1;
	int    result_id = 0;
	size_t pos = 0;
	SString read_filename, file_type, box_name, str;
	// @v8.5.9 SSrchParam srch_param;
	// @v8.5.9 memzero(&srch_param, sizeof(SSrchParam));
	// @v8.5.9 srch_param.Flags = SSPF_WORDS;
	//FtpClient * p_ftp_client = P_FtpCli;
	if(!(State & stMsgList)) {
		THROW(GetMessageList(pLocalPath));
	}
	//
	// ��������� ������� ������
	//
	const uint _pos = MsgList.getPointer();
	if(_pos < MsgList.getCount()) {
		// ������������� ����� ������
		const PPEdiMessageEntry & r_eme = MsgList.at(_pos);
		MsgList.incPointer();
        (ImpFileName = TempPath).SetLastSlash().Cat(r_eme.SId);
        if(fileExists(ImpFileName)) {
			SFile file(ImpFileName, SFile::mRead);
			SString file_buf;
			int64 file_size = 0;
			file.CalcSize(&file_size);
			SBuffer buf((size_t)file_size + 1);
			if(file.IsValid()) {
				SStrScan Scan;
				long ReEmmId = 0;
				Scan.RegisterRe("[ ]+<", &ReEmmId);
				while(file.ReadLine(file_buf)) {
					Scan.Set(file_buf, 0);
					if(Scan.GetRe(ReEmmId, str))
						file_buf.ReplaceStr(str, "<", 0);
					else
						Scan.Incr();
					file_buf.ReplaceCR().ReplaceStr("\n", "", 0);
					buf.Write((const char *)file_buf, file_buf.Len());
				}
			}
			file.Close();
			if(file.Open(ImpFileName, SFile::mWrite)) {
				file.Write(buf.GetBuf(), buf.GetAvailableSize());
			}
			file.Close();
			if(MessageType == PPEDIOP_APERAK) {
				THROW(ParseAperakResp()); // ����� �������� �����
			}
            result_id = r_eme.ID;
		}
		else {
			ProcessError("ReceiveDoc");
			ok = 0;
		}
	}
	else
		ok = -1;
	CATCHZOK
	if(!ok)
		SysLogMessage(SYSLOG_RECEIVEDOC);
	ASSIGN_PTR(pResultId, result_id);
	return ok;
}

int ImportCls::ParseFileName(const char * pFileName, PPEdiMessageEntry * pEntry) const
{
	int    ok = 0;
	SString left, right;
    SPathStruc ps(pFileName);
    pEntry->EdiOp = 0;
	if(ps.Nam.Divide('_', left, right) > 0) {
		if(left.CmpNC("OrdRsp") == 0)
			pEntry->EdiOp = PPEDIOP_ORDERRSP;
		else if(left.CmpNC("Desadv") == 0)
			pEntry->EdiOp = PPEDIOP_DESADV;
		else if(left.CmpNC("Aperak") == 0)
			pEntry->EdiOp = PPEDIOP_APERAK;
		else if(left.CmpNC("alcodesadv") == 0 || left.CmpNC("alcrpt") == 0 || left.CmpNC("alcdes") == 0)
			pEntry->EdiOp = PPEDIOP_ALCODESADV;
		pEntry->Uuid.FromStr(right);
		STRNSCPY(pEntry->SId, pFileName);
	}
	else {
		if(left.CmpPrefix("ordrsp", 1) == 0)
			pEntry->EdiOp = PPEDIOP_ORDERRSP;
		else if(left.CmpPrefix("Desadv", 1) == 0)
			pEntry->EdiOp = PPEDIOP_DESADV;
		else if(left.CmpPrefix("Aperak", 1) == 0)
			pEntry->EdiOp = PPEDIOP_APERAK;
		else if(left.CmpPrefix("alcodesadv", 1) == 0 || left.CmpPrefix("alcrpt", 1) == 0 || left.CmpPrefix("alcdes", 1) == 0)
			pEntry->EdiOp = PPEDIOP_ALCODESADV;
	}
	if(pEntry->EdiOp) {
		ok = 1;
	}
	return ok;
}

SString & ImportCls::GetFtpPath(const PPEdiMessageEntry & rEntry, SString & rBuf) const
{
	return rBuf.Z().Cat(rEntry.Box).SetLastSlash().Cat(rEntry.SId);
}

SString & ImportCls::GetTempPath(const PPEdiMessageEntry & rEntry, SString & rBuf) const
{
	return (rBuf = TempPath).SetLastSlash().Cat(rEntry.Box).SetLastSlash().Cat(rEntry.SId);
}

int ImportCls::GetMessageList(const char * pLocalPath)
{
	MsgList.Clear();

	int    ok = -1;
	SString file_type, box_name;
	SString wildcard;
	if(MessageType == PPEDIOP_ORDERRSP) {
		file_type = "OrdRsp";
		box_name = INBOX;
	}
	else if(MessageType == PPEDIOP_DESADV) {
		file_type = "Desadv";
		box_name = INBOX;
	}
	else if(MessageType == PPEDIOP_APERAK) {
		file_type = "Aperak";
		box_name = REPORTSBOX;
	}
	else if(MessageType == PPEDIOP_ALCODESADV) {
		file_type = "AlcoDesadv";
		box_name = INBOX;
	}
	{
		int   gfr = 0;
		SString dest_path, src_path;
		if(pLocalPath) {
			SDirEntry de;
			(wildcard = pLocalPath).SetLastSlash().Cat(box_name).SetLastSlash().Cat(file_type).Cat("*.xml");
            for(SDirec dir(wildcard, 0); dir.Next(&de) > 0;) {
				PPEdiMessageEntry eme;
				if(ParseFileName(de.FileName, &eme) && (!MessageType || MessageType == eme.EdiOp)) {
					(src_path = pLocalPath).SetLastSlash().Cat(box_name).SetLastSlash().Cat(de.FileName);
					(dest_path = TempPath).SetLastSlash().Cat(de.FileName);
					gfr = SCopyFile(src_path, dest_path, 0, FILE_SHARE_READ, 0);
					if(gfr) {
						eme.Dtm = de.WriteTime;
						STRNSCPY(eme.Box, box_name);
						MsgList.Add(eme);
						ok = 1;
					}
				}
            }
		}
		else {
			if(!P_FtpCli) {
				P_FtpCli = new FtpClient(Header.EdiLogin, Header.EdiPassword);
				THROW(P_FtpCli);
				if(!P_FtpCli->Connect()) {
					ProcessError("FtpConnect");
					CALLEXCEPT();
				}
			}
			{
				SDirEntry de;
				(wildcard = box_name).SetLastSlash().Cat(file_type).Cat("*.xml");
				for(SEnum en = P_FtpCli->Enum(wildcard); en.Next(&de) > 0;) {
					PPEdiMessageEntry eme;
					if(ParseFileName(de.FileName, &eme) && (!MessageType || MessageType == eme.EdiOp)) {
						(src_path = box_name).SetLastSlash().Cat(de.FileName);
						(dest_path = TempPath).SetLastSlash().Cat(de.FileName);
						gfr = P_FtpCli->GetFile(src_path, dest_path);
						if(gfr) {
							eme.Dtm = de.WriteTime;
							STRNSCPY(eme.Box, box_name);
							MsgList.Add(eme);
							ok = 1;
						}
					}
				}
			}
		}
	}
	State |= stMsgList;
	CATCHZOK
	if(!ok)
		SysLogMessage(SYSLOG_RECEIVEDOC);
	return ok;
}
//
// Descr: ������ ����������, ����������� � ������� Receive() ��� APERAK
// Retruns:
//		 0 - ������
//		 1 - ��� ��������� �������
//
int ImportCls::ParseAperakResp()
{
	int    ok = 1, is_correct = 0, exit_while = 0;
	SString str;
	xmlDoc * p_doc = 0;
	AperakInfo.Clear();
	THROWERR_STR(fileExists(ImpFileName), IEERR_IMPFILENOTFOUND, ImpFileName);
	THROWERR((p_doc = xmlReadFile(ImpFileName, NULL, XML_PARSE_NOENT)), IEERR_NULLREADXMLPTR);
	xmlNode * p_node = xmlDocGetRootElement(p_doc);
	THROWERR(p_node, IEERR_XMLREAD);
	if(SXml::IsName(p_node, "APERAK") && p_node->children) // �� ������� ���� ����� ������, ��� ��� Aperak
		is_correct = 1;
	while(is_correct && p_node && p_node->type == XML_ELEMENT_NODE) {
		if(p_node->children && p_node->children->type == XML_READER_TYPE_ELEMENT)
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
			if(is_correct && p_node->children) {
				if(SXml::IsName(p_node, "E0065")) {
					THROWERR_STR(SXml::IsContent(p_node->children, "APERAK"), IEERR_INVMESSAGEYTYPE, "APERAK");
				}
				else if(SXml::IsName(p_node, "E1153")) {
					if(SXml::IsContent(p_node->children, "ON")) {
						if(SXml::IsName(p_node->next, "E1154") && p_node->next->children) // ����� ��������� ������
							AperakInfo.OrderNum.Set(p_node->next->children->content);
					}
				}
				else if(SXml::IsName(p_node, "E2005")) {
					if(SXml::IsContent(p_node->children, "171")) {
						if(SXml::IsName(p_node->next, "E2380") && p_node->next->children) { // ���� ��������� ������
							strtodate(str.Set(p_node->next->children->content).Trim(8), DATF_YMD|DATF_CENTURY, &AperakInfo.OrderDate);
						}
					}
				}
				else if(SXml::IsName(p_node, "E3035")) {
					// �������� �������� ��������
					str.Set(p_node->children->content);
					p_node = p_node->next;
					if(SXml::IsName(p_node, "C082") && SXml::IsName(p_node->children, "E3039")) {
						if(str.CmpNC("BY") == 0)
							AperakInfo.BuyerGLN.Set(p_node->children->children->content); // GLN ����������
						else if(str.CmpNC("SU") == 0)
							AperakInfo.SupplGLN.Set(p_node->children->children->content); // GLN ����������
						else if(str.CmpNC("DP") == 0)
							AperakInfo.AddrGLN.Set(p_node->children->children->content);  // GLN ������ ��������
					}
				}
				else if(SXml::IsName(p_node, "E9321")) { // ��� �������
					AperakInfo.Code = (const char *)p_node->children->content;
				}
				else if(SXml::IsName(p_node, "E4451")) {
					if(strcmp((const char *)p_node->children->content, "AAO") == 0) {
						if(p_node->next) {
							// ���� ������� ����� ����������� ��� ����. � ������ ����� ����� ���������, �� ������ - ��������� ��������
							if(SXml::IsName(p_node->next->children, "E4440") && p_node->next->children->children) {
								// �������� �������� �������
								AperakInfo.Msg = (const char *)p_node->next->children->children->content;
								if(SXml::IsName(p_node->next->children->next, "E4440") && p_node->next->children->next->children)
									AperakInfo.AddedMsg.Set(p_node->next->children->next->children->content);
							}
						}
					}
				}
			}
		}
	}
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
// ��������� ���������� �������� � ��������� Sdr_Bill
//
int ImportCls::ParseForDocData(Sdr_Bill * pBill)
{
	int    ok = 0, exit_while = 0;
	SString str;
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
	pBill->EdiOp = MessageType;
	// } @v8.5.6
	p_node = p_root->children;
	while(p_node && p_node->type == XML_ELEMENT_NODE) {
		if(p_node->children && p_node->children->type == XML_READER_TYPE_ELEMENT)
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
		if(p_node && p_node->type == XML_ELEMENT_NODE && p_node->children) {
			if(SXml::IsName(p_node, "E0065")) {
				if(MessageType == PPEDIOP_ORDERRSP) {
					THROWERR_STR(SXml::IsContent(p_node->children, "ORDRSP"), IEERR_INVMESSAGEYTYPE, "ORDRSP")
				}
				else if(MessageType == PPEDIOP_DESADV) {
					THROWERR_STR(SXml::IsContent(p_node->children, "DESADV"), IEERR_INVMESSAGEYTYPE, "DESADV");
				}
			}
			else if(SXml::IsName(p_node, "E1004")) {
				// ����� ����� ��������� � Papyrus (��-��, ������ � Win1251)
				str.Set(p_node->children->content).Utf8ToChar().CopyTo(pBill->Code, sizeof(pBill->Code));
				ok = 1;
			}
			else if(SXml::IsName(p_node, "C506") && p_node->children->type == XML_READER_TYPE_ELEMENT) {
				p_node = p_node->children; // <E1153>
				if(SXml::IsName(p_node, "E1153") && p_node->children) {
					str.Set(p_node->children->content); // �������� �������� ��������
					p_node = p_node->next; // <E1154>
					if(SXml::IsName(p_node, "E1154") && p_node->children) {
						if(str.CmpNC("ON") == 0) { // ����� ������, �� ������� ������ �������������
							STRNSCPY(pBill->OrderBillNo, p_node->children->content);
							ok = 1;
						}
						else if(str.CmpNC("AAS") == 0) { // ����� ��� (������ ��� ������ ��� DESADV)
							TTN.Set(p_node->children->content).Utf8ToOem();
							ok = 1;
						}
					}
				}
			}
			else if(SXml::IsName(p_node, "C507") && p_node->children->type == XML_READER_TYPE_ELEMENT) {
				p_node = p_node->children; // <E2005>
				if(SXml::IsName(p_node, "E2005") && p_node->children) {
					str.Set(p_node->children->content); // �������� �������� ��������
					p_node = p_node->next; // <E2380>
					if(SXml::IsName(p_node, "E2380") && p_node->children) {
						//
						// Note: ������ � ����� ����� ������: YYYYMMDD. ��� ���� �������� ������� ��������� �����.
						//   ��-�� ����� �������������� �������� ������ ���� �� 8 �������� (������� strtodate ��������� ���������
						//   ���� ��� ������������ ������ ��� ������������� ������� ������ � 8 ��������).
						//
						if(str == "137") { // ���� ���������
							strtodate(str.Set(p_node->children->content).Trim(8), DATF_YMD|DATF_CENTURY, &pBill->Date);
							ok = 1;
						}
						else if(str == "17" || str == "2") { // ���� �������� (���� ���������� ���������)
							strtodate(str.Set(p_node->children->content).Trim(8), DATF_YMD|DATF_CENTURY, &pBill->DueDate);
							ok = 1;
						}
						else if(str == "171") { // ���� ������
							if(!pBill->OrderDate)
								strtodate(str.Set(p_node->children->content).Trim(8), DATF_YMD|DATF_CENTURY, &pBill->OrderDate);
							ok = 1;
						}
					}
				}
			}
			else if(SXml::IsName(p_node, "E3035")) {
				str.Set(p_node->children->content); // �������� �������� ��������
				p_node = p_node->next; // <C082>
				if(SXml::IsName(p_node, "C082") && p_node->children) {
					p_node = p_node->children; // <E3039>
					if(SXml::IsName(p_node, "E3039") && p_node->children) {
						if(str.CmpNC("BY") == 0) { // GLN ����������
							STRNSCPY(pBill->MainGLN, p_node->children->content);
							STRNSCPY(pBill->AgentGLN, p_node->children->content);
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
			else if(SXml::IsName(p_node, "E1229")) { // ��� �������� (���������, ������� ��� ���������, �� �������)
				str.Set(p_node->children->content);
				ok = 1;
			}
			else if(SXml::IsName(p_node, "C516") && p_node->children->type == XML_READER_TYPE_ELEMENT) {
				p_node = p_node->children; // <E5025>
				if(SXml::IsName(p_node, "E5025") && p_node->children) {
					str.Set(p_node->children->content); // �������� �������� �������� ��������
					p_node = p_node->next; // <E5004>
					if(SXml::IsName(p_node, "E5004") && p_node->children) {
						if(str == "128") { // ����� ��������� � ���
							pBill->Amount = atof((const char *)p_node->children->content);
							ok = 1;
						}
						else if(str == "98") { // ����� ��������� ��� ���
							str.Set(p_node->children->content);
							ok = 1;
						}
					}
				}
			}
			else if(SXml::IsName(p_node, "E6069")) {
				if(SXml::IsContent(p_node->children, "2")) {
					p_node = p_node->next; // <E6060>
					if(SXml::IsName(p_node, "E6066") && p_node->children) { // ���������� �������� ������� � ���������
						GoodsCount = atoi((const char *)p_node->children->content);
						ok = 1;
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
int ImportCls::ParseForGoodsData(Sdr_BRow * pBRow)
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
		if(MessageType == PPEDIOP_ORDERRSP)
			goods_segment = "SG26";
		else if(MessageType == PPEDIOP_DESADV)
			goods_segment = "SG17";
		else if(MessageType == PPEDIOP_ALCODESADV)
			goods_segment = "SG17";
		while(p_node && !sg26_end) {
			if(p_node->children && p_node->children->type == XML_READER_TYPE_ELEMENT)
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
			//
			// ��������� ������� ��������� ������ �������� �������
			//
			if(p_node && p_node->type == XML_READER_TYPE_ELEMENT) {
				if(SXml::IsName(p_node, goods_segment) && p_node->children) {
					if(index == (Itr.GetCount() + 1)) {
						while(p_node && !sg26_end) {
							exit_while = 0;
							if(p_node->children && p_node->children->type == XML_READER_TYPE_ELEMENT)
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
								else if(p_node->children) {
									if(SXml::IsName(p_node, "LIN") && p_node->children->type == XML_READER_TYPE_ELEMENT) {
										p_node = p_node->children; // <E1082>
										if(SXml::IsName(p_node, "E1082") && p_node->children) {
                                            pBRow->LineNo = str.Set(p_node->children->content).ToLong(); // ����� �������� �������
											if(MessageType == PPEDIOP_ORDERRSP) {
												p_node = p_node->next; // <E1229>
												if(SXml::IsName(p_node, "E1229") && p_node->children) { // ������ �������� �������
													str.Set(p_node->children->content);
												}
											}
										}
									}
									else if(SXml::IsName(p_node, "C212") && p_node->children->type == XML_READER_TYPE_ELEMENT) {
										p_node = p_node->children; // <E7140>
										if(SXml::IsName(p_node, "E7140") && p_node->children) {
											str.Set(p_node->children->content); // �������� �������� �������� �������� (��������)
											p_node = p_node->next; // <E7143>
											if(SXml::IsName(p_node, "E7143") && SXml::IsContent(p_node->children, "SRV")) { // �������� ������
												STRNSCPY(pBRow->Barcode, str);
											}
										}
									}
									else if(SXml::IsName(p_node, "E4347") && SXml::IsContent(p_node->children, "1")) {
										if(p_node->next && p_node->next->children && p_node->next->children->type == XML_READER_TYPE_ELEMENT) {
											p_node = p_node->next->children; // <E7140> (��������� <C212>)
											if(SXml::IsName(p_node, "E7140") && p_node->children) {
												str.Set(p_node->children->content); // �������� �������� �������� �������� (������� ������ � ����������)
												p_node = p_node->next; // <E7143>
												if(SXml::IsName(p_node, "E7143") && SXml::IsContent(p_node->children, "SA")) { // ������� ����������
													STRNSCPY(pBRow->ArCode, str);
												}
												else if(SXml::IsName(p_node, "E7143") && SXml::IsContent(p_node->children, "MF")) { // ��� ���� ����-���������
													STRNSCPY(pBRow->AlcoCatCode, str);
												}
											}
										}
									}
									else if(SXml::IsName(p_node, "E7008")) { // ������������ ������
										str.Set(p_node->children->content).Utf8ToOem().CopyTo(pBRow->GoodsName, sizeof(pBRow->GoodsName));
									}
									else if(SXml::IsName(p_node, "C186") && p_node->children->type == XML_READER_TYPE_ELEMENT) {
										p_node = p_node->children; // <E6063>
										if(SXml::IsName(p_node, "E6063") && p_node->children) {
											str.Set(p_node->children->content); // �������� �������� �������� ��������
											p_node = p_node->next; // <E6060>
											if(SXml::IsName(p_node, "E6060") && p_node->children) {
												// "113" - ������������� ��������������� ���������� (������������� ����� ����� �� ������������)
												// "170" - ������������� ��������������� ���������� (������������� ����� ����� �� ������������)
												// "12"  - ������������� ������������ ����������
												// "59"  - ������������� ���������� ������ � ��������
												if(str == "21") // ���������� ����������
													str.Set(p_node->children->content);
												else if(str == "113" || str == "170" || str == "12") // �������������� ����������
													pBRow->Quantity = atof((const char *)p_node->children->content);
												else if(str == "59") // ���������� ������ � ��������
													pBRow->UnitPerPack = atof((const char *)p_node->children->content);
											}
										}
									}
									else if(SXml::IsName(p_node, "E6411")) { // ������� ������
										if(SXml::IsContent(p_node->children, "KGM"))
											STRNSCPY(pBRow->UnitName, UNIT_NAME_KG);
										else
											STRNSCPY(pBRow->UnitName, UNIT_NAME_PIECE);
									}
									else if(SXml::IsName(p_node, "C516") && p_node->children->type == XML_READER_TYPE_ELEMENT) {
										p_node = p_node->children; // <E5025>
										if(SXml::IsName(p_node, "E5025") && p_node->children) {
											str.Set(p_node->children->content); // �������� �������� �������� ��������
											if(p_node->next) {
												p_node = p_node->next; // <E5004>
												if(SXml::IsName(p_node, "E5004") && p_node->children) {
													if(str == "203") // ����� �������� ������� ��� ���
														str.Set(p_node->children->content);
													else if(str == "79") // ����� �������� ������� � ���
														str.Set(p_node->children->content);
													else if(MessageType == PPEDIOP_DESADV && str.CmpNC("XB5") == 0) // ���� ������ � ��� ��� DESADV
														pBRow->Cost = atof((const char *)p_node->children->content);
												}
											}
										}
									}
									else if(SXml::IsName(p_node, "C506") && p_node->children->type == XML_READER_TYPE_ELEMENT) {
										p_node = p_node->children; // <E1153>
										if(SXml::IsName(p_node, "E1153") && SXml::IsContent(p_node->children, "FC")) {
											p_node = p_node->next;
											if(SXml::IsName(p_node, "E1154") && p_node->children) { // ��� �������������/���������
                                                STRNSCPY(pBRow->ManufINN, str.Set(p_node->children->content));
											}
										}
										else if(SXml::IsName(p_node, "E1153") && SXml::IsContent(p_node->children, "XA")) {
											p_node = p_node->next;
											if(SXml::IsName(p_node, "E1154") && p_node->children) { // ��� �������������/���������
                                                STRNSCPY(pBRow->ManufKPP, str.Set(p_node->children->content));
											}
										}
									}
									else if(SXml::IsName(p_node, "NAD") && p_node->children->type == XML_READER_TYPE_ELEMENT) {
										p_node = p_node->children; // <E3035>
										if(SXml::IsName(p_node, "E3035") && SXml::IsContent(p_node->children, "MF")) {
											p_node = p_node->next;
											if(SXml::IsName(p_node, "C080") && SXml::IsName(p_node->children, "E3036")) {
												if(p_node->children->children)
													STRNSCPY(pBRow->LotManuf, str.Set(p_node->children->children->content));
											}
										}
									}
									else if(SXml::IsName(p_node, "C509") && p_node->children->type == XML_READER_TYPE_ELEMENT) {
										p_node = p_node->children; // <E5125>
										if(SXml::IsName(p_node, "E5125") && p_node->children) {
											str.Set(p_node->children->content); // �������� �������� �������� ��������
											p_node = p_node->next; // <E5118>
											if(SXml::IsName(p_node, "E5118") && p_node->children) {
												if(str.CmpNC("AAA") == 0) // ���� ������ ��� ���
													str.Set(p_node->children->content);
												else if(str.CmpNC("AAE") == 0) // ���� ������ � ��� ��� ORDRSP
													pBRow->Cost = atof((const char *)p_node->children->content);
											}
										}
									}
								}
							}
						}
					}
					if(MessageType == PPEDIOP_DESADV) {
						if(TTN.NotEmpty()) // ����� ���. �������� �� ��������� ���������
							STRNSCPY(pBRow->TTN, TTN);
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
// ������� ������� �������
//
EXPORT int InitImport(void * pImpHeader, const char * pInputFileName, int * pId)
{
	int    ok = 1;
	SString temp_buf;
	SETIFZ(P_ImportCls, new ImportCls);
	if(P_ImportCls && !(P_ImportCls->State & ImportCls::stInit)) {
		SPathStruc inp_ps;
		if(pImpHeader) {
			P_ImportCls->Header = *static_cast<const Sdr_ImpExpHeader *>(pImpHeader);
			FormatLoginToLogin(P_ImportCls->Header.EdiLogin, temp_buf.Z());
			temp_buf.CopyTo(P_ImportCls->Header.EdiLogin, sizeof(P_ImportCls->Header.EdiLogin));
		}
		if(!isempty(pInputFileName)) {
			inp_ps.Split(pInputFileName);
			if(inp_ps.Nam.Empty())
				(inp_ps.Nam = "kontur_import");
			if(inp_ps.Ext.Empty())
				inp_ps.Ext = "xml";
		}
		else {
			//char   fname[256];
			//GetModuleFileName(NULL, fname, sizeof(fname));
			SString module_file_name;
			SSystem::SGetModuleFileName(0, module_file_name);
			inp_ps.Split(module_file_name);
			inp_ps.Dir.ReplaceStr("\\bin", "\\in", 1);
			(inp_ps.Nam = "kontur_import");
			inp_ps.Ext = "xml";
		}
		{
			SPathStruc ps;
			{
				ps.Copy(&inp_ps, SPathStruc::fDrv|SPathStruc::fDir|SPathStruc::fNam|SPathStruc::fExt);
				ps.Nam = "import_log";
				ps.Ext = "txt";
				ps.Merge(LogName);
				ps.Nam = "system_log";
				ps.Merge(SysLogName);
			}
			{
				ps.Nam.Z();
				ps.Ext.Z();
				ps.Copy(&inp_ps, SPathStruc::fDrv|SPathStruc::fDir);
				ps.Merge(P_ImportCls->TempPath);
				P_ImportCls->TempPath.SetLastSlash().Cat("temp");
				THROW(::createDir(P_ImportCls->TempPath));
			}
		}
		P_ImportCls->Id = 1;
		*pId = P_ImportCls->Id; // �� ������ �������
		P_ImportCls->State |= ImportCls::stInit;
	}
	THROWERR(P_ImportCls, IEERR_IMPEXPCLSNOTINTD);
	CATCH
		SysLogMessage(SYSLOG_INITIMPORT);
		GetErrorMsg(temp_buf.Z());
		SysLogMessage(temp_buf);
		LogMessage(temp_buf);
		ok = 0;
	ENDCATCH;
	return ok;
}

EXPORT int GetImportObj(uint idSess, const char * pObjTypeSymb, void * pObjData, int * pObjId, const char * pMsgType = 0)
{
	//const char * p_local_path = "D:/Papyrus/ppy/out/kontur/test/s-(33633)"; // @debug
	const char * p_local_path = 0;

	int    ok = 1, r = 0;
	SString str;
	Sdr_Bill * p_rec = (Sdr_Bill *)pObjData;
	THROWERR(P_ImportCls, IEERR_IMPEXPCLSNOTINTD);
	THROWERR(idSess == P_ImportCls->Id, IEERR_NOSESS);
	THROWERR(pObjData, IEERR_NODATA);
	THROWERR(GetObjTypeBySymb(pObjTypeSymb, P_ImportCls->ObjType), IEERR_SYMBNOTFOUND);
	THROWERR(P_ImportCls->ObjType == objBill, IEERR_ONLYBILLS);
	// ������� ��� ���������
	THROWERR(pMsgType, IEERR_MSGSYMBNOTFOUND);
	THROWERR(GetMsgTypeBySymb(pMsgType, P_ImportCls->MessageType), IEERR_MSGSYMBNOTFOUND);
	THROWERR(oneof4(P_ImportCls->MessageType, PPEDIOP_ORDERRSP, PPEDIOP_APERAK, PPEDIOP_DESADV, PPEDIOP_ALCODESADV), IEERR_ONLYIMPMSGTYPES);
	THROW(r = P_ImportCls->ReceiveDoc(p_local_path, pObjId));
	if(r == -1)
		ok = -1;
	else if(oneof3(P_ImportCls->MessageType, PPEDIOP_ORDERRSP, PPEDIOP_DESADV, PPEDIOP_ALCODESADV) && r == 1) { // ��� ���� r == -2, �� ��� �� ���� �������� ��������� ����, ��� ��� ���
		THROW(P_ImportCls->ParseForDocData(p_rec)); // ������ �������� � ��������� Sdr_Bill
	}
	else if(P_ImportCls->MessageType == PPEDIOP_APERAK) {
		// ����� ��������� pObjData �� ��������� P_ImportCls->AperakInfo
		// ��� ����� ������� ����� ��������� � GLN ������ ��������, ���� ���������. ����� ���������� ��� ����������� �������������� ������� ����������� ��������
		// GLN ����������
		STRNSCPY(p_rec->OrderBillNo, P_ImportCls->AperakInfo.OrderNum);
		STRNSCPY(p_rec->DlvrAddrCode, P_ImportCls->AperakInfo.AddrGLN);
		STRNSCPY(p_rec->LocCode, P_ImportCls->AperakInfo.AddrGLN);
		STRNSCPY(p_rec->MainGLN, P_ImportCls->AperakInfo.BuyerGLN);
		STRNSCPY(p_rec->GLN, P_ImportCls->AperakInfo.SupplGLN);
		p_rec->OrderDate = P_ImportCls->AperakInfo.OrderDate;
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
	// THROWERR(objId == P_ImportCls->ObjId, IEERR_NOOBJID);
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
	//THROWERR(objId == P_ImportCls->ObjId, IEERR_NOOBJID);
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

int ImportCls::ReplyImportObjStatus(uint idSess, uint objId, void * pObjStatus)
{
	int    ok = 1, r = 1;
	size_t pos = 0;
	SString str;
	const  int32 doc_status = ((Sdr_DllImpObjStatus *)pObjStatus)->DocStatus;
	uint   msg_list_pos = 0;
	if(MsgList.SearchId((int)objId, &msg_list_pos)) {
        PPEdiMessageEntry & r_eme = MsgList.at(msg_list_pos);
		//
		// ���� � Papyrus ���� �����, �� ������� �������� ������������� ��� ������
		//
		if(doc_status == docStatIsSuchDoc) {
			if(r_eme.EdiOp == PPEDIOP_APERAK) {
				// ���-������ ������ � ���� ��������
				str.Z().Cat(AperakInfo.OrderNum.Transf(CTRANSF_INNER_TO_OUTER)).CatDiv(':', 2).Cat(AperakInfo.Msg.Utf8ToChar());
				if(AperakInfo.AddedMsg.NotEmpty())
					str.CatDiv(':', 2).Cat(AperakInfo.AddedMsg.Utf8ToChar());
				LogMessage(str);
			}
			// � ������� ���� ���� �� ����� �� ftp
			FtpClient ftp_client(Header.EdiLogin, Header.EdiPassword);
			if(ftp_client.Connect()) {
				/* @debug ��� ������� �� ������� �����*/
				GetFtpPath(r_eme, str);
				if(!ftp_client.DeleteFile(str)) {
					ProcessError("FtpDeleteFile");
					r = 0;
				}
				/**/
			}
			else {
				ProcessError("FtpConnect");
				r = 0;
			}
		}
		// ���� � Papyrus ������ ������ ���
		else if(doc_status == docStatNoSuchDoc) {
			if(oneof2(r_eme.EdiOp, PPEDIOP_ORDERRSP, PPEDIOP_DESADV)) {
                GetTempPath(r_eme, str);
				SFile::Remove(str); // ������� ��������� ����� xml-��������
			}
			else if(r_eme.EdiOp == PPEDIOP_APERAK) {
				//
				// ���� �� ������ ����� ������ ��� GLN ������ ��������, �� ������� ���� ���� �� FTP,
				// ��� �� ��� ����� �� ����� ��������� �� ����� ���������
				//
				if(AperakInfo.AddrGLN.Empty() || AperakInfo.OrderNum.Empty()) {
					FtpClient ftp_client(Header.EdiLogin, Header.EdiPassword);
					if(ftp_client.Connect()) {
						GetFtpPath(r_eme, str);
						if(!ftp_client.DeleteFile(str)) {
							ProcessError("FtpDeleteFile");
							r = 0;
						}
					}
					else {
						ProcessError("FtpConnect");
						r = 0;
					}
				}
			}
		}
		if(!r) {
			SysLogMessage(SYSLOG_REPLYIMPORTOBJSTATUS);
			GetErrorMsg(str.Z());
			LogMessage(str);
			SysLogMessage(str);
		}
		//InboxReadIndex++;
	}
	return ok;
}
//
// ���������� Papyrus'�� ����� ������� GetImportObj. ����������� Papyrus'� ������� ���� ����� ��� �������
//
EXPORT int ReplyImportObjStatus(uint idSess, uint objId, void * pObjStatus)
{
	return P_ImportCls ? P_ImportCls->ReplyImportObjStatus(idSess, objId, pObjStatus) : 1;
}
//
// ����� ������� �������
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
		P_ImportCls->Finish();
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
	str.CopyTo(pMsg, bufLen);
	ErrorCode = 0;
	StrError = 0;
	return 1;
}

/*void ProcessError(const char * pProcess)
{
	char   temp_err_buf[256];
	SString temp_buf;
	ErrorCode = IEERR_FTP;
	DWORD code = GetLastError();
	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, code, MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), temp_err_buf, 256, 0);
	(temp_buf = pProcess).CatDiv(':', 2).Cat(temp_err_buf);
	// ������� �������������� �������� ������
	if(code == ERROR_INTERNET_EXTENDED_ERROR) {
		DWORD  size = 256;
		MEMSZERO(temp_err_buf);
		code = 0;
		::InternetGetLastResponseInfo(&code, temp_err_buf, &size);
		temp_buf.CatDiv(':', 2).Cat(temp_err_buf);
	}
	StrError = temp_buf;
}*/
void ProcessError(const char * pProcess)
{
	StrError.Z();
	//char   temp_err_buf[256];
	//SString temp_buf;
	SString sys_err_msg;
	ErrorCode = IEERR_FTP;
	DWORD code = GetLastError();
	//::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, code, MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), temp_err_buf, 256, 0);
	//(temp_buf = pProcess).CatDiv(':', 2).Cat(temp_err_buf);
	// @v10.3.11 {
	SSystem::SFormatMessage(code, sys_err_msg);
	(StrError = pProcess).CatDiv(':', 2).Cat(sys_err_msg);
	// } @v10.3.11 
	/* @v10.3.11 (SSystem::SFormatMessage has done it)
	// ������� �������������� �������� ������
	if(code == ERROR_INTERNET_EXTENDED_ERROR) {
		DWORD size = 256;
		MEMSZERO(temp_err_buf);
		code = 0;
		::InternetGetLastResponseInfo(&code, temp_err_buf, &size);
		temp_buf.CatDiv(':', 2).Cat(temp_err_buf);
	}
	StrError = temp_buf;
	*/
}
