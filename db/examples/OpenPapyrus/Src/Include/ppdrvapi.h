// PPDRVAPI.H
// Copyright (c) A.Sobolev 2013, 2019
//
#include <slib.h>

#define EXPORTPROC extern "C" __declspec (dllexport)

class PPBaseDriver;

typedef PPBaseDriver * (*PPDrv_CreateInstanceProc)();

class PPDrvThreadLocalArea {
public:
	friend class PPDrvSession;
	SLAPI  PPDrvThreadLocalArea();
	SLAPI ~PPDrvThreadLocalArea();
	int    SLAPI IsConsistent() const;
	long   SLAPI GetId() const;
	ThreadID SLAPI GetThreadID() const;
private:
	int    ResetFinishEvent();
	int    SignalFinishEvent();

	int    Sign;           // ���� Sign == SIGN_PPDRVTLA, �� ������ ������ �������� �������� (� ���������, �� �������� ������������)
	long   Id;             // @id
	ThreadID TId;          // ������������� ������
	Evnt * P_FinishEvnt;
public:
	PPBaseDriver * I;
	int    LastErr;        // Last error code //
	int    State;          //
	SString AddedMsgString;
};
//
// Descr: ��������� �����, ����������� ������ ������ ������� ����������.
//   ������ ������: KEY1=VAL1; KEY2=VAL2
//   ������� �� �������.
//
class PPDrvInputParamBlock {
public:
	PPDrvInputParamBlock(const char * pInputText);
	int    Get(const char * pParam, SString & rValue);
private:
	StringSet Ss;
	SString TempBuf;
	SString KeyBuf;
	SString ValBuf;
};

class PPDrvSession {
public:
	/* @v10.4.5 The SIntToSymbTabEntry should be used from now on.
	struct TextTableEntry {
		int    Code;
		const char * P_Text;
	};*/
	int    ImplementDllMain(HANDLE hModule, DWORD dwReason);

	PPDrvSession(const char * pName, PPDrv_CreateInstanceProc proc, uint verMajor, uint verMinor, uint errMsgCount, const SIntToSymbTabEntry * pErrMsgTab);
	~PPDrvSession();
	int    SLAPI Init(HINSTANCE hInst);
	int    SLAPI InitThread();
	int    SLAPI ReleaseThread();
	PPDrvThreadLocalArea & SLAPI GetTLA();
	const PPDrvThreadLocalArea & SLAPI GetConstTLA() const;
	PPBaseDriver * GetI();
	//
	// Descr: ������ ������� � ������������� ����� ��������� �� ������ � ������.
	//   �������� ������� ������ ��������������� ��������� TextTableEntry.
	//
	void   DefineErrTextTable(uint numEntries, const SIntToSymbTabEntry * pTab);
	int    GetLastErr();
	int    GetErrText(int errCode, SString & rBuf);
	void   SetErrCode(int errCode);
	void   SetLogFileName(const char * pFileName);

	enum {
		logfTime     = 0x0001,
		logfName     = 0x0002,
		logfThreadId = 0x0004
	};
	int    Log(const char * pMsg, long flags);
	int    Helper_ProcessCommand(const char * pCmd, const char * pInputData, char * pOutputData, size_t outSize);
private:
	void   DefineErrText(int errCode, const char * pText);
	int    CreateI();

	const  SString Name;
	const  uint VerMajor;
	const  uint VerMinor;
	const  PPDrv_CreateInstanceProc Proc;
	long   TlsIdx;         // �� ��������� ������� ������    //
	long   Id;
	ACount LastThread;
	ACount DllRef;         // ������� �������� �������� ��� DLL-�������
	SCriticalSection::Data Cs_Log; // ����������� ������ ������ ������ � Log � ���������� ���������� LogFileName
	SString LogFileName;
	StrAssocArray ErrText;
};
//
//
//
class PPBaseDriver {
public:
	enum {
		serrNotInited  = 1000,
		serrInvCommand,
		serrNoMem,
		serrNotEnoughRetBufLen
	};
	PPBaseDriver();
	virtual ~PPBaseDriver();
	//
	// Descr: ������������ ��� �������, ����� INIT � RELEASE. ����� ������� ��������� � ProcessCommand
	//   � ������� ��������.
	//
	virtual  int  ProcessCommand(const SString & rCmd, const char * pInputData, SString & rOutput);
private:
	ACount RefCount;
	long   Capability;
	SString LogFileName;
};
//
// Descr: ������, ������� ������ �������������� ��� ������������� �������� ����������.
// Sample:
//   static PPDrvSession::TextTableEntry _SampleDrvErrTab[] = { { 1, "error text 1" }, { 2, "error text 2" } };
//   class PPDrvSample : public PPBaseDriver { };
//   PPDRV_INSTANCE_ERRTAB(Sample, 1, 0, PPDrvSample, _SampleDrvErrTab);
//
#define PPDRV_INSTANCE_ERRTAB(name, major_ver, minor_ver, cls, errtab) \
	static PPBaseDriver * CreateInstance_##name() { return new cls; } \
	PPDrvSession DRVS(#name, CreateInstance_##name, major_ver, minor_ver, SIZEOFARRAY(errtab), errtab); \
	BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved) { return DRVS.ImplementDllMain(hModule, dwReason); } \
	extern "C" __declspec (dllexport) int RunCommand(const char * pCmd, const char * pInputData, char * pOutputData, size_t outSize) \
		{ return DRVS.Helper_ProcessCommand(pCmd, pInputData, pOutputData, outSize); }

