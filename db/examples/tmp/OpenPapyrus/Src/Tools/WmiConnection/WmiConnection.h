// WmiConnection.h
// Copyright (c) A.Kurilov 2008
//
#ifndef WMI_CONNECTION_H
#define WMI_CONNECTION_H
#ifndef _WIN32_DCOM
	#define _WIN32_DCOM
#endif
#define EXPORT extern "C" __declspec(dllexport)
// SString
#include <slib.h>
// �������� �������� Win API ��� ������ � WMI
#include <wbemidl.h>
#include <comdef.h>
const size_t MAXLEN = 0x100;
//
// ���� {
//
// ����������� ����� ������� windows
enum RegHKey
{
	CLASSES_ROOT	= 0x80000000,
	CURRENT_USER	= 0x80000001,
	LOCAL_MACHINE	= 0x80000002,
	USERS			= 0x80000003,
	CURRENT_CONFIG	= 0x80000005,
};
// ���� �������� windows
enum ServiceType
{
	SVCTYPE_KERNEL		= 0x1,	// kernel driver
	SVCTYPE_FILESYS		= 0x2,	// file system driver
	SVCTYPE_ADAPTER		= 0x4,	// adapter
	SVCTYPE_RECOGNIZER	= 0x8,	// recognizer driver
	SVCTYPE_OWN			= 0x10,	// own process
	SVCTYPE_SHARE		= 0x20	// share process
};
// ��� �����, �������������  ��������� � ������ ������ ������ ������� windows
enum ErrorControl
{
	ERRCTL_SILENT		= 0,	// silence
	ERRCTL_NOTIFY		= 1,	// notify user
	ERRCTL_LASTGOOD		= 2,	// restart system with last-known-good configuration
	ERRCTL_ATTEMPTGOOD	= 3		// attempt to start system with a good configuration
};
//
// } ����
//
// ������� {
//
BOOL APIENTRY DllMain(HANDLE, DWORD, LPVOID);
// ������ ��� ����� ����� ������������ � �����
EXPORT int LoginPasswordBox(char *pUserNameBuff, char *pPassWordBuff, size_t maxLen = MAXLEN);
// ���������� ��������� �� ��������� ��������������� ��������� �����
EXPORT void GetLastMsg(char *buff, size_t maxLen = MAXLEN);
// ���� ������� �������� �������� UNC-����� ���������� 1, ����� 0
EXPORT int IsUncPath(const char *pPath);
// ��������� ��� ������� �� UNC-������ ���� "\\xxxx\yyyyy\zzzzzz"
EXPORT size_t GetServerName(const char *pUncPath, char *pBuff, size_t maxLen = MAXLEN);
// ��������� WQL ������ ��� ��������� �������� ��������
EXPORT int GetPropertyValue(const char *pQuery, const char *pProperty, char *pBuff, size_t maxLen = MAXLEN);
// ������������ ��������� ���� �� ��������� �������, ��������������� ���������� �� ����� UNC-����
// ���� �������� UNC-���� ��-UNC ��� ��������� ������ ������ 0, � ��������� � ������ �� ���������
EXPORT int GetLocalFromUnc(const char *pUncPath, char *pBuff, size_t maxLen = MAXLEN);
// ���������� � �������� WMI ��� ���������� ����-������
// ���������� 1 ���� �������, 0 - �����
// ������� "Release()" ���� ���������� ���� ������� �����������
EXPORT int Connect (
	// ��������� � localhost ���� ������ �� ������ ��� ������="."
	const char *pServer		= NULL,
	// ��������� ��������: "domain\\user", "user@domain", ".\\user" ��� "user" ��� ������ ������
	// � ��������� ������ �������� �����������, ��������� ������� ��������� �������
	const char *pDomUserName= NULL,
	const char *pUserPasswd	= NULL,
	const char *pNameSpace	= NULL
);
// ������� ����� ������ windos
EXPORT int WinSvcCreate (
	const char *pName,						// max 256 chars, case-sensetive and no [back]slashes
	const char *pDisplayName,				// max 256 chars, case-sensetive and no [back]slashes
	const char *pPathName,					// ���� � ������������ ����� (����� ��� �������, min 14 ��������)
	ServiceType serviceType		= SVCTYPE_OWN,
	ErrorControl errorControl	= ERRCTL_NOTIFY,
	int desktopInteract			= 1,		// does service can interact with desktop?
	const char *pStartName		= NULL,		// windows-������� ��� ������� (min 14 ��������)
	const char *pStartPassword	= NULL,		// ������ ��� ����� ��� ������� (min 14 ��������)
	const char *pLoadOrderGroup	= NULL,		// ��������� ��� ��� ����� �������
	const char *pOrderGrpDeps	= NULL,		// ������ ����� ��������, �� ������� ����� �������� ����
	const char *pServiceDeps	= NULL,		// ������ ��������, �� ������� ����� �������� ����
	int expandIfTooShort		= 1
);
// ������� ������ �� ��� �����
EXPORT int WinSvcDelete(const char *pName, int expandIfTooShort = 1);
// ��������� ������ �� ��� �����
EXPORT int WinSvcStart(const char *pName, int expandIfTooShort = 1);
// ���������� ������ �� ��� �����
EXPORT int WinSvcStop(const char *pName, int expandIfTooShort = 1);
// ��������� ���������� �� ��� ������ � ����� �� ������, ����������� 1 ���� ��, 0 - �����
EXPORT int WinSvcExists(const char *pName, int expandIfTooShort = 1);
// �������� ����������� �������� �������� ������������ �������
EXPORT int WinSvcGetProperty(const char *pName, const char *pProperty, int expandIfTooShort, char* pBuff, size_t maxLen = MAXLEN);
// �������� �������� ���������� ����� �������
EXPORT int WinRegGetStr(long targetBranch, const char *pKeyName, const char *pValueName, int expandIfTooShort, char* pBuff, size_t maxLen = MAXLEN);
// ������� �������� ���������� ����� �������
EXPORT int WinRegAddStr(long targetBranch, const char *pKeyName, const char *pValueName, const char *pKeyValue, int expandIfTooShort = 1);
// ������� ���� �������
EXPORT int WinRegDel(long targetBranch, const char *pKeyName, int expandIfTooShort = 1);
// �������� � ������ ������ �������� windos �� ����������� ��������� ������
EXPORT int WinProcessCreate(const char *pCmdLine, const char *pCurrDir = NULL);
// �� �������� ���������� �������
EXPORT void Release();
//
// } ������
//
#endif // WMI_CONNECTION_H
