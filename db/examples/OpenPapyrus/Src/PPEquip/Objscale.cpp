// OBJSCALE.CPP
// Copyright (c) A.Sobolev 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019
//
#include <pp.h>
#pragma hdrstop
#include <ipexport.h>
#include <charry.h>
//
//
//
static const SVerT __MinPrepFileVer(7, 1, 11);
//
// @todo �������� ������������� SplitString �� SString::Wrap
//
// Descr: ������� SplitStr ��������� ������ pStr �� count ��������, � ������������
//   � ���������� pItem. ������ ��������� ����� ����� �� ����� pItem->len � ������������ //
//   �� ��������� pItem->ptr. �� �����������, SplitStr �������� ��������� ����� �����
//   ������� (�� ��������). ���������� ������� ������ ��������� ���������� pItem->ptr
//   ����������� �� ������� ������� ������.
// Return:
//   1
//
static void FASTCALL SplitString(const char * pStr, int count, SBaseBuffer * pItems)
{
	size_t src_pos = 0;
	for(int i = 0; i < count; i++) {
		size_t dest_pos = 0;
		if(pStr[src_pos] != 0) {
			int    src_space_pos = -1;
			int    dst_space_pos = -1;
			size_t l = pItems[i].Size;
			size_t j = 0;
			while(pStr[src_pos] != 0 && j < l) {
				if(pStr[src_pos] == ' ') {
					src_space_pos = (int)src_pos;
					dst_space_pos = (int)dest_pos;
				}
				pItems[i].P_Buf[dest_pos] = pStr[src_pos];
				dest_pos++;
				src_pos++;
				j++;
			}
			if(pStr[src_pos] && src_space_pos >= 0) {
				src_pos  = src_space_pos+1;
				dest_pos = dst_space_pos;
			}
		}
		pItems[i].P_Buf[dest_pos] = 0;
	}
}
//
//
//
PPScale2::PPScale2()
{
	THISZERO();
}

int SLAPI PPScale::IsValidBcPrefix() const
{
	return BIN((BcPrefix >= 200 && BcPrefix <= 299) || (BcPrefix >= 20 && BcPrefix <= 29));
}

struct ScalePLU { // @transient
	ScalePLU() : GoodsID(0), GoodsNo(0), GrpCode(0), Barcode(0), Flags(0), Price(0.0), Expiry(ZERODATE)
	{
	}
	int    HasAddedMsg() const
	{
		return BIN(AddMsgBuf.NotEmpty());
	}
	enum {
		fCountable = 0x0001 // ������� �����
	};
	PPID   GoodsID;
	long   GoodsNo;
	long   GrpCode;
	long   Barcode;
	long   Flags;
	double Price;
	LDATE  Expiry;
	SString GoodsName;
	SString AddMsgBuf;
};

// static
int SLAPI PPObjScale::EncodeIP(const char * pIP, char * pEncodedIP, size_t ipSize)
{
	int    ok = 1;
	char   ip[16], encoded_ip[8];
	uint   k = 0, i = 0;

	memzero(ip, sizeof(ip));
	memset(encoded_ip, 32, sizeof(encoded_ip));
	encoded_ip[7] = '\0';
	strnzcpy(ip, pIP, ipSize);
	StringSet ss_ip_digit('.', ip);
	for(i = 0; i < ss_ip_digit.getCount(); i++) {
		char   str_dig[16];
		int    dig = 0;
		size_t str_dig_len = 0;
		memzero(str_dig, sizeof(str_dig));
		ss_ip_digit.get(&k, str_dig, sizeof(str_dig));
		str_dig_len = sstrlen(str_dig);
		THROW_PP(str_dig_len < 4 && str_dig_len > 0, PPERR_SCALE_INVIP);
		THROW_PP((dig = atoi(str_dig)) >= 0 && dig < 256, PPERR_SCALE_INVIP);
		THROW_PP(dig != 0 || (i != 0 && str_dig[0] == '0'), PPERR_SCALE_INVIP);
		if(dig == 0)
			encoded_ip[i+3] = (uchar)1;
		else
			encoded_ip[i] = (uchar)dig;
	}
	THROW_PP(i == 4, PPERR_SCALE_INVIP);
	strnzcpy(pEncodedIP, encoded_ip, 8);
	CATCHZOK
	return ok;
}

// static
int SLAPI PPObjScale::DecodeIP(const char * pEncodedIP, char * pIP)
{
	int    ok = 1;
	uint   i = 0;
	char   ip[64];
	ASSIGN_PTR(pIP, 0);
	memzero(ip, sizeof(ip));
	THROW_PP(sstrlen(pEncodedIP) > 4, PPERR_SCALE_INVIP);
	for(i = 0; i < 4; i++) {
		char   str_dig[4];
		ulong  dig = (i != 0 && static_cast<uint>(static_cast<uchar>(pEncodedIP[i+3])) == 1) ? 0 : static_cast<uint>(static_cast<uchar>(pEncodedIP[i]));
		ultoa(dig, str_dig, 10);
		strcat(ip, str_dig);
		if(i < 3)
			strcat(ip, ".");
	}
	strnzcpy(pIP, ip, 16);
	CATCHZOK
	return ok;
}

typedef	HANDLE (WINAPI *ICMPCREATEFILE)();
typedef	BOOL   (WINAPI *ICMPCLOSEHANDLE)(HANDLE);
typedef	DWORD  (WINAPI *ICMPSENDECHO)(HANDLE, DWORD, LPVOID, WORD, PIP_OPTION_INFORMATION, LPVOID, DWORD, DWORD);

// static
int PPObjScale::CheckForConnection(const char * pIPAddress, uint timeout, uint attemptCount)
{
	int    ok = 0;
	InetAddr addr;
	if(addr.FromStr(pIPAddress) > 0) {
		HMODULE h_icmp = ::LoadLibrary(_T("ICMP.DLL"));
		if(h_icmp) {
			ICMPCREATEFILE  proc_IcmpCreateFile  = reinterpret_cast<ICMPCREATEFILE>(GetProcAddress(h_icmp, "IcmpCreateFile"));
			ICMPCLOSEHANDLE proc_IcmpCloseHandle = reinterpret_cast<ICMPCLOSEHANDLE>(GetProcAddress(h_icmp, "IcmpCloseHandle"));
			ICMPSENDECHO    proc_IcmpSendEcho    = reinterpret_cast<ICMPSENDECHO>(GetProcAddress(h_icmp, "IcmpSendEcho"));
			if(proc_IcmpCreateFile && proc_IcmpCloseHandle && proc_IcmpSendEcho) {
				HANDLE hdl = proc_IcmpCreateFile();
				const  size_t echo_len = 32;
				char   echo_buf[echo_len];
				char   reply[sizeof(ICMP_ECHO_REPLY) + 32];
				for(uint c = 0; c < attemptCount; c++) {
					memset(echo_buf, '\xAA', sizeof(echo_buf));
					if(proc_IcmpSendEcho(hdl, addr, echo_buf, echo_len, 0, reply, sizeof(reply), timeout)) {
						ok = 1;
						break;
					}
				}
				if(hdl)
					proc_IcmpCloseHandle(hdl);
			}
			FreeLibrary(h_icmp);
		}
	}
	return ok;
}

class PPScaleDevice {
public:
	SLAPI  PPScaleDevice(int portNo, const PPScalePacket * pData) : UseBuf(0), PortNo(0), ReadCycleCount(0), ReadCycleDelay(0), H_Port(INVALID_HANDLE_VALUE)
	{
		if(pData) {
			if(!(pData->Rec.Flags & SCALF_TCPIP) && !oneof2(pData->Rec.ScaleTypeID, PPSCLT_CRCSHSRV, PPSCLT_DIGI))
				InitPort(portNo);
			else {
				NumGetColl = 0;
				MaxGetCollIters = 0;
			}
			Data = *pData;
		}
		else {
			InitPort(portNo);
			MEMSZERO(Data);
		}
	}
	virtual SLAPI ~PPScaleDevice() // @v10.3.2 @fix non-virtual-->virtual
	{
		if(H_Port != INVALID_HANDLE_VALUE)
			CloseHandle(H_Port);
	}
	virtual int  SLAPI SetConnection() = 0;
	virtual int  SLAPI CloseConnection() { return -1; }
	virtual int  SLAPI SendPLU(const ScalePLU *) = 0;
	virtual void SLAPI GetDefaultSysParams(PPScalePacket *) = 0;
	virtual int  SLAPI GetData(int * pGdsNo, double * pWeight)
	{
		ASSIGN_PTR(pGdsNo, 0);
		ASSIGN_PTR(pWeight, 0);
		return -1;
	}
	int     NumGetColl;
	int     MaxGetCollIters;
protected:
	int    SLAPI InitPort(int portNo);
	int    SLAPI GetChr();
	int    SLAPI PutChr(uint8, int direct = 0, int special = 0);
	int    SLAPI _PutChr(uint8, int direct = 0);
	int    SLAPI PutBuffer();
	int    SLAPI CheckSync(int syncCode, int validRetCode);
	int    SLAPI DistributeFile(const char * pFileName, int rmv);
	enum {
		amlfMaxText   = 0x0001
	};
	int    SLAPI GetAddedMsgLines(const ScalePLU * pPlu, uint maxLineLen, uint maxLineCount, long flags, StringSet & rLines);

	int     PortNo;
	int     ReadCycleCount; // ���������� ������� ������ �� COM-�����
	int     ReadCycleDelay; // �������� ����� ��������� ������ �� COM-�����
	HANDLE  H_Port;
	PPScalePacket Data;
	//SString ExpPaths;
	SBuffer Buf;
	int    UseBuf;
};

int SLAPI PPScaleDevice::GetAddedMsgLines(const ScalePLU * pPlu, uint maxLineLen, uint maxLineCount, long flags, StringSet & rLines)
{
	int    ok = -1;
	uint   line_count = 0;
	rLines.clear();
	if(pPlu->HasAddedMsg()) {
		int    done = 0;
		SString temp_buf, head, tail, line_buf;
		StringSet ss(SLBColumnDelim);
		ss.setBuf(pPlu->AddMsgBuf, sstrlen(pPlu->AddMsgBuf) + 1);
		for(uint p = 0; !done && ss.get(&p, temp_buf);) {
			temp_buf.Strip();
			while(!done && temp_buf.Wrap(maxLineLen, head, tail) > 0) {
				if(flags & amlfMaxText) {
					rLines.add(head);
					line_count++;
				}
				else if(line_count < maxLineCount) {
					rLines.add(head.Align(maxLineLen, ADJ_LEFT));
					line_count++;
				}
				else
					done = 1;
				temp_buf = tail;
			}
			if(!done) {
				if(flags & amlfMaxText) {
					rLines.add(temp_buf);
					line_count++;
				}
				else if(line_count < maxLineCount) {
					rLines.add(temp_buf.Align(maxLineLen, ADJ_LEFT));
					line_count++;
				}
				else
					done = 1;
			}
		}
		if(flags & amlfMaxText) {
			;
		}
		else {
			while(line_count < maxLineCount) {
				rLines.add(temp_buf.Z().Align(maxLineLen, ADJ_LEFT));
				line_count++;
			}
		}
		ok = 1;
	}
	return ok;
}

static PPScaleDevice * SLAPI GetScaleDevice(const PPScalePacket * pScaleData);

#define COM1            0
#define COM2            1

#define SETS            0
#define SEND            1
#define RECV            2
#define RETN            3
//
// combination of abyte
//
#define SCP_DATA_7          0x02
#define SCP_DATA_8          0x03
#define SCP_STOP_1          0x00
#define SCP_STOP_2          0x04
#define SCP_PARITY_NO       0x00
#define SCP_PARITY_ODD      0x08
#define SCP_PARITY_EVEN     0x18
#define SCP_BAUD_110        0x00
#define SCP_BAUD_150        0x20
#define SCP_BAUD_300        0x40
#define SCP_BAUD_600        0x60
#define SCP_BAUD_1200       0x80
#define SCP_BAUD_2400       0xA0
#define SCP_BAUD_4800       0xC0
#define SCP_BAUD_9600       0xE0
//
// upper byte of return cmd value
//
#define TIME_OUT        0x8000
#define TSR_EMPTY       0x4000
#define THR_EMPTY       0x2000
#define BREAK_DETECT    0x1000
#define FRAMING_ERROR   0x0800
#define PARITY_ERROR    0x0400
#define OVERRUN_ERROR   0x0200
#define DATA_READY      0x0100
//
// lower byte of return cmd value
//
#define RLS_DETECT      0x80
#define RING_INDICATOR  0x40
#define DATA_SET_READY  0x20
#define CLEAR_TO_SEND   0x10
#define CHANGE_IN_RLS   0x08
#define TER_DETECTOR    0x04
#define CHANGE_DSR      0x02
#define CHANGE_CTS      0x01
/*
	Default:
		MassaK:
			Get_Tries = 400
			Get_Delay = 5
			Put_Tries = 400
			Put_Delay = 5
		CAS LP15:
			DOS:
				Get_Tries = 2000
				Get_Delay = 1
				Put_Tries = 400
				Put_Delay = 5
			WIN32:
				Get_Tries = Ignored
				Get_Delay = 150
				Put_Tries = Ignored
				Put_Delay = Ignored
*/
int SLAPI PPScaleDevice::PutChr(uint8 c, int direct, int special)
{
	if(c == (uint8)0xff && Data.Rec.ScaleTypeID == PPSCLT_CAS && Data.Rec.LogNum && oneof2(Data.Rec.ProtocolVer, 1, 17) && !special)
		if(!_PutChr((uint8)0xff, direct))
			return 0;
	return _PutChr(c, direct);
}

static SString & __GetLastSystemErr(SString & rBuf)
{
	//const  DWORD last_err = GetLastError();
	//LPVOID p_msg_buf = 0;
	//::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, last_err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&p_msg_buf, 0, 0);
	//rBuf = SUcSwitch(static_cast<const TCHAR *>(p_msg_buf));
	//MessageBox(NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION);
	//LocalFree(p_msg_buf);
	SSystem::SFormatMessage(rBuf); // @v10.3.11
	rBuf.Chomp().Transf(CTRANSF_OUTER_TO_INNER);
	return rBuf;
}

int SLAPI PPScaleDevice::InitPort(int portNo)
{
	PortNo = portNo;
	NumGetColl = 0;
	MaxGetCollIters = 0;
	int    ok = 1;
	SString read_cycling_param;
	SString cycle_count, cycle_delay;
	PPIniFile ini_file;
	if(ini_file.Get(PPINISECT_SYSTEM, PPINIPARAM_COMPORTREADCYCLING, read_cycling_param) > 0) {
		read_cycling_param.Divide(',', cycle_count, cycle_delay);
		ReadCycleCount = cycle_count.ToLong();
		ReadCycleDelay = cycle_delay.ToLong();
	}
	else {
		ReadCycleCount = 0;
		ReadCycleDelay = 0;
	}
	if(H_Port != INVALID_HANDLE_VALUE) {
		CloseHandle(H_Port);
		H_Port = INVALID_HANDLE_VALUE;
	}
	SString file_name;
	GetComDvcSymb(comdvcsCom, portNo+1, 1, file_name);
	H_Port = ::CreateFile(SUcSwitch(file_name), GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0); // @unicodeproblem
	if(H_Port != INVALID_HANDLE_VALUE) {
		DCB    dcb;
		GetCommState(H_Port, &dcb);
		dcb.BaudRate = CBR_9600;	// �������� �������� ������
		dcb.fParity  = FALSE;		// ��������� �������� ��������
		dcb.Parity   = NOPARITY;	// 0,1,2,3,4 = ���, �� ����������, �� ��������, ������ 1, ������ 0
		dcb.ByteSize = 8;			// 4-8 (���������� ����� �������������� ��� � ������������ � ����������� ������.)
		dcb.StopBits = ONESTOPBIT;	// 0,1,2 = 1 �������� ���, 1.5 �������� ���� , 2 �������� ����
		ok = BIN(SetCommState(H_Port, &dcb));

		COMMTIMEOUTS cto;
		cto.ReadIntervalTimeout = MAXDWORD;
		cto.ReadTotalTimeoutMultiplier = MAXDWORD;
		cto.ReadTotalTimeoutConstant = Data.Rec.Get_Delay;
		cto.WriteTotalTimeoutConstant = 0;
		cto.WriteTotalTimeoutMultiplier = 0;
		ok = BIN(SetCommTimeouts(H_Port, &cto));
		ok = 1;                                  // ???

		// @paul {
		//PurgeComm(H_Port, PURGE_RXCLEAR); // ������� ������� ������ � �������� COM �����
		//PurgeComm(H_Port, PURGE_TXCLEAR); // ������� ������� �������� � �������� COM �����
		// }@paul
	}
	else {
		SString msg_buf;
		__GetLastSystemErr(msg_buf);
		PPSetError(PPERR_SCALE_PORTOPENFAULT, msg_buf.Quot('(', ')').CatDiv('-', 1).Cat(file_name));
		ok = 0;
	}
	return ok;
}

int SLAPI PPScaleDevice::GetChr()
{
	int    ok = 0;
	char   buf[32];
	DWORD  sz = 1;
	if(H_Port != INVALID_HANDLE_VALUE) {
		int r = 0;
		int cycle_count = (ReadCycleCount > 0) ? ReadCycleCount : 1;
		int cycle_delay = (ReadCycleDelay > 0) ? ReadCycleDelay : 0;
		int collision = 0;
		for(int i = 0; i < cycle_count; i++) {
			r = ReadFile(H_Port, buf, 1, &sz, 0);
			if(r && sz == 1) {
				// @debug {
				if(collision) {
					NumGetColl++;
					if(collision > MaxGetCollIters)
						MaxGetCollIters = collision;
				}
				// } @debug
				return buf[0];
			}
			else if(cycle_delay) {
				SDelay(cycle_delay);
				collision++;
			}
		}
		if(r == 0) {
			SString msg_buf;
			PPSetAddedMsgString(__GetLastSystemErr(msg_buf));
		}
		else
			PPSetAddedMsgString(Data.Rec.Name);
		return PPSetError(PPERR_SCALE_RCV);
		/*
		int r = ReadFile(H_Port, buf, sz, &sz, 0);
		if(!r || sz != 1) {
			SString msg_buf;
			PPSetAddedMsgString(__GetLastSystemErr(msg_buf));
			return PPSetError(PPERR_SCALE_RCV);
		}
		else
			return buf[0];
		*/
	}
	else {
		PPSetAddedMsgString(Data.Rec.Name);
		return PPSetError(PPERR_SCALE_RCV);
	}
}

int SLAPI PPScaleDevice::_PutChr(uint8 c, int /*direct*/)
{
	if(UseBuf) {
		return Buf.Write(&c, 1) ? 1 : PPSetErrorSLib();
	}
	else {
		uint8  buf[32];
		DWORD  sz = 1;
		buf[0] = c;
		buf[1] = 0;
		SDelay(Data.Rec.Put_Delay);
		return (H_Port != INVALID_HANDLE_VALUE && WriteFile(H_Port, buf, sz, &sz, 0) && sz == 1) ? 1 : PPSetError(PPERR_SCALE_SEND);
	}
}

int SLAPI PPScaleDevice::PutBuffer()
{
	int    ok = -1;
	if(H_Port != INVALID_HANDLE_VALUE) {
		DWORD  sz = static_cast<DWORD>(Buf.GetAvailableSize());
		DWORD  actual_sz = sz;
		STempBuffer temp_buf(sz);
		Buf.Read(temp_buf, sz);
		if(sz) {
			SDelay(Data.Rec.Put_Delay);
			ok = (WriteFile(H_Port, temp_buf.vcptr(), sz, &actual_sz, 0) && actual_sz == temp_buf.GetSize()) ? 1 : PPSetError(PPERR_SCALE_SEND);
		}
	}
	else
		ok = 0;
	return ok;
}

int SLAPI PPScaleDevice::CheckSync(int syncCode, int validRetCode)
{
	int    ok = 0;
	if(PutChr(syncCode)) {
		int ret = GetChr();
		if(ret)
			ok = (ret == validRetCode) ? 1 : PPSetError(PPERR_SCALE_NOSYNC);
	}
	return ok;
}

int SLAPI PPScaleDevice::DistributeFile(const char * pFileName, int rmv)
{
	int    ok = 1;
	SString temp_buf;
	Data.GetExtStrData(Data.extssPaths, temp_buf);
	if(temp_buf.NotEmptyS()) {
		SString buf = pFileName;
		StringSet ss(';', temp_buf);
		for(uint i = 0; ss.get(&i, temp_buf);) {
			SPathStruc::ReplacePath(buf, temp_buf, 1);
			if(fileExists(buf))
				SFile::Remove(buf);
			if(!rmv && !copyFileByName(pFileName, buf))
				return (PPError(PPERR_SLIB), 0);
		}
	}
	return ok;
}
//
// CommLP15
//
#define C_OK            0x10
#define C_FAIL          0x20
#define LOADINGRED      0x00
#define LOADWR          0x01
#define LOADPM          0x04
#define LOADPLU         0x02
#define REQDATE         0x03
#define REQTIME         0x04
#define REQPLU          0x05
#define REQFIP          0x06
#define INITPLU         0x07
#define REQPLUCODE      0x08

class CommLP15 : public PPScaleDevice {
public:
	SLAPI  CommLP15(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData), Err(0), MsgCode(0), SocketHandle(NULL), IsConnected(0)
	{
		if(!(Data.Rec.Flags & SCALF_SYSPINITED))
			GetDefaultSysParams(&Data);
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI CloseConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual int  SLAPI GetData(int * pGdsNo, double * pWeight);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
		if(pData) {
			if(Data.Rec.Flags & SCALF_TCPIP) {
				pData->Rec.Put_Delay = pData->Rec.Get_Delay = 1000;
				pData->Rec.Get_NumTries = pData->Rec.Put_NumTries = 0;
			}
			else {
				pData->Rec.Get_NumTries = 2000;
				pData->Rec.Get_Delay = 1;
				pData->Rec.Put_NumTries = 400;
				pData->Rec.Put_Delay = 5;
			}
		}
	}
private:
	int    SLAPI Is_v16() const
	{
		return BIN(oneof2(Data.Rec.ProtocolVer, 16, 17));
	}
	int    SLAPI CheckSync_16();
	int    SLAPI CheckAckForPLU();
	int    SLAPI GetInt();
	long   SLAPI GetLong();
	void   SLAPI PutInt(int16);
	void   SLAPI PutLong(long);
	void   SLAPI PutStr(const char *);
	int    SLAPI SendPLU_16(const ScalePLU * pPLU);

	int    Err;
	int16  MsgCode;      //
	int16  Reserve;      // @alignment
	int    IsConnected;  //
	SOCKET SocketHandle; //
};

int SLAPI CommLP15::CheckSync_16()
{
	int    ok = 1;
	uint8  ret[2];
	ret[0] = (uint8)Data.Rec.LogNum, ret[1] = 0;
	if(Data.Rec.Flags & SCALF_TCPIP) {
		if(IsConnected) {
			THROW_PP(send(SocketHandle, (const char *)ret, 1, 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
			THROW_PP(recv(SocketHandle, (char *)ret, 2, 0) != SOCKET_ERROR, PPERR_SCALE_RCV);
		}
	}
	else {
		THROW(PutChr(ret[0]));
		ret[0] = static_cast<uint8>(GetChr());
		ret[1] = static_cast<uint8>(GetChr());
	}
	PPSetAddedMsgString(0);
	THROW_PP(ret[0] == (uint8)Data.Rec.LogNum, PPERR_SCALE_NOTREADY); // ���� �� ��������
	THROW_PP(ret[1] == (uint8)0x80, PPERR_SCALE_NOTREADY);        // ���� �� ��������
	CATCHZOK
	return ok;
}

int SLAPI CommLP15::SetConnection()
{
	int    ok = 1;
	if(Data.Rec.Flags & SCALF_TCPIP) {
		char   send_timeout[32];
		char   rcv_timeout[32];
		//char   ip[64];
		long   res;
		SString port_buf;
		struct sockaddr_in sin;
		//memzero(ip, sizeof(ip));
		//THROW(PPObjScale::DecodeIP(Data.Rec.Port, ip));
		Data.GetExtStrData(Data.extssPort, port_buf);
		THROW_PP(PPObjScale::CheckForConnection(port_buf, 100, 5), PPERR_SCALE_NOTREADY);
		THROW_PP((SocketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) != INVALID_SOCKET, PPERR_SCALE_NOSYNC);
		sin.sin_family = AF_INET;
		sin.sin_port   = htons(1001);
		sin.sin_addr.S_un.S_addr = inet_addr(port_buf);
		memzero(send_timeout, sizeof(send_timeout));
		memzero(rcv_timeout,  sizeof(rcv_timeout));
		itoa(Data.Rec.Put_Delay, send_timeout, 10);
		itoa(Data.Rec.Get_Delay, rcv_timeout, 10);
		THROW_PP((res = setsockopt(SocketHandle, SOL_SOCKET, SO_SNDTIMEO, send_timeout, sstrleni(send_timeout))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		THROW_PP((res = setsockopt(SocketHandle, SOL_SOCKET, SO_RCVTIMEO, rcv_timeout, sstrleni(rcv_timeout))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		THROW_PP((res = connect(SocketHandle, reinterpret_cast<const sockaddr *>(&sin), sizeof(sin))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		IsConnected = 1;
		THROW(CheckSync_16());
	}
	else {
		THROW(InitPort(PortNo));
		MsgCode = 0;
		if(Data.Rec.LogNum > 0 && Data.Rec.LogNum < 16) {
			if(Data.Rec.ProtocolVer == 0) {
				THROW(PutChr((uint8)(Data.Rec.LogNum << 4)));
				PPSetAddedMsgString(0);
				THROW_PP(GetChr() == 6, PPERR_SCALE_NOTREADY); // ���� �� ��������
			}
			else if(Data.Rec.ProtocolVer == 1) {
				uint8 r0 = (uint8)0xff;
				uint8 r1 = (uint8)(Data.Rec.LogNum & 0xfL);
				THROW(PutChr(r0, 1, 1));
				THROW(PutChr(r1, 1, 1));
				THROW(r0 = GetChr());
				THROW(r1 = GetChr());
				if(r0 != 0xff || !(r1 == 0x40 || r1 == 0x4b)) {
					char add_msg[32];
					sprintf(add_msg, "0x%x", ((((int)r0) << 8) | r1));
					CALLEXCEPT_PP_S(PPERR_SCALE_NOTREADY, add_msg); // ���� �� ��������
				}
			}
			else if(Is_v16()) {
				if(Data.Rec.ProtocolVer == 17) {
					uint8 r0 = (uint8)0xff;
					uint8 r1 = (uint8)(Data.Rec.LogNum & 0xfL);
					THROW(PutChr(r0, 1, 1));
					THROW(PutChr(r1, 1, 1));
					THROW(r0 = GetChr());
					THROW(r1 = GetChr());
					if(r0 != 0xff || !(r1 == 0x40 || r1 == 0x4b)) {
						char add_msg[32];
						sprintf(add_msg, "0x%x", ((((int)r0) << 8) | r1));
						CALLEXCEPT_PP_S(PPERR_SCALE_NOTREADY, add_msg); // ���� �� ��������
					}
				}
				THROW(CheckSync_16());
			}
		}
	}
	CATCH
		if(Data.Rec.Flags & SCALF_TCPIP) {
			shutdown(SocketHandle, 2);
			closesocket(SocketHandle);
			IsConnected = 0;
		}
		ok = 0;
	ENDCATCH
	return ok;
}

int SLAPI CommLP15::CloseConnection()
{
	int    ok = -1;
	if(!(Data.Rec.Flags & SCALF_TCPIP)) {
		if(Data.Rec.LogNum > 0 && Data.Rec.LogNum < 16) {
			if(oneof2(Data.Rec.ProtocolVer, 1, 17)) {
				int8 r0 = (int8)0xff;
				int8 r1 = (int8)0x7f;
				PutChr(r0, 1, 1);
				PutChr(r1, 1, 1);
				r0 = GetChr();
				r1 = GetChr();
			}
			ok = 1;
		}
	}
	else if(IsConnected) {
		shutdown(SocketHandle, 2);
		closesocket(SocketHandle);
		IsConnected = 0;
	}
	return ok;
}

int SLAPI CommLP15::CheckAckForPLU()
{
	if(Is_v16() || Data.Rec.Flags & SCALF_TCPIP) {
		uint8 ret = 0;
		if(Data.Rec.Flags & SCALF_TCPIP)
			recv(SocketHandle, (char *)&ret, 1, 0);
		else
			ret = static_cast<uint8>(GetChr());
		if(ret == (uint8)0xAA)
			return 1;
		else if(ret == (uint8)0xEE) {
			SDelay(200);
			return -1;
		}
		else
			return PPSetError(PPERR_SCALE_NOACK);
	}
	else if(Data.Rec.ProtocolVer == 20) {
		return 1;
	}
	else {
		for(int i = 0; GetChr() != C_OK; i++)
			if(i > 10)
				return PPSetError(PPERR_SCALE_NOACK);
		return 1;
	}
}

int SLAPI CommLP15::GetInt()
{
	union {
		int16 result;
		uint8 ct[2];
	};
	ct[1] = static_cast<uint8>(GetChr());
	ct[0] = static_cast<uint8>(GetChr());
	return result;
}

long SLAPI CommLP15::GetLong()
{
	union {
		long  result;
		uint8 ct[4];
	};
	for(int i = 3; i >= 0; i--)
		ct[i] = static_cast<uint8>(GetChr());
	return result;
}

void SLAPI CommLP15::PutStr(const char * str)
{
	for(const char * p = str; *p; p++)
		PutChr(*p);
}

void SLAPI CommLP15::PutInt(int16 s_data)
{
	char * p_temp = (char *)&s_data;
	PutChr(p_temp[1]);
	PutChr(p_temp[0]);
}

void SLAPI CommLP15::PutLong(long s_data)
{
	char * p_temp = (char *)&s_data;
	for(int i = 3; i >= 0; i--)
		PutChr(p_temp[i]);
}

static void FillMsgBuf_CAS(const char * pMsg, uint lineLen, uchar * pDataBuf, uint * pIdx)
{
	uint p = DEREFPTRORZ(pIdx);
	const size_t msg_len = sstrlen(pMsg);
	uint i = 0;
	while(i < msg_len && i < lineLen) {
		pDataBuf[p++] = pMsg[i++];
	}
	while(i < lineLen) {
		pDataBuf[p++] = ' ';
		i++;
	}
	ASSIGN_PTR(pIdx, p);
}

int SLAPI CommLP15::SendPLU_16(const ScalePLU * pPLU)
{
	int    ok = 1;
	char   buf[32];
	uchar  data_buf[512];
	size_t p = 0;
	int16  local_msg_code = 0;

	THROW(CheckSync_16());
	data_buf[p++] = 0x82;
	// PLU
	*reinterpret_cast<long *>(data_buf+p) = pPLU->GoodsNo;
	p += 4;
	// BARCODE
	{
		memzero(buf, sizeof(buf));
		longfmtz(pPLU->Barcode, 6, buf, sizeof(buf));
		for(uint i = 0; i < 6; i++)
			data_buf[p++] = buf[5-i] - '0';
	}
	// GOODS NAME
	{
		char   name_part_1[48], name_part_2[48];
		const  int name_items_count = 2;
		SBaseBuffer name_items[name_items_count];
		memzero(name_part_1, sizeof(name_part_1));
		memzero(name_part_2, sizeof(name_part_2));
		name_items[0].Set(name_part_1, 28);
		name_items[1].Set(name_part_2, 28);
		SplitString(pPLU->GoodsName, name_items_count, name_items);
		for(int i = 0; i < name_items_count; i++) {
			size_t j = 0;
			// ���� ���� ������� ������� ����� �������� ���������� ���� ����� (�������������������) {
			//alignstr(name_items[i].ptr, name_items[i].len, ALIGN_CENTER);
			for(j = sstrlen(name_items[i].P_Buf); ((int)j) < name_items[i].Size; j++)
	   	    	name_items[i].P_Buf[j] = 0; // ' '
			// }
			/* ����������� �� ��������
			alignstr(name_items[i].ptr, name_items[i].len, ALIGN_CENTER);
			// @v5.7.8 VADIM {
			//for(j = sstrlen(name_items[i].ptr); j < name_items[i].len; j++)
			for(j = sstrlen(name_items[i].ptr); j >= 0 && name_items[i].ptr[j] == ' '; j--)
	   	    	name_items[i].ptr[j] = 0;
			// } @v5.7.8 VADIM
			*/
			for(j = 0; ((int)j) < name_items[i].Size; j++)
				data_buf[p++] = name_items[i].P_Buf[j];
		}
	}
	// PRICE
	{
		const long price = R0i(pPLU->Price * 100L);
		*reinterpret_cast<int32 *>(data_buf+p) = price;
		p += 4;
	}
	// EXPIRY
	{
		data_buf[p++] = (((pPLU->Expiry.day()   / 10) << 4) | (pPLU->Expiry.day()   % 10));
		data_buf[p++] = (((pPLU->Expiry.month() / 10) << 4) | (pPLU->Expiry.month() % 10));
		data_buf[p++] = (((pPLU->Expiry.year()  / 10) << 4) | (pPLU->Expiry.year()  % 10));
	}
	// TARE WEIGHT
	{
		*reinterpret_cast<int16 *>(data_buf+p) = 0;
		p += 2;
	}
	// GROUP CODE
	{
		memzero(buf, sizeof(buf));
		longfmtz(pPLU->GrpCode, 6, buf, sizeof(buf));
		for(uint i = 0; i < 6; i++)
			data_buf[p++] = buf[5-i] - '0';
	}
	// MSG CODE
	{
		if(pPLU->HasAddedMsg())
			local_msg_code = ++MsgCode;
		else
			local_msg_code = 0;
		*reinterpret_cast<int16 *>(data_buf+p) = local_msg_code;
		p += 2;
	}
	//
	//
	//
	//
	if(Data.Rec.Flags & SCALF_TCPIP) {
		THROW_PP(send(SocketHandle, reinterpret_cast<const char *>(data_buf), (int)p, 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
	}
	else {
		for(uint i = 0; i < p; i++)
			PutChr(data_buf[i]);
	}
	THROW(CheckAckForPLU());
	if(local_msg_code) {
		const uint line_len = 50;
		// @v10.5.0 const uint line_count = 8;
		const uint line_count = (Data.Rec.MaxAddedLnCount > 0 && Data.Rec.MaxAddedLnCount <= 100) ? Data.Rec.MaxAddedLnCount : 8; // @v10.5.0
		const size_t packet_size = (line_len * line_count) + 1 + 2;
		THROW(CheckSync_16());
		p = 0;
		memset(data_buf, ' ', 404U);
		data_buf[p++] = 0x84;
		// MSG CODE
		*reinterpret_cast<int16 *>(data_buf+p) = local_msg_code;
		p += 2;
		// MESSAGE
		{
			StringSet added_lines;
			SString line_buf;
			GetAddedMsgLines(pPLU, line_len, line_count, 0, added_lines);
			for(uint pos = 0; added_lines.get(&pos, line_buf);) {
				for(uint i = 0; i < line_buf.Len(); i++)
					data_buf[p++] = line_buf.C(i);
			}
			while(p < packet_size)
				data_buf[p++] = ' ';
		}
		assert(p == packet_size);
		if(Data.Rec.Flags & SCALF_TCPIP) {
			THROW_PP(send(SocketHandle, (const char *)data_buf, (int)p, 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
		}
		else {
			for(uint i = 0; i < p; i++)
				PutChr(data_buf[i]);
		}
		THROW(CheckAckForPLU());
	}
	CATCHZOK
	return ok;
}

int SLAPI CommLP15::SendPLU(const ScalePLU * pPLU)
{
	int   ok = 1, i;
	if(Is_v16() || Data.Rec.Flags & SCALF_TCPIP) {
		THROW(SendPLU_16(pPLU));
	}
	else {
		THROW(CheckSync(0xA5, 0x77));
		PutChr(LOADPLU);
		THROW_PP(GetChr(), PPERR_SCALE_NOTREGMODE); // SCALE IS NOT REG MODE
		PutInt((int16)pPLU->GoodsNo);
		THROW_PP(GetInt() == (int16)pPLU->GoodsNo, PPERR_SCALE_INVPLUID); // SCALE CAN NOT LOAD DATA
		PutLong(pPLU->Barcode);       // ITEM CODE
		{
			char  name_part_1[48], name_part_2[48];
			const int name_items_count = 2;
			SBaseBuffer name_items[name_items_count];
			memzero(name_part_1, sizeof(name_part_1));
			memzero(name_part_2, sizeof(name_part_2));
			name_items[0].Set(name_part_1, 28);
			name_items[1].Set(name_part_2, 28);
			SplitString(pPLU->GoodsName, name_items_count, name_items);
			for(i = 0; i < name_items_count; i++) {
				alignstr(name_items[i].P_Buf, name_items[i].Size, ALIGN_CENTER);
				// ���� ���� ������� ������� ����� �������� ���������� ���� ����� (�������������������) {
				for(size_t j = sstrlen(name_items[i].P_Buf); ((int)j) < name_items[i].Size; j++)
	   	    		name_items[i].P_Buf[j] = ' ';
				// }
				/* ����������� �� ��������
				// @v5.7.8 VADIM {
				//for(j = sstrlen(name_items[i].ptr); j < name_items[i].len; j++)
				for(j = sstrlen(name_items[i].ptr); j >= 0 && name_items[i].ptr[j] == ' '; j--)
	   	    		name_items[i].ptr[j] = 0;
				// } @v5.7.8 VADIM
				*/
				PutStr(name_items[i].P_Buf);
			}
		}
		PutLong(R0i(pPLU->Price * 100L)); // UNIT PRICE
		{
			long   numdays = diffdate(pPLU->Expiry, getcurdate_());
			if(numdays <= 0 || numdays > 366)
				numdays = 366;
			PutInt((short)numdays);   // SHELF LIFE DATE
		}
		PutInt(0);                    // TARE WEIGHT
		PutInt((int16)pPLU->GrpCode); // GROUP CODE
		PutInt(0);            // MESSAGE CODE
		//PutInt(++MsgCode);            // MESSAGE CODE
		THROW(CheckAckForPLU());
		/*PutChr(LOADINGRED);
		THROW_PP(GetChr(), PPERR_SCALE_NOTREGMODE); // SCALE IS NOT REG MODE
		PutInt(MsgCode);
		THROW_PP(GetInt() == MsgCode, PPERR_SCALE_INVPLUID); // SCALE CAN NOT LOAD DATA
		{
			const char * p = pPLU->GdsAddFld1;
			char  ingr_str[51];
			ingr_str[50] = 0;
			for(i = 0; i < 5; i++) {
				STRNSCPY(ingr_str, p);
				for(j = sstrlen(ingr_str); j < 50; j++)
					ingr_str[j] = ' ';
				PutStr(ingr_str);
				p += sizeof(pPLU->GdsAddFld1);
			}
			memset(ingr_str, ' ', (size_t)50);
			for(; i < 8; i++)
				PutStr(ingr_str);
		}
		THROW(CheckAckForPLU());
		*/
	}
	CATCHZOK
	return ok;
}

int SLAPI CommLP15::GetData(int * pGdsNo, double * pWeight)
{
	int    ok = -1, gds_no = 0;
	double weight = 0.0;
	uchar  ask = 0x89;

	struct _ScaleStatus {
		uchar  Status;
		// ��� 0 = 1 - ����������
		// ��� 1 = 0 - ������
		// ��� 2 = 1 - ������� ����� ������ ����
		// ��� 3 = 1 - ������� ���
		// ��� 4 = 0 - ������
		// ��� 5 = 1 - ������� ��������������� �����
		// ��� 6 = 1 - ��� ����������
		// ��� 7 = 1 - ���� �������� ���� �������������
		uint16 Weight;    // � �������
		long   Price;     // � ��������
		long   Sum;       // � ��������
		long   PLUNumber;
	} sc_st;

	// @paul {
	if(Data.Rec.ProtocolVer == 20) { // ���� CAS AP, CAS ER JR
		const uchar ask_apweight = 0x11;
		const uchar ask_apall = 0x12;
		//
		// ����������� ����� (bcc) � ������� ���� ���������� ��������������� �����������
		// �������� (������), ������� � �������, ��������������� ���������� �� ������ SOH
		// ��� STX � �������� � �� ����� ����������� ����� bcc (��������������)
		//
		char   bcc = 0;
		uchar  c = 0;
		size_t pos = 0;
		SString buf, temp_buf;
		uint8 r0 = 0;
		THROW(SetConnection()); // @v9.0.0
		r0 = (uint8)0x05;
		THROW_PP(PutChr(r0, 1, 1), PPERR_SCALE_NOSYNC); // �������� ������ � ����� �� ��������� ���������� (0x05)
		THROW_PP(r0 = GetChr(), PPERR_SCALE_NOSYNC); // �������� ����� �����
		if(r0 != 0x06) {
			char   add_msg[32];
			sprintf(add_msg, "0x%x", ((((int)r0) << 8)));
			CALLEXCEPT_PP_S(PPERR_SCALE_NOTREADY, add_msg); // ���� �� ��������
		}
		// ���� � ������� 3� ������ ����� ������� ������ 0x06 �� ���������� �� �������� ����� (0x11 ��� 0x12), ������ �����������
		THROW_PP(PutChr(ask_apall), PPERR_SCALE_NOSYNC); // �������� ����� �� ��������� ������

		buf = 0;
		// �������� 37 ���� ������
		for(pos = 0; pos < 37; pos++) {
			THROW_PP(c = GetChr(), PPERR_SCALE_RCV);
			buf.CatChar(c);
		}

		THROW_PP(buf[0] == 0x01, PPERR_SCALE_RCV); // 0x01 (SOH)
		THROW_PP(buf[1] == 0x02, PPERR_SCALE_RCV); // 0x02 (STX)
		THROW_PP(buf[12] == 0x02, PPERR_SCALE_RCV); // 0x02 (STX)
		THROW_PP(buf[25] == 0x02, PPERR_SCALE_RCV); // 0x02 (STX)
		THROW_PP(buf[11] == 0x03, PPERR_SCALE_RCV); // 0x03 (ETX)
		THROW_PP(buf[24] == 0x03, PPERR_SCALE_RCV); // 0x03 (ETX)
		THROW_PP(buf[35] == 0x03, PPERR_SCALE_RCV); // 0x03 (ETX)
		THROW_PP(buf[36] == 0x04, PPERR_SCALE_RCV); // 0x04 (EOT)

		sc_st.Status = 0;
		if(buf[14] == 0x46) {
			sc_st.Status |= (1<<0); // 1 - ����������
		}
		if(buf[13] == 0x53) {
			sc_st.Status |= (1<<6); // 1 - ��� ����������
		}
		if(buf[14] == 0x2d) {
			sc_st.Status |= (1<<7); // 1 - ������������� ���
		}

		THROW_PP(buf[13] == 0x53, PPERR_SCALE_RCV); // ������� ������������ ����: 0x53 ("S") - ��������, 0x55 ("U") - ����������
		THROW_PP(buf[14] == 0x20, PPERR_SCALE_RCV); // ���� ����: ������������� ���: 0x2d ("-"), ������� ��� ������������� ���: 0x20 (" "), ����������: 0x46 ("F")

		// 2 ����� ������� ���������: kg ��� lb
		THROW_PP(buf[21] == 0x6b, PPERR_SCALE_RCV); // 0x6b ("k")
		THROW_PP(buf[22] == 0x67, PPERR_SCALE_RCV); // 0x67 ("g")

		buf.Sub(2,8,temp_buf); // �������� ����� ���������� �� ���������
		bcc = 0;
		for(pos = 0; pos < temp_buf.Len(); pos++) {
			bcc ^= temp_buf[pos];
		}
		THROW_PP(bcc == buf[10], PPERR_SCALE_RCV); // ����������� ����
		sc_st.Sum = (long)round(temp_buf.ToReal() * 100.0, 0); // ���������
		buf.Sub(13, 10, temp_buf);
		bcc = 0;
		for(pos = 0; pos < temp_buf.Len(); pos++) {
			bcc ^= temp_buf[pos];
		}
		THROW_PP(bcc == buf[23], PPERR_SCALE_RCV); // ����������� ����
		buf.Sub(15, 6, temp_buf); // �������� ����� ���������� �� ���
		sc_st.Weight = static_cast<uint16>(round(temp_buf.ToReal() * 1000.0, 0)); // ���
		if(sc_st.Weight == 0) {
			sc_st.Status |= (1<<3); // 1 - ������� ���
		}
		buf.Sub(26, 8, temp_buf); // �������� ����� ���������� �� ����
		bcc = 0;
		for(pos = 0; pos < temp_buf.Len(); pos++) {
			bcc ^= temp_buf[pos];
		}
		THROW_PP(bcc == buf[34], PPERR_SCALE_RCV); // ����������� ����
		sc_st.Price = static_cast<long>(round(temp_buf.ToReal() * 100.0, 0)); // ����
		weight = static_cast<double>(sc_st.Weight) / 1000.0;
		ok = 1;
	}
	// } @paul
	else {
		THROW(SetConnection());
		// ��������� ������ ��� TCP/IP !!!
		if(Data.Rec.Flags & SCALF_TCPIP) {
			THROW_PP(send(SocketHandle, (const char *)&ask, 1, 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
		}
		else
			PutChr(ask);
		THROW(CheckAckForPLU());
		if(Data.Rec.Flags & SCALF_TCPIP) {
			THROW_PP(recv(SocketHandle, (char *)&sc_st, sizeof(sc_st), 0) != SOCKET_ERROR, PPERR_SCALE_RCV);
		}
		else {
			sc_st.Status    = (uchar)GetChr();
			sc_st.Weight    = GetInt();
			sc_st.Price     = GetLong();
			sc_st.Sum       = GetLong();
			sc_st.PLUNumber = GetLong();
		}
		// ���������� ��� ������ ������ ������ � �����,
		// �.�. � ��������� ������� ok = -1 ���� ��������� �� ������
		PPSetError(PPERR_SCALE_RCV);
		//
		if((sc_st.Status & 0x40) && !(sc_st.Status & 0x8D)) {
			gds_no = (int)sc_st.PLUNumber;
			weight = ((double)sc_st.Weight) / 1000.0;
			ok = 1;
		}
	}
	CATCHZOK
	CloseConnection();
	ASSIGN_PTR(pGdsNo,  gds_no);
	ASSIGN_PTR(pWeight, weight);
	return ok;
}
//
// CasCL5000J
//
class CasCL5000J : public PPScaleDevice {
public:
	SLAPI  CasCL5000J(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData), Err(0), MsgCode(0), SocketHandle(NULL), IsConnected(0)
	{
		if(!(Data.Rec.Flags & SCALF_SYSPINITED))
			GetDefaultSysParams(&Data);
		{
			SString buf, err_codes;
			PPLoadText(PPTXT_CASCL5000J_ERRCODES, err_codes);
			StringSet ss(';', err_codes);
			for(uint i = 0; ss.get(&i, buf);) {
				SString str_id, str_text;
				buf.Divide(':', str_id, str_text);
				ErrorCodes.Add(str_id.ToLong(), str_text);
			}
			ErrorCodes.SortByID();
		}
		{
			PPGoodsConfig goods_cfg;
			PPObjGoods::ReadConfig(&goods_cfg);
			WghtPrefix2 = goods_cfg.WghtPrefix;
			(WghtPrefix1 = WghtPrefix2).ShiftLeft();
		}
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI CloseConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual int  SLAPI GetData(int * pGdsNo, double * pWeight);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
		if(pData) {
			if(Data.Rec.Flags & SCALF_TCPIP)
				pData->SetSysParams(0, 1000, 0, 1000);
			else
				pData->SetSysParams(2000, 1, 400, 5);
		}
	}
private:
	int    SLAPI SendBarcodeFormat(const char * pBarcode);
	int    SLAPI SendData(const char * pBuf, size_t bufLen);
	int    SLAPI CheckSync();
	int    SLAPI CheckAck();
	int    SLAPI GetInt();
	long   SLAPI GetLong();
	void   SLAPI PutInt(int16);
	void   SLAPI PutLong(long);
	void   SLAPI PutStr(const char *);

	StrAssocArray ErrorCodes;
	SString WghtPrefix1;
	SString WghtPrefix2;
	int    Err;
	int16  MsgCode;
	int16  Reserve;      // @alignment
	int    IsConnected;
	SOCKET SocketHandle;
};

int SLAPI CasCL5000J::CheckSync()
{
	return 1;
	/*
	int    ok = 1;
	uint8  ret[2];
	ret[0] = (uint8)Data.LogNum, ret[1] = 0;
	if(Data.Flags & SCALF_TCPIP) {
		if(IsConnected) {
			THROW_PP(send(SocketHandle, (const char *)ret, 1, 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
			THROW_PP(recv(SocketHandle, (char *)ret, 2, 0) != SOCKET_ERROR, PPERR_SCALE_RCV);
		}
	}
	PPSetAddedMsgString(0);
	THROW_PP(ret[0] == (uint8)Data.LogNum, PPERR_SCALE_NOTREADY); // ���� �� ��������
	THROW_PP(ret[1] == (uint8)0x80, PPERR_SCALE_NOTREADY);        // ���� �� ��������
	CATCHZOK
	return ok;
	*/
}

int SLAPI CasCL5000J::SetConnection()
{
	int    ok = 1;
	if(Data.Rec.Flags & SCALF_TCPIP) {
		char   send_timeout[10], rcv_timeout[10];
		//char   ip[16];
		SString port_buf;
		long   res;
		struct sockaddr_in sin;
		//memzero(ip, sizeof(ip));
		//THROW(PPObjScale::DecodeIP(Data.Rec.Port, ip));
		Data.GetExtStrData(Data.extssPort, port_buf);
		THROW_PP(PPObjScale::CheckForConnection(port_buf, 100, 5), PPERR_SCALE_NOTREADY);
		THROW_PP((SocketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) != INVALID_SOCKET, PPERR_SCALE_NOSYNC);
		sin.sin_family = AF_INET;
		sin.sin_port   = htons(20304);
		sin.sin_addr.S_un.S_addr = inet_addr(port_buf);
		memzero(send_timeout, sizeof(send_timeout));
		memzero(rcv_timeout,  sizeof(rcv_timeout));
		itoa(Data.Rec.Put_Delay, send_timeout, 10);
		itoa(Data.Rec.Get_Delay, rcv_timeout, 10);
		THROW_PP((res = setsockopt(SocketHandle, SOL_SOCKET, SO_SNDTIMEO, (const char *)send_timeout, sstrleni(send_timeout))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		THROW_PP((res = setsockopt(SocketHandle, SOL_SOCKET, SO_RCVTIMEO, (const char *)rcv_timeout, sstrleni(rcv_timeout))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		THROW_PP((res = connect(SocketHandle, reinterpret_cast<const sockaddr *>(&sin), sizeof(sin))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		IsConnected   = 1;
		THROW(CheckSync());
	}
	CATCH
		if(Data.Rec.Flags & SCALF_TCPIP) {
			shutdown(SocketHandle, 2);
			closesocket(SocketHandle);
			IsConnected = 0;
		}
		ok = 0;
	ENDCATCH
	return ok;
}

int SLAPI CasCL5000J::CloseConnection()
{
	int    ok = -1;
	if(IsConnected) {
		shutdown(SocketHandle, 2);
		closesocket(SocketHandle);
		IsConnected = 0;
		ok = 1;
	}
	return ok;
}

int SLAPI CasCL5000J::CheckAck()
{
	int    ok = -1;
	if(Data.Rec.Flags & SCALF_TCPIP) {
		char recv_buf[1024];
		memzero(recv_buf, sizeof(recv_buf));
		recv(SocketHandle, (char *)recv_buf, sizeof(recv_buf), 0);
		if(recv_buf[0] == 'G')
			ok = 1;
		else if(recv_buf[0] == 'N') {
			uint8  err_code = recv_buf[14];
			SString msg;
			ErrorCodes.GetText((long)err_code, msg);
			ok = PPSetError(PPERR_SCALE_NOACK_S, msg);
		}
		else
			ok = PPSetError(PPERR_SCALE_NOACK);
	}
	return ok;
}

int SLAPI CasCL5000J::GetInt()
{
	union {
		int16 result;
		uint8 ct[2];
	};
	ct[1] = static_cast<uint8>(GetChr());
	ct[0] = static_cast<uint8>(GetChr());
	return result;
}

long SLAPI CasCL5000J::GetLong()
{
	union {
		long  result;
		uint8 ct[4];
	};
	for(int i = 3; i >= 0; i--)
		ct[i] = static_cast<uint8>(GetChr());
	return result;
}

void SLAPI CasCL5000J::PutStr(const char * str)
{
	for(const char * p = str; *p; p++)
		PutChr(*p);
}

void SLAPI CasCL5000J::PutInt(int16 s_data)
{
	const char * p_temp = reinterpret_cast<const char *>(&s_data);
	PutChr(p_temp[1]);
	PutChr(p_temp[0]);
}

void SLAPI CasCL5000J::PutLong(long s_data)
{
	const char * p_temp = reinterpret_cast<const char *>(&s_data);
	for(int i = 3; i >= 0; i--)
		PutChr(p_temp[i]);
}

int SLAPI CasCL5000J::SendData(const char * pBuf, size_t bufLen)
{
	int    ok = 1;
	size_t data_size = bufLen + 3;
	uint32 check_sum = 0;
	uchar * p_buf = new uchar[data_size];
	p_buf[bufLen] = ':';
	// CHECK SUM
	for(uint i = 0; i <= bufLen; i++) {
		if(i < bufLen)
			p_buf[i] = (uchar)pBuf[i];
		if(i > 1)
			check_sum += (uint32)p_buf[i];
	}
	p_buf[bufLen + 1] = (uint8)(check_sum % 0x100);
	// CR
	p_buf[bufLen + 2] = 0x0D;
	if(Data.Rec.Flags & SCALF_TCPIP) {
		THROW_PP(send(SocketHandle, reinterpret_cast<const char *>(p_buf), (int)data_size, 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
	}
	else {
		for(uint i = 0; i < data_size; i++)
			PutChr(p_buf[i]);
	}
	THROW(CheckAck());
	CATCHZOK
	delete [] p_buf; // @v10.5.0 @fix
	return ok;
}

int SLAPI CasCL5000J::SendPLU(const ScalePLU * pPLU)
{
	const  size_t direct_message_max_size = 300;
	int    ok = 1;
	char   data_buf[1024], buf[8];
	char   direct_message[direct_message_max_size*2]; // *2 - ���������
	const  uint16 data_size = 147;
	SString barcode;
	size_t p = 0;
	size_t msg_pos = 0;
	THROW(CheckSync());
	longfmtz(pPLU->Barcode, 6, buf, sizeof(buf));
	barcode = buf;
	if(barcode.CmpPrefix(WghtPrefix1, 1) == 0) {
		//
		// DIRECT MESSAGE
		//
		{
			direct_message[0] = 0;
			SString line_buf;
			StringSet added_lines;
			if(GetAddedMsgLines(pPLU, NZOR(Data.Rec.MaxAddedLn, 50), direct_message_max_size-1, amlfMaxText, added_lines) > 0) {
				for(uint pos = 0; msg_pos < (direct_message_max_size-1) && added_lines.get(&pos, line_buf);) {
					line_buf.Transf(CTRANSF_INNER_TO_OUTER); // @fix @v8.6.4
					const size_t _len = line_buf.Len();
					if(_len) {
						if(msg_pos)
							direct_message[msg_pos++] = '\x0A';
						for(uint i = 0; msg_pos < (direct_message_max_size-1) && i < _len; i++)
							direct_message[msg_pos++] = line_buf.C(i);
					}
				}
			}
			if(msg_pos)
				direct_message[msg_pos++] = 0;
		}
		barcode.ShiftLeft();
		memzero(data_buf, sizeof(data_buf));
		data_buf[p++] = 'W';            // Write
		data_buf[p++] = 'L';            // Barcode
		*reinterpret_cast<uint32 *>(data_buf + p) = 0; // Address
		p += sizeof(uint32);
		data_buf[p++] = ',';
		data_buf[p] = static_cast<char>(data_size+msg_pos); // Size of data
		p += sizeof(uint16);
		data_buf[p++] = ':';
		// DEPARTMENT NUMBER
		PTR16(data_buf + p)[0] = 1;
		p += sizeof(uint16);
		// PLU
		*reinterpret_cast<uint32 *>(data_buf + p) = pPLU->GoodsNo;
		p += sizeof(uint32);
		// TYPE
		*reinterpret_cast<uint8 *>(data_buf + p) = 1;
		p += sizeof(uint8);
		// NAME1
		{
			char   name_part_1[128], name_part_2[128];
			SString goods_name;
			SBaseBuffer name_items[2];
			memzero(name_part_1, sizeof(name_part_1));
			memzero(name_part_2, sizeof(name_part_2));
			name_items[0].Set(name_part_1, 40);
			name_items[1].Set(name_part_2, 40);
			goods_name = pPLU->GoodsName;
			goods_name.Transf(CTRANSF_INNER_TO_OUTER);
			SplitString(goods_name, 2, name_items);
			strnzcpy((data_buf + p), name_part_1, 41);
			p += 40;
			// NAME2
			strnzcpy((data_buf + p), name_part_2, 41);
			p += 40;
			// NAME3
			p += 5;
		}
		// GROUP CODE
		PTR16(data_buf + p)[0] = 0;
		p += sizeof(uint16);
		// LABEL NUMBER
		PTR16(data_buf + p)[0] = 0;
		p += sizeof(uint16);
		// RESERVE
		PTR16(data_buf + p)[0] = 0;
		p += sizeof(uint16);
		// ORIGIN NUMBER
		PTR16(data_buf + p)[0] = 0;
		p += sizeof(uint16);
		// RESERVE
		*reinterpret_cast<uint8 *>(data_buf + p) = 0;
		p += sizeof(uint8);
		// FIXED WEIGHT
		*reinterpret_cast<uint32 *>(data_buf + p) = 0;
		p += sizeof(uint32);
		// ITEM CODE
		*reinterpret_cast<uint32 *>(data_buf + p) = barcode.ToLong();
		p += sizeof(uint32);
		// PCS
		PTR16(data_buf + p)[0] = 0;
		p += sizeof(uint16);
		// PCS SYMBOL
		*reinterpret_cast<uint8 *>(data_buf + p) = 0;
		p += sizeof(uint8);
		// USE FIXED PRICE TYPE
		*reinterpret_cast<uint8 *>(data_buf + p) = 0;
		p += sizeof(uint8);
		// UNIT PRICE
		*reinterpret_cast<uint32 *>(data_buf + p) = R0i(pPLU->Price * 100L);
		p += sizeof(uint32);
		// SPECIAL PRICE
		*reinterpret_cast<uint32 *>(data_buf + p) = 0;
		p += sizeof(uint32);
		// TARE WEIGHT
		*reinterpret_cast<uint32 *>(data_buf + p) = 0;
		p += sizeof(uint32);
		// TARE NUMBER
		*reinterpret_cast<uint8 *>(data_buf + p) = 0;
		p += sizeof(uint8);
		// BARCODE NUMBER
		PTR16(data_buf + p)[0] = (uint16)1;
		p += sizeof(uint16);
		// RESERVE
		PTR16(data_buf + p)[0] = 0;
		p += sizeof(uint16);
		// PRODUCE DATE
		PTR16(data_buf + p)[0] = 0;
		p += sizeof(uint16);
		// PACKED DATE
		PTR16(data_buf + p)[0] = 0;
		p += sizeof(uint16);
		// PACKED TIME
		*reinterpret_cast<uint8 *>(data_buf + p) = 0;
		p += sizeof(uint8);
		// SELL BY DATE (EXPIRY in days)
		{
			if(pPLU->Expiry.year()) {
				*reinterpret_cast<uint32 *>(data_buf + p) = diffdate(pPLU->Expiry, getcurdate_()) + 1;
			}
			else
				*reinterpret_cast<uint32 *>(data_buf + p) = 0;
		}
		p += sizeof(uint32);
		// SELL BY TIME
		*reinterpret_cast<uint8 *>(data_buf + p) = 0;
		p += sizeof(uint8);
		// MESSAGE NUMBER
		PTR16(data_buf + p)[0] = 0;
		p += sizeof(uint16);
		// RESERVE
		PTR16(data_buf + p)[0] = 0;
		p += sizeof(uint16);
		// RESERVE
		PTR16(data_buf + p)[0] = 0;
		p += sizeof(uint16);
		// SALE MESSAGE NUMBER
		*reinterpret_cast<uint8 *>(data_buf + p) = 0;
		p += sizeof(uint8);
		if(msg_pos) {
			memcpy(data_buf+p, direct_message, msg_pos);
			p += msg_pos;
		}
		//
		// BARCODE
		//
		{
			const char * p_fmt = "IIIIIWWWWW";
			SString fmt;
			fmt.Cat(WghtPrefix2).Cat(p_fmt);
			THROW(SendBarcodeFormat(fmt));
		}
		THROW(SendData(data_buf, p));
	}
	CATCHZOK
	return ok;
}

int SLAPI CasCL5000J::SendBarcodeFormat(const char * pFmt)
{
	int    ok = 1;
	if(!isempty(pFmt)) {
		uint16 data_size = 36;
		char   data_buf[40], fmt[31];
		size_t p = 0;

		memzero(data_buf, sizeof(data_buf));
		strnzcpy(fmt, pFmt, sstrlen(pFmt) + 1);
		data_buf[p++] = 'W';                      // Op Write
		data_buf[p++] = 'B';                      // Barcode
		*((uint32 *)(data_buf + p)) = 0;           // Address
		p += sizeof(uint32);
		data_buf[p++] = ',';
		PTR16(data_buf + p)[0] = data_size;     // Size of data
		p += sizeof(uint16);
		data_buf[p++] = ':';
		// BARCODE NUMBER
		*(uint32 *)(data_buf + p) = 1;
		p += sizeof(uint32);
		// BARCODE Type
		*(uint8 *)(data_buf + p) = 1; // EAN13
		p += sizeof(uint8);
		// BARCODE format
		strnzcpy((data_buf + p), fmt, sstrlen(fmt) + 1);
		p += sizeof(fmt);
		THROW(ok = SendData(data_buf, p));
	}
	CATCHZOK
	return ok;
}

int SLAPI CasCL5000J::GetData(int * pGdsNo, double * pWeight)
{
	return -1;
}
//
//
//
class CommMassaK : public PPScaleDevice {
public:
	SLAPI  CommMassaK(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData), Err(0)
	{
		if(!(Data.Rec.Flags & SCALF_SYSPINITED))
			GetDefaultSysParams(&Data);
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
		CALLPTRMEMB(pData, SetSysParams(400, 5, 400, 5));
	}
private:
	int    SLAPI CheckAck(uchar ackCode = 0x01);
	int    SLAPI GetNumber(long *, int numBytes);
	int    SLAPI PutBuf(char *, size_t numBytes);
	int    SLAPI PutNumber(long number, size_t numBytes);
	int    SLAPI MakeNumber(long number, char * pBuf, size_t bufLen);
	int    SLAPI MakeString(const char * pStr, char * pBuf, size_t bufLen);

	int    Err;
};

int SLAPI CommMassaK::SetConnection()
{
	//bioscom(SETS, (SCP_BAUD_9600 | SCP_DATA_8 | SCP_STOP_1 | SCP_PARITY_NO), PortNo);
	InitPort(PortNo);
	return 1;
}

int SLAPI CommMassaK::CheckAck(uchar ackCode)
	{ return BIN(GetChr() == ackCode); }

int SLAPI CommMassaK::MakeNumber(long number, char * pBuf, size_t bufLen)
{
	union {
		long  n;
		uchar c[4];
	};
	n = number;
	for(uint i = 0; i < bufLen; i++)
		pBuf[i] = c[i];
	return 1;
}

int SLAPI CommMassaK::MakeString(const char * pStr, char * pBuf, size_t bufLen)
{
	size_t len = sstrlen(pStr);
	for(size_t i = 0; i < bufLen; i++)
		pBuf[i] = (i < len) ? pStr[i] : ' ';
	return 1;
}

int SLAPI CommMassaK::PutBuf(char * pBuf, size_t numBytes)
{
	for(uint i = 0; i < numBytes; i++)
		if(!PutChr(pBuf[i]))
			return 0;
	return 1;
}

int SLAPI CommMassaK::PutNumber(long number, size_t numBytes)
{
	char   b[32];
	MakeNumber(number, b, numBytes);
	return PutBuf(b, numBytes);
}

int SLAPI CommMassaK::SendPLU(const ScalePLU * pPLU)
{
	int    ok = 1;
	char   b[128];
	char   name_part_1[48], name_part_2[48];
	SBaseBuffer name_items[2];
	name_items[0].Set(name_part_1, 24);
	name_items[1].Set(name_part_2, 24);
	THROW(CheckSync(0xA5, 0x77));
	THROW(CheckSync(0x05, 0x01));
	THROW(PutNumber(pPLU->GoodsNo, 2));
	THROW(CheckSync(0x21, 0x01));
	THROW(PutNumber(0, 1));
	THROW(PutNumber(1 /* @v4.5.8 pPLU->GrpCode */, 1));
	THROW(PutNumber(pPLU->Barcode, 3));
	THROW(PutNumber((long)(R3(pPLU->Price * 100L)), 3));
	THROW(PutNumber(0, 2));
	THROW(PutNumber(pPLU->Expiry.year() % 100, 1));
	THROW(PutNumber(pPLU->Expiry.month(), 1));
	THROW(PutNumber(pPLU->Expiry.day(), 1));
	SplitString(pPLU->GoodsName, 2, name_items);
	MakeString(name_part_1, b, 24);
	THROW(PutBuf(b, 24));
	MakeString(name_part_2, b, 24);
	THROW(PutBuf(b, 24));
	THROW(CheckAck(0x10));
	CATCHZOK
	return ok;
}
//
//	COMMassaK
//
class COMMassaK : public PPScaleDevice {
public:
	SLAPI  COMMassaK(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData), P_DrvMassaK(0), ResCode(mkErrOK)
	{
		if(H_Port != INVALID_HANDLE_VALUE) {
			CloseHandle(H_Port);
			H_Port = INVALID_HANDLE_VALUE;
		}
		PPGoodsConfig goods_cfg;
		PPObjGoods::ReadConfig(&goods_cfg);
		WghtPrefix = atol(goods_cfg.WghtPrefix);
	}
	SLAPI ~COMMassaK()
	{
		delete P_DrvMassaK;
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI CloseConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
	}
private:
	enum {
		Result,
		CurrentNumber,
		Initialize,
		Finalize,
		SetBasicGoodsParams
	};
	enum ResultCodes {
		mkErrOK,
		mkErrInit,
		mkErrSynk,
		mkErrNotReady,
		mkErrIO,
		mkErrAnswer,
		mkErrParam
	};
	ComDispInterface * SLAPI InitDriver();
	int  SLAPI SetMKProp(PPID id, long lVal);
	int  SLAPI SetParam(long lVal);
	int  SLAPI SetParam(const char * pStrVal);
	int  SLAPI ExecMKOper(PPID id);
	ComDispInterface * P_DrvMassaK;
	int  ResCode;
	long WghtPrefix;
};

ComDispInterface * SLAPI COMMassaK::InitDriver()
{
	ComDispInterface * p_drv = 0;
	THROW_MEM(p_drv = new ComDispInterface);
	THROW(p_drv->Init("VP.Weigher.1"));
	THROW(ASSIGN_ID_BY_NAME(p_drv, Result) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, CurrentNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Initialize) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Finalize) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, SetBasicGoodsParams) > 0);
	CATCH
		ZDELETE(p_drv);
	ENDCATCH
	return p_drv;
}

int SLAPI COMMassaK::SetMKProp(PPID id, long lVal)
	{ return BIN(P_DrvMassaK && P_DrvMassaK->SetProperty(id, lVal) > 0); }
int SLAPI COMMassaK::SetParam(long lVal)
	{ return BIN(P_DrvMassaK && P_DrvMassaK->SetParam(lVal) > 0); }
int SLAPI COMMassaK::SetParam(const char * pStrVal)
	{ return BIN(P_DrvMassaK && P_DrvMassaK->SetParam(pStrVal) > 0); }

int SLAPI COMMassaK::ExecMKOper(PPID id)
{
	int    ok = BIN(P_DrvMassaK && P_DrvMassaK->CallMethod(id) > 0 && P_DrvMassaK->GetProperty(Result, &ResCode) > 0 &&  ResCode == mkErrOK);
	if(!ok) {
		if(ResCode == mkErrSynk || ResCode == mkErrIO)
			PPSetError(PPERR_SCALE_NOSYNC);
		else if(ResCode == mkErrInit || ResCode == mkErrNotReady)
			PPSetError(PPERR_SCALE_NOTREADY);
		else if(ResCode == mkErrAnswer)
			PPSetError(PPERR_SCALE_NOACK);
		else if(ResCode == mkErrParam)
			PPSetError(PPERR_SCALE_SEND);
	}
	return ok;
}

int SLAPI COMMassaK::SetConnection()
{
	int    ok = 1;
	long   log_num = 0;
	THROW(P_DrvMassaK = InitDriver());
	THROW(SetMKProp(CurrentNumber, Data.Rec.LogNum));
	THROW(SetParam(PortNo + 1));
	THROW(ExecMKOper(Initialize));
	CATCHZOK
	return ok;
}

int SLAPI COMMassaK::SendPLU(const ScalePLU * pScalePLU)
{
	int    ok = -1;
	if(pScalePLU) {
		long   lprice, wght_prefix = WghtPrefix;
		SString temp_buf;
		LDATE  expiry = pScalePLU->Expiry;
		THROW(SetParam(pScalePLU->GoodsNo)); // PLU
		THROW(SetParam(0L));                 // Kind (0 - �������, 1 - �������)
		SETIFZ(wght_prefix, 20 + pScalePLU->Barcode / 100000);
		THROW(SetParam(wght_prefix));        // GrpNo (use as WeightPrefix)
		THROW(SetParam(pScalePLU->Barcode)); // Barcode
		lprice = R0i(pScalePLU->Price * 100);
		THROW(SetParam(lprice));             // Price
		THROW(SetParam(0L));                 // TareWeight
		SETIFZ(expiry, plusdate(getcurdate_(), 365));
		THROW(SetParam(expiry.year() % 100));
		THROW(SetParam(expiry.month()));
		THROW(SetParam(expiry.day()));
		{
			char   goods_name[64], name_part_1[32], name_part_2[32];
			SBaseBuffer name_items[2];
			memzero(name_part_1, sizeof(name_part_1));
			memzero(name_part_2, sizeof(name_part_2));
			name_items[0].Set(name_part_1, 24);
			name_items[1].Set(name_part_2, 24);
			(temp_buf = pScalePLU->GoodsName).Transf(CTRANSF_INNER_TO_OUTER);
			STRNSCPY(goods_name, temp_buf);
			SplitString(goods_name, 2, name_items);
			THROW(SetParam(name_part_1));
			THROW(SetParam(name_part_2));
		}
		THROW(ExecMKOper(SetBasicGoodsParams));
		ok = 1;
	}
	CATCH
		ExecMKOper(Finalize);
		ok = 0;
	ENDCATCH
	return ok;
}

int SLAPI COMMassaK::CloseConnection()
{
	return ExecMKOper(Finalize);
}
//
//	COMMassaKVPN
//
class COMMassaKVPN : public PPScaleDevice {
public:
	SLAPI  COMMassaKVPN(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData), P_DbfTbl(0), P_Csv(0)
	{
		PPGoodsConfig goods_cfg;
		PPObjGoods::ReadConfig(&goods_cfg);
		WghtPrefix = atol(goods_cfg.WghtPrefix);
	}
	SLAPI ~COMMassaKVPN()
	{
		delete P_DbfTbl;
		delete P_Csv;
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI CloseConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
	}
private:
	int SLAPI CheckAckForPLU(); // @vmiller

	int    ResCode;
	long   WghtPrefix;
	DbfTable * P_DbfTbl;
	SFile * P_Csv;
};


int SLAPI COMMassaKVPN::SetConnection()
{
	int    ok = 1, num_flds = 0;
	SString path;
	if(Data.Rec.ProtocolVer == 11) {
		/*
		PLU;TYPE;LABEL_NUMBER;BARCODE_NUMBER;BARCODE_PREFIX;PRICE;TARE;CODE;BEST_BEFORE;SHELF_LIFE;CERTIFICATE;GROUP;CENTERING;NAME;CONTENT;INFO_TYPE;INFO
		00000110;1;1;1;0;11050;0;00000110;25.11.2011 16:30:00;0;;0;0;������� � ��������;;1;2000001914014
		*/
		SString line_buf;
		PPGetFilePath(PPPATH_OUT, "masakvpn.csv", path);
		ZDELETE(P_Csv);
		THROW(P_Csv = new SFile(path, SFile::mWrite));
		line_buf = "PLU;TYPE;LABEL_NUMBER;BARCODE_NUMBER;BARCODE_PREFIX;PRICE;TARE;CODE;BEST_BEFORE;SHELF_LIFE;CERTIFICATE;GROUP;CENTERING;NAME;CONTENT;INFO_TYPE;INFO";
		P_Csv->WriteLine(line_buf.CR());
	}
	else {
		DBFCreateFld fld_list[32];
		PPGetFilePath(PPPATH_OUT, "masakvpn.dbf", path);
		THROW(P_DbfTbl = new DbfTable(path));

		fld_list[num_flds++].Init("PRD_ID",   'C', 12, 0);
		fld_list[num_flds++].Init("PRD_NAME", 'C', 255, 0);
		fld_list[num_flds++].Init("FLG_CNTR", 'C', 1, 0);
		fld_list[num_flds++].Init("PRD_CODE", 'C', 32, 0);
		fld_list[num_flds++].Init("PRD_PLU",  'C', 8, 0);
		fld_list[num_flds++].Init("PRD_PREF", 'C', 2, 0);
		fld_list[num_flds++].Init("PRD_BAR",  'C', 13, 0);
		fld_list[num_flds++].Init("GRP_ID",   'C', 12, 0);
		fld_list[num_flds++].Init("GRP_NAME", 'C', 255, 0);
		fld_list[num_flds++].Init("MSR_ID",   'C', 3, 0);
		fld_list[num_flds++].Init("MSR_NAME", 'C', 25, 0);
		fld_list[num_flds++].Init("PRD_PRCE", 'N', 13, 2);
		fld_list[num_flds++].Init("PRD_CERT", 'C', 4, 0);
		fld_list[num_flds++].Init("PRD_CMP1", 'C', 255, 0);
		fld_list[num_flds++].Init("PRD_CMP2", 'C', 255, 0);
		fld_list[num_flds++].Init("PRD_TARE", 'C', 15, 0);
		fld_list[num_flds++].Init("PRD_LIFE", 'N', 16, 0);
		fld_list[num_flds++].Init("PRD_DATE", 'C', 16, 0);
		fld_list[num_flds++].Init("PRD_INFO", 'C', 255, 0);
		THROW(P_DbfTbl->create(num_flds, fld_list));
		THROW(P_DbfTbl->open());
	}
	CATCHZOK
	return ok;
}

int SLAPI COMMassaKVPN::CloseConnection()
{
	if(P_DbfTbl) {
		P_DbfTbl->close();
		ZDELETE(P_DbfTbl);
	}
	if(P_Csv) {
		ZDELETE(P_Csv);
	}
	return 1;
}

int SLAPI COMMassaKVPN::SendPLU(const ScalePLU * pScalePLU)
{
	int    ok = -1;
	if(pScalePLU) {
		SString goods_name, temp_buf;
		long   wght_prefix = WghtPrefix;
		long   expiry_minutes = 0;
		LDATE  expiry = pScalePLU->Expiry;
		LDATE  cur_dt;
		StringSet ss(SLBColumnDelim);

		(goods_name = pScalePLU->GoodsName);
		SETIFZ(wght_prefix, 20 + pScalePLU->Barcode / 100000);
		getcurdate(&cur_dt);
		expiry_minutes = (expiry > cur_dt) ? diffdate(expiry, cur_dt) : 0; // * 24 * 60;
		ss.setBuf(pScalePLU->AddMsgBuf, sstrlen(pScalePLU->AddMsgBuf)+1); // @v9.6.0 @fix (+1)

		if(P_DbfTbl) {
			DbfRecord dbf_rec(P_DbfTbl);
			dbf_rec.put(1, pScalePLU->GoodsID);              // PRD_ID
			dbf_rec.put(2, goods_name.Transf(CTRANSF_INNER_TO_OUTER).cptr()); // PRD_NAME
			dbf_rec.put(3, "0");                             // FLG_CNTR
			dbf_rec.put(4, pScalePLU->Barcode);              // PRD_CODE
			dbf_rec.put(5, pScalePLU->GoodsNo);              // PRD_PLU
			dbf_rec.put(6, wght_prefix);                     // PRD_PREF
			dbf_rec.put(7, "");                              // PRD_BAR
			dbf_rec.put(8, "");                              // GRP_ID
			dbf_rec.put(9, "");                              // GRP_NAME
			dbf_rec.put(10, "");                             // MSR_ID
			dbf_rec.put(11, "");                             // MSR_NAME
			dbf_rec.put(12, R2(pScalePLU->Price));           // PRD_PRCE
			dbf_rec.put(13, "");                             // PRD_CERT

			for(uint p = 0, j = 0; ss.get(&p, temp_buf.Z()) > 0 && j < 2; j++)
				dbf_rec.put(14 + j, temp_buf.Transf(CTRANSF_INNER_TO_OUTER).cptr());  // PRD_CMP1, PRD_CMP2
			dbf_rec.put(16, "0");                            // PRD_TARE
			dbf_rec.put(17, expiry_minutes);                 // PRD_LIFE
			dbf_rec.put(18, "");                             // PRD_DATE
			dbf_rec.put(19, "");                             // PRD_INFO
			P_DbfTbl->appendRec(&dbf_rec);
			ok = 1;
		}
		else if(P_Csv) {
		/*
		PLU     ;TYPE;LABEL_NUMBER;BARCODE_NUMBER;BARCODE_PREFIX;PRICE;TARE;    CODE;        BEST_BEFORE;SHELF_LIFE;CERTIFICATE;GROUP;CENTERING;              NAME;CONTENT;INFO_TYPE;INFO
		00000110;   1;           1;             1;             0;11050;   0;00000110;25.11.2011 16:30:00;         0;           ;    0;        0;������� � ��������;       ;        1;2000001914014
		*/
			SString line_buf;
			line_buf.Cat(pScalePLU->GoodsNo).Semicol();
			line_buf.CatChar('1').Semicol(); // type 1 - �������, 0 - �������
			line_buf.CatChar('1').Semicol(); // label_number
			line_buf.CatChar('1').Semicol(); // barcode_number
			line_buf.Cat(wght_prefix).Semicol(); // barcode_prefix
			line_buf.Cat(R2(pScalePLU->Price) * 100.0).Semicol(); // price
			line_buf.CatChar('0').Semicol(); // tare
			line_buf.CatChar(pScalePLU->GoodsID).Semicol(); // code
			temp_buf.Z();
			if(expiry_minutes)
				temp_buf.Cat(expiry, DATF_GERMAN|DATF_CENTURY).Space().Cat(encodetime(23, 0, 0, 0), TIMF_HMS);
			line_buf.Cat(temp_buf).Semicol(); // best_before
			line_buf.CatChar('0').Semicol(); // shelf_life
			line_buf.Semicol(); // certificate
			line_buf.CatChar('0').Semicol(); // group
			line_buf.CatChar('0').Semicol(); // CENTERING
			line_buf.Cat(goods_name.Transf(CTRANSF_INNER_TO_OUTER)).Semicol(); // NAME
			line_buf.Cat("").Semicol(); // CONTENT
			line_buf.CatChar('0').Semicol(); // INFO_TYPE
			line_buf.Semicol(); // INFO
			P_Csv->WriteLine(line_buf.CR());
		}
	}
	return ok;
}
//
// @vmiller {
//
//	COMMassaKVer1
//
class COMMassaKVer1 : public PPScaleDevice {
public:
	SLAPI  COMMassaKVer1(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData), P_DrvMassaK(0), ResCode(mkErrOK)
	{
	}
	SLAPI ~COMMassaKVer1()
	{
		delete P_DrvMassaK;
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *) { return 1; };
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
	}
	virtual int  SLAPI GetData(int * pGdsNo, double * pWeight);
	int SLAPI CloseConnection_();
private:
	enum {
		Connection,
		Weight,
		Division,
		Stable,
		OpenConnection,
		CloseConnection,
		ReadWeight
	};
	enum ResultCodes {
		mkErrOK = 0,
		mkErrSynk,
		mkErrIO,
		mkErrNotReady,
		mkErrParam,
		mkErrSetParam
	};
	ComDispInterface * SLAPI InitDriver();
	int  SLAPI SetMKProp(PPID id, long lVal);
	int  SLAPI SetMKProp(PPID id, const char * pStrVal);
	int  SLAPI SetParam(long lVal);
	int  SLAPI SetParam(const char * pStrVal);
	int  SLAPI GetMKProp(PPID id, int & rVal);
	int  SLAPI GetMKProp(PPID id, double & rVal);
	int  SLAPI ExecMKOper(PPID id);
	ComDispInterface * P_DrvMassaK;
	int  ResCode;
};

ComDispInterface * SLAPI COMMassaKVer1::InitDriver()
{
	ComDispInterface * p_drv = 0;
	THROW_MEM(p_drv = new ComDispInterface);
	THROW(p_drv->Init("ScalesMassaK.Scale"));
	THROW(ASSIGN_ID_BY_NAME(p_drv, Connection) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Weight) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Division) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Stable) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, OpenConnection) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, CloseConnection) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ReadWeight) > 0);
	CATCH
		ZDELETE(p_drv);
	ENDCATCH
	return p_drv;
}

int SLAPI COMMassaKVer1::SetMKProp(PPID id, long lVal)
	{ return BIN(P_DrvMassaK && P_DrvMassaK->SetProperty(id, lVal) > 0); }
int SLAPI COMMassaKVer1::SetMKProp(PPID id, const char * pStrVal)
	{ return BIN(P_DrvMassaK && P_DrvMassaK->SetProperty(id, pStrVal) > 0); }
int SLAPI COMMassaKVer1::SetParam(long lVal)
	{ return BIN(P_DrvMassaK && P_DrvMassaK->SetParam(lVal) > 0); }
int SLAPI COMMassaKVer1::SetParam(const char * pStrVal)
	{ return BIN(P_DrvMassaK && P_DrvMassaK->SetParam(pStrVal) > 0); }
int SLAPI COMMassaKVer1::GetMKProp(PPID id, int & rVal)
	{ return BIN(P_DrvMassaK && P_DrvMassaK->GetProperty(id, &rVal) > 0); }
int SLAPI COMMassaKVer1::GetMKProp(PPID id, double & rVal)
	{ return BIN(P_DrvMassaK && P_DrvMassaK->GetProperty(id, &rVal) > 0); }

int SLAPI COMMassaKVer1::ExecMKOper(PPID id)
{
	int    ok = 1;
	if(P_DrvMassaK) {
		ok = P_DrvMassaK->CallMethod(id);
		//if(!ok) {
		// � ���������� ��� ���������, ����������� �� ��� ������, ������� ����� ���������
		// ����������� ERROR, �� �� �� �������������� ��� �������������
		//if(ResCode != mkErrOK) {
			//if(ResCode == mkErrSynk)
			//	PPSetError(PPERR_SCALE_NOSYNC);
			//else if(ResCode == mkErrIO || ResCode == mkErrNotReady)
			//	PPSetError(PPERR_SCALE_NOTREADY);
			//else if(ResCode == mkErrParam || ResCode == mkErrSetParam)
			//	PPSetError(PPERR_SCALE_SEND);
			//ok = 0;
		//}

		// ������� �������� ������, ������ ��� ���� ��� ���������� ���������� �� ������� ok = 1,
		// � ���� �������� ������ � �������, ������� ����������� ����������� ��� ������ ����� ����� F5
		PPSetError(PPERR_SCALE_NOTREADY);
	}
	//else
	//	ok = 0;
	return ok;
}

int SLAPI COMMassaKVer1::SetConnection()
{
	int    ok = 1;
	long   log_num = 0;
	SString port_buf;
	//char   ip[16];
	THROW(P_DrvMassaK = InitDriver());
	//THROW(PPObjScale::DecodeIP(Data.Rec.Port, ip));
	Data.GetExtStrData(Data.extssPort, port_buf);
	THROW(SetMKProp(Connection, port_buf));
	THROW(ExecMKOper(OpenConnection));
	CATCHZOK
	return ok;
}

int SLAPI COMMassaKVer1::CloseConnection_()
{
	return ExecMKOper(CloseConnection);
}

int SLAPI COMMassaKVer1::GetData(int * pGdsNo, double * pWeight)
{
	int    ok = 1;
	int    division = 0, stable = 0;
	double weight = 0.0;
	//char   ip[16];
	SString port_buf;
	THROW(SetConnection());
	//THROW(PPObjScale::DecodeIP(Data.Rec.Port, ip));
	Data.GetExtStrData(Data.extssPort, port_buf);
	THROW(SetMKProp(Connection, port_buf));
	THROW(ExecMKOper(ReadWeight));
	THROW(GetMKProp(Weight, weight)); //
	THROW(GetMKProp(Division, division)); // 0 - ��, 1 - �, 2 - ��
	THROW(GetMKProp(Stable, stable)); // 0 - �� ��������, 1 - ��������
	if(division == 0) {
		ASSIGN_PTR(pWeight, weight / 1000000);
	}
	else if(division == 1) {
		ASSIGN_PTR(pWeight, weight / 1000);
	}
	else {
		ASSIGN_PTR(pWeight, weight);
	}
	CATCHZOK;
	return ok;
}
// }

//
// Mettler Toledo
//
static const char * TrfrDLLPath = "TransferEth.dll";
static const char * CrcDLLPath = "CrcModule.dll";

typedef unsigned (__stdcall * MT_EthernetProc)(char *);
typedef ushort (__cdecl * MT_CalcCrcProc)(const char *, uint);

class TCPIPMToledo : public PPScaleDevice {
public:
	SLAPI  TCPIPMToledo(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData), P_DrvMT(0), P_AddStrAry(0),
		P_PLUStream(0), TrfrDLLHandle(0), TrfrDLLCall(0), CalcCrcCall(0), SocketHandle(0), IsConnected(0)
	{
		UseNewAlg = (Data.Rec.ProtocolVer >= 100) ? 0 : 1;
		if(!(Data.Rec.Flags & SCALF_SYSPINITED))
			GetDefaultSysParams(&Data);
		SETIFZ(Data.Rec.LogNum, 1);
		SOemToChar(Data.Rec.Name);
	}
	SLAPI ~TCPIPMToledo()
	{
		SFile::ZClose(&P_PLUStream);
		delete P_DrvMT;
		delete P_AddStrAry;
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI CloseConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
		CALLPTRMEMB(pData, SetSysParams(0, 1000, 0, 1000));
	}
private:
	struct _AddStrEntry {
		long  AddStrCode;
		char  AddStr[512];
	};
	enum {
		ErrorCode,
		OpenScaleEth,
		WriteMessage
	};
    enum MTErrCodes{
        mtErrOK,
        mtErrSetPort,
        mtErrBadPar,
        mtErrWrite,
        mtErrRead,
        mtErrAnswer,
        mtErrCantDo,
        mtErrConnect
    };
	ComDispInterface * SLAPI InitDriver();
	int  SLAPI SetParam(long lVal);
	int  SLAPI SetParam(const char * pStrVal);
	int  SLAPI ExecMTOper(PPID id);
	int  SLAPI NewAlgSendPLU(const ScalePLU *);
	ComDispInterface * P_DrvMT;
	SArray * P_AddStrAry;
	FILE * P_PLUStream;
	HMODULE  TrfrDLLHandle;
	MT_EthernetProc TrfrDLLCall;
	MT_CalcCrcProc  CalcCrcCall;
	SOCKET SocketHandle;
	int    IsConnected;
	int    UseNewAlg;
};

int SLAPI TCPIPMToledo::SetConnection()
{
	int    ok = 1;
	//char   ip[16];
	SString port_buf;
	FILE * p_stream = 0;
	//memzero(ip, sizeof(ip));
	//THROW(PPObjScale::DecodeIP(Data.Rec.Port, ip));
	Data.GetExtStrData(Data.extssPort, port_buf);
	if(UseNewAlg) {
		char   send_timeout[10], rcv_timeout[10];
		long   res;
		struct sockaddr_in sin;
		THROW_PP((SocketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) != INVALID_SOCKET, PPERR_SCALE_NOSYNC);
		sin.sin_family           = AF_INET;
		sin.sin_port             = htons(3001);
		sin.sin_addr.S_un.S_addr = inet_addr(port_buf);
		memzero(send_timeout, sizeof(send_timeout));
		memzero(rcv_timeout,  sizeof(rcv_timeout));
		itoa(Data.Rec.Put_Delay, send_timeout, 10);
		itoa(Data.Rec.Get_Delay, rcv_timeout, 10);
		THROW_PP((res = setsockopt(SocketHandle, SOL_SOCKET, SO_SNDTIMEO, (const char *)send_timeout, sstrleni(send_timeout))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		THROW_PP((res = setsockopt(SocketHandle, SOL_SOCKET, SO_RCVTIMEO, (const char *)rcv_timeout, sstrleni(rcv_timeout))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		THROW_PP((res = connect(SocketHandle, reinterpret_cast<sockaddr *>(&sin), sizeof(sin))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		IsConnected = 1;
		///* @v10.4.12
		PPSetAddedMsgString(CrcDLLPath);
		if(!TrfrDLLHandle) {
			THROW_PP(TrfrDLLHandle = ::LoadLibrary(SUcSwitch(CrcDLLPath)), PPERR_SCALE_INITMTDLL); // @unicodeproblem
			THROW_PP(CalcCrcCall   = (MT_CalcCrcProc)GetProcAddress(TrfrDLLHandle, "?CalcCRC16@@YAGQBDI@Z"), PPERR_SCALE_INITMTDLL);
		} 
		//*/
	}
	else {
		SString buf;
		THROW_PP(PPObjScale::CheckForConnection(port_buf, 1000, 5), PPERR_SCALE_NOTREADY);
		SETIFZ(P_DrvMT, InitDriver());
		PPSetAddedMsgString(TrfrDLLPath);
		if(!TrfrDLLHandle) {
			THROW_PP(TrfrDLLHandle = ::LoadLibrary(SUcSwitch(TrfrDLLPath)), PPERR_SCALE_INITMTDLL); // @unicodeproblem
			THROW_PP(TrfrDLLCall   = (MT_EthernetProc)GetProcAddress(TrfrDLLHandle, "Transfer_Ethernet"), PPERR_SCALE_INITMTDLL);
		}
		PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_CFG, buf);
		PPSetAddedMsgString(buf);
		THROW_PP(p_stream = fopen(buf, "w"), PPERR_CANTOPENFILE);
		fprintf(p_stream, "[CONFIG]\nMEDIA=1\n");
		fprintf(p_stream, "COMPORT=2\n");
		fprintf(p_stream, "THREADNUM=1\n");
		fprintf(p_stream, "[%i]\nNAME=%s\n", (int)Data.Rec.LogNum, Data.Rec.Name);
		fprintf(p_stream, "IP=%s\n", port_buf.cptr());
		fprintf(p_stream, "PORT=3001");
		SFile::ZClose(&p_stream);
		PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_DATA, buf);
		PPSetAddedMsgString(buf);
		THROW_PP(P_PLUStream = fopen(buf, "w"), PPERR_CANTOPENFILE);
	}
	CATCH
		if(UseNewAlg) {
			shutdown(SocketHandle, 2);
			closesocket(SocketHandle);
			IsConnected = 0;
		}
		ok = 0;
	ENDCATCH
	SFile::ZClose(&p_stream);
	return ok;
}

int SLAPI TCPIPMToledo::CloseConnection()
{
	int     ok = -1;
	SString buf, path, out_path;
	FILE * p_stream = 0;
	if(UseNewAlg) {
		if(IsConnected) {
			shutdown(SocketHandle, 2);
			closesocket(SocketHandle);
			IsConnected = 0;
		}
	}
	else {
		if(P_PLUStream) {
			uint  r = 1;
			SFile::ZClose(&P_PLUStream);
			PPSetAddedMsgString(TrfrDLLPath);
			THROW_PP(TrfrDLLHandle && TrfrDLLCall, PPERR_SCALE_INITMTDLL);
			PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_OUT, out_path);
			PPSetAddedMsgString(out_path);
			THROW_PP(p_stream = fopen(out_path, "w"), PPERR_CANTOPENFILE);
			PPGetFileName(PPFILNAM_MTSCALE_DATA, path);
			fprintf(p_stream, "%s\n%i", path.cptr(), (int)Data.Rec.LogNum);
			SFile::ZClose(&p_stream);
			{
				char   temp_path[512];
				out_path.CopyTo(temp_path, sizeof(temp_path));
				THROW((r = TrfrDLLCall(temp_path)) != 0);
			}
			SFile::Remove(out_path);
			PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_CFG, path);
			SFile::Remove(path);
			PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_DATA, path);
			SFile::Remove(path);
			PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_LOG, path);
			SFile::Remove(path);
			PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_BINLZ, path);
			SFile::Remove(path);
			PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_BINOUT, path);
			SFile::Remove(path);
			PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_LOG2, path);
			SFile::Remove(buf.Printf(path, Data.Rec.LogNum));
			PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_TRFIN, path);
			SFile::Remove(path);
			PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_TRFIN2, path);
			SFile::Remove(buf.Printf(path, Data.Rec.LogNum));
			PPGetFilePath(PPPATH_BIN, PPFILNAM_MTSCALE_SCPLU, path);
			SFile::Remove(buf.Printf(path, Data.Rec.LogNum));
		}
		if(P_DrvMT && P_AddStrAry) {
			SString port_buf;
			//char   ip[16];
			//memzero(ip, sizeof(ip));
			//THROW(PPObjScale::DecodeIP(Data.Rec.Port, ip));
			Data.GetExtStrData(Data.extssPort, port_buf);
			THROW(SetParam(port_buf));
			THROW(SetParam(3001L));
			THROW(ExecMTOper(OpenScaleEth));
			for(uint p = 0; p < P_AddStrAry->getCount(); p++) {
				_AddStrEntry  as_e = *static_cast<const _AddStrEntry *>(P_AddStrAry->at(p));
				THROW(SetParam(as_e.AddStrCode));
				THROW(SetParam(as_e.AddStr));
				THROW(ExecMTOper(WriteMessage));
			}
		}
	}
	CATCHZOK
	if(TrfrDLLHandle) {
		SDelay(500);
		FreeLibrary(TrfrDLLHandle);
		TrfrDLLHandle = 0;
	}
	SFile::ZClose(&p_stream);
	ZDELETE(P_DrvMT);
	ZDELETE(P_AddStrAry);
	return ok;
}

int SLAPI TCPIPMToledo::NewAlgSendPLU(const ScalePLU * pPLU)
{
	struct _PacketHeader {
		uchar  StartByte;   // = 0x02
		ushort TotalLength; // = sizeof(CommandHeader) + PageNumber * PageLength
		ushort PageNumber;  // = PLU count
		ushort PageLength;  // = PLU entry length
	};
	struct _CommandHeader {
		uchar  Action;      // = 0 - send
		ushort WhatIsIt;    // = 207 - PLU, 209 - Added Message
		ushort WhatToDo;    // = 0 - to write
		ushort Department;  // = 1
		uchar  ScaleNumber; // = 0 - not use
	};

	const  int16 _NonWeightPriceFlag = 0x0001;
	const  int use_ext_messages = (Data.Rec.ProtocolVer >= 10) ? 1 : 0;
	const  int protocol_ver = (Data.Rec.ProtocolVer % 10);
	int    ok = 1;
	int    is_msg = pPLU->HasAddedMsg();
	char   reply;
	uchar  data_buf[512];
	ushort crc;
	long   numdays = diffdate(pPLU->Expiry, getcurdate_());
	size_t p;
	SString  barcode;
	_PacketHeader  pack_head;
	_CommandHeader comm_head;
	if(numdays <= 0 || numdays > 499)
		numdays = 499;
	if(protocol_ver == 2) {
		struct _Protocol2PLUEntry {
			long   PLUNumber;
			char   Barcode[13];
			char   GoodsName1[30];
			char   GoodsName2[30];
			char   Empty1;
			long   Price;
			char   Empty2[10];
			int16  Flags;
			ushort Expiry;
			char   Empty3[2];
			ushort MsgNo;
		} p2_entry;

		char   goods_name[64], name_part1[32], name_part2[32];
		SBaseBuffer name_items[2];
		name_items[0].Set(name_part1, 28);
		name_items[1].Set(name_part2, 28);
		STRNSCPY(goods_name, pPLU->GoodsName);
		SplitString(goods_name, 2, name_items);

		pack_head.StartByte   = 0x02;
		pack_head.TotalLength = (ushort)(sizeof(_CommandHeader) + sizeof(_Protocol2PLUEntry));
		pack_head.PageNumber  = 1;
		pack_head.PageLength  = (ushort)sizeof(_Protocol2PLUEntry);

		comm_head.Action      = 0;
		comm_head.WhatIsIt    = 207;
		comm_head.WhatToDo    = 0;
		comm_head.Department  = 1;
		comm_head.ScaleNumber = 0;

		MEMSZERO(p2_entry);
		p2_entry.PLUNumber = pPLU->GoodsNo;
		barcode.CatLongZ(pPLU->Barcode, 13).CopyTo(p2_entry.Barcode, sizeof(p2_entry.Barcode) + 1);
		STRNSCPY(p2_entry.GoodsName1, name_part1);
		if(name_part2[0])
			STRNSCPY(p2_entry.GoodsName2, name_part2);
		p2_entry.Price  = R0i(pPLU->Price * 100);
		if(pPLU->Flags & ScalePLU::fCountable)
			p2_entry.Flags |= _NonWeightPriceFlag;
		p2_entry.Expiry = (ushort)numdays;
		p2_entry.MsgNo  = is_msg ? (ushort)pPLU->GoodsNo : 0;

		memcpy(data_buf, &pack_head, sizeof(pack_head));
		p  = sizeof(pack_head);
		memcpy(data_buf + p, &comm_head, sizeof(comm_head));
		p += sizeof(comm_head);
		memcpy(data_buf + p, &p2_entry, sizeof(p2_entry));
		p += sizeof(p2_entry);
		crc = CalcCrcCall((const char *)data_buf + 1, (int)(p - 1));
		*(data_buf + p++) = PTR8C(&crc)[1];
		*(data_buf + p++) = PTR8C(&crc)[0];
		THROW_PP(send(SocketHandle, (const char *)data_buf, (int)p, 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
		THROW_PP(recv(SocketHandle, &reply, 1, 0) != SOCKET_ERROR && reply == 0x06, PPERR_SCALE_RCV);
	}
	else {
		struct _Protocol1PLUEntry {
			long   PLUNumber;
			char   Barcode[13];
			char   GoodsName[28];
			char   Empty1;
			long   Price;
			char   Empty2[10];
			int16  Flags;
			ushort Expiry;
			char   Empty3[2];
			ushort MsgNo;
		} p1_entry;

		pack_head.StartByte   = 0x02;
		pack_head.TotalLength = (ushort)(sizeof(_CommandHeader) + sizeof(_Protocol1PLUEntry));
		pack_head.PageNumber  = 1;
		pack_head.PageLength  = (ushort)sizeof(_Protocol1PLUEntry);

		comm_head.Action      = 0;
		comm_head.WhatIsIt    = 207;
		comm_head.WhatToDo    = 0;
		comm_head.Department  = 1;
		comm_head.ScaleNumber = 0;

		MEMSZERO(p1_entry);
		p1_entry.PLUNumber = pPLU->GoodsNo;
		barcode.CatLongZ(pPLU->Barcode, 13).CopyTo(p1_entry.Barcode, sizeof(p1_entry.Barcode) + 1);
		STRNSCPY(p1_entry.GoodsName, pPLU->GoodsName);
		p1_entry.Price  = R0i(pPLU->Price * 100);
		if(pPLU->Flags & ScalePLU::fCountable)
			p1_entry.Flags |= _NonWeightPriceFlag;
		p1_entry.Expiry = (ushort)numdays;
		p1_entry.MsgNo  = is_msg ? (ushort)pPLU->GoodsNo : 0;

		memcpy(data_buf, &pack_head, sizeof(pack_head));
		p  = sizeof(pack_head);
		memcpy(data_buf + p, &comm_head, sizeof(comm_head));
		p += sizeof(comm_head);
		memcpy(data_buf + p, &p1_entry, sizeof(p1_entry));
		p += sizeof(p1_entry);
		crc = CalcCrcCall((const char *)data_buf + 1, (int)(p - 1));
		*(data_buf + p++) = PTR8C(&crc)[1];
		*(data_buf + p++) = PTR8C(&crc)[0];
		THROW_PP(send(SocketHandle, (const char *)data_buf, (int)p, 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
		THROW_PP(recv(SocketHandle, &reply, 1, 0) != SOCKET_ERROR && reply == 0x06, PPERR_SCALE_RCV);
	}
	if(is_msg) {
		const size_t max_text = 200; 
		struct _AddStr {
			ushort  AddStrCode;
			char    AddStrTxt[max_text];
		} as_entry;
		uint8 stk_pad[64]; // @padding @v10.5.1
		memzero(stk_pad, sizeof(stk_pad));
		//const size_t _add_str_size = sizeof(ushort) + max_text;
		StringSet add_str("\x0A");
		GetAddedMsgLines(pPLU, NZOR(Data.Rec.MaxAddedLn, 50), max_text, amlfMaxText, add_str);
		//
		pack_head.StartByte   = 0x02;
		pack_head.TotalLength = (ushort)(sizeof(_CommandHeader) + sizeof(_AddStr));
		pack_head.PageNumber  = 1;
		pack_head.PageLength  = (ushort)sizeof(_AddStr);

		comm_head.Action      = 0;
		comm_head.WhatIsIt    = 209;
		comm_head.WhatToDo    = 0;
		comm_head.Department  = 1;
		comm_head.ScaleNumber = 0;

		MEMSZERO(as_entry);
		as_entry.AddStrCode = (ushort)pPLU->GoodsNo;
		{
			const char * p_full_buf = add_str.getBuf();
			const size_t full_buf_len = sstrlen(p_full_buf);
			if(use_ext_messages && full_buf_len > max_text) {
				size_t _text_buf_offs = 0;
				ushort _plu = (ushort)pPLU->GoodsNo;
				uint   _n = 0;
				const uint _max_n = 4;
				SString _plu_text;
				do {
					_n++;
					MEMSZERO(as_entry);
					as_entry.AddStrCode = _plu;
					const size_t part_len = sstrlen(p_full_buf + _text_buf_offs);
					if(_n < _max_n && part_len <= max_text) {
						STRNSCPY(as_entry.AddStrTxt, p_full_buf + _text_buf_offs);
						_text_buf_offs += part_len;
					}
					else {
						const size_t ext_plu_len = 4;
						const size_t ext_size = ext_plu_len + 2;
						_plu = (ushort)pPLU->GoodsNo + (2000 * _n);
						_plu_text.Z().CatLongZ((long)_plu, ext_plu_len);
						if(_plu_text.Len() == 4) {
							size_t tp = 0;
							as_entry.AddStrTxt[tp++] = 15;
							as_entry.AddStrTxt[tp++] = '}';
							memcpy(as_entry.AddStrTxt+tp, _plu_text.cptr(), ext_plu_len);
							tp += ext_plu_len;
							strnzcpy(as_entry.AddStrTxt+tp, p_full_buf + _text_buf_offs, max_text-tp); // \x15}xxxx
							_text_buf_offs += (max_text - ext_size);
						}
						else {
							strnzcpy(as_entry.AddStrTxt, p_full_buf + _text_buf_offs, max_text);
							const size_t ast_len = sstrlen(as_entry.AddStrTxt);
							_text_buf_offs = full_buf_len; // @exit
						}
					}
					{
						memcpy(data_buf, &pack_head, sizeof(pack_head));
						p  = sizeof(pack_head);
						memcpy(data_buf + p, &comm_head, sizeof(comm_head));
						p += sizeof(comm_head);
						memcpy(data_buf + p, &as_entry, sizeof(as_entry));
						p += sizeof(as_entry);
						crc = CalcCrcCall(reinterpret_cast<const char *>(data_buf) + 1, (int)(p - 1));
						*(data_buf + p++) = reinterpret_cast<const uchar *>(&crc)[1];
						*(data_buf + p++) = reinterpret_cast<const uchar *>(&crc)[0];
						THROW_PP(send(SocketHandle, (const char *)data_buf, (int)p, 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
						THROW_PP(recv(SocketHandle, &reply, 1, 0) != SOCKET_ERROR && reply == 0x06, PPERR_SCALE_RCV);
					}
				} while(_text_buf_offs < full_buf_len && _n < _max_n);
			}
			else {
				size_t spc_offs = 0;
				//as_entry.AddStrTxt[spc_offs++] = 15;
				//as_entry.AddStrTxt[spc_offs++] = '{';
				strnzcpy(as_entry.AddStrTxt+spc_offs, p_full_buf, sizeof(as_entry.AddStrTxt)-spc_offs);
				{
					memcpy(data_buf, &pack_head, sizeof(pack_head));
					p  = sizeof(pack_head);
					memcpy(data_buf + p, &comm_head, sizeof(comm_head));
					p += sizeof(comm_head);
					memcpy(data_buf + p, &as_entry, sizeof(as_entry));
					p += sizeof(as_entry);
					crc = CalcCrcCall((const char *)data_buf + 1, (int)(p - 1));
					*(data_buf + p++) = PTR8C(&crc)[1];
					*(data_buf + p++) = PTR8C(&crc)[0];
					THROW_PP(send(SocketHandle, (const char *)data_buf, (int)p, 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
					THROW_PP(recv(SocketHandle, &reply, 1, 0) != SOCKET_ERROR && reply == 0x06, PPERR_SCALE_RCV);
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI TCPIPMToledo::SendPLU(const ScalePLU * pPLU)
{
	int    ok = 1;
	if(UseNewAlg) {
		THROW(NewAlgSendPLU(pPLU));
	}
	else {
		int    protocol = Data.Rec.ProtocolVer - 100;
		int    is_msg = 0;
		char   buf[512];
		long   numdays = diffdate(pPLU->Expiry, getcurdate_());
		if(numdays <= 0 || numdays > 499)
			numdays = 499;
		if(P_DrvMT && pPLU->HasAddedMsg()) {
			is_msg = 1;
			if(!P_AddStrAry)
				THROW_MEM(P_AddStrAry = new SArray(sizeof(_AddStrEntry)));
		}
		if(protocol == 2) {
			char  goods_name[64], name_part1[32], name_part2[32];
			SBaseBuffer name_items[2];
			name_items[0].Set(name_part1, 28);
			name_items[1].Set(name_part2, 28);
			STRNSCPY(goods_name, pPLU->GoodsName);
			SplitString(goods_name, 2, name_items);
			if(name_part2[0] == 0)
				strcpy(name_part2, " ");
			sprintf(buf, "%ld,%ld,0,%.2lf,0,%ld,0,0,%ld,0,0,0,0,%s,%s\n", pPLU->GoodsNo, pPLU->Barcode,
				pPLU->Price, is_msg ? pPLU->GoodsNo : 0, numdays, name_part1, name_part2);
			fprintf(P_PLUStream, buf);
		}
		else {
			sprintf(buf, "%ld,%ld,0,%.2lf,0,%ld,0,0,%ld,0,0,0,0,%s\n", pPLU->GoodsNo, pPLU->Barcode,
				pPLU->Price, is_msg ? pPLU->GoodsNo : 0, numdays, pPLU->GoodsName.cptr());
			fprintf(P_PLUStream, buf);
		}
		if(is_msg) {
			const size_t max_text = 300;
			_AddStrEntry  as_e;
			StringSet  add_str("\x0A");
			GetAddedMsgLines(pPLU, NZOR(Data.Rec.MaxAddedLn, 50), max_text, amlfMaxText, add_str);
			as_e.AddStrCode = pPLU->GoodsNo;
			STRNSCPY(as_e.AddStr, add_str.getBuf());
			SOemToChar(as_e.AddStr);
			THROW_SL(P_AddStrAry->insert(&as_e));
		}
	}
	CATCHZOK
	return ok;
}

ComDispInterface * SLAPI TCPIPMToledo::InitDriver()
{
	ComDispInterface * p_drv = 0;
	THROW_MEM(p_drv = new ComDispInterface);
	THROW(p_drv->Init("Tiger.ScaleTiger"));
	THROW(ASSIGN_ID_BY_NAME(p_drv, ErrorCode) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, OpenScaleEth) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, WriteMessage) > 0);
	CATCH
		ZDELETE(p_drv);
	ENDCATCH
	return p_drv;
}

int SLAPI TCPIPMToledo::SetParam(long lVal)
{
	return BIN(P_DrvMT && P_DrvMT->SetParam(lVal) > 0);
}

int SLAPI TCPIPMToledo::SetParam(const char * pStrVal)
{
	return BIN(P_DrvMT && P_DrvMT->SetParam(pStrVal) > 0);
}

int SLAPI TCPIPMToledo::ExecMTOper(PPID id)
{
	int  err_code = mtErrOK;
	int  ok = BIN(P_DrvMT && P_DrvMT->CallMethod(id) > 0 &&
		P_DrvMT->GetProperty(ErrorCode, &err_code) > 0 && err_code == mtErrOK);
	if(!ok) {
		if(err_code == mtErrConnect)
			PPSetError(PPERR_SCALE_NOTREGMODE);
		else if(err_code == mtErrBadPar || err_code == mtErrSetPort || err_code == mtErrCantDo)
			PPSetError(PPERR_SCALE_SEND);
		else if(err_code == mtErrWrite || err_code == mtErrRead)
			PPSetError(PPERR_SCALE_NOACK);
		else if(err_code == mtErrAnswer)
			PPSetError(PPERR_SCALE_NOSYNC);
	}
	return ok;
}
//
//	�������� ������ ��������
//
class CrystalCashServer : public PPScaleDevice {
public:
	SLAPI  CrystalCashServer(const PPScalePacket * pData) :
		PPScaleDevice(0, pData), P_OutTblScale(0), P_AddStrAry(0)
	{
		//ExpPaths = pExpPaths;
		PPObjGoods::ReadConfig(&GCfg);
	}
	SLAPI ~CrystalCashServer()
	{
		delete P_OutTblScale;
		delete P_AddStrAry;
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI CloseConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
	}
private:
	struct _AddStrEntry {
		long  AddStrCode;
		char  AddStr[256];
	};

	SString ScalePath;
	SString FlagPath;
	SArray * P_AddStrAry;
	PPGoodsConfig GCfg;
	DbfTable * P_OutTblScale;
};

int SLAPI CrystalCashServer::SetConnection()
{
	int    ok = 1;
	THROW(PPGetFilePath(PPPATH_OUT, PPFILNAM_CS_SCALE_DBF, ScalePath));
	THROW(PPGetFilePath(PPPATH_OUT, PPFILNAM_CS_SCALE_UPD, FlagPath));
	{
		FILE * ff = fopen(FlagPath, "w");
		SFile::ZClose(&ff);
	}
	ZDELETE(P_OutTblScale);
	THROW(P_OutTblScale = CreateDbfTable(DBFS_CRCS_SCALE_EXPORT, ScalePath, 1));
	CATCH
		ZDELETE(P_OutTblScale);
		ok = 0;
	ENDCATCH
	return ok;
}

int SLAPI CrystalCashServer::SendPLU(const ScalePLU * pScalePLU)
{
	int    ok = -1;
	if(P_OutTblScale) {
		int    is_msg = 0;
		SString head, tail;
		DbfRecord dbfrS(P_OutTblScale);
		if(pScalePLU->HasAddedMsg()) {
			is_msg = 1;
			if(!P_AddStrAry)
				THROW_MEM(P_AddStrAry = new SArray(sizeof(_AddStrEntry)));
		}

		dbfrS.empty();
		dbfrS.put(1, Data.Rec.LogNum);
		dbfrS.put(2, head.Z().Cat(pScalePLU->GoodsID));
		head.Z().Cat(GCfg.WghtPrefix).CatLongZ(pScalePLU->Barcode % 100000, 5);
		dbfrS.put(3, head);
		dbfrS.put(4, GCfg.WghtPrefix);
		{
			if(Data.Rec.MaxAddedLn > 0 && Data.Rec.MaxAddedLn < 64 && pScalePLU->GoodsName.Wrap(Data.Rec.MaxAddedLn, head, tail) > 0) {
				dbfrS.put(5, head);
				dbfrS.put(6, tail);
			}
			else
				dbfrS.put(5, pScalePLU->GoodsName);
		}
		dbfrS.put(7, pScalePLU->GoodsNo);
		dbfrS.put(8, pScalePLU->Price);
		if(pScalePLU->Expiry != ZERODATE) {
			long   expiry = diffdate(pScalePLU->Expiry, getcurdate_());
			if(expiry > 0)
				dbfrS.put(9, expiry);
		}
		dbfrS.put(10, 1);      // ����� ������ (������ 1)
		dbfrS.put(11, 1);      // ������� �������� ������
		THROW_PP(P_OutTblScale->appendRec(&dbfrS), PPERR_DBFWRFAULT);
		if(is_msg) {
			const size_t max_text = 255;
			StringSet  add_str("\x0A");
			GetAddedMsgLines(pScalePLU, NZOR(Data.Rec.MaxAddedLn, 50), max_text, amlfMaxText, add_str);
			// as_e.AddStrCode = pPLU->GoodsNo;
			// STRNSCPY(as_e.AddStr, add_str.getBuf());
		}
	}
	ok = 1;
	CATCH
		ZDELETE(P_OutTblScale);
		ok = 0;
	ENDCATCH
	return ok;
}

int SLAPI CrystalCashServer::CloseConnection()
{
	int    ok = 1;
	SString add_msg_path;
	DbfTable * p_msg_tbl = 0;
	//
	// �������� ���. ���������� �� ������
	//
	/* ���� ��������� ��� �������
	if(P_AddStrAry) {
		THROW(PPGetFilePath(PPPATH_OUT, PPFILNAM_CS_SCALE_ADDMSGDBF, add_msg_path, sizeof(add_msg_path)));
		THROW(p_msg_tbl = CreateDbfTable(DBFS_CRCS_SCALE_EXPORT, add_msg_path, 1));

		for(uint p = 0; p < P_AddStrAry->getCount(); p++) {
			DbfRecord dbfr(p_msg_tbl);
			_AddStrEntry  as_e = *(_AddStrEntry *)(P_AddStrAry->at(p));
			dbfr.put(1, as_e.AddStrCode);
			dbfr.put(2, as_e.AddStrText);
			THROW_PP(p_msg_tbl->appendRec(&dbfr), PPERR_DBFWRFAULT);
		}
	}
	*/
	THROW(DistributeFile(FlagPath,  0));
	THROW(DistributeFile(ScalePath, 0));
	/* ���� ��������� ��� �������
	if(add_msg_path.Len())
		THROW(DistributeFile(add_msg_path, 0));
	*/
	THROW(DistributeFile(FlagPath,  1));
	CATCHZOK
	ZDELETE(P_OutTblScale);
	ZDELETE(P_AddStrAry);
	ZDELETE(p_msg_tbl);
	return ok;
}
//
//
//
class WeightTerm : public PPScaleDevice {
public:
	SLAPI  WeightTerm(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData)
	{
		if(!(Data.Rec.Flags & SCALF_SYSPINITED))
			GetDefaultSysParams(&Data);
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *) { return -1; }
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData);
	virtual int  SLAPI GetData(int * pGdsNo, double * pWeight);
};

int SLAPI WeightTerm::SetConnection()
{
	InitPort(PortNo);
	return 1;
}

void SLAPI WeightTerm::GetDefaultSysParams(PPScalePacket * pData)
{
	CALLPTRMEMB(pData, SetSysParams(100, 500, 100, 50));
}

int SLAPI WeightTerm::GetData(int * pGdsNo, double * pWeight)
{
	int    ok = -1, gds_no = 0;
	double weight = 0.0;
	if(CheckSync(0x18, 0xFF)) {
		int i;
		char buf[10];
		THROW(PutChr(0x11));
		THROW(buf[0] = GetChr());
		if(buf[0] >= 0x31 && buf[0] <= 0x39)
			gds_no = buf[0] - 0x30;
		THROW(CheckSync(0x19, 0xFF));
		THROW(PutChr(0x10));
		for(i = 0; i < 9; i++)
			THROW(buf[i] = GetChr());
		if(buf[0] == '=') {
			buf[8] = 0;
			strtodoub(buf + 1, &weight);
		}
		THROW(PutChr(0x02));
		ok = 1;
	}
	CATCHZOK
	ASSIGN_PTR(pGdsNo,  gds_no);
	ASSIGN_PTR(pWeight, weight);
	return ok;
}
//
//	DIGI
//
class DIGI : public PPScaleDevice {
public:
	SLAPI  DIGI(const PPScalePacket * pData) : PPScaleDevice(0, pData), P_ScaleData(0), IntGrpCode(0), SocketHandle(NULL), Connected(0)
	{
		// @v10.5.7 ExpPaths = pExpPaths;
	}
	SLAPI ~DIGI()
	{
		SFile::ZClose(&P_ScaleData);
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI CloseConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
		if(Data.Rec.Flags & SCALF_TCPIP)
			CALLPTRMEMB(pData, SetSysParams(0, 15000, 0, 15000));
	}
private:
	int    SLAPI ConvertDIGI_Text(const char * pSrcName, uchar fontSize, uint lineLen, uint maxLines, SString & rDestName);

	long   IntGrpCode;
	SString ScalePath;
	FILE * P_ScaleData;
	int    Connected;
	SOCKET SocketHandle;
};

int SLAPI DIGI::SetConnection()
{
	int    ok = 1;
	if(!(Data.Rec.Flags & SCALF_TCPIP)) {
		SString out_rec, scale_name;
		PPObjGoodsGroup gg_obj;
		//THROW(PPGetFileName(PPFILNAM_DIGI_IMPORT_DAT, fname));
		SString fname;
		THROW_PP(Data.Rec.LogNum > 0 && Data.Rec.LogNum < 255, PPERR_SCALE_INVLOGNUM);
		(fname = "dgimp").CatLongZ(Data.Rec.LogNum, 3).Dot().Cat("dat");
		PPGetFilePath(PPPATH_OUT, fname, ScalePath);
		THROW_PP(P_ScaleData = fopen(ScalePath, "w"), PPERR_CANTOPENFILE);
		IntGrpCode = Data.Rec.ID % 10000L;
		scale_name = Data.Rec.Name;
		scale_name.Trim(30);
		out_rec.Printf("B%04ld0001%-30s\n", IntGrpCode, scale_name.cptr());
		fputs(out_rec.Transf(CTRANSF_INNER_TO_OUTER), P_ScaleData);
	}
	else {
		char   send_timeout[10], rcv_timeout[10];
		//char   ip[16];
		SString port_buf;
		struct sockaddr_in sin;
		THROW_PP((SocketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) != INVALID_SOCKET, PPERR_SCALE_NOSYNC);
		//memzero(ip, sizeof(ip));
		//THROW(PPObjScale::DecodeIP(Data.Rec.Port, ip));
		Data.GetExtStrData(Data.extssPort, port_buf);
		sin.sin_family = AF_INET;
		sin.sin_port   = htons(2000 + (uint16)Data.Rec.LogNum);
		sin.sin_addr.S_un.S_addr = inet_addr(port_buf);
		memzero(send_timeout, sizeof(send_timeout));
		memzero(rcv_timeout,  sizeof(rcv_timeout));
		itoa(Data.Rec.Put_Delay, send_timeout, 10);
		itoa(Data.Rec.Get_Delay, rcv_timeout, 10);
		THROW_PP(setsockopt(SocketHandle, SOL_SOCKET, SO_SNDTIMEO, (const char *)send_timeout, sstrleni(send_timeout)) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		THROW_PP(setsockopt(SocketHandle, SOL_SOCKET, SO_RCVTIMEO, (const char *)rcv_timeout,  sstrleni(rcv_timeout)) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		THROW_PP(connect(SocketHandle, reinterpret_cast<const sockaddr *>(&sin), sizeof(sin)) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		Connected = 1;
	}
	CATCH
		if(Data.Rec.Flags & SCALF_TCPIP) {
			shutdown(SocketHandle, 2);
			closesocket(SocketHandle);
		}
		else
			SFile::ZClose(&P_ScaleData);
		ok = 0;
	ENDCATCH
	return ok;
}

static int FASTCALL LongToHexBytesStr(long val, int prec, SString & rBuf)
{
	int    ok = 1;
	rBuf.Z();
	SString temp_buf;
	SString fmt;
	if(prec)
		fmt.Printf("%%0%dX", prec * 2);
	else
		fmt.Cat("%X");
	const size_t len = temp_buf.Printf(fmt.cptr(), val).Len();
	const char * p_buf = temp_buf.cptr();
	for(uint i = 0; i < len; i += 2) {
		int    v1 = hex(p_buf[i]);
		int    v2 = hex(p_buf[i+1]);
		rBuf.CatCharN(v1 * 16 + v2, 1);
	}
	return ok;
}

int FASTCALL LongToBCDStr(long val, const char * pFmt, SString & rBuf)
{
	rBuf.Z();
	SString temp_buf;
	const char * p_buf = temp_buf.Printf(pFmt, val).cptr();
	for(uint i = 0; i < temp_buf.Len(); i += 2) {
		int v1 = hex(p_buf[i]);
		int v2 = hex(p_buf[i + 1]);
		rBuf.CatCharN(v1 * 16 + v2, 1);
	}
	return 1;
}

int SLAPI DIGI::ConvertDIGI_Text(const char * pSrcName, uchar fontSize, uint lineLen, uint maxLines, SString & rDestName)
{
	const int line_len_limit = oneof2(Data.Rec.ProtocolVer, 500, 503) ? 48 : 25;
	if(lineLen == 0 || lineLen > 100)
		lineLen = line_len_limit;
	if(maxLines == 0 || maxLines > 32)
		maxLines = 4;
	SETIFZ(fontSize, 0x07);
	uint   j = 0;
	char   org_goods_name[1024];
	strip(STRNSCPY(org_goods_name, pSrcName));
	size_t name_len = sstrlen(org_goods_name);
	if(name_len > (lineLen * maxLines)) {
		org_goods_name[lineLen * maxLines] = 0;
		name_len = sstrlen(strip(org_goods_name));
	}
	if(org_goods_name[name_len-1] == '%')
		org_goods_name[name_len-1] = '.';
	uint   name_items_count = (uint)(name_len / lineLen + BIN(name_len % lineLen));
	SString str_len;
	if(name_items_count > 1) {
		SETMIN(name_items_count, maxLines);
		char   name_part[32][128];
		SBaseBuffer name_items[32];
		for(j = 0; j < name_items_count; j++) {
			name_part[j][0] = 0;
			name_items[j].Set(name_part[j], lineLen);
		}
		SplitString(org_goods_name, name_items_count, name_items);
		for(j = 0; j < name_items_count; j++) {
			rDestName.CatChar(fontSize); // ������ ������
			const long len = sstrleni(name_items[j].P_Buf);
			LongToHexBytesStr(len, 1, str_len);
			rDestName.CatChar(str_len.C(0)); // ����� ������ ��� ���������� � ������������
			rDestName.Cat(name_items[j].P_Buf).CatChar((j < (name_items_count-1)) ? 13 : 12);
		}
	}
	else {
		rDestName.CatChar(fontSize); // ������ ������
		long   len = sstrleni(org_goods_name);
		LongToHexBytesStr(len, 1, str_len);
		rDestName.CatChar(str_len.C(0)); // ����� ������ ��� ���������� � ������������
		rDestName.Cat(org_goods_name).CatChar(12);
	}
	rDestName.ReplaceChar((char)239, (char)159);
	return 1;
}

int SLAPI DIGI::SendPLU(const ScalePLU * pScalePLU)
{
	int    ok = -1;
	long   expiry = 0;
	if(pScalePLU && pScalePLU->Expiry) {
		const long   _e = diffdate(pScalePLU->Expiry, getcurdate_());
		expiry = (_e < 0 || _e >= 1000) ? 0 : _e;
	}
	if(!(Data.Rec.Flags & SCALF_TCPIP)) {
		SString out_rec;
		struct _DIGI_Imp_Fmt {
			char HeaderA;      // ��������� "A" - ������� �������� ������
			char Barcode[12];  // �������� - 2CCCCCC00000
			char PLU[15];      // PLU (��� GoodsID), ����� - ����
			char Div[4];       // ����� ������ - 0000
			char GdsGrp[4];    // ����� �������� ������ - 0000
			char GdsType;      // ��� ������ - 0 - �������
			char PriceBase;    // ������� ���� - 0 - 1��, 1 - 100�
			char Price[8];     // ��������� ���� - CCCCC.CC
			char Expiry[3];    // ���� ��������, ��� - CCC
			char TareNo[4];    // ����� ���� - 0000
			char GdsName[81];  // ������������ ������ (����������� 80 ��������) + 0x00
		} /*dif*/;
		if(P_ScaleData && pScalePLU) {
			out_rec.Printf("A%-07ld00000%015ld0001%04ld00%08.2lf%03ld0000%-80s\n",
				pScalePLU->Barcode + 2000000L, pScalePLU->GoodsNo, IntGrpCode, pScalePLU->Price, expiry, pScalePLU->GoodsName.cptr());
			fputs(out_rec.Transf(CTRANSF_INNER_TO_OUTER), P_ScaleData);
			ok = 1;
		}
	}
	else if(Connected) {
		const int line_len_limit = oneof2(Data.Rec.ProtocolVer, 500, 503) ? 48 : 25;
		// @v10.5.0 const int max_added_lines = 8;
		const int max_added_lines = (Data.Rec.MaxAddedLnCount > 0 && Data.Rec.MaxAddedLnCount <= 100) ? Data.Rec.MaxAddedLnCount : 8; // @v10.5.0
		uint   j = 0;
		size_t size_offs = 0;
		char   result[16];
		char   out_rec[4096];
		SString kop;
		SString rub;
		SString data_len;
		SString goods_no;
		SString str_len;
		SString barcode;
		SString s_expiry;
		SString grp_no;
		SString goods_name;
		SString ext_text_buf;
		SString temp_buf;
		ConvertDIGI_Text(pScalePLU->GoodsName, /*5*/3 /* fontSize */, line_len_limit, 4/* maxLinesCount */, goods_name); // @v8.6.6 fontSize 7-->5
		{
			/* @v9.8.9 if(pScalePLU->HasAddedMsg()) {
			 	ConvertDIGI_Text(pScalePLU->AddMsgBuf, 3, Data.MaxAddedLine, max_added_lines, ext_text_buf);
			} */
			// @v9.8.9 {
			StringSet ss;
			ext_text_buf.Z();
			if(GetAddedMsgLines(pScalePLU, line_len_limit, max_added_lines, 0, ss) > 0) {
				for(uint ssp = 0; ss.get(&ssp, temp_buf);) {
					ext_text_buf.CatChar(3); // ������ ������
					LongToHexBytesStr(static_cast<long>(temp_buf.Len()), 1, str_len);
					ext_text_buf.CatChar(str_len.C(0)); // ����� ������ ��� ���������� � ������������
					ext_text_buf.Cat(temp_buf).CatChar(13);
				}
				if(ext_text_buf.Last() == 13)
					ext_text_buf.TrimRight().CatChar(12);
			}
			// } @v9.8.9
		}
		double price = R2(pScalePLU->Price);
		long   fract = R0i(ffrac(price) * 100.0);
		LongToBCDStr(fract, "%02ld", kop);
		LongToBCDStr((long)price, "%06ld", rub);
		LongToHexBytesStr((long)goods_name.Len(), 1, str_len);
		LongToBCDStr((long)pScalePLU->Barcode + 2000000L, "%-014ld", barcode);
		LongToBCDStr((long)Data.Rec.LogNum, "%04ld", grp_no);
		{
			long   __plu = 0;
			if(oneof2(Data.Rec.ProtocolVer, 3, 503))
				__plu = (pScalePLU->Barcode + 2000000L) % 100000;
			else
				__plu = pScalePLU->GoodsNo;
			LongToBCDStr(__plu, "%08ld", goods_no);
		}
		LongToBCDStr((long)expiry, "%04ld", s_expiry);

		memzero(out_rec, sizeof(out_rec));
		size_t len = 0;
		out_rec[len++] = (char)0xF1; // #1
		out_rec[len++] = 0x25;       // #2 ������� ����, ��� ������ �����
		for(j = 0; j < 4; j++)
			out_rec[len++] = ((const char *)goods_no)[j]; // #6 ����� PLU
		size_offs = len; // �� ����� �������� � ����� ��������� ������� ������ ������

		out_rec[len++] = 0; // ��� ����� ����� ������
		out_rec[len++] = 0; // ��� ���� �� �������� �����
		// ������� �����, �������� ���� �������� � �������, ����� ��������
		/*
			���� 1
			��� 0 ��� ������ (0 - �������; 1 - �������)
			��� 1 ���� �� ������� ���� (0 - 1-� ����; 1 - 2-� ����)
			��� 2 �������� ���� ������� (0 - ���; 1- ��)
			��� 3 �������� ���� ������������� (0 - ���; 1 - ��)
			��� 4 �������� ���� �������� (0 - ���; 1 - ��)
			��� 5 �������� ����� ������� (0 - ���; 1 - ��)
			��� 6 �������� ����� �������� (0 - ���; 1 - ��)
			��� 7 �������� ������� �������� (0 - ����; 1 - ���� � ����������)
			���� 2
			��� 0 �������� ������� ������� (0 - ����; 1 - ���� � ����������)
			��� 1 ���� ���� (0 - ���� �� ��; 1 - **���� �� 100 �)
			��� 3 �������� ������������� (0 - ���; 1 - ��)
			��� 4 ������������� ���� �� ������� ���� (0 - ���; 1 - ��)
			��� 5 �������� ���� ������� (0 - ����; 1 - ���� ��������)
			��� 6 ������� ��������� ����� ������������ (0 - �����; 1 - ��� PLU)
			��� 7 ����������� ������ PLU (0 - ���; 1 - ��)
		*/
		char status1_flags = 0x04 | 0x10 | 0x40; // 0x54
		if(pScalePLU->Flags & pScalePLU->fCountable) { // @v10.0.0
			status1_flags |= 0x01;
		}
		out_rec[len++] = status1_flags; // #9 PLU ������ 1
		// ����� �������� � �� ���������� �����, ���� �� 1 ��
		out_rec[len++] = 0x00;                           // #10 PLU ������ 1
		/*
		���  0: ������ 1-� ��������	(0-�� ��������� (�� SPEC); 1-������ ����)
		���  1: ������ 2-� �������� (0-���; 1-������)
		���  2: ������ ���������    (0-�� ��������� (�� SPEC); 1-������ ����)
		���  3: ������� ������ (������ ��) (0-���; 1-������ ����)
		���  4: ����� �������� ������ (0-�� ��������� (997); 1-������ ����)
		���  5: ���� ��������������� (0-���; 1-����)
		���  6: ���� ���� �����	(0-���; 1-����)
		���  7: ���� ����������� � ����� �������� (0-���; 1-����)
		*/
		// �������� ������ ������� ����. ��� ����� ����, ����������,
		// �������������. ���� ���� �������� ���������, �������
		// ��������� � ������� 1-� ��������. ������ �������� �� ����������.
		out_rec[len++] = 0x1D;                           // #11 PLU ������ 2
		/* ������� ����� � ��������� (0-���; 1-����)
		���  0: ���� �������� ����������
		���  1: ���� ������� ����. ����������
		���  2: ���� ������ �����������
		���  3: ���� �������
		���  4: ���� ���������������
		���  5: ���� ��������� ������
		���  6: ���� ������ ����������� � PLU ����. ����������
		���  7: ���� ������ ����������� � PLU �����������
		*/
		// � ������ PLU ������������ �������� ������,
		// ����������� � ����. ���������, ������� ������ � ������������� �����������
		// @v7.9.2 out_rec[len++] = 0x20;                           // #12 PLU ������ 2
		out_rec[len++] = ext_text_buf.NotEmpty() ? (0x80|0x20) : (0x20); // #12 PLU ������ 2
		/* ������� ����� � ��������� (0-���; 1-����)
		���  0:	���� ����� ������� � �����
		���  1:	���� ����� ������� � ����� � ��������
		���  2: ���� ����� ������������� � �����
		���  3: ���� ����� �������� � ���� �� �����
		���  4: ���� ������ ��������
		���  5: ���� ������ �����
		���  6: ���� ������ ��������
		*��� 7: ���� �PLU ������ » � �PLU ������ 2»
		*/
		// ���� ���� ��� ����� ������� � ����. ���� ����� �������������,
		// �����  ������� � ����� � ����� �������� � ����. ��� ����� ������ � ������ ����� ��������.
		out_rec[len++] = 0x01;                          // #13 PLU ������ 2
		for(j = 0; j < 3; j++)
			out_rec[len++] = rub.C(j);      // #15
		out_rec[len++] = kop.C(0);          // #16
		out_rec[len++] = 0x11; // #17 ����� ������� �������� � ����� �������������� ��������� ������ F1
		out_rec[len++] = 0x05; // #18 ������ ��������� (2 ����� �����, 5 �������� � 5 ����)
		for(j = 0; j < 7; j++)
			out_rec[len++] = barcode.C(j); // #25
		for(j = 0; j < 2; j++)
			out_rec[len++] = grp_no.C(j);  // #27
		for(j = 0; j < 2; j++)
			out_rec[len++] = s_expiry.C(j); // #29
		for(j = 0; j < goods_name.Len(); j++) {
			out_rec[len++] = goods_name.C(j); // #29 + goods_name.Len()
		}
		if(ext_text_buf.NotEmpty()) {
			for(j = 0; j < ext_text_buf.Len(); j++) {
				out_rec[len++] = ext_text_buf.C(j); // #29 + goods_name.Len() + ext_text_buf.Len()
			}
		}
		out_rec[len++] = 0x00; // ����������� ����� (������ ���� 0x00) // #29 + goods_name.Len() + ext_text_buf.Len() + 1
		//
		// ������, ����� ���� ������ ������, ������� ��� � ����� �� �������� size_offs
		//
		LongToHexBytesStr((long)(len - 2), 2, data_len); // -2 - ������ ��������� ������
		for(j = 0; j < 2; j++)
			out_rec[size_offs+j] = data_len.C(j); // #8

		/*
		const char test[] = {0xF1, 0x25, 0x00, 0x00, 0x00, 0x01, 0x00, 0x3B, 0x54, 0x00, 0x0D, 0x26, 0x01, 0x00, 0x02,
			0x84, 0x39, 0x11, 0x05, 0x22, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x01, 0x07, 0x07, 0x8A,
			0x8E, 0x8B, 0x81, 0x80, 0x91, 0x80, 0x0D, 0x03, 0x11, 0x22, 0x91, 0xAE, 0xA1, 0xA0, 0xE7, 0xEC, 0xEF, 0x20,
			0xE0, 0xA0, 0xA4, 0xAE, 0xE1, 0xE2, 0xEC, 0x22, 0x0C, 0x00
		}; - ��� ����� �����������, ����� 61 ������
		*/
		THROW_PP(send(SocketHandle, (const char *)out_rec, (int)len, 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
		memzero(result, sizeof(result));
		THROW_PP(recv(SocketHandle, result, sizeof(result), 0) != SOCKET_ERROR, PPERR_SCALE_RCV);
		THROW_PP(result[0] == 0x06, PPERR_SCALE_NOACK);
		ok = 1;
	}
	CATCHZOK
	return ok;
}

int SLAPI DIGI::CloseConnection()
{
	int    ok = 1;
	if(!(Data.Rec.Flags & SCALF_TCPIP)) {
		SFile::ZClose(&P_ScaleData);
		ok = DistributeFile(ScalePath, 0);
	}
	else if(Connected) {
		shutdown(SocketHandle, 2);
		closesocket(SocketHandle);
		Connected = 0;
	}
	return ok;
}
//
//	Bizerba
//
class Bizerba : public PPScaleDevice {
public:
	SLAPI  Bizerba(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData)
	{
		PPIniFile ini_file;
		ini_file.GetInt(PPINISECT_SYSTEM, PPINIPARAM_BIZERBA_WHOLEBARCODE, &CutBarcode);
		CutBarcode = !CutBarcode;
		PPGoodsConfig goods_cfg;
		PPObjGoods::ReadConfig(&goods_cfg);
		if(goods_cfg.Flags & GCF_USESCALEBCPREFIX && Data.Rec.IsValidBcPrefix()) {
			if(Data.Rec.BcPrefix >= 200 && Data.Rec.BcPrefix <= 299)
				WghtPrefix = Data.Rec.BcPrefix / 10;
			else
				WghtPrefix = Data.Rec.BcPrefix;
		}
		else
			WghtPrefix   = atol(goods_cfg.WghtPrefix);
		IsConnected    = 0;
		P_DrvBizerba   = 0;
		SocketHandle   = 0;
		AddInfoFieldId = 1;
		ResCode = bzErrOK;
		UseNewAlg = BIN(Data.Rec.ProtocolVer >= 100);
	}
	SLAPI ~Bizerba()
	{
		delete P_DrvBizerba;
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI CloseConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
	}
private:
	enum {
		ErrorCode,
		ScaleErrorCode,
		OpenScale,
		CloseScale,
		WritePLU,
		ClearPLU,
		WriteMsg
	};
	enum BizerbaErrCodes {
		bzErrOK,
		bzErrConnect,
		bzErrParam,
		bzErrWrite,
		bzErrRead,
		bzErrAnswer
	};
	int SLAPI SendPLUByDriver(const ScalePLU * pScalePLU);
	ComDispInterface * SLAPI InitDriver();
	int    SLAPI SetParam(int    iVal);
	int    SLAPI SetParam(long   lVal);
	int    SLAPI SetParam(double dVal);
	int    SLAPI SetParam(const char * pStrVal);
	int    SLAPI ExecBZOper(PPID id);
	int    SLAPI PrepareGoodsName(const char * pGoodsName, char * pBuf, size_t bufSize);
	long   SLAPI PreparePrice(double price, long * pWeightClass);
	long   SLAPI PrepareExpiry(LDATE expiry);

	ComDispInterface * P_DrvBizerba;
	SOCKET SocketHandle;
	int    IsConnected;
	int    UseNewAlg;
	int    ResCode;
	int    CutBarcode;
	long   WghtPrefix;
	long   AddInfoFieldId;
};

ComDispInterface * SLAPI Bizerba::InitDriver()
{
	ComDispInterface * p_drv = 0;
	THROW_MEM(p_drv = new ComDispInterface);
	THROW(p_drv->Init("bizsc.ScaleBizSC"));
	THROW(ASSIGN_ID_BY_NAME(p_drv, ErrorCode) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ScaleErrorCode) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, OpenScale) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, CloseScale) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, WritePLU) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ClearPLU) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, WriteMsg) > 0);
	CATCH
		ZDELETE(p_drv);
	ENDCATCH
	return p_drv;
}

int SLAPI Bizerba::SetParam(int iVal)
	{ return BIN(P_DrvBizerba && P_DrvBizerba->SetParam(iVal) > 0); }
int SLAPI Bizerba::SetParam(long lVal)
	{ return BIN(P_DrvBizerba && P_DrvBizerba->SetParam(lVal) > 0); }
int SLAPI Bizerba::SetParam(double dVal)
	{ return BIN(P_DrvBizerba && P_DrvBizerba->SetParam(dVal) > 0); }
int SLAPI Bizerba::SetParam(const char * pStrVal)
	{ return BIN(P_DrvBizerba && P_DrvBizerba->SetParam(pStrVal) > 0); }

int SLAPI Bizerba::ExecBZOper(PPID id)
{
	int    ok = BIN(P_DrvBizerba && P_DrvBizerba->CallMethod(id) > 0 &&
		P_DrvBizerba->GetProperty(ErrorCode, &ResCode) > 0 && ResCode == bzErrOK);
	if(!ok) {
		int    err = 0;
		switch(ResCode) {
			case bzErrConnect: err = PPERR_SCALE_NOTREGMODE; break;
			case bzErrParam:   err = PPERR_SCALE_SEND;   break;
			case bzErrWrite:
			case bzErrRead:    err = PPERR_SCALE_NOACK;  break;
			case bzErrAnswer:  err = PPERR_SCALE_NOSYNC; break;
		}
		PPSetError(err);
	}
	return ok;
}

int SLAPI Bizerba::SetConnection()
{
	int    ok = 1;
	//char   ip[16];
	SString port_buf;
	AddInfoFieldId = 1;
	//memzero(ip, sizeof(ip));
	//THROW(PPObjScale::DecodeIP(Data.Rec.Port, ip));
	Data.GetExtStrData(Data.extssPort, port_buf);
	if(UseNewAlg) {
		char   send_timeout[10], rcv_timeout[10];
		long   res;
		struct sockaddr_in sin;
		THROW_PP((SocketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) != INVALID_SOCKET, PPERR_SCALE_NOSYNC);
		sin.sin_family           = AF_INET;
		sin.sin_port             = htons(1025);
		sin.sin_addr.S_un.S_addr = inet_addr(port_buf);
		memzero(send_timeout, sizeof(send_timeout));
		memzero(rcv_timeout,  sizeof(rcv_timeout));
		itoa(Data.Rec.Put_Delay, send_timeout, 10);
		itoa(Data.Rec.Get_Delay, rcv_timeout, 10);
		THROW_PP((res = setsockopt(SocketHandle, SOL_SOCKET, SO_SNDTIMEO, (const char *)send_timeout, sstrleni(send_timeout))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		THROW_PP((res = setsockopt(SocketHandle, SOL_SOCKET, SO_RCVTIMEO, (const char *)rcv_timeout, sstrleni(rcv_timeout))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		THROW_PP((res = connect(SocketHandle, reinterpret_cast<const sockaddr *>(&sin), sizeof(sin))) != SOCKET_ERROR, PPERR_SCALE_NOSYNC);
		IsConnected = 1;
	}
	else {
		THROW(P_DrvBizerba = InitDriver());
		THROW(SetParam(port_buf));
		THROW(SetParam(1025L));
		THROW(SetParam(Data.Rec.LogNum));
		THROW(ExecBZOper(OpenScale));
		//THROW(ExecBZOper(ClearPLU));
	}
	CATCH
		if(UseNewAlg) {
			shutdown(SocketHandle, 2);
			closesocket(SocketHandle);
			IsConnected = 0;
		}
		ok = 0;
	ENDCATCH
	return ok;
}

int SLAPI Bizerba::PrepareGoodsName(const char * pGoodsName, char * pBuf, size_t bufSize)
{
	char   gds_name[256];
	SString temp_buf;
	(temp_buf = pGoodsName).Transf(CTRANSF_INNER_TO_OUTER);
	STRNSCPY(gds_name, temp_buf);
	char * p = 0;
	if(sstrlen(gds_name) > 28) {
		char   buf[30], * p = 0;
		STRNSCPY(buf, gds_name + 28);
		gds_name[28] = 0;
		if((p = strrchr(gds_name, ' ')) != 0) {
			*p++ = 10;
			gds_name[28] = buf[0];
		}
		else {
			gds_name[28] = 10;
			strcpy(p = gds_name + 29, buf);
		}
		if(sstrlen(gds_name) > 57) {
			gds_name[57] = 0;
			if((p = strrchr(p, ' ')) != 0)
				*p = 0;
		}
	}
	strnzcpy(pBuf, gds_name, bufSize);
	return 1;
}

long SLAPI Bizerba::PreparePrice(double price, long * pWeightClass)
{
	int    mult = 100, weight_class = 0;
	if(price >= 10000) {
		weight_class = 1;
		mult = 10;
	}
	ASSIGN_PTR(pWeightClass, weight_class);
	return R0i(price * mult);
}

long SLAPI Bizerba::PrepareExpiry(LDATE expiry)
{
	long expiry_days = 0;
	if(expiry != ZERODATE) {
		expiry_days = diffdate(expiry, getcurdate_());
		if(expiry_days < 0 || expiry_days > 999)
			expiry_days = 0;
	}
	return expiry_days;
}

int SLAPI Bizerba::SendPLUByDriver(const ScalePLU * pScalePLU)
{
	int    ok = -1;
	if(pScalePLU) {
		long   expiry = 0, weight_class = 0, mult = 100, lprice = 0, barcode = 0, msg_no = 0;
		char   goods_name[128];
		if(pScalePLU->HasAddedMsg()) {
			const size_t max_len = 200;
			StringSet ss("\n");
			GetAddedMsgLines(pScalePLU, NZOR(Data.Rec.MaxAddedLn, 50), max_len, amlfMaxText, ss);
			SString added_msg = ss.getBuf();
			if(added_msg.Len()) {
				added_msg.Trim(max_len).Transf(CTRANSF_INNER_TO_OUTER);
				msg_no = AddInfoFieldId;
				THROW(SetParam(msg_no));
				THROW(SetParam(added_msg.cptr()));
				THROW(ExecBZOper(WriteMsg));
				AddInfoFieldId++;
			}
		}
		THROW(SetParam(pScalePLU->GoodsNo)); // PLU
		THROW(SetParam(1L));                 // DivNo
		THROW(SetParam(1L));                 // GrpNo

		PrepareGoodsName(pScalePLU->GoodsName, goods_name, sizeof(goods_name));
		THROW(SetParam(goods_name));
		THROW(SetParam(PreparePrice(pScalePLU->Price, &weight_class)));
		THROW(SetParam(WghtPrefix ? WghtPrefix : 20L + (long)(pScalePLU->Barcode / 100000L)));
		barcode = pScalePLU->Barcode % 100000L;
		if(CutBarcode)
			barcode /= 10;
		THROW(SetParam(barcode));
		THROW(SetParam(msg_no));   // MessageNo
		THROW(SetParam(PrepareExpiry(pScalePLU->Expiry)));
		THROW(SetParam(0L));   // Expiry2
		THROW(SetParam(weight_class));
		THROW(ExecBZOper(WritePLU));
		ok = 1;
	}
	CATCHZOK
	return ok;
}

int SLAPI Bizerba::SendPLU(const ScalePLU * pScalePLU)
{
	int    ok = -1;
	if(pScalePLU) {
		if(UseNewAlg) {
			int    weight_goods = 1, department = 1;
			long   weight_class = 0, expiry_days = 0, lprice = 0;
			char   delimiter = 0x1b, barcode[14], reply[128];
			SString alt_texts, tfzu_texts, empty_tfzu, goods_name, reply_msg, send_buf, log_num;

			(goods_name = pScalePLU->GoodsName).ReplaceChar('@', 'a').Transf(CTRANSF_INNER_TO_OUTER);
			THROW_PP(pScalePLU->GoodsNo > 0 && pScalePLU->GoodsNo <= 999999, PPERR_SCALE_INVPLUID);
			THROW_PP(!goods_name.Empty(), PPERR_NAMENEEDED);
			expiry_days  = PrepareExpiry(pScalePLU->Expiry);
			lprice       = PreparePrice(pScalePLU->Price, &weight_class);
			lprice = (lprice >= 0 && lprice <= 999999) ? lprice : 0;
			log_num.Printf("%02ld", Data.Rec.LogNum);
			memzero(barcode, sizeof(barcode));
			{
				long lbcode = 0, wght_prefix = (WghtPrefix) ? WghtPrefix : (20L + (long)(pScalePLU->Barcode / 100000L));
				SString buf;
				lbcode = pScalePLU->Barcode % 100000L;
				/*if(CutBarcode)
					lbcode /= 10; */
				buf.Printf("0%02ld%05ld00000", wght_prefix, lbcode);
				buf.CopyTo(barcode, sizeof(barcode));
			}
			(reply_msg = "QUIT0").CatChar(delimiter).Cat("BLK 0").CatChar(delimiter);
			tfzu_texts = "@00@04";
			empty_tfzu = "@00@04";
			{
				// @v10.5.0 const  uint msgs_count = 4;
				const  uint msgs_count = (Data.Rec.MaxAddedLnCount > 0 && Data.Rec.MaxAddedLnCount <= 100) ? Data.Rec.MaxAddedLnCount : 4;
				StringSet add_str("\n");
				GetAddedMsgLines(pScalePLU, 63, msgs_count, 0, add_str);
				SString line_buf;
				int    r = 1;
				for(uint p = 0, line_no = 0; line_no < msgs_count;) {
					long   msg_num = 0;
					if(r)
						r = add_str.get(&p, line_buf);
					else
						line_buf.Z();
					line_no++;
					if(line_buf.NotEmptyS()) {
						line_buf.Transf(CTRANSF_INNER_TO_OUTER).ReplaceChar('@', 'a').ReplaceStr("\xD\xA", "\n", 0);
						send_buf.Z().Cat("ATST  ").CatChar(delimiter).Cat("S").Cat(log_num).CatChar(delimiter);
						send_buf.Cat("WALO0").CatChar(delimiter);
						send_buf.Cat("ATNU").Cat(AddInfoFieldId).CatChar(delimiter);
						send_buf.Cat("ATTE").Cat(line_buf).CatChar(delimiter).Cat("BLK ").CatChar(delimiter);

						send_buf.ToUtf8();
						THROW_PP(send(SocketHandle, (const char *)send_buf, (int)send_buf.Len(), 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
						THROW_PP(recv(SocketHandle, reply, sizeof(reply), 0) != SOCKET_ERROR, PPERR_SCALE_RCV);
						msg_num = AddInfoFieldId;
						AddInfoFieldId++;
					}
					alt_texts.Cat("ALT").Cat(line_no).Cat(msg_num).CatChar(delimiter);
				}
			}
			send_buf.Z().Cat("PLST  ").CatChar(delimiter).Cat("S").Cat(log_num).CatChar(delimiter);
			send_buf.Cat("WALO0").CatChar(delimiter);
			send_buf.Cat("PNUM").Cat(pScalePLU->GoodsNo).CatChar(delimiter);
			send_buf.Cat("ABNU").Cat(department).CatChar(delimiter);
			send_buf.Cat("ANKE0").CatChar(delimiter);

			if(!weight_goods)
				send_buf.Cat("KLAR3").CatChar(delimiter);
			else
				send_buf.Cat("KLAR0").CatChar(delimiter);

			send_buf.Cat("GPR1").Cat(lprice).CatChar(delimiter);
			send_buf.Cat("RABZ1").CatChar(delimiter);
			send_buf.Cat("PTYP4").CatChar(delimiter);
			send_buf.Cat("WGNU").Cat(pScalePLU->GrpCode).CatChar(delimiter);
			send_buf.Cat("ECO1").Cat(barcode).CatChar(delimiter);
			send_buf.Cat("HBA1").Cat(expiry_days).CatChar(delimiter);
			send_buf.Cat("HBA20").CatChar(delimiter);
			send_buf.Cat("KLGE").Cat(weight_class).CatChar(delimiter);
			send_buf.Cat(alt_texts);
			send_buf.Cat("PLTE").Cat(goods_name).CatChar(delimiter);
			send_buf.Cat("BLK ").CatChar(delimiter);

			memzero(reply, sizeof(reply));
			send_buf.ToUtf8();
			THROW_PP(send(SocketHandle, (const char *)send_buf, (int)send_buf.Len(), 0) != SOCKET_ERROR, PPERR_SCALE_SEND);
			THROW_PP(recv(SocketHandle, reply, sizeof(reply), 0) != SOCKET_ERROR, PPERR_SCALE_RCV);
		}
		else
			THROW(ok = SendPLUByDriver(pScalePLU));
	}
	CATCHZOK
	return ok;
}

int SLAPI Bizerba::CloseConnection()
{
	int    ok = 1;
	if(UseNewAlg) {
		if(IsConnected) {
			shutdown(SocketHandle, 2);
			closesocket(SocketHandle);
			IsConnected = 0;
		}
	}
	else
		ok = ExecBZOper(CloseScale);
	return ok;
}
//
//	ShtrihPrint
//
#define RESCODE_NO_ERROR   0x0000
#define DEF_BAUD_RATE_NO   2L // ��� 9600 ���
#define DEF_REMOTE_PORT    1111L
#define DEF_LOCAL_PORT     2000L

class ShtrihPrint : public PPScaleDevice {
public:
	SLAPI  ShtrihPrint(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData), P_DrvShtrih(0), StrCountInMsg(1), ResCode(RESCODE_NO_ERROR)
	{
		if(H_Port != INVALID_HANDLE_VALUE) {
			CloseHandle(H_Port);
			H_Port = INVALID_HANDLE_VALUE;
		}
		SString temp_buf;
		PPGoodsConfig goods_cfg;
		PPObjGoods::ReadConfig(&goods_cfg);
		WghtPrefix = (temp_buf = goods_cfg.WghtPrefix).Strip().ToLong();
		WghtCntPrefix = (temp_buf = goods_cfg.WghtCntPrefix).Strip().ToLong();
		MsgN = 0;
	}
	SLAPI ~ShtrihPrint()
	{
		delete P_DrvShtrih;
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI CloseConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
	}
private:
	enum {
		ResultCode,
		ResultCodeDescription,
		DeviceInterface,
		PortNumber,
		ComNumber,
		BaudRate,
		TimeOut,
		RemoteHost,
		RemotePort,
		LocalPort,
		TimeoutUDP,
		Connect,
		Disconnect,
		PrefixBCType,
		SetPrefixBCType,
		PLUNumber,
		Price,
		Tare,
		ItemCode,
		NameFirst,
		NameSecond,
		ShelfLife,
		GroupCode,
		MessageNumber,
		PictureNumber,
		ROSTEST,
		ExpiryDate,               //
		GoodsType,                //
		StringsCountInMessage,    //
		StringNumber,             //
		MessageString,            //
		SetPLUData,               // ������� �������� ���������� � ������
		SetPLUDataEx,             // ������� ����������� �������� ���������� � ������
		GetStringsCountInMessage, // ������� ��������� ���������� ����� � ���������
		SetMessageData            // ������� ������ ��������� � ������
	};
	ComDispInterface * SLAPI InitDriver();
	int    SLAPI GetSPProp(PPID id, long & rVal);
	int    SLAPI SetSPProp(PPID id, long lVal, int writeOnly = 0);
	int    SLAPI SetSPProp(PPID id, double dVal, int writeOnly = 0);
	int    SLAPI SetSPProp(PPID id, const char * pStrVal, int writeOnly = 0);
	int    SLAPI SetSPProp(PPID id, LDATE dtVal, int writeOnly = 0);
	int    SLAPI ExecSPOper(PPID id);
	void   SLAPI SetErrorMessage();
	ComDispInterface * P_DrvShtrih;
	int    ResCode;
	long   WghtPrefix;
	long   WghtCntPrefix;
	long   StrCountInMsg; // ������������ ���������� ����� � ��������� � ������
	long   MsgN;          // ����� ���������. ������������� ����� ������ ����� �������� ���������.
};

ComDispInterface * SLAPI ShtrihPrint::InitDriver()
{
	ComDispInterface * p_drv = 0;
	THROW_MEM(p_drv = new ComDispInterface);
	THROW(p_drv->Init("AddIn.DrvLP"));
	THROW(ASSIGN_ID_BY_NAME(p_drv, ResultCode) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ResultCodeDescription) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, DeviceInterface) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, PortNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ComNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, BaudRate) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, TimeOut) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, RemoteHost) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, RemotePort) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, LocalPort) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, TimeoutUDP) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Connect) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Disconnect) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, PrefixBCType) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, SetPrefixBCType) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, PLUNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Price) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, Tare) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, PLUNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ItemCode) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, NameFirst) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, NameSecond) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ShelfLife) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, GroupCode) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, MessageNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, PictureNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ROSTEST) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, ExpiryDate) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, GoodsType) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, StringsCountInMessage) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, StringNumber) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, MessageString) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, SetPLUData) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, SetPLUDataEx) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, GetStringsCountInMessage) > 0);
	THROW(ASSIGN_ID_BY_NAME(p_drv, SetMessageData) > 0);
	CATCH
		ZDELETE(p_drv);
	ENDCATCH
	return p_drv;
}

void SLAPI ShtrihPrint::SetErrorMessage()
{
	char   err_buf[MAXPATH];
	MEMSZERO(err_buf);
	if(ResCode != RESCODE_NO_ERROR && P_DrvShtrih &&
		P_DrvShtrih->GetProperty(ResultCodeDescription, err_buf, sizeof(err_buf) - 1) > 0) {
		SString err_msg;
		PPSetAddedMsgString(err_msg.Cat(err_buf).ToOem());
		PPSetError(PPERR_SHTRIHPRINT);
	}
}

int SLAPI ShtrihPrint::GetSPProp(PPID id, long & rVal)
	{ return (P_DrvShtrih && P_DrvShtrih->GetProperty(id, &rVal) > 0) ? 1 : 0; }
int SLAPI ShtrihPrint::SetSPProp(PPID id, long lVal, int writeOnly /*=0*/)
	{ return (P_DrvShtrih && P_DrvShtrih->SetProperty(id, lVal, writeOnly) > 0) ? 1 : 0; }
int SLAPI ShtrihPrint::SetSPProp(PPID id, double dVal, int writeOnly /*=0*/)
	{ return (P_DrvShtrih && P_DrvShtrih->SetProperty(id, dVal, writeOnly) > 0) ? 1 : 0; }
int SLAPI ShtrihPrint::SetSPProp(PPID id, const char * pStrVal, int writeOnly /*=0*/)
	{ return (P_DrvShtrih && P_DrvShtrih->SetProperty(id, pStrVal, writeOnly) > 0) ? 1 : 0; }
int SLAPI ShtrihPrint::SetSPProp(PPID id, LDATE dtVal, int writeOnly /*=0*/)
	{ return (P_DrvShtrih && P_DrvShtrih->SetProperty(id, dtVal, writeOnly) > 0) ? 1 : 0; }

int SLAPI ShtrihPrint::ExecSPOper(PPID id)
{
	int  ok = (P_DrvShtrih && P_DrvShtrih->CallMethod(id) > 0 &&
		P_DrvShtrih->GetProperty(ResultCode, &ResCode) > 0 &&  ResCode == RESCODE_NO_ERROR) ? 1 : 0;
	if(!ok)
		SetErrorMessage();
	return ok;
}

int SLAPI ShtrihPrint::SetConnection()
{
	int   ok = 1;
	THROW(P_DrvShtrih = InitDriver());
	if(Data.Rec.Flags & SCALF_TCPIP) {
		SString port_buf;
		//char   ip[16];
		//THROW(PPObjScale::DecodeIP(Data.Rec.Port, ip));
		Data.GetExtStrData(Data.extssPort, port_buf);
		THROW(SetSPProp(DeviceInterface, 1L)); // Ethernet
		THROW(SetSPProp(RemoteHost, port_buf));
		THROW(SetSPProp(RemotePort, DEF_REMOTE_PORT));
		THROW(SetSPProp(LocalPort,  DEF_LOCAL_PORT));
		THROW(SetSPProp(TimeoutUDP, (long)Data.Rec.Get_Delay));
	}
	else {
		long   time_out = (Data.Rec.Get_Delay > 0 && Data.Rec.Get_Delay < 256) ? Data.Rec.Get_Delay : 0;
		THROW(SetSPProp(DeviceInterface, 0L)); // RS-232
		THROW(SetSPProp(PortNumber, 0L));
		THROW(SetSPProp(ComNumber,  (long)(PortNo + 1)));
		THROW(SetSPProp(BaudRate,   DEF_BAUD_RATE_NO));
		THROW(SetSPProp(TimeOut,    time_out));
	}
	THROW(ExecSPOper(Connect));
	{
		long   sc = 0;
		if(ExecSPOper(GetStringsCountInMessage) && GetSPProp(StringsCountInMessage, sc))
			StrCountInMsg = (sc > 1 && sc <= 20) ? sc : 1;
	}
	MsgN = 0;
	CATCHZOK
	return ok;
}

int SLAPI ShtrihPrint::SendPLU(const ScalePLU * pScalePLU)
{
	int    ok = -1;
	if(pScalePLU) {
		long   wght_prefix = WghtPrefix;
		int    is_msg = pScalePLU->HasAddedMsg(); //(���� �������� � ��������� ���������)
		SString temp_buf;
		THROW(SetSPProp(PLUNumber, pScalePLU->GoodsNo));
		THROW(SetSPProp(Price, pScalePLU->Price ));
		THROW(SetSPProp(Tare, 0L));
		THROW(SetSPProp(ItemCode, pScalePLU->Barcode % 100000));
		{
			char  goods_name[128];
			char  name_part_1[128], name_part_2[128];
			SBaseBuffer name_items[2];
			memzero(name_part_1, sizeof(name_part_1));
			memzero(name_part_2, sizeof(name_part_2));
			name_items[0].Set(name_part_1, 28);
			name_items[1].Set(name_part_2, 28);
			(temp_buf = pScalePLU->GoodsName).Transf(CTRANSF_INNER_TO_OUTER).CopyTo(goods_name, sizeof(goods_name));
			SplitString(goods_name, 2, name_items);
			THROW(SetSPProp(NameFirst, name_part_1));
			THROW(SetSPProp(NameSecond, name_part_2));
		}
		{
			long   shelf_life = 0;
			if(checkdate(pScalePLU->Expiry)) {
				shelf_life = diffdate(pScalePLU->Expiry, getcurdate_());
				if(shelf_life < 0)
					shelf_life = 0;
			}
			THROW(SetSPProp(ShelfLife, shelf_life));
		}
		SETIFZ(wght_prefix, 20 + pScalePLU->Barcode / 100000);
		THROW(SetSPProp(GroupCode, wght_prefix));
		THROW(SetSPProp(MessageNumber, is_msg ? ++MsgN : 0L));
		THROW(SetSPProp(PictureNumber, 0L));
		THROW(SetSPProp(ROSTEST, ""));
		if(pScalePLU->Flags & ScalePLU::fCountable) {
			THROW(SetSPProp(ExpiryDate, encodedate(1, 1, 2001)));
			THROW(SetSPProp(GoodsType, 1L));
			THROW(ExecSPOper(SetPLUDataEx));
		}
		else {
			THROW(ExecSPOper(SetPLUData));
		}
		if(is_msg) {
			StringSet add_str("\x0A");
			GetAddedMsgLines(pScalePLU, MIN(labs(Data.Rec.MaxAddedLn), 50), StrCountInMsg, 0, add_str);
			for(uint p = 0, strn = 0; (long)strn < StrCountInMsg && add_str.get(&p, temp_buf); strn++) {
				THROW(SetSPProp(MessageNumber, MsgN));
				// ��� �������� �� ������������: ������� �������� ��� �� ����������� (???) THROW(SetSPProp(StringNumber, (long)(strn+1)));
				THROW(SetSPProp(StringNumber, (long)(strn+1), 1));
				THROW(SetSPProp(MessageString, temp_buf.Transf(CTRANSF_INNER_TO_OUTER)));
				THROW(ExecSPOper(SetMessageData));
			}
		}
		ok = 1;
	}
	CATCHZOK
	return ok;
}

int SLAPI ShtrihPrint::CloseConnection()
{
	return ExecSPOper(Disconnect);
}
//
//
//
class ShtrihCE : public PPScaleDevice {
public:
	SLAPI  ShtrihCE(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData), FixedMsgResID(0x00102030), P_FOut(0)
	{
	}
	SLAPI ~ShtrihCE()
	{
		delete P_FOut;
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI CloseConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
	}
private:
	long   FixedMsgResID; // ������������� ������������� ������� ��������� ���������
	StrAssocArray MsgResLines;
	PPObjGoods GObj;
	SFile * P_FOut;
};

int SLAPI ShtrihCE::SetConnection()
{
	int    ok = 1;
	ZDELETE(P_FOut);
	MsgResLines.Z();
	{
		SString file_name;
		PPGetFilePath(PPPATH_OUT, "pcscale.txt", file_name);
		THROW_MEM(P_FOut = new SFile(file_name, SFile::mWrite));
		THROW_SL(P_FOut->IsValid());
		{
			SString line_buf;
			P_FOut->WriteLine(line_buf.Z().Cat("##@@&&").CR());
			P_FOut->WriteLine(line_buf.Z().Cat("#").CR());
		}
	}
	CATCH
		ok = 0;
		ZDELETE(P_FOut);
	ENDCATCH
	return ok;
}

int SLAPI ShtrihCE::CloseConnection()
{
	if(P_FOut) {
		{
			SString line_buf;
			if(MsgResLines.getCount()) {
				for(uint i = 0; i < MsgResLines.getCount(); i++) {
					StrAssocArray::Item item = MsgResLines.Get(i);
					line_buf.Z().Cat("<R").Space();
					line_buf.Cat(item.Id).Semicol();
					line_buf.Cat((long)1).Semicol();
					line_buf.CatCharN(';', 19);          // #3-#21
					line_buf.Cat(item.Txt);              // #22
					line_buf.CR();
					P_FOut->WriteLine(line_buf);
				}
			}
			P_FOut->WriteLine((line_buf = "$$$RPL").Cat("{TOV}").Cat("{RES}").CR());
		}
		//
		ZDELETE(P_FOut);
	}
	return 1;
}

int SLAPI ShtrihCE::SendPLU(const ScalePLU * pScalePLU)
{
/*
##@@&&
@
        0                           1          2                                                     3
        12            3 4      567890123 45678901                                      23456789      01     2
<D 11271;;������� ����;0;539.00;;;;;;;;;0;;;;;;;;"���������� ���� ��� �� ����� �� ����";;;;;;;;111271;;211271
<R 111271;1;;;;;;;;;;;;;;;;;;;;"���=�� ����� � ����� ������� � ��� �� �������"
<R 211271;2;;;;;;;;;;;;;;;;;;;;2811271.txt
$$$RPL


1 ��� (������ / ������ �������); ����� 1..999999
2 �����-���; ����� 13 ������ 1
3 ������������ ������; ������ 255 ��������
4 ��� ������; ����� 0 � ������� �����, 1 � ������� �����, 2 � ������ �������
5 ����; ������� 0..9999.99
6 ��� ������-��������� ������ 1-��� ������ ��������; ����� 0..999999 (0 � ��� ������-��������� 1-��� ������ ��������)
7 ��� ������-��������� ������ 2-��� ������ ��������; ����� 0..999999 (0 � ��� ������-��������� 2-��� ������ ��������)
8 ��� ������-��������� ������ 3-��� ������ ��������; ����� 0..999999 (0 � ��� ������-��������� 3-��� ������ ��������)
9 ��� ������-��������� ������ 4-��� ������ ��������; ����� 0..999999 (0 � ��� ������-��������� 4-��� ������ ��������)
12 ���� ��������; ����� 0..9999 (� ����) 2
13 ���� ����������; ���� ������: ��.��.���� 2
22 ���������� ������; ������ 255 ��������
23 ����� ����; ������� 0..������������ �������� ����
30 ��� ��������� (��� �������, ��� ������� � �����); ����� 4 ����� 3
31 ��� ����� �������� (��� �������, ��� ������� � ���� ��������); ����� 4 ����� 4
32 ��� ����� ��������� (��� �������, ��� ������� � ��������� ����); ����� 4 ����� 5
*/
/*
##@@&&
#
--���������
<D [4 ������� ��� ������� �������];;[8 �������];[7];[������� 5 ������ xx.00];;;;;;;;;[0 ������];;;;;;;;;;;;;;;;[4 ������� ������ ������� ������� ����� 1];;[4 ������� ������ ������� ������� ����� 2]
--�������� ������ 1
<R [4 ������� ������ ������� ������� ����� 1];[������ 1];;;;;;;;;;;;;;;;;;;;[9 �������, ������ 255 ��������]
--�������� ������ 2
<R [4 ������� ������ ������� ������� ����� 2];[������ 2];;;;;;;;;;;;;;;;;;;;[4 �������, 2 �������, .txt ������]

$$$RPL
*/
	int    ok = -1;
	if(pScalePLU && P_FOut) {
		SString line_buf, temp_buf;
		StringSet ext_lines;
		line_buf.Cat("<D").Space();
        line_buf.Cat(pScalePLU->GoodsID).Semicol();                    // #1
		temp_buf.Cat(pScalePLU->Barcode); // @v9.8.12 (temp_buf = pScalePLU->Barcode)-->(temp_buf.Cat(pScalePLU->Barcode))
		if(temp_buf.Len() > 5)
			temp_buf.ShiftLeft(temp_buf.Len() - 5);
        line_buf.Cat(temp_buf).Semicol();                    // #2
		(temp_buf = pScalePLU->GoodsName).Transf(CTRANSF_INNER_TO_OUTER);
        line_buf.Cat(temp_buf).Semicol();                  // #3
        line_buf.CatChar(pScalePLU->fCountable ? '1' : '0').Semicol(); // #4
		line_buf.Cat(pScalePLU->Price, MKSFMTD(0, 2, 0)).Semicol();    // #5
		line_buf.CatCharN(';', 6);                                        // #6-#11
		const long   numdays = diffdate(pScalePLU->Expiry, getcurdate_());
		if(numdays > 0 && numdays <= 366)
			line_buf.Cat(numdays);
		line_buf.Semicol();                                            // #12
		line_buf.Semicol();                                            // #13 ���� ����������
		line_buf.CatCharN(';', 8);                                        // #14-#21
		line_buf.Semicol();                                            // #22 ����������
		line_buf.Semicol();                                            // #23 ����� ����
		line_buf.CatCharN(';', 6);                                        // #24-#29
		if(GetAddedMsgLines(pScalePLU, 255, 1, 0, ext_lines) > 0) {
            ext_lines.get(0U, temp_buf);
			if(temp_buf.NotEmptyS()) {
				temp_buf.Transf(CTRANSF_INNER_TO_OUTER);
				MsgResLines.Add(pScalePLU->GoodsID, temp_buf);
				line_buf.Cat(pScalePLU->GoodsID);
			}
		}
		line_buf.Semicol();                                            // #30 ����� ���������
		line_buf.Semicol();                                            // #31 ��� ����� ��������
		// none line_buf.Cat(FixedMsgResID);                           // #32 ������������� ������� ���������
		P_FOut->WriteLine(line_buf.CR());
	}
	return ok;
}
//
// ExportToFile
//
class ExportToFile : public PPScaleDevice {
public:
	SLAPI  ExportToFile(int p, const PPScalePacket * pData) : PPScaleDevice(p, pData), P_GoodsExp(0)
	{
	}
	SLAPI ~ExportToFile()
	{
		ZDELETE(P_GoodsExp);
	}
	virtual int  SLAPI SetConnection();
	virtual int  SLAPI CloseConnection();
	virtual int  SLAPI SendPLU(const ScalePLU *);
	virtual void SLAPI GetDefaultSysParams(PPScalePacket * pData)
	{
	}
private:
	PPGoodsExporter * P_GoodsExp;
	PPObjGoods GObj;
};

int SLAPI ExportToFile::SetConnection()
{
	int    ok = -1;
	SString ini_file_name;
	PPGoodsImpExpParam param;
	THROW(PPGetFilePath(PPPATH_BIN, PPFILNAM_IMPEXP_INI, ini_file_name));
	THROW_MEM(P_GoodsExp = new PPGoodsExporter);
	{
		SString scale_name;
		PPIniFile ini_file(ini_file_name, 0, 1, 1);
		param.Direction = 0;
		THROW(LoadSdRecord(PPREC_GOODS2, &param.InrRec));
   		param.ProcessName(1, (scale_name = Data.Rec.Name));
		if(!isempty(scale_name))
			THROW(param.ReadIni(&ini_file, scale_name, 0));
		param.ReadIni(&ini_file, scale_name, 0);
		THROW(ok = P_GoodsExp->Init(&param));
	}
	CATCHZOK
	return ok;
}

int SLAPI ExportToFile::CloseConnection()
{
	ZDELETE(P_GoodsExp);
	return 1;
}

int SLAPI ExportToFile::SendPLU(const ScalePLU * pScalePLU)
{
	int    ok = -1;
	if(pScalePLU && P_GoodsExp) {
		PPGoodsPacket pack;
		if(GObj.GetPacket(pScalePLU->GoodsID, &pack, 0) > 0) {
			SString barcode;
			barcode.Cat(pScalePLU->Barcode);
			ok = P_GoodsExp->ExportPacket(&pack, barcode.cptr(), Data.Rec.AltGoodsGrp);
		}
	}
	return ok;
}
//
//
//
int SLAPI GetPort(const char * pPortName, int * pPortNo)
{
	int    p = -1;
	if(IsComDvcSymb(pPortName, &p) == comdvcsCom && p > 0 && p < 32) {
		ASSIGN_PTR(pPortNo, p-1);
		return 1;
	}
	else {
		ASSIGN_PTR(pPortNo, -1);
		return PPSetError(PPERR_SCALE_INVPORT);
	}
}

SLAPI PPScalePacket::PPScalePacket() : PPExtStrContainer()
{
}

PPScalePacket & SLAPI PPScalePacket::Z()
{
	PPExtStrContainer::Z();
	MEMSZERO(Rec);
	return *this;
}

void SLAPI PPScalePacket::SetSysParams(uint _getNumTries, uint _getDelay, uint _putNumTries, uint _putDelay)
{
	Rec.Get_NumTries = static_cast<uint16>(_getNumTries);
	Rec.Get_Delay = static_cast<uint16>(_getDelay);
	Rec.Put_NumTries = static_cast<uint16>(_putNumTries);
	Rec.Put_Delay = static_cast<uint16>(_putDelay);
}

SLAPI PPObjScale::PPObjScale(void * extraPtr) : PPObjReference(PPOBJ_SCALE, extraPtr)
{
	ImplementFlags |= (implStrAssocMakeList | implTreeSelector);
}

int SLAPI PPObjScale::Browse(void * extraPtr)
{
	return RefObjView(this, PPDS_CRRSCALE, 0);
}

class ScaleDialog : public TDialog {
public:
	ScaleDialog() : TDialog(DLG_SCALE)
	{
		TButton * p_sys_btn = static_cast<TButton *>(getCtrlView(CTL_SCALE_SYSBTN));
		if(p_sys_btn)
			DefSysBtnText = p_sys_btn->Title;
	}
	int    setDTS(const PPScalePacket * pData);
	int    getDTS(PPScalePacket * pData);
private:
	DECL_HANDLE_EVENT;
	int    editSysData();
	int    editExpPaths();
	void   ReplyScaleTypeSelection(PPID scaleTypeID);

	PPScalePacket Data;
	//SString ExpPaths;
	SString DefSysBtnText;
};

int ScaleDialog::editSysData()
{
	int    ok = -1;
	PPID   scale_type_id = getCtrlLong(CTLSEL_SCALE_TYPE);
	if(scale_type_id == PPSCLT_EXPORTTOFILE) {
		SString scale_name;
		getCtrlString(CTL_SCALE_NAME, scale_name);
		if(scale_name.Len()) {
			PPGoodsImpExpParam param;
			if(LoadSdRecord(PPREC_GOODS2, &param.InrRec)) {
				param.ProcessName(1, scale_name);
				ok = EditGoodsImpExpParams(scale_name);
			}
			else
				ok = PPErrorZ();
		}
	}
	else if(scale_type_id != 0) {
		TDialog * dlg = new TDialog(DLG_SCALESYS);
		if(CheckDialogPtrErr(&dlg)) {
			if(!(Data.Rec.Flags & SCALF_SYSPINITED)) {
				PPScaleDevice * p_comm = 0;
				if(scale_type_id == PPSCLT_CAS)
					p_comm = new CommLP15(0, &Data);
				else if(scale_type_id == PPSCLT_MASSAK)
					p_comm = new CommMassaK(0, &Data);
				else if(scale_type_id == PPSCLT_MTOLEDO)
					p_comm = new TCPIPMToledo(0, &Data);
				else if(scale_type_id == PPSCLT_BIZERBA)
					p_comm = new Bizerba(0, &Data);
				else if(scale_type_id == PPSCLT_SHTRIHPRINT)
					p_comm = new ShtrihPrint(0, &Data);
				else if(scale_type_id == PPSCLT_WEIGHTTERM)
					p_comm = new WeightTerm(0, &Data);
				else if(scale_type_id == PPSCLT_DIGI)
					p_comm = new DIGI(&Data);
				else if(scale_type_id == PPSCLT_CASCL5000J)
					p_comm = new CasCL5000J(0, &Data);
				else {
					delete dlg;
					return -1;
				}
				p_comm->GetDefaultSysParams(&Data);
				delete p_comm;
			}
			dlg->setCtrlData(CTL_SCALESYS_GETTRIES, &Data.Rec.Get_NumTries);
			dlg->setCtrlData(CTL_SCALESYS_GETDELAY, &Data.Rec.Get_Delay);
			dlg->setCtrlData(CTL_SCALESYS_PUTTRIES, &Data.Rec.Put_NumTries);
			dlg->setCtrlData(CTL_SCALESYS_PUTDELAY, &Data.Rec.Put_Delay);
			if(ExecView(dlg) == cmOK) {
				dlg->getCtrlData(CTL_SCALESYS_GETTRIES, &Data.Rec.Get_NumTries);
				dlg->getCtrlData(CTL_SCALESYS_GETDELAY, &Data.Rec.Get_Delay);
				dlg->getCtrlData(CTL_SCALESYS_PUTTRIES, &Data.Rec.Put_NumTries);
				dlg->getCtrlData(CTL_SCALESYS_PUTDELAY, &Data.Rec.Put_Delay);
				Data.Rec.Flags |= SCALF_SYSPINITED;
				ok = 1;
			}
		}
		else
			ok = 0;
		delete dlg;
	}
	return ok;
}

int ScaleDialog::editExpPaths()
{
	int    ok = -1;
	TDialog * dlg = new TDialog(DLG_SCALE_EXPPATHS);
	if(CheckDialogPtrErr(&dlg)) {
		SString paths;
		Data.GetExtStrData(Data.extssPaths, paths);
		dlg->setCtrlString(CTL_SCALE_EXPPATHS, paths);
		while(ok < 0 && ExecView(dlg) == cmOK) {
			dlg->getCtrlString(CTL_SCALE_EXPPATHS, paths);
			if(paths.NotEmptyS()) {
				Data.PutExtStrData(Data.extssPaths, paths);
				ok = 1;
			}
			else
				PPErrorByDialog(dlg, CTL_SCALE_EXPPATHS, PPERR_DESTDIRNEEDED);
		}
	}
	else
		ok = 0;
	return ok;
}

void ScaleDialog::ReplyScaleTypeSelection(PPID scaleTypeID)
{
	GetClusterData(CTL_SCALE_FLAGS, &Data.Rec.Flags);
	int    use_exp_paths = BIN(scaleTypeID == PPSCLT_CRCSHSRV || (!(Data.Rec.Flags & SCALF_TCPIP) && scaleTypeID == PPSCLT_DIGI));
	TCluster * p_clu = static_cast<TCluster *>(getCtrlView(CTL_SCALE_FLAGS));
	if(p_clu) {
		const uint num_items = p_clu->getNumItems();
		for(uint i = 0; i < num_items; i++)
			p_clu->disableItem(i, oneof2(scaleTypeID, PPSCLT_WEIGHTTERM, PPSCLT_SCALEGROUP));
		DisableClusterItem(CTL_SCALE_FLAGS, 4, 0);
	}
	if(use_exp_paths) {
		SString empty;
		setCtrlString(CTL_SCALE_PORT, empty);
		setCtrlLong(CTL_SCALE_PRTCLVER, 0);
	}
	disableCtrls(use_exp_paths, CTL_SCALE_PORT, CTL_SCALE_PRTCLVER, 0);
	disableCtrls(BIN(scaleTypeID == PPSCLT_SCALEGROUP), CTL_SCALE_PORT, CTL_SCALE_PRTCLVER, CTL_SCALE_LOGNUM,
		CTL_SCALE_BCPREFIX, CTLSEL_SCALE_PARENT, CTLSEL_SCALE_PARENT, 0L);
	if(scaleTypeID == PPSCLT_SCALEGROUP)
		SetupPPObjCombo(this, CTLSEL_SCALE_PARENT, PPOBJ_SCALE, 0, 0, PPObjScale::MakeExtraParam(PPSCLT_SCALEGROUP, 0));

	TButton * p_sys_btn = static_cast<TButton *>(getCtrlView(CTL_SCALE_SYSBTN));
	if(p_sys_btn) {
		SString sys_btn_text;
		if(scaleTypeID == PPSCLT_EXPORTTOFILE)
			PPGetWord(PPWORD_EXPORTCFG, 0, sys_btn_text);
		else
			sys_btn_text = DefSysBtnText;
		sys_btn_text.Transf(CTRANSF_INNER_TO_OUTER);
		p_sys_btn->Title = sys_btn_text;
		// @v9.1.5 SendDlgItemMessage(H(), CTL_SCALE_SYSBTN, WM_SETTEXT, 0, (LPARAM)(const char *)p_sys_btn->Title);
		TView::SSetWindowText(GetDlgItem(H(), CTL_SCALE_SYSBTN), p_sys_btn->Title); // @v9.1.5
	}
}

IMPL_HANDLE_EVENT(ScaleDialog)
{
	TDialog::handleEvent(event);
	if(event.isCmd(cmScaleSys)) {
		long   flags = 0;
		PPID   scale_type_id = getCtrlLong(CTLSEL_SCALE_TYPE);
		GetClusterData(CTL_SCALE_FLAGS, &flags);
		if(scale_type_id == PPSCLT_CRCSHSRV || (!(flags & SCALF_TCPIP) && scale_type_id == PPSCLT_DIGI))
			editExpPaths();
		else
			editSysData();
	}
	else if(event.isCbSelected(CTLSEL_SCALE_TYPE))
		ReplyScaleTypeSelection(getCtrlLong(CTLSEL_SCALE_TYPE));
	else if(event.isCbSelected(CTLSEL_SCALE_GRP)) {
		PPID   prev_grp_id = Data.Rec.AltGoodsGrp;
		Data.Rec.AltGoodsGrp = getCtrlLong(CTLSEL_SCALE_GRP);
		if(Data.Rec.AltGoodsGrp) {
			PPObjGoodsGroup grp_obj;
			Goods2Tbl::Rec grp_rec;
			if(grp_obj.Fetch(Data.Rec.AltGoodsGrp, &grp_rec) > 0) {
				if(grp_rec.Kind != PPGDSK_GROUP || !(grp_rec.Flags & GF_ALTGROUP) || grp_rec.Flags & GF_DYNAMIC) {
					PPError(PPERR_INVSCALEGOODSGROUP);
					setCtrlLong(CTLSEL_SCALE_GRP, Data.Rec.AltGoodsGrp = prev_grp_id);
				}
			}
		}
	}
	else if(event.isCmd(cmClusterClk))
		ReplyScaleTypeSelection(getCtrlLong(CTLSEL_SCALE_TYPE));
	else
		return;
	clearEvent(event);
}

int ScaleDialog::setDTS(const PPScalePacket * pData)
{
	Data = *pData;
	SString temp_buf;
	//ExpPaths = pExpPaths;
	SetupStringCombo(this, CTLSEL_SCALE_TYPE, PPTXT_SCLT, Data.Rec.ScaleTypeID);
	setCtrlData(CTL_SCALE_NAME,   Data.Rec.Name);
	// @v10.5.7 {
	Data.GetExtStrData(Data.extssPort, temp_buf);
	setCtrlString(CTL_SCALE_PORT, temp_buf);
	// @v10.5.7 {
	/* @v10.5.7 if(Data.Rec.Flags & SCALF_TCPIP) {
		char   ip[16];
		memzero(ip, sizeof(ip));
		PPObjScale::DecodeIP(Data.Rec.Port, ip);
		setCtrlData(CTL_SCALE_PORT, ip);
	}
	else
		setCtrlData(CTL_SCALE_PORT, Data.Rec.Port); */
	setCtrlData(CTL_SCALE_PRTCLVER, &Data.Rec.ProtocolVer);
	setCtrlData(CTL_SCALE_ID,     &Data.Rec.ID);
	disableCtrl(CTL_SCALE_ID,     1 /* (int)Data.ID */);
	setCtrlData(CTL_SCALE_LOGNUM, &Data.Rec.LogNum);
	setCtrlData(CTL_SCALE_BCPREFIX, &Data.Rec.BcPrefix);
	Data.GetExtStrData(Data.extssAddedMsgSign, temp_buf);
	// @v10.5.7 setCtrlData(CTL_SCALE_ADDEDMSGSIGN, Data.Rec.AddedMsgSign);
	setCtrlString(CTL_SCALE_ADDEDMSGSIGN, temp_buf); // @v10.5.7
	setCtrlData(CTL_SCALE_ADDEDLINELEN, &Data.Rec.MaxAddedLn);
	setCtrlData(CTL_SCALE_ADDEDLNCT,    &Data.Rec.MaxAddedLnCount); // @v10.5.0
	AddClusterAssoc(CTL_SCALE_FLAGS, 0, SCALF_STRIPWP);
	AddClusterAssoc(CTL_SCALE_FLAGS, 1, SCALF_EXSGOODS);
	AddClusterAssoc(CTL_SCALE_FLAGS, 2, SCALF_TCPIP);
	AddClusterAssoc(CTL_SCALE_FLAGS, 3, SCALF_CHKINVPAR);
	AddClusterAssoc(CTL_SCALE_FLAGS, 4, SCALF_PASSIVE);
	SetClusterData(CTL_SCALE_FLAGS, Data.Rec.Flags);
	SetupPPObjCombo(this, CTLSEL_SCALE_LOC, PPOBJ_LOCATION, Data.Rec.Location, 0);
	SetupPPObjCombo(this, CTLSEL_SCALE_GRP, PPOBJ_GOODSGROUP, Data.Rec.AltGoodsGrp, OLW_CANINSERT, reinterpret_cast<void *>(GGRTYP_SEL_ALT)/* Alt Groups only */);
	SetupPPObjCombo(this, CTLSEL_SCALE_QUOT, PPOBJ_QUOTKIND, Data.Rec.QuotKindID, 0);
	SetupPPObjCombo(this, CTLSEL_SCALE_PARENT, PPOBJ_SCALE, Data.Rec.ParentID, 0, PPObjScale::MakeExtraParam(PPSCLT_SCALEGROUP, 0));
	ReplyScaleTypeSelection(Data.Rec.ScaleTypeID);
	return 1;
}

int ScaleDialog::getDTS(PPScalePacket * pData)
{
	int    ok = 1, sel = 0;
	//char   port[16];
	SString temp_buf;
	//memzero(port, sizeof(port));
	getCtrlData(sel = CTLSEL_SCALE_TYPE, &Data.Rec.ScaleTypeID);
	THROW_PP(Data.Rec.ScaleTypeID, PPERR_SCALETYPENEEDED);
	getCtrlData(sel = CTL_SCALE_NAME,   Data.Rec.Name);
	THROW_PP(*strip(Data.Rec.Name), PPERR_NAMENEEDED);
	getCtrlData(CTL_SCALE_ID,     &Data.Rec.ID);
	// @v10.5.7 getCtrlData(sel = CTL_SCALE_PORT,   port);
	// @v10.5.7 {
	getCtrlString(sel = CTL_SCALE_PORT, temp_buf);
	Data.PutExtStrData(Data.extssPort, temp_buf.Strip());
	// } @v10.5.7
	getCtrlData(sel = CTL_SCALE_LOGNUM,   &Data.Rec.LogNum);
	if(Data.Rec.ScaleTypeID == PPSCLT_DIGI)
		THROW_PP(Data.Rec.LogNum > 0 && Data.Rec.LogNum < 255, PPERR_SCALE_INVLOGNUM);
	getCtrlData(sel = CTL_SCALE_BCPREFIX, &Data.Rec.BcPrefix);
	THROW_PP(Data.Rec.BcPrefix == 0 || Data.Rec.IsValidBcPrefix(), PPERR_USERINPUT);
	getCtrlData(CTL_SCALE_PRTCLVER, &Data.Rec.ProtocolVer);
	getCtrlData(sel = CTLSEL_SCALE_LOC, &Data.Rec.Location);
	if(Data.Rec.ScaleTypeID != PPSCLT_SCALEGROUP)
		THROW_PP(Data.Rec.Location, PPERR_LOCNEEDED);
	getCtrlData(CTLSEL_SCALE_QUOT, &Data.Rec.QuotKindID); // AHTOXA
	getCtrlData(sel = CTLSEL_SCALE_GRP, &Data.Rec.AltGoodsGrp);
	if(Data.Rec.ScaleTypeID != PPSCLT_SCALEGROUP)
		THROW_PP(Data.Rec.AltGoodsGrp && PPObjGoodsGroup::IsAlt(Data.Rec.AltGoodsGrp), PPERR_SCALEGRPNEEDED);
	GetClusterData(CTL_SCALE_FLAGS, &Data.Rec.Flags);
	// @v10.5.7 getCtrlData(sel = CTL_SCALE_ADDEDMSGSIGN, Data.Rec.AddedMsgSign);
	// @v10.5.7 THROW_PP(PPGoodsPacket::ValidateAddedMsgSign(Data.Rec.AddedMsgSign, sizeof(Data.Rec.AddedMsgSign)), PPERR_INVPOSADDEDMSGSIGN);
	// @v10.5.7 {
	getCtrlString(sel = CTL_SCALE_ADDEDMSGSIGN, temp_buf);
	THROW_PP(PPGoodsPacket::ValidateAddedMsgSign(temp_buf, 64), PPERR_INVPOSADDEDMSGSIGN);
	Data.PutExtStrData(Data.extssAddedMsgSign, temp_buf);
	// } @v10.5.7 
	getCtrlData(CTL_SCALE_ADDEDLINELEN, &Data.Rec.MaxAddedLn);
	getCtrlData(CTL_SCALE_ADDEDLNCT,    &Data.Rec.MaxAddedLnCount); // @v10.5.0
	getCtrlData(CTLSEL_SCALE_PARENT, &Data.Rec.ParentID);
	/* @v10.5.7 if(!oneof2(Data.Rec.ScaleTypeID, PPSCLT_CRCSHSRV, PPSCLT_SCALEGROUP)) {
		if(Data.Rec.Flags & SCALF_TCPIP) {
			char   enc_ip[8];
			memzero(enc_ip, sizeof(enc_ip));
			THROW(PPObjScale::EncodeIP(port, enc_ip, sstrlen(port) + 1));
			STRNSCPY(Data.Rec.Port, enc_ip);
		}
		else if(Data.Rec.ScaleTypeID != PPSCLT_DIGI) {
			STRNSCPY(Data.Rec.Port, port);
			THROW(GetPort(Data.Rec.Port, 0));
		}
	}*/
	//rExpPaths = ExpPaths;
	ASSIGN_PTR(pData, Data);
	CATCHZOKPPERRBYDLG
	return ok;
}

int SLAPI PPObjScale::CheckDup(PPID objID, const PPScalePacket * pPack)
{
	if(pPack->Rec.Flags & SCALF_TCPIP || pPack->Rec.BcPrefix) {
		PPScale rec;
		SString buf1;
		SString buf2;
		for(PPID id = 0; EnumItems(&id, &rec) > 0;) {
			if(id != objID) {
				if(pPack->Rec.BcPrefix && rec.BcPrefix == pPack->Rec.BcPrefix)
					return PPSetError(PPERR_DUPSCALEBCPREFIX, rec.Name);
				else if(pPack->Rec.Flags & SCALF_TCPIP && rec.Flags & SCALF_TCPIP) {
					PPScalePacket test_pack;
					if(GetPacket(rec.ID, &test_pack) > 0) {
						//if(memcmp(rec.Port, pPack->Port, 4) == 0)
						const int fld_id_list[] = { PPScalePacket::extssPort };
						if(pPack->IsEqual(test_pack, SIZEOFARRAY(fld_id_list), fld_id_list))
							return PPSetError(PPERR_DUPSCALEIP, rec.Name);
					}
				}
			}
		}
	}
	return 1;
}

/*
struct PPScale2 {          // @persistent @store(Reference2Tbl+)
	long   Tag
	long   ID
	char   Name[48]
	char   Symb[20]
	PPID   ParentID
	char   AddedMsgSign[8]
	int16  MaxAddedLn
	int16  MaxAddedLnCount
	char   Reserve[8]
	char   Port[8]
	uint16 Get_NumTries
	uint16 Get_Delay
	uint16 Put_NumTries
	uint16 Put_Delay
	int16  BcPrefix
	uint16 Reserve2
	PPID   QuotKindID
	PPID   ScaleTypeID
	long   ProtocolVer
	long   LogNum
	long   Flags
	long   Location
	long   AltGoodsGrp
};
*/

int SLAPI PPObjScale::IsPacketEq(const PPScalePacket & rS1, const PPScalePacket & rS2, long flags)
{
#define CMP_MEMB(m)  if(rS1.Rec.m != rS2.Rec.m) return 0;
	CMP_MEMB(ID);
	CMP_MEMB(ParentID);
	CMP_MEMB(MaxAddedLn);
	CMP_MEMB(MaxAddedLnCount);
	CMP_MEMB(Get_NumTries);
	CMP_MEMB(Get_Delay);
	CMP_MEMB(Put_NumTries);
	CMP_MEMB(Put_Delay);
	CMP_MEMB(BcPrefix);
	//CMP_MEMB(Reserve2);
	CMP_MEMB(QuotKindID);
	CMP_MEMB(ScaleTypeID);
	CMP_MEMB(ProtocolVer);
	CMP_MEMB(LogNum);
	CMP_MEMB(Flags);
	CMP_MEMB(Location);
	CMP_MEMB(AltGoodsGrp);
#undef CMP_MEMB
	if(!sstreq(rS1.Rec.Name, rS2.Rec.Name))
		return 0;
	else if(!sstreq(rS1.Rec.Symb, rS2.Rec.Symb))
		return 0;
	/*else if(!sstreq(rS1.Rec.AddedMsgSign, rS2.Rec.AddedMsgSign))
		return 0;
	else if(!sstreq(rS1.Rec.Port, rS2.Rec.Port))
		return 0;*/
	else {
		SString t1, t2;
        rS1.GetExtStrData(PPScalePacket::extssAddedMsgSign, t1);
        rS2.GetExtStrData(PPScalePacket::extssAddedMsgSign, t2);
        if(t1 != t2)
			return 0;
		else {
			rS1.GetExtStrData(PPScalePacket::extssPort, t1);
			rS2.GetExtStrData(PPScalePacket::extssPort, t2);
			if(t1 != t2)
				return 0;
			else {
				rS1.GetExtStrData(PPScalePacket::extssPaths, t1);
				rS2.GetExtStrData(PPScalePacket::extssPaths, t2);
				if(t1 != t2)
					return 0;
			}
		}
	}
	return 1;
}

int SLAPI PPObjScale::PutPacket(PPID * pID, PPScalePacket * pPack, int use_ta)
{
	int    ok = 1;
	int    do_dirty = 0;
	PPID   id = DEREFPTRORZ(pID);
	PPID   hid = 0; // ���������� ������������� ��� ���������� � ��������� ������� (���� �� ������������)
	SString temp_buf;
	PPID   log_action_id = 0;
	PPScalePacket org_pack;
	SString ext_buffer;
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		if(id) {
			THROW(GetPacket(id, &org_pack) > 0);
			if(pPack == 0) {
				//
				// �������� ������
				//
				THROW(CheckRights(PPR_DEL));
				THROW(ref->RemoveItem(Obj, *pID, 0));
				do_dirty = 1;
			}
			else {
				//
				// ��������� ������
				//
				if(IsPacketEq(*pPack, org_pack, 0)) {
					//
					// ������ �� ����������
					//
					log_action_id = 0;
				}
				else {
					THROW(CheckRights(PPR_MOD));
					THROW(CheckDupName(*pID, pPack->Rec.Name));
					THROW(CheckDupSymb(*pID, pPack->Rec.Symb));
					pPack->Rec.Ver_Signature = DS.GetVersion();
					THROW(ref->UpdateItem(Obj, *pID, &pPack->Rec, 0, 0));
					(ext_buffer = pPack->GetBuffer()).Strip();
					THROW(ref->UtrC.SetText(TextRefIdent(Obj, id, PPTRPROP_SCALEEXT), ext_buffer.Transf(CTRANSF_INNER_TO_UTF8), 0));
					log_action_id = PPACN_OBJUPD;
					do_dirty = 1;
				}
			}
		}
		else if(pPack) {
			//
			// ������� ������ ������
			//
			THROW(CheckRights(PPR_INS));
			pPack->Rec.Ver_Signature = DS.GetVersion();
			THROW(ref->AddItem(Obj, pID, &pPack->Rec, 0));
			id = *pID;
			(ext_buffer = pPack->GetBuffer()).Strip();
			THROW(ref->UtrC.SetText(TextRefIdent(Obj, id, PPTRPROP_SCALEEXT), ext_buffer.Transf(CTRANSF_INNER_TO_UTF8), 0));
			log_action_id = PPACN_OBJADD;
		}
		if(log_action_id) {
			DS.LogAction(log_action_id, Obj, id, hid, 0);
		}
		THROW(tra.Commit());
	}
	if(do_dirty)
		Dirty(id);
	CATCHZOK
	return ok;
}

int SLAPI PPObjScale::GetPacket(PPID id, PPScalePacket * pPack)
{
	int    ok = -1;
	assert(pPack);
	pPack->Z();
	if(PPCheckGetObjPacketID(Obj, id)) {
		if(Search(id, &pPack->Rec) > 0) {
			{
				SString text_buf;
				THROW(ref->UtrC.GetText(TextRefIdent(Obj, id, PPTRPROP_SCALEEXT), text_buf));
				text_buf.Transf(CTRANSF_UTF8_TO_INNER);
				pPack->SetBuffer(text_buf.Strip());
			}
			ok = 1;
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPObjScale::Edit(PPID * pID, void * extraPtr)
{
	int    ok = 1;
	int    r = cmCancel;
	int    valid_data = 0;
	int    is_new = 0;
	PPScalePacket pack;
	ScaleDialog * dlg = 0;
	//SString paths;
	THROW(CheckDialogPtr(&(dlg = new ScaleDialog())));
	THROW(EditPrereq(pID, dlg, &is_new));
	if(!is_new) {
		THROW(GetPacket(*pID, &pack) > 0);
		/*if(oneof2(pack.Rec.ScaleTypeID, PPSCLT_CRCSHSRV, PPSCLT_DIGI))
			ref->GetPropVlrString(PPOBJ_SCALE, *pID, SCLPRP_EXPPATHS, paths);*/
	}
	else {
		SString temp_buf;
		for(int i = 1; i <= 32; i++) {
			// @v9.4.9 sprintf(scale.Name, "Scale #%d", i);
			(temp_buf = "Scale").Space().CatChar('#').Cat(i); // @v9.4.9
			STRNSCPY(pack.Rec.Name, temp_buf); // @v9.4.9
			if(CheckDupName(*pID, pack.Rec.Name))
				break;
			else
				MEMSZERO(pack.Rec.Name);
		}
	}
	dlg->setDTS(&pack);
	while(!valid_data && (r = ExecView(dlg)) == cmOK) {
		if(dlg->getDTS(&pack)) {
			if(*pID)
				*pID = pack.Rec.ID;
			if(!CheckName(*pID, pack.Rec.Name, 0))
				dlg->selectCtrl(CTL_SCALE_NAME);
			else if(!CheckDup(*pID, &pack))
				PPErrorByDialog(dlg, CTL_SCALE_PORT);
			else if(!PutPacket(pID, &pack, 1))
				PPError();
			else {
				SETIFZ(*pID, pack.Rec.ID);
				/*if(oneof2(scale.ScaleTypeID, PPSCLT_CRCSHSRV, PPSCLT_DIGI))
					THROW(ref->PutPropVlrString(PPOBJ_SCALE, *pID, SCLPRP_EXPPATHS, paths));*/
				Dirty(*pID);
				valid_data = 1;
			}
		}
	}
	CATCHZOKPPERR
	delete dlg;
	return ok ? r : 0;
}

int SLAPI PPObjScale::HandleMsg(int msg, PPID _obj, PPID _id, void * extraPtr)
{
	int    ok = DBRPL_OK;
	if(msg == DBMSG_OBJDELETE && oneof3(_obj, PPOBJ_GOODSGROUP, PPOBJ_QUOTKIND, PPOBJ_LOCATION)) {
		SArray list(sizeof(PPScale));
		int  r = ref->LoadItems(Obj, &list);
		if(r > 0) {
			PPScale * p_scale;
			for(uint i = 0; ok && list.enumItems(&i, (void **)&p_scale);) {
				if(_obj == PPOBJ_GOODSGROUP && p_scale->AltGoodsGrp == _id)
					ok = RetRefsExistsErr(Obj, p_scale->ID);
				else if(_obj == PPOBJ_QUOTKIND && p_scale->QuotKindID == _id)
					ok = RetRefsExistsErr(Obj, p_scale->ID);
				else if(_obj == PPOBJ_LOCATION && p_scale->Location == _id)
					ok = RetRefsExistsErr(Obj, p_scale->ID);
			}
		}
		else if(r == 0)
			ok = DBRPL_ERROR;
	}
	return ok;
}

int SLAPI PPObjScale::SerializePacket(int dir, PPScalePacket * pPack, SBuffer & rBuf, SSerializeContext * pSCtx)
{
	int    ok = 1;
	THROW_SL(ref->SerializeRecord(dir, &pPack->Rec, rBuf, pSCtx));
	THROW(pPack->PPExtStrContainer::SerializeB(dir, rBuf, pSCtx));
	CATCHZOK
	return ok;
}

IMPL_DESTROY_OBJ_PACK(PPObjScale, PPScalePacket);

int SLAPI PPObjScale::Read(PPObjPack * p, PPID id, void * stream, ObjTransmContext * pCtx)
	{ return Implement_ObjReadPacket<PPObjScale, PPScalePacket>(this, p, id, stream, pCtx); }

int SLAPI PPObjScale::Write(PPObjPack * p, PPID * pID, void * stream, ObjTransmContext * pCtx)
{
	int    ok = 1, r;
	if(p && p->Data) {
		PPScalePacket * p_pack = static_cast<PPScalePacket *>(p->Data);
		if(stream == 0) {
			if(*pID == 0) {
				PPID   same_id = 0;
				PPScale same_rec;
				if(p_pack->Rec.ID < PP_FIRSTUSRREF) {
					if(Search(p_pack->Rec.ID, &same_rec) > 0) {
						*pID = same_id = p_pack->Rec.ID;
						ok = 1;
					}
				}
				else if(p_pack->Rec.Symb[0] && SearchBySymb(p_pack->Rec.Symb, &same_id, &same_rec) > 0) {
					*pID = same_id;
					ok = 1;
				}
				else if(p_pack->Rec.Name[0] && SearchByName(p_pack->Rec.Name, &same_id, &same_rec) > 0) {
					*pID = same_id;
					ok = 1;
				}
				else {
					same_id = p_pack->Rec.ID = 0;
				}
				if(same_id == 0) {
					p_pack->Rec.ID = 0;
					r = PutPacket(pID, p_pack, 1);
					if(r == 0) {
						pCtx->OutputAcceptErrMsg(PPTXT_ERRACCEPTSCALE, p_pack->Rec.ID, p_pack->Rec.Name);
						ok = -1;
					}
					else if(r > 0)
						ok = 1; // 101; // @ObjectCreated
					else
						ok = 1;
				}
			}
			else {
				p_pack->Rec.ID = *pID;
				r = PutPacket(pID, p_pack, 1);
				if(r == 0) {
					pCtx->OutputAcceptErrMsg(PPTXT_ERRACCEPTSCALE, p_pack->Rec.ID, p_pack->Rec.Name);
					ok = -1;
				}
				else if(r > 0)
					ok = 1; // 102; // @ObjectUpdated
				else
					ok = 1;
			}
		}
		else {
			SBuffer buffer;
			THROW(SerializePacket(+1, p_pack, buffer, &pCtx->SCtx));
			THROW_SL(buffer.WriteToFile(static_cast<FILE *>(stream), 0, 0))
		}
	}
	else
		ok = -1;
	CATCHZOK
	return ok;
}

int SLAPI PPObjScale::ProcessObjRefs(PPObjPack * p, PPObjIDArray * ary, int replace, ObjTransmContext * pCtx)
{
	int    ok = 1;
	if(p && p->Data) {
		PPScalePacket * p_pack = static_cast<PPScalePacket *>(p->Data);
		THROW(ProcessObjRefInArray(PPOBJ_SCALE, &p_pack->Rec.ParentID, ary, replace));
		THROW(ProcessObjRefInArray(PPOBJ_LOCATION, &p_pack->Rec.Location, ary, replace));
		THROW(ProcessObjRefInArray(PPOBJ_QUOTKIND, &p_pack->Rec.QuotKindID, ary, replace));
	}
	else
		ok = -1;
	CATCHZOK
	return ok;
}

int SLAPI PPObjScale::GetListWithBcPrefix(LAssocArray * pList)
{
	int    ok = -1;
	PPScale rec;
	for(PPID id = 0; EnumItems(&id, &rec) > 0;) {
		if(rec.IsValidBcPrefix()) {
			CALLPTRMEMB(pList, AddUnique(rec.ID, rec.BcPrefix, 0));
			ok = 1;
		}
	}
	return ok;
}
//
// PPObjScale::PrepareData
//
static int SLAPI GetOutputFileName(PPID id, SString & rBuf)
{
	SString path;
	path.Cat("SC").CatLongZ(id, 6).Dot().Cat("PLU");
	return BIN(PPGetFilePath(PPPATH_OUT, path, rBuf));
}

int SLAPI PPObjScale::IsPassive(PPID id, const PPScale * pScale)
{
	int    ok = 0;
	PPScale scale;
	if(!RVALUEPTR(scale, pScale)) {
		THROW(Search(id, &scale) > 0);
	}
	ok = BIN(scale.Flags & SCALF_PASSIVE);
	if(!ok && scale.ParentID) {
		THROW(Search(scale.ParentID, &scale) > 0);
		ok = BIN(scale.Flags & SCALF_PASSIVE);
	}
	CATCHZOK
	return ok;
}

// static
void * FASTCALL PPObjScale::MakeExtraParam(long onlyGroups, long groupID)
{
	return reinterpret_cast<void *>((onlyGroups << 24) | groupID);
}

StrAssocArray * SLAPI PPObjScale::MakeStrAssocList(void * extraPtr)
{
	const  long extra_param = reinterpret_cast<long>(extraPtr);
	PPID   scale_type = extra_param >> 24;
	PPID   group_id   = (extra_param & 0x00ffffff);
	PPID   id = 0;
	StrAssocArray * p_list = new StrAssocArray();
	THROW_MEM(p_list);
	{
		PPScale rec;
		for(SEnum en = ref->Enum(Obj, 0); en.Next(&rec) > 0;) {
			if((!scale_type || scale_type == rec.ScaleTypeID) && (!group_id || rec.ParentID == group_id)) {
				if(*strip(rec.Name) == 0)
					ideqvalstr(rec.ID, rec.Name, sizeof(rec.Name));
				THROW_SL(p_list->Add(rec.ID, rec.ParentID, rec.Name));
			}
		}
	}
	p_list->SortByText();
	CATCH
		ZDELETE(p_list);
	ENDCATCH
	return p_list;
}

static const char * P_ScalePrepareFormatSignature = "PPSCPF"; // @persistent

int SLAPI PPObjScale::PrepareData(PPID id, long flags, PPLogger * pLogger)
{
	int    ok = -1;
	PPViewGoodsRest * grv = 0;
	PPScalePacket pack;
	THROW(GetPacket(id, &pack) > 0);
	if(!IsPassive(id, &pack.Rec)) {
		if(pack.Rec.ScaleTypeID == PPSCLT_SCALEGROUP) {
			StrAssocArray * p_list = MakeStrAssocList(PPObjScale::MakeExtraParam(0, id));
			if(p_list) {
				for(uint i = 0; i < p_list->getCount(); i++) {
					int r = PrepareData(p_list->Get(i).Id, flags|fTrSkipListing, pLogger); // @recursion
					ok = (ok != 0) ? r : ok;
				}
			}
			ZDELETE(p_list);
		}
		else if(pack.Rec.ScaleTypeID != PPSCLT_WEIGHTTERM) {
			int    barcode_kind = 0;
			char   wp[16], cp[16];
			SString temp_buf;
			SString fmt_buf;
			SString msg_buf;
			SString msg_buf2;
			SString path;
			SString bc_buf;
			PPObjGoods goods_obj;
			GoodsRestFilt flt;
			PPEquipConfig eq_cfg;
			PPWait(1);
			ReadEquipConfig(&eq_cfg);
			strip(strcpy(wp, goods_obj.GetConfig().WghtPrefix));
			strip(strcpy(cp, goods_obj.GetConfig().WghtCntPrefix));
			if(pack.Rec.IsValidBcPrefix() && goods_obj.GetConfig().Flags & GCF_USESCALEBCPREFIX)
				barcode_kind = 2;
			else if(goods_obj.GetConfig().Flags & GCF_LOADTOSCALEGID) {
				barcode_kind = 1;
			}
			PPWaitMsg(msg_buf.Printf(PPLoadTextS(PPTXT_SCALE_PREPARE, fmt_buf), pack.Rec.Name));
			CALLPTRMEMB(pLogger, Log(msg_buf));
			THROW_PP(pack.Rec.AltGoodsGrp && PPObjGoodsGroup::IsAlt(pack.Rec.AltGoodsGrp), PPERR_NOTALTGRP);
			flt.Flags     |= GoodsRestFilt::fBarCode;
			if(!(pack.Rec.Flags & SCALF_EXSGOODS))
				flt.Flags |= GoodsRestFilt::fNullRest;
			flt.AmtType    = 2; // � ����� ����������
			flt.CalcMethod = GoodsRestParam::pcmMostRecent; // (LConfig.RealizeOrder == RLZORD_FIFO) ? 1 : 2;
			flt.LocList.Add(pack.Rec.Location);
			flt.GoodsGrpID = pack.Rec.AltGoodsGrp;
			THROW_MEM(grv = new PPViewGoodsRest);
			THROW(grv->Init_(&flt));
			THROW(GetOutputFileName(pack.Rec.ID, path));
			{
				const  size_t wpl = sstrlen(wp);
				const  size_t cpl = sstrlen(cp);
				char   barcode[32];
				PPGoodsPacket  gds_pack;
				GoodsRestViewItem gr_item;
				RetailPriceExtractor::ExtQuotBlock eqb(pack.Rec.QuotKindID);
				RetailPriceExtractor rpe(pack.Rec.Location, &eqb, 0, ZERODATETIME, 0);
				SString line_buf;
				SFile out_file(path, SFile::mWrite);
				THROW_SL(out_file.IsValid());
				{
					char   ver_text[64];
					PPVersionInfo vi = DS.GetVersionInfo();
					vi.GetVersionText(ver_text, sizeof(ver_text));
					line_buf.Z().Cat(P_ScalePrepareFormatSignature).Space().Cat(ver_text).CR();
					out_file.WriteLine(line_buf);
				}
				for(grv->InitIteration(PPViewGoodsRest::OrdByGoodsName); grv->NextIteration(&gr_item) > 0;) {
					int    is_wp = 0;
					int    to_export = 1;
					int    r2 = -1;
					RetailExtrItem  rtl_ext_item;
					ScalePLU plu;
					plu.GoodsID = gr_item.GoodsID;
					// @v9.2.2 grv->GetGoodsNumByAlterGroup(gr_item.GoodsID, scale.AltGoodsGrp, &plu.GoodsNo);
					goods_obj.P_Tbl->GetGoodsCodeInAltGrp(gr_item.GoodsID, pack.Rec.AltGoodsGrp, &plu.GoodsNo); // @v9.2.2
					plu.GrpCode = pack.Rec.AltGoodsGrp;
					if(barcode_kind == 2 && (r2 = goods_obj.GenerateScaleBarcode(gr_item.GoodsID, pack.Rec.ID, bc_buf)) > 0) {
						bc_buf.CopyTo(barcode, sizeof(barcode));
						is_wp = 1;
					}
					else if(barcode_kind == 1) {
			#define PETROVICH_GOODS_ID_BIAS 0 // Same as in Petrovi.cpp // @v6.3.4 200-->0
						sprintf(barcode, "%s%05ld", wp, gr_item.GoodsID+PETROVICH_GOODS_ID_BIAS);
						is_wp = 1;
					}
					else {
						grv->GetGoodsBarcode(gr_item.GoodsID, barcode, sizeof(barcode));
						is_wp = goods_obj.GetConfig().IsWghtPrefix(barcode);
						if(is_wp == 2)
							plu.Flags |= plu.fCountable;
					}
					if(r2 == 0)
						PPLogMessage(PPFILNAM_ERR_LOG, 0, LOGMSGF_LASTERR|LOGMSGF_TIME|LOGMSGF_USER);
					THROW(rpe.GetPrice(gr_item.GoodsID, 0, 0.0, &rtl_ext_item));
					if(pack.Rec.Flags & SCALF_CHKINVPAR) {
						if(barcode_kind == 0 && !is_wp) {
							if(barcode[0] == 0)
								strcpy(barcode, "0");
							PPFormatT(PPTXT_LOG_SCALE_INVBARCODE, &msg_buf2, gr_item.GoodsID, barcode);
							to_export = 0;
						}
						if(to_export && rtl_ext_item.Expiry != ZERODATE && diffdate(rtl_ext_item.Expiry, getcurdate_()) < 0) {
							PPFormatT(PPTXT_LOG_SCALE_INVEXPIRY, &msg_buf2, gr_item.GoodsID, rtl_ext_item.Expiry);
							to_export = 0;
						}
					}
					if(to_export) {
						if(pack.Rec.Flags & SCALF_STRIPWP && barcode_kind == 0)
							strcpy(barcode, barcode + wpl);
						const size_t bclen = sstrlen(barcode);
						if(bclen > 6)
							strcpy(barcode, barcode + bclen - 6);
						plu.Barcode = (long)atof(barcode);
						const double price_ = (eq_cfg.Flags & eq_cfg.fUncondAsyncBasePrice) ? rtl_ext_item.BasePrice : rtl_ext_item.Price;
						plu.Price  = R2(rtl_ext_item.ExtPrice ? rtl_ext_item.ExtPrice : price_);
						plu.Expiry = rtl_ext_item.Expiry;
						plu.GoodsName = gr_item.GoodsName;
						plu.GoodsName.ReplaceChar('\t', ' ');
						goods_obj.GetPacket(plu.GoodsID, &gds_pack, PPObjGoods::gpoSkipQuot);
						line_buf.Z();
						line_buf.Cat(plu.GoodsID).Tab();
						line_buf.Cat(plu.GoodsNo).Tab();
						line_buf.Cat(plu.GrpCode).Tab();
						line_buf.Cat(plu.Barcode).Tab();
						line_buf.Cat(plu.Price).Tab();
						line_buf.Cat(plu.Expiry.v).Tab();
						line_buf.Cat(plu.Flags).Tab();
						line_buf.Cat(plu.GoodsName).Tab();
						{
							StringSet ss(SLBColumnDelim);
							pack.GetExtStrData(pack.extssAddedMsgSign, temp_buf);
							gds_pack.PrepareAddedMsgStrings(/*pack.Rec.AddedMsgSign*/temp_buf, 0, &rtl_ext_item.ManufDtm, ss);
							plu.AddMsgBuf = ss.getBuf();
						}
						line_buf.Cat(plu.AddMsgBuf).CR();
						out_file.WriteLine(line_buf);
					}
					else if(pLogger)
						pLogger->Log(msg_buf2);
					PPWaitPercent(grv->GetCounter(), msg_buf);
				}
				THROW_DB(BTROKORNFOUND);
			}
			PPWait(0);
			if(!(flags & fTrSkipListing) && PPMessage(mfConf|mfYesNo, PPCFM_PRINTEXPLP15) == cmYes)
				THROW(grv->Print(0));
			ok = 1;
		}
	}
	CATCH
		ok = 0;
		if(pLogger) {
			pLogger->LogLastError();
			PPWait(0);
		}
		else
			PPError();
	ENDCATCH
	delete grv;
	return ok;
}

void SLAPI PPObjScale::InitStat()
{
	MEMSZERO(StatBuf);
}

void SLAPI PPObjScale::GetStat(Stat * pStat) const
{
	ASSIGN_PTR(pStat, StatBuf);
}
//
// PPObjScale::TransmitData
//
int SLAPI PPObjScale::SendPlu(PPScalePacket * pScaleData, const char * pFileName, int updateOnly, PPLogger * pLogger)
{
	int    ok = 1;
	int    port = 0;
	int    connection_opened = 0;
	long   success_count = 0;
	PPID   stat_id = 0;
	char   countbuf[16];
	SString fmt_buf, msg_buf;
	PPScaleDevice * p_comm = 0;
	ScalePLU * p_plu = 0;
	DeviceLoadingStat dls;
	PPIDArray update_list;
	LDATETIME since;
	TSCollection <ScalePLU> load_list;
	if(updateOnly) {
		since.Set(getcurdate_(), ZEROTIME);
		dls.GetUpdatedObjects(PPOBJ_GOODS, since, &update_list);
		update_list.sort();
	}
	THROW(p_comm = GetScaleDevice(pScaleData));
	InitStat();
	countbuf[0] = 0;
	PPWaitMsg(msg_buf.Printf(PPLoadTextS(PPTXT_SCALE_TRANSMIT, fmt_buf), pScaleData->Rec.Name, countbuf));
	CALLPTRMEMB(pLogger, Log(msg_buf));
	{
		SString line_buf;
		SFile in_stream(pFileName, SFile::mRead);
		THROW_PP(in_stream.IsValid(), PPERR_SCALE_FOPEN);
		if(in_stream.ReadLine(line_buf)) {
			line_buf.Chomp();
			if(line_buf.CmpPrefix(P_ScalePrepareFormatSignature, 0) == 0) {
				StringSet ss_rec("\t");
				SString fld_buf;
				while(in_stream.ReadLine(line_buf)) {
					if(line_buf.Chomp().Strip().Len()) {
						int    to_load = 1;
						THROW_MEM(p_plu = new ScalePLU);
						ss_rec.clear();
						ss_rec.setBuf(line_buf, line_buf.Len()+1);
						uint   fld_no = 0;
						for(uint   pos = 0; ss_rec.get(&pos, fld_buf); fld_no++) {
							switch(fld_no) {
								case 0: p_plu->GoodsID = fld_buf.ToLong(); break;
								case 1: p_plu->GoodsNo = fld_buf.ToLong(); break;
								case 2: p_plu->GrpCode = fld_buf.ToLong(); break;
								case 3: p_plu->Barcode = fld_buf.ToLong(); break;
								case 4: p_plu->Price   = fld_buf.ToReal(); break;
								case 5: p_plu->Expiry.v = strtoul(fld_buf.Strip(), 0, 10); break;
								case 6: p_plu->Flags = fld_buf.ToLong(); break;
								case 7: p_plu->GoodsName = fld_buf.Strip(); break;
								case 8: p_plu->AddMsgBuf = fld_buf.Strip(); break;
							}
						}
						if(updateOnly && !update_list.bsearch(p_plu->GoodsID)) {
							DlsObjTbl::Rec dlso_rec;
							if(dls.GetLastObjInfo(dvctScales, pScaleData->Rec.ID, PPOBJ_GOODS, p_plu->GoodsID, since.d, &dlso_rec) > 0)
								if(dbl_cmp(dlso_rec.Val, p_plu->Price) == 0)
									to_load = 0;
						}
						if(to_load) {
							THROW_SL(load_list.insert(p_plu));
							p_plu = 0;
						}
						else
							ZDELETE(p_plu);
					}
				}
			}
		}
	}
	THROW(p_comm->SetConnection());
	connection_opened = 1;
	{
		dls.StartLoading(&stat_id, dvctScales, pScaleData->Rec.ID, 1);
		PROFILE_START
		for(uint i = 0; i < load_list.getCount(); i++) {
			const ScalePLU * p_item = load_list.at(i);
			ok = 0;
			uint   collision = 0;
			for(uint j = 0; !ok && j < 10; j++) {
				if(p_comm->SendPLU(p_item)) {
					ok = 1;
				}
				else {
					collision++;
					SDelay(100);
				}
			}
			if(collision) {
				StatBuf.NumSendPluColl++;
				if(StatBuf.MaxSendPluCollIters < collision)
					StatBuf.MaxSendPluCollIters = collision;
			}
			if(stat_id) {
				DeviceLoadingStat::GoodsInfo item_info;
				item_info.ID = p_item->GoodsID;
				STRNSCPY(item_info.Name, p_item->GoodsName);
				item_info.PLU   = p_item->GoodsNo;
				item_info.Price = p_item->Price;
				dls.RegisterGoods(stat_id, &item_info);
			}
			if(!ok) {
				PPFormatT(PPTXT_LOG_SCALELOADERR, &msg_buf, p_item->GoodsID, p_item->GoodsID, p_item->GoodsNo, p_item->Price);
				if(pLogger)
					pLogger->Log(msg_buf);
				else
					PPLogMessage(PPFILNAM_SCALE_LOG, msg_buf, LOGMSGF_USER|LOGMSGF_TIME);
			}
			else
				success_count++;
			// @v10.2.2 THROW(ok);
			PPWaitPercent(i+1, load_list.getCount(), msg_buf.Printf(fmt_buf, pScaleData->Rec.Name, countbuf));
		}
		PROFILE_END
		if(stat_id)
			dls.FinishLoading(stat_id, 1, 1);
	}
	CATCHZOK
	if(p_comm) {
		StatBuf.NumGetColl      = p_comm->NumGetColl;
		StatBuf.MaxGetCollIters = p_comm->MaxGetCollIters;
		if(CConfig.Flags & CCFLG_DEBUG) {
			PPFormatT(PPTXT_LOG_SCALELOADSTAT, &msg_buf, StatBuf.NumSendPluColl, StatBuf.MaxSendPluCollIters, StatBuf.NumGetColl, StatBuf.MaxGetCollIters);
			if(pLogger)
				pLogger->Log(msg_buf);
			else
				PPLogMessage(PPFILNAM_SCALE_LOG, msg_buf, LOGMSGF_USER|LOGMSGF_TIME);
		}
	}
	if(connection_opened) {
		if(p_comm->CloseConnection())
			PPLogMessage(PPFILNAM_SCALE_LOG, "Scale is loaded", LOGMSGF_USER|LOGMSGF_TIME);
		connection_opened = 0;
	}
	delete p_comm;
	delete p_plu;
	CALLPTRMEMB(pLogger, Log(msg_buf.Printf(PPLoadTextS(PPTXT_LOG_SCALELOADED, fmt_buf), pScaleData->Rec.Name, success_count)));
	return ok;
}

int SLAPI PPObjScale::TransmitData(PPID id, long flags, PPLogger * pLogger)
{
	int    ok = -1;
	PPScalePacket pack;
	THROW(GetPacket(id, &pack) > 0);
	PPSetAddedMsgString(pack.Rec.Name);
	if(IsPassive(id, &pack.Rec) == 0) {
		if(pack.Rec.ScaleTypeID == PPSCLT_SCALEGROUP) {
			StrAssocArray * p_list = MakeStrAssocList(PPObjScale::MakeExtraParam(0, id));
			if(p_list) {
				for(uint i = 0; i < p_list->getCount(); i++) {
					int r = TransmitData(p_list->Get(i).Id, flags|fTrSkipListing, pLogger); // @recursion
					ok = (ok != 0) ? r : ok;
				}
			}
			ZDELETE(p_list);
		}
		else if(pack.Rec.ScaleTypeID != PPSCLT_WEIGHTTERM) {
			int    port, todo = 1;
			SString fname, nname, line_buf;
			long   sPL_ = 0x007E4C50L; // "PL~"
			int    do_prepare = 0;
			THROW(GetOutputFileName(id, fname));
			if(fileExists(fname)) {
				LDATETIME mt;
				if(!SFile::GetTime(fname, 0, 0, &mt) || diffdatetimesec(getcurdatetime_(), mt) > 3600) {
					//
					// ���� ���� ������ �������� ������� ����� ��� �� ���, �� �� ����� ���� ���������� - ������� �����.
					//
					do_prepare = 1;
				}
				else {
					//
					// ���� ���� ��� ����������, �� ���������� ��������� ��� �� �������������
					// � ����������� ������� __MinPrepFileVer.
					//
					SFile in_stream(fname, SFile::mRead);
					THROW_PP(in_stream.IsValid(), PPERR_SCALE_FOPEN);
					if(in_stream.ReadLine(line_buf)) {
						line_buf.Chomp();
						if(line_buf.CmpPrefix(P_ScalePrepareFormatSignature, 0) == 0) {
							line_buf.Excise(0, sstrlen(P_ScalePrepareFormatSignature));
							line_buf.Strip();
							SVerT ver;
							if(!ver.FromStr(line_buf) || ver.Cmp(&__MinPrepFileVer) < 0)
								do_prepare = 1;
						}
					}
				}
			}
			else
				do_prepare = 1;
			if(do_prepare) {
				if(!PrepareData(id, flags, pLogger))
					todo = 0;
				else {
					THROW(GetOutputFileName(id, fname));
				}
			}
			if(todo) {
				PPWait(1);
				if(pack.Rec.ScaleTypeID != PPSCLT_CRCSHSRV && pack.Rec.ScaleTypeID != PPSCLT_DIGI) {
					SString port_buf;
					pack.GetExtStrData(pack.extssPort, port_buf);
					if(pack.Rec.Flags & SCALF_TCPIP) {
						//THROW(PPObjScale::DecodeIP(pack.Rec.Port, 0));
					}
					else {
						THROW(GetPort(port_buf, &port));
					}
				}
				THROW(SendPlu(&pack, fname, BIN(flags & fTrUpdateOnly), pLogger));
				SPathStruc::ReplaceExt(nname = fname, reinterpret_cast<const char *>(&sPL_), 1);
				SFile::Remove(nname);
				SFile::Rename(fname, nname);
				PPWait(0);
				ok = 1;
			}
		}
	}
	CATCH
		ok = 0;
		if(pLogger) {
			pLogger->LogLastError();
			PPWait(0);
		}
		else
			PPError();
	ENDCATCH
	return ok;
}

static PPScaleDevice * SLAPI GetScaleDevice(const PPScalePacket * pScaleData)
{
	int    port = 0;
	PPScaleDevice * p_comm = 0;
	if(oneof2(pScaleData->Rec.ScaleTypeID, PPSCLT_CRCSHSRV, PPSCLT_DIGI)) {
		//SString paths;
		//THROW(PPRef->GetPropVlrString(PPOBJ_SCALE, pScaleData->Rec.ID, SCLPRP_EXPPATHS, paths));
		if(pScaleData->Rec.ScaleTypeID == PPSCLT_CRCSHSRV)
			p_comm = new CrystalCashServer(pScaleData);
		else
			p_comm = new DIGI(pScaleData);
	}
	else {
		SString port_buf;
		pScaleData->GetExtStrData(PPScalePacket::extssPort, port_buf);
		if(pScaleData->Rec.Flags & SCALF_TCPIP) {
			//THROW(PPObjScale::DecodeIP(pScaleData->Rec.Port, 0));
		}
		else {
			THROW(GetPort(port_buf, &port));
		}
		if(pScaleData->Rec.ScaleTypeID == PPSCLT_CAS)
			p_comm = new CommLP15(port, pScaleData);
		else if(pScaleData->Rec.ScaleTypeID == PPSCLT_MASSAK) {
			if(oneof2(pScaleData->Rec.ProtocolVer, 10, 11))
				p_comm = new COMMassaKVPN(port, pScaleData);
			else if(pScaleData->Rec.LogNum)
				p_comm = new COMMassaK(port, pScaleData);
			else if(pScaleData->Rec.ProtocolVer == 20) // @vmiller ����� �� �����
				p_comm = new COMMassaKVer1(port, pScaleData);
			else
				p_comm = new CommMassaK(port, pScaleData);
		}
		else if(pScaleData->Rec.ScaleTypeID == PPSCLT_MTOLEDO)
			p_comm = new TCPIPMToledo(port, pScaleData);
		else if(pScaleData->Rec.ScaleTypeID == PPSCLT_BIZERBA)
			p_comm = new Bizerba(port, pScaleData);
		else if(pScaleData->Rec.ScaleTypeID == PPSCLT_WEIGHTTERM)
			p_comm = new WeightTerm(port, pScaleData);
		else if(pScaleData->Rec.ScaleTypeID == PPSCLT_SHTRIHPRINT)
			p_comm = new ShtrihPrint(port, pScaleData);
		else if(pScaleData->Rec.ScaleTypeID == PPSCLT_CASCL5000J)
			p_comm = new CasCL5000J(port, pScaleData);
		else if(pScaleData->Rec.ScaleTypeID == PPSCLT_EXPORTTOFILE)
			p_comm = new ExportToFile(port, pScaleData);
		else if(pScaleData->Rec.ScaleTypeID == PPSCLT_SHTRIHCE)
			p_comm = new ShtrihCE(port, pScaleData);
		else {
			CALLEXCEPT_PP(PPERR_INVPARAM);
		}
	}
	THROW_PP(p_comm, PPERR_NOMEM);
	CATCH
		p_comm = 0;
	ENDCATCH
	return p_comm;
}

struct ScalePrepData {
	PPID   ScaleID;
	long   Flags;
};

class ScalePrepDlg : public TDialog {
public:
	ScalePrepDlg(uint rezID) : TDialog(rezID)
	{
	}
	int    setDTS(const ScalePrepData *);
	int    getDTS(ScalePrepData *);
private:
	DECL_HANDLE_EVENT;
	int    SetReadyStatus();
	PPObjScale ScaleObj;
	ScalePrepData Data;
};

int ScalePrepDlg::setDTS(const ScalePrepData * pData)
{
	if(!RVALUEPTR(Data, pData))
		MEMSZERO(Data);
	if(Data.ScaleID == 0) {
		PPID   temp_id = 0;
		if(ScaleObj.EnumItems(&temp_id) > 0) {
			Data.ScaleID = temp_id;
			if(ScaleObj.EnumItems(&temp_id) > 0)
				Data.ScaleID = 0;
		}
	}
	SetupPPObjCombo(this, CTLSEL_SCALEPREP_SCALE, PPOBJ_SCALE, Data.ScaleID, OLW_CANSELUPLEVEL, 0);
	AddClusterAssoc(CTL_SCALEPREP_FLAGS, 0, PPObjScale::fTrUpdateOnly);
	SetClusterData(CTL_SCALEPREP_FLAGS, Data.Flags);
	SetReadyStatus();
	return 1;
}

int ScalePrepDlg::getDTS(ScalePrepData * pData)
{
	getCtrlData(CTLSEL_SCALEPREP_SCALE, &Data.ScaleID);
	GetClusterData(CTL_SCALEPREP_FLAGS, &Data.Flags);
	ASSIGN_PTR(pData, Data);
	return 1;
}

int ScalePrepDlg::SetReadyStatus()
{
	if(Id == DLG_SCALETRAN) {
		int    is_ready = -1;
		SString ready;
		PPScalePacket pack;
		if(ScaleObj.GetPacket(Data.ScaleID, &pack) > 0 && (pack.Rec.Flags & SCALF_TCPIP) && ScaleObj.IsPassive(Data.ScaleID, &pack.Rec) == 0) {
			//char   ip[16];
			//memzero(ip, sizeof(ip));
			//if(PPObjScale::DecodeIP(scale.Port, ip)) {
			{
				SString port_buf;
				pack.GetExtStrData(pack.extssPort, port_buf);
				PPGetSubStr(PPTXT_SCALEREADY, 2, ready);
				setStaticText(CTL_SCALEPREP_READY, ready);
				is_ready = PPObjScale::CheckForConnection(port_buf, 1000, 5);
				PPGetSubStr(PPTXT_SCALEREADY, is_ready ? 0 : 1, ready);
				SetCtrlBitmap(CTL_SCALEPREP_READYCOLOR, is_ready ? BM_GREEN : BM_RED);
			}
		}
		showCtrl(CTL_SCALEPREP_READYCOLOR, (is_ready < 0) ? 0 : 1);
		setStaticText(CTL_SCALEPREP_READY, ready);
	}
	return 1;
}

IMPL_HANDLE_EVENT(ScalePrepDlg)
{
	TDialog::handleEvent(event);
	if(event.isCbSelected(CTLSEL_SCALEPREP_SCALE)) {
		getCtrlData(CTLSEL_SCALEPREP_SCALE, &Data.ScaleID);
		SetReadyStatus();
		clearEvent(event);
	}
}

int SLAPI ScalePrepDialog(uint rezID, PPID * pScaleID, long * pFlags)
{
	int    r  = -1;
	ScalePrepData data;
	data.ScaleID = *pScaleID;
	data.Flags = DEREFPTRORZ(pFlags);
	ScalePrepDlg * dlg = 0;
	if(CheckDialogPtrErr(&(dlg = new ScalePrepDlg(rezID)))) {
		dlg->setDTS(&data);
		if(ExecView(dlg) == cmOK) {
			dlg->getDTS(&data);
			*pScaleID = data.ScaleID;
			ASSIGN_PTR(pFlags, data.Flags);
			r = 1;
		}
		delete dlg;
	}
	else
		r = 0;
	return r;
}

// static
int SLAPI PPObjScale::PrepareData(PPID scaleID)
{
	PPID   id = scaleID;
	long   flags = 0;
	PPObjScale sobj;
	while(ScalePrepDialog(DLG_SCALEPREP, &id, &flags) > 0) {
		PPLogger logger;
		if(id == 0) {
			while(sobj.EnumItems(&id) > 0) {
				PPScale rec;
				sobj.Search(id, &rec);
				if(rec.ScaleTypeID != PPSCLT_SCALEGROUP)
					sobj.PrepareData(id, fTrSkipListing, &logger);
			}
			break;
		}
		else
			sobj.PrepareData(id, 0, &logger);
		logger.Save(PPFILNAM_SCALE_LOG, 0);
	}
	return 1;
}

// static
int SLAPI PPObjScale::TransmitData(PPID scaleID)
{
	PPID   id = scaleID;
	long   flags = 0;
	PPObjScale sobj;
	while(ScalePrepDialog(DLG_SCALETRAN, &id, &flags) > 0) {
		PPLogger logger;
		if(id == 0) {
			while(sobj.EnumItems(&id) > 0) {
				PPScalePacket pack;
				sobj.Fetch(id, &pack);
				if(pack.Rec.ScaleTypeID != PPSCLT_SCALEGROUP)
					sobj.TransmitData(id, flags | fTrSkipListing, &logger);
			}
			break;
		}
		else
			sobj.TransmitData(id, flags, &logger);
		logger.Save(PPFILNAM_SCALE_LOG, 0);
	}
	return 1;
}

int SLAPI GetScaleData(PPID scaleID, TIDlgInitData * pData)
{
	int    ok = -1;
	TIDlgInitData   tidi;
	PPScaleDevice * p_scale = 0;
	if(scaleID) {
		PPObjScale sc_obj;
		PPScalePacket pack;
		THROW(sc_obj.GetPacket(scaleID, &pack) > 0);
		if(pack.Rec.ScaleTypeID == PPSCLT_WEIGHTTERM) {
			THROW_MEM(p_scale = new WeightTerm(0, &pack));
		}
		else if(pack.Rec.ScaleTypeID == PPSCLT_CAS) {
			THROW_MEM(p_scale = new CommLP15(0, &pack));
		}
		else if(pack.Rec.ScaleTypeID == PPSCLT_MASSAK) { // @vmiller {
			if(pack.Rec.ProtocolVer == 20) {
				THROW_MEM(p_scale = new COMMassaKVer1(0, &pack));
			}
		} // }
		if(p_scale) {
			int    gds_no = 0, r;
			double weight = 0.0;
			THROW(r = p_scale->GetData(&gds_no, &weight));
			if(r > 0) {
				ObjAssocTbl::Rec assoc_rec;
				MEMSZERO(assoc_rec);
				if(gds_no) {
					THROW(r = PPRef->Assc.SearchNum(PPASS_ALTGOODSGRP, pack.Rec.AltGoodsGrp, gds_no, &assoc_rec));
					tidi.GoodsGrpID = pack.Rec.AltGoodsGrp;
					tidi.GoodsID    = (r > 0) ? assoc_rec.ScndObjID : 0;
				}
				tidi.Quantity = weight;
				ok = 1;
			}
		}
		else
			ok = -2; // ����� ������ � ����� ������ ���� �� ������������
	}
	CATCHZOK
	ASSIGN_PTR(pData, tidi);
	delete p_scale;
	return ok;
}

int SLAPI GetDefScaleData(TIDlgInitData * pData)
{
	int    ok = -1;
	PPEquipConfig  eq_cfg;
	THROW(ReadEquipConfig(&eq_cfg));
	/* @v9.0.4
	if(eq_cfg.ScaleID && eq_cfg.Flags & PPEquipConfig::fCheckScaleInput) {
		THROW(ok = GetScaleData(eq_cfg.ScaleID, pData));
	}
	*/
	// @v9.0.4 {
	if(eq_cfg.DefCashNodeID && eq_cfg.Flags & PPEquipConfig::fCheckScaleInput) {
		PPObjCashNode cn_obj;
		PPSyncCashNode sync_cn_rec;
		if(cn_obj.GetSync(eq_cfg.DefCashNodeID, &sync_cn_rec) > 0 && sync_cn_rec.ScaleID) {
			THROW(ok = GetScaleData(sync_cn_rec.ScaleID, pData));
		}
	}
	// } @v9.0.4
	CATCHZOKPPERR
	return ok;
}
//
//
//
class ScaleCache : public ObjCache {
public:
	SLAPI  ScaleCache() : ObjCache(PPOBJ_SCALE, sizeof(ScaleData)) {}
private:
	virtual int  SLAPI FetchEntry(PPID, ObjCacheEntry * pEntry, long);
	virtual void SLAPI EntryToData(const ObjCacheEntry * pEntry, void * pDataRec) const;
public:
	struct ScaleData : public ObjCacheEntry {
		uint16 Get_NumTries;   //
		uint16 Get_Delay;      //
		uint16 Put_NumTries;   //
		uint16 Put_Delay;      //
		PPID   QuotKindID;     //
		PPID   ScaleTypeID;    //
		long   ProtocolVer;    //
		long   LogNum;         //
		long   Flags;          //
		long   Location;       //
		long   AltGoodsGrp;    //
		int16  BcPrefix;       //
		uint16 Reserve;        // @alignment
		//char   Port[8];        //
		PPID   ParentID;       //
	};
};

int SLAPI ScaleCache::FetchEntry(PPID id, ObjCacheEntry * pEntry, long)
{
	int    ok = 1;
	ScaleData * p_cache_rec = static_cast<ScaleData *>(pEntry);
	PPObjScale sc_obj;
	PPScalePacket pack;
	SString temp_buf;
	if(sc_obj.GetPacket(id, &pack) > 0) {
#define CPY_FLD(Fld) p_cache_rec->Fld=pack.Rec.Fld
		CPY_FLD(Get_NumTries);
		CPY_FLD(Get_Delay);
		CPY_FLD(Put_NumTries);
		CPY_FLD(Put_Delay);
		CPY_FLD(QuotKindID);
		CPY_FLD(ScaleTypeID);
		CPY_FLD(ProtocolVer);
		CPY_FLD(LogNum);
		CPY_FLD(Flags);
		CPY_FLD(Location);
		CPY_FLD(AltGoodsGrp);
		CPY_FLD(BcPrefix);
		CPY_FLD(ParentID);
#undef CPY_FLD
		pack.GetExtStrData(pack.extssPort, temp_buf);
		PPStringSetSCD ss;
		ss.add(pack.Rec.Name);
		ss.add(temp_buf);
		PutName(ss.getBuf(), p_cache_rec);
		//memcpy(p_cache_rec->Port, rec.Port, sizeof(rec.Port));
		//ok = PutName(rec.Name, p_cache_rec);
	}
	else
		ok = -1;
	return ok;
}

void SLAPI ScaleCache::EntryToData(const ObjCacheEntry * pEntry, void * pDataRec) const
{
	PPScalePacket * p_data_pack = static_cast<PPScalePacket *>(pDataRec);
	const ScaleData * p_cache_rec = static_cast<const ScaleData *>(pEntry);
	//memzero(p_data_rec, sizeof(*p_data_rec));
#define CPY_FLD(Fld) p_data_pack->Rec.Fld=p_cache_rec->Fld
	p_data_pack->Rec.Tag = PPOBJ_SCALE;
	CPY_FLD(ID);
	CPY_FLD(Get_NumTries);
	CPY_FLD(Get_Delay);
	CPY_FLD(Put_NumTries);
	CPY_FLD(Put_Delay);
	CPY_FLD(QuotKindID);
	CPY_FLD(ScaleTypeID);
	CPY_FLD(ProtocolVer);
	CPY_FLD(LogNum);
	CPY_FLD(Flags);
	CPY_FLD(Location);
	CPY_FLD(AltGoodsGrp);
	CPY_FLD(BcPrefix);
	CPY_FLD(ParentID);
#undef CPY_FLD
	//memcpy(p_data_rec->Port, p_cache_rec->Port, sizeof(p_data_rec->Port));
	//GetName(pEntry, p_data_rec->Name, sizeof(p_data_rec->Name));
	char   temp_zstr[2048];
	SString temp_buf;
	GetName(pEntry, temp_zstr, sizeof(temp_zstr));
	PPStringSetSCD ss;
	ss.setBuf(temp_zstr, sstrlen(temp_zstr)+1);
	uint   p = 0;
	ss.get(&p, p_data_pack->Rec.Name, sizeof(p_data_pack->Rec.Name));
	ss.get(&p, temp_buf);
	p_data_pack->PutExtStrData(PPScalePacket::extssPort, temp_buf);
}

//IMPL_OBJ_FETCH(PPObjScale, PPScalePacket, ScaleCache);
int FASTCALL PPObjScale::Fetch(PPID id, PPScalePacket * pRec)
{
	ScaleCache * p_cache = GetDbLocalCachePtr <ScaleCache> (Obj);
	return p_cache ? p_cache->Get(id, pRec) : GetPacket(id, pRec);
}

