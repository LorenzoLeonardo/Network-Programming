#pragma once
#include <crtdbg.h>
#include <netfw.h>
#include <objbase.h>
#include <oleauto.h>
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

class CFirewall {
protected:
	HRESULT m_comInit;

public:
	CFirewall();
	~CFirewall();
	bool InitializeCOM();
	void UninitializeCOM();
	int ProcessAppToFirewall(LPCTSTR szProgramName);
	HRESULT WindowsFirewallPortAdd(IN INetFwProfile* fwProfile,
		IN LONG portNumber,
		IN NET_FW_IP_PROTOCOL ipProtocol,
		IN const wchar_t* name);
	HRESULT WindowsFirewallPortIsEnabled(IN INetFwProfile* fwProfile,
		IN LONG portNumber,
		IN NET_FW_IP_PROTOCOL ipProtocol,
		OUT BOOL* fwPortEnabled);
	HRESULT WindowsFirewallAddApp(IN INetFwProfile* fwProfile,
		IN const wchar_t* fwProcessImageFileName,
		IN const wchar_t* fwName);
	HRESULT WindowsFirewallAppIsEnabled(IN INetFwProfile* fwProfile,
		IN const wchar_t* fwProcessImageFileName,
		OUT BOOL* fwAppEnabled);
	void WindowsFirewallCleanup(IN INetFwProfile*& fwProfile);
	HRESULT WindowsFirewallInitialize(OUT INetFwProfile** fwProfile);
};
