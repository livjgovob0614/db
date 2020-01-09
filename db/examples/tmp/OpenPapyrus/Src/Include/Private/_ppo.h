// _PPO.H
// Copyright (c) A.Sobolev 2009
//
struct PPObjectTag_ {   // @persistent @store(ReferenceTbl+)
	PPID   Tag;         // Const=PPOBJ_TAG
	PPID   ID;          // @id
	char   Name[30];    // @name @!refname
	char   Symb[20];    // ������ ��� ������������� � ��������
	char   Reserve[8];  // @reserve
	int16  Flags;       // OTF_XXX
	PPID   LinkObjGrp;  // �������������� �������� ��� ���������� �������
	PPID   TagEnumID;   // ��� ���������� �������
	long   TagDataType; // OTTYP_XXX
	PPID   ObjTypeID;   // ��� �������, ��� �������� ��������� ���
	PPID   TagGroupID;  // ������, � ������� ��������� ���
};

struct PPSecur_ {          // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const={PPOBJ_USR | PPOBJ_USRGRP}
	long   ID;             // @id
	char   Name[32];       // @name @!refname
	PPID   PersonID;       // (USER only) ��������� ���������� //
	long   Reserve[3];     // @reserve
	char   Password[20];   // (USER only) ������
	long   Flags;          // (USER only) ����� (USRF_XXX)
	long   ParentID;       // ������������ ������ (������ | ������������)
	LDATE  PwUpdate;       // (USER only) ���� ���������� ��������� ������ //
};

struct PPBarcodeStruc_ {   // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_BCODESTRUC
	long   ID;             // @id
	char   Name[48];       // @name
	char   Templ[20];      // ������
	long   Flags;          // �����
	long   Reserve1;       // @reserve
	long   Reserve2;       // @reserve
};

struct PPSCardSeries_ {    // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_SCARDSERIES
	long   ID;             // @id
	char   Name[30];       // @name @!refname
	char   CodeTempl[18];  // ������ ������� ����
	long   QuotKindID;     // ��� ���������
	long   PersonKindID;   // @v5.1.8 ��� ����������, ������������ ��� ���������� ���� (�� ��������� - PPPRK_CLIENT)
	long   PDis;           // ������ (.01%)
	double MaxCredit;      // ������������ ������ (��� ��������� ����)
	long   Flags;          // �����
	LDATE  Issue;          // ���� �������
	LDATE  Expiry;         // ���� ��������� �������� //
};

struct PPUnit_ {           // @persistent @store(ReferenceTbl+)
	enum {
		SI       = 0x0001, // (S) ������� ������� ��
		Phisical = 0x0002, // (P) ���������� �������
		Trade    = 0x0004, // (T) �������� ������� (����� �� ����� ������������ ���. �����������)
		Hide     = 0x0008, // (H) ������� �� ������� ����������
		IntVal   = 0x0010  // (I) ������� ����� ���� ������ �������������
	};
	long   Tag;            // Const=PPOBJ_UNIT
	long   ID;             // @id
	char   Name[32];       // @name @!refname
	char   Abbr[16];       // 56
	PPID   BaseUnitID;     // ->Ref(PPOBJ_UNIT)
	double BaseRatio;      // 68
	char   Reserve[16];    // 84
	long   Flags;          // 88
};

struct PPNamedObjAssoc_ {  // @persistent @store(ReferenceTbl+)
	enum {
		fLocAsWarehouse = 0x0001,
		fLocAsWareplace = 0x0002
	};
	long   Tag;            // Const=PPOBJ_NAMEDOBJASSOC
	long   ID;             // @id
	char   Name[48];       // @name @!refname
	char   Symb[12];       //
	long   Reserve;        // @reserve
	long   ScndObjGrp;     //
	long   Flags;          //
	long   PrmrObjType;    // PPOBJ_GOODS ||
	long   ScndObjType;    // PPOBJ_WAREHOUSE || PPOBJ_WAREPLACE || PPOBJ_ARTICLE
};

struct PPPersonKind_ {     // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_PRSNKIND
	long   ID;             // @id
	char   Name[48];       // @name
	long   CodeRegTypeID;  // ��� ���������������� ���������, ������������� ��� ������
	PPID   FolderRegTypeID; // @v5.0.6 ��� ��������, ����������������� ������������ �������� � ����������� �� ����������
	long   Flags;          // @v5.9.10 @flags
	PPID   DefStatusID;    // @v5.9.10 ->Ref(PPOBJ_PRSNSTATUS) ������ ����� ���������� �� ���������
	char   Reserve[16];    // @reserve
};

struct PPPersonStatus_ {   // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_PRSNSTATUS
	long   ID;             // @id
	char   Name[48];       // @name
	char   Reserve1[20];   // @reserve
	long   Flags;          // PSNSTF_XXX
	long   Reserve2[2];    // @reserve
};

struct PPELinkKind_ {      // @persistent @store(ReferenceTbl+)
	PPID   Tag;            // Const=PPOBJ_ELINKKIND
	PPID   ID;             // @id
	char   Name[32];       // @name @!refname
	char   Reserve1[36];   // @reserve
	long   Flags;          //
	PPID   Type;           // ELNKRT_XXX
	long   Reserve2;       // @reserve
};

struct PPCurrency_ {       // @persistent @store(ReferenceTbl+)
	PPID   Tag;            // Const=PPOBJ_CURRENCY
	PPID   ID;             // @id
	char   Name[32];       // @name @!refname
	char   Reserve1[16];   // @reserve
	char   Symb[16];       //
	long   Code;           // Numeric currency code
	long   Flags;          //
	long   Reserve3[2];    //
};

struct PPCurRateType_ {    // @persistent @store(ReferenceTbl+)
	PPID   Tag;            // Const=PPOBJ_CURRATETYPE
	PPID   ID;             // @id
	char   Name[32];       // @name @!refname
	char   Reserve1[36];   // @reserve
	long   Flags;          //
	long   Reserve2[2];    // @reserve
};

struct PPAmountType_ {     // @persistent @store(ReferenceTbl+)
	int    SLAPI IsTax(PPID taxID  /* GTAX_XXX */) const;
	int    SLAPI IsComplementary() const;
	enum {
		fErrOnDefault = 0x0001, // ���� ����� � ��������� �����������, ��
			// ������������ ��������� �� ������
		fManual       = 0x0002, // ����� ����� ���� ������� � ������.
			// ���� ���� ����������, �� ����������� ������ ���� �����.
			// ���� ���, �� ����� ������������� ������������� �� ����������
			// ���������. ���������� - ����������� ����� (PPAMT_MAIN).
			// ��� ����� ���� ��� �������������� ��� � ��������������� �
			// ������ � ����������� �� ���� ��������.
		fTax          = 0x0004, // ����� ������. ���� ���� ���� ����������,
			// �� �������� ����� �������� ����� ������ Tax �� ������� (TaxRate*100) %,
			// ������� ���������� ������ ��������.
		//
		// ���������� ��������� ���� ������ � ���, ����� ����� ���� ������ ���������� ��������
		// ���������� ����� GenCost, GenPrice, GenDiscount �� ��������� ���������.
		//
		fReplaceCost     = 0x0010, // �������� ����� � ����� ����������� (Cost)
		fReplacePrice    = 0x0020, // �������� ����� � ����� ����������� (Price)
		fReplaceDiscount = 0x0040, // �������� ������ (Discount)
			// @#{fReplaceCost^fReplacePrice^fReplaceDiscount}
		fInAmount        = 0x0080, // �������� �����, ��������������� ����� RefAmtTypeID
		fOutAmount       = 0x0100, // ��������� �����, ��������������� ����� RefAmtTypeID
			// @#{fTax^fInAmount^fOutAmount}
		fReplaces        = (fReplaceCost | fReplacePrice | fReplaceDiscount),
		fStaffAmount     = 0x0200  // ������� ����� (������������ ��� ������� ��������)
	};
	long   Tag;            // Const=PPOBJ_AMOUNTTYPE
	long   ID;             // @id
	char   Name[48];       // @name
	char   Symb[20];       // ������ ��� ������������� � ��������
	long   Flags;          // �����
	long   Tax;            // ����� (GTAX_XXX)
	union {
		long   TaxRate;       // ������ ������ � ����� ����� �������� (Flags & fTax)
		PPID   RefAmtTypeID;  // �����, ��������������� ������ (Flags & (fInAmount | fOutAmount))
	};
};

struct PPOprType_ {        // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_OPRTYPE
	long   ID;             // @id
	char   Name[48];       // @name
	char   Pict[6];        //
	char   Reserve1[14];   //
	long   Reserve2;       //
	long   Reserve3;       //
	long   Reserve4;       //
};

struct PPOpCounter_ {      // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_OPCOUNTER
	long   ID;             // @id
	char   Name[30];       // @name @!refname
	char   CodeTemplate[18]; // ������ ���������
	char   Reserve[16];    // @reserve
	PPID   ObjType;        // ��� �������, ���������� ���������. ���� 0, �� PPOBJ_OPRKIND
	long   Flags;          // OPCNTF_XXX
	long   Counter;        //
	long   OwnerObjID;     // ������-�������� �������� (���� 0, �� ����� ������������ ����� ��� ����� ��������)
		// ���� OwnerObjID == -1, �� ������� ����������� ������������ ������� ObjType
};

struct PPGdsCls_ {         // @persistent @store(ReferenceTbl+)
	static int   SLAPI IsEqByDynGenMask(long mask, const GoodsExtTbl::Rec * p1, const GoodsExtTbl::Rec * p2);
	void   SLAPI SetDynGenMask(int fld, int val);
	int    FASTCALL GetDynGenMask(int fld) const;

	enum { // @persistent
		eKind = 1,
		eGrade,
		eAdd,
		eX,
		eY,
		eZ,
		eW,    // @v5.1.9
		eAdd2  // @v5.1.9
	};
	enum { // @persistent
		fUsePropKind    = 0x0001,
		fUsePropGrade   = 0x0002,
		fUsePropAdd     = 0x0004,
		fUseDimX        = 0x0008,
		fUseDimY        = 0x0010,
		fUseDimZ        = 0x0020,
		fStdEditDlg     = 0x0100, // ������������ �����������, � �� �������������� ������
			// �������������� �������, ����������� � ������
		fDupCombine     = 0x0200, // ���������� {PropKind, PropGrade, DimX, DimY, DimZ}
			// ����� ������������� //
		fDisableFreeDim = 0x0400, // ������ �� ���� ������������ �� ������������� � ������
		fUseDimW        = 0x0800, // @v5.1.9 ���������� ����������� W
		fUsePropAdd2    = 0x1000  // @v5.1.9 ���������� �������� Add2
	};

	long   Tag;            // Const=PPOBJ_GOODSCLASS
	long   ID;             // @id
	char   Name[32];       // @name @!refname
	char   Reserve[12];    // @reserve
	PPID   DefGrpID;       // ������ ������ �� ���������. ������������ � ������ �������� ������ ������ �� ���������� ����������.
	PPID   DefUnitID;      // �������� ������� ��������� �� ���������
	PPID   DefPhUnitID;    // ���������� ������� ��������� �� ���������
	long   DefPhUPerU;     // @unused
	PPID   DefTaxGrpID;    // ��������� ������ �� ���������
	PPID   DefGoodsTypeID; // ��� ������ �� ���������
	long   Flags;          // @flags
	uint16 EditDlgID;      // @unused
	uint16 FiltDlgID;      // @unused
	long   DynGenMask;     // @v5.0.1 ����� ������������� ���������. ����� ����� �������������
		// ��������� ������������ (1<<(PPGdsCls::eXXX-1)). ���� ������� ����� ���������� ����� G �
		// ����������� ����������, ���������������� �����, �������������� ���������� ������ X,
		// �� �������, ��� X ����������� G.
};

struct PPAssetWrOffGrp_ {  // @persistent @store(ReferenceTbl+)
	PPID   Tag;            // Const=PPOBJ_ASSTWROFFGRP
	PPID   ID;             // @id
	char   Name[30];       // @name @!refname
	char   Reserve1[4];    // @reserve
	char   Code[14];       // ��� ����� ��������������� ����������
	long   Reserve2[2];    // @reserve
	long   WrOffType;      // ��� �������� (AWOGT_XXX)
	long   WrOffTerm;      // Months
	long   Limit;          // ���������� ���������� ��������� (% �� ��������� ���������)
	long   Flags;          //
	long   Reserve3[2];    // @reserve
};

struct PPOprKind_ {        // @persistent @store(ReferenceTbl+)
	SLAPI  PPOprKind_()
	{
		THISZERO();
	}

	long   Tag;            // Const=PPOBJ_OPRKIND
	long   ID;             // @id
	char   Name[42];       // @name @!refname
	int16  Rank;           //
	PPID   LinkOpID;       //
	PPID   AccSheet2ID;    //
	PPID   OpCounterID;    //
	long   PrnFlags;       //
	PPID   DefLocID;       //
	int16  PrnOrder;       //
	int16  SubType;        // OPSUBT_XXX
	long   Flags;          // OPKF_XXX
	PPID   OpTypeID;       //
	PPID   AccSheetID;     //
};

struct PPBillStatus_ {     // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_BILLSTATUS
	long   ID;             // @id
	char   Name[20];       // @name @!refname
	char   Symb[16];       // ������ �������
	char   Reserve1[32];   // @reserve
	int16  Reserve2;       // @reserve
	int16  Rank;           //
	long   Flags;          // BILSF_XXX
	long   Reserve3;       // @reserve
	// @v5.8.2 (���������� ������� ReferenceTbl::Rec) long   Reserve4;    // @reserve
};

struct PPAccSheet_ {       // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_ACCSHEET
	long   ID;             // @id
	char   Name[48];       // @name
	PPID   BinArID;        // ������ ��� ������ �������� �� ����������� �������
	PPID   CodeRegTypeID;  // �� ���� ���������������� ���������, ����������������� ����������, ��������������� ������.
	char   Reserve1[12];   // @reserve
	long   Flags;          // ACSHF_XXX
	long   Assoc;          // @#{0L, PPOBJ_PERSON, PPOBJ_LOCATION, PPOBJ_ACCOUNT} ��������������� ������
	long   ObjGroup;       // ��������� ��������������� ��������
};

struct PPCashNode_ {       // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_CASHNODE
	long   ID;             // @id
	char   Name[8];        // @name @!refname
	char   Port[8];        // ��� ����� (LPT1, COM1, ...)
	char   Reserve[8];     // @reserve
	PPID   GoodsLocAssocID;  // @v5.7.3 ����������� ���������� �����-�����, ����������� ��� ������ ����� � �������� //
	char   Reserve2[2];    // @reserve
	uint16 SleepTimeout;   // @v5.3.10 ������� ����������� (���), ����� �������� ������ ����������� //
	PPID   CurRestBillID;  // ��������� �������� ������� �������� �� ���������� �������� �������
	char   Accum[8];       // �����������
	//
	// ���� DownBill ����� ����������� ����������.
	// ��� �������� ���������� �������� ������ ���������� ������ ����� �����
	// ���� ��������� �������, ��� � ������ ����� ���������� �� ������� //
	// ������� �� ��� � ���� �������� (������ Btrieve = 2). ��-�����
	// ������ ��� ��������� � ������ �������� ��������� �����������. ��
	// ���� ������� ��� ������� � ���������� ������, �� �������� �����������
	// ����� �������� ��������. ���� DownBill ��� ��� � ���������
	// ������������� ����� ���������. ����� ���������� ������ �������
	// �������� �������� ������ ����� ���������� � ���� ����������.
	// � ���������� �������� ��� ���� ������ �������.
	//
	PPID   DownBill;
	PPID   CashType;       // ��� ��� (PPCMT_XXX)
	long   LogNum;         // ���������� ����� �����
	int16  DrvVerMajor;    //
	int16  DrvVerMinor;    //
	PPID   CurSessID;      // ->CSession.ID ������� �������� ������ (��� ���������� �����)
	PPID   ExtQuotID;      // ->Ref(PPOBJ_QUOTKIND) �������������� ��������� (������������ ���������� ������ ���)
	long   Flags;          // CASHF_XXX      �����
	long   LocID;          // ->Location.ID  �����
	LDATE  CurDate;        // ������� ������������ ����
};

struct PPLocPrinter_ {     // @persistent @store(ReferenceTbl+)
	SLAPI  PPLocPrinter_()
	{
		THISZERO();
	}
	long   Tag;            // Const=PPOBJ_LOCPRINTER
	long   ID;             // @id
	char   Name[20];       // @name @!refname
	char   Port[48];       // ��� ����� ������
	long   Flags;          //
	long   LocID;          // ->Location.ID  �����
	long   Reserve2;       // @reserve
};

struct PPBarcodePrinter_ { // @persistent @store(ReferenceTbl+)
	SLAPI  PPBarcodePrinter_()
	{
		THISZERO();
	}
	int    SLAPI Normalyze();

	enum {
		fPortEx = 0x0001 // ������ ���������� ����������� ���� ����� ����� ������
	};
	PPID   Tag;            // Const=PPOBJ_BCODEPRINTER
	PPID   ID;             // @id
	char   Name[20];       // @name @!refname
	char   Port[18];       //
	char   LabelName[20];  //
	CommPortParams Cpp;    // Size=6
	char   Reserve1[4];    // @reserve
	long   Flags;          //
	PPID   PrinterType;    // PPBCPT_ZEBRA | PPBCPT_DATAMAX
	long   Reserve2;       // @reserve
	//
	SString PortEx;        // @persistent @store(PropertyTbl[PPOBJ_BCODEPRINTER, id, BCPPRP_PORTEX]) @anchor
};

struct PPStyloPalm_ {      // @persistent @store(ReferenceTbl+)
	PPID   Tag;            // Const=PPOBJ_STYLOPALM
	PPID   ID;             // @id
	char   Name[20];       // @name @!refname
	char   Reserve1[28];   // @reserve
	PPID   LocID;          // ->Location.ID
	PPID   GoodsGrpID;     //
	PPID   OrderOpID;      // ->Ref(PPOBJ_OPRKIND)
	PPID   AgentID;        // ->Article.ID; Agent - owner of palm
	PPID   GroupID;        // ->Ref(PPOBJ_STYLOPALM) ������, ������� ����������� ����������
		// ������, ����������� �� ������, ��������� � ������ ��������� ����: LocID, OrderOpID,
		// Flags & (PLMF_IMPASCHECKS|PLMF_EXPCLIDEBT|PLMF_EXPSELL), PPStyloPalmPacket::P_LocList
	long   Flags;          // PLMF_XXX
	PPID   FTPAcctID;      // @v5.0.1 AHTOXA
	long   Reserve4;       //
};

struct PPTouchScreen_ {    // @persistent @store(ReferenceTbl+)
	SLAPI  PPTouchScreen_()
	{
		THISZERO();
	}
	PPID   Tag;            // Const=PPOBJ_TOUCHSCREEN
	PPID   ID;             // @id
	char   Name[20];       // @name @!refname
	PPID   TouchScreenType;  //
	PPID   AltGdsGrpID;    // �������������� ������, ������ �� ������� ������������ � �������� ������ �� ���������
	long   GdsListFontHight;    // @v5.3.11 VADIM
	char   GdsListFontName[32]; // @v5.3.11 VADIM
	uint8  GdsListEntryGap;     // @v5.3.11 �������������� ��������� ������ ������ ������ ������ ������ (pixel)
		// ������������� ��������: [0..8]
	char   Reserve[3];     // @reserve
	long   Flags;          // TSF_XXX
	long   Reserve2[2];    // @reserve
};

struct PPInternetAccount_ { // @persistent @store(ReferenceTbl+)
	enum {
		fFtpAccount = 0x00000001L
	};
	SLAPI  PPInternetAccount_()
	{
		THISZERO();
	}
	void   SLAPI Init()
	{
		ExtStr = 0;
		memzero(this, sizeof(*this)-sizeof(ExtStr));
	}
	int    SLAPI Cmp(PPInternetAccount_ * pAccount);
	int    SLAPI NotEmpty();
	int    SLAPI GetExtField(int fldID, char * pBuf, size_t bufLen);
	int    SLAPI GetExtField(int fldID, SString & rBuf);
	int    SLAPI SetExtField(int fldID, const char * pBuf);
	int    SLAPI SetPassword(const char *, int fldID = MAEXSTR_RCVPASSWORD); // @v5.0.1 AHTOXA change
	int    SLAPI GetPassword(char * pBuf, size_t bufLen, int fldID = MAEXSTR_RCVPASSWORD); // @v5.0.1 AHTOXA change
	int    SLAPI SetMimedPassword(const char * pPassword, int fldID  = MAEXSTR_RCVPASSWORD); // @v5.3.5 AHTOXA
	int    SLAPI GetMimedPassword(char * pBuf, size_t bufLen, int fldID = MAEXSTR_RCVPASSWORD); // @v5.3.5 AHTOXA
	int    SLAPI GetSendPort();
	int    SLAPI GetRcvPort();
	//
	long   Tag;            // Const=PPOBJ_INTERNETACCOUNT
	long   ID;             //
	char   Name[32];       // @name
	uint16 SmtpAuthType;   // ��� �������������� ��� ���������� � SMTP ��������
	char   Reserve1[32];   // @reserve
	int16  Timeout;        // ������� ������� (���)
	long   Flags;          //
	PPID   PersonID;       // ->Person.ID
	long   Reserve2;       // @reserve
	//
	SString ExtStr;        // @anchor
};

struct PPDBDiv_ {          // @persistent @store(ReferenceTbl+)
	//
	// Descr: ��������� ����� ��������� ��������� ������� DBDIVF_XXX.
	//   ������������ ����� PPDBDiv::fXXX ������. ������� ��� ������� �������.
	//
	enum {
		fDispatch              = 0x0001, // ������-���������
		fSCardOnly             = 0x0002, // ����� � �������� ���� ������ �� ������ ����������� ����
		fRcvCSessAndWrOffBills = 0x0004, // ������ ��������� �������� ������ ������ � ����������� �������� //
			// ���� ���� ���� ����������, �� ������� PPObjectTransmit::PutObjectToIndex ���������� ���������
			// �������� �������� ������ �������� �� ���� DBDXF_SENDCSESSION � ������������ ������ �������.
		fConsolid              = 0x0008, // ������, ����������� ��������������� ���������� �� ������
			// �������� (��������� ��� �������� �����)
		fPassive               = 0x0010  // ��������� ������
	};
	long   Tag;           // Const=PPOBJ_DBDIV
	long   ID;            // ��
	char   Name[30];      // ������������ �������
	char   Addr[38];      // ����� ���� e-mail ��� ���-�� � ���� ����
	long   Flags;         // �����
	long   IntrRcptOpr;   // �������� ������������� �������
	long   OutCounter;    // ������� ��������� �������
};

struct PPGoodsType_ {      // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_GOODSTYPE
	long   ID;             //
	char   Name[30];       //
	char   Reserve1[18];   //
	PPID   WrOffGrpID;     // ������ �������� �������� ������ (required GTF_ASSETS)
	PPID   AmtCVat;        // ����� ��� � ����� �����������             //
	PPID   AmtCost;        // ->Ref(PPOBJ_AMOUNTTYPE) ����� ����������� //
	PPID   AmtPrice;       // ->Ref(PPOBJ_AMOUNTTYPE) ����� ����������  //
	PPID   AmtDscnt;       // ->Ref(PPOBJ_AMOUNTTYPE) ����� ������
	long   Flags;          //
	long   Reserve3[2];    //
};

struct PPGoodsStrucHeader_ { // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_GOODSSTRUC
	long   ID;             // @id
	char   Name[30];       // @name
	char   Reserve1[18];   // @reserve
	PPID   VariedPropObjType; // ��� ������� ������������ �������������� ������������� ������ ��������� � ��������� ���������.
	DateRange Period;      // ������ ������������ ���������
	double CommDenom;      //
	long   Flags;          // GSF_XXX
	PPID   ParentID;       // ������������ ���������
	long   Reserve3;       // @reserve
};

struct PPGoodsTax_ {       // @persistent @store(ReferenceTbl+)
	int    SLAPI ToEntry(PPGoodsTaxEntry *) const;
	int    SLAPI FromEntry(const PPGoodsTaxEntry *);
	PPID   Tag;            // Const=PPOBJ_GOODSTAX
	PPID   ID;             //
	char   Name[30];       //
	char   Symb[14];       //
	double VAT;            //
	double Excise;         //
	double SalesTax;       //
	long   Flags;          //
	long   Order;          //
	long   UnionVect;      //
};

struct PPRegisterType_ {   // @persistent @store(ReferenceTbl+)
	PPID   Tag;            // Const=PPOBJ_REGISTERTYPE
	PPID   ID;             // @id
	char   Name[40];       // ������������ ���� ��������
	char   Symb[20];       // ������ ��� ������ �� ������ � �.�.
	PPID   PersonKindID;   // ��� ����������, � ������� ������ ���������� ���� �������. ���� 0, �� � ������ ����.
	PPID   RegOrgKind;     // ->Ref(PPOBJ_PRSNKIND) ��� �������������� �����������
	long   Flags;          // @flags
	PPID   CounterID;      // ->Ref(PPOBJ_OPCOUNTER)
	long   Reserve2;       // @reserve
};

struct PPQuotKind_ {       // @persistent @store(ReferenceTbl+)
	PPID   Tag;            // Const=PPOBJ_QUOTKIND
	PPID   ID;             // @id
	char   Name[30];       // @name
	double Discount;       // ������
	char   Symb[12];       // ������
	DateRange  Period;     // ������ �������� ��������� ���������
	int16  BeginTm;        // ����� ������ �������� ��������� ���������
	int16  EndTm;          // ����� ��������� �������� ��������� ���������
	int16  Rank;           // ������� ���������� ��������� ��� ������ � �������
		// �������������� ��� � ��� �������, ����� ��� ��������� �� ����� ���� ������������
		// � ���� ����������� �� ���������� ����� (��������, ��� �������� � StyloPalm).
		// ��� ���� ��������, ��� ���� ����������� ������������� ����� ���� ���������
		// �� ��������� � �������.
	PPID   OpID;           // ��� ��������, ��� ������� ���������� ���������
	long   Flags;          // ����� (QUOTKF_XXX)
	PPID   AccSheetID;     // ������� ������, � �������� ������������� �������� ���������.
		// ���� AccSheetID == 0, �� ����������, ��� ������� ������ GetSellAccSheet() (����������)
	char   DaysOfWeek;     // @v5.1.7 VADIM ��� ������ �������� ��������� ��������� (0x01 - ��, ... , 0x40 - ��)
	char   UsingWSCard;    // @v5.1.7 VADIM ���������� ������������� � ����������� ������� (uwscXXX)
	enum {                 //
		uwscDefault = 0,   // - ������� ���������, ����� ������ �� �����
		uwscSCardNQuot,    // - ������� ������ �� �����, ����� ���������
		uwscOnlyQuot,      // - ������ ���������
		uwscOnlySCard      // - ������ ������ �� �����
	};
	char   Reserve[2];     // @reserve
};

struct PPPsnOpKind_ {      // @persistent @store(ReferenceTbl+)
	PPID   Tag;            // Const=PPOBJ_PERSONOPKIND
	PPID   ID;             // @id
	char   Name[36];       // @name // @v5.4.7 48->36
	char   Symb[14];       // @v5.4.7 ������
	PPID   PairOp;         // @#{PairOp != ID} ->self.ID   ������ �������� //
	PPID   RegTypeID;      // ->Ref(PPOBJ_REGISTERTYPE).ID ��� ���������������� ���������
	short  Reserve2;       // @reserve
	short  ExValGrp;       // ������ ��������������� �������� (POPKEVG_XXX)
	PPID   ExValSrc;       // �������� ��������������� �������� //
		// if(ExValGrp == POPKEVG_TAG) then ExValSrc ->Ref(PPOBJ_TAG).ID
	short  PairType;       // ��� �������� (POPKPT_XXX)
	long   Flags;          // ����� (POPKF_XXX)
	PPID   LinkBillOpID;   // ->Ref(PPOBJ_OPRKIND).ID @v5.2.2
	PPID   ParentID;       // ������������ ������
};

struct PPWorldObjStatus_ { // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_CITYSTATUS
	long   ID;             // @id
	char   Name[32];       // @name
	char   Abbr[16];       // ����������
	char   Reserve[20];    // @reserve
	long   Flags;          // @reserve
	long   Kind;           // WORLDOBJ_XXX
	long   Code;           // ������������� (��������, ������ � ������� ���������������). ���� Code!=0, �� �������� ���������
};

struct PPPersonRelType_ {  // @persistent @store(ReferenceTbl+)
	enum {
		cOneToOne = 1,
		cOneToMany,
		cManyToOne,
		cManyToMany
	};
	enum {
		ssUndef = 0,
		ssPrivateToPrivate,
		ssPrivateToLegal,
		ssLegalToPrivate,
		ssLegalToLegal
	};
	enum {
		fInhAddr  = 0x0001,
		fInhRAddr = 0x0002,
		fGrouping = 0x0004, // @v5.7.1 ������������ ��������� (������ ��� Cardinality = cManyToOne)
		fInhMainOrgAgreement = 0x0008 // ��������� ���������� � ��������� �� ������� ����������� (������ ��� ��������)
	};
	long   Tag;            // Const=PPOBJ_PERSONRELTYPE
	long   ID;             //
	char   Name[32];       //
	char   Reserve1[16];   //
	char   Symb[20];       //
	int16  StatusRestriction; // ����������� �� ������� ��������� (PPPersonRelType::ssXXX)
	int16  Cardinality;    // ����������� �� ��������������� ��������� (PPPersonRelType::cXXX)
	long   Flags;          // ����� (PPPersonRelType::fXXX)
	long   Reserve2;       // @reserve // @v5.8.2 [2]-->[1] (���������� ������� ReferenceTbl::Rec)
};

struct PPSalCharge_ {      // @persistent @store(ReferenceTbl+)
	enum {
		fGroup       = 0x0001, // ������ ���������� (���������� ������������ ������������� ����� ������������ ����������)
		fWrOffSingle = 0x0002  // ���������� �� ���� ������������ �� ���� ������ ��������� ����� ���������� (����� ������������)
	};
	PPID   Tag;            // Const=PPOBJ_SALCHARGE
	PPID   ID;             // @id
	char   Name[30];       // @name
	char   Symb[10];       // ������
	PPID   AmtID;          // ->Ref(PPOBJ_AMOUNTTYPE) ��� ������� ����� ��� ����� ���������� //
	PPID   CalID;          // ->Ref(PPOBJ_STAFFCAL)   ���������, ������������ ��� ����� ���������� //
	PPID   WrOffOpID;      // ->Ref(PPOBJ_OPRKIND)    ��� �������� �������� //
	char   Reserve2[16];   // @reserve
	long   Flags;          //
	long   Reserve3[2];    // @reserve
};

struct PPDateTimeRep_ {    // @persistent @store(ReferenceTbl+)
	PPID   Tag;            // Const=PPOBJ_DATETIMEREP
	PPID   ID;             // @id
	char   Name[48];       // @name
	DateTimeRepeating Dtr; //
	long   Duration;       // ����������������� (sec)
	char   Reserve1[4];    //
	long   Reserve2[3];    //
};

struct PPDutySched_ {      // @persistent @store(ReferenceTbl+)
	PPID   Tag;            // Const=PPOBJ_DUTYSHED
	PPID   ID;             // @id
	char   Name[20];       // @name
	char   Reserve1[48];   // @reserve
	long   Flags;          //
	PPID   ObjType;        //
	long   ObjGroup;       //
};

struct PPStaffCal_ {       // @persistent @store(ReferenceTbl+)
	enum {
		fInherited       = 0x0001, // ��������� ��������� �������� �� ��������. �� ����,
			// ����� ��������� �� ����� ��������� ����������� ���������.
		fUseNominalPeriod = 0x0002  // ��� ���������� �������� � ��������� �� ���������
			// ����������� ���������� ������ ���������� (�� �����������)
	};
	DECL_INVARIANT_C();

	PPID   Tag;            // Const=PPOBJ_STAFFCAL
	PPID   ID;             // @id
	char   Name[30];       // @name @!refname ��� ����������� ���������� (LinkCalID != 0) - �����
	char   Symb[18];       // ������. ��� ����������� ���������� (LinkCalID != 0) - �����
	char   Reserve2[8];    // @reserve
	PPID   PersonKind;     // @v5.7.7 ��� ����������, � �������� ����������� (��������������� � ������������ ���������)
	PPID   SubstCalID;     // @v5.6.10 ���������, ���������� ������������. ������������ � ��� ������,
		// ���� ������������ ��������� �������� ���������, � StubCalID - �����������.
		// ������: ���������� ��������� (���������) ���������� �������� ��� ������ (�����������)
	PPID   LinkObjType;    // ��� ���������� �������
	long   Flags;          //
	PPID   LinkCalID;      // �� ������������� ��������� //
	PPID   LinkObjID;      // �� ���������� �������
};

struct PPGoodsInfo_ {      // @persistent @store(ReferenceTbl+)
	long   ObjType;
	long   ObjID;
	char   Name[20];
	long   LocID;
	long   TouchScreenID;
	long   LabelPrinterID;
	long   Flags;
	char   Reserve[44];
};

struct PPScale_ {          // @persistent @store(ReferenceTbl+)
	int    SLAPI IsValidBcPrefix() const
	{
		return BIN((BcPrefix >= 200 && BcPrefix <= 299) || (BcPrefix >= 20 && BcPrefix <= 29));
	}
	long   Tag;            // Const=PPOBJ_SCALE
	long   ID;             // @id
	char   Name[30];       // @name @!refname
	//
	// System params @v3.2.9 {
	//
	uint16 Get_NumTries;   // @#{0..32000}
	uint16 Get_Delay;      // @#{0..1000}
	uint16 Put_NumTries;   // @#{0..32000}
	uint16 Put_Delay;      // @#{0..1000}
	// }
	int16  BcPrefix;       // @v5.8.6 ������� �������������� ���������, ������������ �� ����
	char   Reserve1[4];    //
	PPID   QuotKindID;     // ->Ref(PPOBJ_QUOTKIND) ��� ���������, ������������ ��� ��������������� //
	//
	// ���� Flags & SCALF_TCPIP, �� IP-����� ���������� ������������� �
	// ���� Port � ����: Port[0].Port[1].Port[2].Port[3]
	//
	char   Port[8];        // ���� ������
	PPID   ScaleTypeID;    // ��� ����������
	long   ProtocolVer;    // ������ ��������� ������. ������� �� ���� ����������
	long   LogNum;         // ���������� ����� ����������. ����������� ��� ��������� ����� ���������.
	long   Flags;          // SCALF_XXX
	long   Location;       // ->Location.ID �����, � �������� ��������� ����������
	long   AltGoodsGrp;    // ->Goods2.ID   �������������� ������ �������, ����������� �� ����
};

struct PPBhtTerminal_ {    // @persistent @store(ReferenceTbl+)
	enum {
		fDelAfterImport = 0x00000001L
	};
	PPID   Tag;            // Const=PPOBJ_BHT
	PPID   ID;             // @id
	char   Name[30];       // @name @!refname
	short  ReceiptPlace;   // ���� ������ ���������  @todo > Flags
	//
	// ��������� com-�����
	//
	uint16 ComGet_NumTries; // @#{0..32000} not used
	uint16 ComGet_Delay;    // @#{0..1000}  for Win32 only
	uint16 ComPut_NumTries; // @#{0..32000} not used
	uint16 ComPut_Delay;   // @#{0..1000}  not used
	char   Reserve1[6];    // @reserve
	PPID   IntrExpndOpID;  // �������� ����������� ����������� (PPOPT_GOODSEXPEND || PPOPT_DRAFTEXPEND)
	PPID   InventOpID;     // Inventory Operation Kind ID
	char   Port[8];        // Output port name (default "COM1")
	int16  Cbr;            // ComBaudRate (default 19200)
	uint16 BhtpTimeout;    // Bht protocol timeout, mc (default 3000)
	uint16 BhtpMaxTries;   // Bht protocol max attempts sending data (default 10)
	long   Flags;          //
	long   BhtTypeID;      // Reserved (Denso only)
	PPID   ExpendOpID;     // Expend Operation Kind ID (PPOPT_GOODSEXPEND || PPOPT_DRAFTEXPEND)
};

struct PPDraftWrOff_ {     // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_DRAFTWROFF
	long   ID;             // @id
	char   Name[48];       // @name
	PPID   PoolOpID;       // �������� ������������ ���� ���������� �������� //
	PPID   DfctCompensOpID; // �������� ����������� ��������
	PPID   DfctCompensArID; // ���������� � ���������� ����������� ��������
	long   Reserve1[2];    // @reserve
	long   Flags;          // ����� DWOF_XXX
	long   Reserve2[2];    // @reserve
};

struct PPAdvBillKind_ {    // @persistent @store(ReferenceTbl+)
	enum {
		fSkipAccturn = 0x0001 // ��� ������ ������������ ��� ��������� � ����� ����� �� �������� ��� ��������
	};
	long   Tag;            // Const=PPOBJ_ADVBILLKIND
	long   ID;             // @id
	char   Name[48];       // @name
	PPID   LinkOpID;       // �������� ���������� ��������� //
	long   Reserve1[4];    // @reserve
	long   Flags;          // ����� PPAdvBillKind::fXXX
	long   Reserve2[2];    // @reserve
};

struct PPGoodsBasket_ {    // @persistent @store(ReferenceTbl+)
	long   Tag;            // Const=PPOBJ_GOODSBASKET
	long   ID;             // @id
	char   Name[30];       // @name @!refname
	long   Num;            // ���������� ����� (�� ������������, �� ����������������)
	long   Flags;          // GBASKF_XXX
	PPID   User;           // ->Ref(PPOBJ_USR) ������������, ��������� �������
	PPID   SupplID;        // ->Article.ID
	char   Reserve1[34];   // @reserve
};

struct PPDraftCreateRule_ { // @persistent @store(ReferenceTbl+)
	enum PriceAlgoritm {  // ������� ����������� ���� ����������
		pByLastLot   = 1, // �� ���������� ����
		pByAvgSum    = 2, // ������� �� ������� �����
		pByAvgSumDis = 3, // ������� �� ������� ����� ����� ������
		pByQuot        = 4, // �� ���������
		pByCostPctVal  = 5 // �� ���� ����������� ���� ��������� �������
	};
    enum CostAlgoritm {      // �������� ����������� ���� ����������
		cByLastLot      = 1, // �� ���������� ����
		cByPricePctVal  = 2, // �� ���� ���������� ����� ��������� �������
		cByQuot         = 3  // �� ���������
	};
	enum {
		fExclGoodsGrp = 0x0001,
		fIsRulesGroup = 0x0002
	};
	PPID   Tag;            // Const=PPOBJ_DFCREATERULE
	PPID   ID;             // @id
	char   Name[20];       // @offse(28) @name @!refname
	PPID   OpID;           // ->Ref(PPOBJ_OPRKIND)
	PPID   ArID;           // ->Article.ID
	PPID   AgentID;        // ->Article.ID
	PPID   GoodsGrpID;     // ->Goods2.ID
	PPID   CQuot;          // ->Ref(PPOBJ_QUOTKIND)
	PPID   PQuot;          // @offse(52) ->Ref(PPOBJ_QUOTKIND)
	int16  CostAlg;        //
	int16  PriceAlg;       // @offse(56)
	float  CPctVal;        //
	float  PPctVal;        // @offse(64)
	double MaxSum;         // @offse(72)
	long   MaxPos;         //
	int16  Flags;          //
	PPID   ParentID;       //
	char   Reserve[2];     // @offse(84)
	long   Reserve2;       // @offse(88) @v5.4.6
};

