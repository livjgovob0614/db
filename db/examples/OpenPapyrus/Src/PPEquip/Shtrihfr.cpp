// SHTRIHFR.CPP
// Copyright (c) V.Nasonov 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2015, 2016, 2017, 2018, 2019
// @codepage UTF-8
// Интерфейс (синхронный) с ККМ Штрих-ФР
//
#include <pp.h>
#pragma hdrstop
//
//   BillTaxArray
//
struct BillTaxEntry { // @flat
	SLAPI  BillTaxEntry() : VAT(0), SalesTax(0), Amount(0.0)
	{
	}
	long   VAT;      // prec 0.01
	long   SalesTax; // prec 0.01
	double Amount;
};

class BillTaxArray : public SVector { // @v9.8.4 SArray-->SVector
public:
	SLAPI  BillTaxArray() : SVector(sizeof(BillTaxEntry))
	{
	}
	int    SLAPI Search(long VAT, long salesTax, uint * p = 0);
	int    SLAPI Insert(const BillTaxEntry * pEntry, uint * p = 0);
	int    SLAPI Add(BillTaxEntry * pEntry);
	BillTaxEntry & FASTCALL at(uint p);
};

IMPL_CMPFUNC(BillTaxEnKey, i1, i2) 
	{ RET_CMPCASCADE2(static_cast<const BillTaxEntry *>(i1), static_cast<const BillTaxEntry *>(i2), VAT, SalesTax); }

int SLAPI BillTaxArray::Search(long VAT, long salesTax, uint * p)
{
	BillTaxEntry bte;
	bte.VAT      = VAT;
	bte.SalesTax = salesTax;
	return bsearch(&bte, p, PTR_CMPFUNC(BillTaxEnKey));
}

int SLAPI BillTaxArray::Insert(const BillTaxEntry * pEntry, uint * p)
{
	return ordInsert(pEntry, p, PTR_CMPFUNC(BillTaxEnKey)) ? 1 : PPSetErrorSLib();
}

BillTaxEntry & FASTCALL BillTaxArray::at(uint p)
{
	return *static_cast<BillTaxEntry *>(SVector::at(p));
}

int SLAPI BillTaxArray::Add(BillTaxEntry * e)
{
	int    ok = 1;
	uint   p;
	if(Search(e->VAT, e->SalesTax, &p)) {
		BillTaxEntry & bte = at(p);
		bte.Amount += e->Amount;
	}
	else {
		THROW(Insert(e));
	}
	CATCHZOK
	return ok;
}
//
//   Штрих-ФР
//
typedef ComDispInterface  FR_INTRF;

#define COM_PORT                   2
#define MAX_TIMEOUT              255   // Для Штрих-ФР max таймаут 255 мсек
#define CASH_AMOUNT_REG          241   // Денежный регистр, содержащий наличность в кассе за смену
#define CHECK_NUMBER_REG         152   // Операционный регистр, содержащий текущий номер чека
#define DEF_STRLEN                36   // Для Штрих-ФР длина строки
#define DEF_FONT_NUMBER            7   // Кол-во используемых шрифтов
#define DEF_LINEFEED_NUMBER        2   // Кол-во пропускаемых строк
#define DEF_DRAWER_NUMBER          0   // Номер денежного ящика
//
//   Таблица режимов кассы
//
#define FRCASHMODE_TBL             1   // Номер таблицы
#define FRCASHMODE_ROW             1   // Номер ряда для признаков
#define FRCASHMODE_AUTOCASHNULL    2   // Номер поля с признаком автоматического обнуления наличности
#define FRCASHMODE_OPENDRAWER      7   // Номер поля с признаком открывания денежного ящика
#define FRCASHMODE_CUTTING         8   // Номер поля с признаком отрезки чека
#define FRCASHMODE_USEWGHTSENSOR  15   // Номер поля с признаком использования весового датчика
#define FRCASHMODE_TAXALLCHK      17   // Номер поля с признаком начисления налогов на весь чек
#define FRCASHMODE_AUTOTIMING     18   // Номер поля с признаком автоматического перевода времени
#define FRCASHMODE_TAXPRINT       19   // Номер поля с признаком печати налогов
#define FRCASHMODE_SAVESTRING     22   // Номер поля с признаком сохранения строк в буфере
//
//   Таблица режимов касс Штрих-Мини-ФР-К И Штрих-Комбо-ФР-К (поля, отличные от Штрих-ФР)
//
#define COMBOCASHMODE_OPENDRAWER   6   // Номер поля с признаком открывания денежного ящика
#define COMBOCASHMODE_CUTTING      7   // Номер поля с признаком отрезки чека
#define COMBOCASHMODE_USEWGHTSENSOR 14 // Номер поля с признаком использования весового датчика
#define COMBOCASHMODE_TAXALLCHK   15   // Номер поля с признаком начисления налогов на весь чек
#define COMBOCASHMODE_AUTOTIMING  16   // Номер поля с признаком автоматического перевода времени
#define COMBOCASHMODE_TAXPRINT    17   // Номер поля с признаком печати налогов
#define COMBOCASHMODE_SAVESTRING  20   // Номер поля с признаком сохранения строк в буфере
//
//   Таблица кассиров и администраторов
//
#define FRCASHIER_TBL              2   // Номер таблицы
#define FRCASHIER_ROW              1   // Номер ряда для 1-го кассира
#define FRCASHIER_ADMINROW        30   // Номер ряда для администратора
#define FRCASHIER_FIELD_PSSW       1   // Номер поля с паролем кассира/администратора
#define FRCASHIER_FIELD_NAME       2   // Номер поля с именем кассира/администратора
//
//   Таблица перевода времени
//
#define FRTIMESWITCH_TBL           3   // Номер таблицы
#define FRTIMESWITCH_ALLOW         1   // Номер поля разрешения перевода
#define FRTIMESWITCH_DAY           2   // Номер поля числа
#define FRTIMESWITCH_MONTH         3   // Номер поля месяца
#define FRTIMESWITCH_SEASON        4   // Номер поля времени года
#define FRTIMESWITCH_MAXROW       20   // Max номер ряда
//
//   Таблица типов оплаты
//
#define FRPAYMTYPE_TBL             5   // Номер таблицы
#define FRPAYMTYPE_NAME            1   // Номер поля с наименованием оплаты
//
//   Таблица налоговых ставок
//
#define FRTAX_TBL                  6   // Номер таблицы
#define FRTAX_SALESTAX_ROW         1   // Номер ряда для налога с продаж
#define FRTAX_FIELD_TAXAMT         1   // Номер поля с величиной налога
#define FRTAX_FIELD_TAXNAME        2   // Номер поля с названием налога
//
//   Таблица формата чека
//
#define FRFORMAT_TBL               9   // Номер таблицы
#define FRFORMAT_ROW               1   // Номер ряда
#define FRFORMAT_FIELD_SIZE        3   // Номер поля с размером строки
//
//   Таблица настроек стандартного фискального подкладного документа
//
#define FRSLIPCONFIG_TBL          12   // Номер таблицы
#define FRSLIPCONFIG_ROW           1   // Номер ряда
#define FRSLIPCONFIG_TITLE_STRNUM  6   // Номер поля с номером строки заголовка
//
//   Таблица стандартных операций на подкладном документе
//
#define FRSLIPOPER_TBL            13   // Номер таблицы
#define FRSLIPOPER_ROW             1   // Номер ряда
#define FRSLIPOPER_FONTNUM         7   // Номер поля с номером шрифта строки
#define FRSLIPOPER_FIELD_SIZE     13   // Номер поля с размером строки
//
//   Режимы печати чеков
//
#define PRNMODE_NO_PRINT           0   // Нет печати
#define PRNMODE_NO_PRINT_NO_PAPER  1   // Нет печати, нет бумаги
#define PRNMODE_PRINT_NO_PAPER     2   // Печать, кончилась бумага
#define PRNMODE_AFTER_NO_PAPER     3   // Ожидание команды печати после режима 2
#define PRNMODE_PRINT              4   // Режим печати
#define PRNMODE_PRINT_LONG_REPORT  5   // Режим печати длинного отчета
//
//   Коды возврата при операциях печати
//
#define RESCODE_NO_CONNECTION     -1   // Код возврата "Нет связи: нет устройства"
#define RESCODE_NO_ERROR      0x0000
//#define RESCODE_OPEN_CHECK  0x004A   // 074 Код возврата "Открыт чек - операция невозможна"
//#define RESCODE_PRINT       0x0050   // 080 Код возврата "Идет печать предыдущей команды"
//#define RESCODE_SUBMODE_OFF 0x0072   // 114 Код возврата "Команда не поддерживается в данном подрежиме"
#define RESCODE_DVCCMDUNSUPP  0x0037   // 055 Код возврата "Команда не поддерживается в данной реализации ФР"
#define RESCODE_PAYM_LESS_SUM 0x0045   // 069 Код возврата "Сумма всех видов оплат меньше суммы чека"
#define RESCODE_MODE_OFF      0x0073   // 115 Код возврата "Команда не поддерживается в данном режиме"
#define RESCODE_SLIP_IS_EMPTY 0x00C5   // 197 Код возврата "Буфер подкладного документа пуст"
#define RESCODE_INVEKLZSTATE  163      // 163 Некорректное состояние ЭКЛЗ
//
//   Режимы ШТРИХ-ФР
//
#define FRMODE_OPEN_SESS           2   // Открытая смена
#define FRMODE_OPEN_SESS_LONG      3   // Открытый смена более 24 часов
#define FRMODE_CLOSE_SESS          4   // Закрытая смена
#define FRMODE_OPEN_CHECK          8   // Открытый чек
#define FRMODE_FULL_REPORT        11   // Печать полного фискального отчета
#define FRMODE_LONG_EKLZ_REPORT   12   // Печать длительного отчета ЭКЛЗ
#define FRMODE_PRINT_SLIPDOC      14   // Печать подкладного документа
//
//   Режимы печати подкладного документа
//
#define SLIPMODE_BEFORE_PRINT      0   // Ожидание загрузки подкладного документа
#define SLIPMODE_AFTER_PRINT       6   // Ожидание извлечения подкладного документа после печати
//
//   Модели устройств
//
#define SHTRIH_FRF                 0   // ШТРИХ-ФР-Ф
#define ELVES_FRF                  2   // ЭЛВЕС-МИНИ-ФР-Ф
#define FELIX_RF                   3   // ФЕЛИКС-Р Ф
#define SHTRIH_FRK                 4   // ШТРИХ-ФР-К
#define SHTRIH_950K                5   // ШТРИХ-950К
#define ELVES_FRK                  6   // ЭЛВЕС-ФР-К
#define SHTRIH_MINI_FRK            7   // ШТРИХ-МИНИ-ФР-К
#define SHTRIH_COMBO_FRK           9   // ШТРИХ-КОМБО-ФР-К
#define SHTRIH_POS_F              10   // ШТРИХ-POS-Ф
#define SHTRIH_950K_V2            11   // ШТРИХ-950К, версия 02
#define SHTRIH_COMBO_FRK_V2       12   // ШТРИХ-КОМБО-ФР-К, версия 02
#define SHTRIH_MINI_FRK_V2        14   // ШТРИХ-МИНИ-ФР-К, версия 02
#define SHTRIH_LIGHT_FRK         252   // ШТРИХ-LIGHT-ФР-К
//
//
//
class SCS_SHTRIHFRF : public PPSyncCashSession {
public:
	SLAPI  SCS_SHTRIHFRF(PPID n, char * name, char * port);
	SLAPI ~SCS_SHTRIHFRF();
	virtual int SLAPI PrintCheck(CCheckPacket *, uint flags);
	// @v10.0.0 virtual int SLAPI PrintCheckByBill(const PPBillPacket * pPack, double multiplier, int departN);
	virtual int SLAPI PrintCheckCopy(const CCheckPacket * pPack, const char * pFormatName, uint flags);
	virtual int SLAPI PrintSlipDoc(const CCheckPacket * pPack, const char * pFormatName, uint flags);
	virtual int SLAPI GetSummator(double * val);
	virtual int SLAPI CloseSession(PPID sessID);
	virtual int SLAPI PrintXReport(const CSessInfo *);
	virtual int SLAPI PrintZReportCopy(const CSessInfo *);
	virtual int SLAPI PrintIncasso(double sum, int isIncome);
	virtual int SLAPI GetPrintErrCode();
	virtual int SLAPI OpenBox();
	virtual int SLAPI CheckForSessionOver();
	virtual int SLAPI PrintBnkTermReport(const char * pZCheck);
private:
	// @v10.3.9 virtual int SLAPI InitChannel();
	FR_INTRF  * SLAPI InitDriver();
	int  SLAPI ConnectFR();
	int  SLAPI SetupTables();
	int  SLAPI AnnulateCheck();
	int  SLAPI CheckForCash(double sum);
	int  SLAPI CheckForEKLZOrFMOverflow();
	int  SLAPI PrintReport(int withCleaning);
	int	 SLAPI PrintDiscountInfo(const CCheckPacket * pPack, uint flags);
	int  SLAPI GetCheckInfo(const PPBillPacket * pPack, BillTaxArray * pAry, long * pFlags, SString &rName);
	int  SLAPI InitTaxTbl(BillTaxArray * pBTaxAry, PPIDArray * pVatAry, int * pPrintTaxAction);
	int  SLAPI SetFR(PPID id, int    iVal);
	int  SLAPI SetFR(PPID id, long   lVal);
	int  SLAPI SetFR(PPID id, double dVal);
	int  SLAPI SetFR(PPID id, const char * pStrVal);
	int  SLAPI GetFR(PPID id, int    * pBuf);
	int  SLAPI GetFR(PPID id, long   * pBuf);
	int  SLAPI GetFR(PPID id, double * pBuf);
	int  SLAPI GetFR(PPID id, char   * pBuf, size_t bufLen);
	int  SLAPI ExecFR(PPID id);
	int  SLAPI ExecFRPrintOper(PPID id);
	int  SLAPI AllowPrintOper(PPID id);
	void SLAPI SetErrorMessage();
	void SLAPI WriteLogFile(PPID id);
	void SLAPI CutLongTail(char * pBuf);
	void SLAPI CutLongTail(SString & rBuf);
	void SLAPI SetCheckLine(char pattern, char * pBuf);
	int  SLAPI LineFeed(int lineCount, int useReceiptRibbon, int useJournalRibbon);
	int  SLAPI ReadStringFromTbl(int tblNum, int rowNum, int fldNum, SString & rStr);
	int  SLAPI ReadValueFromTbl(int tblNum, int rowNum, int fldNum, long * pValue);
	int  SLAPI WriteStringToTbl(int tblNum, int rowNum, int fldNum, const char * pStr);
	int  SLAPI WriteValueToTbl(int tblNum, int rowNum, int fldNum, long value);
	int  SLAPI CheckForRibbonUsing(uint ribbonParam); // ribbonParam == SlipLineParam::RegTo
	int  SLAPI Cut(int withCleaning);
	int  SLAPI GetBarcodePrintMethodAndStd(int innerBarcodeStd, int * pOemMethod, int * pOemStd);

	enum {
		ResultCode,                             // #00
		ResultCodeDescription,                  // #01
		Password,                               // #02
		Beep,                                   // #03
		ComNumber,                              // #04
		BaudRate,                               // #05
		Timeout,                                // #06
		GetExchangeParam,                       // #07
		SetExchangeParam,                       // #08
		Connect,                                // #09
		Disconnect,                             // #10
		Quantity,                               // #11
		Price,                                  // #12
		Summ1,                                  // #13
		Summ2,                                  // #14
		Summ3,                                  // #15
		// Tax1..Tax4 должны идти последовательно
		Tax1,                                   // #16
		Tax2,                                   // #17
		Tax3,                                   // #18
		Tax4,                                   // #19
		StringForPrinting,                      // #20
		UseReceiptRibbon,                       // #21
		UseJournalRibbon,                       // #22
		PrintString,                            // #23
		PrintWideString,                        // #24
		StringQuantity,                         // #25
		FeedDocument,                           // #26
		DocumentName,                           // #27
		DocumentNumber,                         // #28
		PrintDocumentTitle,                     // #29
		CheckType,                              // #30
		OpenCheck,                              // #31
		Sale,                                   // #32
		ReturnSale,                             // #33
		CloseCheck,                             // #34
		CutCheck,                               // #35
		DrawerNumber,                           // #36
		OpenDrawer,                             // #37
		TableNumber,                            // #38
		RowNumber,                              // #39
		FieldNumber,                            // #40
		GetFieldStruct,                         // #41
		ReadTable,                              // #42
		WriteTable,                             // #43
		ValueOfFieldInteger,                    // #44
		ValueOfFieldString,                     // #45
		RegisterNumber,                         // #46
		GetOperationReg,                        // #47
		ContentsOfOperationRegister,            // #48
		GetCashReg,                             // #49
		ContentsOfCashRegister,                 // #50
		GetECRStatus,                           // #51
		ECRMode,                                // #52
		ECRModeDescription,                     // #53
		ECRAdvancedMode,                        // #54
		ReceiptRibbonOpticalSensor,             // #55
		JournalRibbonOpticalSensor,             // #56
		ContinuePrint,                          // #57
		CancelCheck,                            // #58
		PrintReportWithCleaning,                // #59
		PrintReportWithoutCleaning,             // #60
		UModel,                                 // #61
		UMinorProtocolVersion,                  // #62
		UMajorProtocolVersion,                  // #63
		GetDeviceMetrics,                       // #64
		CashIncome,                             // #65
		CashOutcome,                            // #66
		ClearSlipDocumentBuffer,                // #67
		FillSlipDocumentWithUnfiscalInfo,       // #68
		StringNumber,                           // #69
		PrintSlipDocument,                      // #70
		IsClearUnfiscalInfo,                    // #71
		InfoType,                               // #72
		EKLZIsPresent,                          // #73
		IsEKLZOverflow,                         // #74
		FMOverflow,                             // #75
		FreeRecordInFM,                         // #76
		IsFM24HoursOver,                        // #77
		IsDrawerOpen,                           // #78
		Department,                             // #79
		ECRModeStatus,                          // #80
		JournalRibbonIsPresent,                 // #81
		ReceiptRibbonIsPresent,                 // #82
		OutputReceipt,                          // #83 @v6.8.2
		ReceiptOutputType,                      // #84 @v6.8.2
		// @v9.1.7 PrintBarCode,                           // @v9.1.4
		// @v9.1.7 Print2DBarcode,                         // @v9.1.4
		PrintBarcodeGraph,                      // @v9.1.4
		// @v9.1.7 PrintBarcodeUsingPrinter,               // @v9.1.4
		// @v9.1.7 PrintBarcodeLine,                       // @v9.1.5
		BarcodeType,                            // @v9.1.4
		BarCode,                                // @v9.1.4
		// @v9.1.7 BarcodeDataLength,                      // @v9.1.4
		// @v9.1.7 BarWidth,                               // @v9.1.4
		// @v9.1.7 BarcodeStartBlockNumber,                // @v9.1.5
		// @v9.1.7 BarcodeParameter1,                      // @v9.1.5
		// @v9.1.7 BarcodeParameter2,                      // @v9.1.5
		// @v9.1.7 BarcodeParameter3,                      // @v9.1.5
		// @v9.1.7 BarcodeParameter4,                      // @v9.1.5
		// @v9.1.7 BarcodeParameter5,                      // @v9.1.5
		// @v9.1.7 BarcodeAlignment,                       // @v9.1.5
		FirstLineNumber,                        // @v9.1.5
		LineNumber,                             // @v9.1.5
		Summ4,                                  // @v10.6.1
		Summ5,                                  // @v10.6.1
		Summ6,                                  // @v10.6.1
		Summ7,                                  // @v10.6.1
		Summ8,                                  // @v10.6.1
		Summ9,                                  // @v10.6.1
		Summ10,                                 // @v10.6.1
		Summ11,                                 // @v10.6.1
		Summ12,                                 // @v10.6.1
		Summ13,                                 // @v10.6.1
		Summ14,                                 // @v10.6.1
		Summ15,                                 // @v10.6.1
		Summ16,                                 // @v10.6.1
		CloseCheckEx,                           // @v10.6.3
	};
	//
	// Descr: Методы вывода штрихкодов
	//
	enum {
		// @v9.1.7 bcmPrintBarcode = SCS_SHTRIHFRF::PrintBarCode,
		// @v9.1.7 bcmPrint2DBarcode = SCS_SHTRIHFRF::Print2DBarcode,
		bcmPrintBarcodeGraph = SCS_SHTRIHFRF::PrintBarcodeGraph, // !
		// @v9.1.7 bcmPrintBarcodeLine = SCS_SHTRIHFRF::PrintBarcodeLine,
		// @v9.1.7 bcmPrintBarcodeUsingPrinter = SCS_SHTRIHFRF::PrintBarcodeUsingPrinter // !
	};
	enum DeviceTypes {
		devtypeUndef,
		devtypeShtrih,
		devtypeCombo,
		devtypeMini,
		devtypeLight
	};
	enum ShtrihFlags {
		sfConnected     = 0x0001, // установлена связь с Штрих-ФР, COM-порт занят
		sfOpenCheck     = 0x0002, // чек открыт
		sfCancelled     = 0x0004, // операция печати чека прервана пользователем
		sfOldShtrih     = 0x0008, // старая версия драйвера Штрих-ФР
		sfPrintSlip     = 0x0010, // печать подкладного документа
		sfNotUseCutter  = 0x0020, // не использовать отрезчик чеков
		sfUseWghtSensor = 0x0040  // использовать весовой датчик
	};
	static FR_INTRF * P_DrvFRIntrf;
	static int  RefToIntrf;
	static uint PayTypeRegFlags;   // @v10.6.1 Флаги успешности получения интерфейсов для Summ1..Summ16
	enum {
		extmethfCloseCheckEx = 0x0001
	};
	static uint ExtMethodsFlags;   // @v10.6.3 Флаги успешности получения расширенных методов драйвера
	long   CashierPassword;    // Пароль кассира
	long   AdmPassword;        // Пароль сист.администратора
	int    ResCode;            //
	int    ErrCode;            //
	int    DeviceType;         //
	int    SCardPaymEntryN;    // @v10.6.2 PPINIPARAM_SHTRIHFRSCARDPAYMENTRY Регистр аппарата, в который заносится оплата по корпоративной карте [1..16]
	long   CheckStrLen;        //
	long   Flags;              //
	uint   RibbonParam;        //
	SString AdmName;           // Имя сист.администратора
};

FR_INTRF * SCS_SHTRIHFRF::P_DrvFRIntrf = 0; // @global
int  SCS_SHTRIHFRF::RefToIntrf = 0;         // @global
uint SCS_SHTRIHFRF::PayTypeRegFlags = 0;    // @global
uint SCS_SHTRIHFRF::ExtMethodsFlags = 0;    // @global

class CM_SHTRIHFRF : public PPCashMachine {
public:
	SLAPI CM_SHTRIHFRF(PPID cashID) : PPCashMachine(cashID)
	{
	}
	PPSyncCashSession * SLAPI SyncInterface();
};

PPSyncCashSession * SLAPI CM_SHTRIHFRF::SyncInterface()
{
	PPSyncCashSession * cs = 0;
	if(IsValid()) {
		cs = new SCS_SHTRIHFRF(NodeID, NodeRec.Name, NodeRec.Port);
		CALLPTRMEMB(cs, Init(NodeRec.Name, NodeRec.Port));
	}
	return cs;
}

REGISTER_CMT(SHTRIHFRF,1,0);

SLAPI SCS_SHTRIHFRF::SCS_SHTRIHFRF(PPID n, char * name, char * port) : PPSyncCashSession(n, name, port),
	CashierPassword(0), AdmPassword(0), ResCode(RESCODE_NO_ERROR), ErrCode(SYNCPRN_NO_ERROR),
	DeviceType(devtypeUndef), CheckStrLen(DEF_STRLEN), Flags(0), RibbonParam(0), SCardPaymEntryN(0)
{
	if(SCn.Flags & CASHF_NOTUSECHECKCUTTER)
		Flags |= sfNotUseCutter;
	RefToIntrf++;
	SETIFZ(P_DrvFRIntrf, InitDriver());
}

SLAPI SCS_SHTRIHFRF::~SCS_SHTRIHFRF()
{
	if(Flags & sfConnected)
		ExecFR(Disconnect);
	if(--RefToIntrf == 0)
		ZDELETE(P_DrvFRIntrf);
}

int SLAPI SCS_SHTRIHFRF::CheckForCash(double sum)
{
	double cash_sum = 0.0;
	if(GetSummator(&cash_sum))
		return (cash_sum < sum) ? -1 : 1;
	return 0;
}

int SLAPI SCS_SHTRIHFRF::CheckForEKLZOrFMOverflow()
{
	int    ok = 1, is_eklz_present = 0, is_eklz_overflow = 0, is_fm_overflow = 0, free_rec_in_fm = 0;
	THROW(ExecFR(GetECRStatus));
	THROW(GetFR(EKLZIsPresent, &is_eklz_present));
	if(is_eklz_present)
		THROW(GetFR(IsEKLZOverflow, &is_eklz_overflow));
	THROW(GetFR(FMOverflow, &is_fm_overflow));
	THROW(GetFR(FreeRecordInFM, &free_rec_in_fm));
	THROW_PP(!is_eklz_overflow && !is_fm_overflow && free_rec_in_fm > 2, PPERR_SYNCCASH_OVERFLOW);
	CATCHZOK
	return ok;
}

int SLAPI SCS_SHTRIHFRF::CheckForSessionOver()
{
	int    ok = -1, is_24_hours_over = 0;
	THROW(ConnectFR());
	THROW(ExecFR(GetECRStatus));
	THROW(GetFR(IsFM24HoursOver, &is_24_hours_over));
	if(is_24_hours_over)
		ok = 1;
	CATCHZOK
	return ok;
}

int  SLAPI SCS_SHTRIHFRF::CheckForRibbonUsing(uint ribbonParam)
{
	int    ok = 1;
	if(ribbonParam) {
		if((RibbonParam & SlipLineParam::fRegRegular) != (ribbonParam & SlipLineParam::fRegRegular)) {
			THROW(SetFR(UseReceiptRibbon, (ribbonParam & SlipLineParam::fRegRegular) ? TRUE : FALSE));
			SETFLAG(RibbonParam, SlipLineParam::fRegRegular, ribbonParam & SlipLineParam::fRegRegular);
		}
		if(DeviceType == devtypeShtrih && (RibbonParam & SlipLineParam::fRegJournal) != (ribbonParam & SlipLineParam::fRegJournal)) {
			THROW(SetFR(UseJournalRibbon, (ribbonParam & SlipLineParam::fRegJournal) ? TRUE : FALSE));
			SETFLAG(RibbonParam, SlipLineParam::fRegJournal, ribbonParam & SlipLineParam::fRegJournal);
		}
	}
	CATCHZOK
	return ok;
}

void SLAPI SCS_SHTRIHFRF::CutLongTail(char * pBuf)
{
	char * p = 0;
	if(pBuf && static_cast<long>(sstrlen(pBuf)) > CheckStrLen) {
		pBuf[CheckStrLen + 1] = 0;
		if((p = strrchr(pBuf, ' ')) != 0)
			*p = 0;
		else
			pBuf[CheckStrLen] = 0;
	}
}

void SLAPI SCS_SHTRIHFRF::CutLongTail(SString & rBuf)
{
	char  buf[256];
	rBuf.CopyTo(buf, sizeof(buf));
	CutLongTail(buf);
	rBuf = buf;
}

void SLAPI SCS_SHTRIHFRF::SetCheckLine(char pattern, char * pBuf)
{
	if(pBuf) {
		memset(pBuf, pattern, CheckStrLen);
		pBuf[CheckStrLen] = 0;
	}
}

// "СУММА БЕЗ СКИДКИ;КАРТА;ВЛАДЕЛЕЦ;СКИДКА;Чек не напечатан;НАЛОГ С ПРОДАЖ;СУММА ПО СТАВКЕ НДС;НСП;ПОЛУЧАТЕЛЬ;КОПИЯ ЧЕКА;ВОЗВРАТ ПРОДАЖИ;ПРОДАЖА;ИТОГ;БЕЗНАЛИЧНАЯ ОПЛАТА"

int	SLAPI SCS_SHTRIHFRF::PrintDiscountInfo(const CCheckPacket * pPack, uint flags)
{
	int    ok = 1;
	double amt = R2(fabs(MONEYTOLDBL(pPack->Rec.Amount)));
	double dscnt = R2(MONEYTOLDBL(pPack->Rec.Discount));
	if(flags & PRNCHK_RETURN)
		dscnt = -dscnt;
	if(dscnt > 0.0) {
		double  pcnt = round(dscnt * 100.0 / (amt + dscnt), 1);
		SString prn_str, temp_str;
		SCardCore scc;
		THROW(SetFR(StringForPrinting, prn_str.Z().CatCharN('-', CheckStrLen)));
		THROW(ExecFRPrintOper(PrintString));
		temp_str.Z().Cat(amt + dscnt, SFMT_MONEY);
		// @v9.7.1 prn_str = "СУММА БЕЗ СКИДКИ"; // @cstr #0
		PPLoadText(PPTXT_CCFMT_AMTWODISCOUNT, prn_str); // @v9.7.1
		prn_str.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
		prn_str.CatCharN(' ', CheckStrLen - prn_str.Len() - temp_str.Len()).Cat(temp_str);
		THROW(SetFR(StringForPrinting, prn_str));
		THROW(ExecFRPrintOper(PrintString));
		if(scc.Search(pPack->Rec.SCardID, 0) > 0) {
			// @v9.7.1 prn_str = "КАРТА"; // @cstr #1
			PPLoadText(PPTXT_CCFMT_CARD, prn_str); // @v9.7.1
			prn_str.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
			prn_str.Space().Cat(scc.data.Code);
			THROW(SetFR(StringForPrinting, prn_str));
			THROW(ExecFRPrintOper(PrintString));
			if(scc.data.PersonID && GetPersonName(scc.data.PersonID, temp_str) > 0) {
				// @v9.7.1 (prn_str = "ВЛАДЕЛЕЦ"); // @cstr #2
				PPLoadText(PPTXT_CCFMT_CARDOWNER, prn_str); // @v9.7.1
				prn_str.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
				prn_str.Space().Cat(temp_str.Transf(CTRANSF_INNER_TO_OUTER));
				CutLongTail(prn_str);
				THROW(SetFR(StringForPrinting, prn_str));
				THROW(ExecFRPrintOper(PrintString));
			}
		}
		temp_str.Z().Cat(dscnt, SFMT_MONEY);
		// @v9.7.1 (prn_str = "СКИДКА"); // @cstr #3
		PPLoadText(PPTXT_CCFMT_DISCOUNT, prn_str); // @v9.7.1
		prn_str.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
		prn_str.Space().Cat(pcnt, MKSFMTD(0, (flags & PRNCHK_ROUNDINT) ? 0 : 1, NMBF_NOTRAILZ)).CatChar('%');
		prn_str.CatCharN(' ', CheckStrLen - prn_str.Len() - temp_str.Len()).Cat(temp_str);
		THROW(SetFR(StringForPrinting, prn_str));
		THROW(ExecFRPrintOper(PrintString));
	}
	CATCHZOK
	return ok;
}

int SLAPI SCS_SHTRIHFRF::LineFeed(int lineCount, int useReceiptRibbon, int useJournalRibbon)
{
	int    ok = 1, cur_receipt, cur_journal;
	THROW(GetFR(UseReceiptRibbon, &cur_receipt));
	THROW(GetFR(UseJournalRibbon, &cur_journal));
	if(cur_receipt != useReceiptRibbon)
		THROW(SetFR(UseReceiptRibbon, useReceiptRibbon));
	if(cur_journal != useJournalRibbon)
		THROW(SetFR(UseJournalRibbon, useJournalRibbon));
	THROW(SetFR(StringQuantity, lineCount));
	THROW(ExecFRPrintOper(FeedDocument));
	if(cur_receipt != useReceiptRibbon)
		THROW(SetFR(UseReceiptRibbon, cur_receipt));
	if(cur_journal != useJournalRibbon)
		THROW(SetFR(UseJournalRibbon, cur_journal));
	CATCHZOK
	return ok;
}

static void SLAPI WriteLogFile_PageWidthOver(const char * pFormatName)
{
	SString msg_fmt, msg;
	msg.Printf(PPLoadTextS(PPTXT_SLIPFMT_WIDTHOVER, msg_fmt), pFormatName);
	PPLogMessage(PPFILNAM_SHTRIH_LOG, msg, LOGMSGF_TIME|LOGMSGF_USER);
}

int SLAPI SCS_SHTRIHFRF::Cut(int withCleaning)
{
	int    ok = 1;
	if(DeviceType == devtypeShtrih) {
		if(!(Flags & sfNotUseCutter))
			THROW(ExecFRPrintOper(CutCheck));
		if(withCleaning) {
			THROW(LineFeed(DEF_LINEFEED_NUMBER, FALSE, TRUE));
		}
	}
	else if(oneof2(DeviceType, devtypeCombo, devtypeMini) && !(Flags & sfNotUseCutter)) {
		THROW(ExecFRPrintOper(CutCheck));
	}
	CATCHZOK
	return ok;
}

int SLAPI SCS_SHTRIHFRF::GetBarcodePrintMethodAndStd(int innerBarcodeStd, int * pOemMethod, int * pOemStd)
{
	int    ok = 1;
	int    oem_std = -1;
	int    oem_method = 0;
	switch(innerBarcodeStd) {
		case BARCSTD_CODE39: oem_std = 4; break;
		case BARCSTD_UPCA: oem_std = 0; break;
		case BARCSTD_UPCE: oem_std = 1; break;
		case BARCSTD_EAN13: oem_std = 2; break;
		case BARCSTD_EAN8: oem_std = 3; break;
		case BARCSTD_ANSI: oem_std = 6; break;
		case BARCSTD_CODE93: oem_std = 7; break;
		case BARCSTD_CODE128: oem_std = 8; break;
		case BARCSTD_PDF417: oem_std = 10; break;
		case BARCSTD_QR:
			oem_method = bcmPrintBarcodeGraph;
			oem_std = 3;
			break;
		default:
			ok = 0;
	}
	ASSIGN_PTR(pOemMethod, oem_method);
	ASSIGN_PTR(pOemStd, oem_std);
	return ok;
}
//
// PrintCheck - коды возврата:
//   SYNCPRN_NO_ERROR           - OK
//   SYNCPRN_ERROR_AFTER_PRINT  - сбой после печати чека
//   SYNCPRN_ERROR              - сбой
//   SYNCPRN_ERROR_WHILE_PRINT  - сбой во время печати чека
//   SYNCPRN_CANCEL             - отмена до начала печати чека
//   SYNCPRN_CANCEL_WHILE_PRINT - отмена во время печати чека
// Если SYNCPRN_ERROR_WHILE_PRINT, SYNCPRN_CANCEL_WHILE_PRINT -
//   надо оставить флаг прерывания печати чека CASHF_LASTCHKCANCELLED в NodeRec.Flags
//
int SLAPI SCS_SHTRIHFRF::PrintCheck(CCheckPacket * pPack, uint flags)
{
	int     ok = 1;
	int     chk_no = 0;
	int     is_format = 0;
	SString temp_buf;
	SString fmt_buf;
	SString msg_buf;
	SString added_buf;
	ResCode = RESCODE_NO_ERROR;
	ErrCode = SYNCPRN_ERROR;
	THROW_INVARG(pPack);
	if(pPack->GetCount() == 0)
		ok = -1;
	else {
		SlipDocCommonParam sdc_param;
		double amt = fabs(R2(MONEYTOLDBL(pPack->Rec.Amount)));
		double sum = fabs(pPack->_Cash) + 0.001;
		double running_total = 0.0;
		// @v10.1.11 double fiscal = 0.0;
		// @v10.1.11 double nonfiscal = 0.0;
		// @v10.1.11 pPack->HasNonFiscalAmount(&fiscal, &nonfiscal);
		// @v10.1.11 {
		double real_fiscal = 0.0;
		double real_nonfiscal = 0.0;
		pPack->HasNonFiscalAmount(&real_fiscal, &real_nonfiscal);
		const double _fiscal = (_PPConst.Flags & _PPConst.fDoSeparateNonFiscalCcItems) ? real_fiscal : (real_fiscal + real_nonfiscal);
		const CcAmountList & r_al = pPack->AL_Const();
		const int is_al = BIN(r_al.getCount());
		const double amt_bnk = is_al ? r_al.Get(CCAMTTYP_BANK) : ((pPack->Rec.Flags & CCHKF_BANKING) ? _fiscal : 0.0);
		const double amt_cash = (_PPConst.Flags & _PPConst.fDoSeparateNonFiscalCcItems) ? (_fiscal - amt_bnk) : (is_al ? r_al.Get(CCAMTTYP_CASH) : (_fiscal - amt_bnk));
		const double amt_ccrd = is_al ? r_al.Get(CCAMTTYP_CRDCARD) : (real_fiscal + real_nonfiscal - _fiscal);
		// } @v10.1.11 
		THROW(ConnectFR());
		if(flags & PRNCHK_LASTCHKANNUL)
			THROW(AnnulateCheck());
		if(flags & PRNCHK_RETURN && !(flags & PRNCHK_BANKING)) {
			int    is_cash;
			THROW(is_cash = CheckForCash(amt));
			THROW_PP(is_cash > 0, PPERR_SYNCCASH_NO_CASH);
		}
		if(P_SlipFmt) {
			int      prn_total_sale = 1, r = 0;
			SString  line_buf;
			const SString format_name = "CCheck";
			SlipLineParam sl_param;
			THROW(r = P_SlipFmt->Init(format_name, &sdc_param));
			if(r > 0) {
				is_format = 1;
				if(static_cast<long>(sdc_param.PageWidth) > CheckStrLen)
					WriteLogFile_PageWidthOver(format_name);
				RibbonParam = 0;
				CheckForRibbonUsing(sdc_param.RegTo);
				if(_fiscal != 0.0) {
					THROW(SetFR(CheckType, (flags & PRNCHK_RETURN) ? 2L : 0L));
					THROW(ExecFRPrintOper(OpenCheck));
				}
				else {
					THROW(SetFR(DocumentName, "" /*sdc_param.Title*/));
					THROW(ExecFRPrintOper(PrintDocumentTitle));
				}
				Flags |= sfOpenCheck;
				for(P_SlipFmt->InitIteration(pPack); P_SlipFmt->NextIteration(line_buf, &sl_param) > 0;) {
					if(sl_param.Flags & SlipLineParam::fRegFiscal) {
						CheckForRibbonUsing(SlipLineParam::fRegRegular|SlipLineParam::fRegJournal);
						//double _q = round(sl_param.Qtty, 3);
						//double _p = round(sl_param.Price, 2);
						const double _q = sl_param.Qtty;
						const double _p = sl_param.Price;
						running_total += (_q * _p);
						THROW(SetFR(Quantity, _q));
						THROW(SetFR(Price, fabs(_p)));
						{
							(temp_buf = sl_param.Text).Strip().Transf(CTRANSF_INNER_TO_OUTER).Trim(CheckStrLen);
							THROW(SetFR(StringForPrinting, temp_buf)); // @v9.7.8 @fix ""-->temp_buf
						}
						THROW(SetFR(Department, (sl_param.DivID > 16 || sl_param.DivID < 0) ? 0 :  sl_param.DivID));
						THROW(SetFR(Tax1, 0L));
						THROW(ExecFRPrintOper((flags & PRNCHK_RETURN) ? ReturnSale : Sale));
						Flags |= sfOpenCheck;
						prn_total_sale = 0;
					}
					else if(sl_param.Kind == sl_param.lkBarcode) {
						;
					}
					else if(sl_param.Kind == sl_param.lkSignBarcode) {
						if(line_buf.NotEmptyS()) {
							CheckForRibbonUsing(SlipLineParam::fRegRegular);
							int    oem_method = 0;
							int    oem_std = 0;
							if(GetBarcodePrintMethodAndStd(sl_param.BarcodeStd, &oem_method, &oem_std) > 0) {
								/*if(oem_method == bcmPrintBarcodeUsingPrinter) {
									//Password
									//BarCode
									//LineNumber
									//BarcodeType
									//BarWidth
									//FontType
									//HRIPosition
								}
								else*/ if(oem_method == bcmPrintBarcodeGraph) {
									/*
                                    BarCode
                                    LineNumber
                                    BarcodeType
                                    BarWidth
                                    BarcodeAlignment
                                    PrintBarcodeText
                                    */
									THROW(SetFR(BarCode, line_buf));
									THROW(SetFR(BarcodeType, oem_std));
									THROW(SetFR(FirstLineNumber, 0L));
									THROW(SetFR(LineNumber, NZOR(sl_param.BarcodeHt, 260L)));
									THROW(ExecFRPrintOper(PrintBarcodeGraph));
								}
							}
						}
					}
					else {
						CheckForRibbonUsing(sl_param.Flags);
						THROW(SetFR(StringForPrinting, line_buf.Trim((sl_param.Font > 1) ? CheckStrLen / 2 : CheckStrLen)));
						THROW(ExecFRPrintOper((sl_param.Font > 1) ? PrintWideString : PrintString));
					}
				}
				running_total = fabs(running_total);
				CheckForRibbonUsing(SlipLineParam::fRegRegular|SlipLineParam::fRegJournal);
				THROW(SetFR(StringForPrinting, ""));
				if(prn_total_sale) {
					if(_fiscal != 0.0) {
						if(!pPack->GetCount()) {
							THROW(SetFR(Quantity, 1L));
							THROW(SetFR(Price, amt));
							THROW(SetFR(Tax1, 0L));
							THROW(ExecFRPrintOper((flags & PRNCHK_RETURN) ? ReturnSale : Sale));
							Flags |= sfOpenCheck;
							running_total += amt;
						}
						else /*if(fiscal != 0.0)*/ {
							THROW(SetFR(Quantity, 1L));
							THROW(SetFR(Price, _fiscal));
							THROW(SetFR(Tax1, 0L));
							THROW(ExecFRPrintOper((flags & PRNCHK_RETURN) ? ReturnSale : Sale));
							Flags |= sfOpenCheck;
							running_total += _fiscal;
						}
					}
				}
				else if(running_total != amt) {
					PPLoadText(PPTXT_SHTRIH_RUNNGTOTALGTAMT, fmt_buf);
					const char * p_sign = (running_total > amt) ? " > " : ((running_total < amt) ? " < " : " ?==? ");
					added_buf.Z().Cat(running_total, MKSFMTD(0, 12, NMBF_NOTRAILZ)).Cat(p_sign).Cat(amt, MKSFMTD(0, 12, NMBF_NOTRAILZ));
					msg_buf.Printf(fmt_buf, added_buf.cptr());
					PPLogMessage(PPFILNAM_SHTRIH_LOG, msg_buf, LOGMSGF_TIME|LOGMSGF_USER);
				}
			}
		}
		if(!is_format) {
			CCheckLineTbl::Rec ccl;
			for(uint pos = 0; pPack->EnumLines(&pos, &ccl) > 0;) {
				const int division = (ccl.DivID >= CHECK_LINE_IS_PRINTED_BIAS) ? ccl.DivID - CHECK_LINE_IS_PRINTED_BIAS : ccl.DivID;
				// Наименование товара
				GetGoodsName(ccl.GoodsID, temp_buf);
				temp_buf.Strip().Transf(CTRANSF_INNER_TO_OUTER).Trim(CheckStrLen);
				THROW(SetFR(StringForPrinting, temp_buf));
				// Цена
				THROW(SetFR(Price, R2(intmnytodbl(ccl.Price) - ccl.Dscnt)));
				// Количество
				THROW(SetFR(Quantity, R3(fabs(ccl.Quantity))));
				{
					int _dep = (division > 16 || division < 0) ? 0 : division;
					if(_dep > 0 && _dep <= 16)
						THROW(SetFR(Department, _dep));
				}
				THROW(SetFR(Tax1, 0L));
				THROW(ExecFRPrintOper((flags & PRNCHK_RETURN) ? ReturnSale : Sale));
				Flags |= sfOpenCheck;
			}
			// Информация о скидке
			if(DeviceType == devtypeShtrih)
				THROW(SetFR(UseJournalRibbon, FALSE));
			THROW(PrintDiscountInfo(pPack, flags));
			if(DeviceType == devtypeShtrih)
				THROW(SetFR(UseJournalRibbon, TRUE));
			temp_buf.Z().CatCharN('=', CheckStrLen);
			THROW(SetFR(StringForPrinting, temp_buf));
		}
		/* @v10.4.11 
		if(nonfiscal > 0.0) {
			if(_fiscal > 0.0) {
				if(flags & PRNCHK_BANKING) {
					THROW(SetFR(Summ2, _fiscal));
					THROW(SetFR(Summ1, 0L));
				}
				else {
					THROW(SetFR(Summ1, _fiscal));
					THROW(SetFR(Summ2, 0L));
				}
			}
		}
		else {
			if(running_total > sum || ((flags & PRNCHK_BANKING) && running_total != sum))
				sum = running_total;
			if(flags & PRNCHK_BANKING) {
				double  add_paym = 0.0;
				const double add_paym_epsilon = 0.01;
				const double add_paym_delta = (add_paym - sum);
				if(add_paym_delta > 0.0 || fabs(add_paym_delta) < add_paym_epsilon)
					add_paym = 0.0;
				if(add_paym) {
					THROW(SetFR(Summ1, sum - amt + add_paym));
					THROW(SetFR(Summ2, amt - add_paym));
				}
				else {
					THROW(SetFR(Summ2, sum));
					THROW(SetFR(Summ1, 0L));
				}
			}
			else {
				THROW(SetFR(Summ1, sum));
				THROW(SetFR(Summ2, 0L));
			}
		}*/
		// @v10.4.11 {
		{
			if(running_total > sum || ((flags & PRNCHK_BANKING) && running_total != sum))
				sum = running_total;
			const double __amt_bnk = amt_bnk;
			const double __amt_ccrd = amt_ccrd;
			const double __amt_cash = sum - __amt_bnk - __amt_ccrd;
			// @v10.6.2 {
			const int ccrd_entry_n = inrangeordefault(static_cast<long>(SCardPaymEntryN), 1, 16, (ExtMethodsFlags & extmethfCloseCheckEx) ? 14 : 2);
			PPID  ccrd_entry_id = 0;
			switch(ccrd_entry_n) {
				case  1: ccrd_entry_id =  Summ1; break;
				case  2: ccrd_entry_id =  Summ2; break;
				case  3: ccrd_entry_id =  Summ3; break;
				case  4: ccrd_entry_id =  Summ4; break;
				case  5: ccrd_entry_id =  Summ5; break;
				case  6: ccrd_entry_id =  Summ6; break;
				case  7: ccrd_entry_id =  Summ7; break;
				case  8: ccrd_entry_id =  Summ8; break;
				case  9: ccrd_entry_id =  Summ9; break;
				case 10: ccrd_entry_id = Summ10; break;
				case 11: ccrd_entry_id = Summ11; break;
				case 12: ccrd_entry_id = Summ12; break;
				case 13: ccrd_entry_id = Summ13; break;
				case 14: ccrd_entry_id = Summ14; break;
				case 15: ccrd_entry_id = Summ15; break;
				case 16: ccrd_entry_id = Summ16; break;
			}
			if(PayTypeRegFlags & (1U << ccrd_entry_n) && !ccrd_entry_id) {
				ccrd_entry_id = Summ2;
			}
			// } @v10.6.2 
			{
				msg_buf.Z().Cat("Payment").CatDiv(':', 2).Cat("PayTypeRegFlags-hex").CatChar('=').CatHex(static_cast<ulong>(PayTypeRegFlags));
				msg_buf.Space().Cat("PayTypeRegFlags-dec").CatChar('=').Cat(PayTypeRegFlags);
				msg_buf.Space().CatEq("sum", sum, MKSFMTD(0, 6, 0)).Space().CatEq("amt_cash", __amt_cash, MKSFMTD(0, 6, 0)).Space().
					CatEq("amt_bnk", __amt_bnk, MKSFMTD(0, 6, 0)).Space().CatEq("amt_ccrd", __amt_ccrd, MKSFMTD(0, 6, 0));
				PPLogMessage(PPFILNAM_SHTRIH_LOG, msg_buf, LOGMSGF_TIME|LOGMSGF_USER|LOGMSGF_DBINFO);
			}
			// 1  - наличная оплата
			// 2  - безналичная оплата
			// 14 - предварительная оплата
			// 15 - последующая оплата
			// @v10.6.0 if(amt_bnk != 0.0)
			THROW(SetFR(Summ1, (ccrd_entry_id == Summ1) ? (__amt_cash + __amt_ccrd) : __amt_cash));
			THROW(SetFR(Summ2, (ccrd_entry_id == Summ2) ? (__amt_bnk  + __amt_ccrd) : __amt_bnk));
			if(!oneof3(ccrd_entry_id, Summ1, Summ2, 0)) {
				THROW(SetFR(ccrd_entry_id, __amt_ccrd));
			}
		}
		// } @v10.4.11 
		if(_fiscal != 0.0) {
			if(ExtMethodsFlags & extmethfCloseCheckEx) { // @v10.6.3
				THROW(ExecFRPrintOper(CloseCheckEx));
			}
			else {
				THROW(ExecFRPrintOper(CloseCheck));
			}
		}
		else {
			THROW(SetFR(ReceiptOutputType, 0));
			THROW(ExecFRPrintOper(OutputReceipt));
		}
		Flags &= ~sfOpenCheck;
		ErrCode = SYNCPRN_ERROR_AFTER_PRINT;
		THROW(Cut(1));
		THROW(SetFR(RegisterNumber, CHECK_NUMBER_REG));
		THROW(ExecFR(GetOperationReg));
		THROW(GetFR(ContentsOfOperationRegister, &chk_no));
		pPack->Rec.Code = chk_no;
		ErrCode = SYNCPRN_NO_ERROR;
	}
	CATCH
		if(Flags & sfCancelled) {
			Flags &= ~sfCancelled;
			if(ErrCode != SYNCPRN_ERROR_AFTER_PRINT) {
				SString no_print_txt;
				PPLoadText(PPTXT_CHECK_NOT_PRINTED, no_print_txt);
				ErrCode = (Flags & sfOpenCheck) ? SYNCPRN_CANCEL_WHILE_PRINT : SYNCPRN_CANCEL;
				PPLogMessage(PPFILNAM_SHTRIH_LOG, CCheckCore::MakeCodeString(&pPack->Rec, no_print_txt), LOGMSGF_TIME|LOGMSGF_USER);
				ok = 0;
			}
		}
		else {
			SetErrorMessage();
			ExecFR(Beep);
			if(Flags & sfOpenCheck)
				ErrCode = SYNCPRN_ERROR_WHILE_PRINT;
			ok = 0;
		}
	ENDCATCH
	return ok;
}

int SLAPI SCS_SHTRIHFRF::OpenBox()
{
	int     ok = -1, is_drawer_open = 0;
	ResCode = RESCODE_NO_ERROR;
	ErrCode = SYNCPRN_ERROR;
	THROW(ConnectFR());
	THROW(ExecFR(GetECRStatus));
	THROW(GetFR(IsDrawerOpen, &is_drawer_open));
	if(!is_drawer_open) {
		THROW(SetFR(DrawerNumber, DEF_DRAWER_NUMBER));
		THROW(ExecFR(OpenDrawer));
		ok = 1;
	}
	CATCH
		if(Flags & sfCancelled) {
			Flags &= ~sfCancelled;
			if(ErrCode != SYNCPRN_ERROR_AFTER_PRINT) {
				ErrCode = (Flags & sfOpenCheck) ? SYNCPRN_CANCEL_WHILE_PRINT : SYNCPRN_CANCEL;
				ok = 0;
			}
		}
		else {
			SetErrorMessage();
			ExecFR(Beep);
			if(Flags & sfOpenCheck)
				ErrCode = SYNCPRN_ERROR_WHILE_PRINT;
			ok = 0;
		}
	ENDCATCH
	return ok;
}

int SLAPI SCS_SHTRIHFRF::GetCheckInfo(const PPBillPacket * pPack, BillTaxArray * pAry, long * pFlags, SString &rName)
{
	int    ok = 1, wovatax = 0;
	long   flags = 0;
	BillTaxEntry  bt_entry;
	PPID   main_org_id;
	if(GetMainOrgID(&main_org_id)) {
		PersonTbl::Rec prec;
		if(SearchObject(PPOBJ_PERSON, main_org_id, &prec) > 0 && prec.Flags & PSNF_NOVATAX)
			wovatax = 1;
	}
	THROW_INVARG(pPack && pAry);
	RVALUEPTR(flags, pFlags);
	if(pPack->OpTypeID == PPOPT_ACCTURN) {
		long   s_tax = 0;
		double amt1 = 0.0, amt2 = 0.0;
		PPObjAmountType amtt_obj;
		TaxAmountIDs    tais;
		double sum = BR2(pPack->Rec.Amount);
		if(sum < 0.0)
			flags |= PRNCHK_RETURN;
		sum = fabs(sum);
		amtt_obj.GetTaxAmountIDs(&tais, 1);
		if(tais.STaxAmtID)
			s_tax = tais.STaxRate;
		if(tais.VatAmtID[0])
			amt1 = fabs(pPack->Amounts.Get(tais.VatAmtID[0], 0L));
		if(tais.VatAmtID[1])
			amt2 = fabs(pPack->Amounts.Get(tais.VatAmtID[1], 0L));
		bt_entry.VAT = (amt1 || amt2) ? ((amt1 > amt2) ? tais.VatRate[0] : tais.VatRate[1]) : 0;
		bt_entry.SalesTax = s_tax;
		bt_entry.Amount   = sum;
		THROW(pAry->Add(&bt_entry));
	}
	else {
		PPTransferItem * ti;
		PPObjGoods  goods_obj;
		if(pPack->OpTypeID == PPOPT_GOODSRETURN) {
			PPOprKind op_rec;
			if(GetOpData(pPack->Rec.OpID, &op_rec) > 0 && IsExpendOp(op_rec.LinkOpID) > 0)
				flags |= PRNCHK_RETURN;
		}
		else if(pPack->OpTypeID == PPOPT_GOODSRECEIPT)
			flags |= PRNCHK_RETURN;
		for(uint i = 0; pPack->EnumTItems(&i, &ti);) {
			int re;
			PPGoodsTaxEntry  gt_entry;
			THROW(goods_obj.FetchTax(ti->GoodsID, pPack->Rec.Dt, pPack->Rec.OpID, &gt_entry));
			bt_entry.VAT = wovatax ? 0 : gt_entry.VAT;
			re = (ti->Flags & PPTFR_RMVEXCISE) ? 1 : 0;
			bt_entry.SalesTax = ((CConfig.Flags & CCFLG_PRICEWOEXCISE) ? !re : re) ? 0 : gt_entry.SalesTax;
			bt_entry.Amount   = ti->CalcAmount();
			THROW(pAry->Add(&bt_entry));
		}
	}
	if(pPack->Rec.Object)
		GetArticleName(pPack->Rec.Object, rName);
	CATCHZOK
	ASSIGN_PTR(pFlags, flags);
	return ok;
}

int SLAPI SCS_SHTRIHFRF::InitTaxTbl(BillTaxArray * pBTaxAry, PPIDArray * pVatAry, int * pPrintTaxAction)
{
	int    ok = 1, print_tax_action = 0;
	uint   pos;
	long   cshr_pssw = CashierPassword;
	long   s_tax = 0;
	pVatAry->freeAll();
	for(pos = 0; pos < pBTaxAry->getCount(); pos++) {
		BillTaxEntry & bte = pBTaxAry->at(pos);
		if(bte.SalesTax)
			s_tax = bte.SalesTax;
		if(bte.VAT && !pVatAry->bsearch(bte.VAT))
			THROW_SL(pVatAry->ordInsert(bte.VAT, 0));
	}
	// Выбор режима печати налогов
	CashierPassword = AdmPassword;
	THROW(SetFR(TableNumber, FRCASHMODE_TBL));
	THROW(SetFR(RowNumber,   FRCASHMODE_ROW));
	THROW(SetFR(FieldNumber, oneof2(DeviceType, devtypeCombo, devtypeMini) ? COMBOCASHMODE_TAXALLCHK : FRCASHMODE_TAXALLCHK));
	THROW(ExecFR(GetFieldStruct));
	THROW(ExecFR(ReadTable));
	THROW(GetFR(ValueOfFieldInteger, &print_tax_action));
	// Начисление налогов на весь чек мы не используем,
	// а для Штрих-Комбо  и Штрих_Мини нельзя настраивать таблицу налоговых ставок при открытой смене
	if(print_tax_action || DeviceType == devtypeCombo || DeviceType == devtypeMini)
		print_tax_action = 0;
	else {
		THROW(SetFR(TableNumber, FRCASHMODE_TBL));
		THROW(SetFR(RowNumber,   FRCASHMODE_ROW));
		THROW(SetFR(FieldNumber, FRCASHMODE_TAXPRINT));
		THROW(ExecFR(GetFieldStruct));
		THROW(ExecFR(ReadTable));
		THROW(GetFR(ValueOfFieldInteger, &print_tax_action));
	}
	if(print_tax_action) {
		SString temp_buf;
		SString vat_str;
		// Настройка таблицы налоговых ставок
		// Налог с продаж
		THROW(SetFR(TableNumber, FRTAX_TBL));
		if(s_tax) {
			THROW(SetFR(RowNumber,   FRTAX_SALESTAX_ROW));
			THROW(SetFR(FieldNumber, FRTAX_FIELD_TAXAMT));
			THROW(ExecFR(GetFieldStruct));
			THROW(SetFR(ValueOfFieldInteger, s_tax));
			THROW(ExecFR(WriteTable));
			THROW(SetFR(FieldNumber, FRTAX_FIELD_TAXNAME));
			THROW(ExecFR(GetFieldStruct));
			{
				// @v9.7.1 temp_buf = "НАЛОГ С ПРОДАЖ"; // @cstr #5
				PPLoadText(PPTXT_CCFMT_STAX, temp_buf); // @v9.7.1
				temp_buf.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
				THROW(SetFR(ValueOfFieldString, temp_buf));
			}
			THROW(ExecFR(WriteTable));
		}
		// Ставки НДС
		for(pos = 0; pos < pVatAry->getCount(); pos++) {
			THROW(SetFR(RowNumber, (long)(FRTAX_SALESTAX_ROW + pos + 1)));
			THROW(SetFR(FieldNumber, FRTAX_FIELD_TAXAMT));
			THROW(ExecFR(GetFieldStruct));
			THROW(SetFR(ValueOfFieldInteger, pVatAry->at(pos)));
			THROW(ExecFR(WriteTable));
			THROW(SetFR(FieldNumber, FRTAX_FIELD_TAXNAME));
			THROW(ExecFR(GetFieldStruct));
			{
				// @v9.0.2 {
				PPLoadString("vat", temp_buf);
				temp_buf.Transf(CTRANSF_INNER_TO_OUTER).Space();
				// } @v9.0.2
				// @v9.0.2 PPGetWord(PPWORD_VAT, 1, temp_buf).Space();
				(vat_str = temp_buf).Cat(fdiv100i(pVatAry->at(pos)), MKSFMTD(0, 2, NMBF_NOTRAILZ)).CatChar('%');
				THROW(SetFR(ValueOfFieldString, vat_str));
			}
			THROW(ExecFR(WriteTable));
		}
	}
	CATCHZOK
	CashierPassword = cshr_pssw;
	ASSIGN_PTR(pPrintTaxAction, print_tax_action);
	return ok;
}

#if 0 // @v10.0.0 {
int SLAPI SCS_SHTRIHFRF::PrintCheckByBill(const PPBillPacket * pPack, double multiplier, int departN) // @removed
{
	int     ok = 1, print_tax = 0;
	uint    pos;
	long    flags = 0;
	double  price, sum = 0.0;
	SString prn_str, name;
	SString temp_buf;
	BillTaxArray  bt_ary;
	PPIDArray     vat_ary;
	ResCode = RESCODE_NO_ERROR;
	ErrCode = SYNCPRN_ERROR;
	THROW_INVARG(pPack);
	THROW(GetCheckInfo(pPack, &bt_ary, &flags, name));
	if(bt_ary.getCount() == 0)
		return -1;
	THROW(ConnectFR());
	THROW(AnnulateCheck());
	// @v5.6.0 THROW(CheckForEKLZOrFMOverflow());
	if(multiplier < 0)
		flags |= PRNCHK_RETURN;
	if(flags & PRNCHK_RETURN) {
		int    is_cash;
		THROW(is_cash = CheckForCash(fabs(BR2(pPack->Rec.Amount) * multiplier)));
		THROW_PP(is_cash > 0, PPERR_SYNCCASH_NO_CASH);
	}
	THROW(InitTaxTbl(&bt_ary, &vat_ary, &print_tax));
	for(pos = 0; pos < bt_ary.getCount(); pos++) {
		int  tax_no = Tax1;
		uint vat_pos;
		BillTaxEntry & bte = bt_ary.at(pos);
		// Цена
		price = R2(fabs(bte.Amount * multiplier));
		sum += price;
		THROW(SetFR(Price, price));
		// Количество
		THROW(SetFR(Quantity, 1L));
		// @v9.5.7 {
		if(departN > 0 && departN <= 16) {
			THROW(SetFR(Department, departN));
		}
		// } @v9.5.7
		// Налоги
		if(print_tax) {
			if(bte.SalesTax) {
				THROW(SetFR(tax_no++, 1L));
			}
			if(bte.VAT && vat_ary.bsearch(bte.VAT, &vat_pos)) {
				THROW(SetFR(tax_no++, (long)(vat_pos + 2)));
			}
		}
		THROW(SetFR(tax_no, 0L));
		// @v9.7.1 prn_str = "СУММА ПО СТАВКЕ НДС"; // @cstr #6
		PPLoadText(PPTXT_CCFMT_AMTBYVATRATE, prn_str); // @v9.7.1
		prn_str.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
		prn_str.Space().Cat(fdiv100i(bte.VAT), MKSFMTD(0, 2, NMBF_NOTRAILZ)).CatChar('%');
		if(bte.SalesTax) {
			// @v9.7.1 temp_buf = "НСП"; // @cstr #7
			PPLoadText(PPTXT_CCFMT_STAX_S, temp_buf); // @v9.7.1
			temp_buf.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
			prn_str.Space().Cat(temp_buf).Space().Cat(fdiv100i(bte.SalesTax), MKSFMTD(0, 2, NMBF_NOTRAILZ)).CatChar('%');
		}
		THROW(SetFR(StringForPrinting, prn_str));
		THROW(ExecFRPrintOper((flags & PRNCHK_RETURN) ? ReturnSale : Sale));
		Flags |= sfOpenCheck;
	}
	if(name.NotEmptyS()) {
		// @v9.7.1 prn_str = "ПОЛУЧАТЕЛЬ"; // @cstr #8
		PPLoadText(PPTXT_CCFMT_RECEIVER, prn_str); // @v9.7.1
		prn_str.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
		prn_str.Space().Cat(name.Transf(CTRANSF_INNER_TO_OUTER));
		CutLongTail(prn_str);
		THROW(SetFR(StringForPrinting, prn_str));
		THROW(ExecFRPrintOper(PrintString));
	}
	THROW(SetFR(Summ1, sum));
	THROW(SetFR(Summ2, 0L));
	THROW(SetFR(Summ3, 0L));
	THROW(SetFR(Tax1,  0L));
	THROW(SetFR(StringForPrinting, prn_str.Z().CatCharN('=', CheckStrLen)));
	THROW(ExecFRPrintOper(CloseCheck));
	Flags &= ~sfOpenCheck;
	ErrCode = SYNCPRN_ERROR_AFTER_PRINT;
	THROW(Cut(1));
	ErrCode = SYNCPRN_NO_ERROR;
	CATCH
		if(Flags & sfCancelled) {
			Flags &= ~sfCancelled;
			if(ErrCode != SYNCPRN_ERROR_AFTER_PRINT) {
				ErrCode = (Flags & sfOpenCheck) ? SYNCPRN_CANCEL_WHILE_PRINT : SYNCPRN_CANCEL;
				ok = 0;
			}
		}
		else {
			SetErrorMessage();
			ExecFR(Beep);
			if(Flags & sfOpenCheck)
				ErrCode = SYNCPRN_ERROR_WHILE_PRINT;
			ok = 0;
		}
	ENDCATCH
	return ok;
}
#endif // } 0 @v10.0.0

int SLAPI SCS_SHTRIHFRF::PrintSlipDoc(const CCheckPacket * pPack, const char * pFormatName, uint flags)
{
	int    ok = -1;
	SString  temp_buf;
	ResCode = RESCODE_NO_ERROR;
	ErrCode = SYNCPRN_ERROR_AFTER_PRINT;
	THROW_INVARG(pPack);
	THROW(ConnectFR());
	if(CConfig.Flags & CCFLG_DEBUG) {
		(temp_buf = "SCS_SHTRIHFRF::PrintSlipDoc entry").CatDiv(':', 2).CatEq("DeviceType", static_cast<long>(devtypeCombo)).
			CatDiv(';', 2).CatEq("SlipFormat", pFormatName);
		PPLogMessage(PPFILNAM_SHTRIH_LOG, temp_buf, LOGMSGF_TIME|LOGMSGF_USER);
	}
	if(DeviceType == devtypeCombo && P_SlipFmt) {
		int   r = 1;
		SString   line_buf, format_name = (pFormatName && pFormatName[0]) ? pFormatName : "SlipDocument";
		StringSet head_lines(reinterpret_cast<const char *>(&r));
		SlipDocCommonParam  sdc_param;
		THROW(r = P_SlipFmt->Init(format_name, &sdc_param));
		if(r > 0) {
			int   str_num, print_head_lines = 0, fill_head_lines = 1;
			SlipLineParam sl_param;
			Flags |= sfPrintSlip;
			THROW(ExecFRPrintOper(ClearSlipDocumentBuffer));
			for(str_num = 0, P_SlipFmt->InitIteration(pPack); P_SlipFmt->NextIteration(line_buf, &sl_param) > 0;) {
				if(sl_param.Font > 1 && sl_param.Font <= DEF_FONT_NUMBER)
					line_buf.PadLeft(1, sl_param.Font).PadLeft(1, 27);
				if(print_head_lines) {
					for(uint i = 0; head_lines.get(&i, temp_buf);) {
						THROW(SetFR(StringForPrinting, temp_buf));
						THROW(SetFR(StringNumber, ++str_num));
						THROW(ExecFRPrintOper(FillSlipDocumentWithUnfiscalInfo));
					}
					print_head_lines = 0;
				}
				else if(fill_head_lines)
					if(str_num < (int)sdc_param.HeadLines)
						head_lines.add(line_buf);
					else
						fill_head_lines = 0;
				THROW(SetFR(StringForPrinting, line_buf));
				THROW(SetFR(StringNumber, ++str_num));
				THROW(ExecFRPrintOper(FillSlipDocumentWithUnfiscalInfo));
				if(str_num == sdc_param.PageLength) {
					THROW(SetFR(IsClearUnfiscalInfo, FALSE));
					THROW(SetFR(InfoType, 0L));
					THROW(ExecFRPrintOper(PrintSlipDocument));
					print_head_lines = 1;
					str_num = 0;
				}
			}
			if(str_num) {
				THROW(SetFR(IsClearUnfiscalInfo, FALSE));
				THROW(SetFR(InfoType, 0L));
				THROW(ExecFRPrintOper(PrintSlipDocument));
			}
			ok = 1;
		}
	}
	else {
		if(CConfig.Flags & CCFLG_DEBUG) {
			(temp_buf = "SCS_SHTRIHFRF::PrintSlipDoc printing skiped");
			PPLogMessage(PPFILNAM_SHTRIH_LOG, temp_buf, LOGMSGF_TIME|LOGMSGF_USER);
		}
	}
	ErrCode = SYNCPRN_NO_ERROR;
	CATCH
		if(Flags & sfCancelled) {
			Flags &= ~sfCancelled;
			ok = -1;
		}
		else {
			SetErrorMessage();
			ok = (ExecFR(Beep), 0);
		}
	ENDCATCH
	Flags &= ~sfPrintSlip;
	return ok;
}

int SLAPI SCS_SHTRIHFRF::PrintCheckCopy(const CCheckPacket * pPack, const char * pFormatName, uint flags)
{
	int     ok = 1, is_format = 0;
	SlipDocCommonParam  sdc_param;
	ResCode = RESCODE_NO_ERROR;
	ErrCode = SYNCPRN_ERROR_AFTER_PRINT;
	THROW_INVARG(pPack);
	THROW(ConnectFR());
	THROW(SetFR(DocumentNumber, pPack->Rec.Code));
	if(P_SlipFmt) {
		int   r = 0;
		SString  line_buf, format_name = (pFormatName && pFormatName[0]) ? pFormatName : ((flags & PRNCHK_RETURN) ? "CCheckRetCopy" : "CCheckCopy");
		SlipLineParam sl_param;
		THROW(r = P_SlipFmt->Init(format_name, &sdc_param));
		if(r > 0) {
			is_format = 1;
			if(static_cast<long>(sdc_param.PageWidth) > CheckStrLen)
				WriteLogFile_PageWidthOver(format_name);
			RibbonParam = 0;
			CheckForRibbonUsing(sdc_param.RegTo);
			THROW(SetFR(DocumentName, sdc_param.Title));
			THROW(ExecFRPrintOper(PrintDocumentTitle));
			for(P_SlipFmt->InitIteration(pPack); P_SlipFmt->NextIteration(line_buf, &sl_param) > 0;) {
				CheckForRibbonUsing(sl_param.Flags);
				THROW(SetFR(StringForPrinting, line_buf.Trim((sl_param.Font > 1) ? CheckStrLen / 2 : CheckStrLen)));
				THROW(ExecFRPrintOper((sl_param.Font > 1) ? PrintWideString : PrintString));
			}
		}
	}
	if(!is_format) {
		uint    pos;
		SString prn_str;
		SString temp_buf;
		CCheckLineTbl::Rec ccl;
		if(DeviceType == devtypeShtrih)
			THROW(SetFR(UseJournalRibbon, FALSE));
		{
			// @v9.7.1 temp_buf = "КОПИЯ ЧЕКА"; // @cstr #9
			PPLoadText(PPTXT_CCFMT_CHKCOPY, temp_buf); // @v9.7.1
			temp_buf.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
			THROW(SetFR(DocumentName, temp_buf));
		}
		THROW(ExecFRPrintOper(PrintDocumentTitle));
		{
			// @v9.7.1 temp_buf = (flags & PRNCHK_RETURN) ? "ВОЗВРАТ ПРОДАЖИ" : "ПРОДАЖА"; // @cstr #10 #11
			PPLoadText((flags & PRNCHK_RETURN) ? PPTXT_CCFMT_RETURN : PPTXT_CCFMT_SALE, temp_buf); // @v9.7.1
			temp_buf.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
			THROW(SetFR(StringForPrinting, temp_buf));
		}
		THROW(ExecFRPrintOper(PrintString));
		for(pos = 0; pPack->EnumLines(&pos, &ccl) > 0;) {
			double  price = intmnytodbl(ccl.Price) - ccl.Dscnt;
			double  qtty  = R3(fabs(ccl.Quantity));
			GetGoodsName(ccl.GoodsID, prn_str);
			CutLongTail(prn_str.Transf(CTRANSF_INNER_TO_OUTER));
			THROW(SetFR(StringForPrinting, prn_str));
			THROW(ExecFRPrintOper(PrintString));
			if(qtty != 1.0) {
				temp_buf.Z().Cat(qtty, MKSFMTD(0, 3, NMBF_NOTRAILZ)).CatDiv('X', 1).Cat(price, SFMT_MONEY);
				THROW(SetFR(StringForPrinting, prn_str.Z().CatCharN(' ', CheckStrLen - temp_buf.Len()).Cat(temp_buf)));
				THROW(ExecFRPrintOper(PrintString));
			}
			temp_buf.Z().CatEq(0, qtty * price, SFMT_MONEY);
			THROW(SetFR(StringForPrinting, prn_str.Z().CatCharN(' ', CheckStrLen - temp_buf.Len()).Cat(temp_buf)));
			THROW(ExecFRPrintOper(PrintString));
		}
		THROW(PrintDiscountInfo(pPack, flags));
		THROW(SetFR(StringForPrinting, prn_str.Z().CatCharN('=', CheckStrLen)));
		THROW(ExecFRPrintOper(PrintString));
		temp_buf.Z().CatEq(0, fabs(MONEYTOLDBL(pPack->Rec.Amount)), SFMT_MONEY);
		// @v9.7.1 prn_str = "ИТОГ"; // @cstr #12
		PPLoadText(PPTXT_CCFMT_TOTAL, prn_str); // @v9.7.1
		prn_str.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
		prn_str.CatCharN(' ', CheckStrLen / 2 - prn_str.Len() - temp_buf.Len()).Cat(temp_buf);
		THROW(SetFR(StringForPrinting, prn_str));
		THROW(ExecFRPrintOper(PrintWideString));
	}
	THROW(LineFeed(6, TRUE, FALSE));
	THROW(Cut(1));
	ErrCode = SYNCPRN_NO_ERROR;
	CATCH
		if(Flags & sfCancelled) {
			Flags &= ~sfCancelled;
			ok = -1;
		}
		else {
			SetErrorMessage();
			ok = (ExecFR(Beep), 0);
		}
	ENDCATCH
	return ok;
}

int SLAPI SCS_SHTRIHFRF::PrintReport(int withCleaning)
{
	int    ok = 1, mode = 0;
	long   cshr_pssw = 0;
	ResCode = RESCODE_NO_ERROR;
	THROW(ConnectFR());
	// Закрыть сессию можно только под паролем администратора
	cshr_pssw = CashierPassword;
	CashierPassword = AdmPassword;
	//
	Flags |= sfOpenCheck;
	THROW(ExecFR(GetECRStatus));
	THROW(GetFR(ECRMode, &mode));
	if(withCleaning) {
		THROW_PP(mode != FRMODE_CLOSE_SESS, PPERR_SYNCCASH_DAYCLOSED);
		THROW(ExecFRPrintOper(PrintReportWithCleaning));
	}
	else {
		THROW(ExecFRPrintOper(PrintReportWithoutCleaning));
	}
	THROW(Cut(withCleaning));
	CATCH
		if(Flags & sfCancelled) {
			Flags &= ~sfCancelled;
			ok = -1;
		}
		else {
			SetErrorMessage();
			ok = (ExecFR(Beep), 0);
		}
	ENDCATCH
	if(Flags & sfOpenCheck) {
		Flags &= ~sfOpenCheck;
		CashierPassword = cshr_pssw;
	}
	return ok;
}

int SLAPI SCS_SHTRIHFRF::CloseSession(PPID sessID)
{
	return PrintReport(1);
}

int SLAPI SCS_SHTRIHFRF::PrintXReport(const CSessInfo *)
{
	return PrintReport(0);
}

int SLAPI SCS_SHTRIHFRF::PrintZReportCopy(const CSessInfo * pInfo)
{
	int  ok = -1;
	ResCode = RESCODE_NO_ERROR;
	ErrCode = SYNCPRN_ERROR_AFTER_PRINT;
	THROW_INVARG(pInfo);
	THROW(ConnectFR());
	THROW(SetFR(DocumentNumber, pInfo->Rec.SessNumber));
	if(P_SlipFmt) {
		int   r = 0;
		SString  line_buf, format_name = "ZReportCopy";
		SlipDocCommonParam  sdc_param;
		THROW(r = P_SlipFmt->Init(format_name, &sdc_param));
		if(r > 0) {
			SlipLineParam sl_param;
			if(sdc_param.PageWidth > (uint)CheckStrLen)
				WriteLogFile_PageWidthOver(format_name);
			RibbonParam = 0;
			CheckForRibbonUsing(sdc_param.RegTo);
			THROW(SetFR(DocumentName, sdc_param.Title));
			THROW(ExecFRPrintOper(PrintDocumentTitle));
			for(P_SlipFmt->InitIteration(pInfo); P_SlipFmt->NextIteration(line_buf, &sl_param) > 0;) {
				CheckForRibbonUsing(sl_param.Flags);
				THROW(SetFR(StringForPrinting, line_buf.Trim((sl_param.Font > 1) ? CheckStrLen / 2 : CheckStrLen)));
				THROW(ExecFRPrintOper((sl_param.Font > 1) ? PrintWideString : PrintString));
			}
			THROW(LineFeed(6, TRUE, FALSE));
			THROW(Cut(1));
		}
	}
	ErrCode = SYNCPRN_NO_ERROR;
	CATCH
		if(Flags & sfCancelled) {
			Flags &= ~sfCancelled;
			ok = -1;
		}
		else {
			SetErrorMessage();
			ok = (ExecFR(Beep), 0);
		}
	ENDCATCH
	return ok;
}

int SLAPI SCS_SHTRIHFRF::PrintIncasso(double sum, int isIncome)
{
	int    ok = 1;
	ResCode = RESCODE_NO_ERROR;
	THROW(ConnectFR());
	Flags |= sfOpenCheck;
	THROW(SetFR(Summ1, sum));
	if(isIncome) {
		THROW(LineFeed(6, TRUE, FALSE));
		THROW(ExecFRPrintOper(CashIncome));
	}
	else {
		int    is_cash;
		THROW(is_cash = CheckForCash(sum));
		THROW_PP(is_cash > 0, PPERR_SYNCCASH_NO_CASH);
		THROW(LineFeed(6, TRUE, FALSE));
		THROW(ExecFRPrintOper(CashOutcome));
	}
	THROW(Cut(1));
	CATCH
		if(Flags & sfCancelled) {
			Flags &= ~sfCancelled;
			ok = -1;
		}
		else {
			SetErrorMessage();
			ok = (ExecFR(Beep), 0);
		}
	ENDCATCH
	Flags &= ~sfOpenCheck;
	return ok;
}

int SLAPI SCS_SHTRIHFRF::GetPrintErrCode()
{
	return ErrCode;
}

int SLAPI SCS_SHTRIHFRF::GetSummator(double * val)
{
	int    ok = 1;
	double cash_amt = 0.0;
	ResCode = RESCODE_NO_ERROR;
	THROW(ConnectFR());
	THROW(SetFR(RegisterNumber, CASH_AMOUNT_REG));
	THROW(ExecFR(GetCashReg));
	THROW(GetFR(ContentsOfCashRegister, &cash_amt));
	CATCH
		ok = (SetErrorMessage(), 0);
	ENDCATCH
	ASSIGN_PTR(val, cash_amt);
	return ok;
}

// @v10.3.9 int SLAPI SCS_SHTRIHFRF::InitChannel() { return 1; }

FR_INTRF * SLAPI SCS_SHTRIHFRF::InitDriver()
{
	FR_INTRF * p_drv = 0;
	SCS_SHTRIHFRF::PayTypeRegFlags = 0;
	THROW_MEM(p_drv = new ComDispInterface);
	THROW(p_drv->Init("AddIn.DrvFR"));
	THROW(ASSIGN_ID_BY_NAME(p_drv, ResultCode) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ResultCodeDescription) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Password) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Beep) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ComNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, BaudRate) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Timeout) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, GetExchangeParam) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, SetExchangeParam) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Connect) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Disconnect) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Quantity) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Price) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Summ1) > 0);
	PayTypeRegFlags |= (1U << 1); // @v10.6.1
	THROW(ASSIGN_ID_BY_NAME(p_drv, Summ2) > 0);
	PayTypeRegFlags |= (1U << 2); // @v10.6.1
	THROW(ASSIGN_ID_BY_NAME(p_drv, Summ3) > 0);
	PayTypeRegFlags |= (1U << 3); // @v10.6.1

	if(ASSIGN_ID_BY_NAME(p_drv,  Summ4)) PayTypeRegFlags |= (1U <<  4); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv,  Summ5)) PayTypeRegFlags |= (1U <<  5); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv,  Summ6)) PayTypeRegFlags |= (1U <<  6); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv,  Summ7)) PayTypeRegFlags |= (1U <<  7); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv,  Summ8)) PayTypeRegFlags |= (1U <<  8); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv,  Summ9)) PayTypeRegFlags |= (1U <<  9); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv, Summ10)) PayTypeRegFlags |= (1U << 10); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv, Summ11)) PayTypeRegFlags |= (1U << 11); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv, Summ12)) PayTypeRegFlags |= (1U << 12); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv, Summ13)) PayTypeRegFlags |= (1U << 13); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv, Summ14)) PayTypeRegFlags |= (1U << 14); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv, Summ15)) PayTypeRegFlags |= (1U << 15); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv, Summ16)) PayTypeRegFlags |= (1U << 16); // @v10.6.1
	if(ASSIGN_ID_BY_NAME(p_drv, CloseCheckEx)) ExtMethodsFlags |= extmethfCloseCheckEx; // @v10.6.3

	THROW(ASSIGN_ID_BY_NAME(p_drv, Tax1) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Tax2) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Tax3) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Tax4) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, StringForPrinting) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, UseReceiptRibbon) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, UseJournalRibbon) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, PrintString) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, PrintWideString) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, StringQuantity) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, FeedDocument) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, DocumentName) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, DocumentNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, PrintDocumentTitle) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, CheckType) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, OpenCheck) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Sale) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ReturnSale) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, CloseCheck) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, CutCheck) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, DrawerNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, OpenDrawer) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, TableNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, RowNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, FieldNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, GetFieldStruct) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ReadTable) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, WriteTable) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ValueOfFieldInteger) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ValueOfFieldString) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, RegisterNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, GetOperationReg) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ContentsOfOperationRegister) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, GetCashReg) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ContentsOfCashRegister) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, GetECRStatus) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ECRMode) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ECRModeDescription) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ECRAdvancedMode) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ReceiptRibbonOpticalSensor) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, JournalRibbonOpticalSensor) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ContinuePrint) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, CancelCheck) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, PrintReportWithCleaning) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, PrintReportWithoutCleaning) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, UModel) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, UMajorProtocolVersion) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, UMinorProtocolVersion) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, GetDeviceMetrics) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, CashIncome) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, CashOutcome) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ClearSlipDocumentBuffer) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, FillSlipDocumentWithUnfiscalInfo) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, StringNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, PrintSlipDocument) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, IsClearUnfiscalInfo) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, InfoType) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, EKLZIsPresent) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, IsEKLZOverflow) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, FMOverflow) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, FreeRecordInFM) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, IsFM24HoursOver) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, IsDrawerOpen) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Department) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ECRModeStatus) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, JournalRibbonIsPresent) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ReceiptRibbonIsPresent) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, OutputReceipt) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ReceiptOutputType) > 0);
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, PrintBarCode) > 0); // @v9.1.4
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, Print2DBarcode) > 0); // @v9.1.4
	THROW(ASSIGN_ID_BY_NAME(p_drv, PrintBarcodeGraph) > 0); // @v9.1.4
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, PrintBarcodeUsingPrinter) > 0); // @v9.1.4
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, PrintBarcodeLine) > 0); // @v9.1.4
	THROW(ASSIGN_ID_BY_NAME(p_drv, BarcodeType) > 0); // @v9.1.4
	THROW(ASSIGN_ID_BY_NAME(p_drv, BarCode) > 0); // @v9.1.4
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, BarcodeDataLength) > 0); // @v9.1.4
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, BarWidth) > 0); // @v9.1.4
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, BarcodeStartBlockNumber) > 0);
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, BarcodeParameter1) > 0);
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, BarcodeParameter2) > 0);
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, BarcodeParameter3) > 0);
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, BarcodeParameter4) > 0);
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, BarcodeParameter5) > 0);
	// @v9.1.7 THROW(ASSIGN_ID_BY_NAME(p_drv, BarcodeAlignment) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, FirstLineNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, LineNumber) > 0);
	CATCH
		ZDELETE(p_drv);
	ENDCATCH
	return p_drv;
}

int SLAPI SCS_SHTRIHFRF::AnnulateCheck()
{
	int    ok = -1, mode = 0, adv_mode = 0, cut = 0;
	int    dont_use_cont_prn = 0;
	PPIniFile ini_file;
	ini_file.GetInt(PPINISECT_CONFIG, PPINIPARAM_SHTRIHFRNOUSECONTPRN, &dont_use_cont_prn);
	THROW(ExecFR(GetECRStatus));
	if(!dont_use_cont_prn) {
		// Проверка на незавершенную печать
		THROW(GetFR(ECRAdvancedMode, &adv_mode));
		if(adv_mode == PRNMODE_AFTER_NO_PAPER) {
			Flags |= sfOpenCheck;
			THROW(ExecFRPrintOper(ContinuePrint));
			do {
				THROW(ExecFR(GetECRStatus));
				THROW(GetFR(ECRAdvancedMode, &adv_mode));
			} while(adv_mode == PRNMODE_PRINT);
			Flags &= ~sfOpenCheck;
			cut = 1;
		}
	}
	// Проверка на наличие открытого чека, который надо аннулировать
	THROW(GetFR(ECRMode, &mode));
	if(mode == FRMODE_OPEN_CHECK) {
		Flags |= sfOpenCheck | sfCancelled;
		PPMessage(mfInfo|mfOK, PPINF_SHTRIHFR_CHK_ANNUL);
		THROW(ExecFRPrintOper(CancelCheck));
		Flags &= ~(sfOpenCheck | sfCancelled);
		cut = 1;
		ok = 1;
	}
	if(cut && oneof3(DeviceType, devtypeShtrih, devtypeCombo, devtypeMini) && !(Flags & sfNotUseCutter))
		THROW(ExecFRPrintOper(CutCheck));
	CATCHZOK
	return ok;
}

int SLAPI SCS_SHTRIHFRF::ReadStringFromTbl(int tblNum, int rowNum, int fldNum, SString & rStr)
{
	int    ok = 1;
	char   buf[256];
	THROW(SetFR(TableNumber, tblNum));
	THROW(SetFR(RowNumber,   rowNum));
	THROW(SetFR(FieldNumber, fldNum));
	THROW(ExecFR(GetFieldStruct));
	THROW(ExecFR(ReadTable));
	THROW(GetFR(ValueOfFieldString, buf, sizeof(buf)));
	CATCH
		buf[0] = 0;
		ok = 0;
	ENDCATCH
	rStr = buf;
	return ok;
}

int SLAPI SCS_SHTRIHFRF::ReadValueFromTbl(int tblNum, int rowNum, int fldNum, long * pValue)
{
	int   ok = 1;
	long  val = 0;
	THROW(SetFR(TableNumber, tblNum));
	THROW(SetFR(RowNumber,   rowNum));
	THROW(SetFR(FieldNumber, fldNum));
	THROW(ExecFR(GetFieldStruct));
	THROW(ExecFR(ReadTable));
	THROW(GetFR(ValueOfFieldInteger, &val));
	CATCHZOK
	ASSIGN_PTR(pValue, val);
	return ok;
}

static void WriteLogFileToWriteTblErr(int tblNum, int rowNum, int fldNum, const char * pValue)
{
	if(CConfig.Flags & CCFLG_DEBUG) {
		SString msg_fmt, msg, value;
		(value = pValue).Transf(CTRANSF_OUTER_TO_INNER);
		msg.Printf(PPLoadTextS(PPTXT_LOG_SHTRIH_WRITETBLERR, msg_fmt), tblNum, rowNum, fldNum, value.cptr());
		PPLogMessage(PPFILNAM_SHTRIH_LOG, msg, LOGMSGF_TIME|LOGMSGF_USER);
	}
}

int SLAPI SCS_SHTRIHFRF::WriteStringToTbl(int tblNum, int rowNum, int fldNum, const char * pStr)
{
	int   ok = 1;
	THROW(SetFR(TableNumber, tblNum));
	THROW(SetFR(RowNumber,   rowNum));
	THROW(SetFR(FieldNumber, fldNum));
	THROW(ExecFR(GetFieldStruct));
	THROW(SetFR(ValueOfFieldString, pStr));
	THROW(ExecFR(WriteTable));
	CATCH
		ok = (WriteLogFileToWriteTblErr(tblNum, rowNum, fldNum, pStr), 0);
	ENDCATCH
	return ok;
}

int SLAPI SCS_SHTRIHFRF::WriteValueToTbl(int tblNum, int rowNum, int fldNum, long value)
{
	int     ok = 1;
	SString str_val;
	THROW(SetFR(TableNumber, tblNum));
	THROW(SetFR(RowNumber,   rowNum));
	THROW(SetFR(FieldNumber, fldNum));
	THROW(ExecFR(GetFieldStruct));
	THROW(SetFR(ValueOfFieldInteger, value));
	THROW(ExecFR(WriteTable));
	CATCH
		ok = (WriteLogFileToWriteTblErr(tblNum, rowNum, fldNum, str_val.Cat(value)), 0);
	ENDCATCH
	return ok;
}

int SLAPI SCS_SHTRIHFRF::SetupTables()
{
	int    ok = 1;
	long   cshr_pssw;
	SString temp_buf;//cshr_name;
	SString cshr_str;
	if(PPObjPerson::GetCurUserPerson(0, &temp_buf) == -1) {
		PPObjSecur sec_obj(PPOBJ_USR, 0);
		PPSecur sec_rec;
		if(sec_obj.Fetch(LConfig.User, &sec_rec) > 0)
			temp_buf = sec_rec.Name;
	}
	// @v9.2.1 PPGetWord(PPWORD_CASHIER, 0, cshr_str);
	PPLoadString("cashier", cshr_str); // @v9.2.1
	cshr_str.Space().Cat(temp_buf).Transf(CTRANSF_INNER_TO_OUTER);
	// Получаем пароль кассира
	THROW(ReadValueFromTbl(FRCASHIER_TBL, FRCASHIER_ROW, FRCASHIER_FIELD_PSSW, &cshr_pssw));
	// Приходится проверять незакрытый чек, иначе не удастся записать имя кассира
	CashierPassword = cshr_pssw;
	THROW(AnnulateCheck());
	// Записываем в таблицы настройки (только под паролем администратора)
	CashierPassword = AdmPassword;
	// Имя кассира
	THROW(WriteStringToTbl(FRCASHIER_TBL, FRCASHIER_ROW, FRCASHIER_FIELD_NAME, cshr_str));
	// Имя администратора
	THROW(WriteStringToTbl(FRCASHIER_TBL, FRCASHIER_ADMINROW, FRCASHIER_FIELD_NAME, AdmName.NotEmpty() ? AdmName : cshr_str));
	// Настройки режима работы кассы
	//    Установить автоматическое обнуление наличности
	THROW(WriteValueToTbl(FRCASHMODE_TBL, FRCASHMODE_ROW, FRCASHMODE_AUTOCASHNULL, 1));
	//    Установить открывание денежного ящика
	//THROW(WriteValueToTbl(FRCASHMODE_TBL, FRCASHMODE_ROW, (DeviceType == devtypeCombo ||
	//	DeviceType == devtypeMini) ? COMBOCASHMODE_OPENDRAWER : FRCASHMODE_OPENDRAWER, 1);
	//    Нет автоматической отрезки чека
	THROW(WriteValueToTbl(FRCASHMODE_TBL, FRCASHMODE_ROW,
		(DeviceType == devtypeCombo || DeviceType == devtypeMini) ? COMBOCASHMODE_CUTTING : FRCASHMODE_CUTTING, 0));
	/* @v9.7.4 Проблемы с модернизированными аппаратами
	//    Установить использование весовых датчиков
	THROW(WriteValueToTbl(FRCASHMODE_TBL, FRCASHMODE_ROW, oneof2(DeviceType, devtypeCombo, devtypeMini) ?
		COMBOCASHMODE_USEWGHTSENSOR : FRCASHMODE_USEWGHTSENSOR, BIN(Flags & sfUseWghtSensor)));
	*/
	//    Установить автоматический перевод времени
	//THROW(WriteValueToTbl(FRCASHMODE_TBL, FRCASHMODE_ROW, (DeviceType == devtypeCombo ||
	//	DeviceType == devtypeMini) ? COMBOCASHMODE_AUTOTIMING : FRCASHMODE_AUTOTIMING, 1));
	//    Не сохранять строки в буфере чека
	/* @v9.7.5 В новых аппаратах путаница с нумерацией полей. Просто ничего не будем писать в это поле.
	if(oneof2(DeviceType, devtypeCombo, devtypeMini)) {
		THROW(WriteValueToTbl(FRCASHMODE_TBL, FRCASHMODE_ROW, COMBOCASHMODE_SAVESTRING, 0));
	}
	else {
		THROW(WriteValueToTbl(FRCASHMODE_TBL, FRCASHMODE_ROW, FRCASHMODE_SAVESTRING, 0));
	}
	*/
	/*    Современные версии Штриха запрещают редактирование таблицы перевода времени
	THROW(ReadValueFromTbl(FRCASHMODE_TBL, FRCASHMODE_ROW, (DeviceType == devtypeCombo ||
		DeviceType == devtypeMini) ? COMBOCASHMODE_AUTOTIMING : FRCASHMODE_AUTOTIMING, &auto_timing));
	// Установка автоматического перевода времени
	if(!auto_timing) {
		int    i, k, d, m, y;
		SString str_year;
		LDATE  cur_dt = getcurdate_();
		decodedate(&d, &m, &y, &cur_dt);
		for(i = 1; i <= FRTIMESWITCH_MAXROW; i++) {
			long    td, tm, ty;
			THROW(ReadStringFromTbl(FRTIMESWITCH_TBL, i, FRTIMESWITCH_SEASON, str_year));
			ty = str_year.ToLong();
			if(ty >= y) {
				THROW(ReadValueFromTbl(FRTIMESWITCH_TBL, i, FRTIMESWITCH_DAY, &td));
				THROW(ReadValueFromTbl(FRTIMESWITCH_TBL, i, FRTIMESWITCH_MONTH, &tm));
				if(diffdate(encodedate(td, tm, ty), cur_dt) >= 0)
					break;
			}
		}
		for(k = 1; k < i; k++) {
			THROW(WriteValueToTbl(FRTIMESWITCH_TBL, k, FRTIMESWITCH_ALLOW, 0));
		}
		THROW(WriteValueToTbl(FRCASHMODE_TBL, FRCASHMODE_ROW, (DeviceType == devtypeCombo ||
			DeviceType == devtypeMini) ? COMBOCASHMODE_AUTOTIMING : FRCASHMODE_AUTOTIMING, 1));
	}
	*/
	//
	// Наименования типов оплат
	//
	{
		// @v9.7.1 temp_buf = "БЕЗНАЛИЧНАЯ ОПЛАТА"; // @cstr #13
		PPLoadText(PPTXT_CCFMT_CASHLESSPAYM, temp_buf); // @v9.7.1
		temp_buf.ToUpper().Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
		THROW(WriteStringToTbl(FRPAYMTYPE_TBL, 2, FRPAYMTYPE_NAME, temp_buf));
	}
	THROW(WriteStringToTbl(FRPAYMTYPE_TBL, 3, FRPAYMTYPE_NAME, ""));
	THROW(WriteStringToTbl(FRPAYMTYPE_TBL, 4, FRPAYMTYPE_NAME, ""));
	// Формат чека
	if(!(Flags & sfOldShtrih)) {
		THROW(ReadValueFromTbl(FRFORMAT_TBL, FRFORMAT_ROW, FRFORMAT_FIELD_SIZE, &CheckStrLen));
	}
	CATCHZOK
	CashierPassword = cshr_pssw;
	return ok;
}

int SLAPI SCS_SHTRIHFRF::ConnectFR()
{
	int    ok = -1;
	if(Flags & sfConnected) {
		if(RefToIntrf > 1) {
			THROW(ExecFR(Disconnect));
			THROW(SetFR(ComNumber, Handle));
			THROW(ExecFR(Connect));
			THROW(AnnulateCheck());
		}
	}
	else {
		//#define DEF_BAUD_RATE              2   
		//#define MAX_BAUD_RATE              6   // Для Штрих-ФР max скорость обмена 115200 бод
		const int __def_baud_rate = 2; // Для Штрих-ФР скорость обмена по умолчанию 9600 бод
		const int __max_baud_rate = 6; // Для Штрих-ФР max скорость обмена 115200 бод

		int    baud_rate;
		int    model_type = 0;
		int    major_prot_ver = 0;
		int    minor_prot_ver = 0;
		int    not_use_wght_sensor = 0;
		long   def_baud_rate = __def_baud_rate;
		int    def_timeout = -1;
		SString buf, buf1;
		PPIniFile ini_file;
		int    temp_int = 0;
		SCardPaymEntryN = ini_file.GetInt(PPINISECT_CONFIG, PPINIPARAM_SHTRIHFRSCARDPAYMENTRY, &temp_int) ? inrangeordefault(static_cast<long>(temp_int), 1, 16, 0) : 0; // @v10.6.2
		THROW_PP(ini_file.Get(PPINISECT_SYSTEM, PPINIPARAM_SHTRIHFRPASSWORD, buf) > 0, PPERR_SHTRIHFRADMPASSW);
		buf.Divide(',', buf1, AdmName);
		CashierPassword = AdmPassword = buf1.ToLong();
		AdmName.Strip().Transf(CTRANSF_INNER_TO_OUTER);
		if(ini_file.Get(PPINISECT_CONFIG, PPINIPARAM_SHTRIHFRCONNECTPARAM, buf) > 0) {
			SString  buf2;
			if(buf.Divide(',', buf1, buf2) > 0)
				def_timeout = buf2.ToLong();
			def_baud_rate = buf1.ToLong();
			if(def_baud_rate > __max_baud_rate)
				def_baud_rate = __def_baud_rate;
		}
		THROW_PP(PortType == COM_PORT, PPERR_SYNCCASH_INVPORT);
		THROW(SetFR(ComNumber, Handle));
		if(def_timeout >= 0 && def_timeout < MAX_TIMEOUT)
			THROW(SetFR(Timeout, def_timeout));
		THROW((ok = ExecFR(Connect)) > 0 || ResCode == RESCODE_NO_CONNECTION);
		for(baud_rate = 0; !ok && baud_rate <= __max_baud_rate; baud_rate++) {
			THROW(SetFR(BaudRate, baud_rate));
			THROW((ok = ExecFR(Connect)) > 0 || ResCode == RESCODE_NO_CONNECTION);
		}
		THROW(ok > 0);
		Flags |= sfConnected;
		THROW(GetFR(BaudRate, &baud_rate));
		if(baud_rate != def_baud_rate) {
			THROW(SetFR(BaudRate, def_baud_rate));
			THROW(ExecFR(SetExchangeParam));
			THROW(ExecFR(Disconnect));
			THROW(ExecFR(Connect));
		}
		THROW(ExecFR(GetDeviceMetrics) > 0);
		THROW(GetFR(UModel, &model_type));
		THROW(GetFR(UMajorProtocolVersion, &major_prot_ver));
		THROW(GetFR(UMinorProtocolVersion, &minor_prot_ver));
		if(oneof2(model_type, SHTRIH_FRF, SHTRIH_FRK))
			DeviceType = devtypeShtrih;
		else if(oneof2(model_type, SHTRIH_MINI_FRK, SHTRIH_MINI_FRK_V2))
			DeviceType = devtypeMini;
		else if(oneof2(model_type, SHTRIH_COMBO_FRK, SHTRIH_COMBO_FRK_V2))
			DeviceType = devtypeCombo;
#if 0 // @construction {
		// @v7.2.2 {
		else if(model_type == SHTRIH_LIGHT_FRK) {
			DeviceType = devtypeShtrih; // (Временно будем идентифицировать так это значение)
			// (Должно быть так, но это требует перестройки многих точек кода) DeviceType = devtypeLight;
		}
		// } @v7.2.2
#endif // } 0 @construction
		SETFLAG(Flags, sfOldShtrih, (DeviceType == devtypeShtrih) && major_prot_ver < 2 && minor_prot_ver < 2);
		THROW(ini_file.GetInt(PPINISECT_CONFIG, PPINIPARAM_SHTRIHFRNOTUSEWEIGHTSENSOR, &not_use_wght_sensor));
		SETFLAG(Flags, sfUseWghtSensor, !not_use_wght_sensor);
		THROW(SetupTables());
	}
	CATCH
		if(Flags & sfConnected) {
			SetErrorMessage();
			ExecFR(Disconnect);
		}
		else {
			Flags |= sfConnected;
			SetErrorMessage();
		}
		Flags &= ~sfConnected;
		ok = 0;
	ENDCATCH
	return ok;
}

int SLAPI SCS_SHTRIHFRF::SetFR(PPID id, int iVal)
	{ return BIN(P_DrvFRIntrf && P_DrvFRIntrf->SetProperty(id, iVal) > 0); }
int SLAPI SCS_SHTRIHFRF::SetFR(PPID id, long lVal)
	{ return BIN(P_DrvFRIntrf && P_DrvFRIntrf->SetProperty(id, lVal) > 0); }
int SLAPI SCS_SHTRIHFRF::SetFR(PPID id, double dVal)
	{ return BIN(P_DrvFRIntrf && P_DrvFRIntrf->SetProperty(id, dVal) > 0); }
int SLAPI SCS_SHTRIHFRF::SetFR(PPID id, const char * pStrVal)
	{ return BIN(P_DrvFRIntrf && P_DrvFRIntrf->SetProperty(id, pStrVal) > 0); }

int SLAPI SCS_SHTRIHFRF::GetFR(PPID id, int * pBuf)
	{ return BIN(P_DrvFRIntrf && P_DrvFRIntrf->GetProperty(id, pBuf) > 0); }
int SLAPI SCS_SHTRIHFRF::GetFR(PPID id, long * pBuf)
	{ return BIN(P_DrvFRIntrf && P_DrvFRIntrf->GetProperty(id, pBuf) > 0); }
int SLAPI SCS_SHTRIHFRF::GetFR(PPID id, double * pBuf)
	{ return BIN(P_DrvFRIntrf && P_DrvFRIntrf->GetProperty(id, pBuf) > 0); }
int SLAPI SCS_SHTRIHFRF::GetFR(PPID id, char * pBuf, size_t bufLen)
	{ return BIN(P_DrvFRIntrf && P_DrvFRIntrf->GetProperty(id, pBuf, bufLen) > 0); }

int SLAPI SCS_SHTRIHFRF::ExecFR(PPID id)
{
	int    ok = 1;
	THROW(P_DrvFRIntrf);
	THROW(P_DrvFRIntrf->SetProperty(Password, CashierPassword) > 0);
	THROW(P_DrvFRIntrf->CallMethod(id) > 0);
	THROW(P_DrvFRIntrf->GetProperty(ResultCode, &ResCode) > 0);
	THROW(ResCode == RESCODE_NO_ERROR);
	CATCHZOK
	return ok;
}

int SLAPI SCS_SHTRIHFRF::ExecFRPrintOper(PPID id)
{
	int    ok = 1;
	THROW(P_DrvFRIntrf && P_DrvFRIntrf->SetProperty(Password, CashierPassword) > 0);
	do {
		THROW(P_DrvFRIntrf->CallMethod(id) > 0);
		THROW(P_DrvFRIntrf->GetProperty(ResultCode, &ResCode) > 0);
		if(ResCode == RESCODE_DVCCMDUNSUPP || (Flags & sfPrintSlip && ResCode == RESCODE_SLIP_IS_EMPTY)) {
			ok = -1;
			break;
		}
	} while(ResCode != RESCODE_NO_ERROR && (ok = AllowPrintOper(id)) > 0);
	CATCHZOK
	return ok;
}

static int IsModeOffPrint(int mode)
{
	return oneof5(mode, FRMODE_OPEN_SESS, FRMODE_CLOSE_SESS, FRMODE_OPEN_CHECK, FRMODE_FULL_REPORT, FRMODE_LONG_EKLZ_REPORT) ? 0 : 1;
}

void SLAPI SCS_SHTRIHFRF::WriteLogFile(PPID id)
{
	if(CConfig.Flags & CCFLG_DEBUG) {
		int     adv_mode = 0;
		size_t  pos = 0;
		char    mode_descr[MAXPATH];
		SString msg_fmt, msg;
		SString err_msg = DS.GetConstTLA().AddedMsgString;
		SString oper_name;
		PPLoadText(PPTXT_LOG_SHTRIH, msg_fmt);
		P_DrvFRIntrf->GetNameByID(id, oper_name);
		memzero(mode_descr, sizeof(mode_descr));
		GetFR(ECRModeDescription, mode_descr, sizeof(mode_descr) - 1);
		SCharToOem(mode_descr);
		if(err_msg.SearchChar('\n', &pos))
			err_msg.Trim(pos);
		GetFR(ECRAdvancedMode, &adv_mode);
		msg.Printf(msg_fmt, oper_name.cptr(), err_msg.cptr(), mode_descr, adv_mode);
		PPLogMessage(PPFILNAM_SHTRIH_LOG, msg, LOGMSGF_TIME|LOGMSGF_USER);
	}
}
//
// Функция AllowPrintOper разбирается со всеми ситуациями,
//  которые могут возникнуть при печати чека.
// Код возврата: 1 - операция печати разрешена, 0 - запрещена.
//
int SLAPI SCS_SHTRIHFRF::AllowPrintOper(PPID id)
{
	int    ok = 1, mode = 0, adv_mode = PRNMODE_NO_PRINT, slip_mode_status = SLIPMODE_BEFORE_PRINT, last_res_code = ResCode;
	int    is_chk_rbn = 1, is_jrn_rbn = 1;
	int    wait_prn_err = 0;

	SetErrorMessage();
	// Ожидание окончания операции печати
	do {
		THROW(ExecFR(GetECRStatus));
		THROW(GetFR(ECRMode,         &mode));
		THROW(GetFR(ECRAdvancedMode, &adv_mode));
		if(Flags & sfPrintSlip) {
			THROW(GetFR(ECRModeStatus, &slip_mode_status));
		}
		else {
			THROW(GetFR(ReceiptRibbonOpticalSensor, &is_chk_rbn));
			if(is_chk_rbn && (Flags & sfUseWghtSensor))
				THROW(GetFR(ReceiptRibbonIsPresent, &is_chk_rbn));
			if(DeviceType == devtypeShtrih) {
				THROW(GetFR(JournalRibbonOpticalSensor, &is_jrn_rbn));
				if(is_jrn_rbn && Flags & sfUseWghtSensor)
					THROW(GetFR(JournalRibbonIsPresent, &is_jrn_rbn));
			}
		}
		if((!(Flags & sfPrintSlip) && adv_mode == PRNMODE_NO_PRINT) || oneof2(adv_mode, PRNMODE_PRINT, PRNMODE_PRINT_LONG_REPORT))
			wait_prn_err = 1;
	} while(oneof2(adv_mode, PRNMODE_PRINT, PRNMODE_PRINT_LONG_REPORT) || (slip_mode_status != SLIPMODE_BEFORE_PRINT && slip_mode_status != SLIPMODE_AFTER_PRINT));
	if(Flags & sfPrintSlip) {
		if(oneof2(slip_mode_status, SLIPMODE_BEFORE_PRINT, SLIPMODE_AFTER_PRINT)) {
			ExecFR(Beep);
			wait_prn_err = 1;
			if(PPMessage(mfConf|mfYesNo, PPCFM_CONTSLIPDOCPRINT) != cmYes) {
				Flags |= sfCancelled;
				ok = 0;
			}
		}
	}
	else {
		// На всякий случай помечаем, что чек открыт
		// (иначе при сбое операции открытия чека неизвестно: чек уже открыт или нет)
		if(mode == FRMODE_OPEN_CHECK)
			Flags |= sfOpenCheck;
		// Ожидание заправки чековой ленты или выхода из режима, когда нельзя печатать чек
		while(ok && (oneof2(adv_mode, PRNMODE_NO_PRINT_NO_PAPER, PRNMODE_PRINT_NO_PAPER) ||
			(last_res_code == RESCODE_MODE_OFF && IsModeOffPrint(mode)))) {
			int  send_msg = 0, r;
			if(!is_chk_rbn) {
				PPSetError(PPERR_SYNCCASH_NO_CHK_RBN);
				send_msg = 1;
			}
			else if(!is_jrn_rbn) {
				PPSetError(PPERR_SYNCCASH_NO_JRN_RBN);
				send_msg = 1;
			}
			else
				WriteLogFile(id);
			ExecFR(Beep);
			wait_prn_err = 1;
			r = PPError();
			if((!send_msg && r != cmOK) || (send_msg && ExecFR(Beep) &&
				PPMessage(mfConf|mfYesNo, PPCFM_SETPAPERTOPRINT) != cmYes)) {
				Flags |= sfCancelled;
				ok = 0;
			}
			THROW(ExecFR(GetECRStatus));
			THROW(GetFR(ECRMode,         &mode));
			THROW(GetFR(ECRAdvancedMode, &adv_mode));
		}
		// Проверяем, надо ли завершить печать после заправки ленты
		if(adv_mode == PRNMODE_AFTER_NO_PAPER) {
			WriteLogFile(id);
			THROW(ExecFRPrintOper(ContinuePrint));
			THROW(ExecFR(GetECRStatus));
			THROW(GetFR(ECRMode,         &mode));
			THROW(GetFR(ECRAdvancedMode, &adv_mode));
			wait_prn_err = 1;
		}
		//
		//   Это, конечно, не отрывок из "Илиады", а очередная попытка
		//   справиться с идиотскими ошибками, возникающими из-за этой дерьмовой ЭКЛЗ.
		//
		if(mode == FRMODE_OPEN_CHECK && id == CutCheck) {
			WriteLogFile(id);
			SString  err_msg(DS.GetConstTLA().AddedMsgString), added_msg;
			if(PPLoadText(PPTXT_APPEAL_CTO, added_msg))
				err_msg.CR().Cat("\003").Cat(added_msg);
			PPSetAddedMsgString(err_msg);
			ExecFR(Beep);
			PPError();
			ok = -1;
		}
	}
	//
	// Если ситуация не связана непосредственно с процессом печати, выдаем сообщение об ошибке
	// @v5.9.2 При закрытии чека - сумма оплаты меньше суммы чека - не связано с процессом печати, но wait_prn_err == 1
	//
	if(!wait_prn_err || oneof2(last_res_code, RESCODE_PAYM_LESS_SUM, RESCODE_INVEKLZSTATE)) {
		WriteLogFile(id);
		ExecFR(Beep);
		PPError();
		Flags |= sfCancelled;
		ok = 0;
	}
	CATCHZOK
	return ok;
}

void SLAPI SCS_SHTRIHFRF::SetErrorMessage()
{
	char   err_buf[MAXPATH];
	memzero(err_buf, sizeof(err_buf));
	if((Flags & sfConnected) && ResCode != RESCODE_NO_ERROR && GetFR(ResultCodeDescription, err_buf, sizeof(err_buf)-1) > 0) {
		SString err_msg;
		err_msg.Cat(err_buf);
		if(ResCode == RESCODE_MODE_OFF && ExecFR(GetECRStatus) > 0) {
			char mode_descr[MAXPATH];
			memzero(mode_descr, sizeof(mode_descr));
			if(GetFR(ECRModeDescription, mode_descr, sizeof(mode_descr) - 1) > 0) {
				SString temp_buf;
				// @v9.7.1 temp_buf = "Режим"; // @cstr
				PPLoadText(PPTXT_CCFMT_MODE, temp_buf); // @v9.7.1
				temp_buf.Transf(CTRANSF_INNER_TO_OUTER); // @v9.7.1
				err_msg.CR().CatChar('\003').Cat(temp_buf).CatDiv(':', 2).Cat(mode_descr);
			}
		}
		err_msg.Transf(CTRANSF_OUTER_TO_INNER);
		PPSetError(PPERR_SYNCCASH, err_msg);
	}
}

// @vmiller
int SLAPI SCS_SHTRIHFRF::PrintBnkTermReport(const char * pZCheck)
{
	int     ok = 1;
	StringSet str_set('\n', pZCheck);
	SString line_buf;
	// @v9.7.10 SlipDocCommonParam sdc_param;
	// @v9.7.10 THROW(P_SlipFmt->Init("CCheck", &sdc_param));
	ResCode = RESCODE_NO_ERROR;
	ErrCode = SYNCPRN_ERROR_AFTER_PRINT;
	THROW(ConnectFR());
	RibbonParam = 0;
	CheckForRibbonUsing(SlipLineParam::fRegRegular);
	THROW(SetFR(DocumentName, ""));
	THROW(ExecFRPrintOper(PrintDocumentTitle));
	for(uint pos = 0; str_set.get(&pos, line_buf) > 0;) {
		CheckForRibbonUsing(SlipLineParam::fRegRegular);
		CutLongTail(line_buf);
		THROW(SetFR(StringForPrinting, line_buf));
		THROW(ExecFRPrintOper(PrintString));
	}
	THROW(LineFeed(3, TRUE, FALSE));
	THROW(Cut(1));
	CATCHZOK
	return ok;
}
