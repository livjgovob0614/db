; PPWS.NSI
; Copyright (c) A.Sobolev 2005, 2006, 2007, 2008
; ������ �������� ����������� ������� ����� ������� Papyrus
; *******************************************************************************
; Definitions {
!define PRODUCT_PUBLISHER "A.Fokin, A.Sobolev"
!define PRODUCT_WEB_SITE "http://www.petroglif.ru"
!define PRODUCT_VERSION "5.7.5" ; @debug
!define SRC_ROOT   "j:\papyrus" ; @debug
!define SRC_REDIST   "${SRC_ROOT}\src\redist"
!define SRC_TARGET   "${SRC_ROOT}\PPY\BIN"
!define PRODUCT_NAME "Papyrus Job Server"

!define PRODUCT_UNINST_ROOT_KEY "HKLM"
; HKEY_CLASSES_ROOT		= 0x80000000
; HKEY_CURRENT_USER		= 0x80000001
; HKEY_LOCAL_MACHINE	= 0x80000002
; HKEY_USERS			= 0x80000003
; HKEY_CURRENT_CONFIG	= 0x80000004
!define PRODUCT_UNINST_ROOT_KEY_ID 0x80000002 ; for "HKLM"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

!define SERVICE_ALL_ACCESS    0xF01FF
!define SC_MANAGER_ALL_ACCESS 0x3F
!define SERVICE_CONTROL_STOP  1
!define FALSEPASSW            "FaLsEpAsSwOrD"
!define SERVICE_NAME          "PAPYRUS_SERVICE"
!define UNINSTALLER			"uninst-jobserver.exe"

Var hwnd
Var hSCManager
Var hPPWS
Var FuncName

Var InstTaskServ
Var AccountName
Var Password
Var Err

Var InstallType
Var TargetServer
; WMI access
Var user
Var pasw

SetCompressor bzip2
; @v5.7.0 {
Caption "Papyrus JobServer Setup"
BGGradient 007070 00e0f0 FFFFFF
InstallColors FF8080 000030
XPStyle on
BrandingText " "
; } @v5.7.0

;MUI 1.67 compatible ------
!include "MUI2.nsh"
!include "Library.nsh"
!include "FileFunc.nsh"

;MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${SRC_ROOT}\SRC\RSRC\ICO\P2.ICO"
!define MUI_UNICON "${SRC_ROOT}\SRC\RSRC\ICO\P2.ICO"
!define MUI_FINISHPAGE_NOAUTOCLOSE
; } Definitions
; *******************************************************************************
; Functions {
Function .onInit
	InitPluginsDir
	File /oname=$PLUGINSDIR\ppws.ini "ppws.ini"
FunctionEnd
; �������� �����/������ ��� ������� � ������� (�����������)
Function AskAccountSettings
	StrCpy $0 "--------------------------------------------------------------------------------"
	StrCpy $2 $0
	System::Call 'wmi_connection::AuthBox(t r0r1, t r2r3, i 80) i.r4'
	IntCmp $4 0 UNDEF_ACCOUNT_SETTINGS
	StrCpy $user $1
	StrCpy $pasw $3
	GoTo DONE
UNDEF_ACCOUNT_SETTINGS:
	StrCpy $user ""
	StrCpy $pasw ""
DONE:
FunctionEnd
; ���� $INSTDIR �������� remote ���� �� ���������� ��� ������� � ���������� $InstallType="remote"
; ����� ���������� $InstallType="local"
Function DetermineInstallType
	StrCpy $0 $INSTDIR 2
	StrCmp $0 "\\" REMOTE 
	StrCpy $InstallType "local"
	GoTo DONE
REMOTE:
	StrCpy $InstallType "remote"
	; ������ ���� ���������� ��� �������
	StrCpy $0 $INSTDIR
	System::Call 'wmi_connection::GetServerName(t r0, t .r1, i 64) i r0'
	StrCpy $TargetServer $1
	call AskAccountSettings
DONE:
FunctionEnd
; ������ ������ ������������� ������
Function SelectVersion
	System::Call 'versel::SelectVersion(i 0, t r0r2, i 0) i.r1'
	StrCmp $2 "" SET_DEFAULT_PATH
	StrCpy $INSTDIR "$2\bin"
	GoTo DONE
SET_DEFAULT_PATH:
	StrCpy $INSTDIR "C:\papyrus\ppy\bin"
DONE:
FunctionEnd
;
!insertmacro GetDrives
Function GetDiskName
	StrCpy $ERR "ERRORPATH"
	StrCpy $1 "$INSTDIR" 3
	StrCmp $1 $9 +1 DONE
	StrCpy $0 "StopGetDrives"
	StrCpy $ERR ""
DONE:
	Push $0
FunctionEnd
;
Function CheckPath
	${GetDrives} "HDD" "GetDiskName"
	call DetermineInstallType
	StrCmp $Err "ERRORPATH" +1 DONE
DONE:
FunctionEnd
;
Function CheckPPWS
	StrCmp $InstallType "remote" REMOTE ; ������� �� ��������� ������� ���������� �������
	StrCpy $hSCManager 0
	;
	; ��������� ���������� � ���������� ��������� �����
	;
	DetailPrint "��������� ���������� � ���������� ��������� �����"
	StrCpy $FuncName "OpenSCManagerA"
	StrCpy $0 "false"
	System::Call 'advapi32::OpenSCManagerA(n, n, i ${SC_MANAGER_ALL_ACCESS})i.r4'
	StrCpy $hSCManager $4
	IntCmp $hSCManager 0 ERROR
	;
	; ���� ������ ����� ����������, ������ ��������� �� ����,
	;
	StrCpy $hPPWS 0
	DetailPrint "�������� ����� ��������� ������� �����"
	StrCpy $FuncName OpenServiceA
	System::Call 'advapi32::OpenServiceA(i $hSCManager, t ${SERVICE_NAME}, i ${SERVICE_ALL_ACCESS})i.r5'
	StrCpy $hPPWS $5
	IntCmp $hPPWS 0 DONE
	;
	; ������ ������� �������� ����
	;
	WriteINIStr "$PLUGINSDIR\ppws.ini" "Field 1" "text" "���������� ������� �����"
	WriteINIStr "$PLUGINSDIR\ppws.ini" "Field 2" "text" "�������� ������ �����"
	;
	; � ������� ���� � login'��
	;
	ReadRegStr $AccountName HKLM "SYSTEM\CurrentControlSet\Services\${SERVICE_NAME}" "ObjectName"
	WriteINIStr "$PLUGINSDIR\ppws.ini" "Field 3" "state" $AccountName
	WriteINIStr "$PLUGINSDIR\ppws.ini" "Field 4" "state" ${FALSEPASSW}
	WriteINIStr "$PLUGINSDIR\ppws.ini" "Field 5" "state" ${FALSEPASSW}
	GoTo DONE
REMOTE:
	StrCpy $0 $TargetServer
	StrCpy $1 $user
	StrCpy $2 $pasw
	System::Call 'wmi_connection::Connect(t r0, t r1, t r2, t NULL) i .r3'
	IntCmp $3 0 CONNECTION_ERROR
	StrCpy $0 ${SERVICE_NAME}
	;System::Call 'wmi_connection::WinSvcDelete(t r0) v'
	System::Call 'wmi_connection::WinSvcExists(t r0, i 1) i .r1'
	IntCmp $1 0 WMI_DISCONNECT
	DetailPrint "����������, ��� ������ �� ��������� ������ ��� ����������, ���������� ���������� ��� �������"
	StrCpy $1 "StartName"
	System::Call 'wmi_connection::WinSvcGetProperty(t r0, t r1, i 1, t .r2, i 64) t r0'
	StrCmp $2 "LocalSystem" FORCE_EMPTY_NAME
	WriteINIStr "$PLUGINSDIR\ppws.ini" "Field 3" "state" $2
FORCE_EMPTY_NAME:
	WriteINIStr "$PLUGINSDIR\ppws.ini" "Field 3" "state" ""
	WriteINIStr "$PLUGINSDIR\ppws.ini" "Field 4" "state" ${FALSEPASSW}
	WriteINIStr "$PLUGINSDIR\ppws.ini" "Field 5" "state" ${FALSEPASSW}
	GoTo WMI_DISCONNECT
CONNECTION_ERROR:
	DetailPrint "�� ������� ����������� �� ������� WMI �� $TargetServer"
	Abort
ERROR:
	StrCpy $Err ERROR
	DetailPrint "������ ��������� ������� �����. ������� - $FuncName"
	Abort
WMI_DISCONNECT:
	System::Call 'wmi_connection::Release(v) v'
DONE:
FunctionEnd
;
Function ReloadPPWSWindow
	;
	; ���������� ����� ��������� ������� �����
	;
	GetDlgItem $1 $hwnd 1202 ; PathRequest control (1200 + field 3 - 1)
	EnableWindow $1 0
	GetDlgItem $1 $hwnd 1203 ; PathRequest control (1200 + field 4 - 1)
	EnableWindow $1 0
	GetDlgItem $1 $hwnd 1204 ; PathRequest control (1200 + field 5 - 1)
	EnableWindow $1 0
	StrCpy $InstTaskServ ""
	StrCpy $Password ""
	StrCpy $AccountName ""
	StrCpy $Err GOOD

	ReadINIStr $0 "$PLUGINSDIR\ppws.ini" "Field 2" "State"
	IntCmp $0 0 DONE
	StrCpy $InstTaskServ 1
	GetDlgItem $1 $hwnd 1202 ; PathRequest control (1200 + field 3 - 1)
	EnableWindow $1 1
	GetDlgItem $1 $hwnd 1203 ; PathRequest control (1200 + field 4 - 1)
	EnableWindow $1 1
	GetDlgItem $1 $hwnd 1204 ; PathRequest control (1200 + field 5 - 1)
	EnableWindow $1 1
DONE:
FunctionEnd
;
Function SetCustom
	;Display the InstallOptions dialog
	; In this mode InstallOptions returns the window handle so we can use it
	call CheckPPWS
	InstallOptions::initDialog /NOUNLOAD "$PLUGINSDIR\ppws.ini"
	Pop $hwnd
	call ReloadPPWSWindow
	InstallOptions::show
	Pop $0
FunctionEnd
;
Function ValidateCustom
CHECK_PRESS:
	;
	; ������������ ����� �� ������
	; ���� �������� �� �����
	;
	ReadINIStr $0 "$PLUGINSDIR\ppws.ini" "Settings" "State"
	IntCmp $0 0 NEXTBUTTON	;������ Next?
	;
	; ����� Reload ��������
	;
	call ReloadPPWSWindow
	Abort ; Return to the page
	;
	; ������������ ����� �� ������ �����
	;
NEXTBUTTON:
	;
	; ������� ������������ ���������� ����� � ������ � �������, ���� ����,
	; ���� �� ������� ��������� ������� �����, �� ��� ������
	;
	StrCmp $InstTaskServ "" DONE
	ReadINIStr $AccountName "$PLUGINSDIR\ppws.ini" "Field 3" "State"
	;StrCmp $AccountName "" ERRORNAME
	ReadINIStr $Password "$PLUGINSDIR\ppws.ini" "Field 4" "State"
	ReadINIStr $1 "$PLUGINSDIR\ppws.ini" "Field 5" "State"
	StrCmp $Password $1 DONE
	MessageBox MB_ICONEXCLAMATION|MB_OK "����������� ������ ������!"
	Abort ; Return to the page
ERRORNAME:
	MessageBox MB_ICONEXCLAMATION|MB_OK "����������� ������� ������� ������!"
	Abort ; Return to the page
DONE:
	ReadINIStr $AccountName "$PLUGINSDIR\ppws.ini" "Field 3" "State"
FunctionEnd
;
Function un.onUninstSuccess
	HideWindow
	MessageBox MB_ICONINFORMATION|MB_OK "�������� ��������� $(^Name) ���� ������� ���������."
FunctionEnd
;
Function un.onInit
	MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "�� ������� � ���, ��� ������� ������� $(^Name) � ��� ���������� ���������?" IDYES +2
	Abort
FunctionEnd
; } Functions
; *******************************************************************************
; Pages {
; welcome
!define MUI_WELCOMEPAGE_TITLE "��� ������������ ������ ��������� ${PRODUCT_NAME}"
!define MUI_WELCOMEFINISHPAGE_BITMAP   "${SRC_ROOT}\Src\Rsrc\Bitmap\nsis-welcome-02.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${SRC_ROOT}\Src\Rsrc\Bitmap\nsis-welcome-02.bmp"
!insertmacro MUI_PAGE_WELCOME
; ����� � ��������� ���� ���������
PageEx directory
	PageCallbacks SelectVersion "" CheckPath
PageExEnd
; ��������� ���������
Page custom SetCustom ValidateCustom ": ������ �����"
; } Pages
; *******************************************************************************
; Sections {
;Start menu page
var ICONS_GROUP
;!define MUI_STARTMENUPAGE_NODISABLE
;!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Papyrus"
;!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
;!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
;!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"

;Instfiles page
!insertmacro MUI_PAGE_INSTFILES

;Finish page
!define MUI_FINISHPAGE_TITLE "���������� ������ ������� ��������� ${PRODUCT_NAME}"
!insertmacro MUI_PAGE_FINISH

;Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

;Language files
!insertmacro MUI_LANGUAGE "Russian"
;Reserve files
; @v5.7.0 !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile PpyJobSrvr_${PRODUCT_VERSION}.exe

InstallDir "C:\PPY\BIN"
;InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "����� ����������" SEC01
	StrCmp $InstallType "remote" REMOTE
	; ��������� ������
	DetailPrint "��������� ������� �����"
	ExecWait '"$INSTDIR\ppws.exe" servicestop' $0
	IntCmp $0 0 +2
	DetailPrint "! ������ ��������� �������"
	; �������� exe ���� ������� �����
	SetOutPath "$INSTDIR"
	SetOverwrite on
	File "${SRC_TARGET}\ppws.exe"
	;DetailPrint "��-����������� ������� �����"
	;ExecWait '"$INSTDIR\ppws.exe" uninstall'
	; ���������� ���������
	StrCpy $2 ppws
	ReadINIStr $R2 "$PLUGINSDIR\ppws.ini" "Field 3" "State"
	ReadINIStr $R3 "$PLUGINSDIR\ppws.ini" "Field 4" "State"
	; ����������� �������
	DetailPrint "����������� c������ �����"
	ExecWait '"$INSTDIR\ppws.exe" install $R2 $R3' $0
	IntCmp $0 0 +2
	DetailPrint "! ������ ����������� �������"
	; ������ �������
	DetailPrint "������ ������� �����"
	ExecWait '"$INSTDIR\ppws.exe" servicestart' $0
	IntCmp $0 0 +2
	DetailPrint "! ������ ������� �������"
	GoTo DONE
REMOTE:
	StrCpy $0 $TargetServer
	StrCpy $1 $user
	StrCpy $2 $pasw
	DetailPrint "���������� � $1@$0 ..."
	System::Call "wmi_connection::Connect(t r0, t r1, t r2, t NULL) i .r3"
	IntCmp $3 0 CONNECTION_ERROR
	System::Call "wmi_connection::GetLastMsg(t .r2, i 64) v"
	DetailPrint "$2"
	;
	StrCpy $0 $INSTDIR
	System::Call "wmi_connection::GetLocalFromUnc(t r0, t .r1, 256) i r0"
	StrCpy $0 "$1\ppws.exe"
	StrCpy $1 "$0 servicestop"
	DetailPrint "������� ��������� ������� �� ��������� ������..."
	System::Call "wmi_connection::WinProcessCreate(t r1, t '') i r2"
	;System::Call "wmi_connection::WinSvcStop(t r0) v" ; ���������� ��������� ������
	System::Call "wmi_connection::GetLastMsg(t .r1, i 128) v"
	DetailPrint "$1"
	; �������� exe ���� ������� �����
	SetOutPath "$INSTDIR"
	SetOverwrite on
	File "${SRC_TARGET}\ppws.exe"
	; ����������� �������
	ReadINIStr $R2 "$PLUGINSDIR\ppws.ini" "Field 3" "State"
	ReadINIStr $R3 "$PLUGINSDIR\ppws.ini" "Field 4" "State"
	StrCpy $1 "$0 install"
	StrCmp $R2 "" REG_SVC
	StrCpy $2 "$1 $R2"
	StrCpy $1 $2
	StrCmp $R3 "" REG_SVC
	StrCpy $3 "$1 $R3"
	StrCpy $1 $3
REG_SVC:
	DetailPrint "��������� ����������� c������..."
	System::Call "wmi_connection::WinProcessCreate(t r1, t '') i .r2"
	;System::Call "wmi_connection::WinSvcCreate(t r0, t 'Papyrus Job Server', t r2, i 16, i 1, i 1, t r12, t r13, t NULL, t NULL, t NULL) v"	
	System::Call "wmi_connection::GetLastMsg(t .r1, 128) v"
	DetailPrint "$1"
	DetailPrint "��������� ������ �������..."
	StrCpy $1 "$0 service"
	System::Call "wmi_connection::WinProcessCreate(t r1, t '') v"
	System::Call "wmi_connection::GetLastMsg(t .r1, 128) v"
	DetailPrint "$1"
	GoTo DONE
WMI_DISCONNECT:
	System::Call "wmi_connection::Release(v) v"
	GoTo DONE
CONNECTION_ERROR:
	DetailPrint "�� ������� ����������� �� ������� WMI �� $TargetServer"
	Abort
DONE:
SectionEnd
;
Section -Post
	StrCmp $Err ERROR DONE
	;
	; ���������� ����������� � ����������
	;
	WriteUninstaller "$INSTDIR\${UNINSTALLER}"
	StrCmp $InstallType "remote" REMOTE
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\${UNINSTALLER}"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\ppws.exe"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
	GoTo DONE
REMOTE:
	StrCpy $0 $TargetServer
	StrCpy $1 $user
	StrCpy $2 $pasw
	DetailPrint "���������� � $1@$0 ..."
	System::Call "wmi_connection::Connect(t r0, t r1, t r2, t NULL) i .r3"
	IntCmp $3 0 CONNECTION_ERROR
	System::Call "wmi_connection::GetLastMsg(t .r1, 128) i .r3"
	DetailPrint "$1"
	;
	DetailPrint "������ �������� � ������ �� �������� ������..."
	;
	StrCpy $0 "${PRODUCT_UNINST_KEY}"
	StrCpy $1 "UninstallString"
	StrCpy $2 "$INSTDIR\${UNINSTALLER}"
	System::Call "wmi_connection::GetLocalFromUnc(t r2, t .r3, i 256) i .r3"
	System::Call "wmi_connection::WinRegAddStr(i ${PRODUCT_UNINST_ROOT_KEY_ID}, t r0, t r1, t r3) v"
	;
	StrCpy $1 "$INSTDIR\ppws.exe"
	System::Call "wmi_connection::GetLocalFromUnc(t r1, t .r2, 256)"
	;
	StrCpy $1 "DisplayIcon"
	System::Call "wmi_connection::WinRegAddStr(i ${PRODUCT_UNINST_ROOT_KEY_ID}, t r0, t r1, t r2) v"
	;
	StrCpy $1 "DisplayVersion"
	StrCpy $2 "${PRODUCT_VERSION}"
	System::Call "wmi_connection::WinRegAddStr(i ${PRODUCT_UNINST_ROOT_KEY_ID}, t r0, t r1, t r2) v"
	;
	StrCpy $1 "URLInfoAbout"
	StrCpy $2 "${PRODUCT_WEB_SITE}"
	System::Call "wmi_connection::WinRegAddStr(i ${PRODUCT_UNINST_ROOT_KEY_ID}, t r0, t r1, t r2) v"
	;
	StrCpy $1 "Publisher"
	StrCpy $2 "${PRODUCT_PUBLISHER}"
	System::Call "wmi_connection::WinRegAddStr(i ${PRODUCT_UNINST_ROOT_KEY_ID}, t r0, t r1, t r2) v"
	;
	System::Call "wmi_connection::GetLastMsg(t .r1, 128)"
	DetailPrint "$1"
WMI_DISCONNECT:
	System::Call "wmi_connection::Release(v) v"
	GoTo DONE
CONNECTION_ERROR:
	DetailPrint "�� ������� ����������� �� ������� WMI �� $TargetServer"
	Abort
DONE:
SectionEnd
;
Section Uninstall
	Delete "$INSTDIR\${UNINSTALLER}"
	DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
	StrCpy $hSCManager 0

	DetailPrint "��������� ������� �����"
	ExecWait '"$INSTDIR\ppws.exe" stopservice'
	DetailPrint "��-����������� ������� �����"
	ExecWait '"$INSTDIR\ppws.exe" uninstall'
	;
	; ����� exe ����
	;
	SetOutPath "$INSTDIR"
	ClearErrors
	Delete "$INSTDIR\ppws.exe"
	ifErrors  -2
	GoTo DONE
ERROR:
	DetailPrint "������ �������� ������� �����"
DONE:
SectionEnd
; } Sections
