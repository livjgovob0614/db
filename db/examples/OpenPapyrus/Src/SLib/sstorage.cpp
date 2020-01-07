// SSTORAGE.CPP
// Copyright (c) A.Sobolev 2015, 2016, 2019
//
#include <slib.h>
#include <tv.h>
#pragma hdrstop

class SStorage {
public:
	class Address {
	public:
		SLAPI   Address() : P(0)
		{
		}
		SLAPI   Address(uint64 p) : P(p)
		{
		}
		SLAPI   Address(const Address & rS) : P(rS.P)
		{
		}
        uint64  P;
	};
	SLAPI  SStorage();
	SLAPI ~SStorage();
    int    Add(const void * pData, uint size, Address * pA);
	int    Remove(const Address & rS);
    int    Get(const Address & rS, void * pData, uint * pSize);
    uint32 GetPageSize() const { return PgSize; }
    uint32 GetRecSize() const { return RcSize; }
	uint   GetFlags() const { return Flags; }
private:
	static const uint32 DefPgDelta;

	enum {
		fFixRcSize = 0x0001,
		fFixPgSize = 0x0002,
		fStError   = 0x0004
	};
    uint32 PgSize;
    uint32 RcSize;  //
    uint16 Flags;
    uint8  AlignBits;  // �������� ������������ ������� �������� ��� ����������
    uint8  Reserve;    // @alignment
    //
    class Page : public SBaseBuffer {
	public:
		SLAPI  Page(const SStorage & rS, uint pgIndex);
		SLAPI  Page(uint32 pgSize, uint32 rcSize, uint flags, uint pgIndex);
		SLAPI ~Page();
		int    Add(const void * pData, uint size, SStorage::Address & rA);
		int    Remove(const SStorage::Address & rA);
		int    Get(const Address & rS, void * pData, uint * pSize);
		int    Debug_Output(SString & rBuf) const
		{
			rBuf.Z();
			rBuf.CatEq("PgIndex", PgIndex).CatDiv(';', 2).CatEq("PgSize", PgSize).CatDiv(';', 2).CatEq("RcSize", RcSize).CatDiv(';', 2).CatEq("Flags", Flags).CR();
			if(Flags & fFixRcSize) {
				for(uint i = 0; i < FixRcCount; i++) {

				}
			}
			else {
				for(uint i = 0; i < P_E->getCount(); i++) {
				}
			}
			return 1;
		}
	private:
		int    Helper_Construct(uint32 pgSize, uint32 rcSize, uint flags, uint pgIndex);
		void   MakeAddress(uint32 offs, SStorage::Address & rA) const;
		int    Realloc(uint reqSize);
		int    GetFreeEntry(uint reqSize, uint * pFreeEntryPos);
		int    UseFreeEntry(uint freeEntryPos, const void * pData, size_t size, SStorage::Address & rAddr);
		void   DestroyEntries();
		struct Entry32 {
			Entry32(uint p, uint s);
			uint32 P;
			uint32 S;
		};
		TSArray <uint32> FreeEntries;
		uint32 PgSize;  // ������ ��������
		uint32 RcSize;  // ������������� ������ ������
		uint16 Flags;
		uint8  AlignBits;  // �������� ������������ ������� �������� ��� ����������
		uint8  Reserve;    // @alignment
		uint32 PgIndex;    // ������ �������� � ������� �������� ������. ��������� ��� ���������� ��������� �������.
		int64  MapAddress; // ����� � �����, �� ������� ������������ ��������
		union {
			uint32 FixRcCount;
			TSArray <Entry32> * P_E;
		};
    };
    class PageAllocationTable : public TSArray <SStorage::Page> {
	public:
		SLAPI  PageAllocationTable() : TSArray <SStorage::Page> (aryEachItem|aryDataOwner), PageSize(0)
		{
		}
        int    SLAPI Put(const void * pData, uint size, SStorage::Address & rA);
        const void * SLAPI GetDataPtr(SStorage::Address & rA) const;
	private:
		virtual	void FASTCALL freeItem(void * p)
		{
            CALLTYPEPTRMEMB(SStorage::Page, p, Destroy());
		}
		int    PageSize;
    };
	PageAllocationTable Pat;
};

const uint32 SStorage::DefPgDelta = 1024;

SLAPI SStorage::Page::Page(const SStorage & rS, uint pgIndex)
{
	Helper_Construct(rS.GetPageSize(), rS.GetRecSize(), rS.GetFlags(), pgIndex);
}

SLAPI SStorage::Page::Page(uint32 pgSize, uint32 rcSize, uint flags, uint pgIndex)
{
	Helper_Construct(pgSize, rcSize, flags, pgIndex);
}

SLAPI SStorage::Page::~Page()
{
	Destroy();
	DestroyEntries();
}

int SStorage::Page::Add(const void * pData, uint size, SStorage::Address & rA)
{
	int    ok = 1;
	uint   fe_pos = 0;
	THROW(ok = GetFreeEntry(size, &fe_pos));
	if(ok > 0) {
		THROW(UseFreeEntry(fe_pos, pData, size, rA));
	}
	CATCHZOK
	return ok;
}

int SStorage::Page::Remove(const SStorage::Address & rA)
{
	return 0;
}

int SStorage::Page::Get(const Address & rS, void * pData, uint * pSize)
{
	return 0;
}

int SStorage::Page::Helper_Construct(uint32 pgSize, uint32 rcSize, uint flags, uint pgIndex)
{
	Init();
	MapAddress = 0;
	PgSize = pgSize;
	RcSize = rcSize;
	AlignBits = 8;
	Flags  = (uint16)flags;
	PgIndex = pgIndex;
	if(Flags & fFixRcSize) {
		assert(RcSize);
		FixRcCount = 0;
	}
	else {
		P_E = new TSArray <Entry32>;
		Entry32 free_entry(0, PgSize);
		P_E->insert(&free_entry);
	}
	Realloc(RcSize);
	return 1;
}

void SStorage::Page::MakeAddress(uint32 offs, SStorage::Address & rA) const
{
	rA.P = ((uint64)PgIndex << 32) | ((uint64)offs & 0x00000000ffffffff);
}

int SStorage::Page::Realloc(uint reqSize)
{
	int    ok = 1;
	const size_t org_size = Size;
	assert(!(Flags & fFixRcSize) || reqSize == RcSize);
	THROW(!(Flags & fFixRcSize) || reqSize == RcSize);
	if(Flags & fFixPgSize) {
		if(Size == 0) {
			assert(PgSize);
			assert(P_Buf == 0);
			THROW(Alloc(PgSize));
			FreeEntries.clear();
			if(Flags & fFixRcSize) {
				const uint _c = Size / RcSize;
				for(uint i = 0; i < _c; i++) {
					uint32 p = i * RcSize;
					FreeEntries.insert(&p);
				}
				FixRcCount = _c;
			}
			else {
				Entry32 free_entry(0, Size);
				P_E->insert(&free_entry);
			}
		}
		else {
			assert(Size == PgSize);
			ok = -1;
		}
	}
	else {
		if(PgSize == 0 || (Size+reqSize) <= PgSize) {
			size_t new_size = ALIGNSIZE(Size + reqSize, AlignBits);
			if(PgSize) {
				SETMIN(new_size, PgSize);
			}
			THROW(Alloc(new_size));
			if(Flags & fFixRcSize) {
				const uint _c = new_size / RcSize;
				for(uint i = (org_size / RcSize); i < _c; i++) {
					uint32 p = i * RcSize;
					FreeEntries.insert(&p);
				}
			}
			else {
				Entry32 free_entry(org_size, new_size - org_size);
				P_E->insert(&free_entry);
			}
		}
		else
			ok = -1;
	}
	CATCHZOK
	return ok;
}

int SStorage::Page::GetFreeEntry(uint reqSize, uint * pFreeEntryPos)
{
	int    ok = -1;
	uint32 offs = 0;
	uint32 size = 0;
	uint32 min_gap = UINT_MAX;
	uint   fe_pos = 0;
	THROW(!(Flags & fFixRcSize) || reqSize == RcSize);
	for(int try_realloc = 1; ok < 0 && try_realloc; try_realloc = 0) {
		for(uint i = 0; i < FreeEntries.getCount(); i++) {
			uint32 p = FreeEntries.at(i);
			if(Flags & fFixRcSize) {
				offs = p;
				size = RcSize;
				fe_pos = p;
				ok = 1;
				break;
			}
			else {
				const Entry32 & r_entry = P_E->at(p);
				if(r_entry.S >= reqSize && (r_entry.S - reqSize) < min_gap) {
					offs = r_entry.P;
					size = r_entry.S;
					fe_pos = p;
					min_gap = r_entry.S - reqSize;
					ok = 1;
					if(min_gap == 0)
						break;
				}
			}
		}
		if(ok < 0 && try_realloc) {
			int rar = Realloc(reqSize);
			THROW(rar);
			if(rar < 0)
				break;
		}
	}
	ASSIGN_PTR(pFreeEntryPos, fe_pos);
	CATCHZOK
	return ok;
}

int SStorage::Page::UseFreeEntry(uint freeEntryPos, const void * pData, size_t size, SStorage::Address & rAddr)
{
	int    ok = 1;
	uint32 fe_offs = 0;
	uint32 fe_size = 0;
	if(Flags & fFixRcSize) {
		assert(size == RcSize);
		fe_offs = FreeEntries.at(freeEntryPos);
		fe_size = size;
		assert((fe_offs % RcSize) == 0);
		memmove(PTR8(P_Buf)+fe_offs, pData, size);
		FreeEntries.atFree(freeEntryPos);
		MakeAddress(fe_offs, rAddr);
	}
	else {
		const Entry32 & r_entry = P_E->at(FreeEntries.at(freeEntryPos));
		fe_offs = r_entry.P;
		fe_size = r_entry.S;
		assert(fe_size >= size);
		{
			memmove(PTR8(P_Buf)+fe_offs, pData, size);
			Entry32 new_entry(fe_offs, size);
			THROW(P_E->insert(&new_entry));
		}
		FreeEntries.atFree(freeEntryPos);
		if(fe_size > size) {
			Entry32 new_free_entry(r_entry.P + size, fe_size - size);
			uint   new_free_entry_pos = P_E->getCount();
			THROW(P_E->insert(&new_free_entry));
			THROW(FreeEntries.insert(&new_free_entry_pos));
		}
	}
	CATCHZOK
	return ok;
}

void SStorage::Page::DestroyEntries()
{
	if(Flags & fFixRcSize) {
	}
	else {
		ZDELETE(P_E);
	}
}

SStorage::Page::Entry32::Entry32(uint p, uint s) : P(p), S(s)
{
}

SLAPI SStorage::SStorage() : Flags(0), PgSize(0), RcSize(0)
{
}

SLAPI SStorage::~SStorage()
{
}


