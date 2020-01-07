// FRONTOL.H
// Copyright (c) A.Sobolev 2016
//
// �����������, ������������ ��������� ��������, ����������� � ����������� ����������� �������
//
//
//   ��������� ��� ������ � ATOL_CARD
//
#define AC_MAX_OPER_COUNT   250

//
//   ��������� � �������� "�������"
//

//   ����� � ATOL ��� PAPYRUS
#define ATOL_INNER_SCHEME           1
#define ATOL_OUTER_SCHEME        1001
#define AC_BIAS_CARD_CAT_CODE   10000 //   BIAS ��� ���� ��������� ���� � ATOL-CARD
#define AC_DEF_CARD_CODE_LEN       40 //   ����� ���� ���� �����
//   ��� ����
#define FRONTOL_CHK_SALE            0   // �������
#define FRONTOL_CHK_RETURN          1   // �������
//   ��� ������
#define FRONTOL_PAYMENT_CASH        1   // ���������
//   ��� �������� (����������) � �����
#define FRONTOL_OPTYPE_CHKLINEFREE  1   // ������ ���� �� ��������� ����
#define FRONTOL_OPTYPE_STORNOFREE   2   // ������ ������ ���� �� ��������� ����
#define FRONTOL_OPTYPE_CHKLINE     11   // ������ ����
#define FRONTOL_OPTYPE_STORNO      12   // ������ ������
#define FRONTOL_OPTYPE_PAYM1       40   // ������ � ������ ����� �������
#define FRONTOL_OPTYPE_PAYM2       41   // ������ ��� ����� ����� �������
#define FRONTOL_OPTYPE_CHKCLOSED   55   // �������� ����
#define FRONTOL_OPTYPE_CANCEL      56   // ������ ����
#define FRONTOL_OPTYPE_ZREPORT     61   // Z-�����

struct _FrontolZRepEntry {
	long   PosN;
	long   ZRepN;
	long   SessID;
};

class _FrontolZRepArray : public TSArray <_FrontolZRepEntry> {
public:
	_FrontolZRepArray() : TSArray <_FrontolZRepEntry> ()
	{
	}
	int    Search(long posN, long zRepN, uint * pPos) const
	{
		_FrontolZRepEntry key;
		MEMSZERO(key);
		key.PosN = posN;
		key.ZRepN = zRepN;
		return lsearch(&key, pPos, PTR_CMPFUNC(_2long));
	}
	int    Search(long sessID, uint * pPos) const
	{
		return lsearch(&sessID, pPos, CMPF_LONG);
	}
};

struct AtolGoodsDiscountEntry {
	long   GetSchemeID() const
	{
		return (GoodsID + 10000);
	}
	PPID   GoodsID;
	PPID   QuotKindID;
	double AbsDiscount;
};

struct FrontolCcPayment {
	FrontolCcPayment()
	{
		Reset();
	}
	void Reset()
	{
		Amount = 0.0;
		CashAmt = 0.0;
		BankAmt = 0.0;
		CrdSCardList.freeAll();
	}
	double Amount;
	double CashAmt;
	double BankAmt;
	RAssocArray CrdSCardList;
};
