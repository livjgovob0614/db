// PPINICFG.CPP
// Copyright (c) A.Sobolev 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2012, 2013, 2015, 2016, 2017, 2018, 2019
// @Kernel
//
#include <pp.h>
#pragma hdrstop
//
// PPIniFile
//

//static
int FASTCALL PPIniFile::GetParamSymb(int idx, SString & rBuf) { return PPLoadText(idx, rBuf.Z()); }
//static
int FASTCALL PPIniFile::GetSectSymb(int idx, SString & rBuf) { return PPLoadText(idx, rBuf.Z()); }

SLAPI PPIniFile::PPIniFile(const char * pFileName, int fcreate, int winCoding, int useIniBuf) :
	SIniFile(pFileName, fcreate, winCoding, useIniBuf)
{
	if(!pFileName)
		Init(getExecPath(TempBuf).SetLastSlash().Cat("PP.INI"), fcreate, winCoding, useIniBuf);
}

SLAPI PPIniFile::PPIniFile() : SIniFile(0, 0, 0, 0)
{
	Init(getExecPath(TempBuf).SetLastSlash().Cat("PP.INI"), 0, 0, 0);
}

int SLAPI PPIniFile::ParamIdToStrings(uint sectId, uint paramId, SString * pSectName, SString * pParam)
{
	if(pSectName) {
		pSectName->Z();
		PPIniFile::GetSectSymb(sectId, *pSectName);
	}
	if(pParam) {
		pParam->Z();
		PPIniFile::GetParamSymb(paramId, *pParam);
	}
	return 1;
}

int SLAPI PPIniFile::Get(uint sectId, uint paramId, SString & rBuf)
{
	SString & r_sect_name = SLS.AcquireRvlStr(); // @v9.9.12
	SString & r_param_name = SLS.AcquireRvlStr(); // @v9.9.12
	ParamIdToStrings(sectId, paramId, &r_sect_name, &r_param_name);
	return GetParam(r_sect_name, r_param_name, rBuf);
}

int SLAPI PPIniFile::Get(uint sectId, const char * pParamName, SString & rBuf)
{
	SString & r_sect_name = SLS.AcquireRvlStr(); // @v9.9.12
	ParamIdToStrings(sectId, 0, &r_sect_name, 0);
	return GetParam(r_sect_name, pParamName, rBuf);
}

int SLAPI PPIniFile::Get(const char * pSectName, uint paramId, SString & rBuf)
{
	SString & r_param_name = SLS.AcquireRvlStr(); // @v9.9.12
	ParamIdToStrings(0, paramId, 0, &r_param_name);
	return GetParam(pSectName, r_param_name, rBuf);
}

int SLAPI PPIniFile::GetInt(uint sectId, uint paramId, int * pVal)
{
	SString & r_sect_name = SLS.AcquireRvlStr(); // @v9.9.12
	SString & r_param_name = SLS.AcquireRvlStr(); // @v9.9.12
	ParamIdToStrings(sectId, paramId, &r_sect_name, &r_param_name);
	return GetIntParam(r_sect_name, r_param_name, pVal);
}

int SLAPI PPIniFile::GetInt(const char * pSectName, uint paramId, int * pVal)
{
	SString & r_param_name = SLS.AcquireRvlStr(); // @v9.9.12
	ParamIdToStrings(0, paramId, 0, &r_param_name);
	return GetIntParam(pSectName, r_param_name, pVal);
}

int SLAPI PPIniFile::GetDataSize(const char * pSectName, uint paramId, int64 * pVal)
{
	SString & r_param_name = SLS.AcquireRvlStr();
	ParamIdToStrings(0, paramId, 0, &r_param_name);
	return GetDataSizeParam(pSectName, r_param_name, pVal);
}

int SLAPI PPIniFile::GetDataSize(uint sectId, uint paramId, int64 * pVal)
{
	SString & r_sect_name = SLS.AcquireRvlStr();
	SString & r_param_name = SLS.AcquireRvlStr();
	ParamIdToStrings(sectId, paramId, &r_sect_name, &r_param_name);
	return GetDataSizeParam(r_sect_name, r_param_name, pVal);
}

int SLAPI PPIniFile::GetEntryList(uint sectId, StringSet * pEntries, int storeAllString)
{
	SString sect_name; // don't use SLS.AcquireRvlStr() (GetEnties uses loop)
	ParamIdToStrings(sectId, 0, &sect_name, 0);
	return SIniFile::GetEntries(sect_name, pEntries, storeAllString);
}

int SLAPI PPIniFile::Append(uint sectId, uint paramId, const char * pVal, int overwrite)
{
	SString sect_name;
	SString param_name;
	ParamIdToStrings(sectId, paramId, &sect_name, &param_name);
	return AppendParam(sect_name, param_name, pVal, overwrite);
}

int SLAPI PPIniFile::IsWinCoding()
{
	int    is_win_coding = BIN(Flags & fWinCoding);
	if(!is_win_coding) {
		int    code_page = 0;
		GetInt(PPINISECT_SYSTEM, PPINIPARAM_CODEPAGE, &code_page);
		is_win_coding = BIN(code_page == 1251);
	}
	return is_win_coding;
}

int SLAPI PPIniFile::Backup(uint maxCopies)
{
	int    ok = 1;
	struct FE { // @flat
		LDATETIME Dtm;
		long   C;
	};
	SString file_name = GetFileName();
	SPathStruc ps(file_name);
	SString new_name, temp_name;
	SString nam = ps.Nam;
	long   c = 0;
	SVector fe_list(sizeof(FE)); // @v9.8.11 SArray-->SVector
	do {
		(ps.Nam = nam).CatChar('-').CatLongZ(++c, 3);
		ps.Merge(temp_name);
		if(fileExists(temp_name)) {
			SFileUtil::Stat st;
			SFileUtil::GetStat(temp_name, &st);
			FE fe;
			fe.Dtm = st.ModTime;
			fe.C = c;
			fe_list.insert(&fe);
		}
		else 
			new_name.SetIfEmpty(temp_name);
	} while(c < 999);
	if(new_name.NotEmpty()) {
		fe_list.sort(PTR_CMPFUNC(LDATETIME));
		if(SCopyFile(file_name, new_name, 0, FILE_SHARE_READ, 0)) {
			if(maxCopies > 0) {
				while(fe_list.getCount() > (maxCopies-1)) {
					(ps.Nam = nam).CatChar('-').CatLongZ(static_cast<const FE *>(fe_list.at(0))->C, 3);
					ps.Merge(new_name);
					SFile::Remove(new_name);
					fe_list.atFree(0);
				}
			}
		}
		else
			ok = 0;
	}
	else
		ok = 0;
	return ok;
}

int SLAPI PPIniFile::UpdateFromFile(const char * pSrcFileName)
{
	int    ok = -1;
	int    backup_done = 0;
	if(fileExists(pSrcFileName)) {
		PPIniFile src_ini(pSrcFileName);
		StringSet sect_set, entry_set;
		SString sect_buf, entry_buf, val_buf;
		THROW_SL(src_ini.GetSections(&sect_set));
		for(uint sp = 0; sect_set.get(&sp, sect_buf);) {
			entry_set.clear();
			THROW(src_ini.GetEntries(sect_buf, &entry_set, 0));
			for(uint ep = 0; entry_set.get(&ep, entry_buf);) {
				src_ini.GetParam(sect_buf, entry_buf, val_buf);
				if(!backup_done) {
					THROW(Backup());
					backup_done = 1;
				}
				THROW(AppendParam(sect_buf, entry_buf, val_buf, 1));
				ok = 1;
			}
		}
	}
	CATCHZOK
	return ok;
}
//
//
//
#if 1 // @construction {

PPConfigDatabase::CObjHeader::CObjHeader() : ID(0), Ver(0), Type(0), Flags(0)
{
}

int PPConfigDatabase::CObjHeader::Cmp(const CObjHeader & rS, long flags) const
{
	int    c = 0;
	if(!(flags & 0x0001)) {
		c = CMPSIGN(ID, rS.ID);
	}
	if(c == 0) {
		CMPCASCADE3(c, this, &rS, Ver, Type, Flags);
		if(c == 0) {
			c = Name.Cmp(rS.Name, 1);
			if(c == 0) {
				c = SubSymb.Cmp(rS.SubSymb, 1);
				if(c == 0) {
					c = DbSymb.Cmp(rS.DbSymb, 1);
					if(c == 0)
						c = OwnerSymb.Cmp(rS.OwnerSymb, 1);
				}
			}
		}
	}
	return c;
}

SLAPI PPConfigDatabase::CObjTbl::CObjTbl(BDbDatabase * pDb) : BDbTable(BDbTable::ConfigHash("ppconfig.db->cobj", 0, 0, 0), pDb), SeqID(0)
{
	//
	// ������ �� �����
	//
	class Idx01 : public SecondaryIndex {
	public:
		virtual int Cb(const BDbTable::Buffer & rKey, const BDbTable::Buffer & rData, BDbTable::Buffer & rResult)
		{
			int32  id = 0;
			rData.Get(&id, sizeof(id));
			rResult.Set(&id, sizeof(id));
			return 0;
		}
	};
	//
	// ������ �� ����
	//
	class Idx02 : public SecondaryIndex {
	public:
		virtual int Cb(const BDbTable::Buffer & rKey, const BDbTable::Buffer & rData, BDbTable::Buffer & rResult)
		{
			SBuffer buf;
			rKey.Get(buf);
			CObjHeader hdr;
			((PPConfigDatabase::CObjTbl *)P_MainT)->SerializeKeyBuf(-1, &hdr, buf);
			rResult = hdr.Type;
			return 0;
		}
	};
	new BDbTable(BDbTable::ConfigHash("ppconfig.db->cobj_idx01", 0, 0, 0), pDb, new Idx01, this);
	new BDbTable(BDbTable::ConfigHash("ppconfig.db->cobj_idx02", cfDup, 0, 0), pDb, new Idx02, this);
	if(P_Db)
		THROW(P_Db->CreateSequence("seq_cobj_id", 0, &SeqID));
	CATCH
		Close();
	ENDCATCH
}

SLAPI PPConfigDatabase::CObjTbl::~CObjTbl()
{
	CALLPTRMEMB(P_Db, CloseSequence(SeqID));
}

//virtual
int PPConfigDatabase::CObjTbl::Implement_Cmp(const BDbTable::Buffer * pKey1, const BDbTable::Buffer * pKey2)
{
	int    c = 0;
	if(pKey1 && pKey2) {
		CObjHeader key1, key2;
		SBuffer buf;
		pKey1->Get(buf);
		SerializeKeyBuf(-1, &key1, buf);
		pKey2->Get(buf);
		SerializeKeyBuf(-1, &key2, buf);
		//c = key1.Cmp(key2, 0x0001);
		c = CMPSIGN(key1.Type, key2.Type);
		if(c == 0) {
			c = key1.Name.Cmp(key2.Name, 1);
			if(c == 0) {
				c = key1.SubSymb.Cmp(key2.SubSymb, 1);
				if(c == 0) {
					c = key1.DbSymb.Cmp(key2.DbSymb, 1);
					if(c == 0) {
						c = key1.OwnerSymb.Cmp(key2.OwnerSymb, 1);
					}
				}
			}
		}
	}
	else if(pKey1 == 0 && pKey2 == 0)
		c = 1; // ����� ������������ ����������� ��������� ������
	return c;
}

int SLAPI PPConfigDatabase::CObjTbl::SerializeKeyBuf(int dir, CObjHeader * pRec, SBuffer & rBuf)
{
	int    ok = 1;
	SSerializeContext * p_sctx = GetSCtx();
	THROW_DB(p_sctx);
	THROW_SL(p_sctx->Serialize(dir, pRec->Ver, rBuf));
	THROW_SL(p_sctx->Serialize(dir, pRec->Type, rBuf));
	THROW_SL(p_sctx->Serialize(dir, pRec->Flags, rBuf));
	THROW_SL(p_sctx->Serialize(dir, pRec->Name, rBuf));
	THROW_SL(p_sctx->Serialize(dir, pRec->SubSymb, rBuf));
	THROW_SL(p_sctx->Serialize(dir, pRec->DbSymb, rBuf));
	THROW_SL(p_sctx->Serialize(dir, pRec->OwnerSymb, rBuf));
	CATCHZOK
	return ok;
}

SLAPI PPConfigDatabase::CObjTailTbl::CObjTailTbl(BDbDatabase * pDb) : BDbTable(BDbTable::ConfigHash("ppconfig.db->cobjtail", 0, 0, 0), pDb)
{
}

SLAPI PPConfigDatabase::CObjTailTbl::~CObjTailTbl()
{
}

SLAPI PPConfigDatabase::PPConfigDatabase(const char * pDbDir) : P_OT(0), P_OtT(0), P_Db(0)
{
	if(pDbDir)
		Open(pDbDir);
}

SLAPI PPConfigDatabase::~PPConfigDatabase()
{
	Close();
}

int SLAPI PPConfigDatabase::Open(const char * pDbPath)
{
	int    ok = 1;
	Close();
	THROW_MEM(P_Db = new BDbDatabase(pDbPath));
	THROW(!!*P_Db);
	THROW_MEM(P_OT = new CObjTbl(P_Db));
	THROW_MEM(P_OtT = new CObjTailTbl(P_Db));
	CATCH
		ok = 0;
		Close();
	ENDCATCH
	return ok;
}

void SLAPI PPConfigDatabase::Close()
{
	ZDELETE(P_OT);
	ZDELETE(P_OtT);
	ZDELETE(P_Db);
}

int SLAPI PPConfigDatabase::GetObjList(int type, const char * pSubSymb, const char * pDbSymb, const char * pOwner, StrAssocArray & rList)
{
	int    ok = 1;
	BDbTable::Buffer key_buf, data_buf;
	SBuffer buf;
	CObjHeader rec;
	key_buf = (int32)type;
	data_buf.Alloc(256);
	BDbCursor curs(*P_OT, 2);
	if(curs.Search(key_buf, data_buf, spEq)) {
		do {
			int32  id = 0;
			buf.Z();
			key_buf.Get(buf);
			P_OT->SerializeKeyBuf(-1, &rec, buf);
			if(rec.Type == type) {
				if((!pDbSymb || rec.DbSymb.Cmp(pDbSymb, 1) == 0) && (!pOwner || rec.OwnerSymb.Cmp(pOwner, 1) == 0) && (!pSubSymb || rec.SubSymb.Cmp(pSubSymb, 1) == 0)) {
					data_buf.Get(&id);
					rList.Add(id, rec.Name);
				}
				key_buf = (int32)type;
			}
			else
				break;
		} while(curs.Search(key_buf, data_buf, spNextDup));
	}
	else
		ok = -1;
	return ok;
}

int SLAPI PPConfigDatabase::DeleteObj(int32 id, int use_ta)
{
	int    ok = 1;
	BDbTable::Buffer key_buf, data_buf;
	BDbTransaction tra(P_Db, use_ta);
	THROW_DB(tra);
	if(id) {
		data_buf.Alloc(1024);
		THROW_PP_S(P_OT->Search(1, key_buf = id, data_buf), PPERR_CDBOBJNFOUND, id);
		THROW_DB(P_OT->DeleteRec(key_buf));
	}
	THROW_DB(tra.Commit(1));
	CATCHZOK
	return ok;
}

int SLAPI PPConfigDatabase::PutObj(int32 * pID, CObjHeader & rHdr, SBuffer & rData, int use_ta)
{
	int    ok = 1;
	BDbTable::Buffer key_buf, data_buf;
	SBuffer buf;
	BDbTransaction tra(P_Db, use_ta);
	THROW_DB(tra);
	if(*pID == 0) {
		P_OT->SerializeKeyBuf(+1, &rHdr, buf.Z());
		key_buf = buf;
		data_buf.Alloc(16);
		if(P_OT->Search(key_buf, data_buf)) {
			data_buf.Get(pID);
		}
	}
	if(*pID) {
		data_buf.Alloc(1024);
		if(P_OT->Search(1, key_buf = *pID, data_buf)) {
			CObjHeader rec;
			key_buf.Get(buf);
			THROW(P_OT->SerializeKeyBuf(-1, &rec, buf));
			if(rHdr.Cmp(rec, 0x0001) != 0) {
				THROW_DB(P_OT->DeleteRec(key_buf));
				{
					P_OT->SerializeKeyBuf(+1, &rHdr, buf.Z());
					key_buf = buf;
					data_buf = *pID;
					THROW_DB(P_OT->InsertRec(key_buf, data_buf));
				}
			}
		}
	}
	else {
		int64 _id = 0;
		THROW_DB(P_Db->GetSequence(P_OT->SeqID, &_id));
		*pID = (int32)_id;
		P_OT->SerializeKeyBuf(+1, &rHdr, buf.Z());
		key_buf = buf;
		data_buf = *pID;
		THROW_DB(P_OT->InsertRec(key_buf, data_buf));
	}
	THROW_DB(P_OtT->InsertRec(key_buf = *pID, data_buf = rData));
	THROW_DB(tra.Commit(1));
	CATCHZOK
	return ok;
}

int SLAPI PPConfigDatabase::GetObj(int32 id, CObjHeader * pHdr, SBuffer * pData)
{
	int    ok = -1;
	BDbTable::Buffer key_buf, data_buf;
	SBuffer buf;
	data_buf.Alloc(4096);
	if(P_OT->Search(1, key_buf = id, data_buf)) {
		ok = 1;
		if(pHdr) {
			key_buf.Get(buf);
			P_OT->SerializeKeyBuf(-1, pHdr, buf);
			pHdr->ID = id;
		}
		if(pData) {
			if(P_OtT->Search(key_buf = id, data_buf)) {
				data_buf.Get(*pData);
				ok = 2;
			}
			else
				ok = 0;
		}
	}
	return ok;
}

#endif // } 0 @construction
