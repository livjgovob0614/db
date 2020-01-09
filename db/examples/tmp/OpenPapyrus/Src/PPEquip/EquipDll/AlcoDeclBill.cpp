// ALCODECLBILL.CPP
// ���������� ��� �������� ����������� ��������� � xml
//

// � ���������� �������� ��������� ������ �� ���� ������: ������ ������������ (��������������-����������),
// ������ �������� � ������ ����.
// ����� ������������ � ����������� � ���������� - ������� ������������.
// � ������ ����������� ����� � ���������, �����, ������������� (���������������). ���������� ��� ������ � �����
// � ��������� ����� ���������� ��������� ����� �����������. ���� � ������������� � ����������� ����.
//
#include <slib.h>
#include <ppbrow.h>
#include <sxml.h>

#define EXPORT	extern "C" __declspec (dllexport)
#define THROWERR(expr,val) { if(!(expr)){ SetError(val, ""); goto __scatch; } }
#define THROWERR_STR(expr,val,str) { if(!(expr)){ SetError(val, str); goto __scatch; } }

// ����� ��������� xml-�����
#define ELEMENT_NAME_FILE			"����"
#define ELEMENT_NAME_DOCUMENT		"��������"
#define ELEMENT_NAME_TURN			"������"
#define ELEMENT_NAME_MANUFDATA		"����������������"
#define ELEMENT_NAME_PRODUCT		"���������"
#define ELEMENT_NAME_MOVEMENT		"��������"
#define ELEMENT_NAME_CATALOG		"�����������"
#define ELEMENT_NAME_REQUISITES		"���������"
#define ELEMENT_NAME_RESPPERS		"���������"	// ������������� ����
#define ELEMENT_NAME_CONTRAGS		"�����������"
#define ELEMENT_NAME_RESIDENT		"��������"
#define ELEMENT_NAME_LICENSES		"��������"
#define ELEMENT_NAME_LICENSE		"��������"
#define ELEMENT_NAME_P000000000008	"�000000000008"	// ����� �����������
#define ELEMENT_NAME_COUNTRYCODE	"���������"
#define ELEMENT_NAME_INDEX			"������"
#define ELEMENT_NAME_REGIONCODE		"���������"
#define ELEMENT_NAME_DISTRICT		"�����"
#define ELEMENT_NAME_TOWN			"�����"
#define ELEMENT_NAME_COMMUNITY		"����������"
#define ELEMENT_NAME_STREET			"�����"
#define ELEMENT_NAME_HOUSE			"���"
#define ELEMENT_NAME_HOUSING		"������"
#define ELEMENT_NAME_LETTER			"������"
#define ELEMENT_NAME_SQUARE			"�����"
#define ELEMENT_NAME_LPERSON		"��"
#define ELEMENT_NAME_PRODUCER		"�������������"
#define ELEMENT_NAME_TRANSPORTER	"����������"

// ����� ��������� xml-�����
#define ATRIBUTE_NAME_DATEDOC		"�������"		// ���� ��������
#define ATRIBUTE_NAME_VERSFORM		"��������"
#define ATRIBUTE_NAME_PROGNAME		"��������"
#define ATRIBUTE_NAME_PN			"�N"			// ���������� �����
#define ATRIBUTE_NAME_P000000000003	"�000000000003"	// ��� ���� ����������
#define ATRIBUTE_NAME_P000000000007	"�000000000007"	// ������������ �����������
#define ATRIBUTE_NAME_P000000000009	"�000000000009"	// ��� �����������
#define ATRIBUTE_NAME_P000000000010	"�000000000010"	// ��� �����������
#define ATRIBUTE_NAME_P000000000011	"�000000000011"	// �����,����� ��������
#define ATRIBUTE_NAME_P000000000012	"�000000000012"	// ���� ������ ��������
#define ATRIBUTE_NAME_P000000000013	"�000000000013"	// ���� ��������� ��������
#define ATRIBUTE_NAME_P000000000014	"�000000000014"	// �������������� �����
#define ATRIBUTE_NAME_NAMEORG		"NameOrg"
#define ATRIBUTE_NAME_INN			"INN"
#define ATRIBUTE_NAME_KPP			"KPP"
#define ATRIBUTE_NAME_P200000000013	"�200000000013"	// ���� ���������
#define ATRIBUTE_NAME_P200000000014	"�200000000014"	// ����� ���
#define ATRIBUTE_NAME_P200000000015	"�200000000015"	// ���
#define ATRIBUTE_NAME_P200000000016	"�200000000016"	// ���������� � ����������
#define ATRIBUTE_NAME_P100000000006	"�100000000006"
#define ATRIBUTE_NAME_P100000000007	"�100000000007"
#define ATRIBUTE_NAME_P100000000008	"�100000000008"	// ����� ���������� ������ � ����������
#define ATRIBUTE_NAME_P100000000009	"�100000000009"
#define ATRIBUTE_NAME_P100000000010	"�100000000010"	// ����� ���������� ������ � ����������
#define ATRIBUTE_NAME_P100000000011	"�100000000011"
#define ATRIBUTE_NAME_P100000000012	"�100000000012"
#define ATRIBUTE_NAME_P100000000013	"�100000000013"
#define ATRIBUTE_NAME_P100000000014	"�100000000014"	// ����� ���������� ������ � ����������
#define ATRIBUTE_NAME_P100000000015	"�100000000015"
#define ATRIBUTE_NAME_P100000000016	"�100000000016"
#define ATRIBUTE_NAME_P100000000017	"�100000000017"
#define ATRIBUTE_NAME_P100000000018	"�100000000018"
#define ATRIBUTE_NAME_P100000000019	"�100000000019"
#define ATRIBUTE_NAME_P100000000020	"�100000000020"
#define ATRIBUTE_NAME_IDCONTR		"�������"
#define ATRIBUTE_NAME_IDLICENSE		"����������"
#define ATRIBUTE_NAME_VALUE			"value"

// ������ ���������
//#define KVARTAL_1		3	// 1 �������
//#define KVARTAL_2		6	// 2 �������
//#define KVARTAL_3		9	// 3 �������
//#define KVARTAL_4		0	// 4 �������

// ���� ������ � ���������
#define IEERR_SYMBNOTFOUND			1			// ������ �� ������ (������ ���� ������� �������/��������)
#define IEERR_NODATA				2			// ��� ������
#define IEERR_NOSESS				3			// ������ � ����� ������� �� ����������
#define IEERR_ONLYBILLS				4			// ������ DLL ����� �������� ������ � �����������
#define IEERR_NOOBJID				5			// ������� � ����� ��������������� ���
#define IEERR_IMPEXPCLSNOTINTD		6			// ������ ��� �������/�������� �� ���������������
#define IERR_IMPFILENOTFOUND		7			// ���� ������� �� ������: %s
#define IERR_INVMESSAGEYTYPE		8			// �������� ��� ���������. ��������� %S
#define IERR_ARRAYNOTINITED			9			// ������ ������������� �������

class ExportCls;

int ErrorCode = 0;
SString StrError = "";
static ExportCls * P_ExportCls = 0;

int GetObjTypeBySymb(const char * pSymb, uint & rType);
int SetError(int errCode, const char * pStr = "") { ErrorCode = errCode, StrError = pStr; return 1; }

/*struct ErrMessage {
	uint Id;
	const char * P_Msg;
};*/

/*struct ObjectTypeSymbols {
	char * P_Symb;
	uint Type;
};*/

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

/*ObjectTypeSymbols*/SIntToSymbTabEntry Symbols[] = {
	{objGood,  "GOODS"},
	{objBill,  "BILLS"},
	{objCheck, "CHECKS"},
	{objCashSess, "CSESS"},
	{objPriceList, "PRICELIST"},
	{objLot, "LOTS"},
	{objPhoneList, "PHONELIST"},
	{objCliBnkData, "CLIBANKDATA"}
};

/*ErrMessage*/SIntToSymbTabEntry ErrMsg[] = {
	{IEERR_SYMBNOTFOUND,		"������ �� ������"},
	{IEERR_NODATA,				"������ �� ��������"},
	{IEERR_NOSESS,				"������ � ����� ������� ���"},
	{IEERR_ONLYBILLS,			"Dll ����� �������� ������ � �����������"},
	{IEERR_NOOBJID,				"������� � ����� ��������������� ���"},
	{IEERR_IMPEXPCLSNOTINTD,	"������ ��� �������/�������� �� ���������������"},
	{IERR_IMPFILENOTFOUND,		"���� ������� �� ������: "},
	{IERR_INVMESSAGEYTYPE,		"�������� ��� ���������. ��������� "},
	{IERR_ARRAYNOTINITED,		"������ ������������� �������"}
};

enum FileTypes {
	fContrag = 0x0001,
	fBeer = 0x0002,
	fAlco = 0x0004,
	fReturn = 0x0008
};

class Iterator {
public:
	void Init()
	{
		Count = 0;
	}
	const uint GetCount()
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

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    return true;
}

struct GoodInfoSt {
	GoodInfoSt()
	{
		Clear();
	}
	void Clear()
	{
		Quantity = 0.0;
		ReceiptDate = ZERODATE;
		GoodKind = 0;
		OrgName.Z();
		INN.Z();
		KPP.Z();
		TTN.Z();
		GTD.Z();
	};
	double Quantity;
	LDATE  ReceiptDate;
	long   GoodKind;
	SString OrgName;
	SString INN;
	SString KPP;
	SString	TTN;
	SString GTD;
};

struct ContragInfoSt {
	ContragInfoSt()
	{
		Clear();
	}
	void Clear()
	{
		House = 0;
		CountryCode = 0;
		RegionCode = 0;
		Date = ZERODATE;
		Expiry = ZERODATE;
		Index.Z();
		ContragName.Z();
		LicOrgName.Z();
		LicID = 0;
		LicSerial.Z();
		LicNum.Z();
		District.Z();
		Town.Z();
		Community.Z();
		Street.Z();
		Housing.Z();
		Letter.Z();
		INN.Z();
		KPP.Z();
		Producer.Z();
		Transporter.Z();
		IsManuf = 0;
	}
	int		House;
	int		CountryCode;
	int		RegionCode;
	int		IsManuf;
	long	LicID;
	LDATE	Date;
	LDATE	Expiry;
	SString	Index;
	SString ContragName;
	SString LicOrgName;
	SString LicSerial;
	SString LicNum;
	SString	District;
	SString Town;
	SString Community;
	SString Street;
	SString	Housing;
	SString	Letter;
	SString INN;
	SString KPP;
	SString Producer;
	SString Transporter;
};

struct ShopInfoSt {
	ShopInfoSt()
	{
		Init();
	}
	~ShopInfoSt()
	{
		Clear();
	}
	void Init()
	{
		ShopId = 0;
		ContragName = 0;
		ShopAddr = 0;
		ShopAlco.freeAll();
		ShopBeer.freeAll();
		//ShopContrag.freeAll();
	}
	void Clear()
	{
		ShopAlco.freeAll();
		ShopBeer.freeAll();
		//ShopContrag.freeAll();
	}
	long   ShopId;
	SString ContragName;
	SString ShopAddr;
	TSCollection <GoodInfoSt> ShopAlco;
	TSCollection <GoodInfoSt> ShopBeer;
	TSCollection <GoodInfoSt> ShopAlcoRet; // @vmiller new
	TSCollection <GoodInfoSt> ShopBeerRet; // @vmiller new
	//TSCollection <ContragInfoSt> ShopContrag;
};
//
// �������:
// ��� ��������� ����� ������������ �� �����. ������ ���� ����.
// ����� ����, ����� ��� ����� ����� ��� �������.
// ���� �����, ��� ������ ������������
// ��� ������ ��� ������ ������������, � ������, � ������� �������. ���� ����������, ��� � ��� ��� ����.
// ��� �������� ������ ���������, �������� ����� ���������. ���� ���������� �����������, �� ������ � ��������� ��� �������� �����,
// ������� � ���� ���������.
//
class ExportCls {
public:
	ExportCls();
	~ExportCls();
	void   CreateFileName(long DocType, const char * pContragName, const char * pShopAddr, uint grpNum, SString & rFileName);
	//
	// Descr: ��������� ������� � ����� �� �������� � ����
	//
	int    GetInfo(Sdr_BRow * pBRow, TSCollection <GoodInfoSt> * pArr);
	int    GetContragInfo(Sdr_BRow * pBRow, TSCollection <ContragInfoSt> * pArr); // ��������� ������ � ����� � ������������
	int    SaveInfo(TSCollection <GoodInfoSt> * pArr, xmlTextWriter * pXmlPtr, int isBeer = 0); // ��������� ������ �� �������� � ���� � ��������������� �����
	int    SaveContragInfo(TSCollection <ContragInfoSt> * pArr); // ��������� ������ � ������������ � ����

	uint   Id;					// ��, ������� Papyrus ����� ������������ ��� �� ������ ��������
	uint   ObjId;				// �� ��������������� ������� (��������, ��� ������ ���������� ������� ��������� ������������� ���� ��)
	uint   ObjType;				// ��� �������������� ��������
	uint   Inited;
	//uint   Kvart;				// ����� �������� ����������. ��� ����� �������� �����
	//uint   Year;				// ��� ����������. ��� ����� ��������� �����
	//long BeerTypeId;			// �� ������ ������ "����"
	long   ShopPos;				// ������� ���� � ������� ShopInfo ��� �������� ��������
	LDATE  BillDate;
	xmlTextWriter * P_WXmlAlco;
	xmlTextWriter * P_WXmlBeer;
	xmlTextWriter * P_WXmlAlcoRet; // @vmiller new
	xmlTextWriter * P_WXmlBeerRet; // @vmiller new
	xmlTextWriter * P_WXmlContrag;
	Iterator Itr;
	SPathStruc PathStruct;
	SString Prefix;				// ������� ����� ���������. ��� �� ����� ���������, ����� �����
	Sdr_ImpExpHeader Header;
	TSCollection <ShopInfoSt> ShopInfo;	// ������ ����� �������� ��� ������� ��������
	TSCollection <ContragInfoSt> ContragInfo; // ������ � ����� � ������������ (���������������/����������)
private:
	void   GetPeriod(LDATE billDate, int quarterNum, int year);
	SString & DeleteExtSymbols(const char * pIn, SString & rOut);
};

ExportCls::ExportCls() : Id(0), ObjId(0), ObjType(0), ShopPos(0), Inited(0), BillDate(ZERODATE), P_WXmlAlco(0), P_WXmlBeer(0),
	P_WXmlAlcoRet(0), P_WXmlBeerRet(0), P_WXmlContrag(0)
{
	ErrorCode = 0;
	//	BeerTypeId = 0; // @vmiller
}

ExportCls::~ExportCls()
{
	P_WXmlAlco = 0;
	P_WXmlBeer = 0;
	P_WXmlAlcoRet = 0; // @vmiller new
	P_WXmlBeerRet = 0; // @vmiller new
	P_WXmlContrag = 0;
	ShopInfo.freeAll();
	ContragInfo.freeAll();
}

static SString & PreprocessFnText(SString & rT)
{
	SString temp_buf;
	const size_t src_len = rT.Len();
	for(size_t i = 0; i < src_len; i++) {
		const char c = rT.C(i);
		if(oneof8(c, ',', '\\', '/', '.', '\"', '*', ':', '?')) 
			temp_buf.CatChar(' ');
		else
			temp_buf.CatChar(c); // @v10.5.3
	}
	temp_buf.ReplaceStr("  ", " ", 0).Strip();
	/*rT.ReplaceChar(',', ' ').ReplaceChar('\\', ' ').ReplaceChar('/', ' ').ReplaceChar('.', ' ').
		ReplaceChar('\"', ' ').ReplaceChar('*', ' ');
	rT.ReplaceStr("  ", " ", 0).Strip();*/
	rT = temp_buf;
	return rT;
}

void ExportCls::CreateFileName(long DocType, const char * pContragName, const char * pShopAddr, uint grpNum, SString & rFileName)
{
	int doc_num = 1, exit_while = 0;
	SString fmt, dir;
	SFile file;
	while(!exit_while) {
		rFileName.Z().Cat(PathStruct.Drv).CatChar(':').Cat(PathStruct.Dir);
		if(DocType & fContrag) {
			// ����� � ������ �����������
			if(pContragName) {
				PreprocessFnText(fmt = pContragName);
				rFileName.Cat(fmt).CatChar('\\');
				dir = rFileName;
			}
			//rFileName.Cat("�����������_GRP").Cat(grpNum).CatChar('(').Cat(doc_num).CatChar(')').Dot().Cat(PathStruct.Ext);
			rFileName.Cat("�����������").CatChar('(').Cat(/*doc_num*/1).CatChar(')').Dot().Cat(PathStruct.Ext);
			// ���� ����� ���� ��� ����������, �� ������� ���� � �������, ������� �� �������

			// ��� �� �������
			exit_while = 1;
		}
		else {
			// ����� � ������ �����������
			if(pContragName) {
				PreprocessFnText(fmt = pContragName);
				rFileName.Cat(fmt).CatChar('\\');
				dir = rFileName;
			}
			// �������
			if(DocType & fReturn)
				rFileName.Cat("Return_");
			// �������
			rFileName.Cat(Prefix);
			// ��� �������, ������� ����� ����������� � ����� (������� �������� ��� ����)
			if(DocType & fAlco)
				rFileName.Cat("11_");
			else if(DocType & fBeer)
				rFileName.Cat("12_");
			// ����� �������� (��� �����)
			//Year = BillDate.year();
			//Kvart = (uint)ceil((double)BillDate.month() / 3);
			//if(Kvart <= 3)
			//	Kvart = KVARTAL_1;
			//else if((Kvart <= 6) && (Kvart >= 4))
			//	Kvart = KVARTAL_2;
			//else if((Kvart <= 9) && (Kvart >=5))
			//	Kvart = KVARTAL_3;
			//else if(Kvart >= 10)
			//	Kvart = KVARTAL_4;
			//rFileName.CatChar('0').Cat(Kvart);
			//// ������ ����� ����
			//fmt.Z().Cat(BillDate.year());
			//rFileName.CatChar(fmt.C(0)).CatChar('_');
			// ���� ��������
			/*LDATE date;
			getcurdate(&date);
			fmt.Z().Cat(date.day());
			if(fmt.Len() == 1)
				fmt.PadLeft(1, '0');
			rFileName.Cat(fmt);
			fmt.Z().Cat(date.month());
			if(fmt.Len() == 1)
				fmt.PadLeft(1, '0');
			rFileName.Cat(fmt).Cat(date.year());*/
			// �� ���� ������� �� ��������
			//rFileName.CatChar('_').Cat(shopId).CatChar('_');
			// ����� ��������
			if(pShopAddr) {
				PreprocessFnText(fmt = pShopAddr);
				rFileName.Cat(fmt);
			}
			rFileName.CatChar('(').Cat(doc_num).CatChar(')').Dot().Cat(PathStruct.Ext);
			// ����� �����
			//rFileName.Cat("GRP").Cat(grpNum).CatChar('(').Cat(doc_num).CatChar(')').Dot().Cat(PathStruct.Ext);
		}
		if(pContragName && !pathValid(dir, 1))
			createDir(dir);
		// ���� ���� � ����� ������ ����������, ����������� ����� �����
		if(!file.Open(rFileName, SFile::mRead))
			exit_while = 1;
		else
			doc_num++;
	}
}

int ExportCls::GetInfo(Sdr_BRow * pBRow, TSCollection <GoodInfoSt> * pList)
{
	int    ok = 1;
	THROWERR(pBRow, IEERR_NODATA);
	THROWERR(pList, IERR_ARRAYNOTINITED);
	{
		GoodInfoSt * p_new_item = new GoodInfoSt;
		p_new_item->GoodKind = pBRow->GoodKindCode;
		p_new_item->OrgName = pBRow->LotManuf;
		p_new_item->INN = pBRow->ManufINN;
		p_new_item->KPP = pBRow->ManufKPP;
		p_new_item->ReceiptDate = pBRow->BillDate/*LotDocDate*/;
		p_new_item->TTN = /*pBRow->TTN*/pBRow->BillCode; // � ��������� ����� �������� ��������� � ������� ���
		p_new_item->GTD = pBRow->GTD;
		p_new_item->Quantity = fabs(pBRow->PhQtty);
		pList->insert(p_new_item);
	}
	CATCHZOK
	return ok;
}

int ExportCls::GetContragInfo(Sdr_BRow * pBRow, TSCollection <ContragInfoSt> * pArr)
{
	int    ok = 1, same_found = 0;
	THROWERR(pBRow, IEERR_NODATA);
	for(size_t i = 0; (i < pArr->getCount()) && !same_found; i++) {
		const ContragInfoSt * p_item = pArr->at(i);
		if(p_item) {
			if(!p_item->ContragName.CmpNC(pBRow->LotManuf) && !p_item->INN.CmpNC(pBRow->ManufINN) &&
				!p_item->KPP.CmpNC(pBRow->ManufKPP) && !p_item->LicSerial.CmpNC(pBRow->LicenseSerial) &&
				!p_item->LicNum.CmpNC(pBRow->LicenseNum) && (p_item->LicID == pBRow->LicenseID))
				same_found = 1;
		}
	}
	if(!same_found) {
		ContragInfoSt * p_new_item = new ContragInfoSt;
		p_new_item->ContragName = pBRow->LotManuf;
		p_new_item->LicOrgName = pBRow->RegAuthority;
		p_new_item->Expiry = pBRow->LicenseExpiry;
		p_new_item->Date = pBRow->LicenseDate;
		p_new_item->LicSerial = pBRow->LicenseSerial;
		p_new_item->LicNum = pBRow->LicenseNum;
		p_new_item->LicID = pBRow->LicenseID;
		p_new_item->CountryCode = 643; // @vmiller
		p_new_item->Index = pBRow->ManufIndex;
		p_new_item->RegionCode = pBRow->ManufRegionCode;
		p_new_item->District = pBRow->ManufDistrict;
		p_new_item->Town = pBRow->ManufCityName;
		p_new_item->Community.Z();
		p_new_item->Street = pBRow->ManufStreet;
		p_new_item->House = pBRow->ManufHouse;
		p_new_item->Housing.Z().Cat(pBRow->ManufHousing);
		p_new_item->Letter.Z();
		p_new_item->INN = pBRow->ManufINN;
		p_new_item->KPP = pBRow->ManufKPP;
		p_new_item->IsManuf = pBRow->IsManuf;
		pArr->insert(p_new_item);
	}
	CATCHZOK
	return ok;
}

int ExportCls::SaveInfo(TSCollection <GoodInfoSt> * pArr, xmlTextWriter * pXmlPtr, int isBeer/* = 0*/)
{
	// index - ���������� ����� ������ �������
	int    ok = 1, index = 1;
	long   good_kind_code = 0;
	double total_qtt = 0.0;
	SString str, fmt;
	LDATE  date = ZERODATE;
	TSCollection <GoodInfoSt> one_grp;
	SString org_name = "", inn = "", kpp = "";

	THROW(pXmlPtr);

	xmlTextWriterStartElement(pXmlPtr, (const xmlChar *)ELEMENT_NAME_FILE);
		xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_DATEDOC);
			getcurdate(&date);
			fmt.Z().Cat(date.day());
			if(fmt.Len() == 1)
				fmt.PadLeft(1, '0');
			str.Z().Cat(fmt);
			fmt.Z().Cat(date.month());
			if(fmt.Len() == 1)
				fmt.PadLeft(1, '0');
			str.Dot().Cat(fmt).Dot().Cat(date.year());
			xmlTextWriterWriteString(pXmlPtr, str.ucptr());
		xmlTextWriterEndAttribute(pXmlPtr); //�������
		xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_VERSFORM);
			xmlTextWriterWriteString(pXmlPtr, (const xmlChar *)"4.20"); // ������ ������������� @v8.6.12 "4.20"-->"4.30"; @v8.7.11 "4.30"-->"4.20"
		xmlTextWriterEndAttribute(pXmlPtr); //��������
		xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_PROGNAME);
			xmlTextWriterWriteString(pXmlPtr, (const xmlChar *)Header.SrcSystemName);
		xmlTextWriterEndAttribute(pXmlPtr); //��������

		xmlTextWriterStartElement(pXmlPtr, (const xmlChar *)ELEMENT_NAME_DOCUMENT);
		// GoodsKind
		// OrgName
		// ReceiptDate
			while(pArr->getCount()) {
				// ��������� �� ���� ������
				GoodInfoSt good_info;
				const GoodInfoSt * p_item = pArr->at(0);
				good_kind_code = p_item->GoodKind;
				//
				// ������� ������ � ���� ������
				//
				xmlTextWriterStartElement(pXmlPtr, (const xmlChar *)ELEMENT_NAME_TURN);
					xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_PN);
						xmlTextWriterWriteString(pXmlPtr, str.Z().Cat(index).ucptr());
					xmlTextWriterEndAttribute(pXmlPtr); //�N
					xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_P000000000003);
						xmlTextWriterWriteString(pXmlPtr, str.Z().Cat(good_kind_code).ucptr());
					xmlTextWriterEndAttribute(pXmlPtr); //�0000000003
				for(uint j = 0; j < pArr->getCount();) {
					one_grp.freeAll();
					total_qtt = 0.0;
					const GoodInfoSt * p_item_j = pArr->at(j);
					org_name = p_item_j->OrgName;
					inn = p_item_j->INN;
					kpp = p_item_j->KPP;
					for(uint i = 0; i < pArr->getCount();) {
						//
						// ��������� �� �����������
						//
						const GoodInfoSt * p_item_i = pArr->at(i);
						if((p_item_i->GoodKind == good_kind_code) && p_item_i->OrgName.CmpNC(org_name) == 0 && p_item_i->INN.CmpNC(inn) == 0 && p_item_i->KPP.CmpNC(kpp) == 0) {
							GoodInfoSt * p_new_item = new GoodInfoSt;
							*p_new_item = *p_item_i;
							one_grp.insert(p_new_item);
							pArr->atFree(i);
						}
						else
							i++;
					}
					if(one_grp.getCount()) {
						// ���������� ������ � ������� ����������� ����
						for(size_t k = 0; k < one_grp.getCount(); k++) {
							const  GoodInfoSt * p_item_k = one_grp.at(k);
							LDATE  min_date = p_item_k->ReceiptDate;
							size_t min_index = k;
							for(size_t d = 0; d < one_grp.getCount(); d++) {
								const  GoodInfoSt * p_item_d = one_grp.at(d);
								if(p_item_d->ReceiptDate < min_date) {
									min_date = p_item_d->ReceiptDate;
									min_index = d;
								}
								one_grp.swap(d, min_index);
							}
						}
						// ��� ������ ����� ���������� ������
							xmlTextWriterStartElement(pXmlPtr, (const xmlChar *)ELEMENT_NAME_MANUFDATA);
								xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_NAMEORG);
									xmlTextWriterWriteString(pXmlPtr, org_name.ucptr());
								xmlTextWriterEndAttribute(pXmlPtr); //NameOrg
								xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_INN);
									xmlTextWriterWriteString(pXmlPtr, inn.ucptr());
								xmlTextWriterEndAttribute(pXmlPtr); //INN
								xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_KPP);
									xmlTextWriterWriteString(pXmlPtr, kpp.ucptr());
								xmlTextWriterEndAttribute(pXmlPtr); //KPP
								// ����� ���� � ������ ������
								for(size_t k = 0; k < one_grp.getCount(); k++) {
									const  GoodInfoSt * p_item_k = one_grp.at(k);
									xmlTextWriterStartElement(pXmlPtr, (const xmlChar *)ELEMENT_NAME_PRODUCT);
										xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_P200000000013);
											fmt.Z().Cat(p_item_k->ReceiptDate.day());
											if(fmt.Len() == 1)
												fmt.PadLeft(1, '0');
											str.Z().Cat(fmt);
											fmt.Z().Cat(p_item_k->ReceiptDate.month());
											if(fmt.Len() == 1)
												fmt.PadLeft(1, '0');
											str.Dot().Cat(fmt).Dot().Cat(p_item_k->ReceiptDate.year());
											xmlTextWriterWriteString(pXmlPtr, str.ucptr());
										xmlTextWriterEndAttribute(pXmlPtr); //�20000000013
										xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_P200000000014);
											xmlTextWriterWriteString(pXmlPtr, p_item_k->TTN.ucptr());
										xmlTextWriterEndAttribute(pXmlPtr); //�20000000014
										xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_P200000000015);
											(str = p_item_k->GTD).Transf(CTRANSF_INNER_TO_OUTER); // ������-�� ����� ��� �������������� ��������� �����, � �� ���������
											xmlTextWriterWriteString(pXmlPtr, /*p_item_k->GTD*/str.ucptr());
										xmlTextWriterEndAttribute(pXmlPtr); //�20000000015
										xmlTextWriterStartAttribute(pXmlPtr, (const xmlChar *)ATRIBUTE_NAME_P200000000016);
											str.Z().Cat(p_item_k->Quantity / 10); // � ����������
											total_qtt += p_item_k->Quantity / 10;
											xmlTextWriterWriteString(pXmlPtr, str.ucptr());
										xmlTextWriterEndAttribute(pXmlPtr); //�20000000016
									xmlTextWriterEndElement(pXmlPtr); //���������
								}

							xmlTextWriterEndElement(pXmlPtr); //����������������
					}
					else
						j++;
				}
				xmlTextWriterEndElement(pXmlPtr); //������
				index++;
			}
		xmlTextWriterEndElement(pXmlPtr); //��������
	xmlTextWriterEndElement(pXmlPtr); //����
	CATCHZOK
	one_grp.freeAll();
	return ok;
}

int ExportCls::SaveContragInfo(TSCollection <ContragInfoSt> * pArr)
{
	int    ok = 1;
	SString str, fmt;
	xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_CATALOG);
	for(size_t i = 0; i < pArr->getCount(); i++) {
		const ContragInfoSt * p_item = pArr->at(i);
		xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_CONTRAGS);
			xmlTextWriterStartAttribute(P_WXmlContrag, (const xmlChar *)ATRIBUTE_NAME_IDCONTR);
				xmlTextWriterWriteString(P_WXmlContrag, str.Z().Cat(i + 1).ucptr());
			xmlTextWriterEndAttribute(P_WXmlContrag); //�������
			xmlTextWriterStartAttribute(P_WXmlContrag, (const xmlChar *)ATRIBUTE_NAME_P000000000007);
				xmlTextWriterWriteString(P_WXmlContrag, p_item->ContragName.ucptr());
			xmlTextWriterEndAttribute(P_WXmlContrag); //�000000000007
			xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_RESIDENT);
				if((p_item->Expiry != ZERODATE) && (p_item->Date != ZERODATE) && (p_item->LicOrgName) && p_item->LicSerial.NotEmpty() && p_item->LicNum) {
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_LICENSES);
						xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_LICENSE);
							xmlTextWriterStartAttribute(P_WXmlContrag, (const xmlChar *)ATRIBUTE_NAME_P000000000014);
								xmlTextWriterWriteString(P_WXmlContrag, p_item->LicOrgName.ucptr());
							xmlTextWriterEndAttribute(P_WXmlContrag); //�000000000014
							xmlTextWriterStartAttribute(P_WXmlContrag, (const xmlChar *)ATRIBUTE_NAME_P000000000013);
								fmt.Z().Cat(p_item->Expiry.day());
								if(fmt.Len() == 1)
									fmt.PadLeft(1, '0');
								str.Z().Cat(fmt);
								fmt.Z().Cat(p_item->Expiry.month());
								if(fmt.Len() == 1)
									fmt.PadLeft(1, '0');
								str.Dot().Cat(fmt).Dot().Cat(p_item->Expiry.year());
								xmlTextWriterWriteString(P_WXmlContrag, str.ucptr());
							xmlTextWriterEndAttribute(P_WXmlContrag); //�000000000013
							xmlTextWriterStartAttribute(P_WXmlContrag, (const xmlChar *)ATRIBUTE_NAME_P000000000012);
								fmt.Z().Cat(p_item->Date.day());
								if(fmt.Len() == 1)
									fmt.PadLeft(1, '0');
								str.Z().Cat(fmt);
								fmt.Z().Cat(p_item->Date.month());
								if(fmt.Len() == 1)
									fmt.PadLeft(1, '0');
								str.Dot().Cat(fmt).Dot().Cat(p_item->Date.year());
								xmlTextWriterWriteString(P_WXmlContrag, str.ucptr());
							xmlTextWriterEndAttribute(P_WXmlContrag); //�000000000012
							xmlTextWriterStartAttribute(P_WXmlContrag, (const xmlChar *)ATRIBUTE_NAME_P000000000011);
								(str = p_item->LicSerial).Comma().Cat(p_item->LicNum);
								xmlTextWriterWriteString(P_WXmlContrag, str.ucptr());
							xmlTextWriterEndAttribute(P_WXmlContrag); //�000000000011
							xmlTextWriterStartAttribute(P_WXmlContrag, (const xmlChar *)ATRIBUTE_NAME_IDLICENSE);
								xmlTextWriterWriteString(P_WXmlContrag, (const xmlChar *)"1"); // 1 - ������������, �������� � �������� �������������� ���������
																							// ������, � ��� ����� �����������������
							xmlTextWriterEndAttribute(P_WXmlContrag); //����������
						xmlTextWriterEndElement(P_WXmlContrag); //��������
					xmlTextWriterEndElement(P_WXmlContrag); //��������
				}
				xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_P000000000008);
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_COUNTRYCODE);
						xmlTextWriterWriteString(P_WXmlContrag, str.Z().Cat(p_item->CountryCode).ucptr());
					xmlTextWriterEndElement(P_WXmlContrag); //���������
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_INDEX);
						xmlTextWriterWriteString(P_WXmlContrag, str.Z().Cat(p_item->Index).ucptr());
					xmlTextWriterEndElement(P_WXmlContrag); //������
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_REGIONCODE);
						xmlTextWriterWriteString(P_WXmlContrag, str.Z().Cat(p_item->RegionCode).ucptr());
					xmlTextWriterEndElement(P_WXmlContrag); //���������
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_DISTRICT);
						xmlTextWriterWriteString(P_WXmlContrag, p_item->District.ucptr());
					xmlTextWriterEndElement(P_WXmlContrag); //�����
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_TOWN);
						xmlTextWriterWriteString(P_WXmlContrag, p_item->Town.ucptr());
					xmlTextWriterEndElement(P_WXmlContrag); //�����
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_COMMUNITY);
						xmlTextWriterWriteString(P_WXmlContrag, (const xmlChar *)"");
					xmlTextWriterEndElement(P_WXmlContrag); //����������
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_STREET);
						xmlTextWriterWriteString(P_WXmlContrag, p_item->Street.ucptr());
					xmlTextWriterEndElement(P_WXmlContrag); //�����
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_HOUSE);
						xmlTextWriterWriteString(P_WXmlContrag, str.Z().Cat(p_item->House).ucptr());
					xmlTextWriterEndElement(P_WXmlContrag); //���
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_HOUSING);
						xmlTextWriterWriteString(P_WXmlContrag, p_item->Housing.ucptr());
					xmlTextWriterEndElement(P_WXmlContrag); //������
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_LETTER);
						xmlTextWriterWriteString(P_WXmlContrag, p_item->Letter.ucptr());
					xmlTextWriterEndElement(P_WXmlContrag); //������
					xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_SQUARE);
						xmlTextWriterWriteString(P_WXmlContrag, (const xmlChar *)"");
					xmlTextWriterEndElement(P_WXmlContrag); //�����
				xmlTextWriterEndElement(P_WXmlContrag); //�000000000008
				xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_LPERSON);
					xmlTextWriterStartAttribute(P_WXmlContrag, (const xmlChar *)ATRIBUTE_NAME_P000000000009);
						xmlTextWriterWriteString(P_WXmlContrag, p_item->INN.ucptr());
					xmlTextWriterEndAttribute(P_WXmlContrag); //�000000000009
					xmlTextWriterStartAttribute(P_WXmlContrag, (const xmlChar *)ATRIBUTE_NAME_P000000000010);
						xmlTextWriterWriteString(P_WXmlContrag, p_item->KPP.ucptr());
					xmlTextWriterEndAttribute(P_WXmlContrag); //�000000000010
				xmlTextWriterEndElement(P_WXmlContrag); //��
				xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_PRODUCER);
					xmlTextWriterStartAttribute(P_WXmlContrag, (const xmlChar *)ATRIBUTE_NAME_VALUE);
						if(p_item->IsManuf)
							xmlTextWriterWriteString(P_WXmlContrag, (const xmlChar *)"True");
						else
							xmlTextWriterWriteString(P_WXmlContrag, (const xmlChar *)"False");
					xmlTextWriterEndAttribute(P_WXmlContrag); //value
				xmlTextWriterEndElement(P_WXmlContrag); //�������������
				xmlTextWriterStartElement(P_WXmlContrag, (const xmlChar *)ELEMENT_NAME_TRANSPORTER);
					xmlTextWriterStartAttribute(P_WXmlContrag, (const xmlChar *)ATRIBUTE_NAME_VALUE);
							xmlTextWriterWriteString(P_WXmlContrag, (const xmlChar *)"False");
					xmlTextWriterEndAttribute(P_WXmlContrag); //value
				xmlTextWriterEndElement(P_WXmlContrag); //����������
			xmlTextWriterEndElement(P_WXmlContrag); //��������
		xmlTextWriterEndElement(P_WXmlContrag); //�����������
	}
	xmlTextWriterEndElement(P_WXmlContrag); //�����������
	//pArr->freeAll(); // @vmiller
	return ok;
}

void ExportCls::GetPeriod(LDATE billDate, int quarterNum, int year)
{
	int month = billDate.month();
	quarterNum = fceili(fdivi(month, 3));
	year = billDate.year();
}

SString & ExportCls::DeleteExtSymbols(const char * pIn, SString & rOut)
{
	rOut.CopyFrom(pIn);
	for(size_t i = 0; i < rOut.Len();) {
		const char c = rOut.C(i);
		if(oneof5(c, '/', '\\', '.', ',', '?'))
			rOut.Excise(i, 1);
		else
			i++;
	}
	return rOut;
}

// Returns:
//		0 - ������ �� ������
//		1 - ������ ������
int GetObjTypeBySymb(const char * pSymb, uint & rType)
{
	rType = static_cast<uint>(SIntToSymbTab_GetId(Symbols, SIZEOFARRAY(Symbols), pSymb)); // @v10.4.12
	return BIN(rType != 0); // @v10.4.12
	/* @v10.4.12 for(size_t i = 0; i < SIZEOFARRAY(Symbols); i++) {
		if(strcmp(Symbols[i].P_Symb, pSymb) == 0) {
			rType = Symbols[i].Type;
			return 1;
		}
	}
	return 0;*/
}
//
// ������� ������� ��������
//
EXPORT int InitExport(void * pExpHeader, const char * pOutFileName, int * pId)
{
	int    ok = 1;
	SETIFZ(P_ExportCls, new ExportCls);
	if(P_ExportCls && !P_ExportCls->Inited) {
		if(pExpHeader)
			P_ExportCls->Header = *static_cast<const Sdr_ImpExpHeader *>(pExpHeader);
		if(!isempty(pOutFileName)) {
			P_ExportCls->PathStruct.Split(pOutFileName);
			P_ExportCls->PathStruct.Ext = "xml";
		}
		else {
			//char fname[256];
			//GetModuleFileName(NULL, fname, sizeof(fname));
			SString module_file_name;
			SSystem::SGetModuleFileName(0, module_file_name);
			P_ExportCls->PathStruct.Split(module_file_name);
			P_ExportCls->PathStruct.Dir.ReplaceStr("\\bin", "\\out", 1);
			P_ExportCls->PathStruct.Ext = "xml";
		}
		P_ExportCls->Id = 1;
		*pId = P_ExportCls->Id; // �� ������ ��������
		// �������� �� ���������� ������������ ������
		P_ExportCls->Inited = 1;
	}
	THROWERR(P_ExportCls, IEERR_IMPEXPCLSNOTINTD);
	CATCHZOK
	return ok;
}

EXPORT int SetExportObj(uint idSess, const char * pObjTypeSymb, void * pObjData, int * pObjId)
{
	int    ok = 1;
	size_t pos = 0;
	THROWERR(P_ExportCls, IEERR_IMPEXPCLSNOTINTD);
	THROWERR(idSess == P_ExportCls->Id, IEERR_NOSESS);
	THROWERR(pObjData, IEERR_NODATA);
	THROWERR(GetObjTypeBySymb(pObjTypeSymb, P_ExportCls->ObjType), IEERR_SYMBNOTFOUND);
	THROWERR(P_ExportCls->ObjType == objBill, IEERR_ONLYBILLS);
	//P_ExportCls->BeerTypeId = ((Sdr_Bill *)pObjData)->BeerGrpID; // @vmiller
	P_ExportCls->BillDate = ((Sdr_Bill *)pObjData)->Date;
	P_ExportCls->Prefix = ((Sdr_Bill *)pObjData)->XmlPrefix;
	P_ExportCls->ObjId++;
	*pObjId = P_ExportCls->ObjId;

	if(((Sdr_Bill *)pObjData)->DlvrAddrID) {
		for(pos = 0; pos < P_ExportCls->ShopInfo.getCount(); pos++) {
			if(P_ExportCls->ShopInfo.at(pos)->ShopId == ((Sdr_Bill *)pObjData)->DlvrAddrID) {
				P_ExportCls->ShopPos = pos;
				break;
			}
		}
	}
	else {
		for(pos = 0; pos < P_ExportCls->ShopInfo.getCount(); pos++) {
			const ShopInfoSt * p_item = P_ExportCls->ShopInfo.at(pos);
			if(!p_item->ContragName.CmpNC(((Sdr_Bill *)pObjData)->CntragName) && !p_item->ShopId) {
				P_ExportCls->ShopPos = pos;
				break;
			}
		}
	}
	if((pos == P_ExportCls->ShopInfo.getCount()) || (P_ExportCls->ShopInfo.getCount() == 0)) {
		ShopInfoSt * p_new_item = new ShopInfoSt;
		P_ExportCls->ShopPos = pos;
		p_new_item->ShopId = ((Sdr_Bill *)pObjData)->DlvrAddrID;
		p_new_item->ShopAddr = ((Sdr_Bill *)pObjData)->DlvrAddr;
		p_new_item->ContragName = ((Sdr_Bill *)pObjData)->CntragName;
		P_ExportCls->ShopInfo.insert(p_new_item);
	}
	CATCHZOK
	return ok;
}

EXPORT int InitExportObjIter(uint idSess, uint objId)
{
	int    ok = 1;
	THROWERR(P_ExportCls, IEERR_IMPEXPCLSNOTINTD);
	THROWERR(idSess == P_ExportCls->Id, IEERR_NOSESS);
	THROWERR(objId == P_ExportCls->ObjId, IEERR_NOOBJID);
	P_ExportCls->Itr.Init();
	CATCHZOK
	return ok;
}

EXPORT int NextExportObjIter(uint idSess, uint objId, void * pBRow)
{
	int    ok = 1;
	THROWERR(P_ExportCls, IEERR_IMPEXPCLSNOTINTD);
	THROWERR(pBRow, IEERR_NODATA);
	THROWERR(idSess == P_ExportCls->Id, IEERR_NOSESS);
	THROWERR(objId == P_ExportCls->ObjId, IEERR_NOOBJID);
	{
		ShopInfoSt * p_item = P_ExportCls->ShopInfo.at(P_ExportCls->ShopPos);
		//if(P_ExportCls->BeerTypeId != ((Sdr_BRow *)pBRow)->GoodGrpID) // @vmiller
		if(((Sdr_BRow *)pBRow)->IsAlco) { // @vmiller
			if(((Sdr_BRow *)pBRow)->Quantity < 0) { // @vmiller new
				// �������
				THROW(P_ExportCls->GetInfo((Sdr_BRow *)pBRow, &p_item->ShopAlco))
			// @vmiller new {
			}
			else {
				// �������
				THROW(P_ExportCls->GetInfo((Sdr_BRow *)pBRow, &p_item->ShopAlcoRet))
			}
			// } @vmiller new
		}
		//else if(P_ExportCls->BeerTypeId == ((Sdr_BRow *)pBRow)->GoodGrpID) // @vmiller
		else if(((Sdr_BRow *)pBRow)->IsBeer) // @vmiller
			if(((Sdr_BRow *)pBRow)->Quantity < 0) { // @vmiller new
				// �������
				THROW(P_ExportCls->GetInfo((Sdr_BRow *)pBRow, &p_item->ShopBeer));
			// @vmiller new {
			}
			else {
				// �������
				THROW(P_ExportCls->GetInfo((Sdr_BRow *)pBRow, &p_item->ShopBeerRet));
			}
			// } @vmiller new
		if(((Sdr_BRow *)pBRow)->IsAlco || ((Sdr_BRow *)pBRow)->IsBeer)
			THROW(P_ExportCls->GetContragInfo((Sdr_BRow *)pBRow, &P_ExportCls->ContragInfo)); // ���� � ������������� ����
			//THROW(P_ExportCls->GetContragInfo((Sdr_BRow *)pBRow, &p_item->ShopContrag)); // ����� ������ � ������������� ����� ���������� ������� ��������
		P_ExportCls->Itr.Next();
	}
	CATCHZOK
	return ok;
}

EXPORT int EnumExpReceipt(void * pReceipt)
{
	return -1;
}

EXPORT int FinishImpExp()
{
	int    ok = 1;
	if(P_ExportCls) {
		SString alco_name, beer_name, contrag_name;
		for(size_t pos = 0; pos < P_ExportCls->ShopInfo.getCount(); pos++) {
			ShopInfoSt * p_item = P_ExportCls->ShopInfo.at(pos);
			if(p_item->ShopAlco.getCount()) {
				P_ExportCls->CreateFileName(fAlco, p_item->ContragName, p_item->ShopAddr, pos + 1, alco_name);
				P_ExportCls->P_WXmlAlco = xmlNewTextWriterFilename(alco_name, 0);
				if(P_ExportCls->P_WXmlAlco) {
					xmlTextWriterSetIndentString(P_ExportCls->P_WXmlAlco, reinterpret_cast<const xmlChar *>("\t"));
					xmlTextWriterStartDocument(P_ExportCls->P_WXmlAlco, 0, "windows-1251", 0);
					P_ExportCls->SaveInfo(&p_item->ShopAlco, P_ExportCls->P_WXmlAlco);
					xmlTextWriterEndDocument(P_ExportCls->P_WXmlAlco);
					xmlFreeTextWriter(P_ExportCls->P_WXmlAlco);
					P_ExportCls->P_WXmlAlco = 0;
				}
			}
			if(p_item->ShopBeer.getCount()) {
				P_ExportCls->CreateFileName(fBeer, p_item->ContragName, p_item->ShopAddr, pos + 1, beer_name);
				P_ExportCls->P_WXmlBeer = xmlNewTextWriterFilename(beer_name, 0);
				if(P_ExportCls->P_WXmlBeer) {
					xmlTextWriterSetIndentString(P_ExportCls->P_WXmlBeer, reinterpret_cast<const xmlChar *>("\t"));
					xmlTextWriterStartDocument(P_ExportCls->P_WXmlBeer, 0, "windows-1251", 0);
					P_ExportCls->SaveInfo(&p_item->ShopBeer, P_ExportCls->P_WXmlBeer, 1);
					xmlTextWriterEndDocument(P_ExportCls->P_WXmlBeer);
					xmlFreeTextWriter(P_ExportCls->P_WXmlBeer);
					P_ExportCls->P_WXmlBeer = 0;
				}
			}
			// @vmiller new {
			// ��������
			if(p_item->ShopAlcoRet.getCount()) {
				P_ExportCls->CreateFileName(fAlco | fReturn, p_item->ContragName, p_item->ShopAddr, pos + 1, alco_name);
				P_ExportCls->P_WXmlAlcoRet = xmlNewTextWriterFilename(alco_name, 0);
				if(P_ExportCls->P_WXmlAlcoRet) {
					xmlTextWriterSetIndentString(P_ExportCls->P_WXmlAlcoRet, reinterpret_cast<const xmlChar *>("\t"));
					xmlTextWriterStartDocument(P_ExportCls->P_WXmlAlcoRet, 0, "windows-1251", 0);
					P_ExportCls->SaveInfo(&p_item->ShopAlcoRet, P_ExportCls->P_WXmlAlcoRet);
					xmlTextWriterEndDocument(P_ExportCls->P_WXmlAlcoRet);
					xmlFreeTextWriter(P_ExportCls->P_WXmlAlcoRet);
					P_ExportCls->P_WXmlAlcoRet = 0;
				}
			}
			if(p_item->ShopBeerRet.getCount()) {
				P_ExportCls->CreateFileName(fBeer | fReturn, p_item->ContragName, p_item->ShopAddr, pos + 1, beer_name);
				P_ExportCls->P_WXmlBeerRet = xmlNewTextWriterFilename(beer_name, 0);
				if(P_ExportCls->P_WXmlBeerRet) {
					xmlTextWriterSetIndentString(P_ExportCls->P_WXmlBeerRet, reinterpret_cast<const xmlChar *>("\t"));
					xmlTextWriterStartDocument(P_ExportCls->P_WXmlBeerRet, 0, "windows-1251", 0);
					P_ExportCls->SaveInfo(&p_item->ShopBeerRet, P_ExportCls->P_WXmlBeerRet, 1);
					xmlTextWriterEndDocument(P_ExportCls->P_WXmlBeerRet);
					xmlFreeTextWriter(P_ExportCls->P_WXmlBeerRet);
					P_ExportCls->P_WXmlBeerRet = 0;
				}
			}
			// } @vmiller new
			// �����������
			P_ExportCls->CreateFileName(fContrag, p_item->ContragName, 0, pos + 1, contrag_name);
			if(!fileExists(contrag_name)) {
				P_ExportCls->P_WXmlContrag = xmlNewTextWriterFilename(contrag_name, 0);
				if(P_ExportCls->P_WXmlContrag) {
					xmlTextWriterSetIndentString(P_ExportCls->P_WXmlContrag, reinterpret_cast<const xmlChar *>("\t"));
					xmlTextWriterStartDocument(P_ExportCls->P_WXmlContrag, 0, "windows-1251", 0);
					P_ExportCls->SaveContragInfo(&P_ExportCls->ContragInfo);
					xmlTextWriterEndDocument(P_ExportCls->P_WXmlContrag);
					xmlFreeTextWriter(P_ExportCls->P_WXmlContrag);
					P_ExportCls->P_WXmlContrag = 0;
				}
			}
			// ����� ������ � ������������� ����� ���������� ������� ��������
			/*if(p_item->ShopContrag.getCount()) {
				P_ExportCls->CreateFileName(fContrag, 0, pos + 1, contrag_name);
				P_ExportCls->P_WXmlContrag = xmlNewTextWriterFilename(contrag_name, 0);
				if(P_ExportCls->P_WXmlContrag) {
					xmlTextWriterSetIndentString(P_ExportCls->P_WXmlContrag, reinterpret_cast<const xmlChar *>("\t"));
					xmlTextWriterStartDocument(P_ExportCls->P_WXmlContrag, 0, "windows-1251", 0);
					P_ExportCls->SaveContragInfo(&p_item->ShopContrag);
					xmlTextWriterEndDocument(P_ExportCls->P_WXmlContrag);
					xmlFreeTextWriter(P_ExportCls->P_WXmlContrag);
					P_ExportCls->P_WXmlContrag = 0;
				}
			}*/
		}
		//// ���� � ������������� ���� {
		//P_ExportCls->CreateFileName(fContrag, 0, 0, pos + 1, contrag_name);
		//P_ExportCls->P_WXmlContrag = xmlNewTextWriterFilename(contrag_name, 0);
		//if(P_ExportCls->P_WXmlContrag) {
		//	xmlTextWriterSetIndentString(P_ExportCls->P_WXmlContrag, reinterpret_cast<const xmlChar *>("\t"));
		//	xmlTextWriterStartDocument(P_ExportCls->P_WXmlContrag, 0, "windows-1251", 0);
		//	P_ExportCls->SaveContragInfo(&P_ExportCls->ContragInfo);
		//	xmlTextWriterEndDocument(P_ExportCls->P_WXmlContrag);
		//	xmlFreeTextWriter(P_ExportCls->P_WXmlContrag);
		//	P_ExportCls->P_WXmlContrag = 0;
		//}
		//// }
	}
	ZDELETE(P_ExportCls);
	return ok;
}

EXPORT int GetErrorMessage(char * pMsg, uint bufLen)
{
	SString str = "";
	/* @v10.4.12 for(size_t i = 0; i < SIZEOFARRAY(ErrMsg); i++) {
		if(ErrMsg[i].Id == ErrorCode) {
			str.Cat(ErrMsg[i].P_Msg);
			break;
		}
	}*/
	SIntToSymbTab_GetSymb(ErrMsg, SIZEOFARRAY(ErrMsg), ErrorCode, str); // @v10.4.12
	if(ErrorCode == IERR_IMPFILENOTFOUND || IERR_INVMESSAGEYTYPE)
		str.Cat(StrError);
	memzero(pMsg, bufLen);
	if(str.NotEmpty() && pMsg)
		str.CopyTo(pMsg, bufLen < (str.Len() + 1) ? bufLen : (str.Len() + 1));
	ErrorCode = 0;
	StrError = "";
	return 1;
}
