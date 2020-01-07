// REPORT.H
// Copyright (c) A.Sobolev 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2010, 2015, 2016, 2019
// @codepage UTF-8
//
#ifndef __REPORT_H
#define __REPORT_H

//#include <Windows.h>
//#include <slib.h>

class TVRez;
//
// Встроенные переменные
//
#define BIVAR_CURDATE             101
#define BIVAR_CURTIME             102
#define BIVAR_PAGE                103
//
// Типы зон отчета
//
#define FIRST_TEXT_ID            1001
#define RPT_HEAD                   -1
#define RPT_FOOT                   -2
#define PAGE_HEAD                  -3
#define PAGE_FOOT                  -4
#define GROUP_HEAD                 -5
#define GROUP_FOOT                 -6
#define DETAIL_BODY                -7
//
// Форматирующие флаги полей
//
#define FLDFMT_BOLD            0x0001
#define FLDFMT_ITALIC          0x0002
#define FLDFMT_UNDERLINE       0x0004
#define FLDFMT_SUPERSCRIPT     0x0008
#define FLDFMT_SUBSCRIPT       0x0010
#define FLDFMT_ESC             0x0020
#define FLDFMT_NOREPEATONPG    0x0040
#define FLDFMT_NOREPEATONRPT   0x0080
#define FLDFMT_STRETCHVERT     0x0100
#define FLDFMT_AGGR            0x0800 // Поле содержит агрегатную функцию
#define FLDFMT_SKIP            0x1000 // Пропустить печать пол
//
// Форматирующие флаги групп
//
#define GRPFMT_NEWPAGE         0x0001
#define GRPFMT_SWAPHEAD        0x0002
#define GRPFMT_SWAPFOOT        0x0004
#define GRPFMT_REPRINTHEAD     0x0008
#define GRPFMT_RESETPGNMB      0x0010
#define GRPFMT_SUMMARYONBOTTOM 0x0020 // Итог на дно страницы
/* @v10.5.6 replaced with AGGRFUNC_XXX (slib.h)
//
// Агрегатные функции для группировки
//
#define AGGR_NONE                   0
#define AGGR_COUNT                  1
#define AGGR_SUM                    2
#define AGGR_AVG                    3
#define AGGR_MIN                    4
#define AGGR_MAX                    5
*/
//
// Опции печати
//
#define SPRN_EJECTBEFORE       0x00000001 // Прогон страницы перед печатью
#define SPRN_EJECTAFTER        0x00000002 // Прогон страницы после печати
#define SPRN_NLQ               0x00000004
#define SPRN_CPI10             0x00000008
#define SPRN_CPI12             0x00000010
#define SPRN_CONDS             0x00000020
#define SPRN_LANDSCAPE         0x00000040 // Альбомная ориентация листа (иначе книжная)
#define SPRN_SKIPGRPS          0x00004000 // Пропустить группировку при печати
#define SPRN_TRUEPRINTER       0x00008000 // Служебный флаг (истинный принтер)
#define SPRN_PREVIEW           0x00010000 // @v6.2.6 Предварительный просмотр
#define SPRN_DONTRENAMEFILES   0x00020000 // @v6.3.5 Обычно, при печати файлы данных переименовываются дабы
	// не мешать работе предыдущему (последующему) сеансу печати. Если эта опция передается в
	// функцию CrystalReportPrint, то файлы переименовываться не будут (иногда это важно).
#define SPRN_USEDUPLEXPRINTING 0x00040000 // Использовать дуплексную печать
//
// Опции функции SPrinter::setEffect
//
#define FONT_BOLD              0x0001
#define FONT_ITALIC            0x0002
#define FONT_UNDERLINE         0x0004
#define FONT_SUPERSCRIPT       0x0008
#define FONT_SUBSCRIPT         0x0010

#define SPCMDSET_DEFAULT            0
#define SPCMDSET_EPSON              1
#define SPCMDSET_PCL                2

#define SPMRG_LEFT                  1
#define SPMRG_RIGHT                 2
#define SPMRG_TOP                   3
#define SPMRG_BOTTOM                4

#define SPQLTY_DRAFT                1
#define SPQLTY_NLQ                  2

#define SPCPI_10                    1
#define SPCPI_12                    2
#define SPCPI_COND                  3

#define SPFS_BOLD                   1
#define SPFS_ITALIC                 2
#define SPFS_UNDERLINE              3

class SPrnCmdSet {
public:
	static SPrnCmdSet * SLAPI CreateInstance(long, long = 0);

	SLAPI SPrnCmdSet();
	virtual SLAPI ~SPrnCmdSet();
	//
	// Каждая из виртуальных функций этого (и порожденных) классов
	// должна занести в буфер, заданный последним параметром, управляющую
	// последовательность и вернуть количество символов в этой
	// последовательности. Не следует завершать управляющую
	// последовательность нулем, так как длина все равно определяетс
	// возвращаемым значением.
	//
	virtual int SLAPI InitPrinter(char *) { return 0; }
	virtual int SLAPI ResetPrinter(char *) { return 0; }
	//
	// SetPageLength задает длину листа (строк - НЕ ДЮЙМОВ !)
	//
	virtual int SLAPI SetPageLength(int, char *) { return 0; }
	virtual int SLAPI SetOrientation(int /* 0 - portrait, !0 - landscapce */, char *) { return 0; }
	virtual int SLAPI SetMargin(int what, int, char *) { return 0; }
	virtual int SLAPI SetQuality(int, char *) { return 0; }
	virtual int SLAPI SetCPI(int, char *) { return 0; }
	virtual int SLAPI SetFontStyle(int, int on_off, char *) { return 0; }
	virtual int SLAPI SetLinesPerInch(int, char *) { return 0; }
private:
	int    dummy;
};

struct ReportDescrEntry {
	//
	// Descr: Типы параметров описания отчетов в report.ini и stdrpt.ini
	//
	enum {
		tUnkn = 0,
		tComment,
		tData,
		tDescr,
		tDiffIdByScope,
		tModifDate,
		tStd,
		tFormat,
		tDestination,
		tSilent,
		tExistFile
	};
	static int FASTCALL GetIniToken(const char * pBuf, SString * pFileName);

	SLAPI  ReportDescrEntry();
	int    SLAPI ParseIniString(const char * pLine, const ReportDescrEntry * pBaseEntry);
	int    SLAPI SetReportFileName(const char * pFileName);

	enum {
		fInheritedTblNames = 0x0001,
		fDiff_ID_ByScope   = 0x0002, // Если этот флаг установлен, то наименования полей идентификаторов
			// записей в таблицах, соответствующих разным областям будут отличаться.
			// Эта опция необходима из-за того, что единовременно перевести все структуры на
			// различающиеся наименования таких полей невозможно по причине необходимости верификации
			// соответствующих отчетов
		fTddoResource      = 0x0004
	};
	long   Flags;
	SString ReportPath_;
	SString Description_;
	SString DataName_;
	SString OutputFormat;
};

struct PrnDlgAns {
	SLAPI  PrnDlgAns(const char * pReportName);
	SLAPI ~PrnDlgAns();
	int    SLAPI SetupReportEntries(const char * pContextSymb);

	enum {
		aUndef = 0,
		aPrint = 1,
		aExport,
		aPreview,
		aExportXML,
		aPrepareData,
		aPrepareDataAndExecCR,
		aExportTDDO
	};
	enum {
		fForceDDF          = 0x0001,
		fEMail             = 0x0002, // Действителен при Dest == aExport
		fUseDuplexPrinting = 0x0004  // Дуплексная печать
	};
	long   Dest;
	int    Selection;
	uint   NumCopies;
	long   Flags;
	const  char * P_ReportName;
	const  char * P_DefPrnForm;
	SString PrepareDataPath;
	SString Printer;
	SString EmailAddr;
	SString ContextSymb;
	TSCollection <ReportDescrEntry> Entries;
	DEVMODEA * P_DevMode; // @v10.4.10
private:
	int    SLAPI PreprocessReportFileName(const char * pFileName, ReportDescrEntry * pEntry);
};

class SPrinter {
public:
	SLAPI  SPrinter();
	SLAPI ~SPrinter();
	int    SLAPI setupCmdSet(long cmdsetID, long extra = 0);
	int    SLAPI setPort(char *);
	int    SLAPI startDoc();
	int    SLAPI endDoc();
	int    SLAPI abortDoc();
	int    SLAPI startPage();
	int    SLAPI endPage();
	int    SLAPI escape(int, char *);
	int    SLAPI printLine(const char * buf, size_t maxLen);
	int    SLAPI printChar(int c);
	int    SLAPI checkPort();
	uint   SLAPI getStyle();
	int    SLAPI setEffect(int);
	int    SLAPI initDevice();
	int    SLAPI resetDevice();
	int    SLAPI checkNWCapture();
	//
	// Descr: Функция HandlePrintError может быть установленна прикладной программой для интерактивной обработки
	//   ошибки печати. Если эта функция возвращает 0, то печать будет прервана по ошибке, в противном случае
	//   последует повторная попытка. По умолчанию эта функция равна NULL, что обрабатывается как завершение по ошибке.
	//
	static int (*HandlePrintError)(int errCode);
	SPrnCmdSet * cmdSet;
	char * device;
	char   port[64];
	int    prnport;
	int    captured; // Признак захвата порта сетевым сервером печати
	int    fd;       // file handler
	int    pgl;
	int    pgw;
	int    leftMarg;
	int    rightMarg;
	int    topMarg;
	int    bottomMarg;
	uint   options;
};

SPrinter * SLAPI getDefaultPrinter();
//
// Функция ReportIterator должна возвращать следующие значения:
// -1 - Конец итерации
//  0 - Ошибка
//  1 - Успешная итерация //
//  2 - Успешная итерация, агрегатные функции не пересчитывать
//
typedef int (*ReportIterator)(int first);

class SReport {
public:
	struct Field {
		int16  id;
		int    name; // @ Index in SReport::text buffer or -1
		TYPEID type;
		union {
			long   format;
			int16  len;  // text
		};
		uint   fldfmt;
		union {
			void * data;
			long   offs; // text
		};
		char * lastval;
	};
	struct Aggr {
		int16  fld;
		int16  aggr;  // Агрегатная функци
		int16  dpnd;  // Поле, которое требуется агрегировать
		int16  scope; // -1 весь отчет, 0.. номер группы
		union {
			double   rtemp;
			double * ptemp;
		};
	};
	struct Group {
		int16   band;
		int16 * fields;
		char  * lastval;
	};
	struct Band {
		int     SLAPI addField(int id);
		int16   kind;
		int16   ht;
		int16   group;
		uint16  options;
		int16 * fields;
	};

	static int defaultIterator(int);
	static SArray * FillRptArray();
	explicit SLAPI SReport(const char *);
	SLAPI  SReport(uint rezID, long flags /* INIREPF_XXX */);
	SLAPI ~SReport();
	int    SLAPI IsValid() const;
	void   SLAPI disableGrouping();
	int    SLAPI addField(int id, TYPEID typ, long fmt, uint rptfmt, char * nam);
	int    SLAPI setAggrToField(int fld, int aggr, int dpnd);
	int    SLAPI addText(char * txt);
	int    SLAPI addBand(SReport::Band * band, int * grp_fld, uint * pos);
	int    SLAPI calcAggr(int grp, int mode); // 0 - init, 1 - calc, 2 - summary
	int    SLAPI check();
	int    SLAPI setData(int id, void * data);
	int    SLAPI skipField(int id, int enable);
	int    SLAPI setIterator(ReportIterator);
	int    SLAPI setPrinter(SPrinter*);
	int    SLAPI setDefaultPrinter();
	int    SLAPI getNumCopies() const;
	int    SLAPI setNumCopies(int);
	int    SLAPI enumFields(SReport::Field **, SReport::Band *, int *);
	int    SLAPI getFieldName(SReport::Field *, char * buf, size_t buflen);
	int    SLAPI getFieldName(int id, char * buf, size_t buflen);
	int    SLAPI printDataField(SReport::Field * f);
	int    SLAPI printPageHead(int kind, int _newpage);
	int    SLAPI printGroupHead(int kind, int grp);
	int    SLAPI checkval(int16 * flds, char ** ptr);
	int    SLAPI printDetail();
	int    SLAPI printTitle(int kind);
	int    SLAPI writeResource(FILE *, uint);
	int    SLAPI readResource(TVRez *, uint resID);
	SReport::Band * SLAPI searchBand(int kind, int grp);
	int    SLAPI createDataFiles(const char * pDataName, const char * pRptPath);
	const  SString & SLAPI getDataName() const { return DataName; }
	int    PrnDest;
	int    SLAPI Export();
	int    SLAPI preview();
private:
	int    SLAPI prepareData();
	int    SLAPI createBodyDataFile(SString & rFileName, SCollection * fldIDs);
	int    SLAPI createVarDataFile(SString & rFileName, SCollection * fldIDs);
public:
	enum rptFlags {
		DisableGrouping = 0x0001,
		FooterOnBottom  = 0x0002,
		Landscape       = 0x0004,
		// @v8.6.6 @obsolete NoEjectAfter    = 0x0008,
		PrintingNoAsk   = 0x0010,
		NoRepError      = 0x0020, // Не выдавать сообщение об ошибке
		XmlExport       = 0x0040, // Экспорт в XML
		Preview         = 0x0080  // Предварительный просмотр
	};
	SString Name;
	SString DataName;
	long   main_id;
	int    page;
	int    line;
	int    fldCount;
	Field * fields;
	int    agrCount;
	Aggr * agrs;
	int    grpCount;
	Group * groups;
	int    bandCount;
	Band * bands;
	//
	// Следующие три поля необходимы для записи в ресурс и передачи классу SPrinter
	//
	int    PageLen;
	int    LeftMarg;
	int    PrnOptions;
	int    PageHdHt; // Высота верхнего колонтитула
	int    PageFtHt; // Высота нижнего колонтитула
	int    Count;
	SPrinter * P_Prn;
	ReportIterator iterator;
private:
	int    Error;
	int    NumCopies;
	size_t TextLen;
	char * P_Text;
};

int SLAPI EditPrintParam(PrnDlgAns * pData);
int SLAPI CrystalReportPrint(const char *, const char * pDir, const char * pPrinter, int numCopies, int options, const DEVMODEA *pDevMode);  //erik{DEVMODEA *pDevMode} add param v10.4.10
int SLAPI CrystalReportExport(const char *, const char * pDir, const char * pReportName, const char * pEMailAddr, int options);

#endif /* __REPORT_H */

