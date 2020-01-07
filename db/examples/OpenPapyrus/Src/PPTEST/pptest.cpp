// PPTEST.CPP
// Copyright (c) A.Sobolev 2006, 2007, 2008, 2010, 2012, 2015, 2016, 2017, 2019
//
#include <pp.h>
#pragma hdrstop

//#define TEST_LOCALE
//#define TEST_HASH_AND_SEARCH
//#define TEST_DBTEXTFILE
//#define TEST_EDITIMPEXPPARAM
//#define TEST_REGEXP
//#define TEST_DL600

// @v9.8.11 int SLAPI Debug_GetFilesFromMessage(const char * pMsgFile); // filtrnsm.cpp

#if 0 // {

class TestQ {
public:
	unsigned long v;
};

class TestABC {
public:
	TestABC();
	TestABC(int);
	operator double ();
	operator long ();
	operator TestQ ();
	int operator -- ();
	int foo(int a, int b);
	int fooC(int a, int b) const;
	int M;
};

class Test2 {
public:
	int R;
};

int TestABC::foo(int a, int b) { return a+b; }
int TestABC::fooC(int a, int b) const { return a+b; }
int TestABC::operator -- () { return --M; }
TestABC::operator double () { return (double)M; }
TestABC::operator long () { return (long)M; }
TestABC::operator TestQ() { TestQ d; d.v = (ulong)M; return d; }
int operator + (TestABC & rAbc, int a) { return rAbc.M + a; }
int operator - (TestABC & rAbc, int a) { return rAbc.M + a; }
int operator * (TestABC & rAbc, int a) { return rAbc.M * a; }
int operator / (TestABC & rAbc, int a) { return rAbc.M / a; }
int operator != (TestABC & rAbc, int a) { return (rAbc.M != a); }
int operator <= (TestABC & rAbc, int a) { return (rAbc.M <= a); }
int operator >= (TestABC & rAbc, int a) { return (rAbc.M >= a); }
int test_foo(TestABC & rAbc) { return 0; }
int test_foo(TestABC * pAbc) { return 0; }
int test_foo(TestABC a1, TestABC a2) { return 0; }

//?test_foo@@YAHVTestQ@@VTestABC@@VTest2@@12@Z       == int test_foo(TestQ q, TestABC a1, Test2 _2, TestABC a2, Test2 _4)
//?test_foo@@YAHVTestQ@@VTestABC@@VTest2@@1PAV3@@Z   == int test_foo(TestQ q, TestABC a1, Test2 _2, TestABC a2, Test2 * _4)
//?test_foo@@YAHVTestQ@@VTestABC@@VTest2@@PAV2@2@Z   == int test_foo(TestQ q, TestABC a1, Test2 _2, TestABC * a2, Test2 _4)
//?test_foo@@YAHAAVTestQ@@VTestABC@@VTest2@@PAV2@2@Z == int test_foo(TestQ & q, TestABC a1, Test2 _2, TestABC * a2, Test2 _4)
//?test_foo@@YA?AVTestQ@@AAV1@@Z                     == TestQ test_foo(TestQ & q)
//?test_foo@@YAAAVTestQ@@AAV1@@Z                     == TestQ & test_foo(TestQ & q)

int  test_foo(int a, int b) { return (a+b); }
int  test_foo(int * a, int b) { return (*a + b); }
void test_foo2(int * a, int b) { *a = *a+b; }

#endif // } 0

#if !SLTEST_RUNNING // {

//#include <ppsoapclient.h>

int ClassfGoods();
int TestTddo();
int GetGoodsFromService(SString & rCode, PPGoodsPacket * pPack);
int SLAPI TestSTree();

int SLAPI TestNoLogin()
{
	//TestSTree(); // @debug
	return -1;
}

int SLAPI TestLogin()
{
	//ClassfGoods(); // @debug
	//TestTddo(); // @debug
	//Debug_GetFilesFromMessage("D:\\PAPYRUS\\PPY\\IN\\00000001(2).msg");

	/*
	PPUhttClient UhttClient;
	UhttSCardPacket pack;

	if(UhttClient.Auth()) {
		UhttClient.GetSCardByNumber("123ABC", pack);

		UhttCheckPacket check;
		check.Amount = 1234.56;
		int r = UhttClient.CreateSCardCheck("myloc3", "123ABC", check);

		UhttClient.WithdrawSCardAmount("123ABC", 10.5);

		double rest = 0;
		r = UhttClient.GetSCardRest("123ABC", 0, rest);

		UhttClient.DepositSCardAmount("123ABC", 100);
		r = UhttClient.GetSCardRest("123ABC", 0, rest);
	}

	// SCARD_1:AE;SCARD_2:AO;A;SCARD_2:C;SCARD_1:D;CO
	PPGlobalAccRights per_blk("A;:E");

	int ad = per_blk.IsAllow(PPGlobalAccRights::fAccess);
	int cd = per_blk.IsAllow(PPGlobalAccRights::fCreate);
	int ed = per_blk.IsAllow(PPGlobalAccRights::fEdit);
	int dd = per_blk.IsAllow(PPGlobalAccRights::fDelete);
	int od = per_blk.IsAllow(PPGlobalAccRights::fOperation);

	int a1 = per_blk.IsAllow(PPGlobalAccRights::fAccess, "SCARD_1");
	int c1 = per_blk.IsAllow(PPGlobalAccRights::fCreate, "SCARD_1");
	int e1 = per_blk.IsAllow(PPGlobalAccRights::fEdit, "SCARD_1");
	int d1 = per_blk.IsAllow(PPGlobalAccRights::fDelete, "SCARD_1");
	int o1 = per_blk.IsAllow(PPGlobalAccRights::fOperation, "SCARD_1");

	int a2 = per_blk.IsAllow(PPGlobalAccRights::fAccess, "SCARD_2");
	int c2 = per_blk.IsAllow(PPGlobalAccRights::fCreate, "SCARD_2");
	int e2 = per_blk.IsAllow(PPGlobalAccRights::fEdit, "SCARD_2");
	int d2 = per_blk.IsAllow(PPGlobalAccRights::fDelete, "SCARD_2");
	int o2 = per_blk.IsAllow(PPGlobalAccRights::fOperation, "SCARD_2");
	*/
	return -1;
}

#else //

#include <locale.h>

/*
#define ABCD virtual void abc(int & r)

class A {
public:
	A();
	ABCD {
		r = a;
	}
	int a;
};
*/


static int TestSearch(int alg, int flags, const SString & rPat, const SString & rText, size_t numSucc, size_t * pSuccList)
{
	size_t num_suc_test = 0, srch_pos = 0;
	const  size_t max_suc = 256;
	size_t suc_test[max_suc];
	int    ok = 1, r = 0;

	SSrchPattern blk(rPat, flags, alg);
	//PROFILE_START
	for(size_t start = 0; (r = blk.Search(rText, start, rText.Len(), &srch_pos)) != 0; start = srch_pos+1) {
		if(num_suc_test < max_suc)
			suc_test[num_suc_test] = srch_pos;
		num_suc_test++;
	}
	//PROFILE_END
	if(num_suc_test == numSucc) {
		for(size_t j = 0; j < num_suc_test; j++)
			if(j < max_suc && pSuccList[j] != suc_test[j]) {
				PPLogMessage(PPFILNAM_DEBUG_LOG, "������ ��������� ������ �����: �� ��������� �������", 0);
				ok = 0;
			}
	}
	else {
		PPLogMessage(PPFILNAM_DEBUG_LOG, "������ ��������� ������ �����: �� ��������� ���������� ����", 0);
		ok = 0;
	}
	return ok;
}

int Test_Hash_And_Search(const char * pInputFileName)
{
	int    ok = 1;
	size_t sz = 0;
	MemLeakTracer mlt;
	{
		for(int i = 10000; sz == 0; i++)
			if(IsPrime(i))
				sz = (size_t)i;
	}
	{
		SymbHashTable tab(sz);
		SFile file(pInputFileName, SFile::mRead | SFile::mBinary);
		if(file.IsValid()) {
			SString word_buf, text_buf;
			SSrchPattern blk(0);
			long   file_pos = file.Tell();
			uint   i = 1;
			uint   pos = 0;
			//
			// ��������� ������ � ���-�������
			//
			// ������������ ����������� ������ � ����� ����� ��� ������������ ������� ������.
			//
			while(ok && file.ReadLine(word_buf)) {
				word_buf.Chomp();
				if(!tab.Search(word_buf, 0, &pos)) {
					text_buf.Cat(word_buf);
					if(!tab.Add(word_buf, (uint)file_pos, &pos))
						ok = 0;
				}
				else
					ok++;
				file_pos = file.Tell();
			}

			//
			// ��������� ��������� �� ���� ��������� ������ � ���-�������?
			//
			file.Seek(0, SEEK_SET);
			file_pos = file.Tell();
			while(ok && file.ReadLine(word_buf)) {
				uint val = 0;
				if(tab.Search(word_buf.Chomp(), &val, 0)) {
					size_t srch_pos = 0;
					size_t num_suc = 0;
					const size_t max_suc = 256;
					size_t suc[max_suc];

					if(val != (uint)file_pos)
						ok = 0;
					file_pos = file.Tell();
					//
					// ��������� ������� ������
					//
					int srch_flags = 0;//SSrchPattern::fNoCase;
					if(srch_flags == SSrchPattern::fNoCase) {
						int r;
						memzero(suc, sizeof(suc));
						PROFILE_START_S("standard no case");
						SSrchPattern blk(word_buf, srch_flags, SSrchPattern::algDefault);
						for(size_t start = 0; (r = blk.Search(text_buf, start, text_buf.Len(), &srch_pos)) != 0; start = srch_pos+1) {
							if(num_suc < max_suc)
								suc[num_suc] = srch_pos;
							num_suc++;
						}
						PROFILE_END
					}
					else {
						//
						// ������������ ����� ��� ��������� ������������������
						//
						memzero(suc, sizeof(suc));
						PROFILE_START_S("standard");
						const char * p = 0;
						for(size_t start = 0; (p = strstr(text_buf+start, word_buf)) != 0; start = srch_pos+1) {
							srch_pos = (p-text_buf);
							if(num_suc < max_suc)
								suc[num_suc] = srch_pos;
							num_suc++;
						}
						PROFILE_END
					}
					PROFILE_S(TestSearch(SSrchPattern::algBmGoodSfx, srch_flags, word_buf, text_buf, num_suc, suc), "algBmGoodSfx");
					PROFILE_S(TestSearch(SSrchPattern::algBmBadChr,  srch_flags, word_buf, text_buf, num_suc, suc), "algBmBadChr");
					PROFILE_S(TestSearch(SSrchPattern::algDefault,   srch_flags, word_buf, text_buf, num_suc, suc), "algDefault");
				}
				else
					ok = 0;
			}
			{
				//
				// ��������� ������� ������ ��� ��������� �����. ��������� ������ ���� �������������.
				//
				char * miss_strings[] = {
					"��������� �����",
					"if(!r || strncmp(text_buf + srch_pos, word_buf, word_buf.Len()) != ",
					"",
					"$$##!@ ",
					"��������� ������ ���� �������������."
				};
				for(i = 0; i < sizeof(miss_strings) / sizeof(char *); i++) {
					size_t srch_pos = 0;
					blk.Init(word_buf);
					int r = blk.Search(text_buf, 0, text_buf.Len(), &srch_pos);
					if(r && strncmp(text_buf + srch_pos, word_buf, word_buf.Len()) != 0)
						ok = 0;
				}
			}
			//
			// ���������� ��� �������� ���-������� � ��������� �� � ��������� ����.
			// � ���������� �� ������ �������� ����, ���������� ����� ������� �� �����, ��� � �������� ����.
			//
			{
				SString fn = pInputFileName;
				//replaceExt(fn, "OUT", 1);
				SPathStruc::ReplaceExt(fn, "out", 1);
				SFile out_file(fn, SFile::mWrite);
				if(out_file.IsValid()) {
					SymbHashTable::Iter iter;
					for(tab.InitIteration(&iter); tab.NextIteration(&iter, &i, 0, &word_buf);)
						out_file.WriteLine(word_buf.CR());
				}
				else
					ok = 0;
			}
			//
			//
			//
		}
		else
			ok = 0;
	}
	return ok;
}

#ifdef TEST_REGEXP
int Test_RegExp(const char * pInputFileName)
{
	const  char * p_regexp_list[] = {
		"[ \t]+",   // whitespaces
		"\"[^\"]*\"",      // quoted string
		"[+-]?[0-9]+",  // integer
		"[+-]?[0-9]*([\\.][0-9]*)([Ee][+-]?[0-9]+)?", // real
		"\\[[ \t]*[0-9]+(\\.[0-9]*)?[ \t]*\\]",
		"\\([ \t]*[0-9]+(\\.[0-9]*)?[ \t]*\\)",
		"[0-9]+[ \t]*\\,[ \t]*[0-9]+",
		"(\\[[0-9]+(\\.[0-9]*)?\\]) | (\\([0-9]+(\\.[0-9]*)?\\)) | ([0-9]+\\,[0-9]+)"
	};
	int    ok = 1;
	char   fn[MAXPATH];
	SFile file(pInputFileName, SFile::mRead | SFile::mBinary);
	if(file.IsValid()) {
		SString line_buf, temp_buf;
		CRegExp re;

		STRNSCPY(fn, pInputFileName);
		replaceExt(fn, "OUT", 1);
		SFile out_file(fn, SFile::mWrite);

		for(uint r = 0; r < sizeof(p_regexp_list) / sizeof(*p_regexp_list); r++) {
			long   file_pos = file.Tell(SEEK_SET);
			uint   i = 1;
			uint   pos = 0;
			if(re.Compile(p_regexp_list[r])) {
				temp_buf = "RE: ";
				temp_buf.Cat(p_regexp_list[r]).CR();
				out_file.WriteLine(temp_buf);
				file.Seek(0, SEEK_SET);
				while(ok && file.ReadLine(line_buf)) {
					line_buf.Chomp();
					out_file.WriteLine(line_buf);
					SStrScan scan(line_buf);
					if(re.Find(&scan)) {
						do {
							out_file.WriteLine(0);
							scan.Get(temp_buf);
							out_file.WriteLine(temp_buf.Quot('*', '*').CR());
							scan.Offs += scan.Len;
						} while(re.Find(&scan));
					}
					else
						out_file.WriteLine((temp_buf = "Not Found").CR());
					//out_file.WriteLine(0);
				}
				//out_file.WriteLine(0);
			}
			else {
				temp_buf.Printf("Error compiling regexp %s; code = %d\n\n\n", p_regexp_list[r], SLibError);
				out_file.WriteLine(temp_buf);
			}
		}
	}
	return ok;
}
#endif

//#define PP_TEST

#ifdef PP_TEST
int TestSPathStruc(const char * pInputFileName)
{
	int    ok = 1;
	SFile file(pInputFileName, SFile::mRead | SFile::mBinary);
	if(file.IsValid()) {
		char   fn[MAXPATH];
		SString line_buf;
		STRNSCPY(fn, pInputFileName);
		replaceExt(fn, "OUT", 1);
		SFile out_file(fn, SFile::mWrite);
		SPathStruc ps;
		while(file.ReadLine(line_buf)) {
			SInvariantParam ip;
			ps.Split(line_buf.Chomp());
			if(!ps.Invariant(&ip))
				ok = 0;
			out_file.WriteLine(ip.MsgBuf.CR());
		}
	}
	return ok;
}
#endif
//
//
//
int Test_Alg_SS_Z(const char * pInputFileName);
int TestTextDbFile(const char * pInDbfFile);
int Test_InterfaceCall();

int SLAPI TestNoLogin()
{
	int    ok = -1;
	STestSuite s;
	SRng::CreateInstance((SRng::Algorithm)0, 0); // linking RandomNumberGeneragtor
	s.Run("\\papyrus\\src\\pptest\\testdef.ini");
	//Test_Alg_SS_Z("c:\\papyrus\\src\\pptest\\words.");
	//Test_InterfaceCall();
#ifdef PP_TEST
	TestSPathStruc("c:\\papyrus\\src\\pptest\\path.txt");
#endif
#ifdef TEST_LOCALE
	char * p_loc = setlocale(LC_COLLATE, "rus_rus.1251");
	SString c1, c2;
	c1 = "������";
	c2 = "������";
	//c1.ToOem();
	//c2.ToOem();
	if(stricmp(c1, c2) != 0) {
		ok = 0;
	}
#endif // TEST_LOCALE
#ifdef TEST_HASH_AND_SEARCH
	Test_Hash_And_Search("c:\\papyrus\\src\\pptest\\words.");
#endif // TEST_HASH_AND_SEARCH
#ifdef TEST_DBTEXTFILE
	TestTextDbFile("c:\\papyrus\\src\\pptest\\sp_goods.dbf");
#endif
#ifdef TEST_REGEXP
	Test_RegExp("c:\\papyrus\\src\\pptest\\re.txt");
#endif
	return ok;
}

int SLAPI Test_ImpExpParamDialog();
int Test_DL6_Rtm();

int SLAPI TestLogin()
{
	STestSuite s;
	s.Run("\\papyrus\\src\\pptest\\testdef_login.ini");
	// Debug_GetFilesFromMessage("D:\PAPYRUS\PPY\IN\00000001(2).msg");
#ifdef TEST_EDITIMPEXPPARAM
	Test_ImpExpParamDialog();
#endif
#ifdef TEST_DL600
	Test_DL6_Rtm();
#endif
	return -1;
}

#endif // } NDEBUG

// turistti	@v5.3.1 {
//
//Descr:��������� ��������� �����.
//  ����� ������������ ��� ������������������������ �����,
//  ��� � �������������� �� ���������� �-�� �����...
//
#ifndef VAR

#ifdef _WIN32
	#ifdef DLL
		#ifdef DLL_EXPORT
			#define VAR extern __declspec(dllexport)
		#else
			#define VAR extern __declspec(dllimport)
		#endif
	#else
		#define VAR extern
	#endif
#else
	#define VAR extern
#endif

#endif

typedef struct {
    const char * P_Name;
    uint Max;
    uint Min;
    size_t Size;
    void (* P_Set) (void * P_State, uint Seed);
    uint (* P_Get) (void * P_State);
    double (* P_GetDouble) (void * P_State);
} RngType;

typedef struct {
    const RngType * P_Type;
    void * P_State;
} Rng;

VAR   const RngType * Rng_Mt_19937;
VAR   uint RngDefaultSeed;
VAR   const RngType * RngDefault;

const RngType ** RngTypesSetup();
Rng * RngAlloc(const RngType * pT);
void   RngFree(Rng * pR);
void   RngSet(const Rng * pR, uint seed);
uint   RngMax(const Rng * pR);
uint   RngMin(const Rng * pR);
const  char * RngName(const Rng * pR);
size_t RngSize(const Rng * pR);
void * RngState(const Rng * pR);
const  RngType * RngEnvSetup();
uint   RngGet(const Rng * pR);
double RngUniform(const Rng * pR);
double RngUniformPos(const Rng * pR);
uint   RngUniformInt(const Rng * pR, uint n);

#ifdef HAVE_INLINE //{

extern inline uint RngGet (const Rng * pR);
extern inline uint RngGet (const Rng * pR) {return (pR->P_Type->P_Get) (pR->P_State);}
extern inline double RngUniform (const Rng * pR);
extern inline double RngUniform (const Rng * pR) {return (pR->P_Type->P_GetDouble) (pR->P_State);}
extern inline double RngUniformPos (const Rng * pR);
extern inline double RngUniformPos (const Rng * pR)
{
	double x;
	do {
		x = (pR->P_Type->P_GetDouble) (pR->P_State) ;
	}
	while (x == 0) ;
	return x ;
}

extern inline uint RngUniformInt (const Rng * pR, uint n);
extern inline uint RngUniformInt (const Rng * pR, uint n)
{
	uint offset = pR->P_Type->Min;
	uint range = pR->P_Type->Max - offset;
	uint scale;
	uint k;
	if(n > range || n == 0) {
		GSL_ERROR_VAL ("invalid n, either 0 or exceeds maximum value of generator", GSL_EINVAL, 0);
	}
	scale = range / n;
	do {
		k = (((pR->P_Type->P_Get) (pR->P_State)) - offset) / scale;
	}
	while (k >= n);
	return k;
}

#endif // } HAVE_INLINE

static inline uint MtGet (void * p_vstate);
static double MtGetDouble (void * p_vstate);
static void MtSet (void * p_state, uint s);

#define N 624   //Period parameters
#define M 397

static const ulong UPPER_MASK = 0x80000000UL; //most significant w-r bits
static const ulong LOWER_MASK = 0x7fffffffUL; //least significant r bits

typedef struct {
	unsigned long mt[N];
	int mti;
} MtStateT;

static inline uint MtGet(void * p_vstate)
{
	MtStateT * p_state = static_cast<MtStateT *>(p_vstate);
	unsigned long k ;
	ulong * const mt = p_state->mt;
#define MAGIC_RNG(y) (((y)&0x1) ? 0x9908b0dfUL : 0)
	if(p_state->mti >= N) {
		//generate N words at one time
		int kk = 0;
		for(kk = 0; kk < N - M; kk++) {
			unsigned long y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + M] ^ (y >> 1) ^ MAGIC_RNG(y);
		}
		for(; kk < N - 1; kk++) {
			unsigned long y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ MAGIC_RNG(y);
		}
		{
			unsigned long y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
			mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ MAGIC_RNG(y);
		}
		p_state->mti = 0;
	}
	//Tempering
	k = mt[p_state->mti];
	k ^= (k >> 11);
	k ^= (k << 7) & 0x9d2c5680UL;
	k ^= (k << 15) & 0xefc60000UL;
	k ^= (k >> 18);
	p_state->mti++;
	return k;
}

static double MtGetDouble (void * p_vstate) {return MtGet (p_vstate) / 4294967296.0;}

static void MtSet(void * p_vstate, uint s)
{
	MtStateT * p_state = static_cast<MtStateT *>(p_vstate);
	int i;
	if(s == 0)
		s = 4357;   //the default seed is 4357
	p_state->mt[0]= s & 0xffffffffUL;
	for (i = 1; i < N; i++) {
		p_state->mt[i] = (1812433253UL * (p_state->mt[i-1] ^ (p_state->mt[i-1] >> 30)) + i);
		p_state->mt[i] &= 0xffffffffUL;
	}
	p_state->mti = i;
}

static const RngType MtType = {
	"Mt_19937",			//name
	0xffffffffUL,		//RAND_MAX
	0,					//RAND_MIN
	sizeof (MtStateT),
	&MtSet,
	&MtGet,
	&MtGetDouble
};

const RngType * Rng_Mt_19937 = &MtType;

//MT19937 is the default generator, so define that here too
const RngType * RngDefault = &MtType;
uint RngDefaultSeed = 0;

void RngSet(const Rng * pR, uint seed) {(pR->P_Type->P_Set) (pR->P_State, seed);}

Rng * RngAlloc(const RngType * pT)
{
	Rng * pR = static_cast<Rng *>(malloc(sizeof(Rng)));
	/*if (pR == 0) {
		ERROR_VAL ("failed to allocate space for rng struct", ENOMEM, 0);
    };*/
	pR->P_State = malloc(pT->Size);
	if (pR->P_State == 0) {
		free (pR);		//exception in constructor, avoid memory leak
		/*ERROR_VAL ("failed to allocate space for rng state", ENOMEM, 0);*/
	};
	pR->P_Type = pT;
	RngSet (pR, RngDefaultSeed);		//seed the generator
	return pR;
}

#ifndef HIDE_INLINE_STATIC
uint RngGet(const Rng * pR) {return (pR->P_Type->P_Get) (pR->P_State);}
double RngUniform (const Rng * pR) {return (pR->P_Type->P_GetDouble) (pR->P_State);}
//
//Descr: ��������� ������������������������
//  ��������� ����� � ��������� ������
//
double RngUniformPos (const Rng * pR)
{
	double x ;
	do {x = (pR->P_Type->P_GetDouble) (pR->P_State) ;}
	while (x == 0) ;
	return x ;
}
//
//Descr: ��������� ������������������������
//  ����� ��������� �����
//
//Note: to avoid integer overflow in (range+1) we work with scale =
//  range/n = (max-min)/n rather than scale=(max-min+1)/n, this reduces
//  efficiency slightly but avoids having to check for the out of range
//  value.  Note that range is typically O(2^32) so the addition of 1
//  is negligible in most usage.
//
uint RngUniformInt (const Rng * pR, uint n)
{
	uint offset = pR->P_Type->Min;
	uint range = pR->P_Type->Max - offset;
	uint scale;
	uint k;
	/*if (n > range || n == 0) {
		ERROR_VAL ("invalid n, either 0 or exceeds maximum value of generator", EINVAL, 0) ;
	}*/
	scale = range / n;
	do {k = (((pR->P_Type->P_Get) (pR->P_State)) - offset) / scale;}
	while (k >= n);
	return k;
}
#endif

uint RngMax (const Rng * pR) {return pR->P_Type->Max;}
uint RngMin (const Rng * pR) {return pR->P_Type->Min;}
const char * RngName (const Rng * pR) {return pR->P_Type->P_Name;}
size_t RngSize (const Rng * pR) {return pR->P_Type->Size;}
void * RngState (const Rng * pR) {return pR->P_State;}
void RngFree (Rng * pR)
{
	if (pR) {
		free (pR->P_State);
		free (pR);
	}
}
//
//The initial defaults are defined in the file mt.c, so we can get
//  access to the static parts of the default generator.

const RngType * RngEnvSetup()
{
	uint seed = 0;
	const char *p = getenv("RNG_TYPE");
	if (!p) {
		RngDefault = Rng_Mt_19937;
	}
	p = getenv ("RNG_SEED");
	if (p)
	{
		seed = strtoul (p, 0, 0);
		/*fprintf (stderr, "RNG_SEED=%lu\n", seed);*/
	};
	RngDefaultSeed = seed;
	return RngDefault;
}
//�������� ������������� ��������� �����
class RandNumbGen {
public:
	double Exponential(const Rng * P_r, const double mu);
	double Gaussian(const Rng * P_r, const double sigma);
};
//
//Descr: ��������� ��������� ����� ��������������
//  �� ������������������� ������
//
double RandNumbGen::Exponential(const Rng * P_r, const double mu)
{
	double u = RngUniformPos(P_r);
	return -mu * log(u);
}
//
//Descr: ��������� ��������� ����� ��������������
//  �� ������ ������
//
double RandNumbGen::Gaussian(const Rng * P_r, const double sigma)
{
	double x, y, r2;
	do {
		//choose x,y in uniform square (-1,-1) to (+1,+1)
		x = -1 + 2 * RngUniformPos(P_r);
		y = -1 + 2 * RngUniformPos(P_r);
		//see if it is in the unit circle
		r2 = x * x + y * y;
	} while(r2 > 1.0 || r2 == 0);
	//Box-Muller transform
	return sigma * y * sqrt (-2.0 * log (r2) / r2);
}

//��������� �������� ���������� �� ����������������� �-��
int GetRandom(int min, int max)
{
	return (int)(((double)rand() / (RAND_MAX + 1)) * (max - min) + min);  //��������� ��������������� �����
}

int SLAPI GenerateGoodsBills()
{
	int    ok = -1;
	PPViewOpGrouping * p_vop = 0;
	PPViewGoodsOpAnalyze * p_vgoods = 0;
	Rng * p_rng = 0;

	int  i = 0, j = 0, m = 0;  //�������� � ������
	uint k = 0;
	double cost = 0, price = 0; //����� � ����
	double qtty = 0; //���������� ������
	long int k1 = 0;  //������������ ����� ��� ���������� ������ ���� ��������
	double k2 = 0;    //������������ ����� ��� ���������� ������ ������ ������
	ulong iter = 0; //���-�� ��������
	//��� ����������� ���������� �������
	DateRange period;
	//��� ����������� ���� � ���� ��������
	PPID  op_id = 0,  id = 0;//id ��������
	PPID  op_type = 0; //��� ��������
	PPOprKind op_kind; //��� ��������
	PPIDArray ops;  //������ ����� �������� �� ������� � ��������
	PPIDArray allow_ops;  //������ ��������� �������� �� ��������� ������
	uint opscount, allow_opscount;  //���-�� ��������� � ������� ���������
	// ��� ����������
	const RngType * p_type = 0;
	RandNumbGen generator;
	int mu = 100;     //���������� ��� ���������� �� �������������� �-��
	//��� �������� ���-�� ����������
	SString title, inp_title;
	SString msg_buf;
	double bills_count = 0;
	//��� ������� "����������� ��������"
	OpGroupingFilt op_flt;
	BillStatArray list;
	//��� ������� "�������� ����� �� ��������"
	GoodsOpAnalyzeFilt goods_flt;
	GoodsOpAnalyzeViewItem goods_item;
	PPIDArray goods_list;
	int count = 0;    //���-�� ���-��� � ������� �� ������ �� �������
	// ��� ������� ������������
	PPID acc_sheet_id = 0, acc_sheet2_id = 0; //������� ������
	PPIDArray contragent_list; //������� ������������
	long contragent_count = 0; //���������� ������������
	PPObjArticle ar_obj;
	//PPLogger logger;

	PPIniFile ini_file;
	int    enbl = 0;
	THROW_PP(ini_file.GetInt(PPINISECT_CONFIG, PPINIPARAM_ENABLEBILLGENERATOR, &enbl) && enbl, PPERR_BILLGEN_NOTALLOWED);

	//��������� ����������
	RngEnvSetup();
	p_type = RngDefault;
	p_rng = RngAlloc(p_type);

	//��������� ������� ������� (��� ����� �� ����������)
	period.SetDate(LConfig.OperDate);
	plusperiod(&period.low, PRD_ANNUAL, -1, 0);

	//�������� ���������� ����������
	PPLoadText(PPTXT_BILL_COUNT, title);
	PPLoadText(PPTXT_INP_BILL_COUNT, inp_title);
	if(InputQttyDialog(title, inp_title, &bills_count) > 0) {
		PPWait(1);
		//
		// ������ "����������� ��������"
		//
		op_flt.Flags = OpGroupingFilt::fCalcAvgLn;
		op_flt.Period.upp = period.upp;
		op_flt.Period.low = period.low;

		THROW_MEM(p_vop = new PPViewOpGrouping);
		THROW(p_vop->Init_(&op_flt));
		//
		// �������� BillStatArray �������
		//
		p_vop->CalcStat(&list);
		allow_opscount = list.getCount();
		//
		// ������ �������� �������� ��� �������
		//
		while(EnumOperations(0, &op_id, &op_kind) > 0)
			if(oneof2(op_kind.OpTypeID, PPOPT_GOODSRECEIPT, PPOPT_GOODSEXPEND))
				ops.add(op_id);
		opscount = ops.getCount();
		{
			//
			// ������������� ������� "�������� ����� �� ��������"
			//
			goods_flt.OpGrpID = GoodsOpAnalyzeFilt::ogIncoming;
			goods_flt.Period.low = period.low;
			goods_flt.Period.upp = period.upp;
			THROW_MEM(p_vgoods = new PPViewGoodsOpAnalyze);
			THROW(p_vgoods->Init_(&goods_flt));
			//
			// ����� ��������� �� ����� �������� ��� ���������
			//
			for(p_vgoods->InitIteration(PPViewGoodsOpAnalyze::OrdByIncome); p_vgoods->NextIteration(&goods_item) > 0;)
				goods_list.add(goods_item.GoodsID);
			if(goods_list.getCount()) {
				count = goods_list.getCount();
				{
					PPTransaction tra(1);
					THROW(tra);
					PPWait(1);
					//
					// ��������� �������� ����������
					//
					for(j = 0; j < bills_count; j++) {
						PPBillPacket bpack;
						//����. ����� ID �������� �� ������� ��������� �������� � ��������
						//������������� ���� �������� � ���� ����� ��� ������
						while(op_type != PPOPT_GOODSRECEIPT && op_type != PPOPT_GOODSEXPEND){
							if(allow_opscount !=0) {
								k1 = RngUniformInt(p_rng, allow_opscount);
								op_id = list.at(k1).OpID;
								op_type = GetOpType(op_id);
							}
						}
						//
						// �������� ������ ��������� � ��������� ����� ��������
						//
						THROW(bpack.CreateBlank(op_id, 0, LConfig.Location, 0));
						//
						// ����� ������� ������
						//
						GetOpCommonAccSheet(op_id, &acc_sheet_id, &acc_sheet2_id);
						if(ar_obj.P_Tbl->GetListBySheet(acc_sheet_id, &contragent_list, &contragent_count) > 0) { // ������ ������������
							bpack.Rec.Object = contragent_list.at(RngUniformInt(p_rng, contragent_count)); //������ �����������
							contragent_list.freeAll(); // ������� ������ ������������ � ����������� ������
						}
						//
						// ���������� � ������������� ���������
						//
						sprintf(bpack.Rec.Memo, "//@autogen");
						//
						// ���-�� ����� � ������������ ���������
						//
						int billrows_count = 0;
						for(k = 0; k < list.getCount(); k++) {
							BillStatFunc element = list.at(k);
							if(element.OpID == op_id){
								billrows_count = element.AvgLines;
								k = list.getCount();
							}
						}
						//
						// ���������� ���������� ������ ��������� ��������� ��������
						//
						for(i = 0; i < billrows_count; i++) {
							PPID   goods_id = 0;
							PPTransferItem ti;
							ILTI   ilti;
							int    sign = 0;
							ReceiptTbl::Rec rcpt_rec;
							GoodsRestParam rest_param;
							Transfer trnsf;
							double rest = 0;
							// @v10.6.4 MEMSZERO(rcpt_rec);
							MEMSZERO(ilti);
							//
							// ����� ID ������ � �������������� ���������� �� ������. �-��
							// � ������ ������� ������ �� ������� ID
							//
							while(rest <= 0){
								k2 = count + 1;
								while(k2 > count)
									k2 = generator.Exponential(p_rng, mu);
								goods_id = goods_list.at((uint)k2);
								rest_param.CalcMethod = GoodsRestParam::pcmDiff;
								rest_param.Date = LConfig.OperDate;
								rest_param.LocID = LConfig.Location;
								rest_param.GoodsID = goods_id;
								trnsf.GetRest(&rest_param);
								rest = rest_param.Total.Rest;
							}
							//
							// ���-�� ������
							//
							qtty = (rest > 2) ? GetRandom(1, (int)rest) : rest;
							//
							// ���� ���������� �� ���������� ����
							//
							BillObj->trfr->Rcpt.GetCurrentGoodsPrice(goods_id, LConfig.Location, GPRET_INDEF, &price, &rcpt_rec);
							cost = rcpt_rec.Cost; //���� ����������� �� ������
							//ti.Discount = GetRandom(0, 10); //������ �� ������
							//���� ���������
							sign = (op_type == PPOPT_GOODSRECEIPT) ? 1 : -1;
							//���������� �������� ������
							ilti.Setup(goods_id, sign, qtty, cost, price);
							BillObj->ConvertILTI(&ilti, &bpack, 0, 0, 0);
							THROW(bpack.InitAmounts());
						}
						//���������� ���������
						THROW(BillObj->TurnPacket(&bpack, 0));
						//logger.Log("Bill was added");

						op_id = 0;
						op_type = 0;
						//��������� ��������
						(msg_buf = "��������� �������� ����������").Transf(CTRANSF_OUTER_TO_INNER);
						PPWaitPercent(++iter, (ulong)bills_count, msg_buf);
					}
					THROW(tra.Commit());
				}
				ok = 1;
			}
		}
	}
	CATCHZOKPPERR
	ZDELETE(p_vop);
	ZDELETE(p_vgoods);
	PPWait(0);
	if(p_rng)
		RngFree(p_rng);
	//
	// �������� ��������� ���������
	//
	if(ok > 0) {
		BillFilt filt;
		filt.Period.low = filt.Period.upp = LConfig.OperDate;
		ViewGoodsBills(&filt, 1);
	}
	return ok;
}	// } turistti change @v5.3.9
//
//
//


static int _StringFuncSS(const char * pStr)
{
	SString buf;
	(buf = pStr).Cat("abcdefg-012345");
	return (buf.CmpNC("xyz") != 0);
}

static int _StringFuncSP(const char * pStr, SStringPool * pSp)
{
	uint   spp = 0;
	SString & buf = *pSp->Alloc(&spp);
	(buf = pStr).Cat("abcdefg-012345");
	int ret = (buf.CmpNC("xyz") != 0);
	pSp->Free(spp);
	return ret;
}

static int _StringFuncC(const char * pStr)
{
	char buf[512];
	strcat(strcpy(buf, pStr), "abcdefg-012345");
	return !sstreqi_ascii(buf, "xyz");
}

int TestSStringPerf()
{
	SString test_buf;
	PROFILE_START
	for(long i = 1; i < 1000000; i++) {
    	test_buf.Z().CatLongZ(i, 20);
		_StringFuncSS(test_buf);
    }
    PROFILE_END
	PROFILE_START
	for(long i = 1; i < 1000000; i++) {
    	test_buf.Z().CatLongZ(i, 20);
		_StringFuncC(test_buf);
    }
    PROFILE_END
	{
		SStringPool sp;
		PROFILE_START
		for(long i = 1; i < 1000000; i++) {
    		test_buf.Z().CatLongZ(i, 20);
			_StringFuncSP(test_buf, &sp);
		}
		PROFILE_END
	}
    return 1;
}
//
//
//
#if 0 // @v10.2.4 {

#include <fann.h>

int TestFann()
{
	const size_t input_count = 48;
	const size_t output_count = 1;
	const uint   iter_count = 5; // ��������� �������� - ������ ��������� (��� ��������)
	int    ok = 1;
#ifndef NDEBUG // {
	uint   i;
	//uint   layers[5] = {input_count, 512, 1024, 512, output_count};
	LongArray _layers;
	_layers.addzlist(input_count, 512, 1024, 512, output_count, 0);
	Fann * p_ann1 = 0;
	Fann * p_ann2 = 0;
	float result1[output_count];
	float result2[output_count];
	float pre_result1[output_count];
	float pre_result2[output_count];
	THROW(p_ann1 = fann_create_standard_array(/*SIZEOFARRAY(layers), layers*/_layers));
	p_ann1->SetTrainingAlgorithm(Fann::FANN_TRAIN_INCREMENTAL);
	p_ann1->SetLearningRate(0.06f);
	p_ann1->SetActivationFunctionHidden(Fann::FANN_SIN_SYMMETRIC/*FANN_SIGMOID_STEPWISE*/);
	//fann_randomize_weights(p_ann1, 0, +873.0f);
	THROW(p_ann2 = fann_create_standard_array(/*SIZEOFARRAY(layers), layers*/_layers));
	p_ann2->SetTrainingAlgorithm(Fann::FANN_TRAIN_INCREMENTAL);
	p_ann2->SetLearningRate(0.05f);
	p_ann2->SetActivationFunctionHidden(Fann::FANN_COS_SYMMETRIC/*FANN_GAUSSIAN*/);
	//fann_randomize_weights(p_ann2, 0, +911.0f);
    {
    	SString line_buf, input_buf;
    	StringSet ss;
    	LongArray pos_list;
    	SFile f_in("D:\\Papyrus\\Src\\PPTEST\\DATA\\nn-test-01.txt", SFile::mRead);
    	THROW_SL(f_in.IsValid());
    	while(f_in.ReadLine(line_buf)) {
			if(line_buf.Chomp().NotEmptyS()) {
				uint   ss_pos = 0;
				ss.add(line_buf, &ss_pos);
				pos_list.add((long)ss_pos);
			}
    	}
    	{
    		SFile f_out("D:\\Papyrus\\Src\\PPTEST\\nn-test-outp-01.txt", SFile::mWrite);
			pos_list.shuffle();
			memzero(result1, sizeof(result1));
			memzero(result2, sizeof(result2));
			memzero(pre_result1, sizeof(pre_result1));
			memzero(pre_result2, sizeof(pre_result2));
			float init1 = -1.0f;
			float init2 = -2.0f;
			for(uint iter = 0; iter < iter_count; iter++) {
				for(i = 0; i < pos_list.getCount(); i++) {
					const uint ss_pos = pos_list.get(i);
					ss.get(ss_pos, input_buf);
					input_buf.Trim(input_count);
					float input[input_count];
					memzero(input, sizeof(input));
					for(uint j = 0; j < input_buf.Len() && j < input_count; j++) {
						input[j] = (float)(uchar)input_buf.C(j);
					}
					{
						const float * p_temp_result = p_ann1->Run(input);
						THROW(p_temp_result);
						memcpy(pre_result1, p_temp_result, sizeof(pre_result1));
					}
					{
						const float * p_temp_result = p_ann2->Run(input);
						THROW(p_temp_result);
						memcpy(pre_result2, p_temp_result, sizeof(pre_result2));
					}
					if(iter < (iter_count-1)) {
						p_ann1->TrainWithOutput(input, pre_result2, result1);
						p_ann2->TrainWithOutput(input, pre_result1, result2);
						line_buf.Z().Cat(iter).CatChar('/').Cat(i).Tab().
							Cat(result1[0], MKSFMTD(0, 10, 0)).Tab().
							Cat(result2[0], MKSFMTD(0, 10, 0)).Tab().
							Cat(result2[0]-result1[0], MKSFMTD(0, 10, 0)).Tab().
							Cat(input_buf).CR();
					}
					else {
						line_buf.Z().Cat(iter).CatChar('/').Cat(i).Tab().
							Cat(pre_result1[0], MKSFMTD(0, 10, 0)).Tab().
							Cat(pre_result2[0], MKSFMTD(0, 10, 0)).Tab().
							Tab().
							Cat(input_buf).CR();
					}
					f_out.WriteLine(line_buf);
				}
			}
    	}
	}
    CATCH
		ok = 0;
    ENDCATCH
	fann_destroy(p_ann1);
	fann_destroy(p_ann2);
#endif // } NDEBUG
    return ok;
}
#endif // } @v10.2.4