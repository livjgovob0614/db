// SSTRING.CPP
// Copyright (c) A.Sobolev 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019
// @codepage UTF-8
//
#include <slib.h>
#include <tv.h>
#pragma hdrstop
#include <wchar.h>
//
//
//
const char * FASTCALL SUcSwitchW(const wchar_t * pStr)
{
	SString & r_temp_buf = SLS.AcquireRvlStr();
	r_temp_buf.CopyUtf8FromUnicode(pStr, sstrlen(pStr), 1);
	return r_temp_buf.Transf(CTRANSF_UTF8_TO_OUTER);
}

const wchar_t * FASTCALL SUcSwitchW(const char * pStr)
{
	SStringU & r_temp_buf_u = SLS.AcquireRvlStrU();
	return r_temp_buf_u.CopyFromMb_OUTER(pStr, sstrlen(pStr)).ucptr();
}
//
//
//
SRevolver_SString::SRevolver_SString(uint c) : TSRevolver <SString> (c) {}
SString & SRevolver_SString::Get() { return Implement_Get().Z(); }

SRevolver_SStringU::SRevolver_SStringU(uint c) : TSRevolver <SStringU> (c) {}
SStringU & SRevolver_SStringU::Get() { return Implement_Get().Z(); }
//
//
//
SRegExpSet::SRegExpSet() : P_ReQuotedStr(0), P_ReNumber(0), P_ReHex(0), P_ReIdent(0), P_ReDigits(0), P_ReEMail(0), P_ReDate(0)
{
}

SRegExpSet::~SRegExpSet()
{
	ZDELETE(P_ReQuotedStr);
	ZDELETE(P_ReNumber);
	ZDELETE(P_ReHex);
	ZDELETE(P_ReIdent);
	ZDELETE(P_ReDigits);
	ZDELETE(P_ReEMail);
	ZDELETE(P_ReDate);
}

int SRegExpSet::RegisterRe(const char * pRe, long * pHandler)
{
	long   handler = 0;
	CRegExp * p_re = new CRegExp(pRe);
	if(p_re && p_re->IsValid()) {
		ReList.insert(p_re);
		handler = static_cast<long>(ReList.getCount());
	}
	ASSIGN_PTR(pHandler, handler);
	return BIN(handler);
}

int SRegExpSet::InitReNumber()
{
	if(!P_ReNumber) {
		// [0-9]+ означает, что число должно содержать как минимум одну цифру до возможной точки.
		// То есть вариант ".02" не будет восприниматься как число
		P_ReNumber = new CRegExp("^[+-]?[0-9]+([\\.][0-9]*)?([Ee][+-]?[0-9]+)?");
	}
	return P_ReNumber ? 1 : (SLibError = SLERR_NOMEM, 0);
}

int SRegExpSet::InitReHex()
{
	if(!P_ReHex) {
		// [0-9]+ означает, что число должно содержать как минимум одну цифру до возможной точки.
		// То есть вариан ".02" не будет восприниматься как число
		P_ReHex = new CRegExp("^[+-]?0[xX][0-9a-fA-F]+");
	}
	return P_ReHex ? 1 : (SLibError = SLERR_NOMEM, 0);
}

int SRegExpSet::InitReEmail()
{
	if(!P_ReEMail) {
		P_ReEMail = new CRegExp("^[-A-Za-z0-9!#$%&\'*+/=?^_`{|}~]+(\\.[-A-Za-z0-9!#$%&\'*+/=?^_`{|}~]+)*\\@([A-Za-z0-9][-A-Za-z0-9]*\\.)+[A-Za-z]+");
	}
	return P_ReEMail ? 1 : (SLibError = SLERR_NOMEM, 0);
}
//
//
//
SStrScan::SStrScan(const char * pBuf, size_t offs) : SRegExpSet()
{
	Set(pBuf, offs);
}

SStrScan::~SStrScan()
{
}

int FASTCALL SStrScan::Push(uint * pPrevPos)
{
	uint   prev_pos = Stk.getPointer();
	Stk.push(Offs);
	ASSIGN_PTR(pPrevPos, prev_pos);
	return 1;
}

int FASTCALL SStrScan::Pop()
{
	size_t offs = 0;
	if(Stk.pop(offs)) {
		Offs = offs;
		return 1;
	}
	else
		return 0;
}

int FASTCALL SStrScan::Pop(uint prevPos)
{
	int    ok = 1;
	if(Stk.getPointer() > prevPos) {
		do {
			if(!Pop())
				ok = 0;
		} while(ok && Stk.getPointer() > prevPos);
	}
	else
		ok = 0;
	return ok;
}

int FASTCALL SStrScan::IsRe(long reHandler)
{
	if(reHandler > 0 && reHandler <= static_cast<long>(ReList.getCount())) {
		CRegExp * p_re = ReList.at(reHandler-1);
		if(p_re)
			return BIN(p_re->Find(P_Buf+Offs));
	}
	return 0;
}

int SLAPI SStrScan::GetRe(long reHandler, SString & rBuf)
{
	if(reHandler > 0 && reHandler <= static_cast<long>(ReList.getCount())) {
		CRegExp * p_re = ReList.at(reHandler-1);
		if(p_re && p_re->Find(this)) {
			Get(rBuf);
			IncrLen();
			return 1;
		}
	}
	return 0;
}

int FASTCALL SStrScan::Is(const char * pS) const
{
	const size_t len = sstrlen(pS);
	return BIN(len && strnicmp((P_Buf+Offs), pS, len) == 0);
}

int FASTCALL SStrScan::Is(char c) const
{
	return BIN(P_Buf[Offs] == c);
}

int SStrScan::IsLegalUtf8() const
{
	const uint8 * p = reinterpret_cast<const uint8 *>(P_Buf + Offs);
	const size_t extra = SUtfConst::TrailingBytesForUTF8[*p];
	if(extra == 0)
		return 1;
	else {
		//
		// Чтобы не вызывать лишний раз функцию strlen для наиболее частых случаев (extra == 1 и extra == 2)
		// сделаем отдельные ветки алгоритма.
		//
		if(extra == 1) {
			return (p[1] != 0 && SUnicode::IsLegalUtf8(p, 2)) ? 2 : 0;
		}
		else if(extra == 2) {
			return (p[1] != 0 && p[2] != 0 && SUnicode::IsLegalUtf8(p, 3)) ? 3 : 0;
		}
		else {
			const size_t tail = sstrlen(p);
			return ((extra+1) <= tail && SUnicode::IsLegalUtf8(p, extra+1)) ? static_cast<int>(1+extra) : 0;
		}
	}
}

int FASTCALL SStrScan::GetUtf8(SString & rBuf)
{
	rBuf.Z();
	int    n = IsLegalUtf8();
	if(n) {
		rBuf.CatN(P_Buf+Offs, n);
		Offs += n;
	}
	return n;
}

int FASTCALL SStrScan::IsTagBrace() const
{
	return BIN(P_Buf[Offs] == '<');
}

SStrScan & SStrScan::Z()
{
	Offs = 0;
	Len = 0;
	P_Buf = 0;
	return *this;
}

void FASTCALL SStrScan::Set(const char * pBuf, size_t offs)
{
	Offs = offs;
	Len = 0;
	P_Buf = pBuf;
}

SString & FASTCALL SStrScan::Get(SString & rBuf) const
{
	return rBuf.CopyFromN(P_Buf+Offs, Len);
}

int FASTCALL SStrScan::GetQuotedString(SString & rBuf)
{
	SETIFZ(P_ReQuotedStr, new CRegExp("^\"[^\"]*\""));
	if(P_ReQuotedStr->Find(this)) {
		Offs++;   // "
		Len -= 2; // ""
		Get(rBuf);
		Len++; // Убрать завершающую кавычку
		IncrLen();
		return 1;
	}
	else
		return 0;
}

static int FASTCALL _is_eqq_ident_chr(char c)
{
	return BIN((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_');
}

int SLAPI SStrScan::GetEqQ(SString & rKey, SString & rVal)
{
	int    ok = 0;
	const  size_t preserve_offs = Offs;
	rKey.Z();
	rVal.Z();
	SString temp_buf;
	char  c = P_Buf[Offs];
	while(c && _is_eqq_ident_chr(c)) {
		temp_buf.CatChar(c);
		c = P_Buf[++Offs];
	}
	if(temp_buf.NotEmpty()) {
		Skip();
		if(P_Buf[Offs] == '=') {
			Incr(1);
			Skip();
			if(GetQuotedString(rVal)) {
				rKey = temp_buf;
				ok = 1;
			}
		}
	}
	if(!ok)
		Offs = preserve_offs;
	return ok;
}

int SLAPI SStrScan::GetEqN(SString & rKey, double & rVal)
{
	int    ok = 0;
	const  size_t preserve_offs = Offs;
	rKey.Z();
	rVal = 0.0;
	SString temp_buf;
	char  c = P_Buf[Offs];
	while(c && _is_eqq_ident_chr(c)) {
		temp_buf.CatChar(c);
		c = P_Buf[++Offs];
	}
	if(temp_buf.NotEmpty()) {
		Skip();
		if(P_Buf[Offs] == '=') {
			Incr(1);
			Skip();
			SString value_buf;
			if(GetNumber(value_buf)) {
				rKey = temp_buf;
				rVal = value_buf.ToReal();
				ok = 1;
			}
		}
	}
	if(!ok)
		Offs = preserve_offs;
	return ok;
}

int SLAPI SStrScan::Get(const char * pPattern, SString & rBuf)
{
	const size_t len = sstrlen(pPattern);
	if(len && strnicmp((P_Buf+Offs), pPattern, len) == 0) {
		Len = len;
		Get(rBuf);
		IncrLen();
		return 1;
	}
	else
		return 0;
}

int FASTCALL SStrScan::GetIdent(SString & rBuf)
{
	if(!P_ReIdent)
		P_ReIdent = new CRegExp("^[_a-zA-Z][_0-9a-zA-Z]*");
	if(P_ReIdent->Find(this)) {
		Get(rBuf);
		IncrLen();
		return 1;
	}
	else
		return 0;
}

int FASTCALL SStrScan::GetDigits(SString & rBuf)
{
    int    ok = 0;
    if(SETIFZ(P_ReDigits, new CRegExp("^[0-9]+"))) {
        if(P_ReDigits->Find(this)) {
            Get(rBuf);
            IncrLen();
            ok = 1;
        }
    }
    return ok;
}

int SLAPI SStrScan::IsDigits()
{
    int    ok = 0;
    if(SETIFZ(P_ReDigits, new CRegExp("^[0-9]+"))) {
        if(P_ReDigits->Find(P_Buf+Offs))
            ok = 1;
    }
	return ok;
}

int SLAPI SStrScan::IsEnd() const
{
	return (!P_Buf || P_Buf[Offs] == 0);
}

int SLAPI SStrScan::IsNumber()
{
	return BIN(InitReNumber() && P_ReNumber->Find(P_Buf+Offs));
}

int SLAPI SStrScan::IsDotPrefixedNumber()
{
    int    ok = 0;
    const size_t preserve_offs = Offs;
    if(Is('.')) {
        Incr();
        if(IsDigits())
            ok = 1;
    }
    else if(Is("+.")) {
        Incr(2);
        if(IsDigits())
            ok = 1;
    }
    else if(Is("-.")) {
        Incr(2);
        if(IsDigits())
            ok = 1;
    }
    else
        ok = IsNumber();
    Offs = preserve_offs;
    return ok;
}

int FASTCALL SStrScan::GetDotPrefixedNumber(SString & rBuf)
{
    int    ok = 0;
    const size_t preserve_offs = Offs;
	rBuf.Z();
    if(Is('.')) {
        Incr();
        SString & r_temp_buf = SLS.AcquireRvlStr();
        if(GetDigits(r_temp_buf)) {
            rBuf.Cat("0.").Cat(r_temp_buf);
            ok = 1;
        }
    }
    else if(Is("+.")) {
        Incr(2);
		SString & r_temp_buf = SLS.AcquireRvlStr();
        if(GetDigits(r_temp_buf)) {
            rBuf.Cat("0.").Cat(r_temp_buf);
            ok = 1;
        }
    }
    else if(Is("-.")) {
        Incr(2);
		SString & r_temp_buf = SLS.AcquireRvlStr();
        if(GetDigits(r_temp_buf)) {
            rBuf.Cat("-0.").Cat(r_temp_buf);
            ok = 1;
        }
    }
    else
        ok = GetNumber(rBuf);
    if(!ok)
        Offs = preserve_offs;
    return ok;
}

int FASTCALL SStrScan::GetNumber(SString & rBuf)
{
	if(InitReNumber() && P_ReNumber->Find(this)) {
		Get(rBuf);
		IncrLen();
		return 1;
	}
	else
		return 0;
}

int SStrScan::IsHex()
{
	return BIN(InitReHex() && P_ReHex->Find(P_Buf+Offs));
}

uint FASTCALL SStrScan::IsEol(SEOLFormat eolf) const
{
	const char c = P_Buf[Offs];
	if(oneof2(c, '\xA', '\xD')) {
		if(eolf == eolUnix)
			return (c == '\xA') ? 1 : 0;
		else if(eolf == eolMac)
			return (c == '\xD') ? 1 : 0;
		else if(eolf == eolWindows)
			return (c == '\xD' && (P_Buf[Offs+1] == '\xA')) ? 2 : 0;
		else if(eolf == eolUndef)
			return (c == '\xD' && (P_Buf[Offs+1] == '\xA')) ? 2 : 1;
	}
	return 0;
}

int FASTCALL SStrScan::GetEol(SEOLFormat eolf)
{
	uint n = IsEol(eolf);
	if(n) {
		Incr(n);
		return 1;
	}
	else
		return 0;
}

int FASTCALL SStrScan::GetHex(SString & rBuf)
{
	if(InitReHex() && P_ReHex->Find(this)) {
		Get(rBuf);
		IncrLen();
		return 1;
	}
	else
		return 0;
}

int SLAPI SStrScan::GetDate(long datefmt, LDATE & rDate)
{
	rDate = ZERODATE;
	int    ok = 0;
	if(!P_ReDate)
		P_ReDate = new CRegExp("^[0-9]+[\\./\\-][0-9]+[\\./\\-][0-9]+");
	if(P_ReDate->Find(this)) {
		LDATE temp_date = ZERODATE;
		SString temp_buf;
		Get(temp_buf);
		strtodate(temp_buf, datefmt, &temp_date);
		if(checkdate(temp_date)) {
			rDate = temp_date;
			IncrLen();
			ok = 1;
		}
	}
	return ok;
}

int FASTCALL SStrScan::GetEMail(SString & rBuf)
{
	if(InitReEmail() && P_ReEMail->Find(this)) {
		Get(rBuf);
		IncrLen();
		return 1;
	}
	else
		return 0;
}

int SLAPI SStrScan::GetWord(const char * pDiv, SString & rBuf)
{
	const char * p_def_div = " \t\n\r.,;:()[]{}+=^&@!$%/"; // @v8.9.10 append '/'
	SETIFZ(pDiv, p_def_div);
	rBuf.Z();
	size_t p = Offs;
	char   c = P_Buf[p];
	while(c != 0 && !sstrchr(pDiv, c)) {
		c = P_Buf[++p];
	}
	if(p > Offs) {
        Len = p - Offs;
		Get(rBuf);
		IncrLen();
		return 1;
	}
	else
		return 0;
}

int FASTCALL SStrScan::GetTagBrace(SString & rText, int * pKind)
{
	rText.Z();
	int    kind = -1;
	size_t p = Offs;
	char   c = P_Buf[p];
	if(c == '<') {
		c = P_Buf[++p];
		if(c == '/') { // конец тега вида </tag>
			kind = 1;
			c = P_Buf[++p];
		}
		while(!oneof3(c, '/', '>', 0)) {
			rText.CatChar(c);
			c = P_Buf[++p];
		}
		if(kind == -1) {
			if(c == '/' && P_Buf[p+1] == '>') { // учитываем теги вида <tag/>, т.е. без закрывающего тега
				kind = 2;
				p += 2;
			}
			else if(c == '>')
				kind = 0;
			else
				kind = -1; // @error
		}
	}
	ASSIGN_PTR(pKind, kind);
	if(kind >= 0) {
		Len = (p - Offs);
		return 1;
	}
	else
		return 0;
}

size_t FASTCALL SStrScan::SetLen(size_t newLen)
{
	const size_t prev_len = Len;
	Len = newLen;
	return prev_len;
}

size_t FASTCALL SStrScan::Incr(size_t incr)
{
	const size_t prev_offs = Offs;
	Offs += incr;
	return prev_offs;
}

size_t FASTCALL SStrScan::IncrLen(size_t addedIncr /*=0*/)
{
	const size_t prev_offs = Offs;
	Offs += (Len+addedIncr);
	return prev_offs;
}

int FASTCALL SStrScan::IncrChr(int chr)
{
	const char * p = *this;
	if(p[0] == chr) {
		Incr(1);
		return 1;
	}
	else
		return 0;
}

int FASTCALL SStrScan::SearchChar(int c)
{
	if(P_Buf)
		for(size_t p = Offs; P_Buf[p]; p++)
			if(P_Buf[p] == c) {
				Len = p-Offs;
				return 1;
			}
	return 0;
}

int FASTCALL SStrScan::Search(const char * pPattern)
{
	if(P_Buf) {
		size_t len = sstrlen(pPattern);
		const  char * p_buf = P_Buf+Offs;
		const  char * p = strstr(p_buf, pPattern);
		if(p) {
			Len = (p-p_buf);
			return 1;
		}
	}
	return 0;
}

SStrScan & SLAPI SStrScan::Skip()
{
	if(P_Buf) {
		const char * p = (P_Buf+Offs);
		if(oneof2(*p, ' ', '\t')) {
			do {
				p++;
			} while(oneof2(*p, ' ', '\t'));
			Offs += (p - (P_Buf + Offs));
		}
	}
	return *this;
}

SStrScan & FASTCALL SStrScan::Skip(int ws)
{
	if(P_Buf && ws) {
		size_t i = 0;
		char   buf[16];
		memzero(buf, sizeof(buf));
		if(ws & wsSpace)
			buf[i++] = ' ';
		if(ws & wsTab)
			buf[i++] = '\t';
		if(ws & wsNewLine) {
			buf[i++] = '\xD'; //'\n';
			buf[i++] = '\xA';
		}
		if(ws & wsComma)
			buf[i++] = ',';
		if(ws & wsSemicol)
			buf[i++] = ';';
		const char * p = (P_Buf+Offs);
		while(memchr(buf, *p, i)) {
			if(ws & wsNewLine && *p == '\xD' && p[1] == '\xA')
				p++;
			p++;
		}
		Offs += (p - (P_Buf + Offs));
	}
	return *this;
}

static size_t FASTCALL _MakeWhitespaceCharSet(int ws, char * pBuf)
{
	size_t i = 0;
	if(ws & SStrScan::wsSpace)
		pBuf[i++] = ' ';
	if(ws & SStrScan::wsTab)
		pBuf[i++] = '\t';
	if(ws & SStrScan::wsNewLine) {
		pBuf[i++] = '\xD'; //'\n';
		pBuf[i++] = '\xA';
	}
	if(ws & SStrScan::wsComma)
		pBuf[i++] = ',';
	if(ws & SStrScan::wsSemicol)
		pBuf[i++] = ';';
	return i;
}

int FASTCALL SStrScan::IsSpace(int ws) const
{
	int    yes = 0;
	if(P_Buf && ws) {
		char   buf[16];
		memzero(buf, sizeof(buf));
		size_t i = _MakeWhitespaceCharSet(ws, buf);
		const char * p = (P_Buf+Offs);
		if(memchr(buf, *p, i)) {
			yes = 1;
		}
	}
	return yes;
}

SStrScan & SLAPI SStrScan::Skip(int ws, uint * pLineCount)
{
	uint   line_count = 0;
	if(P_Buf && ws) {
		char   buf[16];
		memzero(buf, sizeof(buf));
		size_t i = _MakeWhitespaceCharSet(ws, buf);
		const char * p = (P_Buf+Offs);
		while(memchr(buf, *p, i)) {
			if(*p == '\xD' && p[1] == '\xA') {
				p++;
				line_count++;
			}
			else if(oneof2(*p, '\xD', '\xA'))
				line_count++;
			p++;
		}
		Offs += (p - (P_Buf + Offs));
	}
	ASSIGN_PTR(pLineCount, line_count);
	return *this;
}

SStrScan & SStrScan::SkipOptionalDiv(int div, int skipWs)
{
	if(Skip(skipWs)[0] == div) {
		Incr();
		Skip();
	}
	return *this;
}
//
//
//
SStringTag::SStringTag() : SString(), Id(0)
{
}
//
//
//
IMPL_INVARIANT_C(SString)
{
	S_INVARIANT_PROLOG(pInvP);
	S_ASSERT_P(Size >= L, pInvP);
	if(P_Buf) {
		S_ASSERT_P(Len() == sstrlen(P_Buf), pInvP);
	}
	else {
		S_ASSERT_P(Size == 0, pInvP);
		S_ASSERT_P(L  == 0, pInvP);
	}
	S_INVARIANT_EPILOG(pInvP);
}

SLAPI SString::SString() : L(0), Size(0), P_Buf(0)
{
}

SLAPI SString::SString(size_t initSize) : L(0), Size(0), P_Buf(0)
{
	if(initSize)
		Alloc(initSize);
}

SLAPI SString::SString(const char * pS) : L(0), Size(0), P_Buf(0)
{
	CopyFrom(pS);
}

SLAPI SString::SString(const SString & s) : L(0), Size(0), P_Buf(0)
{
	CopyFrom(s);
}

SLAPI SString::~SString()
{
	L = Size = 0;
	//
	// Обнуление членов класса осуществляется для того, чтобы
	// при ссылке на разрушенный экземпляр объекта не возникало исключения.
	//
	if(P_Buf) // @speedcritical
		ZFREE(P_Buf);
}

void SLAPI SString::Destroy()
{
	L = Size = 0;
	ZFREE(P_Buf);
}
//
// trivial functions {
//
// (inlined) size_t SLAPI SString::Len() const { return L ? (L-1) : 0; }
size_t SLAPI SString::Len() const { return L ? (L-1) : 0; }
size_t SLAPI SString::BufSize() const { return Size; }
int    FASTCALL SString::C(size_t n) const { return (n < Len()) ? P_Buf[n] : 0; }
int    SLAPI SString::Single() const { return (L == 2) ? P_Buf[0] : 0; }
SString & FASTCALL SString::operator = (const SString & s) { return CopyFrom(s); }
SString & FASTCALL SString::operator = (const char * pS) { return CopyFrom(pS); }
SString & FASTCALL SString::Set(const uchar * pS) { return CopyFrom(reinterpret_cast<const char *>(pS)); }
int    FASTCALL SString::operator == (const char * pS) const { return IsEqual(pS); }
int    FASTCALL SString::operator != (const char * pS) const { return BIN(!IsEqual(pS)); }
int    FASTCALL SString::operator == (const SString & rS) const { return IsEqual(rS); }
int    FASTCALL SString::operator != (const SString & rS) const { return BIN(!IsEqual(rS)); }
int    FASTCALL SString::IsEqNC(const SString & rS) const { return (CmpNC(rS) == 0); }
int    FASTCALL SString::IsEqNC(const char * pS) const { return (CmpNC(pS) == 0); }
int    SLAPI SString::Last() const { return (L > 1) ? P_Buf[L-2] : 0; }
int    SLAPI SString::Empty() const { return BIN(Len() == 0); }
int    SLAPI SString::NotEmpty() const { return BIN(Len()); }
int    SLAPI SString::NotEmptyS() { return Strip().NotEmpty(); }
SString & FASTCALL SString::Tab(uint c) { return (oneof2(c, 1, 0) || c > 1000) ? CatChar('\t') : CatCharN('\t', c); }
SString & SLAPI SString::Tab()     { return CatChar('\t'); }
SString & SLAPI SString::Space()   { return CatChar(' ');  }
SString & SLAPI SString::Dot()     { return CatChar('.');  }
SString & SLAPI SString::Comma()   { return CatChar(',');  }
SString & SLAPI SString::Semicol() { return CatChar(';');  }
SString & SLAPI SString::Eq()      { return CatChar('=');  }
SString & SLAPI SString::CR()      { return CatChar('\n'); }
SString & SLAPI SString::CRB()     { return CatChar('\xD').CatChar('\xA'); }
SString & FASTCALL SString::CatQStr(const char * pStr) { return CatChar('\"').Cat(pStr).CatChar('\"'); }
SString & FASTCALL SString::CatParStr(const char * pStr) { return CatChar('(').Cat(pStr).CatChar(')'); }
SString & FASTCALL SString::CatParStr(long val) { return CatChar('(').Cat(val).CatChar(')'); }
SString & FASTCALL SString::CatBrackStr(const char * pStr) { return CatChar('[').Cat(pStr).CatChar(']'); }
SString & FASTCALL SString::CatLongZ(int    val, uint numDigits) { return CatLongZ(static_cast<long>(val), numDigits); }
SString & FASTCALL SString::CatLongZ(uint   val, uint numDigits) { return CatLongZ(static_cast<long>(val), numDigits); }
SString & FASTCALL SString::CatLongZ(uint32 val, uint numDigits) { return CatLongZ(static_cast<long>(val), numDigits); }
SString & FASTCALL SString::SetInt(int val) { return Z().Cat(val); }
SString & SLAPI SString::ToUtf8() { return Helper_MbToMb(CP_ACP, CP_UTF8); }
SString & SLAPI SString::Utf8ToChar() { return Helper_MbToMb(CP_UTF8, CP_ACP); }
SString & SLAPI SString::Utf8ToOem() { return Helper_MbToMb(CP_UTF8, CP_OEMCP); }
SString & FASTCALL SString::Utf8ToCp(SCodepageIdent cp) { return Helper_MbToMb(CP_UTF8, static_cast<int>(cp)); }
SString & SLAPI SString::CatEq(const char * pKey,  const char * pVal) { return Cat(pKey).CatChar('=').Cat(pVal); }
SString & SLAPI SString::CatEqQ(const char * pKey, const char * pVal) { return Cat(pKey).CatChar('=').CatChar('\"').Cat(pVal).CatChar('\"'); }
SString & SLAPI SString::CatEq(const char * pKey, uint16 val) { return Cat(pKey).CatChar('=').Cat(val); }
SString & SLAPI SString::CatEq(const char * pKey, uint val)   { return Cat(pKey).CatChar('=').Cat(val); }
SString & SLAPI SString::CatEq(const char * pKey, long val)   { return Cat(pKey).CatChar('=').Cat(val); }
SString & SLAPI SString::CatEq(const char * pKey, ulong val)  { return Cat(pKey).CatChar('=').Cat(val); }
SString & SLAPI SString::CatEq(const char * pKey, int64 val)  { return Cat(pKey).CatChar('=').Cat(val); }
SString & SLAPI SString::CatEq(const char * pKey, uint64 val) { return Cat(pKey).CatChar('=').Cat(val); }
SString & SLAPI SString::CatEq(const char * pKey, double val, long fmt) { return Cat(pKey).CatChar('=').Cat(val, fmt); }
SString & SLAPI SString::CatEq(const char * pKey, LTIME val,  long fmt) { return Cat(pKey).CatChar('=').Cat(val, fmt); }
SString & SLAPI SString::CatEq(const char * pKey, LDATE val,  long fmt) { return Cat(pKey).CatChar('=').Cat(val, fmt); }
SString & SLAPI SString::CatEq(const char * pKey, LDATETIME val, long dtFmt, long tmFmt) { return Cat(pKey).CatChar('=').Cat(val, dtFmt, tmFmt); }
// } trivial functions

void SLAPI SString::Obfuscate()
{
	if(Size && P_Buf) {
		SlThreadLocalArea & r_tla = SLS.GetTLA();
		r_tla.Rg.ObfuscateBuffer(P_Buf, Size);
		const char * p_zero = static_cast<const char *>(memchr(P_Buf, '\0', Size));
		if(p_zero) {
			L = (p_zero - P_Buf);
		}
		else
			P_Buf[L-1] = 0;
	}
}

const char * SLAPI SString::SearchCharPos(size_t startPos, int c, size_t * pPos) const
{
	size_t pos = 0;
	const  char * p = 0;
	if(L > (startPos+1)) {
		p = static_cast<const char *>(memchr(P_Buf+startPos, static_cast<uchar>(c), Len()-startPos));
		if(p)
			pos = static_cast<size_t>(p - P_Buf);
	}
	ASSIGN_PTR(pPos, pos);
	return p;
}

const char * SLAPI SString::SearchChar(int c, size_t * pPos) const
{
	/* @v10.3.11
	size_t pos = 0;
	const  char * p = 0;
	if(L) {
		p = static_cast<const char *>(memchr(P_Buf, static_cast<uchar>(c), Len()));
		if(p)
			pos = static_cast<size_t>(p - P_Buf);
	}
	ASSIGN_PTR(pPos, pos);
	return p;
	*/
	return SearchCharPos(0, c, pPos); // @v10.3.11
}

int FASTCALL SString::HasChr(int c) const
{
	if(c == 0)
		return 0;
	else {
		const char * p_buf = P_Buf;
		switch(L) {
			case 0:
			case 1:  return 0;
			case 2:  return (p_buf[0] == c);
			case 3:  return (p_buf[0] == c || p_buf[1] == c);
			case 4:  return (p_buf[0] == c || p_buf[1] == c || p_buf[2] == c);
			case 5:  return (p_buf[0] == c || p_buf[1] == c || p_buf[2] == c || p_buf[3] == c);
			default: return BIN(memchr(p_buf, static_cast<uchar>(c), L-1));
		}
	}
}

int SLAPI SString::IsLatin() const
{
	int    ok = 1;
	for(uint i = 0; ok && i < Len(); i++) {
		int symb = C(i);
		if(!((symb >= '0' && symb <= '9') || (symb >= 'a' && symb <= 'z') || (symb >=  'A' && symb <= 'Z')))
			ok = 0;
	}
	return ok;
}

int SLAPI SString::GetWord(size_t * pPos, SString & rBuf) const
{
	size_t pos = DEREFPTRORZ(pPos);
	rBuf.Z();
	while(pos < Len()) {
		char c = C(pos);
		if(isalnum(c) || c == '_') {
			rBuf.CatChar(c);
			pos++;
		}
		else
			break;
	}
	ASSIGN_PTR(pPos, pos);
	return static_cast<int>(rBuf.Len());
}

int SLAPI SString::Tokenize(const char * pDelimChrSet, StringSet & rResult) const
{
	int    ok = 1;
	const size_t len = Len();
	if(len) {
		SETIFZ(pDelimChrSet, " \t\n\r");
		const size_t delim_len = sstrlen(pDelimChrSet);
		if(delim_len == 0)
			rResult.add(*this);
		else {
			SString & r_temp_buf = SLS.AcquireRvlStr(); // @v9.9.5
			uint   i = 0;
			if(delim_len == 1) { // @speedcritical {
				const char delim = pDelimChrSet[0];
				do {
					r_temp_buf.Z();
					while(i < len && P_Buf[i] != delim)
						r_temp_buf.CatChar(P_Buf[i++]);
					if(r_temp_buf.NotEmpty()) {
						rResult.add(r_temp_buf);
						ok = 2;
					}
					while(i < len && P_Buf[i] == delim)
						i++;
				} while(i < len);
			}
			else { // } @speedcritical
				do {
					r_temp_buf.Z();
					while(i < len && !memchr(pDelimChrSet, P_Buf[i], delim_len))
						r_temp_buf.CatChar(P_Buf[i++]);
					if(r_temp_buf.NotEmpty()) {
						rResult.add(r_temp_buf);
						ok = 2;
					}
					while(i < len && memchr(pDelimChrSet, P_Buf[i], delim_len))
						i++;
				} while(i < len);
			}
		}
	}
	else
		ok = -1;
	return ok;
}

#ifndef _WIN32_WCE // {

int SLAPI SString::Search(const SSrchPattern * pBlk, size_t startPos, size_t * pPos) const
{
	return (pBlk && startPos < Len()) ? pBlk->Search(P_Buf, startPos, Len(), pPos) : 0;
}

int SLAPI SString::Search(const char * pPattern, size_t startPos, int ignoreCase, size_t * pPos) const
{
	int    ok = 0;
	const size_t _len = Len();
	if(startPos < _len) {
		const size_t _pat_len = sstrlen(pPattern);
		if(_pat_len) {
			if(_pat_len == 1 && !ignoreCase) {
				const char * p = static_cast<const char *>(memchr(P_Buf+startPos, pPattern[0], _len - startPos));
				if(p) {
					ASSIGN_PTR(pPos, (p - P_Buf));
					ok = 1;
				}
			}
			else if((_len - startPos) <= 1024 && _pat_len <= 8) {
				const char * p = 0;
				if(ignoreCase) {
					p = stristr866(P_Buf+startPos, pPattern);
				}
				else {
                    p = strstr(P_Buf+startPos, pPattern);
				}
				if(p) {
					ASSIGN_PTR(pPos, (p - P_Buf));
					ok = 1;
				}
			}
			else {
				SSrchPattern blk(pPattern, ignoreCase ? SSrchPattern::fNoCase : 0, SSrchPattern::algBmGoodSfx);
				ok = blk.Search(P_Buf, startPos, _len, pPos);
			}
		}
	}
	return ok;
}

#endif // }

int SLAPI SString::Divide(int divChr, SString & rLeft, SString & rRight) const
{
	rLeft.Z();
	rRight.Z();

	int    ok = 0;
	size_t pos = 0;
	const char * p = SearchChar(divChr, &pos);
	if(p) {
		Sub(0, pos, rLeft);
		Sub(pos+1, UINT_MAX, rRight);
		ok = 1;
	}
	else {
		rLeft = *this;
		ok = -1;
	}
	return ok;
}

int SLAPI SString::Wrap(uint maxLen, SString & rHead, SString & rTail) const
{
	int    ok = 1;
	size_t len = Len();
	if(len > 0) {
		size_t p = maxLen;
		if(p > 0 && p < len) {
			size_t temp_pos = p;
			size_t next_pos = p;
			while(P_Buf[temp_pos] != ' ')
				if(temp_pos)
					temp_pos--;
				else
					break;
			if(temp_pos) {
				p = temp_pos;
				next_pos = temp_pos+1;
			}
			rHead.CopyFromN(P_Buf, p);
			rTail.CopyFrom(P_Buf+next_pos);
		}
		else {
			rHead = *this;
			rTail.Z();
			ok = -1;
		}
	}
	else {
		rHead.Z();
		rTail.Z();
		ok = 0;
	}
	return ok;
}
//
// Compute the snapped size for a given requested size.  By snapping to powers
// of 2 like this, repeated reallocations are avoided.
//
size_t FASTCALL SnapUpSize(size_t i)
{
	if(i < 12)
		i = 12;
	else {
		size_t j = i;
		// Assumes your system is at least 32 bits, and your string is
		// at most 4GB is size
		j |= (j >>  1);
		j |= (j >>  2);
		j |= (j >>  4);
		j |= (j >>  8);
		j |= (j >> 16);
		i = j + 1; // Least power of two greater than i
	}
	return i;
}

int FASTCALL SString::Alloc(size_t sz)
{
	int    ok = 1;
	if(sz == 0) {
		Trim(0);
	}
	else if(sz > Size) {
		size_t new_size = SnapUpSize(sz);
		char * p = 0;
		if((7 * Size) < (8 * L)) { // Assume probability of a non-moving realloc is 0.125
			// If L is close to Size in size then use realloc to reduce the memory defragmentation
			p = static_cast<char *>(SAlloc::R(P_Buf, new_size));
		}
		else {
			// If L is not close to Size then avoid the penalty of copying
			// the extra bytes that are allocated, but not considered part of the string
			p = static_cast<char *>(SAlloc::M(new_size));
			if(!p)
				p = static_cast<char *>(SAlloc::R(P_Buf, new_size));
			else {
				if(L)
					memcpy(p, P_Buf, L);
				SAlloc::F(P_Buf);
			}
		}
		if(p) {
			Size = new_size;
			P_Buf = p;
			// @v9.4.9 @fix (big mistake) P_Buf[L] = 0;
			// @v9.4.9 {
			if(L)
				P_Buf[L-1] = 0;
			else {
				// @v9.9.5 P_Buf[0] = 0;
				assert(Size >= 4);
				PTR32(P_Buf)[0] = 0; // @speedcritical @v9.9.5
			}
			// } @v9.4.9
		}
		else
			ok = 0;
	}
	return ok;
}

SString & FASTCALL SString::CopyFrom(const SString & s)
{
	const size_t sl = s.L;
	if(Alloc(sl) && sl) {
		memcpy(P_Buf, s.P_Buf, sl);
		L = sl;
	}
	return *this;
}

SString & FASTCALL SString::CopyFromN(const char * pS, size_t maxLen)
{
	size_t new_len = 1;
	if(pS) {
		if(maxLen) {
			const char * p_zero = static_cast<const char *>(memchr(pS, 0, maxLen));
			new_len += p_zero ? (p_zero - pS) : maxLen;
		}
	}
	if(Alloc(new_len)) {
		if(pS && new_len > 1)
			memcpy(P_Buf, pS, new_len-1);
		P_Buf[new_len-1] = 0;
		L = new_len;
	}
	return *this;
}

SString & FASTCALL SString::CopyFrom(const char * pS)
{
	const size_t new_len = pS ? (sstrlen(pS) + 1) : 1;
	if(Alloc(new_len)) {
		if(pS) {
			memcpy(P_Buf, pS, new_len);
			L = new_len;
		}
		else {
			P_Buf[0] = 0;
			L = 1;
		}
	}
	return *this;
}

SString & FASTCALL SString::SetIfEmpty(const char * pS)
{
	if(Empty())
		CopyFrom(pS);
	return *this;
}

SString & FASTCALL SString::SetIfEmpty(const SString & rS)
{
	if(Empty())
		CopyFrom(rS);
	return *this;
}

int SLAPI SString::GetSubFrom(const char * pStr, int div, int idx)
{
	uint   pos = 0;
	StringSet ss(div, pStr);
	for(int i = 0; ss.get(&pos, *this); i++)
		if(i == idx)
			return 1;
	Z();
	return 0;
}

int  SLAPI SString::GetIdxBySub(const char * pSubStr, int div)
{
	int    idx = -1;
	uint   pos = 0;
	SString buf;
	StringSet ss(div, P_Buf);
	for(int i = 0; idx == -1 && ss.get(&pos, buf); i++)
		if(buf.CmpNC(pSubStr) == 0)
			idx = i;
	return idx;
}

char * FASTCALL SString::CopyTo(char * pS, size_t bufLen) const
{
	// @v9.6.3 return strnzcpy(pS, P_Buf, bufLen);
	/* @construction @todo (такой вариант предпочтительнее поскольку функция критична по быстродействию, но нужен тест) */
	// @v9.6.3 (test has been done) {
	if(pS) {
		const size_t src_len = Len();
		if(src_len) {
			if(bufLen) {
				if(src_len < bufLen)
					memcpy(pS, P_Buf, src_len+1);
				else {
					memcpy(pS, P_Buf, bufLen-1);
					pS[bufLen-1] = 0;
				}
			}
			else {
				// @v10.3.1 strcpy(pS, P_Buf);
				memcpy(pS, P_Buf, src_len+1); // @v10.3.1
			}
		}
		else
			pS[0] = 0;
	}
	return pS;
	// @v9.6.3 {
}

BSTR FASTCALL SString::CopyToOleStr(BSTR * pBuf) const
{
	size_t wbuflen = Len()+1;
	WCHAR  wname_stk_buf[256];
	WCHAR * p_wname = (wbuflen > SIZEOFARRAY(wname_stk_buf)) ? static_cast<WCHAR *>(SAlloc::M(wbuflen * sizeof(WCHAR))) : wname_stk_buf;
	if(p_wname) {
		p_wname[0] = 0;
#ifndef _WIN32_WCE // {
		if(pBuf) {
			if(*pBuf)
				SysFreeString(*pBuf);
			MultiByteToWideChar(CP_OEMCP, 0, P_Buf, static_cast<int>(Len()), p_wname, static_cast<int>(wbuflen));
			p_wname[wbuflen-1] = 0;
			*pBuf = SysAllocString(p_wname);
		}
#endif // } _WIN32_WCE
	}
	if(p_wname != wname_stk_buf)
		SAlloc::F(p_wname);
	return *pBuf;
}

SString & FASTCALL SString::CopyFromOleStr(const BSTR s)
{
#ifndef _WIN32_WCE // {
	size_t len = SysStringLen(s);
	size_t new_len = len+1;
	if(Alloc(new_len)) {
		if(len) {
			WideCharToMultiByte(CP_OEMCP, 0, s, -1, P_Buf, static_cast<int>(new_len), 0, 0);
			L = new_len;
			P_Buf[new_len-1] = 0;
		}
		else {
			P_Buf[0] = 0;
			L = 1;
		}
	}
#endif // } _WIN32_WCE
	return *this;
}

SString & SLAPI SString::Z()
{
	//
	// Функция вызывается экстремально часто. Потому максимально оптимизирована.
	//
	if(sizeof(uint32) <= Size || Alloc(sizeof(uint32))) { // 4 байта быстрее обнуляются, чем 1 поэтому требуем минимальную длину - 4 символа (4 байта)
		*PTR32(P_Buf) = 0;
		L = 1;
	}
	else if(P_Buf) {
		P_Buf[0] = 0;
		L = 1;
	}
	else
		L = 0;
	return *this;
}

SString & SLAPI SString::Quot(int leftQuotChar, int rightQuotChar)
{
	ShiftRight(1, leftQuotChar);
	return CatChar(rightQuotChar);
}

SString & SLAPI SString::ReplaceChar(int patternChr, int replaceChr)
{
	/* @todo
	if(L > 1) {
		char * p = P_Buf;
		size_t s = L-1;
		while((p = memchr(p, patternChr, s)) != 0) {
			*p++ = replaceChr;
			s -= (p - P_Buf);
		}
	}
	*/
	if(L > 1)
		for(size_t i = 0; i < L-1; i++)
			if(P_Buf[i] == patternChr)
				P_Buf[i] = replaceChr;
	return *this;
}

int SLAPI SString::ReplaceStrR(const char * pPattern, const char * pReplacer, int once)
{
	int    count = 0;
	const size_t patt_len = sstrlen(pPattern);
	if(patt_len) {
		const size_t rep_len = sstrlen(pReplacer);
		size_t pos = 0;
		while(Search(pPattern, pos, 0, &pos) > 0) {
			Excise(pos, patt_len);
			if(rep_len) {
				Insert(pos, pReplacer);
				pos += rep_len;
				count++;
			}
			if(once)
				break;
		}
	}
	return count;
}

SString & SLAPI SString::ReplaceStr(const char * pPattern, const char * pReplacer, int once)
{
	ReplaceStrR(pPattern, pReplacer, once);
	return *this;
}

struct _SpcSymbEntry {
	char   chr;
	const  char * str;
};

static _SpcSymbEntry HtmlSpcSymbTab[] = {
	{ '\x22', "&quot;"   }, // "
	{ '\x26', "&amp;"    }, // &
	{ '\x3c', "&lt;"     }, // <
	{ '\x3e', "&gt;"     }, // >
	{ '\x23', "&#035;"   }  // #
};

static _SpcSymbEntry LatexSpcSymbTab[] = {
	{ '\x25', "\\%"   }, // %
	{ '\x26', "\\&"   }, // &
	{ '\x23', "\\#"   }, // #
	{ '_', "\\_"   }, // _
	{ '$', "\\$"   },  // $
	{ '\\', "\\textbackslash" } // '\'
};

SString & FASTCALL SString::ReplaceSpecSymb(int fileFormat)
{
	if(P_Buf && L > 1) {
		const _SpcSymbEntry * p_tab = 0;
		size_t tab_size = 0;
		if(fileFormat == SFileFormat::Html) {
			p_tab = HtmlSpcSymbTab;
			tab_size = SIZEOFARRAY(HtmlSpcSymbTab);
		}
		else if(fileFormat == SFileFormat::Latex) {
			p_tab = LatexSpcSymbTab;
			tab_size = SIZEOFARRAY(LatexSpcSymbTab);
		}
		if(p_tab && tab_size) {
			SString temp_buf;
			const size_t len = Len();
			for(uint i = 0; i < len; i++) {
				const char c = P_Buf[i];
				int   s = 0;
				for(uint j = 0; j < tab_size; j++) {
					if(c == p_tab[j].chr) {
						temp_buf.Cat(p_tab[j].str);
						s = 1;
						break;
					}
				}
				if(!s)
					temp_buf.CatChar(c);
			}
			CopyFrom(temp_buf);
		}
	}
	return *this;
}

SString & FASTCALL SString::RevertSpecSymb(int fileFormat)
{
	if(P_Buf && L > 1) {
		const _SpcSymbEntry * p_tab = 0;
		size_t tab_size = 0;
		if(fileFormat == SFileFormat::Html) {
			p_tab = HtmlSpcSymbTab;
			tab_size = SIZEOFARRAY(HtmlSpcSymbTab);
		}
		else if(fileFormat == SFileFormat::Latex) {
			p_tab = LatexSpcSymbTab;
			tab_size = SIZEOFARRAY(LatexSpcSymbTab);
		}
		if(p_tab && tab_size) {
			char   first_special_symb = 0;
			{
				for(uint ei = 0; ei < tab_size; ei++) {
					if(!ei)
						first_special_symb = p_tab[ei].str[0];
					else if(p_tab[ei].str[0] != first_special_symb) {
						first_special_symb = 0;
						break;
					}
				}
			}
			if(!first_special_symb || HasChr(first_special_symb)) {
				SString temp_buf;
				const size_t len = Len();
				for(size_t i = 0; i < len; i++) {
					const char c = P_Buf[i];
					int   s = 0;
					if(!first_special_symb || c == first_special_symb) {
						for(uint j = 0; j < tab_size; j++) {
							const size_t ent_len = sstrlen(p_tab[j].str);
							if(memcmp(p_tab[j].str, P_Buf+i, ent_len) == 0) {
								temp_buf.CatChar(p_tab[j].chr);
								i += (ent_len-1);
								s = 1;
								break;
							}
						}
					}
					if(!s)
						temp_buf.CatChar(c);
				}
				CopyFrom(temp_buf);
			}
		}
	}
	return *this;
}

#if 0 // @v8.8.3 {
SString & SLAPI SString::ReplaceHtmlSpecSymb()
{
	if(P_Buf && L > 1) {
		SString temp_buf;
		const size_t len = Len();
		for(uint i = 0; i < len; i++) {
			const char c = P_Buf[i];
			int   s = 0;
			for(uint j = 0; j < SIZEOFARRAY(HtmlSpcSymbTab); j++) {
				if(c == HtmlSpcSymbTab[j].chr) {
					temp_buf.CatChar('&').Cat(HtmlSpcSymbTab[j].str).Semicol();
					s = 1;
					break;
				}
			}
			if(!s)
				temp_buf.CatChar(c);
		}
		CopyFrom(temp_buf);
	}
	return *this;
}

SString & SLAPI SString::RevertHtmlSpecSymb()
{
	if(P_Buf && L > 1) {
		SString temp_buf, ent_buf;
		const size_t len = Len();
		for(uint i = 0; i < len; i++) {
			const char c = P_Buf[i];
			int   s = 0;
			if(c == '&') {
				for(uint j = 0; j < SIZEOFARRAY(HtmlSpcSymbTab); j++) {
					ent_buf.Z().CatChar('&').Cat(HtmlSpcSymbTab[j].str).Semicol();
					if(memcmp(ent_buf, P_Buf+i, ent_buf.Len()) == 0) {
						temp_buf.CatChar(HtmlSpcSymbTab[j].chr);
						i += (ent_buf.Len()-1);
						s = 1;
						break;
					}
				}
			}
			if(!s)
				temp_buf.CatChar(c);
		}
		CopyFrom(temp_buf);
	}
	return *this;
}
#endif // } 0 @v8.8.3

SString & SLAPI SString::ReplaceCR()
{
	if(P_Buf && L > 1) {
		size_t len = Len();
		for(size_t i = 0; i < len; i++) {
			if((P_Buf[i] == '\r' && P_Buf[i+1] == '\n') || (P_Buf[i] == '\n' && P_Buf[i+1] == '\r')) {
				if(len <= (i+2)) {
					P_Buf[i] = 0;
					len -= 2;
				}
				else {
					memcpy(P_Buf+i, P_Buf+i+1, len-i-2);
					P_Buf[i] = ' ';
					len--;
				}
			}
			else if(P_Buf[i] == '\xA') {
				P_Buf[i] = ' ';
			}
		}
		L = len+1;
		P_Buf[len] = 0;
	}
	return *this;
}

SString & SLAPI SString::Escape()
{
	const size_t _len = Len();
	if(_len) {
		int    do_replace = 0;
		size_t i;
		for(i = 0; !do_replace && i < _len; i++) {
			const char c = P_Buf[i];
			switch(c) {
				case '\\':
				case '\"':
				case '/':
				case '\b':
				case '\f':
				case '\n':
				case '\r':
				case '\t':
					do_replace = 1;
					break;
				default:
					if(c < 0) { // non-BMP character
						;
					}
					else if(c < 0x20)
						do_replace = 1;
					else {
						;
					}
					break;
			}
		}
		if(do_replace) {
			for(i = 0; i < _len; i++) {
				const char c = P_Buf[i];
				switch(c) {
					case '\\': CatCharN(c, 2); break;
					case '\"': Cat("\\\""); break;
					case '/':  Cat("\\/"); break;
					case '\b': Cat("\\b"); break;
					case '\f': Cat("\\f"); break;
					case '\n': Cat("\\n"); break;
					case '\r': Cat("\\r"); break;
					case '\t': Cat("\\t"); break;
					default:
						if(c < 0) // non-BMP character
							CatChar(c);
						else if(c < 0x20)
							Cat("\\u00").CatHex(static_cast<uint8>(c));
						else
							CatChar(c);
						break;
				}
			}
			ShiftLeft(_len);
		}
	}
	return *this;
}

SString & SLAPI SString::Unescape()
{
	const size_t _len = Len();
	if(_len) {
		for(size_t i = 0; i < _len; i++) {
			const char c1 = P_Buf[i];
			if(c1 == '\\') {
				const char c = P_Buf[++i];
				switch(c) {
					case '\"':
					case '\\':
					case '/': // literal translation
						CatChar(c);
						break;
					case 'b': CatChar('\b'); break;
					case 'f': CatChar('\f'); break;
					case 'n': CatChar('\n'); break;
					case 'r': CatChar('\r'); break;
					case 't': Tab(); break;
					case 'u':
						{
							char   ubuf[5];
							ubuf[0] = P_Buf[++i];
							ubuf[1] = P_Buf[++i];
							ubuf[2] = P_Buf[++i];
							ubuf[3] = P_Buf[++i];
							ubuf[4] = 0;
							int64  unicode = strtol(ubuf, NULL, 16);
							if(unicode < 0x80) // ASCII: map to UTF-8 literally
								CatChar(static_cast<char>(unicode&0xff));
							else if(unicode < 0x800) { // two-byte-encoding
								char one = '\xC0'; /* 110 00000 */
								char two = '\x80'; /* 10 000000 */
								two += static_cast<char>(unicode & 0x3F);
								unicode >>= 6;
								one += static_cast<char>(unicode & 0x1F);
								CatChar(one).CatChar(two);
							}
							else if(unicode < 0x10000) {
								if(unicode < 0xD800 || 0xDBFF < unicode) { // three-byte-encoding
									char one = '\xE0';   // 1110 0000 
									char two = '\x80';   // 10 000000 
									char three = '\x80'; // 10 000000 
									three += static_cast<char>(unicode & 0x3F);
									unicode >>= 6;
									two += static_cast<char>(unicode & 0x3F);
									unicode >>= 6;
									one += static_cast<char>(unicode & 0xF);
									CatChar(one).CatChar(two).CatChar(three);
								}
								else {
									// unicode is a UTF-16 high surrogate, continue with the low surrogate
									int64 high_surrogate = unicode;	// 110110 00;00000000
									int64 low_surrogate;
									char one   = '\xF0'; // 11110 000
									char two   = '\x80'; // 10 000000
									char three = '\x80'; // 10 000000
									char four  = '\x80'; // 10 000000
									if(P_Buf[++i] != '\\')
										break;
									if(P_Buf[++i] != 'u')
										break;

									ubuf[0] = P_Buf[++i];
									ubuf[1] = P_Buf[++i];
									ubuf[2] = P_Buf[++i];
									ubuf[3] = P_Buf[++i];
									ubuf[4] = 0;

									low_surrogate = strtol(ubuf, NULL, 16); /* 110111 00;00000000 */

									// strip surrogate markers
									high_surrogate -= 0xD800; // 11011000;00000000
									low_surrogate  -= 0xDC00; // 11011100;00000000

									unicode = (high_surrogate << 10) + (low_surrogate) + 0x10000;

									// now encode into four-byte UTF-8 (as we are larger than 0x10000)
									four += static_cast<char>(unicode & 0x3F);
									unicode >>= 6;
									three += static_cast<char>(unicode & 0x3F);
									unicode >>= 6;
									two += static_cast<char>(unicode & 0x3F);
									unicode >>= 6;
									one += static_cast<char>(unicode & 0x7);
									CatChar(one).CatChar(two).CatChar(three).CatChar(four);
								}
							}
							else {
								CatChar('#err').CatHex(unicode);
							}
							break;
						}
					default:
						CatChar(c1);
						// assert(0);
						break;
				}
			}
			else {
				CatChar(c1);
			}
		}
		ShiftLeft(_len);
	}
	return *this;
}

SString & SLAPI SString::ToUrl()
{
#define NORMURLC(c) (((c)>='0'&&(c)<= '9')||((c)>='a'&&(c)<='z')||((c)>='A'&&(c)<='Z')||oneof9((c),'-','_','.','!','~','*','\'','(',')'))
    const size_t _len = Len();
    if(_len) {
		size_t cvt_offs = _len;
		size_t i;
		for(i = 0; cvt_offs == _len && i < _len; i++) {
			const char c = P_Buf[i];
			if(!NORMURLC(c))
				cvt_offs = i;
		}
		if(cvt_offs < _len) {
			SString temp_buf;
			if(cvt_offs)
				temp_buf.CopyFromN(*this, cvt_offs);
            for(i = cvt_offs; i < _len; i++) {
				const char c = P_Buf[i];
				if(!NORMURLC(c)) {
					temp_buf.CatChar('%').CatHex(static_cast<uint8>(c));
				}
				else {
					temp_buf.CatChar(c);
				}
            }
            CopyFrom(temp_buf);
		}
    }
    return *this;
#undef NORMURLC
}

SString & SLAPI SString::FromUrl()
{
    const size_t _len = Len();
    if(_len) {
		size_t inv_chr_pos = _len;
		size_t cvt_offs = _len;
		size_t i;
		for(i = 0; cvt_offs == _len && inv_chr_pos == _len && i < _len; i++) {
			const char c = P_Buf[i];
			if(c == '%') {
				if((i+2) < _len && ishex(P_Buf[i+1]) && ishex(P_Buf[i+2]))
					cvt_offs = i;
				else
					inv_chr_pos = i;
			}
		}
		if(inv_chr_pos == _len) {
			if(cvt_offs < _len) {
				SString temp_buf;
				if(cvt_offs)
					temp_buf.CopyFromN(*this, cvt_offs);
				for(i = cvt_offs; i < _len; i++) {
					const char c = P_Buf[i];
					if(c == '%') {
						// Выше мы убедились, что "плохих" сочетаний нет. Следовательно, в этом цикле
						// мы их встретить не можем.
						assert((i+2) < _len && ishex(P_Buf[i+1]) && ishex(P_Buf[i+2]));
                        const uint8 real_c = hextobyte(P_Buf+i+1);
                        temp_buf.CatChar(static_cast<char>(real_c));
                        i++;
                        i++;
					}
					else
						temp_buf.CatChar(c);
				}
				CopyFrom(temp_buf);
			}
		}
    }
    return *this;
}

SString & SLAPI SString::ToOem()
{
	if(Len())
		CharToOemA(P_Buf, P_Buf); // @unicodeproblem
	return *this;
}

SString & SLAPI SString::ToChar()
{
	if(Len())
		OemToCharA(P_Buf, P_Buf); // @unicodeproblem
	return *this;
}

SString & FASTCALL SString::Transf(int ctransf)
{
	if(Len()) {
		switch(ctransf) {
			case CTRANSF_INNER_TO_OUTER: OemToCharA(P_Buf, P_Buf); break; // @unicodeproblem
			case CTRANSF_OUTER_TO_INNER: CharToOemA(P_Buf, P_Buf); break; // @unicodeproblem
			case CTRANSF_INNER_TO_UTF8:
				OemToCharA(P_Buf, P_Buf); // @unicodeproblem
				return Helper_MbToMb(CP_ACP, CP_UTF8);
			case CTRANSF_OUTER_TO_UTF8: return Helper_MbToMb(CP_ACP, CP_UTF8);
			case CTRANSF_UTF8_TO_INNER: return Helper_MbToMb(CP_UTF8, CP_OEMCP);
			case CTRANSF_UTF8_TO_OUTER: return Helper_MbToMb(CP_UTF8, CP_ACP);
		}
	}
	return *this;
}

SString & SLAPI SString::Helper_MbToMb(uint srcCodepage, uint destCodepage)
{
	const size_t middle_buf_len = 2048;
	WCHAR wtext[middle_buf_len];
	char  text[middle_buf_len];
	if(srcCodepage == CP_UTF8) {
		//
		// Для UTF8 сделан специальный блок кода по причине того, что
		// отрезками преобразовывать UTF8 нельзя из-за потенциальной опасности
		// исказить конвертируемую строку на конце отдельного отрезка:
		// размер символа UTF8 - не фиксированный.
		//
		SStringU & r_temp_ustr = SLS.AcquireRvlStrU();
		r_temp_ustr.CopyFromUtf8(P_Buf, Len());
		const size_t len = r_temp_ustr.Len();
		Trim(0);
		for(size_t offs = 0; offs < len;) {
			size_t s = MIN((len-offs), middle_buf_len/2);
			int ret = WideCharToMultiByte(destCodepage, 0, static_cast<const wchar_t *>(r_temp_ustr)+offs, static_cast<int>(s), text, static_cast<int>(sizeof(text)), 0, 0);
			if(ret > 0) {
				offs += s;
				CatN(text, static_cast<size_t>(ret));
			}
			else
				break;
		}
		return *this;
	}
	else {
		SString & r_temp_buf = SLS.AcquireRvlStr();
		const size_t len = Len();
		for(size_t offs = 0; offs < len;) {
			size_t s = MIN((len-offs), middle_buf_len/2);
			int    ret = MultiByteToWideChar(srcCodepage, 0, P_Buf+offs, static_cast<int>(s), wtext, SIZEOFARRAY(wtext));
			if(ret > 0) {
				offs += s;
				ret = WideCharToMultiByte(destCodepage, 0, wtext, ret, text, sizeof(text), 0, 0);
				if(ret > 0) {
					assert(offs <= len);
					if(offs == len) {
						//
						// Отдельная проверка на окончание цикла для того, чтобы
						// избежать лишнего копирования во временный буфер.
						//
						return CopyFrom(r_temp_buf).CatN(text, static_cast<size_t>(ret));
					}
					else
						r_temp_buf.CatN(text, static_cast<size_t>(ret));
				}
			}
			else
				break;
		}
		return CopyFrom(r_temp_buf);
	}
}

SString & SLAPI SString::Utf8ToLower()
{
	if(L > 1) {
		SStringU & r_us = SLS.AcquireRvlStrU(); // @v9.9.4
		if(r_us.CopyFromUtf8Strict(P_Buf, L-1)) {
			r_us.ToLower();
			r_us.CopyToUtf8(*this, 1);
		}
	}
	return *this;
}

SString & SLAPI SString::Utf8ToUpper()
{
	if(L > 1) {
		SStringU & r_us = SLS.AcquireRvlStrU(); // @v9.9.4
		if(r_us.CopyFromUtf8Strict(P_Buf, L-1)) {
			r_us.ToUpper();
			r_us.CopyToUtf8(*this, 1);
		}
	}
	return *this;
}

SString & SLAPI SString::ToUpper()
{
	strupr866(P_Buf);
	return *this;
}

SString & SLAPI SString::ToLower()
{
	strlwr866(P_Buf);
	return *this;
}

SString & SLAPI SString::ToUpper1251()
{
	strupr1251(P_Buf);
	return *this;
}

SString & SLAPI SString::ToLower1251()
{
	strlwr1251(P_Buf);
	return *this;
}

SString & FASTCALL SString::SetCase(int ccas)
{
	if(P_Buf && L > 1) {
		if(ccas == CCAS_LOWER) {
			strlwr(P_Buf);
		}
		else if(ccas == CCAS_UPPER) {
			strupr(P_Buf);
		}
		else if(ccas == CCAS_CAPITAL) {
			strlwr(P_Buf);
			P_Buf[0] = toupper(P_Buf[0]);
		}
	}
	return *this;
}

SString & SString::NumberToLat(uint value)
{
	CopyFrom(0);
	if(value) {
		while(value) {
			CatChar('A' + value % 26);
			value /= 26;
		}
	}
	else
		CatChar('A');
	Reverse();
	return *this;
}

int FASTCALL SString::IsEqual(const SString & rS) const
{
	const size_t len = Len();
	if(len == rS.Len()) {
		const char * p_buf = P_Buf;
		const char * p_sbuf = rS.P_Buf;
		assert(len == 0 || (p_buf && p_sbuf));
		switch(len) {
			case 0: return 1;
			case 1: return BIN(p_buf[0] == p_sbuf[0]);
			case 2: return BIN(PTR16C(p_buf)[0] == PTR16C(p_sbuf)[0]);
			case 3: return BIN(p_buf[0] == p_sbuf[0] && p_buf[1] == p_sbuf[1] && p_buf[2] == p_sbuf[2]);
			case 4: return BIN(PTR32C(p_buf)[0] == PTR32C(p_sbuf)[0]);
			case 8: return BIN(PTR64C(p_buf)[0] == PTR64C(p_sbuf)[0]);
			default: return BIN(memcmp(p_buf, p_sbuf, len) == 0);
		}
	}
	else
		return 0;
}

int FASTCALL SString::IsEqual(const char * pS) const
{
	const size_t len = Len();
	const size_t len2 = sstrlen(pS);
	if(len == len2) {
		const char * p_buf = P_Buf;
		assert(len == 0 || p_buf);
		switch(len) {
			case 0: return 1;
			case 1: return BIN(p_buf[0] == pS[0]);
			case 2: return BIN(PTR16C(p_buf)[0] == PTR16C(pS)[0]);
			case 3: return BIN(p_buf[0] == pS[0] && p_buf[1] == pS[1] && p_buf[2] == pS[2]);
			case 4: return BIN(PTR32C(p_buf)[0] == PTR32C(pS)[0]);
			case 8: return BIN(PTR64C(p_buf)[0] == PTR64C(pS)[0]);
			default: return BIN(memcmp(p_buf, pS, len) == 0);
		}
	}
	else
		return 0;
}

int FASTCALL SString::Cmp(const char * pS, int ignoreCase) const
{
	if(P_Buf == 0 || pS == 0)
		return -1;
	else
		return ignoreCase ? stricmp866(P_Buf, pS) : strcmp(P_Buf, pS);
}

bool FASTCALL SString::IsEqiAscii(const char * pS) const // @v10.3.5 int-->bool
{
	if(P_Buf != pS) {
        const size_t len = Len();
        if(len != sstrlen(pS))
			return false;
		else if(len) {
            for(size_t i = 0; i < len; i++) {
				/*char*/uint c1 = P_Buf[i];
				/*char*/uint c2 = pS[i];
				if(c1 != c2) {
					if(c1 >= 'A' && c1 <= 'Z')
						c1 += ('a' - 'A');
					if(c2 >= 'A' && c2 <= 'Z')
						c2 += ('a' - 'A');
					if(c1 != c2)
						return false;
				}
            }
		}
	}
	return true;
}

int FASTCALL SString::CmpNC(const char * pS) const
{
	const int this_empty = Empty();
	const int s_empty = isempty(pS);
	if(this_empty && s_empty)
		return 0;
	else if(this_empty && !s_empty)
		return -1;
	else if(!this_empty && s_empty)
		return +1;
	else
		return stricmp866(P_Buf, pS);
}

int FASTCALL SString::CmpPrefix(const char * pS, int ignoreCase) const
{
	const size_t len = sstrlen(pS);
	if(len && Len() >= len)
		return ignoreCase ? strnicmp866(P_Buf, pS, len) : strncmp(P_Buf, pS, len);
	else
		return -1;
}

int FASTCALL SString::CmpL(const char * pS, int ignoreCase) const
{
	const size_t len = Len();
	if(len == 0 || pS == 0)
		return -1;
	else
		return ignoreCase ? strnicmp866(P_Buf, pS, len) : strncmp(P_Buf, pS, len);
}

int FASTCALL SString::CmpSuffix(const char * pS, int ignoreCase) const
{
	if(P_Buf == 0 || pS == 0)
		return -1;
	else {
		const  size_t len = sstrlen(pS);
		const  int delta = static_cast<int>(Len())-static_cast<int>(len);
		if(len && delta >= 0)
			return ignoreCase ? strnicmp866(P_Buf+delta, pS, len) : strncmp(P_Buf+delta, pS, len);
		else
			return -1;
	}
}

uint SLAPI SString::OneOf(int div, const char * pPattern, int ignoreCase) const
{
	if(P_Buf && !isempty(pPattern)) {
		StringSet ss(div, pPattern);
		SString temp_buf;
		for(uint pos = 0, idx = 1; ss.get(&pos, temp_buf); idx++)
			if(Cmp(temp_buf, ignoreCase) == 0)
				return idx;
	}
	return 0;
}

SString & SLAPI SString::TrimRight()
{
	if(L > 1) {
		P_Buf[L-2] = 0;
		L--;
	}
	return *this;
}

SString & FASTCALL SString::TrimRightChr(int c)
{
	//
	// Algorithm: { if(Last() == c) TrimRight(); }
	//
	if(L > 1 && P_Buf[L-2] == c) {
		P_Buf[L-2] = 0;
		L--;
	}
	return *this;
}

SString & FASTCALL SString::Trim(size_t n)
{
	if(n < Len()) {
		P_Buf[n] = 0;
		L = n+1;
	}
	return *this;
}

SString & SLAPI SString::TrimToDiv(size_t n, const char * pDivList)
{
	if(n < Len()) {
		if(pDivList) {
			size_t p = n;
			if(p)
				do {
					if(sstrchr(pDivList, P_Buf[--p])) {
						n = p;
						break;
					}
				} while(p);
		}
		P_Buf[n] = 0;
		L = n+1;
	}
	return *this;
}

/*
0123456789
----------
*/

SString & SLAPI SString::Excise(size_t start, size_t size)
{
	size_t len = Len();
	if(start < len) {
		size_t end = start+size;
		if(end < len) {
			memcpy(P_Buf+start, P_Buf+end, len-end+1); // +1 - zero
			L -= size;
		}
		else {
			P_Buf[start] = 0;
			L = start+1;
		}
	}
	return *this;
}

SString & SLAPI SString::Chomp()
{
	const char _last = Last();
	if(_last == '\xA') {
		TrimRight();
		if(Last() == '\xD')
			TrimRight();
	}
	else if(_last == '\xD')
		TrimRight();
	return *this;
}

SString & SLAPI SString::Strip(int dir)
{
	if(dir == 0 || dir == 1) {
		while(Last() == ' ')
			TrimRight();
	}
	if(L > 1 && (dir == 0 || dir == 2)) {
		size_t p = 0;
		while(p < (L-1) && P_Buf[p] == ' ')
			p++;
		ShiftLeft(p);
	}
	return *this;
}

SString & SLAPI SString::Strip()
{
	while(Last() == ' ')
		TrimRight();
	if(L > 1) {
		size_t p = 0;
		while(p < (L-1) && P_Buf[p] == ' ')
			p++;
		ShiftLeft(p);
	}
	return *this;
}

SString & SLAPI SString::StripQuotes()
{
	Strip();
	if(C(0) == '\"') {
		ShiftLeft(1);
		size_t pos;
		if(SearchChar('\"', &pos))
			Trim(pos);
	}
	return *this;
}

SString & FASTCALL SString::CatChar(int chr)
{
	const size_t new_len = (L ? L : 1) + 1;
	if(new_len <= Size || Alloc(new_len)) {
		L = new_len;
		P_Buf[new_len-2] = chr;
		P_Buf[new_len-1] = 0;
	}
	return *this;
}

SString & FASTCALL SString::CatCharN(int chr, size_t n)
{
	if(n) {
		const size_t new_len = (L ? L : 1) + n;
		if(new_len <= Size || Alloc(new_len)) { // @v8.1.11 (new_len <= Size ||)
			memset(P_Buf+Len(), chr, n);
			P_Buf[new_len-1] = 0;
			L = new_len;
		}
	}
	return *this;
}

SString & FASTCALL SString::ShiftLeft(size_t n)
{
	if(n > 0)
		if(Len() > n) {
			memmove(P_Buf, P_Buf+n, L-n);
			L -= n;
		}
		else if(L > 1) {
			L = 1;
			P_Buf[0] = 0;
		}
	return *this;
}

SString & FASTCALL SString::ShiftLeftChr(int chr)
{
	if(L > 1 && P_Buf[0] == chr)
		ShiftLeft(1);
	return *this;
}

SString & SLAPI SString::Reverse()
{
	size_t len = Len();
	if(len > 1) {
		size_t i = len/2;
		memswap(P_Buf, P_Buf+(len-i), i);
	}
	return *this;
}

SString & SLAPI SString::PadLeft(size_t n, int pad)
{
	if(pad != 0)
		ShiftRight(n, pad);
	return *this;
}

SString & SLAPI SString::Align(size_t width, int adj)
{
	size_t diff = Strip().Len();
	diff = (width > diff) ? (width - diff) : 0;
	if(diff) {
		switch(adj) {
			case ADJ_LEFT:  CatCharN(' ', diff);   break;
			case ADJ_RIGHT: ShiftRight(diff, ' '); break;
			case ADJ_CENTER:
				size_t k = diff / 2;
				ShiftRight(diff - k, ' ').CatCharN(' ', k);
				break;
		}
	}
	return *this;
}

SString & SLAPI SString::ShiftRight(size_t n, int chr)
{
	if(n) {
		const size_t new_len = (L ? L : 1) + n;
		if(Alloc(new_len)) {
			if(L)
				memmove(P_Buf+n, P_Buf, L);
			else
				P_Buf[new_len-1] = 0;
			L = new_len;
			memset(P_Buf, chr, n);
		}
	}
	return *this;
}

SString & FASTCALL SString::Cat(const SString & s)
{
	const size_t add_len = s.Len();
	if(add_len) {
		const size_t new_len = (L ? L : 1) + add_len;
		if(new_len <= Size || Alloc(new_len)) {
			memcpy(P_Buf+Len(), s.P_Buf, s.L);
			L = new_len;
		}
	}
	return *this;
}

SString & FASTCALL SString::Cat(const char * pS)
{
	const size_t add_len = sstrlen(pS);
	if(add_len) {
		const size_t new_len = (L ? L : 1) + add_len;
		if(new_len <= Size || Alloc(new_len)) { // @v8.1.11 (new_len <= Size ||)
			memcpy(P_Buf+Len(), pS, add_len+1);
			L = new_len;
		}
	}
	return *this;
}

SString & FASTCALL SString::CatN(const char * pS, size_t maxLen)
{
	if(pS && maxLen) {
		const char * p_zero = static_cast<const char *>(memchr(pS, 0, maxLen));
		size_t add_len = p_zero ? (p_zero - pS) : maxLen;
		if(add_len) {
			const size_t new_len = (L ? L : 1) + add_len;
			if(new_len <= Size || Alloc(new_len)) { // @v8.1.11 (new_len <= Size ||)
				memcpy(P_Buf+Len(), pS, add_len);
				P_Buf[new_len-1] = 0;
				L = new_len;
			}
		}
	}
	return *this;
}

SString & FASTCALL SString::Cat(SBuffer & rS)
{
	size_t prev_len = Len();
	CatN(static_cast<const char *>(rS.GetBuf(rS.GetRdOffs())), rS.GetAvailableSize());
	if(Len() > prev_len)
		rS.SetRdOffs(Len()-prev_len);
	return *this;
}

SString & FASTCALL SString::CatDiv(int c, int addSpaces)
{
	if(c != 0) {
		if(addSpaces > 0 && addSpaces != 2)
			Space();
		CatChar(c);
	}
	if(addSpaces)
		Space();
	return *this;
}

SString & FASTCALL SString::CatDivConditionally(int c, int addSpaces, bool condition)
{
	if(condition) {
		if(c != 0) {
			if(addSpaces > 0 && addSpaces != 2)
				Space();
			CatChar(c);
		}
		if(addSpaces)
			Space();
	}
	return *this;
}

SString & FASTCALL SString::CatDivIfNotEmpty(int c, int addSpaces)
{
	return CatDivConditionally(c, addSpaces, LOGIC(Strip().NotEmpty()));
}

SString & FASTCALL SString::Cat(long i)
{
	char   temp_buf[512];
	return Cat(ltoa(i, temp_buf, 10));
}

SString & FASTCALL SString::Cat(ulong i)
{
	char   temp_buf[512];
	return Cat(ultoa(i, temp_buf, 10));
}

#ifndef _WIN32_WCE // {

SString & SLAPI SString::Cat(int64 i)
{
	char   temp_buf[512];
	return Cat(_i64toa(i, temp_buf, 10));
}

SString & SLAPI SString::Cat(uint64 i)
{
	char   temp_buf[512];
	return Cat(_ui64toa(i, temp_buf, 10));
}
#endif // } _WIN32_WCE

SString & FASTCALL SString::Cat(int i)
{
	char   temp_buf[512];
	return Cat(ltoa(i, temp_buf, 10));
}

SString & FASTCALL SString::Cat(uint i)
{
	char   temp_buf[512];
	return Cat(ultoa(i, temp_buf, 10));
}

char * FASTCALL longfmtz(long val, int numDigits, char * pBuf, size_t bufLen)
{
	SString temp_buf;
	temp_buf.CatLongZ(val, numDigits);
	return strnzcpy(pBuf, temp_buf, bufLen);
}

SString & FASTCALL SString::CatLongZ(long val, uint numDigits)
{
	if(numDigits > 0 && numDigits <= 512) {
		SString & r_temp_buf = SLS.AcquireRvlStr(); // @v9.9.4
		const size_t _slen = r_temp_buf.Cat(val).Len();
		if(_slen < numDigits)
			r_temp_buf.PadLeft(numDigits - _slen, '0');
		Cat(r_temp_buf);
	}
	else
		Cat(val);
	return *this;
}

SString & FASTCALL SString::CatLongZ(int64 val, uint numDigits)
{
	if(numDigits > 0 && numDigits <= 512) {
		SString & r_temp_buf = SLS.AcquireRvlStr(); // @v9.9.4
		const size_t _slen = r_temp_buf.Cat(val).Len();
		if(_slen < numDigits)
			r_temp_buf.PadLeft(numDigits - _slen, '0');
		Cat(r_temp_buf);
	}
	else
		Cat(val);
	return *this;
}

SString & FASTCALL SString::CatHex(long val)
{
	char   temp_buf[512];
	return Cat(ltoa(val, temp_buf, 16));
}

SString & FASTCALL SString::CatHex(ulong val)
{
	char   temp_buf[512];
	return Cat(ultoa(val, temp_buf, 16));
}

SString & FASTCALL SString::CatHex(int64 val)
{
	char   temp_buf[512];
	return Cat(_i64toa(val, temp_buf, 16));
}

SString & FASTCALL SString::CatHex(uint8 val)
{
	//char   temp_buf[512];
	//ltoa(val, temp_buf, 16);
	uint   dig = (val >> 4);
	CatChar(dig + ((dig < 10) ? '0' : ('a'-10)));
	dig = (val & 0x0f);
	CatChar(dig + ((dig < 10) ? '0' : ('a'-10)));
	return *this;
}

SString & FASTCALL SString::CatHexUpper(uint8 val)
{
	//char   temp_buf[512];
	//ltoa(val, temp_buf, 16);
	uint   dig = (val >> 4);
	CatChar(dig + ((dig < 10) ? '0' : ('A'-10)));
	dig = (val & 0x0f);
	CatChar(dig + ((dig < 10) ? '0' : ('A'-10)));
	return *this;
}

SString & SLAPI SString::CatDotTriplet(int ver, int mjr, int mnr)
{
	return Cat(ver).Dot().Cat(mjr).Dot().Cat(mnr);
}

SString & SLAPI SString::CatPercentMsg(long p, long t, const char * pMsg)
{
	if(pMsg)
		Cat(pMsg).Space();
	return Cat(static_cast<long>(t ? (100.0 * (static_cast<double>(p) / static_cast<double>(t))) : 100.0)).CatChar('%');
}

SString & SLAPI SString::Cat(double v, long fmt)
{
	char   temp_buf[512];
	return Cat(realfmt(v, fmt, temp_buf));
}

SString & SLAPI SString::Cat(const RealRange & rR, long fmt)
{
	char   temp_buf[512];
	Cat(realfmt(rR.low, fmt, temp_buf));
	Dot().Dot();
	Cat(realfmt(rR.upp, fmt, temp_buf));
	return *this;
}

SString & SLAPI SString::Cat(double v)
{
	char   temp_buf[512];
	return Cat(realfmt(v, MKSFMTD(0, 6, NMBF_NOTRAILZ), temp_buf));
}

SString & SLAPI SString::CatReal(double v)
{
	char   temp_buf[512];
	return Cat(_gcvt(v, 40, temp_buf));
}

SString & FASTCALL SString::Cat(LDATE dt, long fmt)
{
	char   temp_buf[128];
	return Cat(datefmt(&dt, fmt, temp_buf));
}

SString & FASTCALL SString::Cat(LDATE dt)
{
	char   temp_buf[128];
	return Cat(datefmt(&dt, DATF_DMY, temp_buf));
}

SString & FASTCALL SString::Cat(LTIME tm, long fmt)
{
	char   temp_buf[128];
	return Cat(timefmt(tm, fmt, temp_buf));
}

SString & FASTCALL SString::Cat(LTIME tm)
{
	char   temp_buf[128];
	return Cat(timefmt(tm, TIMF_HMS, temp_buf));
}

SString & FASTCALL SString::Cat(const LDATETIME & rDtm, long datFmt /*=DATF_DMY*/, long timFmt /*=TIMF_HMS*/)
{
	Cat(rDtm.d, datFmt);
	if((datFmt & 0x000f) == DATF_ISO8601 && oneof2(timFmt, 0, TIMF_TIMEZONE)) {
		CatChar('T').Cat(rDtm.t, (timFmt == TIMF_TIMEZONE) ? (TIMF_HMS|TIMF_TIMEZONE) : TIMF_HMS);
	}
	else
		Space().Cat(rDtm.t, timFmt);
	return *this;
}

SString & FASTCALL SString::Cat(const DateRange & rPeriod, int ext)
{
	char   temp_buf[64];
	if(ext)
		periodfmtex(&rPeriod, temp_buf, sizeof(temp_buf));
	else
		periodfmt(&rPeriod, temp_buf);
	return Cat(temp_buf);
}

SString & FASTCALL SString::Cat(const S_GUID & rUuid, int fmt)
{
	SString & r_temp_buf = SLS.AcquireRvlStr(); // @v9.9.4
    return Cat(rUuid.ToStr(fmt, r_temp_buf));
}

SString & SLAPI SString::CatTag(const char * pTag, const char * pData)
{
	return CatTagBrace(pTag, 0).Cat(pData).CatTagBrace(pTag, 1);
}

SString & SLAPI SString::CatTagBrace(const char * pTag, int kind)
{
	CatChar('<');
	if(kind == 1)
		CatChar('/');
	Cat(pTag);
	if(kind == 2)
		CatChar('/');
	CatChar('>');
	return *this;
}

SString & SLAPI SString::CatXmlElem(const char * pName, int kind, const StringSet * pList)
{
	CatChar('<').CatChar('!').Cat("ELEMENT").Space().Cat(pName).Space().CatChar('(');
	if(kind == 0)
		CatChar('#').Cat("PCDATA");
	else if(pList) {
		SString item;
		for(uint p = 0; pList->get(&p, item); p++)
			Cat(item).Comma();
		TrimRightChr(',');
	}
	CatChar(')').CatChar('>');
	return *this;
}

SString & SLAPI SString::Insert(size_t pos, const char * pS)
{
	const size_t add_len = sstrlen(pS);
	if(add_len) {
		if(pos < Len()) {
			const size_t new_len = (L ? L : 1) + add_len;
			if(Alloc(new_len)) {
				memmove(P_Buf+pos+add_len, P_Buf+pos, L-pos);
				memcpy(P_Buf+pos, pS, add_len);
				L = new_len;
			}
		}
		else if(pos == Len())
			Cat(pS);
	}
	return *this;
}

SString & cdecl SString::Printf(const char * pFormat, ...)
{
	const  size_t new_len = 4096;
	if(Alloc(new_len)) {
		va_list argptr;
		va_start(argptr, pFormat);
		_vsnprintf(P_Buf, Size-1, pFormat, argptr);
		P_Buf[Size-1] = 0;
		L = sstrlen(P_Buf) + 1;
	}
	return *this;
}

SString & __cdecl SString::VPrintf(const char * pFormat, va_list vl)
{
	const  size_t new_len = 4096;
	if(Alloc(new_len)) {
		_vsnprintf(P_Buf, Size-1, pFormat, vl);
		P_Buf[Size-1] = 0;
		L = sstrlen(P_Buf) + 1;
	}
	return *this;
}

int SLAPI SString::IsDigit() const
{
	const  size_t len = Len();
	int    ok = 1;
	if(len) {
		for(size_t i = 0; ok && i < len; i++) {
			if(!isdec(P_Buf[i]))
				ok = 0;
		}
	}
	else
		ok = 0;
	return ok;
}

ulong SLAPI SString::ToULong() const
{
	if(L > 1) {
		const char * p = P_Buf;
		while(oneof2(*p, ' ', '\t'))
			p++;
		char * p_end = 0;
		if(p[0] == '0' && oneof2(p[1], 'x', 'X'))
			return strtoul(p, &p_end, 16);
		else
			return strtoul(p, &p_end, 10);
	}
	else
		return 0;
}

const char * sitoa(int val, char * pBuf, size_t bufLen) // @construction
{
	return 0;
}

uint32 FASTCALL _texttodec32(const char * pT, uint len)
{
	uint32 result;
	switch(len) {
		case 0: result = 0; break;
		case 1: result = pT[0] - '0'; break;
		case 2: result = (10 * (pT[0] - '0')) + (pT[1] - '0'); break;
		case 3: result = (100 * (pT[0] - '0')) + (10 * (pT[1] - '0')) + (pT[2] - '0'); break;
		case 4: result = (1000 * (pT[0] - '0')) + (100 * (pT[1] - '0')) + (10 * (pT[2] - '0')) + (pT[3] - '0'); break;
		case 5: result = (10000 * (pT[0] - '0')) + (1000 * (pT[1] - '0')) + (100 * (pT[2] - '0')) + (10 * (pT[3] - '0')) + (pT[4] - '0'); break;
		default:
			result = 0;
			for(uint i = 0; i < len; i++)
				result = (result * 10) + (pT[i] - '0');
			break;
	}
	return result;
}

uint32 FASTCALL _texttodec32(const wchar_t * pT, uint len)
{
	uint32 result;
	switch(len) {
		case 0: result = 0; break;
		case 1: result = pT[0] - L'0'; break;
		case 2: result = (10 * (pT[0] - L'0')) + (pT[1] - L'0'); break;
		case 3: result = (100 * (pT[0] - L'0')) + (10 * (pT[1] - L'0')) + (pT[2] - L'0'); break;
		case 4: result = (1000 * (pT[0] - L'0')) + (100 * (pT[1] - L'0')) + (10 * (pT[2] - L'0')) + (pT[3] - L'0'); break;
		case 5: result = (10000 * (pT[0] - L'0')) + (1000 * (pT[1] - L'0')) + (100 * (pT[2] - L'0')) + (10 * (pT[3] - L'0')) + (pT[4] - L'0'); break;
		default:
			result = 0;
			for(uint i = 0; i < len; i++)
				result = (result * 10) + (pT[i] - L'0');
			break;
	}
	return result;
}

uint64 FASTCALL _texttodec64(const char * pT, uint len)
{
	uint64 result;
	switch(len) {
		case 0: result = 0; break;
		case 1: result = pT[0] - '0'; break;
		case 2: result = (10 * (pT[0] - '0')) + (pT[1] - '0'); break;
		case 3: result = (100 * (pT[0] - '0')) + (10 * (pT[1] - '0')) + (pT[2] - '0'); break;
		case 4: result = (1000 * (pT[0] - '0')) + (100 * (pT[1] - '0')) + (10 * (pT[2] - '0')) + (pT[3] - '0'); break;
		case 5: result = (10000 * (pT[0] - '0')) + (1000 * (pT[1] - '0')) + (100 * (pT[2] - '0')) + (10 * (pT[3] - '0')) + (pT[4] - '0'); break;
		default:
			result = 0;
			for(uint i = 0; i < len; i++)
				result = (result * 10) + (pT[i] - '0');
			break;
	}
	return result;
}

uint32 FASTCALL _texttohex32(const char * pT, uint len)
{
	uint32 result = 0;
	for(uint i = 0; i < len; i++) {
		result = (result * 16) + hex(pT[i]);
	}
	return result;
}

uint32 FASTCALL _texttohex32(const wchar_t * pT, uint len)
{
	uint32 result = 0;
	for(uint i = 0; i < len; i++) {
		result = (result * 16) + hexw(pT[i]);
	}
	return result;
}

int FASTCALL satoi(const char * pT)
{
	int    result = 0;
	if(pT && pT[0]) {
		const  char * _p = pT;
		size_t src_pos = 0;
		while(oneof2(_p[src_pos], ' ', '\t'))
			src_pos++;
		int    is_neg = 0;
		int    is_hex = 0;
		if(_p[src_pos] == '-') {
			src_pos++;
			is_neg = 1;
		}
		else if(_p[src_pos] == '+')
			src_pos++;
		if(_p[src_pos] == '0' && oneof2(_p[src_pos+1], 'x', 'X')) {
			src_pos += 2;
			is_hex = 1;
		}
		if(is_hex) {
			if(ishex(_p[src_pos])) {
				uint   local_len = 0;
				do {
					local_len++;
				} while(ishex(_p[src_pos+local_len]));
				result = static_cast<int>(_texttohex32(_p+src_pos, local_len));
			}
		}
		else {
			if(isdec(_p[src_pos])) {
				uint   local_len = 0;
				do {
					local_len++;
				} while(isdec(_p[src_pos+local_len]));
				result = static_cast<int>(_texttodec32(_p+src_pos, local_len));
			}
		}
		if(is_neg && result)
			result = -result;
	}
	return result;
}

int64 FASTCALL satoi64(const char * pT)
{
	int64  result = 0;
	const char * _p = pT;
	size_t src_pos = 0;
	while(oneof2(_p[src_pos], ' ', '\t'))
		src_pos++;
	int    is_neg = 0;
	int    is_hex = 0;
	if(_p[src_pos] == '-') {
		src_pos++;
		is_neg = 1;
	}
	else if(_p[src_pos] == '+')
		src_pos++;
	if(_p[src_pos] == '0' && oneof2(_p[src_pos+1], 'x', 'X')) {
		src_pos += 2;
		is_hex = 1;
	}
	if(is_hex) {
		if(ishex(_p[src_pos])) { do { result = result * 16 + hex(_p[src_pos]); } while(ishex(_p[++src_pos])); }
	}
	else {
		if(isdec(_p[src_pos])) { do { result = result * 10 + (_p[src_pos] - '0'); } while(isdec(_p[++src_pos])); }
	}
	if(is_neg && result)
		result = -result;
	return result;
}

int FASTCALL satoi(const wchar_t * pT)
{
	int    result = 0;
	if(pT && pT[0]) {
		const  wchar_t * _p = pT;
		size_t src_pos = 0;
		while(oneof2(_p[src_pos], L' ', L'\t'))
			src_pos++;
		int    is_neg = 0;
		int    is_hex = 0;
		if(_p[src_pos] == L'-') {
			src_pos++;
			is_neg = 1;
		}
		else if(_p[src_pos] == L'+')
			src_pos++;
		if(_p[src_pos] == L'0' && oneof2(_p[src_pos+1], L'x', L'X')) {
			src_pos += 2;
			is_hex = 1;
		}
		if(is_hex) {
			if(ishexw(_p[src_pos])) {
				uint   local_len = 0;
				do {
					local_len++;
				} while(ishexw(_p[src_pos+local_len]));
				result = static_cast<int>(_texttohex32(_p+src_pos, local_len));
			}
		}
		else {
			if(isdecw(_p[src_pos])) {
				uint   local_len = 0;
				do {
					local_len++;
				} while(isdecw(_p[src_pos+local_len]));
				result = static_cast<int>(_texttodec32(_p+src_pos, local_len));
			}
		}
		if(is_neg && result)
			result = -result;
	}
	return result;
}

int64 FASTCALL satoi64(const wchar_t * pT)
{
	int64  result = 0;
	const wchar_t * _p = pT;
	size_t src_pos = 0;
	while(oneof2(_p[src_pos], L' ', L'\t'))
		src_pos++;
	int    is_neg = 0;
	int    is_hex = 0;
	if(_p[src_pos] == L'-') {
		src_pos++;
		is_neg = 1;
	}
	else if(_p[src_pos] == L'+')
		src_pos++;
	if(_p[src_pos] == L'0' && oneof2(_p[src_pos+1], L'x', L'X')) {
		src_pos += 2;
		is_hex = 1;
	}
	if(is_hex) {
		if(ishexw(_p[src_pos])) { do { result = result * 16 + hexw(_p[src_pos]); } while(ishexw(_p[++src_pos])); }
	}
	else {
		if(isdecw(_p[src_pos])) { do { result = result * 10 + (_p[src_pos] - L'0'); } while(isdecw(_p[++src_pos])); }
	}
	if(is_neg && result)
		result = -result;
	return result;
}

long SLAPI SString::ToLong() const
{
	// @v9.6.1 return (long)ToULong();
	// @v9.6.1 Более корректная реализация. И в добавок - значительно быстрее. {
	return (L > 1) ? satoi(P_Buf) : 0; // @v10.3.9 body is moved to satoi()
	// } @v9.6.1
}

int64 SLAPI SString::ToInt64() const
{
	/* @v9.6.1
	if(L > 1) {
		const char * p = P_Buf;
		while(oneof2(*p, ' ', '\t'))
			p++;
		return _atoi64(p);
	}
	else
		return 0;
	*/
	// @v9.6.1 Более корректная реализация. И в добавок - значительно быстрее. {
	return (L > 1) ? satoi64(P_Buf) : 0; // @v10.3.9 body is moved to satoi64()
	// } @v9.6.1
}

double SLAPI SString::ToReal() const
{
	double v = 0.0;
	if(L > 1)
		strtodoub(P_Buf, &v);
	return v;
}

float SLAPI SString::ToFloat() const
{
	// @todo Полностью перевести на float (реализовать strtofloat)
	double v = 0.0;
	if(L > 1)
		strtodoub(P_Buf, &v);
	return static_cast<float>(v);
}

int SLAPI SString::ToIntRange(IntRange & rRange, long flags) const
{
	int    result = 0;
	if(L > 1) {
		long   lo = 0;
		long   up = 0;
		size_t src_pos = 0;
		const  char * p_buf = P_Buf;
		while(p_buf[src_pos] == ' ' || p_buf[src_pos] == '\t')
			src_pos++;
		//
		int    is_neg = 0;
		if(p_buf[src_pos] == '-') {
			src_pos++;
			is_neg = 1;
		}
		else if(p_buf[src_pos] == '+')
			src_pos++;
		if(isdec(p_buf[src_pos])) {
			do {
				lo = lo * 10 + (p_buf[src_pos] - '0');
				src_pos++;
			} while(isdec(p_buf[src_pos]));
			if(is_neg)
				lo = -lo;
			//
			const size_t preserve_upp_src_pos = src_pos;
			int   upp_is_done = 0;
			while(p_buf[src_pos] == ' ' || p_buf[src_pos] == '\t')
				src_pos++;

			uint do_next = 0;
			{
				if(p_buf[src_pos] == ':' && (flags & torfColon))
					do_next = 1;
				else if(p_buf[src_pos] == '-' && (flags & torfHyphen))
					do_next = 1;
				else if(p_buf[src_pos] == '.' && p_buf[src_pos+1] == '.' && (flags & torfDoubleDot))
					do_next = 2;
				else if(p_buf[src_pos] == ',' && p_buf[src_pos+1] == ',' && (flags & torfDoubleComma))
					do_next = 2;
			}
			if(do_next) {
				src_pos += do_next;
				while(p_buf[src_pos] == ' ' || p_buf[src_pos] == '\t')
					src_pos++;
				is_neg = 0;
				if(p_buf[src_pos] == '-') {
					src_pos++;
					is_neg = 1;
				}
				else if(p_buf[src_pos] == '+')
					src_pos++;
				if(isdec(p_buf[src_pos])) {
					do {
						up = up * 10 + (p_buf[src_pos] - '0');
						src_pos++;
					} while(isdec(p_buf[src_pos]));
					if(is_neg)
						up = -up;
					upp_is_done = 1;
				}
			}
			if(upp_is_done) {
				rRange.low = lo;
				rRange.upp = up;
				result = static_cast<int>(src_pos);
			}
			else {
				rRange.low = lo;
				rRange.upp = lo;
				result = static_cast<int>(preserve_upp_src_pos);
			}
		}
		assert(src_pos <= Len());
	}
	return result;
}

SString & SLAPI SString::Sub(size_t startPos, size_t len, SString & rBuf) const
{
	rBuf.Z();
	if(startPos < Len()) {
		const size_t len2 = MIN(len, Len()-startPos);
		rBuf.CopyFromN(P_Buf+startPos, len2);
	}
	return rBuf;
}

SString & SLAPI SString::SetLastDSlash()
{
	int    last = Last();
	if(last) {
		if(last == '\\') {
			TrimRight();
			CatChar('/');
		}
		else if(last != '/')
			CatChar('/');
	}
	return *this;
}

SString & SLAPI SString::SetLastSlash()
{
	int    last = Last();
	if(last && last != '\\' && last != '/')
		CatChar('\\');
	return *this;
}

SString & SLAPI SString::RmvLastSlash()
{
	int    last = Last();
	if(last == '\\' || last == '/')
		TrimRight();
	return *this;
}

int SLAPI SString::Read(FILE * fStream, size_t size)
{
	int    ok = -1;
	if(fStream && size) {
		if(Alloc(size+1)) {
			if(fread(P_Buf, size, 1, fStream) != 1) {
				P_Buf[0] = 0;
				L = 0;
				ok = (SLibError = SLERR_READFAULT, 0);
			}
			else {
				L = size+1;
				P_Buf[L-1] = 0;
				ok = 1;
			}
		}
		else
			ok = 0;
	}
	if(ok <= 0)
		CopyFrom(0);
	return ok;
}

int SLAPI SString::DecodeHex(int swapb, void * pBuf, size_t bufLen, size_t * pRealLen) const
{
	int    ok = 1;
	const size_t src_len = (Len() & ~0x1);
	size_t out_len = 0;
	for(uint i = 0; i < src_len; i+=2) {
		const uint8 byte = (hex(P_Buf[i]) << 4) | hex(P_Buf[i+1]);
		if(out_len < bufLen) {
			PTR8(pBuf)[out_len] = byte;
			out_len++;
			if(swapb) {
				if(!(out_len & 0x1)) {
					uint16 * p_word = PTR16(PTR8(pBuf)+out_len-2);
					*p_word = swapw(*p_word);
					if(!(out_len & 0x3)) {
						uint32 * p_dword = PTR32(PTR8(pBuf)+out_len-4);
						*p_dword = swapdw(*p_dword);
					}
				}
			}
		}
		else
			out_len++;
	}
	ASSIGN_PTR(pRealLen, out_len);
	return ok;
}

SString & SLAPI SString::EncodeMime64(const void * pBuf, size_t dataLen)
{
	size_t needed_size = (dataLen * 2) + 2;
	size_t real_size = 0;
	Alloc(needed_size);
	if(encode64(static_cast<const char *>(pBuf), dataLen, P_Buf, Size, &real_size)) {
		L = real_size;
		assert(Len() == sstrlen(P_Buf));
	}
	else
		Z();
	return *this;
}

size_t FASTCALL SString::CharToQp(char c)
{
	size_t n = 0;
	if((c >= 33 && c <= 126 && c != '=') || c == ' ') {
		CatChar(c); // If the character does not need to be converted, insert it as-is.
		n = 1;
	}
	else {
		CatChar('=').CatHexUpper(static_cast<uchar>(c));
		n = 3;
	}
	return n;
}

SString & SLAPI SString::Encode_QuotedPrintable(const char * pBuf, size_t maxLineLen)
{
	const  size_t slen = sstrlen(pBuf);
	uint   linepos = 0; // Line-length counter for line wrapping in the output.
	for(size_t spos = 0; spos < slen; spos++) {
		char c = pBuf[spos];
		// If the character is a linebreak, add a CRLF and reset the line position.
		if(c == '\xA') {
			CRB();
			linepos = 0;
		}
		else if(c == '\xD') {
			CRB();
			linepos = 0;
			if(pBuf[spos+1])
				spos++;
		}
		else {
			size_t qp_len = CharToQp(c);
			// If this character is going to exceed the maximum line length, insert a soft line break.
			if(maxLineLen && (linepos + qp_len) >= maxLineLen) {
				CatChar('=').CRB();
				linepos = 0;
			}
			else
				linepos += qp_len;
		}
	}
	return *this;
}

int SLAPI SString::Decode_QuotedPrintable(SString & rBuf) const
{
	int    ok = 1;
	const size_t src_len = Len();
	const char * p_src_buf = P_Buf;
	for(size_t p = 0; p < src_len;) {
		int enc_c = 0;
		char c = p_src_buf[p];
		if(c == '=') {
			if(p < (src_len-2)) {
				const char c1 = p_src_buf[p+1];
				const char c2 = p_src_buf[p+2];
				if(c1 == '\xD' && c2 == '\xA') {
					enc_c = 2; // soft wrap
				}
				else if(((c1 >= '0' && c1 <= '9') || (c1 >= 'A' && c1 <= 'Z')) && ((c2 >= '0' && c2 <= '9') || (c2 >= 'A' && c2 <= 'Z'))) {
					c = (hex(c1) << 4) | hex(c2);
					enc_c = 3;
				}
			}
			else if(p == (src_len-1)) {
				enc_c = 1; // soft wrap
			}
		}
		if(enc_c == 0) {
			rBuf.CatChar(c);
			p++;
		}
		else if(enc_c == 3) {
			rBuf.CatChar(c);
			p += 3;
		}
		else if(enc_c == 2) {
			// soft wrap
			p += 3;
		}
		else if(enc_c == 1) {
			p++;
			break;
		}
	}
	return ok;
}

// @construction {
int FASTCALL SString::Decode_XMLENT(SString & rBuf) const
{
	rBuf.Z();
	const uint len = Len();
	uint  cp = 0;
	size_t amp_pos = 0;
	const char * p_buf = P_Buf;
	while(cp < len) {
		const char * p = (p_buf[cp] == '&') ? (p_buf+cp) : static_cast<const char *>(memchr(p_buf+cp, '&', len-cp));
		if(p) {
			rBuf.CatN(p_buf+cp, (p-p_buf-cp));
			uint ofs = (p-p_buf);
			assert(p_buf[ofs] == '&');
			ofs++;
			if(p_buf[ofs] == '#') {
				uint nd = 0;
				uint code = 0;
				ofs++;
				if(oneof2(p_buf[ofs], 'x', 'X')) {
					ofs++;
					while(ishex(p_buf[ofs+nd]))
						nd++;
					code = _texttohex32(p_buf+ofs, nd);
				}
				else {
					while(ishex(p_buf[ofs+nd]))
						nd++;
					code = _texttodec32(p_buf+ofs, nd);
				}
				if(p_buf[ofs+nd] == ';') {
					ofs++;
					// concat code to rBuf
					char  utf8_buf[16];
					uint  utf8_len = SUnicode::Utf32ToUtf8(code, utf8_buf);
					rBuf.Cat_(reinterpret_cast<const uint8 *>(utf8_buf), utf8_len);
					cp = ofs+nd;
				}
				else {
					rBuf.CatN(p, (ofs-(p-p_buf)));
					cp = ofs;
				}
			}
			else {
				rBuf.CatN(p, (ofs-(p-p_buf)));
				cp = ofs;
			}
		}
		else {
			rBuf.CatN(p_buf+cp, (len-cp));
			cp = len;
		}
	}
	return 1;
}
// } @construction

int SLAPI SString::DecodeMime64(void * pBuf, size_t bufLen, size_t * pRealLen) const
{
	size_t out_len = bufLen;
	char   zero_buf[32];
	zero_buf[0] = 0;
	int    ok = decode64(NZOR(P_Buf, zero_buf), Len(), static_cast<char *>(pBuf), &out_len);
	ASSIGN_PTR(pRealLen, out_len);
	return ok;
}

SString & SLAPI SString::Encode_EncodedWordRFC2047(const char * pSrcBuf, SCodepageIdent cp, int rfc2207enc)
{
	Z();
	SString temp_buf;
	cp.ToStr(cp.fmtXML, temp_buf);
	Cat("=?").Cat(temp_buf).CatChar('?');
	if(rfc2207enc == rfc2207encQP)
		CatChar('Q');
	else {
		if(rfc2207enc != rfc2207encMime64)
			rfc2207enc = rfc2207encMime64;
		CatChar('B');
	}
	CatChar('?');
	if(rfc2207enc == rfc2207encQP) {
		temp_buf.Z().Encode_QuotedPrintable(pSrcBuf, 0);
		Cat(temp_buf);
	}
	else { // mime64
		temp_buf.Z().EncodeMime64(pSrcBuf, sstrlen(pSrcBuf));
		Cat(temp_buf);
	}
	Cat("?=");
	return *this;
}

size_t SLAPI SString::Decode_EncodedWordRFC2047(SString & rBuf, SCodepageIdent * pCp, int * pRfc2207enc) const
{
	rBuf.Z();
	size_t result = 0;
	const size_t src_len = Len();
	const char * p_src_buf = P_Buf;
	SString temp_buf;
	SString sub;
	STempBuffer bin_buf(0);
	int    rfc2207enc = 0;
	SCodepageIdent cp;
	for(size_t p = 0; p < src_len;) {
		if(p_src_buf[p] == '=' && p_src_buf[p+1] == '?') {
			temp_buf.Z();
			const size_t org_offs = p;
			int    error = 0;
			size_t i = p+2;
			while(i < src_len && p_src_buf[i] != '?') {
				temp_buf.CatChar(p_src_buf[i++]);
			}
			if(i < src_len && cp.FromStr(temp_buf)) {
				p = i+1;
				if(oneof2(p_src_buf[p], 'B', 'b'))
					rfc2207enc = rfc2207encMime64;
				else if(oneof2(p_src_buf[p], 'Q', 'q'))
					rfc2207enc = rfc2207encQP;
				else
					error = 1;
				if(!error) {
					p++;
					if(p_src_buf[p] == '?') {
						p++;
						temp_buf.Z();
						while(p < src_len && !(p_src_buf[p] == '?' && p_src_buf[p+1] == '=')) {
							temp_buf.CatChar(p_src_buf[p++]);
						}
						if(p < src_len) {
							p += 2;
							sub.Z();
							if(rfc2207enc == rfc2207encQP) {
								temp_buf.Decode_QuotedPrintable(sub);
							}
							else { // rfc2207encMime64
								size_t actual_size = 0;
								bin_buf.Alloc(temp_buf.Len()*2);
								temp_buf.DecodeMime64(bin_buf, bin_buf.GetSize(), &actual_size);
								temp_buf.Z();
								for(uint j = 0; j < actual_size; j++) {
									char b = static_cast<const char *>(bin_buf)[j];
									if(b != 0)
										sub.CatChar(b);
									else {
										error = 1;
										break;
									}
								}
							}
							rBuf.Cat(sub);
						}
						else
							error = 1;
					}
				}
			}
			else
				error = 1;
			if(error) {
				rBuf.Cat("=?");
				p = org_offs + 2;
			}
		}
		else {
			rBuf.CatChar(p_src_buf[p]);
			p++;
		}
	}
	return result;
}

SString & SLAPI SString::EncodeUrl(const char * pSrc, int mode)
{
	Z();
	const  size_t slen = sstrlen(pSrc);
	for(size_t spos = 0; spos < slen; spos++) {
		char c = pSrc[spos];
		if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || oneof4(c, '-', '_', '.', '~'))
			CatChar(c);
		else if(mode == 0 && oneof7(c, '#', '&', ',', ':', ';', '=', '?'))
			CatChar(c);
		else
			CatChar('%').CatHexUpper(static_cast<uchar>(c));
	}
	return *this;
}

int SLAPI SString::DecodeUrl(SString & rBuf) const
{
	rBuf.Z();
	int    ok = 1;
	const size_t src_len = Len();
	const char * p_src_buf = P_Buf;
	for(size_t p = 0; p < src_len;) {
		char c = p_src_buf[p];
		if(c == '%') {
			if(p < (src_len-2)) {
				const char c1 = p_src_buf[p+1];
				const char c2 = p_src_buf[p+2];
				if(((c1 >= '0' && c1 <= '9') || (c1 >= 'A' && c1 <= 'Z') || (c1 >= 'a' && c1 <= 'z')) &&
					((c2 >= '0' && c2 <= '9') || (c2 >= 'A' && c2 <= 'Z') || (c2 >= 'a' && c2 <= 'z'))) {
					c = (hex(c1) << 4) | hex(c2);
					rBuf.CatChar(c);
				}
				else {
					ok = 0;
					rBuf.CatChar(c1);
					rBuf.CatChar(c2);
				}
				p += 3;
			}
			else {
				ok = 0;
				rBuf.CatChar(c);
				p++;
			}
		}
		else {
			rBuf.CatChar(c);
			p++;
		}
	}
	return ok;
}

SString & SString::EncodeString(const char * pSrc, const char * pEncodeStr, int decode)
{
	SString buf, pattern, replacer;
	// @v9.2.1 TaggedStringArray list;
	StrAssocArray list; // @v9.2.1
	SString src = pSrc;
	StringSet ss(';', pEncodeStr);
	uint   p;
	for(p = 0; ss.get(&p, buf) > 0;)
		list.AddFast(MAXLONG - static_cast<long>(buf.Len()), buf); // Без проверки на дублирование идентификатора
	list.SortByID();
	ss.setDelim(",");
	for(p = 0; p < list.getCount(); p++) {
		uint p1 = 0;
		buf = list.Get(p).Txt;
		ss.setBuf(buf, buf.Len() + 1);
		ss.get(&p1, (decode == 0) ? replacer : pattern);
		ss.get(&p1, (decode == 0) ? pattern  : replacer);
		if(src.CmpPrefix(pattern, 0) == 0) {
			src.ShiftLeft(pattern.Len());
			(buf = replacer).Cat(src);
			src = buf;
			break;
		}
	}
	*this = src;
	return *this;
}

SString & SString::FormatFileParsingMessage(const char * pFileName, int lineNo, const char * pAddedText)
{
	CopyFrom(0);
	Cat(pFileName).CatChar('(').Cat(lineNo).CatChar(')').CatDiv(':', 2).Cat(pAddedText);
	return *this;
}
//
//
//
SLAPI SStringU::SStringU() : L(0), Size(0), P_Buf(0)
{
}

SLAPI SStringU::SStringU(const SStringU & rS) : L(0), Size(0), P_Buf(0)
{
	CopyFrom(rS);
}

SLAPI SStringU::SStringU(const wchar_t * pS) : L(0), Size(0), P_Buf(0)
{
	CopyFrom(pS);
}

SLAPI SStringU::~SStringU()
{
	L = 0;
	Size = 0;
	if(P_Buf) // @v9.6.4 @speedcritical
		ZFREE(P_Buf);
}

size_t SLAPI SStringU::Len() const { return L ? (L-1) : 0; }
wchar_t FASTCALL SStringU::C(size_t n) const { return (n < Len()) ? P_Buf[n] : 0; }

int FASTCALL SStringU::Alloc(size_t sz)
{
	int    ok = 1;
	if(sz == 0) {
		Trim(0);
	}
	else if(sz > Size) {
		size_t new_size = SnapUpSize(sz);
		wchar_t * p = 0;
		if((7 * Size) < (8 * L)) { // Assume probability of a non-moving realloc is 0.125
			// If L is close to Size in size then use realloc to reduce the memory defragmentation
			p = static_cast<wchar_t *>(SAlloc::R(P_Buf, new_size*sizeof(wchar_t)));
		}
		else {
			// If L is not close to Size then avoid the penalty of copying
			// the extra bytes that are allocated, but not considered part of the string
			p = static_cast<wchar_t *>(SAlloc::M(new_size*sizeof(wchar_t)));
			if(!p)
				p = static_cast<wchar_t *>(SAlloc::R(P_Buf, new_size*sizeof(wchar_t)));
			else {
				if(L)
					memcpy(p, P_Buf, L*sizeof(wchar_t));
				SAlloc::F(P_Buf);
			}
		}
		if(p) {
			Size = new_size;
			P_Buf = p;
			// @v9.4.9 @fix (big mistake) P_Buf[L] = 0;
			// @v9.4.9 {
			if(L)
				P_Buf[L-1] = 0;
			else
				P_Buf[0] = 0;
			// } @v9.4.9
		}
		else
			ok = 0;
	}
	return ok;
}

int FASTCALL SStringU::IsEqual(const SStringU & rS) const
{
	const size_t len = Len();
	if(len == rS.Len()) {
		if(len == 0)
			return 1;
		else {
			assert(P_Buf && rS.P_Buf);
			if(P_Buf && rS.P_Buf) {
				switch(len) {
					case 1: return BIN(P_Buf[0] == rS.P_Buf[0]);
					case 2: return BIN(PTR32(P_Buf)[0] == PTR32(rS.P_Buf)[0]);
					case 3: return BIN(P_Buf[0] == rS.P_Buf[0] && P_Buf[1] == rS.P_Buf[1] && P_Buf[2] == rS.P_Buf[2]);
					case 4: return BIN(PTR64(P_Buf)[0] == PTR64(rS.P_Buf)[0]);
					case 8: return BIN(PTR64(P_Buf)[0] == PTR64(rS.P_Buf)[0] && PTR64(P_Buf)[1] == PTR64(rS.P_Buf)[1]);
					default: return BIN(memcmp(P_Buf, rS.P_Buf, len) == 0);
				}
			}
			else
				return 0;
		}
	}
	else
		return 0;
}

int FASTCALL SStringU::IsEqual(const wchar_t * pS) const
{
	const size_t len = Len();
	const size_t len2 = sstrlen(pS);
	if(len == len2) {
		if(len == 0)
			return 1;
		else {
			assert(P_Buf);
			if(P_Buf) {
				switch(len) {
					case 1: return BIN(P_Buf[0] == pS[0]);
					case 2: return BIN(PTR32C(P_Buf)[0] == PTR32C(pS)[0]);
					case 3: return BIN(P_Buf[0] == pS[0] && P_Buf[1] == pS[1] && P_Buf[2] == pS[2]);
					case 4: return BIN(PTR64C(P_Buf)[0] == PTR64C(pS)[0]);
					case 8: return BIN(PTR64C(P_Buf)[0] == PTR64C(pS)[0] && PTR64C(P_Buf)[1] == PTR64C(pS)[1]);
					default: return BIN(memcmp(P_Buf, pS, len) == 0);
				}
			}
			else
				return 0;
		}
	}
	else
		return 0;
}

int FASTCALL SStringU::Cmp(const SStringU & rS) const
{
	if(P_Buf == 0 || P_Buf[0] == 0) {
		if(rS.P_Buf == 0 || rS.P_Buf[0] == 0)
			return 0;
		else
			return -1;
	}
	else if(rS.P_Buf == 0 || rS.P_Buf[0] == 0)
		return 1;
	else
		return wcscmp(P_Buf, rS.P_Buf);
}

int FASTCALL SStringU::CmpPrefix(const wchar_t * pS) const
{
	const size_t len = sstrlen(pS);
	if(len && Len() >= len)
		return wcsncmp(P_Buf, pS, len);
	else
		return -1;
}

wchar_t SLAPI SStringU::Last() const { return (L > 1) ? P_Buf[L-2] : 0; }

int FASTCALL SStringU::HasChr(wchar_t c) const
{
	if(c == 0)
		return 0;
	else {
		const wchar_t * p_buf = P_Buf;
		switch(L) {
			case 0:
			case 1:  return 0;
			case 2:  return (p_buf[0] == c);
			case 3:  return (p_buf[0] == c || p_buf[1] == c);
			case 4:  return (p_buf[0] == c || p_buf[1] == c || p_buf[2] == c);
			case 5:  return (p_buf[0] == c || p_buf[1] == c || p_buf[2] == c || p_buf[3] == c);
			default: return BIN(wmemchr(p_buf, c, L-1));
		}
	}
}

SStringU & FASTCALL SStringU::operator = (const SStringU & s) { return CopyFrom(s); }
SStringU & FASTCALL SStringU::operator = (const wchar_t * pS) { return CopyFrom(pS); }

SStringU & SLAPI SStringU::Z()
{
	if(2 <= Size || Alloc(2)) { // 4 байта быстрее обнуляются, чем 2 поэтому требуем минимальную длину - 2 символа (4 байта)
		PTR32(P_Buf)[0] = 0;
		L = 1;
	}
	else if(P_Buf) {
		P_Buf[0] = 0;
		L = 1;
	}
	else
		L = 0;
	return *this;
}

SStringU & FASTCALL SStringU::Trim(size_t n)
{
	if(n < Len()) {
		P_Buf[n] = 0;
		L = n+1;
	}
	return *this;
}

SStringU & FASTCALL SStringU::CatChar(wchar_t chr)
{
	const size_t new_len = (L ? L : 1) + 1;
	if(Alloc(new_len)) {
		P_Buf[new_len-2] = chr;
		P_Buf[new_len-1] = 0;
		L = new_len;
	}
	return *this;
}

SStringU & FASTCALL SStringU::CatN(const wchar_t * pS, size_t maxLen)
{
	if(pS && maxLen) {
		const wchar_t * p_zero = static_cast<const wchar_t *>(wmemchr(pS, 0, maxLen));
		size_t add_len = p_zero ? (p_zero - pS) : maxLen;
		if(add_len) {
			const size_t new_len = (L ? L : 1) + add_len;
			if(Alloc(new_len)) {
				memcpy(P_Buf+Len(), pS, add_len*sizeof(wchar_t));
				P_Buf[new_len-1] = 0;
				L = new_len;
			}
		}
	}
	return *this;
}

SStringU & FASTCALL SStringU::CopyFrom(const SStringU & rS)
{
	const size_t sl = rS.L;
	if(sl && Alloc(sl)) {
		memcpy(P_Buf, rS.P_Buf, sl*sizeof(wchar_t));
		L = sl;
	}
	return *this;
}

SStringU & FASTCALL SStringU::CopyFrom(const wchar_t * pS)
{
	const size_t new_len = pS ? (wcslen(pS) + 1) : 1;
	if(Alloc(new_len)) {
		if(pS) {
			memcpy(P_Buf, pS, new_len*sizeof(wchar_t));
			L = new_len;
		}
		else {
			P_Buf[0] = 0;
			L = 1;
		}
	}
	return *this;
}

SStringU & FASTCALL SStringU::CopyFromN(const wchar_t * pS, size_t maxLen)
{
	size_t new_len = 1;
	if(pS) {
		if(maxLen) {
			const wchar_t * p_zero = static_cast<const wchar_t *>(wmemchr(pS, 0, maxLen));
			new_len += p_zero ? (p_zero - pS) : maxLen;
		}
	}
	if(Alloc(new_len)) {
		if(pS && new_len > 1)
			memcpy(P_Buf, pS, (new_len-1)*sizeof(wchar_t));
		P_Buf[new_len-1] = 0;
		L = new_len;
	}
	return *this;
}

wchar_t * FASTCALL SStringU::CopyTo(wchar_t * pS, size_t bufLen) { return strnzcpy(pS, P_Buf, bufLen); }
SStringU & FASTCALL SStringU::CopyFromMb_OUTER(const char * pS, size_t srcLen) { return CopyFromMb(cpANSI, pS, srcLen); }
SStringU & FASTCALL SStringU::CopyFromMb_INNER(const char * pS, size_t srcLen) { return CopyFromMb(cpOEM, pS, srcLen); }

SStringU & FASTCALL SStringU::CopyFromMb(int cp, const char * pS, size_t srcLen)
{
	Trim(0);
	const size_t middle_buf_len = 2048;
	WCHAR wtext[middle_buf_len];
	if(cp == CP_UTF8) {
		//
		// Для UTF8 сделан специальный блок кода по причине того, что
		// отрезками преобразовывать UTF8 нельзя из-за потенциальной опасности
		// исказить конвертируемую строку на конце отдельного отрезка:
		// размер символа UTF8 - не фиксированный.
		//
		Helper_CopyFromUtf8(pS, srcLen, 0, 0);
	}
	else {
		for(size_t offs = 0; offs < srcLen;) {
			size_t s = MIN((srcLen-offs), middle_buf_len/2);
			int    ret = MultiByteToWideChar(cp, 0, pS+offs, static_cast<int>(s), wtext, SIZEOFARRAY(wtext));
			if(ret > 0) {
				CatN(wtext, ret);
				offs += s;
			}
			else
				break;
		}
	}
	return *this;
}

int SLAPI SStringU::CopyToMb(int cp, SString & rBuf) const
{
	rBuf.Z();
	int   ok = 1;
	const size_t middle_buf_len = 2048;
	char  mbtext[middle_buf_len];
	if(cp == CP_UTF8) {
		ok = CopyToUtf8(rBuf, 1);
	}
	else {
		const size_t src_len = Len();
		for(size_t offs = 0; offs < src_len;) {
			const size_t s = MIN((src_len-offs), middle_buf_len/2);
			BOOL   inv_rep = 0;
			int    ret = ::WideCharToMultiByte(cp, 0, P_Buf+offs, (int)s, mbtext, sizeof(mbtext), "?", &inv_rep);
			if(ret > 0) {
				rBuf.CatN(mbtext, ret);
				offs += s;
			}
			else {
				ok = 0;
				break;
			}
		}
	}
	return ok;
}

SStringU & SLAPI SStringU::Sub(size_t startPos, size_t len, SStringU & rBuf) const
{
	rBuf.Z();
	if(startPos < Len()) {
		size_t len2 = MIN(len, Len()-startPos);
		rBuf.CopyFromN(P_Buf+startPos, len2);
	}
	return rBuf;
}
//
// Utility routine to tell whether a sequence of bytes is legal UTF-8.
// This must be called with the length pre-determined by the first byte.
// If not calling this from ConvertUTF8to*, then the length can be set by:
// length = trailingBytesForUTF8[*source]+1;
// and the sequence is illegal right away if there aren't that many bytes available.
// If presented with a length > 4, this returns false.
// The Unicode definition of UTF-8 goes up to 4-byte sequences.
//
// static
int FASTCALL SUnicode::IsLegalUtf8(const uint8 * pSource, size_t length)
{
	uint8 a;
	const uint8 * srcptr = pSource+length;
	switch(length) {
		default:
			return 0;
		// Everything else falls through when "true"...
		case 4:
			if((a = (*--srcptr)) < 0x80 || a > 0xBF)
				return 0;
		case 3:
			if((a = (*--srcptr)) < 0x80 || a > 0xBF)
				return 0;
		case 2:
			if((a = (*--srcptr)) > 0xBF)
				return 0;
			switch(*pSource) {
				// no fall-through in this inner switch
				case 0xE0:
					if(a < 0xA0)
						return 0;
					break;
				case 0xED:
					if(a > 0x9F)
						return 0;
					break;
				case 0xF0:
					if(a < 0x90)
						return 0;
					break;
				case 0xF4:
					if(a > 0x8F)
						return 0;
					break;
				default:
					if(a < 0x80)
						return 0;
			}
		case 1:
			if(*pSource >= 0x80 && *pSource < 0xC2)
				return 0;
	}
	return (*pSource > 0xF4) ? 0 : 1;
}

//static
uint FASTCALL SUnicode::Utf32ToUtf16(uint32 u32, wchar_t * pU16Buf)
{
	if(u32 < UNI_SUPPL_PLANE_START) {
		pU16Buf[0] = static_cast<wchar_t>(u32);
		return 1;
	}
	else {
		pU16Buf[0] = static_cast<wchar_t>(((u32 - UNI_SUPPL_PLANE_START) >> 10) + UNI_SUR_HIGH_START);
		pU16Buf[1] = static_cast<wchar_t>((u32 & 0x3ff) + UNI_SUR_LOW_START);
		return 2;
	}
}

//static
uint  FASTCALL SUnicode::Utf32ToUtf8(uint32 u32, char * pUtf8Buf)
{
	uint    k = 0;
	const uint16 u16l = static_cast<const uint16>(u32 & 0x0000ffff);
	if(u32 < 0x80) {
		pUtf8Buf[k++] = static_cast<char>(u32);
	}
	else if(u32 < 0x800) {
		pUtf8Buf[k++] = static_cast<char>(0xC0 | (u16l >> 6));
		pUtf8Buf[k++] = static_cast<char>(0x80 | (u16l & 0x3f));
	}
	else {
		if((u16l >= UNI_SUR_HIGH_START) && (u16l <= UNI_SUR_LOW_END/*SURROGATE_TRAIL_LAST*/)) {
			// @todo Я не уверен в правильности этого куска кода - надо тестировать!
			uint16 u16u = static_cast<uint16>(u32 >> 16);
			// Half a surrogate pair
			uint xch = 0x10000 + ((u16l & 0x3ff) << 10) + (u16u & 0x3ff);
			pUtf8Buf[k++] = static_cast<char>(0xF0 | (xch >> 18));
			pUtf8Buf[k++] = static_cast<char>(0x80 | ((xch >> 12) & 0x3f));
			pUtf8Buf[k++] = static_cast<char>(0x80 | ((xch >> 6) & 0x3f));
			pUtf8Buf[k++] = static_cast<char>(0x80 | (xch & 0x3f));
		}
		else {
			pUtf8Buf[k++] = static_cast<char>(0xE0 | (u16l >> 12));
			pUtf8Buf[k++] = static_cast<char>(0x80 | ((u16l >> 6) & 0x3f));
			pUtf8Buf[k++] = static_cast<char>(0x80 | (u16l & 0x3f));
		}
	}
	return k;
}

//static
uint FASTCALL SUnicode::Utf8Length(const wchar_t * pUcBuf, uint tlen)
{
	uint len = 0;
	for(uint i = 0; i < tlen && pUcBuf[i]; ) {
		uint uch = pUcBuf[i];
		if(uch < 0x80)
			len++;
		else if(uch < 0x800)
			len += 2;
		else if((uch >= UNI_SUR_HIGH_START) && (uch <= UNI_SUR_LOW_END)) {
			len += 4;
			i++;
		}
		else
			len += 3;
		i++;
	}
	return len;
}

int SString::IsAscii() const
{
	int    ok = 1;
	const  size_t _len = Len();
	for(size_t idx = 0; ok && idx < _len; idx++) {
		const uint8 * p = reinterpret_cast<const uint8 *>(P_Buf+idx);
		if(*p >= 0x80)
			ok = 0;
	}
	return ok;
}

int SString::IsLegalUtf8() const
{
	int    ok = 1;
	const  size_t _len = Len();
	for(size_t idx = 0; ok && idx < _len;) {
		const uint8 * p = reinterpret_cast<const uint8 *>(P_Buf+idx);
		const size_t extra = SUtfConst::TrailingBytesForUTF8[*p];
		if(extra == 0)
			idx++;
		else if(extra == 1) {
			if(p[1] != 0 && SUnicode::IsLegalUtf8(p, 2))
				idx += 2;
			else
				ok = 0;
		}
		else if(extra == 2) {
			if(p[1] != 0 && p[2] != 0 && SUnicode::IsLegalUtf8(p, 3))
				idx += 3;
			else
				ok = 0;
		}
		else {
			const size_t tail = (_len - idx);
			if((extra+1) <= tail && SUnicode::IsLegalUtf8(p, extra+1))
				idx += (1+extra);
			else
				ok = 0;
		}
	}
	return ok;
}
//
//
//
void FASTCALL SString::Cat_(const uint8 * pChr, size_t numChr)
{
	if(numChr) {
		const size_t new_len = (L ? L : 1) + numChr;
		if(new_len <= Size || Alloc(new_len)) {
			switch(numChr) {
				case 1:
					P_Buf[new_len-2] = pChr[0];
					break;
				case 2:
					P_Buf[new_len-3] = pChr[0];
					P_Buf[new_len-2] = pChr[1];
					break;
				case 3:
					P_Buf[new_len-4] = pChr[0];
					P_Buf[new_len-3] = pChr[1];
					P_Buf[new_len-2] = pChr[2];
					break;
				case 4:
					P_Buf[new_len-5] = pChr[0];
					P_Buf[new_len-4] = pChr[1];
					P_Buf[new_len-3] = pChr[2];
					P_Buf[new_len-2] = pChr[3];
					break;
				default:
					{
						char * p = &P_Buf[new_len-numChr-1];
						for(size_t i = 0; i < numChr; i++) {
							//P_Buf[new_len-2] = chr;
							p[i] = pChr[i];
						}
					}
					break;
			}
			P_Buf[new_len-1] = 0;
			L = new_len;
		}
	}
}

int SLAPI SString::CopyUtf8FromUnicode(const wchar_t * pSrc, const size_t len, int strictConversion)
{
	CopyFrom(0); // Обрезаем строку до пустой
	int    ok = 1;
	const  uint32 byteMask = 0xBF;
	const  uint32 byteMark = 0x80;
	uint8  temp_mb[8];
	for(size_t i = 0; i < len; i++) {
		uint32 ch = pSrc[i];
		// If we have a surrogate pair, convert to uint32 first
		if(ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
			// If the 16 bits following the high surrogate are in the p_src buffer...
			if(i < len) {
				uint32 ch2 = pSrc[i+1];
				// If it's a low surrogate, convert to uint32
				if(ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
					ch = ((ch - UNI_SUR_HIGH_START) << SUtfConst::HalfShift) + (ch2 - UNI_SUR_LOW_START) + SUtfConst::HalfBase;
					i++;
				}
				else if(strictConversion) { // it's an unpaired high surrogate
					CALLEXCEPT_S(SLERR_UTFCVT_ILLUTF16);
				}
			}
			else { // We don't have the 16 bits following the high surrogate
				CALLEXCEPT_S(SLERR_UTFCVT_SRCEXHAUSTED);
			}
		}
		else if(strictConversion) {
			// UTF-16 surrogate values are illegal in UTF-32
			if(ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
				CALLEXCEPT_S(SLERR_UTFCVT_ILLUTF16);
			}
		}
		// Figure out how many bytes the result will require
		if(ch < static_cast<uint32>(0x80)) {
			CatChar(static_cast<uint8>(ch));
		}
		else if(ch < static_cast<uint32>(0x800)) {
			temp_mb[0] = static_cast<uint8>((ch >> 6) | SUtfConst::FirstByteMark[2]);
			temp_mb[1] = static_cast<uint8>((ch | byteMark) & byteMask);
			Cat_(temp_mb, 2);
		}
		else if(ch < static_cast<uint32>(0x10000)) {
			temp_mb[0] = static_cast<uint8>((ch >> 12) | SUtfConst::FirstByteMark[3]);
			temp_mb[1] = static_cast<uint8>(((ch >> 6) | byteMark) & byteMask);
			temp_mb[2] = static_cast<uint8>((ch | byteMark) & byteMask);
			Cat_(temp_mb, 3);
		}
		else if(ch < static_cast<uint32>(0x110000)) {
			temp_mb[0] = static_cast<uint8>((ch >> 18) | SUtfConst::FirstByteMark[4]);
			temp_mb[1] = static_cast<uint8>(((ch >> 12) | byteMark) & byteMask);
			temp_mb[2] = static_cast<uint8>(((ch >> 6) | byteMark) & byteMask);
			temp_mb[3] = static_cast<uint8>((ch | byteMark) & byteMask);
			Cat_(temp_mb, 4);
		}
		else {
			ch = UNI_REPLACEMENT_CHAR;
			temp_mb[0] = static_cast<uint8>((ch >> 12) | SUtfConst::FirstByteMark[3]);
			temp_mb[1] = static_cast<uint8>(((ch >> 6) | byteMark) & byteMask);
			temp_mb[2] = static_cast<uint8>((ch | byteMark) & byteMask);
			Cat_(temp_mb, 3);
		}
	}
	CATCHZOK
	return ok;
}

int FASTCALL SStringU::CopyToUtf8(SString & rBuf, int strictConversion) const
{
	return rBuf.CopyUtf8FromUnicode(P_Buf, Len(), strictConversion);
}

int FASTCALL SStringU::CopyFromUtf8(const SString & rS) { return Helper_CopyFromUtf8(rS.cptr(), rS.Len(), 0, 0); }
int FASTCALL SStringU::CopyFromUtf8R(const SString & rS, size_t * pActualSrcSize) { return Helper_CopyFromUtf8(rS.cptr(), rS.Len(), 0, pActualSrcSize); }
int FASTCALL SStringU::CopyFromUtf8(const char * pSrc, size_t srcSize) { return Helper_CopyFromUtf8(pSrc, srcSize, 0, 0); }
int FASTCALL SStringU::CopyFromUtf8R(const char * pSrc, size_t srcSize, size_t * pActualSrcSize) { return Helper_CopyFromUtf8(pSrc, srcSize, 0, pActualSrcSize); }
int FASTCALL SStringU::CopyFromUtf8Strict(const char * pSrc, size_t srcSize) { return Helper_CopyFromUtf8(pSrc, srcSize, 1, 0); }

int FASTCALL SStringU::Helper_CopyFromUtf8(const char * pSrc, size_t srcSize, int strictConversion, size_t * pActualSrcSize)
{
	int    ok = 1;
	Trim(0);
	const uint8 * p_src = reinterpret_cast<const uint8 *>(pSrc);
	wchar_t line[1024];
	size_t line_ptr = 0;
	size_t i = 0;
	while(i < srcSize) {
		if(line_ptr > (SIZEOFARRAY(line)-2)) {
			CatN(line, line_ptr);
			line_ptr = 0;
		}
		uint32 ch = 0;
		uint16 target = 0;
		uint16 extra = SUtfConst::TrailingBytesForUTF8[p_src[i]];
		THROW_S((i + extra + 1) <= srcSize, SLERR_UTFCVT_SRCEXHAUSTED);
		THROW_S(SUnicode::IsLegalUtf8(p_src+i, extra+1), SLERR_UTFCVT_ILLUTF8);
		//
		// The cases all fall through. See "Note A" below.
	 	//
		switch(extra) {
	    	case 5: ch += p_src[i++]; ch <<= 6; // remember, illegal UTF-8
	    	case 4: ch += p_src[i++]; ch <<= 6; // remember, illegal UTF-8
	    	case 3: ch += p_src[i++]; ch <<= 6;
	    	case 2: ch += p_src[i++]; ch <<= 6;
	    	case 1: ch += p_src[i++]; ch <<= 6;
	    	case 0: ch += p_src[i++];
		}
		ch -= SUtfConst::OffsetsFromUTF8[extra];
		if(ch <= UNI_MAX_BMP) { // Target is a character <= 0xFFFF
	    	// UTF-16 surrogate values are illegal in UTF-32
	    	if(ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
				THROW_S(!strictConversion, SLERR_UTFCVT_ILLUTF8);
				line[line_ptr++] = static_cast<wchar_t>(UNI_REPLACEMENT_CHAR);
	    	}
			else
				line[line_ptr++] = static_cast<wchar_t>(ch); // normal case
		}
		else if(ch > UNI_MAX_UTF16) {
	    	THROW_S(!strictConversion, SLERR_UTFCVT_ILLUTF8);
			line[line_ptr++] = static_cast<wchar_t>(UNI_REPLACEMENT_CHAR);
		}
		else {
			// target is a character in range 0xFFFF - 0x10FFFF.
    		ch -= SUtfConst::HalfBase;
			line[line_ptr++] = static_cast<wchar_t>((ch >> SUtfConst::HalfShift) + UNI_SUR_HIGH_START);
    		line[line_ptr++] = static_cast<wchar_t>((ch & SUtfConst::HalfMask) + UNI_SUR_LOW_START);
		}
	}
	CATCHZOK
	CatN(line, line_ptr);
	ASSIGN_PTR(pActualSrcSize, i);
    return ok;
}

long SLAPI SStringU::ToLong() const
{
	if(L) {
		const wchar_t * p = P_Buf;
		while(oneof2(*p, L' ', L'\t'))
			p++;
		wchar_t * p_end = 0;
		if(p[0] == '0' && oneof2(p[1], L'x', L'X'))
			return static_cast<long>(wcstoul(p, &p_end, 16));
		else
			return static_cast<long>(wcstoul(p, &p_end, 10));
	}
	else
		return 0;
}

int64 SLAPI SStringU::ToInt64() const
{
	if(L) {
		const wchar_t * p = P_Buf;
		while(oneof2(*p, L' ', L'\t'))
			p++;
		return _wtoi64(p);
	}
	else
		return 0;
}

SStringU & SLAPI SStringU::ToUpper()
{
	if(P_Buf && L > 1) {
		for(size_t i = 0; i < (L-1); i++) {
			P_Buf[i] = UToUpperCase(P_Buf[i]);
		}
	}
	return *this;
}

SStringU & SLAPI SStringU::ToLower()
{
	if(P_Buf) {
		for(size_t i = 0; i < (L-1); i++) {
			P_Buf[i] = UToLowerCase(P_Buf[i]);
		}
	}
	return *this;
}

int SLAPI SStringU::AnalyzeCase() const
{
	int    tc = tcEmpty;
	if(L) {
		int    all_lwr = 1;
		int    all_upr = 1;
		int    all_alpha = 1;
		int    capital = 0;
		const wchar_t * p = P_Buf;
		while(oneof2(*p, L' ', L'\t'))
			p++;
		if(iswalpha(*p) && iswupper(*p)) {
			capital = 1;
			p++;
		}
		for(; *p; p++) {
			if(iswalpha(*p)) {
				if(iswupper(*p))
					all_lwr = 0;
				else if(iswlower(*p))
					all_upr = 0;
			}
			else
				all_alpha = 0;
		}
		if(all_lwr)
			if(capital)
				tc = tcCapital;
			else
				tc = tcLow;
		else if(all_upr)
			tc = tcUpp;
		else
			tc = tcMix;
	}
	return tc;
}
//
//
//
int SPathStruc::Invariant(SInvariantParam * pInvP)
{
	S_INVARIANT_PROLOG(pInvP);
	SString path, new_path;
	const long f1 = Flags;
	Merge(new_path);
	Split(new_path);
	const long f2 = Flags;
	Merge(path);
	if(f1 != f2) {
		S_ERROR_P(pInvP);
		pInvP->MsgBuf.Printf("SPathStruc: Flags != ret Split(Merge(%s))", new_path.cptr());
	}
	if(new_path.CmpNC(path) != 0) {
		S_ERROR_P(pInvP);
		pInvP->MsgBuf.Printf("SPathStruc: %s != Merge(Split(Merge(%s)))", path.cptr(), new_path.cptr());
	}
	if(pInvP && pInvP->LocalOk) {
		if(Flags & fUNC)
			pInvP->MsgBuf.Cat("UNC").CatDiv('-', 1);
		pInvP->MsgBuf.CatEq("Drv", Drv).CatDiv(';', 2).CatEq("Dir", Dir).
			CatDiv(';', 2).CatEq("Nam", Nam).CatDiv(';', 2).CatEq("Ext", Ext);
	}
	S_INVARIANT_EPILOG(pInvP);
}

SPathStruc::SPathStruc() : Flags(0)
{
}

SPathStruc::SPathStruc(const char * pPath) : Flags(0)
{
	if(!isempty(pPath))
		Split(pPath);
}

SPathStruc & SPathStruc::Copy(const SPathStruc * pS, long flags)
{
	if(flags & fDrv) {
		if(pS) {
			Drv = pS->Drv;
			SETFLAGBYSAMPLE(Flags, fUNC, pS->Flags);
		}
		else {
			Flags &= ~fUNC;
			Drv.Z();
		}
	}
	if(pS) {
		if(flags & fDir)
			Dir = pS->Dir;
		if(flags & fNam)
			Nam = pS->Nam;
		if(flags & fExt)
			Ext = pS->Ext;
	}
	else {
		if(flags & fDir)
			Dir.Z();
		if(flags & fNam)
			Nam.Z();
		if(flags & fExt)
			Ext.Z();
	}
	return *this;
}

SPathStruc & SPathStruc::Clear()
{
	Flags = 0;
	Drv.Z();
	Dir.Z();
	Nam.Z();
	Ext.Z();
	return *this;
}

int SPathStruc::Merge(const SPathStruc * pPattern, long patternFlags, SString & rBuf)
{
	return Copy(pPattern, patternFlags).Merge(rBuf);
}

int FASTCALL SPathStruc::Merge(SString & rBuf) const
{
	rBuf.Z();
	if(Flags & fUNC)
		rBuf.CatCharN('\\', 2);
	int    last = Drv.Last();
	if(last) {
		rBuf.Cat(Drv);
		if(!(Flags & fUNC) && last != ':')
			rBuf.CatChar(':');
	}
	last = Dir.Last();
	if(last) {
		rBuf.Cat(Dir);
		if(last != '\\' && last != '/')
			rBuf.CatChar('\\');
	}
	rBuf.Cat(Nam);
	if(Ext.NotEmpty()) {
		if(Ext.C(0) != '.')
			rBuf.Dot();
		rBuf.Cat(Ext);
	}
	return 1;
}

int FASTCALL SPathStruc::Merge(long mergeFlags, SString & rBuf) const
{
	rBuf.Z();
	int    last = 0;
	if(!mergeFlags || mergeFlags & fDrv) {
		if(Flags & fUNC)
			rBuf.CatCharN('\\', 2);
		last = Drv.Last();
		if(last) {
			rBuf.Cat(Drv);
			if(!(Flags & fUNC) && last != ':')
				rBuf.CatChar(':');
		}
	}
	if(!mergeFlags || mergeFlags & fDir) {
		last = Dir.Last();
		if(last) {
			rBuf.Cat(Dir);
			if(last != '\\' && last != '/')
				rBuf.CatChar('\\');
		}
	}
	if(!mergeFlags || mergeFlags & fNam)
		rBuf.Cat(Nam);
	if((!mergeFlags || mergeFlags & fExt) && Ext.NotEmpty()) {
		if(Ext.C(0) != '.')
			rBuf.Dot();
		rBuf.Cat(Ext);
	}
	return 1;
}

void FASTCALL SPathStruc::Split(const char * pPath)
{
	// \\machine_name\dir\nam.ext
	// c:\dir\nam.ext
	//
	Flags = 0;
	Drv.Z();
	Dir.Z();
	Nam.Z();
	Ext.Z();
	if(pPath) {
		int    fname_as_dir_part = 0;
		SString temp_buf = pPath;
		temp_buf.Strip();
		SStrScan scan(temp_buf);
		const  char * p = 0;
		if(strpbrk(pPath, "*?") == 0) { // @v9.1.8 Следующая проверка возможна только если в пути нет wildcard-символов
			SString & r_buf = SLS.AcquireRvlStr(); // @v10.0.0
			fname_as_dir_part = IsDirectory((r_buf = pPath).RmvLastSlash());
		}
		if(scan.Is("\\\\") || scan.Is("//")) {
			Flags |= fUNC;
			scan.Incr(2);
			p = strpbrk(scan, "\\/");
			scan.SetLen(p ? (p-scan) : sstrlen(scan));
			scan.Get(Drv);
			Flags |= fDrv;
			scan.IncrLen();
		}
		else {
			p = sstrchr(scan, ':');
			if(p) {
				scan.SetLen(p-scan);
				scan.Get(Drv);
				Flags |= fDrv;
				scan.IncrLen();
				scan.Incr(); // Пропускаем двоеточие
			}
		}
		{
			scan.Push();
			const size_t start = scan.GetOffs();
			while((p = strpbrk(scan, "\\/")) != 0) {
				scan.Incr(p-scan+1);
			}
			scan.SetLen(scan.GetOffs() - start);
			scan.Pop();
		}
		scan.Get(Dir);
		if(Dir.Len())
			Flags |= fDir;
		scan.IncrLen();
		//
		//
		//
		{
			scan.Push();
			const size_t start = scan.GetOffs();
			const char * p_last_dot = 0;
			while((p = sstrchr(scan, '.')) != 0) {
				p_last_dot = p;
				scan.Incr(p-scan+1);
			}
			if(p_last_dot) {
				Ext = p_last_dot + 1;
				Flags |= fExt;
				scan.SetLen(scan.GetOffs() - start - 1);
				//scan.Offs = start;
				scan.Pop();
				scan.Get(Nam);
			}
			else {
				Nam = scan;
				scan.Pop();
			}
		}
		if(fname_as_dir_part) {
			Dir.SetLastSlash().Cat(Nam);
			Nam.Z();
		}
		if(Nam.Len())
			Flags |= fNam;
	}
}

//static
uint FASTCALL SPathStruc::GetExt(const SString & rPath, SString * pExt)
{
	ASSIGN_PTR(pExt, 0);
	size_t p = rPath.Len();
	while(p){
		const char _c = rPath.C(--p);
		if(oneof2(_c, '/', '\\')) {
			p = 0;
			break;
		}
		else if(_c == '.') {
			p++;
			if(pExt) {
				for(size_t n = p; n < rPath.Len(); n++) {
                    pExt->CatChar(rPath.C(n));
				}
			}
			break;
		}
	}
	return (uint)p;
}

// static
int FASTCALL SPathStruc::ReplaceExt(SString & rPath, const char * pExt, int force)
{
	int    ok = -1;
	rPath.Strip();
	size_t p = rPath.Len();
	if(p) {
		while(ok < 0 && p--) {
			if(rPath.C(p) == '.' && (p == 0 || rPath.C(p-1) != '.') && rPath.C(p+1) != '.') { // @v10.1.7 (&& (p == 0 || rPath.C(p-1) != '.') && rPath.C(p+1) != '.')
				if(force) {
					rPath.Trim(p+1);
					if(pExt && pExt[0] == '.')
						rPath.Cat(pExt+1);
					else
						rPath.Cat(pExt);
				}
				ok = 1;
			}
		}
		if(ok < 0) {
			if(pExt && pExt[0] == '.')
				rPath.Cat(pExt);
			else
				rPath.CatChar('.').Cat(pExt);
			ok = 1;
		}
	}
	return ok;
	/*
	SPathStruc ps;
	ps.Split(rPath);
	if(force || ps.Ext.Empty()) {
		ps.Ext = pExt;
		ps.Merge(0, rPath);
	}
	else
		ok = -1;
	return 1;
	*/
}

//static
int SPathStruc::ReplacePath(SString & rPath, const char * pNewPath, int force)
{
	int    ok = -1;
	SPathStruc ps(rPath);
	if(force || (ps.Drv.Empty() && ps.Dir.Empty())) {
		if(isempty(pNewPath)) {
			ps.Merge(SPathStruc::fNam|SPathStruc::fExt, rPath);
		}
		else {
			SString new_path = pNewPath;
			SPathStruc ps2(new_path.SetLastSlash());
			ps2.Merge(&ps, SPathStruc::fNam|SPathStruc::fExt, rPath);
		}
		ok = 1;
	}
	return ok;
}
//
//
//
static int FASTCALL SPathIsUNC(const char * pPath)
{
	return (pPath && (pPath[0]=='\\') && (pPath[1]=='\\') && (pPath[2] != '?')) ? 1 : 0;
}

static int SPathCommonPrefix(const char * pFile1, const char * pFile2, char * achPath)
{
	size_t iLen = 0;
	const char * lpszIter1 = pFile1;
	const char * lpszIter2 = pFile2;
	ASSIGN_PTR(achPath, 0);
	if(pFile1 && pFile2) {
		// Handle roots first
		if(SPathIsUNC(pFile1)) {
			if(!SPathIsUNC(pFile2))
				return 0;
			lpszIter1 += 2;
			lpszIter2 += 2;
		}
		else if(SPathIsUNC(pFile2))
			return 0; /* Know already lpszFile1 is not UNC */
		do {
			/* Update len */
			if((!*lpszIter1 || *lpszIter1 == '\\') && (!*lpszIter2 || *lpszIter2 == '\\'))
				iLen = lpszIter1 - pFile1; /* Common to this point */
			if(!*lpszIter1 || (tolower(*lpszIter1) != tolower(*lpszIter2)))
				break; /* Strings differ at this point */
			lpszIter1++;
			lpszIter2++;
		} while (1);
		if(iLen == 2)
			iLen++; /* Feature/Bug compatible with Win32 */
		if(iLen && achPath) {
			memcpy(achPath, pFile1, iLen);
			achPath[iLen] = '\0';
		}
	}
	return (int)iLen;
}

static int FASTCALL SPathRemoveFileSpec(SString & rPath)
{
	size_t file_spec_pos = 0;
	size_t path_pos = 0;
	int    bModified = 0;
	if(rPath.Len()) {
		/* Skip directory or UNC path */
		if(oneof2(rPath[path_pos], '\\', '/'))
			file_spec_pos = ++path_pos;
		if(oneof2(rPath[path_pos], '\\', '/'))
			file_spec_pos = ++path_pos;
		while(rPath[path_pos]) {
			if(oneof2(rPath[path_pos], '\\', '/'))
				file_spec_pos = path_pos; /* Skip dir */
			else if(rPath[path_pos] == ':') {
				file_spec_pos = ++path_pos; /* Skip drive */
				if(oneof2(rPath[path_pos], '\\', '/'))
					file_spec_pos++;
			}
			path_pos++;
		}
		if(file_spec_pos < rPath.Len()) {
			rPath.Trim(file_spec_pos);
			bModified = 1;
		}
	}
	return bModified;
}

static const char * FASTCALL SPathFindNextComponent(const char * pPath)
{
	const size_t len = sstrlen(pPath);
	if(len) {
		const char * p_slash = static_cast<const char *>(memchr(pPath, '\\', len));
		SETIFZ(p_slash, static_cast<const char *>(memchr(pPath, '/', len)));
		if(p_slash) {
			if(oneof2(p_slash[1], '\\', '/'))
				p_slash++;
			return (p_slash + 1);
		}
		else
			return (pPath + len);
	}
	else
		return 0;
}

//static
SString & FASTCALL SPathStruc::NormalizePath(const char * pPath, long flags, SString & rNormalizedPath)
{
	(rNormalizedPath = pPath).Strip();
	if(flags & npfOEM)
		rNormalizedPath.Transf(CTRANSF_INNER_TO_OUTER);
	if(!(flags & npfKeepCase)) {
		if(flags & npfUpper) {
			rNormalizedPath.ToUpper1251();
		}
		else {
			rNormalizedPath.ToLower1251();
		}
	}
	if(flags & npfSlash) {
		rNormalizedPath.ReplaceChar('\\', '/');
	}
	else {
		rNormalizedPath.ReplaceChar('/', '\\');
	}
	return rNormalizedPath;
}

//static
int SPathStruc::GetRelativePath(const char * lpszFrom, uint dwAttrFrom, const char * lpszTo, uint dwAttrTo, SString & rPath)
{
	static const char * szPrevDirSlash = "..\\";
	static const char * szPrevDir = "..";
	int    ok = 1;
	DWORD dwLen;
	SString from(lpszFrom);
	SString to(lpszTo);
	rPath.Z();
	if(lpszFrom && lpszTo) {
		if(!(dwAttrFrom & FILE_ATTRIBUTE_DIRECTORY))
			SPathRemoveFileSpec(from);
		if(!(dwAttrFrom & FILE_ATTRIBUTE_DIRECTORY))
			SPathRemoveFileSpec(to);
		// Paths can only be relative if they have a common root
		if(!(dwLen = SPathCommonPrefix(from, to, 0)))
			ok = 0;
		else {
			// Strip off lpszFrom components to the root, by adding "..\"
			lpszFrom = from + dwLen;
			if(!*lpszFrom)
				rPath = ".";
			if(oneof2(*lpszFrom, '\\', '/'))
				lpszFrom++;
			while(*lpszFrom) {
				lpszFrom = SPathFindNextComponent(lpszFrom);
				rPath.Cat(*lpszFrom ? szPrevDirSlash : szPrevDir);
			}
			// From the root add the components of lpszTo
			lpszTo += dwLen;
			//
			// We check lpszTo[-1] to avoid skipping end of string. See the notes for
			// this function.
			//
			if(*lpszTo && lpszTo[-1]) {
				if(!oneof2(*lpszTo, '\\', '/'))
					lpszTo--;
				rPath.Cat(lpszTo);
			}
		}
	}
	else
		ok = 0;
	return ok;
}
//
//
//
SStringPool::SStringPool() : TSCollection <SString> ()
{
}

SStringPool::~SStringPool()
{
	freeAll();
}

const BitArray & SStringPool::GetMap() const
{
	return BusyList;
}

SString * FASTCALL SStringPool::Alloc(uint * pPos)
{
	//assert(getCount() == BusyList.getCount()); // @debug
	size_t p = BusyList.findFirst(0, 0);
	if(!p) {
		BusyList.insert(1);
		p = BusyList.getCount();
		insert(new SString);
	}
	BusyList.set(p-1, 1);
	ASSIGN_PTR(pPos, (uint)(p-1));
	return at((uint)(p-1));
}

int FASTCALL SStringPool::Free(uint pos)
{
	//assert(getCount() == BusyList.getCount());
	if(pos < getCount()) {
		//assert(BusyList.get(pos) == 1);
		BusyList.set(pos, 0);
		return 1;
	}
	else
		return 0;
}

int FASTCALL SStringPool::Free(const SString * pS)
{
	assert(getCount() == BusyList.getCount());
	uint   i = getCount();
	if(i) do {
		--i;
		if(at(i) == pS)
			return Free(i);
	} while(i);
	return 0;
}

int FASTCALL SStringPool::Free(const BitArray & rMap)
{
	size_t i = rMap.getCount();
	if(i) do {
		--i;
		if(!rMap.get(i))
			Free((uint)i);
	} while(i);
	return 1;
}
//
//
//
// @Muxa {
//
// Функции для изменения регистра unicode символов
//
#define UCHR_STATUS_C	0x01
#define UCHR_STATUS_F	0x02
#define UCHR_STATUS_S	0x03
#define UCHR_STATUS_T	0x04

struct CaseFoldingItem {
	wchar_t Code;
	wchar_t ToCode;
	uint8   Status_;
};

static const CaseFoldingItem u_case_folding_tbl[] = {
	{ 0x0041,	0x0061,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A
	{ 0x0042,	0x0062,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER B
	{ 0x0043,	0x0063,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER C
	{ 0x0044,	0x0064,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D
	{ 0x0045,	0x0065,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E
	{ 0x0046,	0x0066,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER F
	{ 0x0047,	0x0067,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER G
	{ 0x0048,	0x0068,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER H
	{ 0x0049,	0x0069,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I
	{ 0x0049,	0x0131,		UCHR_STATUS_T },	// # LATIN CAPITAL LETTER I
	{ 0x004A,	0x006A,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER J
	{ 0x004B,	0x006B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K
	{ 0x004C,	0x006C,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L
	{ 0x004D,	0x006D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER M
	{ 0x004E,	0x006E,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N
	{ 0x004F,	0x006F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O
	{ 0x0050,	0x0070,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER P
	{ 0x0051,	0x0071,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Q
	{ 0x0052,	0x0072,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R
	{ 0x0053,	0x0073,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S
	{ 0x0054,	0x0074,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T
	{ 0x0055,	0x0075,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U
	{ 0x0056,	0x0076,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER V
	{ 0x0057,	0x0077,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER W
	{ 0x0058,	0x0078,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER X
	{ 0x0059,	0x0079,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y
	{ 0x005A,	0x007A,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Z
	{ 0x00B5,	0x03BC,		UCHR_STATUS_C },	// # MICRO SIGN
	{ 0x00C0,	0x00E0,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH GRAVE
	{ 0x00C1,	0x00E1,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH ACUTE
	{ 0x00C2,	0x00E2,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH CIRCUMFLEX
	{ 0x00C3,	0x00E3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH TILDE
	{ 0x00C4,	0x00E4,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH DIAERESIS
	{ 0x00C5,	0x00E5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH RING ABOVE
	{ 0x00C6,	0x00E6,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER AE
	{ 0x00C7,	0x00E7,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER C WITH CEDILLA
	{ 0x00C8,	0x00E8,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH GRAVE
	{ 0x00C9,	0x00E9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH ACUTE
	{ 0x00CA,	0x00EA,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH CIRCUMFLEX
	{ 0x00CB,	0x00EB,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH DIAERESIS
	{ 0x00CC,	0x00EC,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH GRAVE
	{ 0x00CD,	0x00ED,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH ACUTE
	{ 0x00CE,	0x00EE,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH CIRCUMFLEX
	{ 0x00CF,	0x00EF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH DIAERESIS
	{ 0x00D0,	0x00F0,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER ETH
	{ 0x00D1,	0x00F1,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH TILDE
	{ 0x00D2,	0x00F2,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH GRAVE
	{ 0x00D3,	0x00F3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH ACUTE
	{ 0x00D4,	0x00F4,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH CIRCUMFLEX
	{ 0x00D5,	0x00F5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH TILDE
	{ 0x00D6,	0x00F6,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH DIAERESIS
	{ 0x00D8,	0x00F8,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH STROKE
	{ 0x00D9,	0x00F9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH GRAVE
	{ 0x00DA,	0x00FA,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH ACUTE
	{ 0x00DB,	0x00FB,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH CIRCUMFLEX
	{ 0x00DC,	0x00FC,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH DIAERESIS
	{ 0x00DD,	0x00FD,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH ACUTE
	{ 0x00DE,	0x00FE,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER THORN
	{ 0x0100,	0x0101,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH MACRON
	{ 0x0102,	0x0103,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH BREVE
	{ 0x0104,	0x0105,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH OGONEK
	{ 0x0106,	0x0107,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER C WITH ACUTE
	{ 0x0108,	0x0109,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER C WITH CIRCUMFLEX
	{ 0x010A,	0x010B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER C WITH DOT ABOVE
	{ 0x010C,	0x010D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER C WITH CARON
	{ 0x010E,	0x010F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D WITH CARON
	{ 0x0110,	0x0111,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D WITH STROKE
	{ 0x0112,	0x0113,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH MACRON
	{ 0x0114,	0x0115,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH BREVE
	{ 0x0116,	0x0117,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH DOT ABOVE
	{ 0x0118,	0x0119,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH OGONEK
	{ 0x011A,	0x011B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH CARON
	{ 0x011C,	0x011D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER G WITH CIRCUMFLEX
	{ 0x011E,	0x011F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER G WITH BREVE
	{ 0x0120,	0x0121,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER G WITH DOT ABOVE
	{ 0x0122,	0x0123,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER G WITH CEDILLA
	{ 0x0124,	0x0125,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER H WITH CIRCUMFLEX
	{ 0x0126,	0x0127,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER H WITH STROKE
	{ 0x0128,	0x0129,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH TILDE
	{ 0x012A,	0x012B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH MACRON
	{ 0x012C,	0x012D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH BREVE
	{ 0x012E,	0x012F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH OGONEK
	{ 0x0130,	0x0069,		UCHR_STATUS_T },	// # LATIN CAPITAL LETTER I WITH DOT ABOVE
	{ 0x0132,	0x0133,		UCHR_STATUS_C },	// # LATIN CAPITAL LIGATURE IJ
	{ 0x0134,	0x0135,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER J WITH CIRCUMFLEX
	{ 0x0136,	0x0137,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K WITH CEDILLA
	{ 0x0139,	0x013A,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH ACUTE
	{ 0x013B,	0x013C,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH CEDILLA
	{ 0x013D,	0x013E,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH CARON
	{ 0x013F,	0x0140,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH MIDDLE DOT
	{ 0x0141,	0x0142,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH STROKE
	{ 0x0143,	0x0144,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH ACUTE
	{ 0x0145,	0x0146,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH CEDILLA
	{ 0x0147,	0x0148,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH CARON
	{ 0x014A,	0x014B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER ENG
	{ 0x014C,	0x014D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH MACRON
	{ 0x014E,	0x014F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH BREVE
	{ 0x0150,	0x0151,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
	{ 0x0152,	0x0153,		UCHR_STATUS_C },	// # LATIN CAPITAL LIGATURE OE
	{ 0x0154,	0x0155,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH ACUTE
	{ 0x0156,	0x0157,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH CEDILLA
	{ 0x0158,	0x0159,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH CARON
	{ 0x015A,	0x015B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH ACUTE
	{ 0x015C,	0x015D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH CIRCUMFLEX
	{ 0x015E,	0x015F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH CEDILLA
	{ 0x0160,	0x0161,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH CARON
	{ 0x0162,	0x0163,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T WITH CEDILLA
	{ 0x0164,	0x0165,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T WITH CARON
	{ 0x0166,	0x0167,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T WITH STROKE
	{ 0x0168,	0x0169,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH TILDE
	{ 0x016A,	0x016B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH MACRON
	{ 0x016C,	0x016D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH BREVE
	{ 0x016E,	0x016F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH RING ABOVE
	{ 0x0170,	0x0171,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
	{ 0x0172,	0x0173,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH OGONEK
	{ 0x0174,	0x0175,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER W WITH CIRCUMFLEX
	{ 0x0176,	0x0177,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH CIRCUMFLEX
	{ 0x0178,	0x00FF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH DIAERESIS
	{ 0x0179,	0x017A,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Z WITH ACUTE
	{ 0x017B,	0x017C,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Z WITH DOT ABOVE
	{ 0x017D,	0x017E,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Z WITH CARON
	{ 0x017F,	0x0073,		UCHR_STATUS_C },	// # LATIN SMALL LETTER LONG S
	{ 0x0181,	0x0253,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER B WITH HOOK
	{ 0x0182,	0x0183,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER B WITH TOPBAR
	{ 0x0184,	0x0185,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TONE SIX
	{ 0x0186,	0x0254,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER OPEN O
	{ 0x0187,	0x0188,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER C WITH HOOK
	{ 0x0189,	0x0256,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER AFRICAN D
	{ 0x018A,	0x0257,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D WITH HOOK
	{ 0x018B,	0x018C,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D WITH TOPBAR
	{ 0x018E,	0x01DD,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER REVERSED E
	{ 0x018F,	0x0259,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER SCHWA
	{ 0x0190,	0x025B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER OPEN E
	{ 0x0191,	0x0192,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER F WITH HOOK
	{ 0x0193,	0x0260,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER G WITH HOOK
	{ 0x0194,	0x0263,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER GAMMA
	{ 0x0196,	0x0269,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER IOTA
	{ 0x0197,	0x0268,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH STROKE
	{ 0x0198,	0x0199,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K WITH HOOK
	{ 0x019C,	0x026F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TURNED M
	{ 0x019D,	0x0272,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH LEFT HOOK
	{ 0x019F,	0x0275,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH MIDDLE TILDE
	{ 0x01A0,	0x01A1,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH HORN
	{ 0x01A2,	0x01A3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER OI
	{ 0x01A4,	0x01A5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER P WITH HOOK
	{ 0x01A6,	0x0280,		UCHR_STATUS_C },	// # LATIN LETTER YR
	{ 0x01A7,	0x01A8,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TONE TWO
	{ 0x01A9,	0x0283,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER ESH
	{ 0x01AC,	0x01AD,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T WITH HOOK
	{ 0x01AE,	0x0288,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T WITH RETROFLEX HOOK
	{ 0x01AF,	0x01B0,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH HORN
	{ 0x01B1,	0x028A,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER UPSILON
	{ 0x01B2,	0x028B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER V WITH HOOK
	{ 0x01B3,	0x01B4,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH HOOK
	{ 0x01B5,	0x01B6,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Z WITH STROKE
	{ 0x01B7,	0x0292,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER EZH
	{ 0x01B8,	0x01B9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER EZH REVERSED
	{ 0x01BC,	0x01BD,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TONE FIVE
	{ 0x01C4,	0x01C6,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER DZ WITH CARON
	{ 0x01C5,	0x01C6,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON
	{ 0x01C7,	0x01C9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER LJ
	{ 0x01C8,	0x01C9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH SMALL LETTER J
	{ 0x01CA,	0x01CC,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER NJ
	{ 0x01CB,	0x01CC,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH SMALL LETTER J
	{ 0x01CD,	0x01CE,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH CARON
	{ 0x01CF,	0x01D0,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH CARON
	{ 0x01D1,	0x01D2,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH CARON
	{ 0x01D3,	0x01D4,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH CARON
	{ 0x01D5,	0x01D6,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON
	{ 0x01D7,	0x01D8,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE
	{ 0x01D9,	0x01DA,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON
	{ 0x01DB,	0x01DC,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE
	{ 0x01DE,	0x01DF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON
	{ 0x01E0,	0x01E1,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON
	{ 0x01E2,	0x01E3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER AE WITH MACRON
	{ 0x01E4,	0x01E5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER G WITH STROKE
	{ 0x01E6,	0x01E7,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER G WITH CARON
	{ 0x01E8,	0x01E9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K WITH CARON
	{ 0x01EA,	0x01EB,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH OGONEK
	{ 0x01EC,	0x01ED,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH OGONEK AND MACRON
	{ 0x01EE,	0x01EF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER EZH WITH CARON
	{ 0x01F1,	0x01F3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER DZ
	{ 0x01F2,	0x01F3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D WITH SMALL LETTER Z
	{ 0x01F4,	0x01F5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER G WITH ACUTE
	{ 0x01F6,	0x0195,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER HWAIR
	{ 0x01F7,	0x01BF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER WYNN
	{ 0x01F8,	0x01F9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH GRAVE
	{ 0x01FA,	0x01FB,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE
	{ 0x01FC,	0x01FD,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER AE WITH ACUTE
	{ 0x01FE,	0x01FF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH STROKE AND ACUTE
	{ 0x0200,	0x0201,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH DOUBLE GRAVE
	{ 0x0202,	0x0203,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH INVERTED BREVE
	{ 0x0204,	0x0205,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH DOUBLE GRAVE
	{ 0x0206,	0x0207,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH INVERTED BREVE
	{ 0x0208,	0x0209,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH DOUBLE GRAVE
	{ 0x020A,	0x020B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH INVERTED BREVE
	{ 0x020C,	0x020D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH DOUBLE GRAVE
	{ 0x020E,	0x020F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH INVERTED BREVE
	{ 0x0210,	0x0211,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH DOUBLE GRAVE
	{ 0x0212,	0x0213,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH INVERTED BREVE
	{ 0x0214,	0x0215,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH DOUBLE GRAVE
	{ 0x0216,	0x0217,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH INVERTED BREVE
	{ 0x0218,	0x0219,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH COMMA BELOW
	{ 0x021A,	0x021B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T WITH COMMA BELOW
	{ 0x021C,	0x021D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER YOGH
	{ 0x021E,	0x021F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER H WITH CARON
	{ 0x0220,	0x019E,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH LONG RIGHT LEG
	{ 0x0222,	0x0223,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER OU
	{ 0x0224,	0x0225,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Z WITH HOOK
	{ 0x0226,	0x0227,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH DOT ABOVE
	{ 0x0228,	0x0229,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH CEDILLA
	{ 0x022A,	0x022B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON
	{ 0x022C,	0x022D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH TILDE AND MACRON
	{ 0x022E,	0x022F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH DOT ABOVE
	{ 0x0230,	0x0231,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON
	{ 0x0232,	0x0233,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH MACRON
	{ 0x023A,	0x2C65,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH STROKE
	{ 0x023B,	0x023C,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER C WITH STROKE
	{ 0x023D,	0x019A,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH BAR
	{ 0x023E,	0x2C66,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T WITH DIAGONAL STROKE
	{ 0x0241,	0x0242,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER GLOTTAL STOP
	{ 0x0243,	0x0180,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER B WITH STROKE
	{ 0x0244,	0x0289,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U BAR
	{ 0x0245,	0x028C,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TURNED V
	{ 0x0246,	0x0247,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH STROKE
	{ 0x0248,	0x0249,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER J WITH STROKE
	{ 0x024A,	0x024B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER SMALL Q WITH HOOK TAIL
	{ 0x024C,	0x024D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH STROKE
	{ 0x024E,	0x024F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH STROKE
	{ 0x0345,	0x03B9,		UCHR_STATUS_C },	// # COMBINING GREEK YPOGEGRAMMENI
	{ 0x0370,	0x0371,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER HETA
	{ 0x0372,	0x0373,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ARCHAIC SAMPI
	{ 0x0376,	0x0377,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER PAMPHYLIAN DIGAMMA
	{ 0x0386,	0x03AC,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH TONOS
	{ 0x0388,	0x03AD,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER EPSILON WITH TONOS
	{ 0x0389,	0x03AE,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA WITH TONOS
	{ 0x038A,	0x03AF,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH TONOS
	{ 0x038C,	0x03CC,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMICRON WITH TONOS
	{ 0x038E,	0x03CD,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER UPSILON WITH TONOS
	{ 0x038F,	0x03CE,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA WITH TONOS
	{ 0x0391,	0x03B1,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA
	{ 0x0392,	0x03B2,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER BETA
	{ 0x0393,	0x03B3,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER GAMMA
	{ 0x0394,	0x03B4,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER DELTA
	{ 0x0395,	0x03B5,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER EPSILON
	{ 0x0396,	0x03B6,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ZETA
	{ 0x0397,	0x03B7,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA
	{ 0x0398,	0x03B8,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER THETA
	{ 0x0399,	0x03B9,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA
	{ 0x039A,	0x03BA,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER KAPPA
	{ 0x039B,	0x03BB,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER LAMDA
	{ 0x039C,	0x03BC,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER MU
	{ 0x039D,	0x03BD,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER NU
	{ 0x039E,	0x03BE,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER XI
	{ 0x039F,	0x03BF,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMICRON
	{ 0x03A0,	0x03C0,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER PI
	{ 0x03A1,	0x03C1,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER RHO
	{ 0x03A3,	0x03C3,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER SIGMA
	{ 0x03A4,	0x03C4,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER TAU
	{ 0x03A5,	0x03C5,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER UPSILON
	{ 0x03A6,	0x03C6,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER PHI
	{ 0x03A7,	0x03C7,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER CHI
	{ 0x03A8,	0x03C8,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER PSI
	{ 0x03A9,	0x03C9,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA
	{ 0x03AA,	0x03CA,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
	{ 0x03AB,	0x03CB,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
	{ 0x03C2,	0x03C3,		UCHR_STATUS_C },	// # GREEK SMALL LETTER FINAL SIGMA
	{ 0x03CF,	0x03D7,		UCHR_STATUS_C },	// # GREEK CAPITAL KAI SYMBOL
	{ 0x03D0,	0x03B2,		UCHR_STATUS_C },	// # GREEK BETA SYMBOL
	{ 0x03D1,	0x03B8,		UCHR_STATUS_C },	// # GREEK THETA SYMBOL
	{ 0x03D5,	0x03C6,		UCHR_STATUS_C },	// # GREEK PHI SYMBOL
	{ 0x03D6,	0x03C0,		UCHR_STATUS_C },	// # GREEK PI SYMBOL
	{ 0x03D8,	0x03D9,		UCHR_STATUS_C },	// # GREEK LETTER ARCHAIC KOPPA
	{ 0x03DA,	0x03DB,		UCHR_STATUS_C },	// # GREEK LETTER STIGMA
	{ 0x03DC,	0x03DD,		UCHR_STATUS_C },	// # GREEK LETTER DIGAMMA
	{ 0x03DE,	0x03DF,		UCHR_STATUS_C },	// # GREEK LETTER KOPPA
	{ 0x03E0,	0x03E1,		UCHR_STATUS_C },	// # GREEK LETTER SAMPI
	{ 0x03E2,	0x03E3,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER SHEI
	{ 0x03E4,	0x03E5,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER FEI
	{ 0x03E6,	0x03E7,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER KHEI
	{ 0x03E8,	0x03E9,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER HORI
	{ 0x03EA,	0x03EB,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER GANGIA
	{ 0x03EC,	0x03ED,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER SHIMA
	{ 0x03EE,	0x03EF,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER DEI
	{ 0x03F0,	0x03BA,		UCHR_STATUS_C },	// # GREEK KAPPA SYMBOL
	{ 0x03F1,	0x03C1,		UCHR_STATUS_C },	// # GREEK RHO SYMBOL
	{ 0x03F4,	0x03B8,		UCHR_STATUS_C },	// # GREEK CAPITAL THETA SYMBOL
	{ 0x03F5,	0x03B5,		UCHR_STATUS_C },	// # GREEK LUNATE EPSILON SYMBOL
	{ 0x03F7,	0x03F8,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER SHO
	{ 0x03F9,	0x03F2,		UCHR_STATUS_C },	// # GREEK CAPITAL LUNATE SIGMA SYMBOL
	{ 0x03FA,	0x03FB,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER SAN
	{ 0x03FD,	0x037B,		UCHR_STATUS_C },	// # GREEK CAPITAL REVERSED LUNATE SIGMA SYMBOL
	{ 0x03FE,	0x037C,		UCHR_STATUS_C },	// # GREEK CAPITAL DOTTED LUNATE SIGMA SYMBOL
	{ 0x03FF,	0x037D,		UCHR_STATUS_C },	// # GREEK CAPITAL REVERSED DOTTED LUNATE SIGMA SYMBOL
	{ 0x0400,	0x0450,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IE WITH GRAVE
	{ 0x0401,	0x0451,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IO
	{ 0x0402,	0x0452,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER DJE
	{ 0x0403,	0x0453,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER GJE
	{ 0x0404,	0x0454,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER UKRAINIAN IE
	{ 0x0405,	0x0455,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER DZE
	{ 0x0406,	0x0456,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
	{ 0x0407,	0x0457,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER YI
	{ 0x0408,	0x0458,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER JE
	{ 0x0409,	0x0459,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER LJE
	{ 0x040A,	0x045A,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER NJE
	{ 0x040B,	0x045B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER TSHE
	{ 0x040C,	0x045C,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KJE
	{ 0x040D,	0x045D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER I WITH GRAVE
	{ 0x040E,	0x045E,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SHORT U
	{ 0x040F,	0x045F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER DZHE
	{ 0x0410,	0x0430,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER A
	{ 0x0411,	0x0431,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER BE
	{ 0x0412,	0x0432,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER VE
	{ 0x0413,	0x0433,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER GHE
	{ 0x0414,	0x0434,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER DE
	{ 0x0415,	0x0435,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IE
	{ 0x0416,	0x0436,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ZHE
	{ 0x0417,	0x0437,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ZE
	{ 0x0418,	0x0438,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER I
	{ 0x0419,	0x0439,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SHORT I
	{ 0x041A,	0x043A,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KA
	{ 0x041B,	0x043B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EL
	{ 0x041C,	0x043C,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EM
	{ 0x041D,	0x043D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EN
	{ 0x041E,	0x043E,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER O
	{ 0x041F,	0x043F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER PE
	{ 0x0420,	0x0440,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ER
	{ 0x0421,	0x0441,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ES
	{ 0x0422,	0x0442,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER TE
	{ 0x0423,	0x0443,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER U
	{ 0x0424,	0x0444,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EF
	{ 0x0425,	0x0445,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER HA
	{ 0x0426,	0x0446,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER TSE
	{ 0x0427,	0x0447,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER CHE
	{ 0x0428,	0x0448,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SHA
	{ 0x0429,	0x0449,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SHCHA
	{ 0x042A,	0x044A,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER HARD SIGN
	{ 0x042B,	0x044B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER YERU
	{ 0x042C,	0x044C,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SOFT SIGN
	{ 0x042D,	0x044D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER E
	{ 0x042E,	0x044E,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER YU
	{ 0x042F,	0x044F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER YA
	{ 0x0460,	0x0461,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER OMEGA
	{ 0x0462,	0x0463,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER YAT
	{ 0x0464,	0x0465,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IOTIFIED E
	{ 0x0466,	0x0467,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER LITTLE YUS
	{ 0x0468,	0x0469,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IOTIFIED LITTLE YUS
	{ 0x046A,	0x046B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER BIG YUS
	{ 0x046C,	0x046D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IOTIFIED BIG YUS
	{ 0x046E,	0x046F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KSI
	{ 0x0470,	0x0471,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER PSI
	{ 0x0472,	0x0473,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER FITA
	{ 0x0474,	0x0475,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IZHITSA
	{ 0x0476,	0x0477,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IZHITSA WITH DOUBLE GRAVE ACCENT
	{ 0x0478,	0x0479,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER UK
	{ 0x047A,	0x047B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ROUND OMEGA
	{ 0x047C,	0x047D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER OMEGA WITH TITLO
	{ 0x047E,	0x047F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER OT
	{ 0x0480,	0x0481,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KOPPA
	{ 0x048A,	0x048B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SHORT I WITH TAIL
	{ 0x048C,	0x048D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SEMISOFT SIGN
	{ 0x048E,	0x048F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ER WITH TICK
	{ 0x0490,	0x0491,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER GHE WITH UPTURN
	{ 0x0492,	0x0493,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER GHE WITH STROKE
	{ 0x0494,	0x0495,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER GHE WITH MIDDLE HOOK
	{ 0x0496,	0x0497,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ZHE WITH DESCENDER
	{ 0x0498,	0x0499,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ZE WITH DESCENDER
	{ 0x049A,	0x049B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KA WITH DESCENDER
	{ 0x049C,	0x049D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KA WITH VERTICAL STROKE
	{ 0x049E,	0x049F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KA WITH STROKE
	{ 0x04A0,	0x04A1,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER BASHKIR KA
	{ 0x04A2,	0x04A3,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EN WITH DESCENDER
	{ 0x04A4,	0x04A5,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LIGATURE EN GHE
	{ 0x04A6,	0x04A7,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER PE WITH MIDDLE HOOK
	{ 0x04A8,	0x04A9,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ABKHASIAN HA
	{ 0x04AA,	0x04AB,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ES WITH DESCENDER
	{ 0x04AC,	0x04AD,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER TE WITH DESCENDER
	{ 0x04AE,	0x04AF,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER STRAIGHT U
	{ 0x04B0,	0x04B1,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER STRAIGHT U WITH STROKE
	{ 0x04B2,	0x04B3,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER HA WITH DESCENDER
	{ 0x04B4,	0x04B5,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LIGATURE TE TSE
	{ 0x04B6,	0x04B7,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER CHE WITH DESCENDER
	{ 0x04B8,	0x04B9,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER CHE WITH VERTICAL STROKE
	{ 0x04BA,	0x04BB,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SHHA
	{ 0x04BC,	0x04BD,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ABKHASIAN CHE
	{ 0x04BE,	0x04BF,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ABKHASIAN CHE WITH DESCENDER
	{ 0x04C0,	0x04CF,		UCHR_STATUS_C },	// # CYRILLIC LETTER PALOCHKA
	{ 0x04C1,	0x04C2,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ZHE WITH BREVE
	{ 0x04C3,	0x04C4,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KA WITH HOOK
	{ 0x04C5,	0x04C6,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EL WITH TAIL
	{ 0x04C7,	0x04C8,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EN WITH HOOK
	{ 0x04C9,	0x04CA,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EN WITH TAIL
	{ 0x04CB,	0x04CC,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KHAKASSIAN CHE
	{ 0x04CD,	0x04CE,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EM WITH TAIL
	{ 0x04D0,	0x04D1,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER A WITH BREVE
	{ 0x04D2,	0x04D3,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER A WITH DIAERESIS
	{ 0x04D4,	0x04D5,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LIGATURE A IE
	{ 0x04D6,	0x04D7,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IE WITH BREVE
	{ 0x04D8,	0x04D9,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SCHWA
	{ 0x04DA,	0x04DB,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SCHWA WITH DIAERESIS
	{ 0x04DC,	0x04DD,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ZHE WITH DIAERESIS
	{ 0x04DE,	0x04DF,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ZE WITH DIAERESIS
	{ 0x04E0,	0x04E1,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ABKHASIAN DZE
	{ 0x04E2,	0x04E3,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER I WITH MACRON
	{ 0x04E4,	0x04E5,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER I WITH DIAERESIS
	{ 0x04E6,	0x04E7,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER O WITH DIAERESIS
	{ 0x04E8,	0x04E9,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER BARRED O
	{ 0x04EA,	0x04EB,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER BARRED O WITH DIAERESIS
	{ 0x04EC,	0x04ED,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER E WITH DIAERESIS
	{ 0x04EE,	0x04EF,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER U WITH MACRON
	{ 0x04F0,	0x04F1,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER U WITH DIAERESIS
	{ 0x04F2,	0x04F3,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER U WITH DOUBLE ACUTE
	{ 0x04F4,	0x04F5,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER CHE WITH DIAERESIS
	{ 0x04F6,	0x04F7,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER GHE WITH DESCENDER
	{ 0x04F8,	0x04F9,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER YERU WITH DIAERESIS
	{ 0x04FA,	0x04FB,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER GHE WITH STROKE AND HOOK
	{ 0x04FC,	0x04FD,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER HA WITH HOOK
	{ 0x04FE,	0x04FF,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER HA WITH STROKE
	{ 0x0500,	0x0501,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KOMI DE
	{ 0x0502,	0x0503,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KOMI DJE
	{ 0x0504,	0x0505,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KOMI ZJE
	{ 0x0506,	0x0507,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KOMI DZJE
	{ 0x0508,	0x0509,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KOMI LJE
	{ 0x050A,	0x050B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KOMI NJE
	{ 0x050C,	0x050D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KOMI SJE
	{ 0x050E,	0x050F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER KOMI TJE
	{ 0x0510,	0x0511,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER REVERSED ZE
	{ 0x0512,	0x0513,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EL WITH HOOK
	{ 0x0514,	0x0515,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER LHA
	{ 0x0516,	0x0517,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER RHA
	{ 0x0518,	0x0519,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER YAE
	{ 0x051A,	0x051B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER QA
	{ 0x051C,	0x051D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER WE
	{ 0x051E,	0x051F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ALEUT KA
	{ 0x0520,	0x0521,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EL WITH MIDDLE HOOK
	{ 0x0522,	0x0523,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER EN WITH MIDDLE HOOK
	{ 0x0524,	0x0525,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER PE WITH DESCENDER
	{ 0x0526,	0x0527,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SHHA WITH DESCENDER
	{ 0x0531,	0x0561,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER AYB
	{ 0x0532,	0x0562,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER BEN
	{ 0x0533,	0x0563,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER GIM
	{ 0x0534,	0x0564,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER DA
	{ 0x0535,	0x0565,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER ECH
	{ 0x0536,	0x0566,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER ZA
	{ 0x0537,	0x0567,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER EH
	{ 0x0538,	0x0568,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER ET
	{ 0x0539,	0x0569,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER TO
	{ 0x053A,	0x056A,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER ZHE
	{ 0x053B,	0x056B,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER INI
	{ 0x053C,	0x056C,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER LIWN
	{ 0x053D,	0x056D,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER XEH
	{ 0x053E,	0x056E,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER CA
	{ 0x053F,	0x056F,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER KEN
	{ 0x0540,	0x0570,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER HO
	{ 0x0541,	0x0571,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER JA
	{ 0x0542,	0x0572,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER GHAD
	{ 0x0543,	0x0573,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER CHEH
	{ 0x0544,	0x0574,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER MEN
	{ 0x0545,	0x0575,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER YI
	{ 0x0546,	0x0576,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER NOW
	{ 0x0547,	0x0577,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER SHA
	{ 0x0548,	0x0578,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER VO
	{ 0x0549,	0x0579,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER CHA
	{ 0x054A,	0x057A,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER PEH
	{ 0x054B,	0x057B,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER JHEH
	{ 0x054C,	0x057C,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER RA
	{ 0x054D,	0x057D,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER SEH
	{ 0x054E,	0x057E,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER VEW
	{ 0x054F,	0x057F,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER TIWN
	{ 0x0550,	0x0580,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER REH
	{ 0x0551,	0x0581,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER CO
	{ 0x0552,	0x0582,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER YIWN
	{ 0x0553,	0x0583,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER PIWR
	{ 0x0554,	0x0584,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER KEH
	{ 0x0555,	0x0585,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER OH
	{ 0x0556,	0x0586,		UCHR_STATUS_C },	// # ARMENIAN CAPITAL LETTER FEH
	{ 0x10A0,	0x2D00,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER AN
	{ 0x10A1,	0x2D01,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER BAN
	{ 0x10A2,	0x2D02,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER GAN
	{ 0x10A3,	0x2D03,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER DON
	{ 0x10A4,	0x2D04,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER EN
	{ 0x10A5,	0x2D05,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER VIN
	{ 0x10A6,	0x2D06,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER ZEN
	{ 0x10A7,	0x2D07,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER TAN
	{ 0x10A8,	0x2D08,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER IN
	{ 0x10A9,	0x2D09,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER KAN
	{ 0x10AA,	0x2D0A,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER LAS
	{ 0x10AB,	0x2D0B,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER MAN
	{ 0x10AC,	0x2D0C,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER NAR
	{ 0x10AD,	0x2D0D,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER ON
	{ 0x10AE,	0x2D0E,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER PAR
	{ 0x10AF,	0x2D0F,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER ZHAR
	{ 0x10B0,	0x2D10,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER RAE
	{ 0x10B1,	0x2D11,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER SAN
	{ 0x10B2,	0x2D12,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER TAR
	{ 0x10B3,	0x2D13,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER UN
	{ 0x10B4,	0x2D14,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER PHAR
	{ 0x10B5,	0x2D15,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER KHAR
	{ 0x10B6,	0x2D16,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER GHAN
	{ 0x10B7,	0x2D17,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER QAR
	{ 0x10B8,	0x2D18,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER SHIN
	{ 0x10B9,	0x2D19,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER CHIN
	{ 0x10BA,	0x2D1A,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER CAN
	{ 0x10BB,	0x2D1B,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER JIL
	{ 0x10BC,	0x2D1C,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER CIL
	{ 0x10BD,	0x2D1D,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER CHAR
	{ 0x10BE,	0x2D1E,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER XAN
	{ 0x10BF,	0x2D1F,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER JHAN
	{ 0x10C0,	0x2D20,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER HAE
	{ 0x10C1,	0x2D21,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER HE
	{ 0x10C2,	0x2D22,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER HIE
	{ 0x10C3,	0x2D23,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER WE
	{ 0x10C4,	0x2D24,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER HAR
	{ 0x10C5,	0x2D25,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER HOE
	{ 0x10C7,	0x2D27,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER YN
	{ 0x10CD,	0x2D2D,		UCHR_STATUS_C },	// # GEORGIAN CAPITAL LETTER AEN
	{ 0x1E00,	0x1E01,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH RING BELOW
	{ 0x1E02,	0x1E03,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER B WITH DOT ABOVE
	{ 0x1E04,	0x1E05,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER B WITH DOT BELOW
	{ 0x1E06,	0x1E07,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER B WITH LINE BELOW
	{ 0x1E08,	0x1E09,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER C WITH CEDILLA AND ACUTE
	{ 0x1E0A,	0x1E0B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D WITH DOT ABOVE
	{ 0x1E0C,	0x1E0D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D WITH DOT BELOW
	{ 0x1E0E,	0x1E0F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D WITH LINE BELOW
	{ 0x1E10,	0x1E11,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D WITH CEDILLA
	{ 0x1E12,	0x1E13,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER D WITH CIRCUMFLEX BELOW
	{ 0x1E14,	0x1E15,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH MACRON AND GRAVE
	{ 0x1E16,	0x1E17,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH MACRON AND ACUTE
	{ 0x1E18,	0x1E19,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH CIRCUMFLEX BELOW
	{ 0x1E1A,	0x1E1B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH TILDE BELOW
	{ 0x1E1C,	0x1E1D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH CEDILLA AND BREVE
	{ 0x1E1E,	0x1E1F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER F WITH DOT ABOVE
	{ 0x1E20,	0x1E21,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER G WITH MACRON
	{ 0x1E22,	0x1E23,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER H WITH DOT ABOVE
	{ 0x1E24,	0x1E25,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER H WITH DOT BELOW
	{ 0x1E26,	0x1E27,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER H WITH DIAERESIS
	{ 0x1E28,	0x1E29,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER H WITH CEDILLA
	{ 0x1E2A,	0x1E2B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER H WITH BREVE BELOW
	{ 0x1E2C,	0x1E2D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH TILDE BELOW
	{ 0x1E2E,	0x1E2F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH DIAERESIS AND ACUTE
	{ 0x1E30,	0x1E31,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K WITH ACUTE
	{ 0x1E32,	0x1E33,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K WITH DOT BELOW
	{ 0x1E34,	0x1E35,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K WITH LINE BELOW
	{ 0x1E36,	0x1E37,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH DOT BELOW
	{ 0x1E38,	0x1E39,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH DOT BELOW AND MACRON
	{ 0x1E3A,	0x1E3B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH LINE BELOW
	{ 0x1E3C,	0x1E3D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH CIRCUMFLEX BELOW
	{ 0x1E3E,	0x1E3F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER M WITH ACUTE
	{ 0x1E40,	0x1E41,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER M WITH DOT ABOVE
	{ 0x1E42,	0x1E43,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER M WITH DOT BELOW
	{ 0x1E44,	0x1E45,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH DOT ABOVE
	{ 0x1E46,	0x1E47,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH DOT BELOW
	{ 0x1E48,	0x1E49,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH LINE BELOW
	{ 0x1E4A,	0x1E4B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH CIRCUMFLEX BELOW
	{ 0x1E4C,	0x1E4D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH TILDE AND ACUTE
	{ 0x1E4E,	0x1E4F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH TILDE AND DIAERESIS
	{ 0x1E50,	0x1E51,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH MACRON AND GRAVE
	{ 0x1E52,	0x1E53,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH MACRON AND ACUTE
	{ 0x1E54,	0x1E55,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER P WITH ACUTE
	{ 0x1E56,	0x1E57,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER P WITH DOT ABOVE
	{ 0x1E58,	0x1E59,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH DOT ABOVE
	{ 0x1E5A,	0x1E5B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH DOT BELOW
	{ 0x1E5C,	0x1E5D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH DOT BELOW AND MACRON
	{ 0x1E5E,	0x1E5F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH LINE BELOW
	{ 0x1E60,	0x1E61,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH DOT ABOVE
	{ 0x1E62,	0x1E63,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH DOT BELOW
	{ 0x1E64,	0x1E65,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH ACUTE AND DOT ABOVE
	{ 0x1E66,	0x1E67,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH CARON AND DOT ABOVE
	{ 0x1E68,	0x1E69,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH DOT BELOW AND DOT ABOVE
	{ 0x1E6A,	0x1E6B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T WITH DOT ABOVE
	{ 0x1E6C,	0x1E6D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T WITH DOT BELOW
	{ 0x1E6E,	0x1E6F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T WITH LINE BELOW
	{ 0x1E70,	0x1E71,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER T WITH CIRCUMFLEX BELOW
	{ 0x1E72,	0x1E73,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH DIAERESIS BELOW
	{ 0x1E74,	0x1E75,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH TILDE BELOW
	{ 0x1E76,	0x1E77,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH CIRCUMFLEX BELOW
	{ 0x1E78,	0x1E79,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH TILDE AND ACUTE
	{ 0x1E7A,	0x1E7B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH MACRON AND DIAERESIS
	{ 0x1E7C,	0x1E7D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER V WITH TILDE
	{ 0x1E7E,	0x1E7F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER V WITH DOT BELOW
	{ 0x1E80,	0x1E81,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER W WITH GRAVE
	{ 0x1E82,	0x1E83,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER W WITH ACUTE
	{ 0x1E84,	0x1E85,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER W WITH DIAERESIS
	{ 0x1E86,	0x1E87,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER W WITH DOT ABOVE
	{ 0x1E88,	0x1E89,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER W WITH DOT BELOW
	{ 0x1E8A,	0x1E8B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER X WITH DOT ABOVE
	{ 0x1E8C,	0x1E8D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER X WITH DIAERESIS
	{ 0x1E8E,	0x1E8F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH DOT ABOVE
	{ 0x1E90,	0x1E91,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Z WITH CIRCUMFLEX
	{ 0x1E92,	0x1E93,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Z WITH DOT BELOW
	{ 0x1E94,	0x1E95,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Z WITH LINE BELOW
	{ 0x1E9B,	0x1E61,		UCHR_STATUS_C },	// # LATIN SMALL LETTER LONG S WITH DOT ABOVE
	{ 0x1E9E,	0x00DF,		UCHR_STATUS_S },	// # LATIN CAPITAL LETTER SHARP S
	{ 0x1EA0,	0x1EA1,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH DOT BELOW
	{ 0x1EA2,	0x1EA3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH HOOK ABOVE
	{ 0x1EA4,	0x1EA5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND ACUTE
	{ 0x1EA6,	0x1EA7,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND GRAVE
	{ 0x1EA8,	0x1EA9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE
	{ 0x1EAA,	0x1EAB,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND TILDE
	{ 0x1EAC,	0x1EAD,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND DOT BELOW
	{ 0x1EAE,	0x1EAF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH BREVE AND ACUTE
	{ 0x1EB0,	0x1EB1,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH BREVE AND GRAVE
	{ 0x1EB2,	0x1EB3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH BREVE AND HOOK ABOVE
	{ 0x1EB4,	0x1EB5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH BREVE AND TILDE
	{ 0x1EB6,	0x1EB7,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER A WITH BREVE AND DOT BELOW
	{ 0x1EB8,	0x1EB9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH DOT BELOW
	{ 0x1EBA,	0x1EBB,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH HOOK ABOVE
	{ 0x1EBC,	0x1EBD,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH TILDE
	{ 0x1EBE,	0x1EBF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND ACUTE
	{ 0x1EC0,	0x1EC1,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND GRAVE
	{ 0x1EC2,	0x1EC3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE
	{ 0x1EC4,	0x1EC5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND TILDE
	{ 0x1EC6,	0x1EC7,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND DOT BELOW
	{ 0x1EC8,	0x1EC9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH HOOK ABOVE
	{ 0x1ECA,	0x1ECB,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER I WITH DOT BELOW
	{ 0x1ECC,	0x1ECD,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH DOT BELOW
	{ 0x1ECE,	0x1ECF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH HOOK ABOVE
	{ 0x1ED0,	0x1ED1,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND ACUTE
	{ 0x1ED2,	0x1ED3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND GRAVE
	{ 0x1ED4,	0x1ED5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE
	{ 0x1ED6,	0x1ED7,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND TILDE
	{ 0x1ED8,	0x1ED9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND DOT BELOW
	{ 0x1EDA,	0x1EDB,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH HORN AND ACUTE
	{ 0x1EDC,	0x1EDD,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH HORN AND GRAVE
	{ 0x1EDE,	0x1EDF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH HORN AND HOOK ABOVE
	{ 0x1EE0,	0x1EE1,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH HORN AND TILDE
	{ 0x1EE2,	0x1EE3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH HORN AND DOT BELOW
	{ 0x1EE4,	0x1EE5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH DOT BELOW
	{ 0x1EE6,	0x1EE7,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH HOOK ABOVE
	{ 0x1EE8,	0x1EE9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH HORN AND ACUTE
	{ 0x1EEA,	0x1EEB,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH HORN AND GRAVE
	{ 0x1EEC,	0x1EED,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH HORN AND HOOK ABOVE
	{ 0x1EEE,	0x1EEF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH HORN AND TILDE
	{ 0x1EF0,	0x1EF1,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER U WITH HORN AND DOT BELOW
	{ 0x1EF2,	0x1EF3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH GRAVE
	{ 0x1EF4,	0x1EF5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH DOT BELOW
	{ 0x1EF6,	0x1EF7,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH HOOK ABOVE
	{ 0x1EF8,	0x1EF9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH TILDE
	{ 0x1EFA,	0x1EFB,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER MIDDLE-WELSH LL
	{ 0x1EFC,	0x1EFD,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER MIDDLE-WELSH V
	{ 0x1EFE,	0x1EFF,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Y WITH LOOP
	{ 0x1F08,	0x1F00,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH PSILI
	{ 0x1F09,	0x1F01,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH DASIA
	{ 0x1F0A,	0x1F02,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA
	{ 0x1F0B,	0x1F03,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA
	{ 0x1F0C,	0x1F04,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA
	{ 0x1F0D,	0x1F05,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA
	{ 0x1F0E,	0x1F06,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI
	{ 0x1F0F,	0x1F07,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI
	{ 0x1F18,	0x1F10,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER EPSILON WITH PSILI
	{ 0x1F19,	0x1F11,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER EPSILON WITH DASIA
	{ 0x1F1A,	0x1F12,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER EPSILON WITH PSILI AND VARIA
	{ 0x1F1B,	0x1F13,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER EPSILON WITH DASIA AND VARIA
	{ 0x1F1C,	0x1F14,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER EPSILON WITH PSILI AND OXIA
	{ 0x1F1D,	0x1F15,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER EPSILON WITH DASIA AND OXIA
	{ 0x1F28,	0x1F20,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA WITH PSILI
	{ 0x1F29,	0x1F21,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA WITH DASIA
	{ 0x1F2A,	0x1F22,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA
	{ 0x1F2B,	0x1F23,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA
	{ 0x1F2C,	0x1F24,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA
	{ 0x1F2D,	0x1F25,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA
	{ 0x1F2E,	0x1F26,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI
	{ 0x1F2F,	0x1F27,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI
	{ 0x1F38,	0x1F30,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH PSILI
	{ 0x1F39,	0x1F31,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH DASIA
	{ 0x1F3A,	0x1F32,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH PSILI AND VARIA
	{ 0x1F3B,	0x1F33,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH DASIA AND VARIA
	{ 0x1F3C,	0x1F34,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH PSILI AND OXIA
	{ 0x1F3D,	0x1F35,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH DASIA AND OXIA
	{ 0x1F3E,	0x1F36,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH PSILI AND PERISPOMENI
	{ 0x1F3F,	0x1F37,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH DASIA AND PERISPOMENI
	{ 0x1F48,	0x1F40,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMICRON WITH PSILI
	{ 0x1F49,	0x1F41,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMICRON WITH DASIA
	{ 0x1F4A,	0x1F42,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMICRON WITH PSILI AND VARIA
	{ 0x1F4B,	0x1F43,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMICRON WITH DASIA AND VARIA
	{ 0x1F4C,	0x1F44,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMICRON WITH PSILI AND OXIA
	{ 0x1F4D,	0x1F45,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMICRON WITH DASIA AND OXIA
	{ 0x1F59,	0x1F51,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER UPSILON WITH DASIA
	{ 0x1F5B,	0x1F53,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER UPSILON WITH DASIA AND VARIA
	{ 0x1F5D,	0x1F55,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER UPSILON WITH DASIA AND OXIA
	{ 0x1F5F,	0x1F57,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER UPSILON WITH DASIA AND PERISPOMENI
	{ 0x1F68,	0x1F60,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA WITH PSILI
	{ 0x1F69,	0x1F61,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA WITH DASIA
	{ 0x1F6A,	0x1F62,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA
	{ 0x1F6B,	0x1F63,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA
	{ 0x1F6C,	0x1F64,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA
	{ 0x1F6D,	0x1F65,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA
	{ 0x1F6E,	0x1F66,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI
	{ 0x1F6F,	0x1F67,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI
	{ 0x1F88,	0x1F80,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ALPHA WITH PSILI AND PROSGEGRAMMENI
	{ 0x1F89,	0x1F81,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ALPHA WITH DASIA AND PROSGEGRAMMENI
	{ 0x1F8A,	0x1F82,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA AND PROSGEGRAMMENI
	{ 0x1F8B,	0x1F83,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA AND PROSGEGRAMMENI
	{ 0x1F8C,	0x1F84,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA AND PROSGEGRAMMENI
	{ 0x1F8D,	0x1F85,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA AND PROSGEGRAMMENI
	{ 0x1F8E,	0x1F86,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
	{ 0x1F8F,	0x1F87,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
	{ 0x1F98,	0x1F90,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ETA WITH PSILI AND PROSGEGRAMMENI
	{ 0x1F99,	0x1F91,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ETA WITH DASIA AND PROSGEGRAMMENI
	{ 0x1F9A,	0x1F92,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA AND PROSGEGRAMMENI
	{ 0x1F9B,	0x1F93,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA AND PROSGEGRAMMENI
	{ 0x1F9C,	0x1F94,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA AND PROSGEGRAMMENI
	{ 0x1F9D,	0x1F95,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA AND PROSGEGRAMMENI
	{ 0x1F9E,	0x1F96,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
	{ 0x1F9F,	0x1F97,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
	{ 0x1FA8,	0x1FA0,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER OMEGA WITH PSILI AND PROSGEGRAMMENI
	{ 0x1FA9,	0x1FA1,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER OMEGA WITH DASIA AND PROSGEGRAMMENI
	{ 0x1FAA,	0x1FA2,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA AND PROSGEGRAMMENI
	{ 0x1FAB,	0x1FA3,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA AND PROSGEGRAMMENI
	{ 0x1FAC,	0x1FA4,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA AND PROSGEGRAMMENI
	{ 0x1FAD,	0x1FA5,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA AND PROSGEGRAMMENI
	{ 0x1FAE,	0x1FA6,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
	{ 0x1FAF,	0x1FA7,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
	{ 0x1FB8,	0x1FB0,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH VRACHY
	{ 0x1FB9,	0x1FB1,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH MACRON
	{ 0x1FBA,	0x1F70,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH VARIA
	{ 0x1FBB,	0x1F71,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ALPHA WITH OXIA
	{ 0x1FBC,	0x1FB3,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ALPHA WITH PROSGEGRAMMENI
	{ 0x1FBE,	0x03B9,		UCHR_STATUS_C },	// # GREEK PROSGEGRAMMENI
	{ 0x1FC8,	0x1F72,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER EPSILON WITH VARIA
	{ 0x1FC9,	0x1F73,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER EPSILON WITH OXIA
	{ 0x1FCA,	0x1F74,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA WITH VARIA
	{ 0x1FCB,	0x1F75,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER ETA WITH OXIA
	{ 0x1FCC,	0x1FC3,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER ETA WITH PROSGEGRAMMENI
	{ 0x1FD8,	0x1FD0,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH VRACHY
	{ 0x1FD9,	0x1FD1,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH MACRON
	{ 0x1FDA,	0x1F76,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH VARIA
	{ 0x1FDB,	0x1F77,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER IOTA WITH OXIA
	{ 0x1FE8,	0x1FE0,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER UPSILON WITH VRACHY
	{ 0x1FE9,	0x1FE1,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER UPSILON WITH MACRON
	{ 0x1FEA,	0x1F7A,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER UPSILON WITH VARIA
	{ 0x1FEB,	0x1F7B,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER UPSILON WITH OXIA
	{ 0x1FEC,	0x1FE5,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER RHO WITH DASIA
	{ 0x1FF8,	0x1F78,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMICRON WITH VARIA
	{ 0x1FF9,	0x1F79,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMICRON WITH OXIA
	{ 0x1FFA,	0x1F7C,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA WITH VARIA
	{ 0x1FFB,	0x1F7D,		UCHR_STATUS_C },	// # GREEK CAPITAL LETTER OMEGA WITH OXIA
	{ 0x1FFC,	0x1FF3,		UCHR_STATUS_S },	// # GREEK CAPITAL LETTER OMEGA WITH PROSGEGRAMMENI
	{ 0x2126,	0x03C9,		UCHR_STATUS_C },	// # OHM SIGN
	{ 0x212A,	0x006B,		UCHR_STATUS_C },	// # KELVIN SIGN
	{ 0x212B,	0x00E5,		UCHR_STATUS_C },	// # ANGSTROM SIGN
	{ 0x2132,	0x214E,		UCHR_STATUS_C },	// # TURNED CAPITAL F
	{ 0x2160,	0x2170,		UCHR_STATUS_C },	// # ROMAN NUMERAL ONE
	{ 0x2161,	0x2171,		UCHR_STATUS_C },	// # ROMAN NUMERAL TWO
	{ 0x2162,	0x2172,		UCHR_STATUS_C },	// # ROMAN NUMERAL THREE
	{ 0x2163,	0x2173,		UCHR_STATUS_C },	// # ROMAN NUMERAL FOUR
	{ 0x2164,	0x2174,		UCHR_STATUS_C },	// # ROMAN NUMERAL FIVE
	{ 0x2165,	0x2175,		UCHR_STATUS_C },	// # ROMAN NUMERAL SIX
	{ 0x2166,	0x2176,		UCHR_STATUS_C },	// # ROMAN NUMERAL SEVEN
	{ 0x2167,	0x2177,		UCHR_STATUS_C },	// # ROMAN NUMERAL EIGHT
	{ 0x2168,	0x2178,		UCHR_STATUS_C },	// # ROMAN NUMERAL NINE
	{ 0x2169,	0x2179,		UCHR_STATUS_C },	// # ROMAN NUMERAL TEN
	{ 0x216A,	0x217A,		UCHR_STATUS_C },	// # ROMAN NUMERAL ELEVEN
	{ 0x216B,	0x217B,		UCHR_STATUS_C },	// # ROMAN NUMERAL TWELVE
	{ 0x216C,	0x217C,		UCHR_STATUS_C },	// # ROMAN NUMERAL FIFTY
	{ 0x216D,	0x217D,		UCHR_STATUS_C },	// # ROMAN NUMERAL ONE HUNDRED
	{ 0x216E,	0x217E,		UCHR_STATUS_C },	// # ROMAN NUMERAL FIVE HUNDRED
	{ 0x216F,	0x217F,		UCHR_STATUS_C },	// # ROMAN NUMERAL ONE THOUSAND
	{ 0x2183,	0x2184,		UCHR_STATUS_C },	// # ROMAN NUMERAL REVERSED ONE HUNDRED
	{ 0x24B6,	0x24D0,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER A
	{ 0x24B7,	0x24D1,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER B
	{ 0x24B8,	0x24D2,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER C
	{ 0x24B9,	0x24D3,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER D
	{ 0x24BA,	0x24D4,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER E
	{ 0x24BB,	0x24D5,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER F
	{ 0x24BC,	0x24D6,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER G
	{ 0x24BD,	0x24D7,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER H
	{ 0x24BE,	0x24D8,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER I
	{ 0x24BF,	0x24D9,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER J
	{ 0x24C0,	0x24DA,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER K
	{ 0x24C1,	0x24DB,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER L
	{ 0x24C2,	0x24DC,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER M
	{ 0x24C3,	0x24DD,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER N
	{ 0x24C4,	0x24DE,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER O
	{ 0x24C5,	0x24DF,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER P
	{ 0x24C6,	0x24E0,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER Q
	{ 0x24C7,	0x24E1,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER R
	{ 0x24C8,	0x24E2,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER S
	{ 0x24C9,	0x24E3,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER T
	{ 0x24CA,	0x24E4,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER U
	{ 0x24CB,	0x24E5,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER V
	{ 0x24CC,	0x24E6,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER W
	{ 0x24CD,	0x24E7,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER X
	{ 0x24CE,	0x24E8,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER Y
	{ 0x24CF,	0x24E9,		UCHR_STATUS_C },	// # CIRCLED LATIN CAPITAL LETTER Z
	{ 0x2C00,	0x2C30,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER AZU
	{ 0x2C01,	0x2C31,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER BUKY
	{ 0x2C02,	0x2C32,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER VEDE
	{ 0x2C03,	0x2C33,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER GLAGOLI
	{ 0x2C04,	0x2C34,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER DOBRO
	{ 0x2C05,	0x2C35,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER YESTU
	{ 0x2C06,	0x2C36,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER ZHIVETE
	{ 0x2C07,	0x2C37,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER DZELO
	{ 0x2C08,	0x2C38,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER ZEMLJA
	{ 0x2C09,	0x2C39,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER IZHE
	{ 0x2C0A,	0x2C3A,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER INITIAL IZHE
	{ 0x2C0B,	0x2C3B,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER I
	{ 0x2C0C,	0x2C3C,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER DJERVI
	{ 0x2C0D,	0x2C3D,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER KAKO
	{ 0x2C0E,	0x2C3E,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER LJUDIJE
	{ 0x2C0F,	0x2C3F,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER MYSLITE
	{ 0x2C10,	0x2C40,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER NASHI
	{ 0x2C11,	0x2C41,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER ONU
	{ 0x2C12,	0x2C42,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER POKOJI
	{ 0x2C13,	0x2C43,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER RITSI
	{ 0x2C14,	0x2C44,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER SLOVO
	{ 0x2C15,	0x2C45,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER TVRIDO
	{ 0x2C16,	0x2C46,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER UKU
	{ 0x2C17,	0x2C47,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER FRITU
	{ 0x2C18,	0x2C48,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER HERU
	{ 0x2C19,	0x2C49,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER OTU
	{ 0x2C1A,	0x2C4A,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER PE
	{ 0x2C1B,	0x2C4B,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER SHTA
	{ 0x2C1C,	0x2C4C,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER TSI
	{ 0x2C1D,	0x2C4D,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER CHRIVI
	{ 0x2C1E,	0x2C4E,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER SHA
	{ 0x2C1F,	0x2C4F,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER YERU
	{ 0x2C20,	0x2C50,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER YERI
	{ 0x2C21,	0x2C51,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER YATI
	{ 0x2C22,	0x2C52,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER SPIDERY HA
	{ 0x2C23,	0x2C53,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER YU
	{ 0x2C24,	0x2C54,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER SMALL YUS
	{ 0x2C25,	0x2C55,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER SMALL YUS WITH TAIL
	{ 0x2C26,	0x2C56,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER YO
	{ 0x2C27,	0x2C57,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER IOTATED SMALL YUS
	{ 0x2C28,	0x2C58,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER BIG YUS
	{ 0x2C29,	0x2C59,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER IOTATED BIG YUS
	{ 0x2C2A,	0x2C5A,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER FITA
	{ 0x2C2B,	0x2C5B,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER IZHITSA
	{ 0x2C2C,	0x2C5C,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER SHTAPIC
	{ 0x2C2D,	0x2C5D,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER TROKUTASTI A
	{ 0x2C2E,	0x2C5E,		UCHR_STATUS_C },	// # GLAGOLITIC CAPITAL LETTER LATINATE MYSLITE
	{ 0x2C60,	0x2C61,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH DOUBLE BAR
	{ 0x2C62,	0x026B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH MIDDLE TILDE
	{ 0x2C63,	0x1D7D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER P WITH STROKE
	{ 0x2C64,	0x027D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH TAIL
	{ 0x2C67,	0x2C68,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER H WITH DESCENDER
	{ 0x2C69,	0x2C6A,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K WITH DESCENDER
	{ 0x2C6B,	0x2C6C,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Z WITH DESCENDER
	{ 0x2C6D,	0x0251,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER ALPHA
	{ 0x2C6E,	0x0271,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER M WITH HOOK
	{ 0x2C6F,	0x0250,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TURNED A
	{ 0x2C70,	0x0252,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TURNED ALPHA
	{ 0x2C72,	0x2C73,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER W WITH HOOK
	{ 0x2C75,	0x2C76,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER HALF H
	{ 0x2C7E,	0x023F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH SWASH TAIL
	{ 0x2C7F,	0x0240,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Z WITH SWASH TAIL
	{ 0x2C80,	0x2C81,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER ALFA
	{ 0x2C82,	0x2C83,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER VIDA
	{ 0x2C84,	0x2C85,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER GAMMA
	{ 0x2C86,	0x2C87,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER DALDA
	{ 0x2C88,	0x2C89,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER EIE
	{ 0x2C8A,	0x2C8B,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER SOU
	{ 0x2C8C,	0x2C8D,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER ZATA
	{ 0x2C8E,	0x2C8F,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER HATE
	{ 0x2C90,	0x2C91,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER THETHE
	{ 0x2C92,	0x2C93,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER IAUDA
	{ 0x2C94,	0x2C95,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER KAPA
	{ 0x2C96,	0x2C97,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER LAULA
	{ 0x2C98,	0x2C99,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER MI
	{ 0x2C9A,	0x2C9B,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER NI
	{ 0x2C9C,	0x2C9D,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER KSI
	{ 0x2C9E,	0x2C9F,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER O
	{ 0x2CA0,	0x2CA1,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER PI
	{ 0x2CA2,	0x2CA3,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER RO
	{ 0x2CA4,	0x2CA5,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER SIMA
	{ 0x2CA6,	0x2CA7,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER TAU
	{ 0x2CA8,	0x2CA9,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER UA
	{ 0x2CAA,	0x2CAB,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER FI
	{ 0x2CAC,	0x2CAD,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER KHI
	{ 0x2CAE,	0x2CAF,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER PSI
	{ 0x2CB0,	0x2CB1,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OOU
	{ 0x2CB2,	0x2CB3,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER DIALECT-P ALEF
	{ 0x2CB4,	0x2CB5,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD COPTIC AIN
	{ 0x2CB6,	0x2CB7,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER CRYPTOGRAMMIC EIE
	{ 0x2CB8,	0x2CB9,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER DIALECT-P KAPA
	{ 0x2CBA,	0x2CBB,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER DIALECT-P NI
	{ 0x2CBC,	0x2CBD,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER CRYPTOGRAMMIC NI
	{ 0x2CBE,	0x2CBF,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD COPTIC OOU
	{ 0x2CC0,	0x2CC1,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER SAMPI
	{ 0x2CC2,	0x2CC3,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER CROSSED SHEI
	{ 0x2CC4,	0x2CC5,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD COPTIC SHEI
	{ 0x2CC6,	0x2CC7,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD COPTIC ESH
	{ 0x2CC8,	0x2CC9,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER AKHMIMIC KHEI
	{ 0x2CCA,	0x2CCB,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER DIALECT-P HORI
	{ 0x2CCC,	0x2CCD,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD COPTIC HORI
	{ 0x2CCE,	0x2CCF,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD COPTIC HA
	{ 0x2CD0,	0x2CD1,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER L-SHAPED HA
	{ 0x2CD2,	0x2CD3,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD COPTIC HEI
	{ 0x2CD4,	0x2CD5,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD COPTIC HAT
	{ 0x2CD6,	0x2CD7,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD COPTIC GANGIA
	{ 0x2CD8,	0x2CD9,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD COPTIC DJA
	{ 0x2CDA,	0x2CDB,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD COPTIC SHIMA
	{ 0x2CDC,	0x2CDD,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD NUBIAN SHIMA
	{ 0x2CDE,	0x2CDF,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD NUBIAN NGI
	{ 0x2CE0,	0x2CE1,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD NUBIAN NYI
	{ 0x2CE2,	0x2CE3,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER OLD NUBIAN WAU
	{ 0x2CEB,	0x2CEC,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER CRYPTOGRAMMIC SHEI
	{ 0x2CED,	0x2CEE,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER CRYPTOGRAMMIC GANGIA
	{ 0x2CF2,	0x2CF3,		UCHR_STATUS_C },	// # COPTIC CAPITAL LETTER BOHAIRIC KHEI
	{ 0xA640,	0xA641,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ZEMLYA
	{ 0xA642,	0xA643,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER DZELO
	{ 0xA644,	0xA645,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER REVERSED DZE
	{ 0xA646,	0xA647,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IOTA
	{ 0xA648,	0xA649,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER DJERV
	{ 0xA64A,	0xA64B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER MONOGRAPH UK
	{ 0xA64C,	0xA64D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER BROAD OMEGA
	{ 0xA64E,	0xA64F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER NEUTRAL YER
	{ 0xA650,	0xA651,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER YERU WITH BACK YER
	{ 0xA652,	0xA653,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IOTIFIED YAT
	{ 0xA654,	0xA655,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER REVERSED YU
	{ 0xA656,	0xA657,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IOTIFIED A
	{ 0xA658,	0xA659,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER CLOSED LITTLE YUS
	{ 0xA65A,	0xA65B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER BLENDED YUS
	{ 0xA65C,	0xA65D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER IOTIFIED CLOSED LITTLE YUS
	{ 0xA65E,	0xA65F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER YN
	{ 0xA660,	0xA661,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER REVERSED TSE
	{ 0xA662,	0xA663,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SOFT DE
	{ 0xA664,	0xA665,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SOFT EL
	{ 0xA666,	0xA667,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SOFT EM
	{ 0xA668,	0xA669,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER MONOCULAR O
	{ 0xA66A,	0xA66B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER BINOCULAR O
	{ 0xA66C,	0xA66D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER DOUBLE MONOCULAR O
	{ 0xA680,	0xA681,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER DWE
	{ 0xA682,	0xA683,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER DZWE
	{ 0xA684,	0xA685,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER ZHWE
	{ 0xA686,	0xA687,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER CCHE
	{ 0xA688,	0xA689,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER DZZE
	{ 0xA68A,	0xA68B,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER TE WITH MIDDLE HOOK
	{ 0xA68C,	0xA68D,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER TWE
	{ 0xA68E,	0xA68F,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER TSWE
	{ 0xA690,	0xA691,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER TSSE
	{ 0xA692,	0xA693,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER TCHE
	{ 0xA694,	0xA695,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER HWE
	{ 0xA696,	0xA697,		UCHR_STATUS_C },	// # CYRILLIC CAPITAL LETTER SHWE
	{ 0xA722,	0xA723,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER EGYPTOLOGICAL ALEF
	{ 0xA724,	0xA725,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER EGYPTOLOGICAL AIN
	{ 0xA726,	0xA727,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER HENG
	{ 0xA728,	0xA729,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TZ
	{ 0xA72A,	0xA72B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TRESILLO
	{ 0xA72C,	0xA72D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER CUATRILLO
	{ 0xA72E,	0xA72F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER CUATRILLO WITH COMMA
	{ 0xA732,	0xA733,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER AA
	{ 0xA734,	0xA735,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER AO
	{ 0xA736,	0xA737,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER AU
	{ 0xA738,	0xA739,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER AV
	{ 0xA73A,	0xA73B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER AV WITH HORIZONTAL BAR
	{ 0xA73C,	0xA73D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER AY
	{ 0xA73E,	0xA73F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER REVERSED C WITH DOT
	{ 0xA740,	0xA741,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K WITH STROKE
	{ 0xA742,	0xA743,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K WITH DIAGONAL STROKE
	{ 0xA744,	0xA745,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K WITH STROKE AND DIAGONAL STROKE
	{ 0xA746,	0xA747,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER BROKEN L
	{ 0xA748,	0xA749,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER L WITH HIGH STROKE
	{ 0xA74A,	0xA74B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH LONG STROKE OVERLAY
	{ 0xA74C,	0xA74D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER O WITH LOOP
	{ 0xA74E,	0xA74F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER OO
	{ 0xA750,	0xA751,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER P WITH STROKE THROUGH DESCENDER
	{ 0xA752,	0xA753,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER P WITH FLOURISH
	{ 0xA754,	0xA755,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER P WITH SQUIRREL TAIL
	{ 0xA756,	0xA757,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Q WITH STROKE THROUGH DESCENDER
	{ 0xA758,	0xA759,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER Q WITH DIAGONAL STROKE
	{ 0xA75A,	0xA75B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R ROTUNDA
	{ 0xA75C,	0xA75D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER RUM ROTUNDA
	{ 0xA75E,	0xA75F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER V WITH DIAGONAL STROKE
	{ 0xA760,	0xA761,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER VY
	{ 0xA762,	0xA763,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER VISIGOTHIC Z
	{ 0xA764,	0xA765,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER THORN WITH STROKE
	{ 0xA766,	0xA767,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER THORN WITH STROKE THROUGH DESCENDER
	{ 0xA768,	0xA769,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER VEND
	{ 0xA76A,	0xA76B,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER ET
	{ 0xA76C,	0xA76D,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER IS
	{ 0xA76E,	0xA76F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER CON
	{ 0xA779,	0xA77A,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER INSULAR D
	{ 0xA77B,	0xA77C,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER INSULAR F
	{ 0xA77D,	0x1D79,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER INSULAR G
	{ 0xA77E,	0xA77F,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TURNED INSULAR G
	{ 0xA780,	0xA781,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TURNED L
	{ 0xA782,	0xA783,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER INSULAR R
	{ 0xA784,	0xA785,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER INSULAR S
	{ 0xA786,	0xA787,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER INSULAR T
	{ 0xA78B,	0xA78C,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER SALTILLO
	{ 0xA78D,	0x0265,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER TURNED H
	{ 0xA790,	0xA791,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH DESCENDER
	{ 0xA792,	0xA793,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER C WITH BAR
	{ 0xA7A0,	0xA7A1,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER G WITH OBLIQUE STROKE
	{ 0xA7A2,	0xA7A3,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER K WITH OBLIQUE STROKE
	{ 0xA7A4,	0xA7A5,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER N WITH OBLIQUE STROKE
	{ 0xA7A6,	0xA7A7,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER R WITH OBLIQUE STROKE
	{ 0xA7A8,	0xA7A9,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER S WITH OBLIQUE STROKE
	{ 0xA7AA,	0x0266,		UCHR_STATUS_C },	// # LATIN CAPITAL LETTER H WITH HOOK
	{ 0xFF21,	0xFF41,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER A
	{ 0xFF22,	0xFF42,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER B
	{ 0xFF23,	0xFF43,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER C
	{ 0xFF24,	0xFF44,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER D
	{ 0xFF25,	0xFF45,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER E
	{ 0xFF26,	0xFF46,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER F
	{ 0xFF27,	0xFF47,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER G
	{ 0xFF28,	0xFF48,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER H
	{ 0xFF29,	0xFF49,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER I
	{ 0xFF2A,	0xFF4A,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER J
	{ 0xFF2B,	0xFF4B,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER K
	{ 0xFF2C,	0xFF4C,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER L
	{ 0xFF2D,	0xFF4D,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER M
	{ 0xFF2E,	0xFF4E,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER N
	{ 0xFF2F,	0xFF4F,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER O
	{ 0xFF30,	0xFF50,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER P
	{ 0xFF31,	0xFF51,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER Q
	{ 0xFF32,	0xFF52,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER R
	{ 0xFF33,	0xFF53,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER S
	{ 0xFF34,	0xFF54,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER T
	{ 0xFF35,	0xFF55,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER U
	{ 0xFF36,	0xFF56,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER V
	{ 0xFF37,	0xFF57,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER W
	{ 0xFF38,	0xFF58,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER X
	{ 0xFF39,	0xFF59,		UCHR_STATUS_C },	// # FULLWIDTH LATIN CAPITAL LETTER Y
	{ 0xFF3A,	0xFF5A,		UCHR_STATUS_C }		// # FULLWIDTH LATIN CAPITAL LETTER Z
};
//
// Создание вторичной таблицы (для перевода из строчной и заглавную букву)
//
static const CaseFoldingItem * u_get_sec_table()
{
	static TSVector <CaseFoldingItem> tbl; // @v9.8.4 TSArray-->TSVector
	if(tbl.getCount() == 0) {
		ENTER_CRITICAL_SECTION
			if(tbl.getCount() == 0) {
				const size_t count = SIZEOFARRAY(u_case_folding_tbl);
				assert(count);
				for(size_t i = 0; i < count; i++) {
					CaseFoldingItem item;
					item.Code = u_case_folding_tbl[i].ToCode;
					item.ToCode = u_case_folding_tbl[i].Code;
					item.Status_ = u_case_folding_tbl[i].Status_;
					tbl.insert(&item);
				}
				tbl.sort(PTR_CMPFUNC(int16));
			}
		LEAVE_CRITICAL_SECTION
	}
	return static_cast<const CaseFoldingItem *>(tbl.dataPtr());
}
//
// Бинарный поиск
//
static int FASTCALL u_bsearch(wchar_t code, const CaseFoldingItem * pTbl, uint * pIdx)
{
	int    cmp = 1;
	uint   i  = 0;
	const  uint count = SIZEOFARRAY(u_case_folding_tbl);
	assert(count);
	uint   lo  = 0;
	uint   up  = count - 1;
	while(lo <= up) {
		const CaseFoldingItem * p = pTbl + ((i = (lo + up) >> 1));
		wchar_t c = p->Code;
		cmp = (c > code) ? 1 : (c < code) ? -1 : 0;
		if(cmp < 0)
			lo = i + 1;
		else if(cmp && i)
			up = i - 1;
		else {
			break;
		}
	}
	ASSIGN_PTR(pIdx, i);
	return cmp;
}
//
// 2 - Upper case
// 1 - Lower case
// 0 - Unknown code
//
static int FASTCALL u_get_char_case(wchar_t code, const CaseFoldingItem ** pItem)
{
	int    r = 0;
	uint   idx = 0;
	// Ищем код по заглавным
	int    cmp = u_bsearch(code, u_case_folding_tbl, &idx);
	if(cmp == 0) {
		ASSIGN_PTR(pItem, &u_case_folding_tbl[idx]);
		r = 2;
	}
	else {
		// Ищем код по строчным
		const CaseFoldingItem * p_sec_tbl = u_get_sec_table();
		if(p_sec_tbl) {
			cmp = u_bsearch(code, p_sec_tbl, &idx);
			if(cmp == 0) {
				ASSIGN_PTR(pItem, &p_sec_tbl[idx]);
				r = 1;
			}
		}
	}
	return r;
}

static wchar_t FASTCALL u_to_case(wchar_t code, int c)
{
	wchar_t r = 0;
	const   CaseFoldingItem * p_item = 0;
	const   int ch_case = u_get_char_case(code, &p_item);
	if(ch_case == c) {
		if(p_item)
			r = p_item->ToCode;
	}
	else
		r = code;
	return r;
}

// 2 - Upper case
// 1 - Lower case
// 0 - Unknown code
//
int    FASTCALL UGetCharCase(wchar_t code) { return u_get_char_case(code, 0); }
wchar_t FASTCALL UToUpperCase(wchar_t code) { return u_to_case(code, 1); }
wchar_t FASTCALL UToLowerCase(wchar_t code) { return u_to_case(code, 2); }

#undef UCHR_STATUS_C
#undef UCHR_STATUS_F
#undef UCHR_STATUS_S
#undef UCHR_STATUS_T
//
// } @Muxa
//
SLAPI STokenizer::Param::Param() : Flags(0), Cp(0)
{
}

SLAPI STokenizer::Param::Param(long flags, int cp, const char * pDelim) : Delim(pDelim), Flags(flags), Cp(cp)
{
}

SLAPI STokenizer::STokenizer() : T(1000000, 0), Tc(0), RP(0), SO(0), P_ResourceIndex(0)
{
	SetParam(0);
}

SLAPI STokenizer::STokenizer(const Param & rParam) : T(1000000, 0), Tc(0), RP(0), SO(0), P_ResourceIndex(0)
{
	SetParam(&rParam);
}

SLAPI STokenizer::~STokenizer()
{
	delete P_ResourceIndex;
}

void SLAPI STokenizer::GetParam(Param * pParam) const
{
	ASSIGN_PTR(pParam, P);
}

int SLAPI STokenizer::SetParam(const Param * pParam)
{
	RVALUEPTR(P, pParam);
	if(P.Cp == cpUTF8) {
		SString temp_buf = P.Delim;
		DelimU.CopyFromUtf8(temp_buf);
	}
	return 1;
}

STokenizer & SLAPI STokenizer::Reset(long options)
{
	RP = 0;
	SO = 0;
	S.Z();
	L.clear();
	ZDELETE(P_ResourceIndex);
	if(options & coClearSymbTab) {
		T.Clear();
		CL.clear();
	}
	return *this;
}

void SLAPI STokenizer::ClearInput()
{
	S.Z();
}

int SLAPI STokenizer::Write(const char * pResource, int64 orgOffs, const void * pS, size_t sz)
{
	int    ok = 1;
	THROW(S.Write(pS, sz));
	SO = orgOffs;
	RP = 0;
	if(!isempty(pResource)) {
		uint   tv = 0;
		uint   tp = 0;
		if(T.Search(pResource, &tv, &tp)) {
			RP = tp;
		}
		else {
			tv = ++Tc;
			THROW(T.Add(pResource, tv, &tp));
			RP = tp;
		}
	}
	CATCHZOK
	return ok;
}

uint16 SLAPI STokenizer::NextChr()
{
	uchar c;
	uint16 result = 0;
	if(S.Read(c)) {
		if(P.Cp == cpUTF8) {
			uint8  set[8];
			uint32 ch = 0;
			uint16 extra = SUtfConst::TrailingBytesForUTF8[c];
			size_t actual_size = 0;
			set[0] = c;
			if(extra) {
				assert(extra <= 5);
				actual_size = S.Read(set+1, extra);
			}
			if(actual_size == extra) {
				size_t i = 0;
				if(SUnicode::IsLegalUtf8(set, extra+1)) {
					switch(extra) {
						case 5: ch += set[i++]; ch <<= 6; // remember, illegal UTF-8
						case 4: ch += set[i++]; ch <<= 6; // remember, illegal UTF-8
						case 3: ch += set[i++]; ch <<= 6;
						case 2: ch += set[i++]; ch <<= 6;
						case 1: ch += set[i++]; ch <<= 6;
						case 0: ch += set[i++];
					}
					ch -= SUtfConst::OffsetsFromUTF8[extra];
					if(ch <= UNI_MAX_BMP) { // Target is a character <= 0xFFFF
						if(ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) // UTF-16 surrogate values are illegal in UTF-32
							result = (wchar_t)UNI_REPLACEMENT_CHAR;
						else
							result = (wchar_t)ch; // normal case
					}
					else if(ch > UNI_MAX_UTF16)
						result = (wchar_t)UNI_REPLACEMENT_CHAR;
					else // target is a character in range 0xFFFF - 0x10FFFF.
						result = (wchar_t)UNI_REPLACEMENT_CHAR;
				}
				else
					result = (wchar_t)UNI_REPLACEMENT_CHAR;
			}
		}
		else
			result = (uint16)c;
	}
	return result;
}

int FASTCALL STokenizer::IsDelim(uint16 chr) const
{
	if(P.Delim.NotEmpty()) {
		if(P.Cp == cpUTF8) {
			if(DelimU.HasChr(chr))
				return 1;
		}
		else {
			if(P.Delim.HasChr(chr))
				return 1;
		}
	}
	else {
		if(oneof4(chr, ' ', '\t', '\n', '\r'))
			return 1;
	}
	return 0;
}

void SLAPI STokenizer::_CopySrcToAddTokenBuf(const TSVector <uint16> & rBuf)
{
	AddTokenBuf.Z();
	const uint bc = rBuf.getCount();
	if(P.Cp == cpUTF8) {
		AddTokenBuf.CopyUtf8FromUnicode((const wchar_t *)rBuf.dataPtr(), bc, 1);
	}
	else {
		for(uint i = 0; i < bc; i++) {
			AddTokenBuf.CatChar(rBuf.at(i));
		}
	}
}

int FASTCALL STokenizer::AddToken(TSVector <uint16> & rBuf, int tokType, int64 sp)
{
	int    ok = 1;
	const  uint bc = rBuf.getCount();
	if(bc) {
		Token  tok;
		MEMSZERO(tok);
		_CopySrcToAddTokenBuf(rBuf);
		tok.T = tokType;
		tok.SP = sp;
		uint   tv = 0;
		uint   tp = 0;
		if(AddTokenBuf.NotEmpty()) {
			if(T.Search(AddTokenBuf, &tv, &tp)) {
				tok.PP = tp;
			}
			else {
				tv = ++Tc;
				THROW(T.Add(AddTokenBuf, tv, &tp));
				tok.PP = tp;
				{
					CToken ct;
					ct.T = tok.T;
					ct.PP = tp;
					THROW(CL.insert(&ct));
				}
			}
		}
		else {
			//
			// Неявный разделитель
			//
			assert(tokType == tokDelim);
			tok.PP = 0;
		}
		tok.RP = RP;
		THROW(L.insert(&tok));
	}
	CATCHZOK
	rBuf.clear();
	return ok;
}

int SLAPI STokenizer::IndexResources(int force)
{
	int    ok = 1;
	if(!P_ResourceIndex || force) {
		ZDELETE(P_ResourceIndex);
		P_ResourceIndex = new TSCollection <ResourceIndexItem>;
		uint   _pos_in_rtext = 0;
		uint   _rp = 0;
		for(uint i = 0; i < L.getCount(); i++) {
			const Token & r_t = L.at(i);
			if(r_t.RP) {
				if(r_t.RP == _rp) {
					_pos_in_rtext++;
				}
				else {
					_rp = r_t.RP;
					_pos_in_rtext = 0;
				}
				uint  _p = 0;
				ResourceToken rt_item;
				rt_item.RP = r_t.RP;
				rt_item.PosInRText = _pos_in_rtext;
				if(P_ResourceIndex->lsearch(&r_t.PP, &_p, CMPF_LONG)) {
					P_ResourceIndex->at(_p)->RL.insert(&rt_item);
				}
				else {
					ResourceIndexItem * p_new_item = P_ResourceIndex->CreateNewItem();
					p_new_item->PP = r_t.PP;
					p_new_item->RL.insert(&rt_item);
				}
			}
		}
		P_ResourceIndex->sort(CMPF_LONG);
	}
	else
		ok = -1;
	return ok;
}

int SLAPI STokenizer::ProcessSearchToken(TSVector <uint16> & rBuf, int tokType, TSCollection <STokenizer::SearchBlockEntry> & rResult)
{
	int    ok = -1;
	if(tokType != tokDelim) {
		const  uint bc = rBuf.getCount();
		if(bc) {
			Token  tok;
			MEMSZERO(tok);
			_CopySrcToAddTokenBuf(rBuf);
			tok.T = tokType;
			uint   tv = 0;
			uint   tp = 0;
			if(AddTokenBuf.NotEmpty()) {
				if(T.Search(AddTokenBuf, &tv, &tp)) {
					SearchBlockEntry * p_new_entry = new SearchBlockEntry;
					p_new_entry->T = tp;
					p_new_entry->P = 0;
					if(P_ResourceIndex) {
						uint _p = 0;
						if(P_ResourceIndex->bsearch(&tp, &_p, CMPF_LONG)) {
							const ResourceIndexItem * p_idx_item = P_ResourceIndex->at(_p);
							if(p_idx_item)
								p_new_entry->RL = p_idx_item->RL;
						}
					}
					else {
						for(uint _p = 0; L.lsearch(&tp, &_p, CMPF_LONG, offsetof(Token, PP)); _p++) {
							const Token & r_tok = L.at(_p);
							if(r_tok.RP) {
								ResourceToken idx_item;
								idx_item.RP = r_tok.RP;
								idx_item.PosInRText = static_cast<uint>(-1);
								p_new_entry->RL.insert(&idx_item);
							}
						}
					}
					if(p_new_entry->RL.getCount()) {
						p_new_entry->RL.sort(PTR_CMPFUNC(_2long));
						rResult.insert(p_new_entry);
						ok = 1;
					}
					else {
						ZDELETE(p_new_entry);
					}
				}
				else {
					;
				}
			}
			else {
				//
				// Неявный разделитель
				//
				assert(tokType == tokDelim);
				tok.PP = 0;
			}
			tok.RP = RP;
		}
	}
	rBuf.clear();
	return ok;
}

int SLAPI STokenizer::Search(long flags, TSCollection <STokenizer::SearchBlockEntry> & rResult)
{
	int    ok = 1;
	uint16 chr = 0;
	uint16 prev_delim = 0;
	TokenBuf.clear();
	for(uint16  prev_chr = 0; (chr = NextChr()) != 0; prev_chr = chr) {
		if(IsDelim(chr)) {
			if(prev_delim) {
				if(P.Flags & fEachDelim) {
					THROW(ProcessSearchToken(TokenBuf, tokDelim, rResult));
				}
			}
			else {
				THROW(ProcessSearchToken(TokenBuf, tokWord, rResult));
			}
			TokenBuf.insert(&chr);
			prev_delim = chr;
		}
		else {
			if(prev_delim) {
				THROW(ProcessSearchToken(TokenBuf, tokDelim, rResult));
			}
			else if(P.Flags & fDivAlNum && prev_chr) {
				const int is_dig = BIN(isdigit((uint8)chr));
				const int is_prev_dig = BIN(isdigit((uint8)prev_chr));
				if(is_prev_dig != is_dig) {
					THROW(ProcessSearchToken(TokenBuf, tokWord, rResult));
					THROW(ProcessSearchToken(TokenBuf, tokDelim, rResult));
				}
			}
			TokenBuf.insert(&chr);
			prev_delim = 0;
		}
	}
	THROW(ProcessSearchToken(TokenBuf, (prev_delim ? tokDelim : tokWord), rResult));
	{
		uint   i;
		if(flags & sfAllInPatternOnly) {
			LongArray rp_intersect_list, rp_temp_list;
			for(i = 0; i < rResult.getCount(); i++) {
				const SearchBlockEntry * p_entry = rResult.at(i);
				const uint rl_c = p_entry->RL.getCount();
				if(rl_c) {
					rp_temp_list.clear();
					for(uint j = 0; j < rl_c; j++)
						rp_temp_list.add((long)p_entry->RL.at(j).RP);
					if(i == 0)
						rp_intersect_list = rp_temp_list;
					else
						rp_intersect_list.intersect(&rp_temp_list);
				}
				else {
					rp_intersect_list.clear();
					break;
				}
			}
			if(rp_intersect_list.getCount()) {
				rp_intersect_list.sortAndUndup();
				i = rResult.getCount();
				if(i) do {
					SearchBlockEntry * p_entry = rResult.at(--i);
					uint j = p_entry->RL.getCount();
					if(j) do {
						const long _rp = (long)p_entry->RL.at(--j).RP;
						if(!rp_intersect_list.bsearch(_rp))
							p_entry->RL.atFree(j);
					} while(j);
					if(p_entry->RL.getCount() == 0)
						rResult.atFree(i);
				} while(i);
			}
			else {
				rResult.freeAll();
			}
		}
		if(flags & sfFirstInTextOnly) {
			LongArray rp_suit_list;
			for(i = 0; i < rResult.getCount(); i++) {
				const SearchBlockEntry * p_entry = rResult.at(i);
				for(uint j = 0; j < p_entry->RL.getCount(); j++) {
					if(p_entry->RL.at(j).PosInRText == 0) {
						rp_suit_list.add((long)p_entry->RL.at(j).RP);
					}
				}
			}
			if(rp_suit_list.getCount()) {
				rp_suit_list.sortAndUndup();
				i = rResult.getCount();
				if(i) do {
					SearchBlockEntry * p_entry = rResult.at(--i);
					uint j = p_entry->RL.getCount();
					if(j) do {
						const long _rp = (long)p_entry->RL.at(--j).RP;
						if(!rp_suit_list.bsearch(_rp))
							p_entry->RL.atFree(j);
					} while(j);
					if(p_entry->RL.getCount() == 0)
						rResult.atFree(i);
				} while(i);
			}
			else
				rResult.freeAll();
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI STokenizer::Run(uint * pIdxFirst, uint * pIdxCount)
{
	int    ok = 1;
	const  uint preserve_count = L.getCount();
	uint   idx_first = preserve_count;
	uint   idx_count = 0;
	int64  current_pos = 0;
	int64  start_tok_pos = 0;
	uint16 chr = 0;
	uint16 prev_delim = 0;
	TokenBuf.clear();
	for(uint16 prev_chr = 0; (chr = NextChr()) != 0; prev_chr = chr) {
		if(IsDelim(chr)) {
			if(prev_delim) {
				if(P.Flags & fEachDelim) {
					THROW(AddToken(TokenBuf, tokDelim, start_tok_pos));
					idx_count++;
					start_tok_pos = current_pos;
				}
			}
			else {
				THROW(AddToken(TokenBuf, tokWord, start_tok_pos));
				start_tok_pos = current_pos;
			}
			TokenBuf.insert(&chr);
			prev_delim = chr;
		}
		else {
			if(prev_delim) {
				THROW(AddToken(TokenBuf, tokDelim, start_tok_pos));
				start_tok_pos = current_pos;
			}
			else if(P.Flags & fDivAlNum && prev_chr) {
				int is_dig = 0;
				int is_prev_dig = 0;
				if(P.Cp == cpUTF8) {
					is_dig = isdecw((wchar_t)chr);
					is_prev_dig = isdecw((wchar_t)prev_chr);
				}
				else {
					is_dig = isdec((char)chr);
					is_prev_dig = isdec((char)prev_chr);
				}
				if(is_prev_dig != is_dig) {
					THROW(AddToken(TokenBuf, tokWord, start_tok_pos));
					THROW(AddToken(TokenBuf, tokDelim, current_pos));
					start_tok_pos = current_pos;
				}
			}
			TokenBuf.insert(&chr);
			prev_delim = 0;
		}
		if(P.Flags & fRawOrgOffs)
			current_pos = S.GetRdOffs();
		else
			current_pos++;
	}
	THROW(AddToken(TokenBuf, (prev_delim ? tokDelim : tokWord), start_tok_pos));
	idx_count = L.getCount() - preserve_count;
	CATCHZOK
	ASSIGN_PTR(pIdxFirst, idx_first);
	ASSIGN_PTR(pIdxCount, idx_count);
	return ok;
}

int SLAPI STokenizer::RunSString(const char * pResource, int64 orgOffs, const SString & rS, uint * pIdxFirst, uint * pIdxCount)
	{ return BIN(Write(pResource, orgOffs, rS, rS.Len()+1) && Run(pIdxFirst, pIdxCount)); }
uint SLAPI STokenizer::GetCommCount() const
	{ return CL.getCount(); }
uint SLAPI STokenizer::GetCount() const
	{ return L.getCount(); }
int SLAPI STokenizer::GetSymbHashStat(SymbHashTable::Stat & rStat) const
	{ return T.CalcStat(rStat); }

int SLAPI STokenizer::GetComm(uint idx, STokenizer::Item & rItem) const
{
	if(idx < CL.getCount()) {
		const CToken & r_token = CL.at(idx);
		rItem.Token = r_token.T;
		rItem.TextId = r_token.PP;
		rItem.OrgOffs = 0;
		T.Get(r_token.PP, rItem.Text);
		rItem.Resource.Z();
		return 1;
	}
	else
		return 0;
}

int SLAPI STokenizer::GetTextById(uint txtId, SString & rBuf) const
{
	rBuf.Z();
	return T.Get(txtId, rBuf);
}

int SLAPI STokenizer::Get(uint idx, STokenizer::Item & rItem) const
{
	if(idx < L.getCount()) {
		const Token & r_token = L.at(idx);
		rItem.Token = r_token.T;
		rItem.TextId = r_token.PP;
		rItem.OrgOffs = r_token.SP;
		T.Get(r_token.PP, rItem.Text);
		T.Get(r_token.RP, rItem.Resource);
		return 1;
	}
	else
		return 0;
}

int SLAPI STokenizer::Get_WithoutText(uint idx, STokenizer::Item & rItem) const
{
	if(idx < L.getCount()) {
		const Token & r_token = L.at(idx);
		rItem.Token = r_token.T;
		rItem.TextId = r_token.PP;
		rItem.OrgOffs = r_token.SP;
		rItem.Text.Z();
		rItem.Resource.Z();
		return 1;
	}
	else
		return 0;
}
//
//
// email regexp: (?:[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*|"(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21\x23-\x5b\x5d-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])*")@(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?|\[(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?|[a-z0-9-]*[a-z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\])

//
// {9}-[*]

SLAPI SNaturalTokenStat::SNaturalTokenStat() : Len(0), Seq(0)
{
}

SNaturalTokenArray & SLAPI SNaturalTokenArray::Z()
{
	clear();
	return *this;
}

float FASTCALL SNaturalTokenArray::Has(uint32 tok) const
{
    uint   pos = 0;
    return lsearch(&tok, &pos, CMPF_LONG) ? at(pos).Prob : 0.0f;
}

int SLAPI SNaturalTokenArray::Add(uint32 tok, float prob)
{
	SNaturalToken item;
	item.ID = tok;
	item.Prob = prob;
	return insert(&item);
}

SLAPI STokenRecognizer::STokenRecognizer() : SRegExpSet()
{
}

SLAPI STokenRecognizer::~STokenRecognizer()
{
}

//static
int FASTCALL STokenRecognizer::IsUtf8(const uchar * p, size_t restLen)
{
	const size_t extra = SUtfConst::TrailingBytesForUTF8[*p];
	return (extra == 0) ? 1 : ((restLen > extra && SUnicode::IsLegalUtf8(p, 2)) ? (extra+1) : 0);
}

/*
Возможные номера телефонов:
	99-99-99
	999-999
	999-9999
	russia: 8(999)999-99-99 (8 9999)99-99-99

Числа:
	9'999'999.99
	9'999'999,99
	99,999.99
	[0-9],. '-+
*/

static int FASTCALL _ProbeDate(const SString & rText)
{
	int    ok = 0;
	LDATE  probe_dt = strtodate_(rText, DATF_DMY);
	if(checkdate(probe_dt))
		ok = 1;
	else {
		probe_dt = strtodate_(rText, DATF_MDY);
		if(checkdate(probe_dt))
			ok = 1;
		else {
			probe_dt = strtodate_(rText, DATF_YMD);
			if(checkdate(probe_dt))
				ok = 1;
		}
	}
	return ok;
}

int SLAPI STokenRecognizer::Run(const uchar * pToken, int len, SNaturalTokenArray & rResultList, SNaturalTokenStat * pStat)
{
	int    ok = 1;
	uint32 h = 0;
	SString temp_buf;
    SNaturalTokenStat stat;
    stat.Len = (len >= 0) ? (uint32)len : sstrlen(pToken);
    if(stat.Len) {
		enum {
			fUtf8   = 0x0001
		};
		uint32 f = 0;
		uint   i;
		uchar  num_potential_frac_delim = 0;
		uchar  num_potential_tri_delim = 0;
		LAssocArray chr_list;
		h = 0xffffffffU & ~(SNTOKSEQ_LEADSHARP|SNTOKSEQ_LEADMINUS|SNTOKSEQ_LEADDOLLAR|SNTOKSEQ_BACKPCT);
		const char the_first_chr = pToken[0];
		for(i = 0; i < stat.Len; i++) {
            const uchar c = pToken[i];
			const size_t ul = IsUtf8(pToken+i, stat.Len-i);
			if(ul > 1) {
                f |= fUtf8;
                i += (ul-1);
			}
			else {
                if(!ul)
					h &= ~SNTOKSEQ_UTF8;
				uint  pos = 0;
				if(chr_list.Search(static_cast<long>(c), 0, &pos))
					chr_list.at(pos).Val++;
				else
					chr_list.Add(static_cast<long>(c), 1, 0);
			}
		}
		chr_list.Sort();
		if(f & fUtf8) {
			h &= ~(SNTOKSEQ_DEC|SNTOKSEQ_HEX|SNTOKSEQ_LATLWR|SNTOKSEQ_LATUPR|SNTOKSEQ_LAT|SNTOKSEQ_DECLAT|
				SNTOKSEQ_ASCII|SNTOKSEQ_866|SNTOKSEQ_1251|SNTOKSEQ_HEXHYPHEN|SNTOKSEQ_DECHYPHEN|SNTOKSEQ_HEXCOLON|
				SNTOKSEQ_DECCOLON|SNTOKSEQ_HEXDOT|SNTOKSEQ_DECDOT|SNTOKSEQ_DECSLASH|SNTOKSEQ_NUMERIC);
		}
		else {
			int is_lead_plus = 0;
			int has_dec = 0;
			i = 0;
			if(the_first_chr == '#') {
				h |= SNTOKSEQ_LEADSHARP;
				i++;
			}
			else if(the_first_chr == '-')
				h |= SNTOKSEQ_LEADMINUS;
			else if(the_first_chr == '$')
				h |= SNTOKSEQ_LEADDOLLAR;
			else if(the_first_chr == '+')
				is_lead_plus = 1;
			const uint clc = chr_list.getCount();
			for(; i < clc; i++) {
				const uchar c = static_cast<uchar>(chr_list.at(i).Key);
				if(h & SNTOKSEQ_ASCII && !(c >= 1 && c <= 127))
					h &= ~SNTOKSEQ_ASCII;
				else {
					const int is_hex_c = ishex(c);
					const int is_dec_c = isdec(c);
					if(is_dec_c)
						has_dec = 1;
					if(h & SNTOKSEQ_LAT && !IsLetterASCII(c))
						h &= ~SNTOKSEQ_LAT;
					else {
						if(h & SNTOKSEQ_LATLWR && !(c >= 'a' && c <= 'z'))
							h &= ~SNTOKSEQ_LATLWR;
						if(h & SNTOKSEQ_LATUPR && !(c >= 'A' && c <= 'Z'))
							h &= ~SNTOKSEQ_LATUPR;
					}
					if(h & SNTOKSEQ_HEX && !is_hex_c)
						h &= ~SNTOKSEQ_HEX;
					else if(h & SNTOKSEQ_DEC && !is_dec_c)
						h &= ~SNTOKSEQ_DEC;
					if(h & SNTOKSEQ_DECHYPHEN && !(c == '-' || is_dec_c))
						h &= ~SNTOKSEQ_DECHYPHEN;
					if(h & SNTOKSEQ_HEXHYPHEN && !(c == '-' || is_hex_c))
						h &= ~SNTOKSEQ_HEXHYPHEN;
					if(h & SNTOKSEQ_DECLAT && !((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || is_dec_c))
						h &= ~SNTOKSEQ_DECLAT;
					if(h & SNTOKSEQ_DECCOLON && !(c == ':' || is_dec_c))
						h &= ~SNTOKSEQ_DECCOLON;
					if(h & SNTOKSEQ_HEXCOLON && !(c == ':' || is_hex_c))
						h &= ~SNTOKSEQ_HEXCOLON;
					if(h & SNTOKSEQ_DECDOT && !(c == '.' || is_dec_c))
						h &= ~SNTOKSEQ_DECDOT;
					if(h & SNTOKSEQ_HEXDOT && !(c == '.' || is_hex_c))
						h &= ~SNTOKSEQ_HEXDOT;
					if(h & SNTOKSEQ_DECSLASH && !(c == '/' || is_dec_c))
						h &= ~SNTOKSEQ_DECSLASH;
					if(h & SNTOKSEQ_NUMERIC) {
						if(!is_dec_c && !oneof7(c, ',', '.', '\'', '`', ' ', '+', '-'))
							h &= ~SNTOKSEQ_NUMERIC;
						else if(the_first_chr == ' ')
							h &= ~SNTOKSEQ_NUMERIC;
						else if(oneof2(c, '+', '-')) {
							if(chr_list.at(i).Val > 1)
								h &= ~SNTOKSEQ_NUMERIC;
							else if(c == '+' && !is_lead_plus)
								h &= ~SNTOKSEQ_NUMERIC;
							else if(c == '-' && !(h & SNTOKSEQ_LEADMINUS))
								h &= ~SNTOKSEQ_NUMERIC;
						}
					}
				}
				if(h & SNTOKSEQ_866 && !IsLetter866(c))
					h &= ~SNTOKSEQ_866;
				if(h & SNTOKSEQ_1251 && !IsLetter1251(c))
					h &= ~SNTOKSEQ_1251;
			}
			if(!(h & SNTOKSEQ_ASCII)) {
				h &= ~(SNTOKSEQ_LAT|SNTOKSEQ_LATUPR|SNTOKSEQ_LATLWR|SNTOKSEQ_HEX|SNTOKSEQ_DEC|SNTOKSEQ_DECLAT|
					SNTOKSEQ_HEXHYPHEN|SNTOKSEQ_DECHYPHEN|SNTOKSEQ_HEXCOLON|SNTOKSEQ_DECCOLON|SNTOKSEQ_HEXDOT|
					SNTOKSEQ_DECDOT|SNTOKSEQ_DECSLASH|SNTOKSEQ_NUMERIC);
			}
			else {
				if(!(h & SNTOKSEQ_HEX))
					h &= ~SNTOKSEQ_DEC;
				if(!(h & SNTOKSEQ_LAT))
					h &= ~(SNTOKSEQ_LATLWR|SNTOKSEQ_LATUPR);
			}
			// @v10.5.6 @fix if(SNTOKSEQ_DECLAT) { if(!(h & (SNTOKSEQ_LAT|SNTOKSEQ_DEC))) h &= ~SNTOKSEQ_DECLAT; }
			{
				const uint32 tf = SNTOKSEQ_HEXHYPHEN;
				if(h & tf) {
					if(h & SNTOKSEQ_HEX)
						h &= ~tf;
					else if(clc == 1) {
						assert(chr_list.at(0).Key == '-');
						h &= ~tf;
					}
					else if(clc == 2 && h & SNTOKSEQ_LEADSHARP) {
						assert(chr_list.at(1).Key == '-'); // '#' < '-'
						h &= ~tf;
					}
				}
			}
			{
				const uint32 tf = SNTOKSEQ_DECHYPHEN;
				if(h & tf) {
					if(h & SNTOKSEQ_DEC)
						h &= ~tf;
					else if(clc == 1) {
						assert(chr_list.at(0).Key == '-');
						h &= ~tf;
					}
					else if(clc == 2 && h & SNTOKSEQ_LEADSHARP) {
						assert(chr_list.at(1).Key == '-'); // '#' < '-'
						h &= ~tf;
					}
				}
			}
			{
				const uint32 tf = SNTOKSEQ_HEXCOLON;
				if(h & tf) {
					if(h & SNTOKSEQ_HEX)
						h &= ~tf;
					else if(clc == 1) {
						assert(chr_list.at(0).Key == ':');
						h &= ~tf;
					}
					else if(clc == 2 && h & SNTOKSEQ_LEADSHARP) {
						assert(chr_list.at(1).Key == ':'); // '#' < ':'
						h &= ~tf;
					}
				}
			}
			{
				const uint32 tf = SNTOKSEQ_DECCOLON;
				if(h & tf) {
					if(h & SNTOKSEQ_DEC)
						h &= ~tf;
					else if(clc == 1) {
						assert(chr_list.at(0).Key == ':');
						h &= ~tf;
					}
					else if(clc == 2 && h & SNTOKSEQ_LEADSHARP) {
						assert(chr_list.at(1).Key == ':'); // '#' < ':'
						h &= ~tf;
					}
				}
			}
			{
				const uint32 tf = SNTOKSEQ_HEXDOT;
				if(h & tf) {
					if(h & SNTOKSEQ_HEX)
						h &= ~tf;
					else if(clc == 1) {
						assert(chr_list.at(0).Key == '.');
						h &= ~tf;
					}
					else if(clc == 2 && h & SNTOKSEQ_LEADSHARP) {
						assert(chr_list.at(1).Key == '.'); // '#' < '.'
						h &= ~tf;
					}
				}
			}
			{
				const uint32 tf = SNTOKSEQ_DECDOT;
				if(h & tf) {
					if(h & SNTOKSEQ_DEC)
						h &= ~tf;
					else if(clc == 1) {
						assert(chr_list.at(0).Key == '.');
						h &= ~tf;
					}
					else if(clc == 2 && h & SNTOKSEQ_LEADSHARP) {
						assert(chr_list.at(1).Key == '.'); // '#' < '.'
						h &= ~tf;
					}
				}
			}
			{
				const uint32 tf = SNTOKSEQ_DECSLASH;
				if(h & tf) {
					if(h & SNTOKSEQ_DEC)
						h &= ~tf;
					else if(clc == 1) {
						assert(chr_list.at(0).Key == '/');
						h &= ~tf;
					}
					else if(clc == 2 && h & SNTOKSEQ_LEADSHARP) {
						assert(chr_list.at(1).Key == '/'); // '#' < '/'
						h &= ~tf;
					}
				}
			}
			{
				const uint32 tf = SNTOKSEQ_NUMERIC;
				if(h & tf) {
					if(!has_dec)
						h &= ~tf;
					else {
						uint   comma_chr_pos = 0;
						const  uint comma_count = chr_list.Search(static_cast<long>(','), 0, &comma_chr_pos) ? chr_list.at(comma_chr_pos).Val : 0;
						uint   last_dec_ser = 0;
						uint   j = stat.Len;
						if(j) do {
							const uchar lc = pToken[--j];
							if(isdec(lc))
								last_dec_ser++;
							else {
								if(lc == '.') {
									if(!num_potential_frac_delim)
										num_potential_frac_delim = lc;
									else {
										h &= ~tf;
										break; // Две точки в разбираемом формате невозможны
									}
								}
								else if(lc == ',' && comma_count == 1) {
									// 999,99
									// 999,999 - самый плохой случай. 
									if(last_dec_ser == 3) {
										num_potential_tri_delim = lc;
									}
									else if(num_potential_frac_delim) {
										h &= ~tf;
										break; // Роль десятичного разделителя занята, а на роль разделителя разрядов запятая здесь не годится - уходим.
									}
									if(!num_potential_frac_delim)
										num_potential_frac_delim = lc;
								}
								else if((comma_count > 1) ? oneof4(lc, ',', '\'', '`', ' ') : oneof3(lc, '\'', '`', ' ')) {
									if(last_dec_ser != 3) {
										h &= ~tf;
										break; // Потенциальный резделитель должен иметь точно 3 цифры справа. Это не так - уходим.
									}
									else {
										if(!num_potential_tri_delim) {
											num_potential_tri_delim = lc;
										}
										else if(num_potential_tri_delim != lc) {
											h &= ~tf;
											break; // Более одного символа могут претендовать на роль разделителя разрядов - уходим
										}
									}
								}
								else if(lc == '+') {
									if(j) {
										h &= ~tf;
										break; // + может быть только в первой позиции
									}
								}
								else if(lc == '-') {
									if(j) {
										h &= ~tf;
										break; // - может быть только в первой позиции
									}
								}
								last_dec_ser = 0;
							}
						} while(j);
					}
				}
			}
		}
		if(h & SNTOKSEQ_LEADSHARP) {
			if(h & SNTOKSEQ_HEX && stat.Len == 7) {
				rResultList.Add(SNTOK_COLORHEX, 0.9f);
			}
		}
		/*else if(h & SNTOKSEQ_LEADMINUS) {
		}*/
		else if(h & SNTOKSEQ_LEADDOLLAR) {
		}
		else if(h & SNTOKSEQ_BACKPCT) {
		}
		else {
			if(h & SNTOKSEQ_DEC) {
				uchar last = pToken[stat.Len-1];
				int   cd = 0;
				rResultList.Add(SNTOK_DIGITCODE, 1.0f);
				switch(stat.Len) {
					case 6:
						if(_ProbeDate(temp_buf.Z().CatN(reinterpret_cast<const char *>(pToken), stat.Len))) {
							rResultList.Add(SNTOK_DATE, 0.5f);
						}
						break;
					case 8:
						cd = SCalcBarcodeCheckDigitL(reinterpret_cast<const char *>(pToken), stat.Len-1);
						if((uchar)cd == (last-'0')) {
							if(pToken[0] == '0')
								rResultList.Add(SNTOK_UPCE, 0.9f);
							else
								rResultList.Add(SNTOK_EAN8, 0.9f);
						}
						if(_ProbeDate(temp_buf.Z().CatN(reinterpret_cast<const char *>(pToken), stat.Len))) {
							rResultList.Add(SNTOK_DATE, 0.8f);
						}
						break;
					case 10:
						if(SCalcCheckDigit(SCHKDIGALG_RUINN|SCHKDIGALG_TEST, reinterpret_cast<const char *>(pToken), stat.Len)) {
							rResultList.Add(SNTOK_RU_INN, 1.0f);
						}
						break;
					case 12:
						cd = SCalcBarcodeCheckDigitL(reinterpret_cast<const char *>(pToken), stat.Len-1);
						if(static_cast<uchar>(cd) == (last-'0')) {
							if(pToken[0] == '0')
								rResultList.Add(SNTOK_UPCE, 1.0f);
							else
								rResultList.Add(SNTOK_EAN8, 1.0f);
						}
						else if(SCalcCheckDigit(SCHKDIGALG_RUINN|SCHKDIGALG_TEST, reinterpret_cast<const char *>(pToken), stat.Len)) {
							rResultList.Add(SNTOK_RU_INN, 1.0f);
						}
						break;
					case 13:
						cd = SCalcBarcodeCheckDigitL(reinterpret_cast<const char *>(pToken), stat.Len-1);
						if((uchar)cd == (last-'0')) {
							rResultList.Add(SNTOK_EAN13, 1.0f);
						}
						break;
					case 15:
						if(SCalcCheckDigit(SCHKDIGALG_LUHN|SCHKDIGALG_TEST, reinterpret_cast<const char *>(pToken), stat.Len)) {
							rResultList.Add(SNTOK_IMEI, 0.9f);
							rResultList.Add(SNTOK_DIGITCODE, 0.1f);
						}
						break;
					case 19:
						if(SCalcCheckDigit(SCHKDIGALG_LUHN|SCHKDIGALG_TEST, reinterpret_cast<const char *>(pToken), stat.Len)) {
							rResultList.Add(SNTOK_LUHN, 0.9f);
							rResultList.Add(SNTOK_EGAISWARECODE, 0.1f);
						}
						else {
							rResultList.Add(SNTOK_EGAISWARECODE, 1.0f);
						}
						break;
				}
			}
			if(h & SNTOKSEQ_DECLAT) {
				if(stat.Len == 68)
					rResultList.Add(SNTOK_EGAISMARKCODE, 1.0f);
				else
					rResultList.Add(SNTOK_DIGLAT, 1.0f);
			}
			if(h & SNTOKSEQ_HEXHYPHEN) {
				if(stat.Len == 36) {
					uint   pos = 0;
					long   val = 0;
					if(chr_list.BSearch((long)'-', &val, &pos) && val == 4) {
						rResultList.Add(SNTOK_GUID, 1.0f);
					}
				}
			}
			if(h & (SNTOKSEQ_DECHYPHEN|SNTOKSEQ_DECSLASH|SNTOKSEQ_DECDOT)) {
				// 1-1-1 17-12-2016
				if(stat.Len >= 5 && stat.Len <= 10) {
					temp_buf.CatN(reinterpret_cast<const char *>(pToken), stat.Len);
					StringSet ss;
					const char * p_div = 0;
					if(h & SNTOKSEQ_DECHYPHEN)
						p_div = "-";
					else if(h & SNTOKSEQ_DECSLASH)
						p_div = "/";
					else if(h & SNTOKSEQ_DECDOT)
						p_div = ".";
					temp_buf.Tokenize(p_div, ss);
					const uint ss_count = ss.getCount();
					if(ss_count == 3) {
						if(_ProbeDate(temp_buf.Z().CatN(reinterpret_cast<const char *>(pToken), stat.Len))) {
							rResultList.Add(SNTOK_DATE, 0.8f);
						}
					}
				}
			}
			if(h & SNTOKSEQ_DECDOT) {
				// 1.1.1.1 255.255.255.255
				temp_buf.Z().CatN(reinterpret_cast<const char *>(pToken), stat.Len);
				StringSet ss('.', temp_buf);
				const uint ss_count = ss.getCount();
				if(ss_count == 2) {
					rResultList.Add(SNTOK_REALNUMBER, 0.9f);
				}
				if(stat.Len >= 3 && stat.Len <= 15) {
					if(ss_count == 4) {
						int   is_ip4 = 1;
						for(uint ssp = 0; is_ip4 && ss.get(&ssp, temp_buf);) {
							if(temp_buf.Empty())
								is_ip4 = 0;
							else {
								long v = temp_buf.ToLong();
								if(v < 0 || v > 255)
									is_ip4 = 0;
							}
						}
						if(is_ip4) {
							float prob = 0.95f;
							if(memcmp(pToken, "127.0.0.1", stat.Len) == 0)
								prob = 1.0f;
							rResultList.Add(SNTOK_IP4, prob);
						}
					}
					else if(oneof2(ss_count, 2, 3)) {
						int   is_ver = 1;
						for(uint ssp = 0; is_ver && ss.get(&ssp, temp_buf);) {
							if(temp_buf.Empty())
								is_ver = 0;
							else {
								long v = temp_buf.ToLong();
								if(v < 0 || v > 100)
									is_ver = 0;
							}
						}
						if(is_ver) {
							rResultList.Add(SNTOK_SOFTWAREVER, (ss_count == 3) ? 0.5f : 0.1f);
						}
					}
				}
			}
			if(h & SNTOKSEQ_NUMERIC) {
				if(num_potential_frac_delim && num_potential_frac_delim == num_potential_tri_delim) {
					rResultList.Add(SNTOK_NUMERIC_COM, 0.6f);
					rResultList.Add(SNTOK_NUMERIC_DOT, 0.6f);
				}
				else if(num_potential_frac_delim == ',') {
					rResultList.Add(SNTOK_NUMERIC_COM, num_potential_tri_delim ? 0.7f : 0.95f);
				}
				else {
					rResultList.Add(SNTOK_NUMERIC_DOT, num_potential_tri_delim ? 0.8f : 0.99f);
				}
			}
			if(h & SNTOKSEQ_ASCII) {
				uint   pos = 0;
				long   val = 0;
				if(chr_list.BSearch((long)'@', &val, &pos) && val == 1 && InitReEmail() && P_ReEMail->Find(reinterpret_cast<const char *>(pToken))) {
					size_t _offs = P_ReEMail->start();
					size_t _len = P_ReEMail->end() - P_ReEMail->start();
					if(_offs == 0 && _len == stat.Len)
						rResultList.Add(SNTOK_EMAIL, 1.0f);
				}
			}
		}
    }
	stat.Seq = h;
	ASSIGN_PTR(pStat, stat);
	return ok;
}

#if SLTEST_RUNNING // {

int FASTCALL dconvstr_scan(const char * input, const char ** input_end, double * output, int * output_erange);

struct SlTestFixtureSString {
public:
	SlTestFixtureSString() : P_StrList(0)
	{
	}
	~SlTestFixtureSString()
	{
		delete P_StrList;
	}
	int    FASTCALL InitStrList(const char * pFileName)
	{
		// phrases.en
		int    ok = 1;
        if(!P_StrList) {
			THROW(P_StrList = new SStrCollection);
			{
				SString temp_buf;
				SFile f_in(pFileName, SFile::mRead);
				THROW(f_in.IsValid());
				while(f_in.ReadLine(temp_buf)) {
					char * p_new_str = newStr(temp_buf);
					THROW(p_new_str);
					THROW(P_StrList->insert(p_new_str));
				}
			}
		}
		else
			ok = -1;
        CATCHZOK
        return ok;
	}
	int    FASTCALL InitRandomRealList(uint maxCount)
	{
		int    ok = 1;
		SString temp_buf;
		if(!RandomRealList.getCount()) {
			SlThreadLocalArea & r_tla = SLS.GetTLA();
			for(uint j = 0; j < maxCount; j++) {
				//double r = r_tla.Rg.GetReal();
				double r = r_tla.Rg.GetGaussian(1.0E+9);
				RandomRealList.insert(&r);
				temp_buf.Z().Cat(r, MKSFMTD(0, 32, NMBF_NOTRAILZ));
				SsRrl.add(temp_buf);
			}
		}
		else
			ok = -1;
		return ok;
	}
    SStrCollection * P_StrList;
	RealArray RandomRealList;
	StringSet SsRrl;
};

#include <string>
//
// Аналог strnzcpy но с использованием xeos_memchr вместо memchr
//
static char * FASTCALL xeos_strnzcpy(char * dest, const char * src, size_t maxlen)
{
	if(dest)
		if(src)
			if(maxlen) {
				const char * p = (SLS.GetSSys().CpuCs >= SSystem::cpucsSSE2) ? (const char *)xeos_memchr32_sse2(src, 0, maxlen) : (const char *)xeos_memchr32(src, 0, maxlen);
				if(p)
					memcpy(dest, src, (size_t)(p - src)+1);
				else {
					memcpy(dest, src, maxlen-1);
					dest[maxlen-1] = 0;
				}
			}
			else
				strcpy(dest, src);
		else
			dest[0] = 0;
	return dest;
}

SLTEST_FIXTURE(SString, SlTestFixtureSString)
{
	// benchmark: benchmark=stack;sstring;revolver;atof;satof
	int    ok = 1;
	int    bm = -1;
	const  uint max_bm_phase = 10000;
	const  uint max_strlen_phase = 100000;
	uint32 line_no = 0;
	SBaseBuffer temp_buf;
	SBaseBuffer test_buf;
	temp_buf.Init();
	test_buf.Init();
	SString str, out_buf, in_buf;
    THROW(SLTEST_CHECK_NZ(F.InitStrList(MakeInputFilePath("phrases.en"))));
	F.InitRandomRealList(1000000);
	if(pBenchmark == 0) bm = 0;
	else if(sstreqi_ascii(pBenchmark, "stack"))        bm = 1;
	else if(sstreqi_ascii(pBenchmark, "stack-xeos"))   bm = 6;
	else if(sstreqi_ascii(pBenchmark, "sstring"))      bm = 2;
	else if(sstreqi_ascii(pBenchmark, "std::string"))  bm = 5;
	else if(sstreqi_ascii(pBenchmark, "revolver"))     bm = 3;
	else if(sstreqi_ascii(pBenchmark, "revolver-tla")) bm = 4;
	else if(sstreqi_ascii(pBenchmark, "atof"))         bm = 7;
	else if(sstreqi_ascii(pBenchmark, "satof"))        bm = 8;
	else if(sstreqi_ascii(pBenchmark, "strlen"))       bm = 9;
	else if(sstreqi_ascii(pBenchmark, "sstrlen"))      bm = 10;
	else SetInfo("invalid benchmark");
	if(bm == 0) {
		SFile out(MakeOutputFilePath("SString_NumberToLat.txt"), SFile::mWrite);
		for(uint i = 0; i < 1000; i++) {
			str.Z().NumberToLat(i);
			out.WriteLine(out_buf.Printf("%u\t\t%s\n", i, str.cptr()));
		}
		{
			SLTEST_CHECK_NZ(sstreqi_ascii("u", "u"));
			SLTEST_CHECK_NZ(sstreqi_ascii("u", "U"));
			SLTEST_CHECK_NZ(sstreqi_ascii("U", "u"));
			SLTEST_CHECK_Z(sstreqi_ascii("U", "u1"));
			SLTEST_CHECK_NZ(sstreqi_ascii("string", "sTrInG"));
			SLTEST_CHECK_NZ(sstreqi_ascii("string", "string"));
			SLTEST_CHECK_Z(sstreqi_ascii("string", "string "));
		}
		//
		// Тестирование функции CopyTo
		//
		{
			char   buffer[1024];
			char   preserve_buffer[sizeof(buffer)];
			SLS.GetTLA().Rg.ObfuscateBuffer(buffer, sizeof(buffer));
			memcpy(preserve_buffer, buffer, sizeof(preserve_buffer));
			//
			{
				const size_t _src_len = 503;
				str.Z();
				for(uint si = 0; si < _src_len; si++) {
					uint _v = SLS.GetTLA().Rg.GetUniformInt(100000);
					char _c = (_v % 26) + ((si & 1) ? 'a' : 'A');
					str.CatChar(_c);
				}
				SLTEST_CHECK_EQ(str.Len(), _src_len);
			}
			for(uint tl = 1; tl <= sizeof(buffer); tl++) {
				str.CopyTo(buffer, tl);
				if(tl > 1)
					SLTEST_CHECK_Z(str.CmpPrefix(buffer, 0));
				{
					const size_t real_data_len = MIN(tl, str.Len()+1);
					SLTEST_CHECK_Z(memcmp(str.cptr(), buffer, real_data_len-1));
					SLTEST_CHECK_EQ((uint8)buffer[real_data_len-1], (uint8)0);
					SLTEST_CHECK_Z(memcmp(buffer+real_data_len, preserve_buffer+real_data_len, sizeof(buffer)-real_data_len));
				}
			}
			{
				str.CopyTo(buffer, 0);
				SLTEST_CHECK_Z(str.CmpPrefix(buffer, 0));
				SLTEST_CHECK_Z(memcmp(str.cptr(), buffer, str.Len()));
				SLTEST_CHECK_EQ(strlen(buffer), str.Len());
				SLTEST_CHECK_EQ((uint8)buffer[str.Len()], (uint8)0);
				SLTEST_CHECK_Z(memcmp(buffer+str.Len()+1, preserve_buffer+str.Len()+1, sizeof(buffer)-str.Len()-1));
			}
		}
		{
			SLTEST_CHECK_EQ((str = " abc ").Strip(1), " abc");
			SLTEST_CHECK_EQ((str = " abc ").Strip(2), "abc ");
			SLTEST_CHECK_EQ((str = " abc ").Strip(0), "abc");
			SLTEST_CHECK_EQ((str = " abc ").Strip(), "abc");
			SLTEST_CHECK_EQ((str = "abc").Strip(), "abc");
			SLTEST_CHECK_EQ((str = "\"abc\"").StripQuotes(), "abc");
			SLTEST_CHECK_EQ((str = " \"abc\"" ).StripQuotes(), "abc");
			SLTEST_CHECK_EQ((str = "abc").StripQuotes(), "abc");
		}
		//
		// Тестирование функций EncodeMime64 и DecodeMime64
		//
		{
			SString org_bin_file_name;
			SString test_bin_file_name;
			size_t actual_size = 0;
			THROW(SLTEST_CHECK_NZ(temp_buf.Alloc(4096)));
			THROW(SLTEST_CHECK_NZ(test_buf.Alloc(8192)));
			{
				int r;
				org_bin_file_name = MakeInputFilePath("binfile.");
				SFile inf(org_bin_file_name, SFile::mRead|SFile::mBinary|SFile::mNoStd);
				SFile outf(MakeOutputFilePath("txtfile.temp"), SFile::mWrite);
				THROW(SLTEST_CHECK_NZ(inf.IsValid()));
				THROW(SLTEST_CHECK_NZ(outf.IsValid()));
				while((r = inf.Read(temp_buf.P_Buf, temp_buf.Size, &actual_size)) != 0) {
					size_t as = 0;
					str.EncodeMime64(temp_buf.P_Buf, actual_size).CR();
					str.DecodeMime64(test_buf.P_Buf, test_buf.Size, &as);
					THROW(SLTEST_CHECK_EQ(actual_size, as));
					THROW(SLTEST_CHECK_Z(memcmp(temp_buf.P_Buf, test_buf.P_Buf, as)));
					THROW(SLTEST_CHECK_NZ(outf.WriteLine(str)));
					if(r < 0)
						break;
				}
			}
			//
			{
				(in_buf = GetSuiteEntry()->OutPath).SetLastSlash().Cat("txtfile.temp");
				(test_bin_file_name = GetSuiteEntry()->OutPath).SetLastSlash().Cat("binfile.temp");
				SFile inf(in_buf, SFile::mRead);
				SFile outf(test_bin_file_name, SFile::mWrite|SFile::mBinary);
				THROW(SLTEST_CHECK_NZ(inf.IsValid()));
				THROW(SLTEST_CHECK_NZ(outf.IsValid()));
				THROW(SLTEST_CHECK_NZ(temp_buf.Alloc(8192)));
				while(inf.ReadLine(str)) {
					str.Chomp();
					THROW(SLTEST_CHECK_NZ(str.DecodeMime64(temp_buf.P_Buf, temp_buf.Size, &actual_size)));
					THROW(SLTEST_CHECK_NZ(outf.Write(temp_buf.P_Buf, actual_size)));
				}
			}
			SLTEST_CHECK_LT(0L, SFile::Compare(org_bin_file_name, test_bin_file_name, 0));
			//
			// Тестирование функций конвертации между различными кодировкам
			// Файл rustext.txt должен быть размером более 2048 байт и содержать русский
			// текст в кодировке windows-1251
			//
			{
				(in_buf = GetSuiteEntry()->InPath).SetLastSlash().Cat("rustext.txt");
				SFile inf(in_buf, SFile::mRead);
				(in_buf = GetSuiteEntry()->OutPath).SetLastSlash().Cat("rustext.out");
				SFile outf(in_buf, SFile::mWrite);
				in_buf = 0;
				while(inf.ReadLine(str)) {
					in_buf.Cat(str);
				}
				out_buf = in_buf;
				out_buf.ToUtf8();
				out_buf.Utf8ToOem();
				out_buf.Transf(CTRANSF_INNER_TO_OUTER);
				SLTEST_CHECK_Z(in_buf.Cmp(out_buf, 0));

				out_buf = in_buf;
				out_buf.ToUtf8();
				out_buf.Utf8ToChar();
				out_buf.ToOem();
				out_buf.Transf(CTRANSF_INNER_TO_OUTER);
				SLTEST_CHECK_Z(in_buf.Cmp(out_buf, 0));

				{
					SStringU s16;
					out_buf = in_buf;
					out_buf.ToUtf8();
					s16.CopyFromUtf8Strict(out_buf, out_buf.Len());
					s16.CopyToUtf8(out_buf, 1);
					out_buf.Utf8ToChar();
					out_buf.ToOem();
					out_buf.Transf(CTRANSF_INNER_TO_OUTER);
					SLTEST_CHECK_Z(in_buf.Cmp(out_buf, 0));
				}

				outf.WriteLine(out_buf);
			}
			//
			// Тестирование функции Tokenize
			//
			{
				(in_buf = GetSuiteEntry()->InPath).SetLastSlash().Cat("tokenize.txt");
				SFile inf(in_buf, SFile::mRead);
				StringSet ss_line(";"), ss_result(",");
				SString delim, src_buf, result_str;
				while(inf.ReadLine(str)) {
					str.Chomp();
					if(str.NotEmptyS()) {
						ss_line.setBuf(str);
						uint p = 0;
						if(ss_line.get(&p, src_buf)) {
							if(ss_line.get(&p, delim)) {
								if(ss_line.get(&p, result_str)) {
									ss_result.clear();
									src_buf.Tokenize(delim.NotEmpty() ? (const char *)delim : 0, ss_result);
									SLTEST_CHECK_NZ(result_str == ss_result.getBuf());
								}
							}
						}
					}
				}
			}
			//
			// Тестирование распознавания и валидации email адресов и одновременное тестирование функций
			// sstreq и sstreqi_ascii (воспользуемся теми фактами, что все адреса написаны латиницей и
			// имеют разбросанную длину).
			//
			{
				SStrScan scan;
				(in_buf = GetSuiteEntry()->InPath).SetLastSlash().Cat("email-list.txt");
				SFile inf(in_buf, SFile::mRead);
				line_no = 0;
				SLTEST_CHECK_NZ(sstreq(static_cast<const char *>(0), static_cast<const char *>(0)));
				SLTEST_CHECK_NZ(sstreqi_ascii(static_cast<const char *>(0), static_cast<const char *>(0)));
				SLTEST_CHECK_NZ(sstreqi_ascii((const uchar *)0, (const uchar *)0));
				while(inf.ReadLine(str)) {
					line_no++;
					str.Chomp();
					if(str.NotEmptyS()) {
						scan.Set(str, 0);
						int   emsr = scan.GetEMail(out_buf.Z());
						SLTEST_CHECK_NZ(emsr);
					}
					{
						out_buf = str;
						SLTEST_CHECK_NZ(sstreq(out_buf, str));
						SLTEST_CHECK_NZ(sstreq(str, str));
						out_buf.ToUpper();
						str.ToLower();
						SLTEST_CHECK_Z(sstreq(out_buf, str));
						SLTEST_CHECK_NZ(sstreqi_ascii(str, out_buf));
						out_buf.CatChar('x');
						SLTEST_CHECK_Z(sstreqi_ascii(str, out_buf));
						str.CatChar('y');
						SLTEST_CHECK_Z(sstreqi_ascii(str, out_buf));
					}
				}
			}
			{
				//
				// Тест функции IsLegalUtf8
				//
				(in_buf = GetSuiteEntry()->InPath).SetLastSlash().Cat("phrases-ru-1251.txt");
				SFile inf(in_buf, SFile::mRead);
				while(inf.ReadLine(str)) {
					//const char * p_test_string_1251 = "Захавай еще этих тупых французских булок";
					//str = p_test_string_1251;
					if(str.Chomp().NotEmptyS()) {
						SLTEST_CHECK_Z(str.IsLegalUtf8());
						str.Transf(CTRANSF_OUTER_TO_UTF8);
						SLTEST_CHECK_NZ(str.IsLegalUtf8());
					}
				}
			}
			{
				SLTEST_CHECK_EQ((str = "0").ToLong(), 0);
				SLTEST_CHECK_EQ((str = "abc").ToLong(), 0);
				SLTEST_CHECK_EQ((str = "\t+100 ").ToLong(), 100);
				SLTEST_CHECK_EQ((str = " -197 ").ToLong(), -197);
				SLTEST_CHECK_EQ((str = "0xbeefZ").ToLong(), 0xbeef);
				SLTEST_CHECK_EQ((str = " -0x2BcD7a92 ").ToLong(), -0x2BCD7A92);
				SLTEST_CHECK_EQ((str = " - 17 ").ToLong(), 0); // Между знаком - и числом не должно быть пробелов
				SLTEST_CHECK_EQ((str = "0").ToInt64(), 0);
				SLTEST_CHECK_EQ((str = "abc").ToInt64(), 0);
				SLTEST_CHECK_EQ((str = "\t 0x1ABCDEF234567890").ToInt64(), 0x1ABCDEF234567890LL);
				SLTEST_CHECK_EQ((str = "\t\t123000012878963").ToInt64(), 123000012878963LL);
				SLTEST_CHECK_EQ((str = "-123000012878963").ToInt64(), -123000012878963LL);
			}
			{
				const char * p_atof_tab[] = {
					"0", " 0.001 ", "  -1.0", "+.1", "0.15", "-0.177777777", "+0003.2533333", "0010.0001", "1E-3", ".1e+9",
					"-1.1", "1111111.315687841464787467897878", "-333333.7646878744111165464", "1548754.02117e-17"
				};
				for(uint i = 0; i < SIZEOFARRAY(p_atof_tab); i++) {
					const char * p_text = p_atof_tab[i];
					double v_satof;
					const double v_atof = atof(p_text);
					//SLTEST_CHECK_NZ(satof(p_text, &v_satof));
					//SLTEST_CHECK_EQ(v_satof, v_atof);

					const char * p_end = 0;
					int erange = 0;
					SLTEST_CHECK_NZ(dconvstr_scan(p_text, &p_end, &v_satof, &erange));
					SLTEST_CHECK_EQ(v_satof, v_atof);
				}
				SString atof_buf;
				for(uint j = 0; j < F.RandomRealList.getCount(); j++) {
					atof_buf.Z().Cat(F.RandomRealList.at(j), MKSFMTD(0, 20, NMBF_NOTRAILZ));
					double v_satof;
					const double v_atof = atof(atof_buf);
					const char * p_end = 0;
					int erange = 0;
					SLTEST_CHECK_NZ(dconvstr_scan(atof_buf, &p_end, &v_satof, &erange));
					//SLTEST_CHECK_NZ(satof(atof_buf, &v_satof));
					SLTEST_CHECK_EQ(v_satof, v_atof);
				}
			}
			{
				SLTEST_CHECK_EQ(str.Z().Cat(0.1, MKSFMTD(0, 2, 0)), "0.10");
				SLTEST_CHECK_EQ(str.Z().Cat(17.1997, MKSFMTD(0, 3, NMBF_DECCOMMA)), "17,200");
				SLTEST_CHECK_EQ(str.Z().Cat(135.1997, MKSFMTD(0, 0, 0)), "135");
				SLTEST_CHECK_EQ(str.Z().Cat(3308.04, MKSFMTD(0, 8, NMBF_OMITEPS|NMBF_NOTRAILZ)), "3308.04");
				SLTEST_CHECK_EQ(str.Z().Cat(3308.039999999999512506, MKSFMTD(0, 13, NMBF_OMITEPS|NMBF_NOTRAILZ)), "3308.04");
				SLTEST_CHECK_EQ(str.Z().Cat(3308.04, MKSFMTD(0, 13, NMBF_OMITEPS|NMBF_NOTRAILZ)), "3308.04");
				SLTEST_CHECK_EQ(str.Z().Cat(2572.92*1.00000000001, MKSFMTD(0, 13, NMBF_OMITEPS|NMBF_NOTRAILZ)), "2572.92");
				SLTEST_CHECK_EQ(str.Z().Cat(369.900000000000102333,  MKSFMTD(0, 13, NMBF_OMITEPS|NMBF_NOTRAILZ)), "369.9");
			}
			{
				LDATETIME dtm;
				LDATETIME dtm_converted;
				dtm.d.encode(29, 2, 2016);
				dtm.t.encode(21, 17, 2, 250);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_DMY), "29/02/16");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_DMY), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_DMY|DATF_CENTURY), "29/02/2016");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_DMY), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_GERMAN|DATF_CENTURY), "29.02.2016");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_GERMAN), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_AMERICAN|DATF_CENTURY), "02/29/2016");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_AMERICAN), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_ANSI|DATF_CENTURY), "2016.02.29");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_ANSI), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_ITALIAN|DATF_CENTURY), "29-02-2016");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_ITALIAN), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_ITALIAN), "29-02-16");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_ITALIAN), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_JAPAN), "16/02/29");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_JAPAN), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_ISO8601|DATF_CENTURY), "2016-02-29");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_ISO8601), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_ISO8601), "16-02-29");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_ISO8601), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_AMERICAN|DATF_CENTURY|DATF_NODIV), "02292016");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_AMERICAN|DATF_CENTURY|DATF_NODIV), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_DMY|DATF_NODIV), "290216");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_DMY|DATF_NODIV), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_SQL), "DATE '2016-02-29'");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_SQL), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.d, DATF_INTERNET), "Mon, 29 Feb 2016");
				// (Такой формат не распознается) SLTEST_CHECK_EQ(strtodate_(str, DATF_INTERNET), dtm.d);
				SLTEST_CHECK_EQ(str.Z().Cat(ZERODATE, DATF_DMY), "");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_DMY), ZERODATE);
				SLTEST_CHECK_EQ(str.Z().Cat(ZERODATE, DATF_DMY|DATF_NOZERO), "");
				SLTEST_CHECK_EQ(strtodate_(str, DATF_DMY|DATF_NOZERO), ZERODATE);

				SLTEST_CHECK_EQ(str.Z().Cat(dtm.t, TIMF_HMS), "21:17:02");
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.t, TIMF_HMS|TIMF_NODIV), "211702");
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.t, TIMF_HM), "21:17");
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.t, TIMF_HM|TIMF_NODIV), "2117");
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.t, TIMF_MS), "17:02");
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.t, TIMF_MS|TIMF_NODIV), "1702");
				SLTEST_CHECK_EQ(str.Z().Cat(dtm.t, TIMF_HMS|TIMF_MSEC), "21:17:02.250");

				SLTEST_CHECK_EQ(str.Z().Cat(dtm, DATF_ISO8601|DATF_CENTURY, 0), "2016-02-29T21:17:02");
				SLTEST_CHECK_EQ(str.Z().Cat(dtm, DATF_ISO8601, 0), "16-02-29T21:17:02");
				SLTEST_CHECK_EQ(str.Z().Cat(dtm, DATF_MDY|DATF_CENTURY, TIMF_HMS|TIMF_MSEC), "02/29/2016 21:17:02.250");
				//
				strtotime("211702", TIMF_HMS|TIMF_NODIV, &dtm_converted.t);
				SLTEST_CHECK_EQ(dtm_converted.t, encodetime(21, 17, 02, 0));
				strtotime("\t21:17:02    ", TIMF_HMS, &dtm_converted.t);
				SLTEST_CHECK_EQ(dtm_converted.t, encodetime(21, 17, 02, 0));
				strtotime("      21:17:02.047   ", TIMF_HMS, &dtm_converted.t);
				SLTEST_CHECK_EQ(dtm_converted.t, encodetime(21, 17, 02, 4));
				strtotime(" 21 17 02 ", TIMF_HMS, &dtm_converted.t);
				SLTEST_CHECK_EQ(dtm_converted.t, encodetime(21, 17, 02, 0));
				strtotime(" 2117 ", TIMF_HM|TIMF_NODIV, &dtm_converted.t);
				SLTEST_CHECK_EQ(dtm_converted.t, encodetime(21, 17, 00, 0));
			}
			{
				IntRange ir;
				int    r;
				{
					r = str.Z().ToIntRange(ir, SString::torfDoubleDot);
					SLTEST_CHECK_Z(r);
				}
				{
					r = (str = "12..987").ToIntRange(ir, SString::torfDoubleDot);
					SLTEST_CHECK_NZ(r);
					SLTEST_CHECK_EQ(r, (long)str.Len());
					SLTEST_CHECK_EQ(ir.low, 12);
					SLTEST_CHECK_EQ(ir.upp, 987);
				}
				{
					// Здесь .. не будет распознано как разделитель границ (SString::torfHyphen)
					r = (str = "12..987").ToIntRange(ir, SString::torfHyphen);
					SLTEST_CHECK_NZ(r);
					SLTEST_CHECK_EQ(r, 2L);
					SLTEST_CHECK_EQ(ir.low, 12);
					SLTEST_CHECK_EQ(ir.upp, 12);
				}
				{
					r = (str = " -12 : 987q").ToIntRange(ir, SString::torfAny);
					SLTEST_CHECK_NZ(r);
					SLTEST_CHECK_EQ(r, (long)(str.Len()-1));
					SLTEST_CHECK_EQ(ir.low, -12);
					SLTEST_CHECK_EQ(ir.upp, 987);
				}
				{
					r = (str = " -12,,-987 z").ToIntRange(ir, SString::torfAny);
					SLTEST_CHECK_NZ(r);
					SLTEST_CHECK_EQ(r, (long)(str.Len()-2));
					SLTEST_CHECK_EQ(ir.low, -12);
					SLTEST_CHECK_EQ(ir.upp, -987);
				}
				{
					r = (str = " -12000 --987 z").ToIntRange(ir, SString::torfAny);
					SLTEST_CHECK_NZ((long)r);
					SLTEST_CHECK_EQ(r, (long)(str.Len()-2));
					SLTEST_CHECK_EQ(ir.low, -12000);
					SLTEST_CHECK_EQ(ir.upp, -987);
				}
				{
					SLTEST_CHECK_Z((str = " &12").ToIntRange(ir, SString::torfAny));
				}
				{
					r = (str = " +19 ").ToIntRange(ir, SString::torfAny);
					SLTEST_CHECK_NZ(r);
					SLTEST_CHECK_EQ(r, (long)(str.Len()-1));
					SLTEST_CHECK_EQ(ir.low, 19);
					SLTEST_CHECK_EQ(ir.upp, 19);
				}
				{
					r = (str = " -1234567890 ").ToIntRange(ir, SString::torfAny);
					SLTEST_CHECK_NZ(r);
					SLTEST_CHECK_EQ(r, (long)(str.Len()-1));
					SLTEST_CHECK_EQ(ir.low, -1234567890);
					SLTEST_CHECK_EQ(ir.upp, -1234567890);
				}
			}
		}
		{
			//
			// Тест функций xeos_strlen и xeos_memchr
			//
			uint64 total_len1 = 0;
			uint64 total_len2 = 0;
			char buffer1[2048];
			char buffer2[2048];
			for(uint i = 0; i < F.P_StrList->getCount(); i++) {
				size_t proof_len = strlen(F.P_StrList->at(i));
				assert(proof_len < sizeof(buffer1));
				//assert(proof_len > 16);
				strcpy(buffer1, F.P_StrList->at(i));
				total_len1 += strlen(buffer1);
				buffer2[0] = 0;
				strcpy(buffer2, F.P_StrList->at(i));
				total_len2 += xeos_strlen(buffer2);
				SLTEST_CHECK_NZ(sstreq(buffer1, buffer2));
				SLTEST_CHECK_Z(strcmp(buffer1, buffer2));
				{
					for(uint offs = 0; offs < 8; offs++) {
						const char * p_buf1 = buffer1+offs;
						const char * p_buf2 = buffer2+offs;
						for(int j = 0; j < 256; j++) {
							const char * p1 = (const char *)memchr(p_buf1, j, proof_len+1-offs);
							const char * p2 = (const char *)xeos_memchr(p_buf2, j, proof_len+1-offs);
							SLTEST_CHECK_NZ((!p1 && !p2) || ((p1 && p2) && (p1-p_buf1) == (p2-p_buf2)));
						}
					}
				}
			}
			SLTEST_CHECK_EQ(total_len1, total_len2);
		}
		{
			//
			// Кроме "законных" вариантов представления символов в строки добавлены ложные цели для проверки корректной работы
			// &#, &, &#32 (без ';' в конце)
			//
			const char * p_src_xmlenc = "<ФИОИП &#Фамилия=\"&#x423;&#x421;&#x41A;&#x41E;&#x412;\" &Имя=\"&#x415;&#x412;&#x413;&#x415;&#x41D;&#x418;&#x419;\" &#32Отчество=\"&#x41D;&#x418;&#x41A;&#x41E;&#x41B;&#x410;&#x415;&#x412;&#x418;&#x427;\"/>";
			const char * p_result_xmlenc = "<ФИОИП &#Фамилия=\"УСКОВ\" &Имя=\"ЕВГЕНИЙ\" &#32Отчество=\"НИКОЛАЕВИЧ\"/>";
			(str = p_src_xmlenc).Decode_XMLENT(out_buf);
			SLTEST_CHECK_EQ(out_buf, p_result_xmlenc);
		}
		{
			STokenRecognizer tr;
			SNaturalTokenStat nts;
			SNaturalTokenArray nta;
			tr.Run((const uchar *)"0123", -1, nta.Z(), &nts); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_DIGITCODE));
			tr.Run((const uchar *)"4610017121115", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_DIGITCODE));
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_EAN13));
			tr.Run((const uchar *)"4610017121116", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_DIGITCODE));
			SLTEST_CHECK_EQ(0.0f, nta.Has(SNTOK_EAN13)); // Инвалидная контрольная цифра
			tr.Run((const uchar *)"20352165", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_DIGITCODE));
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_EAN8));
			tr.Run((const uchar *)"100100802804", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_DIGITCODE));
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_RU_INN));
			tr.Run((const uchar *)"0034012000001472206", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_DIGITCODE));
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_EGAISWARECODE));
			tr.Run((const uchar *)"a98P8s00W", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_DIGLAT));
			tr.Run((const uchar *)"4FC737F1-C7A5-4376-A066-2A32D752A2FF", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_GUID));
			tr.Run((const uchar *)"mail.123@gogo-fi.com", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_EMAIL));
			// @notimplemented tr.Run((const uchar *)"123-15-67", -1, nta.Z(), 0); 
			// @notimplemented SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_PHONE));
			// @notimplemented tr.Run((const uchar *)"+7(911)123-15-67", -1, nta.Z(), 0); 
			// @notimplemented SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_PHONE));
			tr.Run((const uchar *)"354190023896443", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_DIGITCODE));
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_IMEI));
			tr.Run((const uchar *)"192.168.0.1", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_IP4));
			tr.Run((const uchar *)"1/12/2018", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_DATE));
			tr.Run((const uchar *)"20180531", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_DATE));
			// @notimplemented tr.Run((const uchar *)"11:30", -1, nta.Z(), 0); 
			// @notimplemented SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_TIME));
			// @notimplemented tr.Run((const uchar *)"11:30:46", -1, nta.Z(), 0); 
			// @notimplemented SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_TIME));
			tr.Run((const uchar *)"10.2.5", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_SOFTWAREVER));
			tr.Run((const uchar *)"#000000", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_COLORHEX));
			tr.Run((const uchar *)"#f82aB7", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_COLORHEX));

			tr.Run((const uchar *)"+100,000.00", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_NUMERIC_DOT));
			tr.Run((const uchar *)"+100 000,00", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_NUMERIC_COM));
			tr.Run((const uchar *)"-100'000,00", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_NUMERIC_COM));
			tr.Run((const uchar *)"9", -1, nta.Z(), 0); 
			SLTEST_CHECK_LT(0.0f, nta.Has(SNTOK_NUMERIC_DOT));
		}
	}
	else {
		uint64 total_len = 0;
		const uint scc = F.P_StrList->getCount();
		if(bm == 1) {
			for(uint phase = 0; phase < max_bm_phase; phase++) {
				for(uint i = 0; i < scc; i++) {
					char buffer[2048];
					strnzcpy(buffer, F.P_StrList->at(i), sizeof(buffer));
					total_len += strlen(buffer);
				}
			}
		}
		else if(bm == 6) {
			for(uint phase = 0; phase < max_bm_phase; phase++) {
				for(uint i = 0; i < scc; i++) {
					char buffer[2048];
					xeos_strnzcpy(buffer, F.P_StrList->at(i), sizeof(buffer));
					total_len += xeos_strlen(buffer);
				}
			}
		}
		else if(bm == 2) {
			for(uint phase = 0; phase < max_bm_phase; phase++) {
				for(uint i = 0; i < scc; i++) {
					SString buffer;
					buffer = F.P_StrList->at(i);
					total_len += strlen(buffer.cptr());
				}
			}
		}
		else if(bm == 5) {
			for(uint phase = 0; phase < max_bm_phase; phase++) {
				for(uint i = 0; i < scc; i++) {
					std::string buffer;
					buffer = F.P_StrList->at(i);
					total_len += strlen(buffer.c_str());
				}
			}
		}
		else if(bm == 3) {
			SRevolver_SString rvl(1024);
			for(uint phase = 0; phase < max_bm_phase; phase++) {
				for(uint i = 0; i < scc; i++) {
					SString & r_buffer = rvl.Get();
					r_buffer = F.P_StrList->at(i);
					total_len += strlen(r_buffer.cptr());
				}
			}
		}
		else if(bm == 4) {
			for(uint phase = 0; phase < max_bm_phase; phase++) {
				for(uint i = 0; i < scc; i++) {
					SString & r_buffer = SLS.AcquireRvlStr();
					r_buffer = F.P_StrList->at(i);
					total_len += strlen(r_buffer.cptr());
				}
			}
		}
		else if(bm == 7) {
			SString atof_buf;
			for(uint ssp = 0; F.SsRrl.get(&ssp, atof_buf);) {
			//for(uint j = 0; j < F.RandomRealList.getCount(); j++) {
				//double r = F.RandomRealList.at(j);
				//atof_buf.Z().Cat(r, MKSFMTD(0, 32, NMBF_NOTRAILZ));
				double r2;
				r2 = atof(atof_buf);
			}
		}
		else if(bm == 8) {
			SString atof_buf;
			for(uint ssp = 0; F.SsRrl.get(&ssp, atof_buf);) {
			//for(uint j = 0; j < F.RandomRealList.getCount(); j++) {
				//double r = F.RandomRealList.at(j);
				//atof_buf.Z().Cat(r, MKSFMTD(0, 32, NMBF_NOTRAILZ));
				double r2;
				//satof(atof_buf, &r2);
				//
				const char * p_end = 0;
				int erange = 0;
				dconvstr_scan(atof_buf, &p_end, &r2, &erange);
			}
		}
		else if(bm == 9) { // strlen
			for(uint phase = 0; phase < max_strlen_phase; phase++) {
				for(uint i = 0; i < scc; i++)
					total_len += strlen(F.P_StrList->at(i));
			}
		}
		else if(bm == 10) { // sstrlen
			for(uint phase = 0; phase < max_strlen_phase; phase++) {
				for(uint i = 0; i < scc; i++)
					total_len += sstrlen(F.P_StrList->at(i));
			}
		}
	}
	CATCH
		CurrentStatus = ok = 0;
	ENDCATCH
	test_buf.Destroy();
	temp_buf.Destroy();
	return CurrentStatus;
}

SLTEST_R(SPathStruc)
{
	int    ok = 1;
	SFile file(MakeInputFilePath("path.txt"), SFile::mRead | SFile::mBinary);
	if(file.IsValid()) {
		SString line_buf;
		SFile out_file(MakeOutputFilePath("path.out"), SFile::mWrite);
		SPathStruc ps;
		while(file.ReadLine(line_buf)) {
			SInvariantParam ip;
			ps.Split(line_buf.Chomp());
			SLTEST_CHECK_NZ(ps.Invariant(&ip));
			out_file.WriteLine(ip.MsgBuf.CR());
		}
	}
	return CurrentStatus;
}

#endif // } SLTEST_RUNNING
