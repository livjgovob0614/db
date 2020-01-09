// PPEDI.H
// Copyright (c) A.Sobolev 2015
//
#ifndef __PPEDI_H // {
#define __PPEDI_H

#include <sxml.h>
#include <ppdefs.h>

struct PPEdiMessageEntry {
	PPEdiMessageEntry()
	{
		THISZERO();
	}
	enum {
		statusUnkn = 0,
		statusNew  = 1
	};
	int    ID;             // ������������� ���������, ���������������� �������� ����������� ���������� (�� �������� �� ����������)
    int    EdiOp;          // ��� EDI-��������
    LDATETIME Dtm;         // ����� ��������/����������� ���������
    S_GUID Uuid;           // GUID ���������
    long   Status;         // PPEdiMessageEntry::statusXXX
    long   Flags;          // �����
    long   PrvFlags;       // �����, ����������� ��� ����������� ����������
    char   Code[24];       // ��� ��������� (����� ���������)
    char   SenderCode[24];
    char   RcvrCode[24];
    char   Box[64];        // ���� �������� ��������� ���������������� �� ������, �� ����� ����� ���� ��� ����� ��� ���������
    char   SId[128];       // ���������� ������������� (����� ���� ������ �����)
};

class PPEdiMessageList : public TSArray <PPEdiMessageEntry> {
public:
	PPEdiMessageList() : TSArray <PPEdiMessageEntry>()
	{
		LastId = 0;
	}
	PPEdiMessageList & Clear()
	{
		freeAll();
		// LastId �� �������� � ����� ����������� ������������ ��������������� ��� ������ ������
		return *this;
	}
	int    SearchId(int id, uint * pPos) const
	{
		return lsearch(&id, pPos, CMPF_LONG);
	}
	int    Add(const PPEdiMessageEntry & rEntry)
	{
		PPEdiMessageEntry new_entry;
		new_entry = rEntry;
		LastId++;
		SETIFZ(new_entry.ID, LastId);
		return insert(&new_entry);
	}
private:
    int    LastId;
};

class SEancomXmlSegment : public SXml {
public:
	SEancomXmlSegment(const xmlNode * pNode = 0);
	int    operator !() const;

	enum {
		refON = 1, // "ON" Order number
        refIT,     // "IT" Internal customer number
        refYC1,    // "YC1" Additional party identification (EAN code)
        refABT,    // "ABT" Custom declaration number (RUS: ����� ��� � SG17)
        refIV      // "IV"
	};
	struct REF {
		REF() : Type(0)
		{
		}
		REF & Clear()
		{
			Type = 0;
			Text.Z();
			return *this;
		}
        int    Type;
        SString Text;
	};
	enum {
		dtmMsg    = 137,     // ����/����� ���������/��������� //
		dtmRef    = 171,     // ��������� ����/����� (���� ���������, �� ������� ��������� ��� ���������)
		dtmSched  = 358,     // ����������� ����/�����
		dtmDlvr   = 35,      // Delivery date/time actual
		dtmRcpt   = 50,      // Goods receipt date/time
		dtmExpiry = 361,     // ���� ��������� ����� ��������
		dtmExpiryDays = 36   // ���� �������� � ����
	};
	struct DTM {
		DTM() : Type(0), Dtm(ZERODATETIME), Days(0)
		{
		}
		DTM & Clear()
		{
			Type = 0;
			Dtm = ZERODATETIME;
			Days = 0;
			return *this;
		}
		int    Type; // dtmXXX
		LDATETIME Dtm;
		int    Days;
	};
	struct MOA {
		MOA() : Type(0), Value(0.0)
		{
			CurrencySymb[0] = 0;
		}
		MOA & Clear()
		{
			Type = 0;
			CurrencySymb[0] = 0;
			Value = 0.0;
			return *this;
		}
		int    Type;
		char   CurrencySymb[8];
		double Value;
	};
	enum {
		nadSU = 1, // ���������
		nadBY,     // ����������
		nadDP,     // Delivery Party
		nadUD,     // Ultimate Delivery party
		nadIV      // ����������
	};
	struct NAD {
		NAD()
		{
			Clear();
		}
		NAD & Clear()
		{
			Type = 0;
			GLN[0] = 0;
			CountryCode[0] = 0;
			PostalCode[0] = 0;
			Name = 0;
			City = 0;
			Address = 0;
			return *this;
		}
        int    Type;
        char   GLN[20];
        char   CountryCode[8];
        char   PostalCode[20];
        SString Name;
        SString City;
        SString Address;
	};

	int    GetNext(SEancomXmlSegment & rSeg);
	int    Is(const char * pName) const;
	int    IsContent(const char * pText) const;
	SString & GetContent(SString & rBuf) const;
	int    GetMsgTypeName() const;
	int    GetMsgTypeContent() const;
	int    GetMsgType(const char * pText) const;
	int    GetInt(int & rVal) const;
	int    GetReal(double & rVal) const;
	int    GetText(SString & rText) const;
	int    GetREF(REF & rR);
	int    GetDate(DTM & rD);
	int    GetMOA(MOA & rM);
	int    GetNAD(NAD & rN);
private:
	int    IsText(const char * pContent, const char * pText) const;
	int    Set(const xmlNode * pNode);

	const xmlNode * P_Node;
	const xmlNode * P_Cur;
};

#endif // } __PPEDI_H
