// IMPEXPEXITE.CPP
// ���������� ��� �������/�������� ���������� � xml ����� ���������� Exite

// � APERAK � ��� ����������� �� ���� ������, � ���� ����������� �������� � ���.
// ��� ��� ������ ������ ��� ���� �� ��������, ����� Papyrus ����� ������������� ������
// �� ��������� ������ �� ������� ����.

#include <slib.h>
#include <ppbrow.h>
#include <sxml.h>
#include <snet.h>
#include <winsock2.h>
#include <wininet.h>

#define EXPORT	extern "C" __declspec (dllexport)
#define THROWERR(expr,val) { if(!(expr)){ SetError(val, ""); goto __scatch; } }
#define THROWERR_STR(expr,val,str) { if(!(expr)){ SetError(val, str); goto __scatch; } }

#define UNIT_NAME_KG			"��"
#define UNIT_NAME_PIECE			"�����"
#define INBOX					"cinbox"
#define OUTBOX					"coutbox"
//#define REPORTSBOX				"Reports"

// ����� ��������� ���������
#define ELEMENT_NAME_ORDERS		"ORDERS"
#define ELEMENT_NAME_RECADV     "RECADV"
#define ELEMENT_NAME_DOCAPERAK  "APERAK"
#define ELEMENT_NAME_UNH		"UNH"
#define ELEMENT_NAME_S009		"S009"
#define ELEMENT_NAME_E0065		"E0065"
#define ELEMENT_NAME_E0052		"E0052"
#define ELEMENT_NAME_E0054		"E0054"
#define ELEMENT_NAME_E0051		"E0051"
#define ELEMENT_NAME_E0057		"E0057"
#define ELEMENT_NAME_BGM		"BGM"
#define ELEMENT_NAME_C002		"C002"
#define ELEMENT_NAME_E1001		"E1001"
#define ELEMENT_NAME_C106		"C106"
#define ELEMENT_NAME_E1004		"E1004"
#define ELEMENT_NAME_E1225		"E1225"
#define ELEMENT_NAME_DTM		"DTM"
#define ELEMENT_NAME_C507		"C507"
#define ELEMENT_NAME_E2005		"E2005"
#define ELEMENT_NAME_SG2		"SG2"
#define ELEMENT_NAME_SG1		"SG1"
#define ELEMENT_NAME_SG4        "SG4"
#define ELEMENT_NAME_RFF		"RFF"
#define ELEMENT_NAME_C506		"C506"
#define ELEMENT_NAME_E1153		"E1153"
#define ELEMENT_NAME_E1154		"E1154"
#define ELEMENT_NAME_SG3		"SG3"
#define ELEMENT_NAME_NAD		"NAD"
#define ELEMENT_NAME_E3035		"E3035"
#define ELEMENT_NAME_SG7		"SG7"
#define ELEMENT_NAME_CUX		"CUX"
#define ELEMENT_NAME_C504		"C504"
#define ELEMENT_NAME_E6347		"E6347"
#define ELEMENT_NAME_E6345		"E6345"
#define ELEMENT_NAME_E6343		"E6343"
#define ELEMENT_NAME_SG28		"SG28"
#define ELEMENT_NAME_SG16       "SG16"
#define ELEMENT_NAME_C212		"C212"
#define ELEMENT_NAME_E7140		"E7140"
#define ELEMENT_NAME_E7143		"E7143"
#define ELEMENT_NAME_PIA		"PIA"
#define ELEMENT_NAME_E4347		"E4347"
#define ELEMENT_NAME_IMD		"IMD"
#define ELEMENT_NAME_E7077		"E7077"
#define ELEMENT_NAME_C273		"C273"
#define ELEMENT_NAME_E7008		"E7008"
#define ELEMENT_NAME_C082		"C082"
#define ELEMENT_NAME_E3039		"E3039"
#define ELEMENT_NAME_SG26		"SG26"
#define ELEMENT_NAME_S17        "SG17"
#define ELEMENT_NAME_CPS        "CPS"
#define ELEMENT_NAME_E7164      "E7164"
#define ELEMENT_NAME_LIN		"LIN"
#define ELEMENT_NAME_E1082		"E1082"
#define ELEMENT_NAME_E1229		"E1229"
#define ELEMENT_NAME_QTY		"QTY"
#define ELEMENT_NAME_C186		"C186"
#define ELEMENT_NAME_E6063		"E6063"
#define ELEMENT_NAME_E6060		"E6060"
#define ELEMENT_NAME_E6411		"E6411"
#define ELEMENT_NAME_SG32		"SG32"
#define ELEMENT_NAME_SG38		"SG38"
#define ELEMENT_NAME_C241		"C241"
#define ELEMENT_NAME_TAX		"TAX"
#define ELEMENT_NAME_E5153		"E5153"
#define ELEMENT_NAME_E5278		"E5278"
#define ELEMENT_NAME_E5283		"E5283"
#define ELEMENT_NAME_C243		"C243"
#define ELEMENT_NAME_MOA		"MOA"
#define ELEMENT_NAME_C516		"C516"
#define ELEMENT_NAME_E5025		"E5025"
#define ELEMENT_NAME_E5004		"E5004"
#define ELEMENT_NAME_CNT		"CNT"
#define ELEMENT_NAME_C270		"C270"
#define ELEMENT_NAME_SG30		"SG30"
#define ELEMENT_NAME_PRI		"PRI"
#define ELEMENT_NAME_C509		"C509"
#define ELEMENT_NAME_E5125		"E5125"
#define ELEMENT_NAME_E5118		"E5118"
#define ELEMENT_NAME_UNS		"UNS"
#define ELEMENT_NAME_E0081		"E0081"
#define ELEMENT_NAME_E6069		"E6069"
#define ELEMENT_NAME_E6066		"E6066"
#define ELEMENT_NAME_UNT		"UNT"
#define ELEMENT_NAME_E0074		"E0074"
#define ELEMENT_NAME_E0062		"E0062"
#define ELEMENT_NAME_�516		"C516"
#define ELEMENT_NAME_E3055		"E3055"
#define ELEMENT_NAME_E9321		"E9321"
#define ELEMENT_NAME_E4451		"E4451"
#define ELEMENT_NAME_E4441		"E4441"
#define ELEMENT_NAME_E4440		"E4440"

// �������� ���������-��������������
#define ELEMENT_CODE_E0065_ORDERS	"ORDERS"	// ��� ��������� - �����
#define ELEMENT_CODE_E0065_RECADV   "RECADV"    // ��� ��������� - ����������� � �������
#define ELEMENT_CODE_E0065_ORDRSP	"ORDRSP"	// ��� ��������� - ������������� ������
#define ELEMENT_CODE_E0065_DESADV   "DESADV"    // ��� �������� - ����������� �� ��������
#define ELEMENT_CODE_E0065_APERAK	"APERAK"	// ��� ��������� - �������� �������
#define ELEMENT_CODE_E0052_D		"D"			// ������ ���������
#define ELEMENT_CODE_E0054_01B		"01B"		// ������ �������
#define ELEMENT_CODE_E0051_UN		"UN"		// ��� ������� �����������
#define ELEMENT_CODE_E0057_EAN010	"EAN010"	// ���, ����������� ������� ������������ (��� ORDERS)
#define ELEMENT_CODE_E0057_EAN006	"EAN006"	// ���, ����������� ������� ������������ (��� RECADV)
#define ELEMENT_CODE_E1001_220		"220"		// ��� ��������� - �����
#define ELEMENT_CODE_E1001_632		"632"		// ��� ��������� - ����������� � �������
#define ELEMENT_CODE_E1225_9		"9"			// ��� ������� ��������� - ��������
#define ELEMENT_CODE_E4347_1		"1"			// �������������� �������������
#define ELEMENT_CODE_E2005_2		"2" 		// ����/����� �������� (ORDRSP) (�������� ��� �������������)
#define ELEMENT_CODE_E2005_17		"17"		// ����/����� �������� (ORDRSP)	(�������� ��� �������������)
#define ELEMENT_CODE_E2005_137		"137"		// ����/����� ���������/���������
#define ELEMENT_CODE_E2005_50		"50"		// ����/����� �������
#define ELEMENT_CODE_E2005_2		"2"			// ����/����� �������� (ORDERS)
#define ELEMENT_CODE_E2379_102		"102"		// ������ ����/������� - CCYYMMDD
#define ELEMENT_CODE_E3035_SU		"SU"		// ������������� ����������
#define ELEMENT_CODE_E3035_BY		"BY"		// ������������� ����������
#define ELEMENT_CODE_E3035_DP		"DP"		// ������������� ����� ��������
#define ELEMENT_CODE_E3035_IV		"IV"		// ������������� �����������
#define ELEMENT_CODE_E3055_9		"9"			// EAN (������������� ���������� �������� ���������)
#define ELEMENT_CODE_E6347_2		"2"			// ��������� ������
#define ELEMENT_CODE_E6345_RUB		"RUB"		// �����
#define ELEMENT_CODE_E6343_9		"9"			// ������ ������
#define ELEMENT_CODE_E6343_4		"4"			// ������ �������� (��, �� ��� � ���� �� � RECADV)
#define ELEMENT_CODE_E7143_SRV		"SRV"		// ������������� ��������� ������
#define ELEMENT_CODE_E7143_SA		"SA"		// ������������� �������� ����������
#define ELEMENT_CODE_E7077_F		"F"			// ��� ������� �������� ������ - �����
//#define ELEMENT_CODE_E6063_21		"21"		// ������������� ����������� ����������
#define ELEMENT_CODE_E6063_194      "194"       // ������������� ��������� ����������
//#define ELEMENT_CODE_E6063_59		"59"		// ������������� ���������� ������ � ��������
//#define ELEMENT_CODE_E6063_113		"113"		// ������������� ��������������� ���������� (������������� ����� ����� �� ������������)
//#define ELEMENT_CODE_E6063_170		"170"		// ������������� ��������������� ���������� (������������� ����� ����� �� ������������)
//#define ELEMENT_CODE_E6063_12       "12"        // ������������� ������������ ����������
#define ELEMENT_CODE_E6411_PCE		"PCE"		// ������� ��������� - ��������� ��������
#define ELEMENT_CODE_E6411_KGM		"KGM"		// ������� ��������� - ����������
#define ELEMENT_CODE_E5125_AAA		"AAA"		// ������������� ���� ������ ��� ���
#define ELEMENT_CODE_E5125_XB5      "XB5"       // ������������� ���� ������ � ��� ��� DESADV
#define ELEMENT_CODE_E5125_AAE		"AAE"		// ������������� ���� ������ � ���
#define ELEMENT_CODE_E0081_S		"S"			// ���� �������� ����������
#define ELEMENT_CODE_E5025_79		"79"		// ����� ���������� �������� �������
#define ELEMENT_CODE_E6069_2		"2"			// ������������� ���������� �������� ������� � ���������
#define ELEMENT_CODE_E5025_9		"9"			// ������������� ����� ����� ��������� c ���
#define ELEMENT_CODE_E5025_98		"98"		// ������������� ����� ����� ��������� ��� ���
#define ELEMENT_CODE_E5025_128		"128"		// ������������� ����� �������� ������� � ���
#define ELEMENT_CODE_E5025_203		"203"		// ������������� ����� �������� ������� ��� ���
#define ELEMENT_CODE_E1153_ON		"ON"		// ����� ������
#define ELEMENT_CODE_E1153_AAK      "AAK"       // ����� ��� (��� RECADV)
#define ELEMENT_CODE_E1153_AAS      "AAS"		// ����� ���
#define ELEMENT_CODE_E5153_VAT		"VAT"		// ������������� ������ ���
#define ELEMENT_CODE_E5283_7		"7"			// ������������� ���������� ����������
#define ELEMENT_CODE_E4451_AAO		"AAO"		// ������������� ���� �������� �������
#define ELEMENT_CODE_E2005_171		"171"		// ������������� ���� ��������� (� APERAK)
#define ELEMENT_CODE_E7164_1        "1"         // ������� �������� �� ���������
#define ELEMENT_CODE_E1153_ABT      "ABT"       // ��� ���� - ���

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
#define IEERR_TTNNEEDED				21			// �������� �� ������. ���������� ������� ���. ��������: %s// ������ ��� �� DESADV, � �� �� ��������� ��������� �������

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
#define SYSLOG_RECADVHEADER             "in RecadvHeader()"
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
int SetError(int errCode, const char * pStr = "") { ErrorCode = errCode, StrError = pStr; return 1; }
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

enum MessageType {
	msgOrder = 1,
	msgOrdRsp,
	msgAperak,
	msgDesadv,
	msgRecadv
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
	{"ORDER",       msgOrder},
	{"ORDRSP",      msgOrdRsp},
	{"APERAK",      msgAperak},
	{"DESADV",      msgDesadv},
	{"RECADV",      msgRecadv}
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
	{IEERR_ONLYIMPMSGTYPES,		"�������� ������� �������� ������ � ������ ������ ORDRSP, DESADV � APERAK"},
	{IEERR_ONLYEXPMSGTYPES,		"�������� �������� �������� ������ � ������ ������ ORDER, RECADV"},
	{IEERR_NOEXPPATH,			"�� ���������� ���������� ��� ����� ��������"},
	{IEERR_TTNNEEDED,			"�������� �� ������. ���������� ������� ���. ��������: "}
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
	if((ErrorCode == IEERR_FTP) || (ErrorCode == IEERR_IMPFILENOTFOUND) || (ErrorCode == IEERR_INVMESSAGEYTYPE) || (ErrorCode  == IEERR_NOCFGFORGLN))
		str.Cat(StrError);
	rMsg.Z().CopyFrom(str);
}

// @vmiller
// ��-��, � � ����� ���������� ����� ��������� ���������

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

// �����, �� �������� ����������� ImportCls � ExportCls. �������� ����� ������, ��� ���� �������
class ImportExportCls {
public:
	ImportExportCls() {};
	~ImportExportCls() {};
	void CleanHeader();

	Sdr_ImpExpHeader Header;
	SString TTN;		// ��� DESADV. �������� �� ��������� ��������� � ������ �� ��� �������� ������
private:
};

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
	int Connect();
	void Disconnect();
	int PutFile(const char * pLocSrc, const char * pFtpDst);
	int GetFile(const char * pFtpSrc, const char * pLocDst);
	int NextFileName(const char * pFtpSrc, SString & rFileName);
	int DeleteFile(const char * pFtpFile);
private:
	int Status;
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
		THROW(HFtpSession = InternetConnect(HInternet, _T("ruftpex.edi.su"), INTERNET_DEFAULT_FTP_PORT, 
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
	CATCHZOK;
	return ok;
}

int FtpClient::GetFile(const char * pFtpSrc, const char * pLocDst)
{
	int    ok = 1;
	if(Status == stDisconnected)
		THROW(Connect());
	THROW(FtpGetFile(HFtpSession, SUcSwitch(pFtpSrc), SUcSwitch(pLocDst), 0, 0, 0, NULL));
	CATCHZOK;
    return ok;
}

// Descr: ���������� ��� ����� � ����� � ���������� ��� ��������.
// ARG(pFtpSrc    IN): ����� �� ftp
// ARG(rFileName OUT): ��� ����� �� ftp
// Retutns:
//   -1 - ��� ������
//    0 - ������
//    1 - ��� ����� ��������
int FtpClient::NextFileName(const char * pFtpSrc, SString & rFileName)
{
	int    ok = 1;
	WIN32_FIND_DATA find_data;
	if(Status == stDisconnected)
		THROW(Connect());
	MEMSZERO(find_data);
	if(!HFtpFind)
		THROW(HFtpFind = FtpFindFirstFile(HFtpSession, SUcSwitch(pFtpSrc), &find_data, 0, NULL))
	else
		InternetFindNextFile(HFtpFind, &find_data);
	if(!isempty(find_data.cFileName))
		rFileName.CopyFrom(SUcSwitch(find_data.cFileName));
	else
		ok = -1;
	CATCHZOK;
    return ok;
}

int FtpClient::DeleteFile(const char * pFtpFile)
{
	int    ok = 1;
	if(Status == stDisconnected)
		THROW(Connect());
	THROW(FtpDeleteFile(HFtpSession, SUcSwitch(pFtpFile)));
	CATCHZOK;
    return ok;
}

EXPORT int FinishImpExp();

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch(dwReason) {
		case DLL_PROCESS_ATTACH:
			{
				SString product_name;
				(product_name = "Papyrus-ImpExpExite");
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
//

class ExportCls : public ImportExportCls {
public:
	ExportCls();
	~ExportCls();
	void CreateFileName(uint num);
	int OrderHeader();
	int RecadvHeader();
	int DocPartiesAndCurrency();
	int GoodsLines(Sdr_BRow * pBRow);
	int EndDoc();
	int SendDoc();
	uint Id;					// ��, ������� Papyrus ����� ������������ ��� �� ������ ��������
	uint ObjId;					// �� ��������������� ������� (��������, ��� ������ ���������� ������� ��������� ������������� ���� ��)
	uint ObjType;				// ��� �������������� ��������
	uint MessageType;			// ��� ���������: ORDER, RECADV
	uint Inited;
	uint SegNum;				// ���������� ��������� (���������) ���������
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

ExportCls::ExportCls()
{
	Id = 0;
	ObjId = 0;
	ObjType = 0;
	MessageType = 0;
	Inited = 0;
	SegNum = 0;
	ReadReceiptNum = 0;
	BillSumWithoutVat = 0.0;
	ExpFileName = 0;
	LogName = 0;
	P_XmlWriter = 0;
	ErrorCode = 0;
	WebServcErrorCode = 0;
	TotalGoodsCount = 0;
	TTN = 0;
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
// �������� ����������� �����
int ExportCls::OrderHeader()
{
	int    ok = 1;
	SString str, fmt;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	BillSumWithoutVat = 0.0;
	SegNum = 0;
	xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_ORDERS);
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_UNH);
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0062); // �� ���������
				xmlTextWriterWriteString(P_XmlWriter, str.Z().Cat(ObjId).ucptr());
			xmlTextWriterEndElement(P_XmlWriter); //E0062
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_S009);
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0065); // ��� ���������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E0065_ORDERS);
				xmlTextWriterEndElement(P_XmlWriter); //E0065
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0052); // ������ ���������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E0052_D);
				xmlTextWriterEndElement(P_XmlWriter); //E0052
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0054); // ������ �������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E0054_01B);
				xmlTextWriterEndElement(P_XmlWriter); //E0054
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0051); // ��� ������� �����������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E0051_UN);
				xmlTextWriterEndElement(P_XmlWriter); //E0051
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0057); // ���, ����������� ������� ������������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E0057_EAN010);
				xmlTextWriterEndElement(P_XmlWriter); //E0057
			xmlTextWriterEndElement(P_XmlWriter); //S009
		xmlTextWriterEndElement(P_XmlWriter); //UNH
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_BGM);
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C002); // ��� ���������/���������
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1001); // ��� ���������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E1001_220); // ����� (���� ������������� ������)
				xmlTextWriterEndElement(P_XmlWriter); //E1001
			xmlTextWriterEndElement(P_XmlWriter); //�002
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C106); // ������������� ���������/���������
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1004); // ����� ������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.Code); // ������ ���� �������� 17 ��������
				xmlTextWriterEndElement(P_XmlWriter); //E1004
			xmlTextWriterEndElement(P_XmlWriter); //�106
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1225); // ��� ������� ���������
				xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E1225_9); // �������� (���� ��� �����, ��������������� �����, ������ � �.�.)
			xmlTextWriterEndElement(P_XmlWriter); //E1225
		xmlTextWriterEndElement(P_XmlWriter); //BGM
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_DTM); // ���� ���������
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C507);
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E2005); // ������������ ������� ����-�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E2005_137); // ����/����� ���������/���������
				xmlTextWriterEndElement(P_XmlWriter); //E2005
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2380"); // ���� ��� �����, ��� ������
					fmt.Z().Cat(Bill.Date.month());
					if(fmt.Len() == 1)
						fmt.PadLeft(1, '0');
					str.Z().Cat(Bill.Date.year()).Cat(fmt);
					fmt.Z().Cat(Bill.Date.day());
					if(fmt.Len() == 1)
						fmt.PadLeft(1, '0');
                    str.Cat(fmt);
					xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E2380
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2379"); // ������ ����/�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E2379_102); // CCYYMMDD
				xmlTextWriterEndElement(P_XmlWriter); //E2379
			xmlTextWriterEndElement(P_XmlWriter); //C507
		xmlTextWriterEndElement(P_XmlWriter); //DTM
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_DTM); // ���� ��������
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C507);
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E2005); // ������������ ������� ����-�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E2005_2); // ����/����� ��������
				xmlTextWriterEndElement(P_XmlWriter); //E2005
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2380"); // ���� ��� �����, ��� ������
					LDATE date;
					if(Bill.DueDate != ZERODATE)
						date = Bill.DueDate;
					else
						date = Bill.Date;
					fmt.Z().Cat(date.month());
					if(fmt.Len() == 1)
						fmt.PadLeft(1, '0');
					str.Z().Cat(date.year()).Cat(fmt);
					fmt.Z().Cat(date.day());
					if(fmt.Len() == 1)
						fmt.PadLeft(1, '0');
					str.Cat(fmt);
					xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E2380
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2379"); // ������ ����/�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E2379_102); // CCYYMMDD
				xmlTextWriterEndElement(P_XmlWriter); //E2379
			xmlTextWriterEndElement(P_XmlWriter); //C507
		xmlTextWriterEndElement(P_XmlWriter); //DTM

	CATCH
		SysLogMessage(SYSLOG_ORDERHEADER);
		ok = 0;
	ENDCATCH;
	return ok;
}

// ������ � ��������� ����������� �� ��������
int ExportCls::RecadvHeader()
{
	int    ok = 1;
	SString str, fmt;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	BillSumWithoutVat = 0.0;
	SegNum = 0;

	xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_RECADV);
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_UNH);
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0062); // �� ���������
				str.Z().Cat(ObjId);
				xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
			xmlTextWriterEndElement(P_XmlWriter); //E0062
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_S009);
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0065); // ��� ���������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E0065_RECADV);
				xmlTextWriterEndElement(P_XmlWriter); //E0065
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0052); // ������ ���������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E0052_D);
				xmlTextWriterEndElement(P_XmlWriter); //E0052
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0054); // ������ �������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E0054_01B);
				xmlTextWriterEndElement(P_XmlWriter); //E0054
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0051); // ��� ������� �����������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E0051_UN);
				xmlTextWriterEndElement(P_XmlWriter); //E0051
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0057); // ���, ����������� ������� ������������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E0057_EAN006);
				xmlTextWriterEndElement(P_XmlWriter); //E0057
			xmlTextWriterEndElement(P_XmlWriter); //S009
		xmlTextWriterEndElement(P_XmlWriter); //UNH
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_BGM);
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C002); // ��� ���������/���������
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1001); // ��� ���������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E1001_632); // ����������� � �������
				xmlTextWriterEndElement(P_XmlWriter); //E1001
			xmlTextWriterEndElement(P_XmlWriter); //�002
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C106); // ������������� ���������/���������
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1004); // ����� ��������� � �������
					str = Bill.Code;
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)(const char *)str.ToUtf8()); // ������ ���� �������� 17 ��������
				xmlTextWriterEndElement(P_XmlWriter); //E1004
			xmlTextWriterEndElement(P_XmlWriter); //�106
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1225); // ��� ������� ���������
				xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E1225_9); // �������� (���� ��� �����, ��������������� �����, ������ � �.�.)
			xmlTextWriterEndElement(P_XmlWriter); //E1225
		xmlTextWriterEndElement(P_XmlWriter); //BGM
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_DTM); // ���� ���������
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C507);
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E2005); // ������������ ������� ����-�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E2005_137); // ����/����� ���������/���������
				xmlTextWriterEndElement(P_XmlWriter); //E2005
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2380"); // ���� ��� �����, ��� ������
					fmt.Z().Cat(Bill.Date.month());
					if(fmt.Len() == 1)
						fmt.PadLeft(1, '0');
					str.Z().Cat(Bill.Date.year()).Cat(fmt);
					fmt.Z().Cat(Bill.Date.day());
					if(fmt.Len() == 1)
						fmt.PadLeft(1, '0');
                    str.Cat(fmt);
					xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E2380
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2379"); // ������ ����/�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E2379_102); // CCYYMMDD
				xmlTextWriterEndElement(P_XmlWriter); //E2379
			xmlTextWriterEndElement(P_XmlWriter); //C507
		xmlTextWriterEndElement(P_XmlWriter); //DTM
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_DTM); // ���� �������
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C507);
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E2005); // ������������ ������� ����-�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E2005_50); // ����/����� �������
				xmlTextWriterEndElement(P_XmlWriter); //E2005
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2380"); // ���� ��� �����, ��� ������
					fmt.Z().Cat(Bill.Date.month());
					if(fmt.Len() == 1)
						fmt.PadLeft(1, '0');
					str.Z().Cat(Bill.Date.year()).Cat(fmt);
					fmt.Z().Cat(Bill.Date.day());
					if(fmt.Len() == 1)
						fmt.PadLeft(1, '0');
                    str.Cat(fmt);
					xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E2380
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"E2379"); // ������ ����/�������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E2379_102); // CCYYMMDD
				xmlTextWriterEndElement(P_XmlWriter); //E2379
			xmlTextWriterEndElement(P_XmlWriter); //C507
		xmlTextWriterEndElement(P_XmlWriter); //DTM

		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG1); // ����� ������ (����� ���������� ������ ��������)
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_RFF);
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C506);
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1153);
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E1153_ON); // ������������ - �����
					xmlTextWriterEndElement(P_XmlWriter); //E1153
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1154); // ����� ��������� ������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.OrderBillNo); // ��� ����� ������ ��������
					xmlTextWriterEndElement(P_XmlWriter); //E1154
				xmlTextWriterEndElement(P_XmlWriter); //C506
            xmlTextWriterEndElement(P_XmlWriter); //RFF
		xmlTextWriterEndElement(P_XmlWriter); //SG1
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG1); // ����� ��������� ���������
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_RFF);
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C506);
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1153);
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E1153_AAK); // ������������ - ��������� ����������� �� ��������
					xmlTextWriterEndElement(P_XmlWriter); //E1153
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1154); // ����� ��������� ������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)(const char *)TTN.ToChar().ToUtf8());
					xmlTextWriterEndElement(P_XmlWriter); //E1154
				xmlTextWriterEndElement(P_XmlWriter); //C506
            xmlTextWriterEndElement(P_XmlWriter); //RFF
		xmlTextWriterEndElement(P_XmlWriter); //SG1

	CATCH
		SysLogMessage(SYSLOG_RECADVHEADER);
		ok = 0;
	ENDCATCH;
	return ok;
}

int ExportCls::DocPartiesAndCurrency()
{
	int    ok = 1;
	SString str, sender_gln, login;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);

		if(MessageType == msgOrder)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG2); // GLN ����������
		else if(MessageType == msgRecadv)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG4); // GLN ����������

			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_NAD); // ������������ � �����
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3035); // ������������ �������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E3035_SU); // ���������
				xmlTextWriterEndElement(P_XmlWriter); //E3035
                xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C082); // ������ �������
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3039); // GLN �������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.GLN);
					xmlTextWriterEndElement(P_XmlWriter); //E3039
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3055); // ��� ������� �����������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E3055_9); // EAN (������������� ���������� �������� ���������)
					xmlTextWriterEndElement(P_XmlWriter); //E3055
				xmlTextWriterEndElement(P_XmlWriter); //C082
			xmlTextWriterEndElement(P_XmlWriter); //NAD
		xmlTextWriterEndElement(P_XmlWriter); //SG2/SG4
		if(MessageType == msgOrder)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG2); // GLN ����������
		else if(MessageType == msgRecadv)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG4); // GLN ����������

			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_NAD); // ������������ � �����
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3035); // ������������ �������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E3035_BY); // ����������
				xmlTextWriterEndElement(P_XmlWriter); //E3035
                xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C082); // ������ �������
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3039); // GLN �������
						// ���� GLN ������ �����, �� ����� GLN ������� �����������
						if(!isempty(Bill.AgentGLN))
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.AgentGLN);
						else
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)Bill.MainGLN);
					xmlTextWriterEndElement(P_XmlWriter); //E3039
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3055); // ��� ������� �����������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E3055_9); // EAN (������������� ���������� �������� ���������)
					xmlTextWriterEndElement(P_XmlWriter); //E3055
				xmlTextWriterEndElement(P_XmlWriter); //C082
			xmlTextWriterEndElement(P_XmlWriter); //NAD
		xmlTextWriterEndElement(P_XmlWriter); //SG2/SG4
		if(MessageType == msgOrder)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG2); // GLN ����� ��������
		else if(MessageType == msgRecadv)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG4); // GLN ����� ��������

			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_NAD); // ������������ � �����
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3035); // ������������ �������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E3035_DP); // �������� ����� ��������
				xmlTextWriterEndElement(P_XmlWriter); //E3035
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C082); // ������ �������
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3039); // GLN �������
						if(!isempty(Bill.DlvrAddrCode))
							str.Z().Cat(Bill.DlvrAddrCode);
						else
							str.Z().Cat(Bill.LocCode);
						xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E3039
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E3055); // ��� ������� �����������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E3055_9); // EAN (������������� ���������� �������� ���������)
					xmlTextWriterEndElement(P_XmlWriter); //E3055
				xmlTextWriterEndElement(P_XmlWriter); //C082
			xmlTextWriterEndElement(P_XmlWriter); //NAD
		xmlTextWriterEndElement(P_XmlWriter); //SG2/SG4

		// ������ � ����� ��������� ��� RECADV ���� ���� �� ��������
		if(MessageType == msgOrder)
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG7); // ��� RECADV ����� ���� ����� ���

			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_CUX); // ������
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C504); // ������
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6347); // ������������ ������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6347_2); // ��������� ������
					xmlTextWriterEndElement(P_XmlWriter); //E6347
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6345); // ������������� ������ �� ISO 4217
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6345_RUB); // �����
					xmlTextWriterEndElement(P_XmlWriter); //E6345
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6343); // ������������ ���� ������
						if(MessageType == msgOrder)
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6343_9); // ������ ������
						else if(MessageType == msgRecadv)
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6343_4); // ������ ���������
					xmlTextWriterEndElement(P_XmlWriter); //E6343
				xmlTextWriterEndElement(P_XmlWriter); //C504
			xmlTextWriterEndElement(P_XmlWriter); //CUX

		if(MessageType == msgOrder)
			xmlTextWriterEndElement(P_XmlWriter); //SG7

	CATCH
		SysLogMessage(SYSLOG_DOCPARTIES);
		ok = 0;
	ENDCATCH;
	return ok;
}

// ��������� ���� � �������
int ExportCls::GoodsLines(Sdr_BRow * pBRow)
{
	int    ok = 1;
	SString str;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);
	THROWERR(pBRow, IEERR_NODATA);

	// ��� RECADV
	// ���� ��� �� ������ ��� � ������ ������, �� �������� �� �������. ��� ������ ��������� ������ ����� ������ ����� ���������,
	// ������� � ���� ������� ������������ ������ ��� ������� ���
	if(MessageType == msgRecadv)
		THROWERR_STR(!isempty(pBRow->TTN), IEERR_TTNNEEDED, Bill.Code);
	// ����� � ���������� ������ ����� ���� ���.
	// �� ������� pBRow->TTN ����� ����������, ��� ��� ���� ����������� �������, ��� ��� � ������� ����������
	// ���� �������� RECADV � DESADV ������ �� ����� ���������.
	// ��� � ���� ����� ����������, ��� ������ �� ������ DESADV
	if((MessageType == msgRecadv) && TTN.Empty()) {
		TTN = pBRow->TTN;
		THROW(P_ExportCls->RecadvHeader());
		THROW(P_ExportCls->DocPartiesAndCurrency());
	}

	if(MessageType == msgOrder)
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG28); // ���� � �������
	else if(MessageType == msgRecadv)
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG16); // ���� � �������

		// ������ ��� RECADV
		if(MessageType == msgRecadv) {
			// ���-�� ��� �������� ������� � ����� ��������
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_CPS);
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E7164);
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E7164_1); // ����� �������� �� ��������� - 1
				xmlTextWriterEndElement(P_XmlWriter); //E7164
			xmlTextWriterEndElement(P_XmlWriter); //CPS
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)"SG22");
		}

			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_LIN);
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1082); // ����� �������� ������
					str.Z().Cat(Itr.GetCount() + 1);
					xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E1082
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C212); // �������� ������������� ������
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E7140); // �����-��� ������
						str.Z().Cat(pBRow->Barcode);
						xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E7140
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E7143); // ��� ���������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E7143_SRV); // EAN.UCC
					xmlTextWriterEndElement(P_XmlWriter); //E7143
				xmlTextWriterEndElement(P_XmlWriter); //C212
			xmlTextWriterEndElement(P_XmlWriter); //LIN
			// �������������� ��������
			if(!isempty(pBRow->ArCode)) {
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_PIA); // ������������� ������������� ������
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E4347); // ��� ���� �������������� ������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E4347_1); // �������������� �������������
					xmlTextWriterEndElement(P_XmlWriter); //E4347
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C212);
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E7140); // �������
							(str = pBRow->ArCode).ToChar().ToUtf8();
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); //E7140
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E7143); // ������������� ���� ��������
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E7143_SA); // ������������� �������� ����������
						xmlTextWriterEndElement(P_XmlWriter); //E7143
					xmlTextWriterEndElement(P_XmlWriter); //C212
				xmlTextWriterEndElement(P_XmlWriter); //PIA
			}
			// ������ ��� ORDERS
			if(MessageType == msgOrder) {
				// �������������� ��������
				if(!isempty(pBRow->GoodsName)) {
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_IMD); // �������� ������
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
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_QTY); // ����������
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C186); // �����������
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6063); // ������������ ���� ����������
						if(MessageType == msgOrder)
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)"21"); // ���������� ���������� ������
						else if(MessageType == msgRecadv)
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6063_194); // �������� ���������� ������
					xmlTextWriterEndElement(P_XmlWriter); //E6063
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6060); // ����������
						str.Z().Cat(pBRow->Quantity);
						xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E6060
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6411); // ������� ���������
						(str = pBRow->UnitName).ToOem().ToUpper().ToChar();
						if(str.CmpNC(UNIT_NAME_KG) == 0)
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6411_KGM); // ����������
						else
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6411_PCE); // ��������� ��������
					xmlTextWriterEndElement(P_XmlWriter); //E6411
				xmlTextWriterEndElement(P_XmlWriter); //C186
			xmlTextWriterEndElement(P_XmlWriter); //QTY
            // ������ ����� �� �������� ������� ��� RECADV � ��������� �� ���������, �� ���� ������� �� � ��������
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_MOA); // ����� �������� ������� � ���
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C516);
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5025); // ������������ ����� �������� �������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E5025_128); //	������������� ����� �������� ������� � ���
					xmlTextWriterEndElement(P_XmlWriter); //E5025
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5004); // �����
						str.Z().Cat(pBRow->Cost * pBRow->Quantity);
						xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E5004
				xmlTextWriterEndElement(P_XmlWriter); //C516
			xmlTextWriterEndElement(P_XmlWriter); //MOA
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_MOA); // ����� �������� ������� ��� ���
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C516);
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5025); // ������������ ����� �������� �������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E5025_203); //	������������� ����� �������� ������� ��� ���
					xmlTextWriterEndElement(P_XmlWriter); //E5025
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5004); // �����
						//str.Z().Cat((pBRow->Cost - (pBRow->Cost / (pBRow->VatRate + 100) * 100)) * pBRow->Quantity);
						str.Z().Cat((pBRow->Cost / (pBRow->VatRate + 100) * 100) * pBRow->Quantity);
						BillSumWithoutVat += str.ToReal();
						xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E5004
				xmlTextWriterEndElement(P_XmlWriter); //C516
			xmlTextWriterEndElement(P_XmlWriter); //MOA

			if(MessageType == msgOrder)
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG32); // ���� ������ � ���

				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_PRI); // ������� ����������
					SegNum++;
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C509);
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5125); // ������������ ����
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E5125_AAE); // ���� ��� ������ � ��������, �� � �������
						xmlTextWriterEndElement(P_XmlWriter); //E5125
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5118); // ����
							str.Z().Cat(pBRow->Cost);
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); //E5118
					xmlTextWriterEndElement(P_XmlWriter); //C509
				xmlTextWriterEndElement(P_XmlWriter); //PRI
			if(MessageType == msgOrder)
				xmlTextWriterEndElement(P_XmlWriter); //SG32

			if(MessageType == msgOrder)
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG32); // ���� ������ ��� ���

				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_PRI); // ������� ����������
					SegNum++;
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C509);
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5125); // ������������ ����
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E5125_AAA); // ������ ���� ��� �������
						xmlTextWriterEndElement(P_XmlWriter); //E5125
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5118); // ����
							//str.Z().Cat(pBRow->Cost - (pBRow->Cost / (pBRow->VatRate + 100) * 100));
							str.Z().Cat(pBRow->Cost / (pBRow->VatRate + 100) * 100);
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
						xmlTextWriterEndElement(P_XmlWriter); //E5118
					xmlTextWriterEndElement(P_XmlWriter); //C509
				xmlTextWriterEndElement(P_XmlWriter); //PRI
			if(MessageType == msgOrder)
				xmlTextWriterEndElement(P_XmlWriter); //SG32

			if(MessageType == msgRecadv)
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG28);
			// ������ ��� RECADV
			if(MessageType == msgRecadv) {
				// ����� ���
				if(!isempty(pBRow->CLB)) {
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_RFF);
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C506);
							xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1153);
								xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E1153_ABT); // ��� ���� - ���
							xmlTextWriterEndElement(P_XmlWriter); //E1153
							xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E1154);
								str.Z().Cat(pBRow->CLB);
								xmlTextWriterWriteString(P_XmlWriter, str.ucptr()); // ���
							xmlTextWriterEndElement(P_XmlWriter); //E1154
						xmlTextWriterEndElement(P_XmlWriter); //C506
					xmlTextWriterEndElement(P_XmlWriter); //RFF
				}
			}

			if(MessageType == msgOrder)
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_SG38); // ������ ���

				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_TAX);
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5283); // ������������
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E5283_7); // ������������� ���������� ����������
					xmlTextWriterEndElement(P_XmlWriter); //E5283
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C241);
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5153); // ������������
							xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E5153_VAT); // ������������� ������ ���
						xmlTextWriterEndElement(P_XmlWriter); //E5153
					xmlTextWriterEndElement(P_XmlWriter); //C241
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C243);
						xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5278);
							str.Z().Cat(pBRow->VatRate);
							xmlTextWriterWriteString(P_XmlWriter, str.ucptr()); // ������ ���
						xmlTextWriterEndElement(P_XmlWriter); //E5278
					xmlTextWriterEndElement(P_XmlWriter); //C243
				xmlTextWriterEndElement(P_XmlWriter); //TAX
			if(MessageType == msgOrder)
				xmlTextWriterEndElement(P_XmlWriter); //SG38

		// ������ ��� RECADV
		if(MessageType == msgRecadv) {
			xmlTextWriterEndElement(P_XmlWriter); //SG28
			xmlTextWriterEndElement(P_XmlWriter); //SG22
		}
	xmlTextWriterEndElement(P_XmlWriter); //SG28/SG16
	CATCH
		SysLogMessage(SYSLOG_GOODSLINES);
		ok = 0;
	ENDCATCH;
	return ok;
}

// ��������� ������������ ���������
int ExportCls::EndDoc()
{
	int    ok = 1;
	SString str;
	THROWERR(P_XmlWriter, IEERR_NULLWRIEXMLPTR);

		// ������ ��� ORDERS
		if(MessageType == msgOrder) {
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_UNS); // ����������� ���
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0081); // ������������� ������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E0081_S); // ���� �������� ����������
				xmlTextWriterEndElement(P_XmlWriter); //E0081
			xmlTextWriterEndElement(P_XmlWriter); //UNS
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_MOA); // ����� ������ � ���
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_�516);
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5025); // ������������ �����
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E5025_9); // ����� ��������� � ���
					xmlTextWriterEndElement(P_XmlWriter); //E5025
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5004); // �����
						str.Z().Cat(Bill.Amount);
						xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E5004
				xmlTextWriterEndElement(P_XmlWriter); //�516
			xmlTextWriterEndElement(P_XmlWriter); //MOA
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_MOA); // ����� ������ ��� ���
				SegNum++;
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_�516);
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5025); // ������������ �����
						xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E5025_98); // ����� ��������� ��� ���
					xmlTextWriterEndElement(P_XmlWriter); //E5025
					xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E5004); // �����
						str.Z().Cat(BillSumWithoutVat);
						xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
					xmlTextWriterEndElement(P_XmlWriter); //E5004
				xmlTextWriterEndElement(P_XmlWriter); //�516
			xmlTextWriterEndElement(P_XmlWriter); //MOA
		}
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_CNT); // �������� ����������
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_C270);
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6069); // ������������ ���� �������� ����������
					xmlTextWriterWriteString(P_XmlWriter, (const xmlChar *)ELEMENT_CODE_E6069_2); // ���������� �������� ������� � ���������
				xmlTextWriterEndElement(P_XmlWriter); //E6069
				xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E6066); // ��������
					str.Z().Cat(Itr.GetCount());
					xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
				xmlTextWriterEndElement(P_XmlWriter); //E6066
			xmlTextWriterEndElement(P_XmlWriter); //C270
		xmlTextWriterEndElement(P_XmlWriter); //CNT
		xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_UNT); // ��������� ���������
			SegNum++;
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0074); // ����� ����� ��������� � ���������
				str.Z().Cat(SegNum);
				xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
			xmlTextWriterEndElement(P_XmlWriter); //E0074
			xmlTextWriterStartElement(P_XmlWriter, (const xmlChar *)ELEMENT_NAME_E0062); // ����� ������������ ��������� (��������� � ��������� � ���������)
				str.Z().Cat(ObjId);
				xmlTextWriterWriteString(P_XmlWriter, str.ucptr());
			xmlTextWriterEndElement(P_XmlWriter); //E0062
		xmlTextWriterEndElement(P_XmlWriter); //UNT
	xmlTextWriterEndElement(P_XmlWriter); //ORDERS/RECADV

	CATCH
		SysLogMessage(SYSLOG_ENDDOC);
		ok = 0;
	ENDCATCH;
	TTN = 0;
	return ok;
}
//
// ������ � FTP
//
// ���������� ��������
int ExportCls::SendDoc()
{
	int    ok = 1;
	SString	inbox_filename;
	SPathStruc path_struct(ExpFileName);
	(inbox_filename = OUTBOX).CatChar('/').Cat(path_struct.Nam).Dot().Cat(path_struct.Ext);
	Sdr_DllImpExpReceipt * p_exp_rcpt = 0;
	FtpClient ftp_client(Header.EdiLogin, Header.EdiPassword);
	// ������������ � ftp
	if(ftp_client.Connect()) {
		// ����������� ����
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
	SFile log_file;
	SPathStruc log_path;
	SString str;

	if(!P_ExportCls) {
		P_ExportCls = new ExportCls;
	}
	if(P_ExportCls && !P_ExportCls->Inited) {
		if(pExpHeader) {
			P_ExportCls->Header = *static_cast<const Sdr_ImpExpHeader *>(pExpHeader);
			SString str;
			//FormatLoginToLogin(P_ExportCls->Header.EdiLogin, str);
			//str.CopyTo(P_ExportCls->Header.EdiLogin, sizeof(P_ExportCls->Header.EdiLogin));
		}
		if(!isempty(pOutFileName)) {
			P_ExportCls->PathStruct.Split(pOutFileName);
			P_ExportCls->PathStruct.Ext = "xml";
			P_ExportCls->PathStruct.Nam.SetIfEmpty("export_");
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
	//THROWERR(GetMsgTypeBySymb(pMsgType, P_ExportCls->MessageType), IEERR_MSGSYMBNOTFOUND);
	THROWERR_STR(GetMsgTypeBySymb(pMsgType, P_ExportCls->MessageType), IEERR_MSGSYMBNOTFOUND, pMsgType);
	THROWERR((P_ExportCls->MessageType == msgOrder) || (P_ExportCls->MessageType == msgRecadv), IEERR_ONLYEXPMSGTYPES);
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
	if(P_ExportCls->MessageType == msgOrder) {
		THROW(P_ExportCls->OrderHeader())
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
struct AperakInfoSt {
	AperakInfoSt() { Clear(); }
	void Clear() {
		OrderNum = 0;
		Code = 0;
		Msg = 0;
		AddedMsg = 0;
		SupplGLN = 0;
		BuyerGLN = 0;
		AddrGLN = 0;
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
	ImportCls();
	~ImportCls();
	void CreateFileName(uint num);
	int ReceiveDoc();
	int ParseForDocData(Sdr_Bill * pBill);
	int ParseForGoodsData(Sdr_BRow * pBRow);
	int ParseAperakResp();
	int GoodsCount;				// ����� ������� � ���������
	uint Id;					// ��, ������� Papyrus ����� ������������ ��� �� ������ ��������
	uint ObjId;					// �� �������������� ������� (��������, ���� ��� ����� ����������, �� � ������� ��������� ���� ��)
	uint ObjType;				// ��� ������������� ��������
	uint MessageType;			// ��� ��������: ORDRESP, APERAK
	uint Inited;
	uint InboxReadIndex;        // ������ ������ �� InboxFiles
	double BillSumWithoutVat;	// � Sdr_Bill ���� ������� �� ����������, ������� �������� ����� ��������� Papyrus'� ��� ��������
	SString ImpFileName;
	SString LogFileName;		// ��� ��, ��� � ImpFileName
	Iterator Itr;
	SPathStruc PathStruct;
	AperakInfoSt AperakInfo;
	StrAssocArray InboxFiles;   // ������ � ������� �������� ������ ������� ����
	StrAssocArray FilesForDel;  // ������ � ������� ������, ������� ��������� ������� �� ����� �� ftp
private:
	int ParseListMBResp(const char * pResp, SString & rPartnerIln, SString & rDocId);
};

ImportCls::ImportCls() : GoodsCount(0), Id(0), ObjId(0), ObjType(0), MessageType(0), Inited(0), InboxReadIndex(0), BillSumWithoutVat(0.0)
{
	ErrorCode = 0;
	WebServcErrorCode = 0;
	AperakInfo.Clear();
	InboxFiles.Z();
	FilesForDel.Z();
}

ImportCls::~ImportCls()
{
}

void ImportCls::CreateFileName(uint num)
{
	ImpFileName.Z().Cat(PathStruct.Drv).CatChar(':').Cat(PathStruct.Dir).Cat(PathStruct.Nam).Cat(num).Dot().Cat(PathStruct.Ext);
}

//
// Descr: �������� ��������. � ������ ������, ���������� ��� � ����.
//
// Returns:
//		-1 - ��� �������� ���������
//		 0 - ������
//		 1 - ��������� ��������
int ImportCls::ReceiveDoc()
{
	int    ok = 1;
	size_t pos = 0;
	SString read_filename, file_type, box_name, str;
	SSrchParam srch_param(0, 0, SSPF_WORDS);
	FtpClient ftp_client(Header.EdiLogin, Header.EdiPassword);
	if(MessageType == msgOrdRsp) {
		file_type = "ordRsp";
		box_name = INBOX;
	}
	else if(MessageType == msgDesadv) {
		file_type = "desadv";
		box_name = INBOX;
	}
	else if(MessageType == msgAperak) {
		file_type = "status";
		box_name = /*REPORTSBOX*/INBOX; // @vmiller �����, � ��� ���� ������ ��������� APERAK
	}
	// ������������� ����������
	if(!ftp_client.Connect()) {
		ProcessError("FtpConnect");
		ok = 0;
	}

	// ��������� ������� ������
	if(ok && !InboxFiles.getCount()) {
		// ������������� ����� ������
		uint i = InboxFiles.getCount();
		while(ftp_client.NextFileName(box_name, read_filename) > 0) {
			// @vmiller !!!!!!!!!!!!!!!!!!!!!!!!!!!
			// ��� �� ��������, ��� � ��� ������������ ���������
			// �������, ��� ��� ��� ������
			srch_param.P_Pattern = file_type;
			if(read_filename.Search(file_type, 0, 1, &pos)) {
				(str = box_name).CatChar('/').Cat(read_filename);
				InboxFiles.Add(i++, str, 1);
			}
		}
	}

	// ������ ������ ���������� ����
	if(InboxReadIndex < InboxFiles.getCount()) {
		SString name = InboxFiles.Get(InboxReadIndex).Txt;
		if(ftp_client.GetFile(InboxFiles.Get(InboxReadIndex).Txt, ImpFileName)) {
			SFile file(ImpFileName, SFile::mRead);
			SString file_buf;
			int64 file_size = 0;
			file.CalcSize(&file_size);
			SBuffer buf((size_t)file_size + 1);
			if(file.IsValid()) {
				//SStrScan Scan;
				//long ReEmmId = 0;
				//Scan.RegisterRe("[ ]+<", &ReEmmId);
				while(file.ReadLine(file_buf)) {
				//	Scan.Set(file_buf, 0);
				//	if(Scan.GetRe(ReEmmId, str)) {
				//		file_buf.ReplaceStr(str, "<", 0);
				//	}
				//	else
				//		Scan.Incr();
					file_buf.ReplaceCR().ReplaceStr("\n", "", 0);
					buf.Write(file_buf.cptr(), file_buf.Len());
				}
			}
			file.Close();
			if(file.Open(ImpFileName, SFile::mWrite)) {
				file.Write(buf.GetBuf(), buf.GetAvailableSize());
			}
			file.Close();
			if(MessageType == msgAperak)
				// ����� �������� �����
				THROW(ParseAperakResp());
		}
		else {
			ProcessError("FtpGetFile");
			ok = 0;
		}
	}

	// ��� �������� ���������
	if(ok && (!InboxFiles.getCount() || (InboxReadIndex >= InboxFiles.getCount())))
		ok = -1;

	CATCHZOK;
	if(!ok)
		SysLogMessage(SYSLOG_RECEIVEDOC);
	return ok;
}

// Descr: ������ ����������, ����������� � ������� Receive() ��� APERAK
// Retruns:
//		 0 - ������
//		 1 - ��� ��������� �������
int ImportCls::ParseAperakResp()
{
	int ok = 1, is_correct = 0, exit_while = 0;
	SString str;
	xmlDoc * p_doc = 0;

	AperakInfo.Clear();
	THROWERR_STR(fileExists(ImpFileName), IEERR_IMPFILENOTFOUND, ImpFileName);
	THROWERR((p_doc = xmlReadFile(ImpFileName, NULL, XML_PARSE_NOENT)), IEERR_NULLREADXMLPTR);
	xmlNode * p_node = xmlDocGetRootElement(p_doc);
	THROWERR(p_node, IEERR_XMLREAD);
	if(sstreq(p_node->name, ELEMENT_NAME_DOCAPERAK) && p_node->children) // �� ������� ���� ����� ������, ��� ��� Aperak
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
				if(sstreq(p_node->name, ELEMENT_NAME_E0065) && p_node->children) {
					THROWERR_STR(sstreq(p_node->children->content, ELEMENT_CODE_E0065_APERAK), IEERR_INVMESSAGEYTYPE, ELEMENT_CODE_E0065_APERAK);
				}
				else if(p_node && sstreq(p_node->name, ELEMENT_NAME_E1153) && p_node->children) {
					if(sstreq(p_node->children->content, ELEMENT_CODE_E1153_ON)) {
						// �������� ����� ��������� ������
						if(p_node->next)
							if(sstreq(p_node->next->name, ELEMENT_NAME_E1154) && p_node->next->children)
								AperakInfo.OrderNum = (const char *)p_node->next->children->content;
					}
				}
				else if(p_node && (sstreq(p_node->name, ELEMENT_NAME_E2005)) && p_node->children) {
					if(sstreq(p_node->children->content, ELEMENT_CODE_E2005_171)) {
						// ������� ���� ��������� ������
						if(p_node->next) {
							if(sstreq(p_node->next->name, "E2380") && p_node->next->children) {
								SString sub;
								str = (const char *)p_node->next->children->content;
								str.Sub(0, 4, sub);
								AperakInfo.OrderDate.setyear((uint)sub.ToLong());
								str.Sub(4, 2, sub);
								AperakInfo.OrderDate.setmonth((uint)sub.ToLong());
								str.Sub(6, 2, sub);
								AperakInfo.OrderDate.setday((uint)sub.ToLong());
							}
						}
					}
				}
				else if(p_node && sstreq(p_node->name, ELEMENT_NAME_E3035) && p_node->children) {
					// �������� �������� ��������
					str = (const char *)p_node->children->content;
					if(p_node->next) {
						p_node = p_node->next;
						if(sstreq(p_node->name, ELEMENT_NAME_C082) && p_node->children) {
							if(sstreq(p_node->children->name, ELEMENT_NAME_E3039)) {
								if(str.CmpNC(ELEMENT_CODE_E3035_BY) == 0)
									// �������� GLN ����������
									AperakInfo.BuyerGLN = (const char *)p_node->children->children->content;
								else if(str.CmpNC(ELEMENT_CODE_E3035_SU) == 0)
									// �������� GLN ����������
									AperakInfo.SupplGLN = (const char *)p_node->children->children->content;
								else if(str.CmpNC(ELEMENT_CODE_E3035_DP) == 0)
									// �������� GLN ������ ��������
									AperakInfo.AddrGLN = (const char *)p_node->children->children->content;
							}
						}
					}
				}
				else if(p_node && sstreq(p_node->name, ELEMENT_NAME_E9321) && p_node->children) {
					// �������� ��� �������
					AperakInfo.Code = (const char *)p_node->children->content;
				}
				else if(p_node && sstreq(p_node->name, ELEMENT_NAME_E4451) && p_node->children) {
					if(sstreq(p_node->children->content, ELEMENT_CODE_E4451_AAO)) {
						if(p_node->next) {
							if(p_node->next->children) {
								// ���� ������� ����� ����������� ��� ����. � ������ ����� ����� ���������, �� ������ - ��������� ��������
								if(sstreq(p_node->next->children->name, ELEMENT_NAME_E4440) && p_node->next->children->children) {
									// �������� �������� �������
									AperakInfo.Msg = (const char *)p_node->next->children->children->content;
									if(p_node->next->children->next) {
										if(sstreq(p_node->next->children->next->name, ELEMENT_NAME_E4440) && p_node->next->children->next->children)
											AperakInfo.AddedMsg = (const char *)p_node->next->children->next->children->content;
									}
								}
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

// ��������� ���������� �������� � ��������� Sdr_Bill
int ImportCls::ParseForDocData(Sdr_Bill * pBill)
{
	int ok = 0, exit_while = 0;
	SString str;
	xmlDoc * p_doc = 0;

	THROWERR_STR(fileExists(ImpFileName), IEERR_IMPFILENOTFOUND, ImpFileName);
	THROWERR(pBill, IEERR_NODATA);
	memzero(pBill, sizeof(Sdr_Bill));
	THROWERR((p_doc = xmlReadFile(ImpFileName, NULL, XML_PARSE_NOENT)), IEERR_NULLREADXMLPTR);
	xmlNode * p_node = 0;
	xmlNode * p_root = xmlDocGetRootElement(p_doc);
	THROWERR(p_root, IEERR_XMLREAD);
	p_node = p_root->children;
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

		if(p_node && (p_node->type == XML_ELEMENT_NODE)) {
			if(sstreq(p_node->name, ELEMENT_NAME_E0065) && p_node->children) {
				if(MessageType == msgOrdRsp)
					THROWERR_STR(sstreq(p_node->children->content, ELEMENT_CODE_E0065_ORDRSP), IEERR_INVMESSAGEYTYPE, ELEMENT_CODE_E0065_ORDRSP)
				else if(MessageType == msgDesadv)
					THROWERR_STR(sstreq(p_node->children->content, ELEMENT_CODE_E0065_DESADV), IEERR_INVMESSAGEYTYPE, ELEMENT_CODE_E0065_DESADV);
			}
			else if(sstreq(p_node->name, ELEMENT_NAME_E1004) && p_node->children) {
				(str = (const char *)p_node->children->content).Utf8ToOem(); // ����� ����� ��������� � Papyrus
				str.CopyTo(pBill->Code, str.Len() + 1);
				ok = 1;
			}
			else if(sstreq(p_node->name, ELEMENT_NAME_C506) && p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT)) {
				p_node = p_node->children; // <E1153>
				if(sstreq(p_node->name, ELEMENT_NAME_E1153) && p_node->children) {
					// �������� �������� ��������
					str = (const char *)p_node->children->content;
					if(p_node->next) {
						p_node = p_node->next; // <E1154>
						if(p_node && sstreq(p_node->name, ELEMENT_NAME_E1154) && p_node->children) {
							if(str.CmpNC(ELEMENT_CODE_E1153_ON) == 0) {
								// ����� ������, �� ������� ������ �������������
								strcpy(pBill->OrderBillNo, (const char *)p_node->children->content);
								ok = 1;
							}
							else if(str.CmpNC(ELEMENT_CODE_E1153_AAS) == 0) {
								// ������ ��� ������ ��� DESADV
								// ����� ���
								(TTN = (const char *)p_node->children->content).Utf8ToOem();
								ok = 1;
							}
						}
					}
				}
			}
			else if(sstreq(p_node->name, ELEMENT_NAME_C507) && p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT)) {
				p_node = p_node->children; // <E2005>
				if(p_node && sstreq(p_node->name, ELEMENT_NAME_E2005) && p_node->children) {
					// �������� �������� ��������
					str = (const char *)p_node->children->content;
					if(p_node->next) {
						p_node = p_node->next; // <E2380>
						if(p_node && sstreq(p_node->name, "E2380") && p_node->children) {
							SString sub_str;
							if(str.CmpNC(ELEMENT_CODE_E2005_137) == 0) {
								// ������� ���� ���������
								str = (const char *)p_node->children->content;
								str.Sub(0, 4, sub_str);
								pBill->Date.setyear((uint)sub_str.ToLong());
								str.Sub(4, 2, sub_str);
								pBill->Date.setmonth((uint)sub_str.ToLong());
								str.Sub(6, 2, sub_str);
								pBill->Date.setday((uint)sub_str.ToLong());
								ok = 1;
							}
							else if((str.CmpNC(ELEMENT_CODE_E2005_17) == 0) || (str.CmpNC(ELEMENT_CODE_E2005_2) == 0)) {
								// ������� ���� �������� (���� ���������� ���������)
								str = (const char *)p_node->children->content;
								str.Sub(0, 4, sub_str);
								pBill->DueDate.setyear((uint)sub_str.ToLong());
								str.Sub(4, 2, sub_str);
								pBill->DueDate.setmonth((uint)sub_str.ToLong());
								str.Sub(6, 2, sub_str);
								pBill->DueDate.setday((uint)sub_str.ToLong());
								ok = 1;
							}
							else if(str.CmpNC(ELEMENT_CODE_E2005_171) == 0) {
								// ������� ���� ������
								str = (const char *)p_node->children->content;
								str.Sub(0, 4, sub_str);
								pBill->OrderDate.setyear((uint)sub_str.ToLong());
								str.Sub(4, 2, sub_str);
								pBill->OrderDate.setmonth((uint)sub_str.ToLong());
								str.Sub(6, 2, sub_str);
								pBill->OrderDate.setday((uint)sub_str.ToLong());
								ok = 1;
							}
						}
					}
				}
			}
			else if(sstreq(p_node->name, ELEMENT_NAME_E3035) && p_node->children) {
				// �������� �������� ��������
				str = (const char *)p_node->children->content;
				if(p_node->next) {
					p_node = p_node->next; // <C082>
					if(sstreq(p_node->name, ELEMENT_NAME_C082) && p_node->children) {
						p_node = p_node->children; // <E3039>
						if(sstreq(p_node->name, ELEMENT_NAME_E3039) && p_node->children) {
							if(str.CmpNC(ELEMENT_CODE_E3035_BY) == 0) {
								// ���������� GLN ����������
								strcpy(pBill->MainGLN, (const char *)p_node->children->content);
								strcpy(pBill->AgentGLN, (const char *)p_node->children->content);
								ok = 1;
							}
							else if(str.CmpNC(ELEMENT_CODE_E3035_SU) == 0) {
								// ���������� GLN ����������
								strcpy(pBill->GLN, (const char *)p_node->children->content);
								ok = 1;
							}
							else if(str.CmpNC(ELEMENT_CODE_E3035_DP) == 0) {
								// ���������� GLN ������ ��������
								strcpy(pBill->DlvrAddrCode, (const char *)p_node->children->content);
								ok = 1;
							}
							else if(str.CmpNC(ELEMENT_CODE_E3035_IV) == 0) {
								// ���������� GLN �����������
								strcpy(pBill->Obj2GLN, (const char *)p_node->children->content);
								ok = 1;
							}
						}
					}
				}
			}
			else if(sstreq(p_node->name, ELEMENT_NAME_E1229) && p_node->children) {
				// ���������� ��� �������� (���������, ������� ��� ���������, �� �������)
				str = (const char *)p_node->children->content;
				ok = 1;
			}
			else if(sstreq(p_node->name, ELEMENT_NAME_C516) && p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT)) {
				p_node = p_node->children; // <E5025>
				if(p_node && sstreq(p_node->name, ELEMENT_NAME_E5025) && p_node->children) {
					// �������� �������� �������� ��������
					str = (const char *)p_node->children->content;
					if(p_node->next) {
						p_node = p_node->next; // <E5004>
						if(p_node && sstreq(p_node->name, ELEMENT_NAME_E5004) && p_node->children) {
							if(str.CmpNC(ELEMENT_CODE_E5025_9) == 0) {
								// ������� ����� ��������� � ���
								pBill->Amount = atof((const char *)p_node->children->content);
								ok = 1;
							}
							else if(str.CmpNC(ELEMENT_CODE_E5025_98) == 0) {
								// ������� ����� ��������� ��� ���
								str = (const char *)p_node->children->content;
								ok = 1;
							}
						}
					}
				}
			}
			else if(sstreq(p_node->name, ELEMENT_NAME_E6069) && p_node->children) {
				if(sstreq(p_node->children->content, ELEMENT_CODE_E6069_2)) {
					if(p_node->next) {
						p_node = p_node->next; // <E6060>
						if(p_node && sstreq(p_node->name, ELEMENT_NAME_E6066) && p_node->children) {
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

// ��������� ���������� �������� � ��������� Sdr_BRow
// Returns:
//		-1 - ������� ��� �������� ������. ������, ���� � ��������� � ���� ������ ���������� �������� �����,
//			������� ��������� ����� �������� �������, �� ������ ������� �� ���������
//		 0 - ������
//		 1 - ������� ��������� �������� ������
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
		if(MessageType == msgOrdRsp)
			goods_segment = ELEMENT_NAME_SG26;
		else if(MessageType == msgDesadv)
			goods_segment = ELEMENT_NAME_S17;
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
				if(sstreq(p_node->name, goods_segment) && p_node->children) {
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
								if((p_node->type == XML_DOCUMENT_NODE) || sstreq(p_node->name, goods_segment)) // ������ ������� ��������� ��� ��������� �������� �������. ���� ����� �� ������, �� ���� ������ ������ �������� ������.
									sg26_end = 1;
								else if(sstreq(p_node->name, ELEMENT_NAME_LIN) && p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT)) {
									p_node = p_node->children; // <E1082>
									if(sstreq(p_node->name, ELEMENT_NAME_E1082) && p_node->children) {
										// ���������� ����� �������� �������
										str = (const char *)p_node->children->content;
										if((MessageType == msgOrdRsp) && p_node->next) {
											p_node = p_node->next; // <E1229>
											if(sstreq(p_node->name, ELEMENT_NAME_E1229) && p_node->children) {
												// ������� ������ �������� �������
												str = (const char *)p_node->children->content;
											}
										}
									}
								}
								else if(sstreq(p_node->name, ELEMENT_NAME_C212) && p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT)) {
									p_node = p_node->children; // <E7140>
									if(sstreq(p_node->name, ELEMENT_NAME_E7140) && p_node->children) {
										// �������� �������� �������� �������� (��������)
										str = (const char *)p_node->children->content;
										if(p_node->next) {
											p_node = p_node->next; // <E7143>
											if(sstreq(p_node->name, ELEMENT_NAME_E7143) && p_node->children) {
												if(sstreq(p_node->children->content, ELEMENT_CODE_E7143_SRV))
													// ������� �������� ������
													str.CopyTo(pBRow->Barcode, sizeof(pBRow->Barcode));
											}
										}
									}
								}
								else if(sstreq(p_node->name, ELEMENT_NAME_E4347) && p_node->children) {
									if(sstreq(p_node->children->content, ELEMENT_CODE_E4347_1)) {
										if(p_node->next && p_node->next->children && (p_node->next->children->type == XML_READER_TYPE_ELEMENT)) {
											p_node = p_node->next->children; // <E7140> (��������� <C212>)
											if(sstreq(p_node->name, ELEMENT_NAME_E7140) && p_node->children) {
												// �������� �������� �������� �������� (������� ������ � ����������)
												str = (const char *)p_node->children->content;
												if(p_node->next) {
													p_node = p_node->next; // <E7143>
													if(sstreq(p_node->name, ELEMENT_NAME_E7143) && p_node->children) {
														if(sstreq(p_node->children->content, ELEMENT_CODE_E7143_SA))
															// ������� ������� ����������
															str.CopyTo(pBRow->ArCode, sizeof(pBRow->ArCode));
													}
												}
											}
										}
									}
								}
								else if(sstreq(p_node->name, ELEMENT_NAME_E7008) && p_node->children) {
									// ���������� ������������ ������
									(str = (const char *)p_node->children->content).Utf8ToOem();
									str.CopyTo(pBRow->GoodsName, str.Len() + 1);
								}
								else if(sstreq(p_node->name, ELEMENT_NAME_C186) && p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT)) {
									p_node = p_node->children; // <E6063>
									if(sstreq(p_node->name, ELEMENT_NAME_E6063) && p_node->children) {
										// �������� �������� �������� ��������
										str = (const char *)p_node->children->content;
										if(p_node->next) {
											p_node = p_node->next; // <E6060>
											if(sstreq(p_node->name, ELEMENT_NAME_E6060) && p_node->children) {
												if(str == "21") // ���������� ���������� ����������
													str = (const char *)p_node->children->content;
												// ELEMENT_CODE_E6063_12 �� DESADV
												else if(str == "113" || str == "170" || str == "12")
													pBRow->Quantity = atof((const char *)p_node->children->content); // ���������� �������������� ����������
												else if(str == "59")
													pBRow->UnitPerPack = atof((const char *)p_node->children->content); // ���������� ���������� ������ � ��������
											}
										}
									}
								}
								else if(sstreq(p_node->name, ELEMENT_NAME_E6411) && p_node->children) {
									// ���������� ������� ������
									if(sstreq(p_node->children->content, ELEMENT_CODE_E6411_KGM))
										strcpy(pBRow->UnitName, UNIT_NAME_KG);
									else
										strcpy(pBRow->UnitName, UNIT_NAME_PIECE);
								}
								else if(sstreq(p_node->name, ELEMENT_NAME_C516) && p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT)) {
									p_node = p_node->children; // <E5025>
									if(sstreq(p_node->name, ELEMENT_NAME_E5025) && p_node->children) {
										// �������� �������� �������� ��������
										str = (const char *)p_node->children->content;
										if(p_node->next) {
											p_node = p_node->next; // <E5004>
											if(sstreq(p_node->name, ELEMENT_NAME_E5004) && p_node->children) {
												if(str.CmpNC(ELEMENT_CODE_E5025_203) == 0)
													// ������� ����� �������� ������� ��� ���
													str = (const char *)p_node->children->content;
												else if(str.CmpNC(ELEMENT_CODE_E5025_79) == 0)
													// ������� ����� �������� ������� � ���
													str = (const char *)p_node->children->content;
												else if((MessageType == msgDesadv) && (str.CmpNC(ELEMENT_CODE_E5125_XB5) == 0)) {
													// ������� ���� ������ � ��� ��� DESADV
													pBRow->Cost = atof((const char *)p_node->children->content);
												}
											}
										}
									}
								}
								else if(sstreq(p_node->name, ELEMENT_NAME_C509) && p_node->children && (p_node->children->type == XML_READER_TYPE_ELEMENT)) {
									p_node = p_node->children; // <E5125>
									if(sstreq(p_node->name, ELEMENT_NAME_E5125) && p_node->children) {
										// �������� �������� �������� ��������
										str = (const char *)p_node->children->content;
										if(p_node->next) {
											p_node = p_node->next; // <E5118>
											if(sstreq(p_node->name, ELEMENT_NAME_E5118) && p_node->children) {
												if(str.CmpNC(ELEMENT_CODE_E5125_AAA) == 0)
													// ������� ���� ������ ��� ���
													str = (const char *)p_node->children->content;
												else if(str.CmpNC(ELEMENT_CODE_E5125_AAE) == 0)
													// ������� ���� ������ � ��� ��� ORDRSP
													pBRow->Cost = atof((const char *)p_node->children->content);
											}
										}
									}
								}
							}
						}
					}
					// ����� ���. �������� �� ��������� ���������
					if((MessageType == msgDesadv) && TTN.NotEmpty())
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

//
// ������� ������� �������
//

EXPORT int InitImport(void * pImpHeader, const char * pInputFileName, int * pId)
{
	int    ok = 1;
	SPathStruc log_path;
	SString str;
	SETIFZ(P_ImportCls, new ImportCls);
	if(P_ImportCls && !P_ImportCls->Inited) {
		if(pImpHeader) {
			P_ImportCls->Header = *static_cast<const Sdr_ImpExpHeader *>(pImpHeader);
			SString str;
			//FormatLoginToLogin(P_ImportCls->Header.EdiLogin, str);
			//str.CopyTo(P_ImportCls->Header.EdiLogin, sizeof(P_ImportCls->Header.EdiLogin));
		}
		if(!isempty(pInputFileName)) {
			P_ImportCls->PathStruct.Split(pInputFileName);
			if(P_ImportCls->PathStruct.Nam.Empty())
				(P_ImportCls->PathStruct.Nam = "edisoft_import_").Cat(P_ImportCls->ObjId);
			if(P_ImportCls->PathStruct.Ext.Empty())
				P_ImportCls->PathStruct.Ext = "xml";
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
	int ok = 1, r = 0;
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
	THROWERR((P_ImportCls->MessageType == msgOrdRsp) || (P_ImportCls->MessageType == msgAperak) ||
		(P_ImportCls->MessageType == msgDesadv), IEERR_ONLYIMPMSGTYPES);
	// �������� ��������
	P_ImportCls->CreateFileName(P_ImportCls->ObjId);
	THROW(r = P_ImportCls->ReceiveDoc());
	if(r == -1)
		ok = -1;
	else {
		if(((P_ImportCls->MessageType == msgOrdRsp) || (P_ImportCls->MessageType == msgDesadv)) && (r == 1)) // ��� ���� r == -2, �� ��� �� ���� �������� ��������� ����, ��� ��� ���
			// ������ �������� � ��������� Sdr_Bill
			THROW(P_ImportCls->ParseForDocData((Sdr_Bill *)pObjData))
		else if(P_ImportCls->MessageType == msgAperak) {
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
	int ok = 1, r = 1;
	size_t pos = 0;
	SString str;
	// ���� � Papyrus ���� �����, �� ������� �������� ������������� ��� ������
	if(((Sdr_DllImpObjStatus *)pObjStatus)->DocStatus == docStatIsSuchDoc) {
		if(P_ImportCls) {
			if(P_ImportCls->MessageType == msgAperak) {
				// ���-������ ������ � ���� ��������
				str.Z().Cat(P_ImportCls->AperakInfo.OrderNum.ToChar()).CatDiv(':', 2).Cat(P_ImportCls->AperakInfo.Msg.Utf8ToChar());
				if(P_ImportCls->AperakInfo.AddedMsg.NotEmpty())
					str.CatDiv(':', 2).Cat(P_ImportCls->AperakInfo.AddedMsg.Utf8ToChar());
				LogMessage(str);
			}
			// � ������� ���� ���� �� ����� �� ftp
			FtpClient ftp_client(P_ImportCls->Header.EdiLogin, P_ImportCls->Header.EdiPassword);
			if(ftp_client.Connect()) {
				if(!ftp_client.DeleteFile(P_ImportCls->InboxFiles.Get(P_ImportCls->InboxReadIndex).Txt)) {
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
	// ���� � Papyrus ������ ������ ���
	else if(((Sdr_DllImpObjStatus *)pObjStatus)->DocStatus == docStatNoSuchDoc) {
		if(P_ImportCls) {
			if((P_ImportCls->MessageType == msgOrdRsp) || (P_ImportCls->MessageType == msgDesadv)) {
				// ������� ��������� ����� xml-��������
				SFile file;
				file.Remove(P_ImportCls->ImpFileName);
			}
			//else if(P_ImportCls->MessageType == msgAperak) {
			//	// ���� �� ������ ����� ������ ��� GLN ������ ��������, �� ������� ���� ���� �� FTP,
			//	// ��� �� ��� ����� �� ����� ��������� �� ����� ���������
			//	if(P_ImportCls->AperakInfo.AddrGLN.Empty() || P_ImportCls->AperakInfo.OrderNum.Empty()) {
			//		FtpClient ftp_client(P_ImportCls->Header.EdiLogin, P_ImportCls->Header.EdiPassword);
			//		if(ftp_client.Connect()) {
			//			if(!ftp_client.DeleteFile(P_ImportCls->InboxFiles.at(P_ImportCls->InboxReadIndex).Txt)) {
			//				ProcessError("FtpDeleteFile");
			//				r = 0;
			//			}
			//		}
			//		else {
			//			ProcessError("FtpConnect");
			//			r = 0;
			//		}
			//	}
			//}
		}
	}
	if(!r) {
		SysLogMessage(SYSLOG_REPLYIMPORTOBJSTATUS);
		GetErrorMsg(str.Z());
		LogMessage(str);
		SysLogMessage(str);
	}
	if(P_ImportCls)
		P_ImportCls->InboxReadIndex++;
	return ok;
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
		if(!P_ImportCls->InboxFiles.getCount()) {
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
	SString str = "";
	GetErrorMsg(str.Z());
	memzero(pMsg, bufLen);
	if(str.NotEmpty() && pMsg)
		str.CopyTo(pMsg, bufLen < (str.Len() + 1) ? bufLen : (str.Len() + 1));
	ErrorCode = 0;
	StrError = "";
	return 1;
}

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
