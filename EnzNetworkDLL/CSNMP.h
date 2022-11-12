#pragma once
#include "EnzNetworkDLL.h"
#include <vector>
#include <string>
#include <stdio.h>


using namespace std;

typedef struct _SNMP_SESSION {
	HSNMP_SESSION hSnmpSession;	 // handle to winsnmp session
	HSNMP_ENTITY hAgentEntity;	 // handle to agent entity
	HSNMP_ENTITY hManagerEntity; // handle to manager entity
	HSNMP_CONTEXT hViewContext;	 // handle to view context
	HSNMP_PDU hPdu;				 // handle to snmp pdu
	HSNMP_VBL hVbl;				 // handle to var bind list
	HWND hWnd;					 // handle to window

	smiINT32 nPduType;	   // current pdu type
	smiINT32 nRequestId;   // current request id
	smiINT32 nError;	   // last system error
	smiINT32 nErrorStatus; // error status
	smiINT32 nErrorIndex;  // error index

} SNMP_SESSION, *PSNMP_SESSION;

#define WM_SNMP_INCOMING WM_USER + 1
#define WM_SNMP_DONE WM_USER + 2

class CSNMP {
private:
	PSNMP_SESSION m_pSession;
	WSAData m_wsaData;
	SNMPAPI_STATUS m_status;
	ULONG m_ulMajorVersion, m_ulMinorVersion, m_ulLevel, m_ulTranslateMode, m_ulRetransmitMode;
	smiOCTETS m_smiCommunity;
	static bool m_fDone;

public:
	CSNMP();
	~CSNMP();

	bool InitSNMP(const char* szAgentIPAddress, const char* szCommunity, int nVersion, DWORD& dwLastError);
	void EndSNMP();
	smiVALUE Get(const char* szOID, DWORD& dwLastError);
	bool CreateNotificationWindow(PSNMP_SESSION pSession);

	static smiOID m_psmilOID;
	static smiVALUE m_nvalue;
	static bool ProcessNotification(PSNMP_SESSION pSession);
	static LRESULT CALLBACK NotificationWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static bool m_bSnmpSuccess;

private:
	inline void GetSessionMessage();
	static bool ProcessVarBind(PSNMP_SESSION pSession);
};
