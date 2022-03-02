#pragma once
#include <windows.h>
#include <crtdbg.h>
#include <netfw.h>
#include <objbase.h>
#include <oleauto.h>
#include <stdio.h>

#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )

class CFirewall
{
public:
    CFirewall();
    ~CFirewall();
    int ImplementFirewall();
        HRESULT WindowsFirewallPortAdd(
            IN INetFwProfile* fwProfile,
            IN LONG portNumber,
            IN NET_FW_IP_PROTOCOL ipProtocol,
            IN const wchar_t* name
        );
        HRESULT WindowsFirewallPortIsEnabled(
            IN INetFwProfile* fwProfile,
            IN LONG portNumber,
            IN NET_FW_IP_PROTOCOL ipProtocol,
            OUT BOOL* fwPortEnabled
        );
        HRESULT WindowsFirewallAddApp(
            IN INetFwProfile* fwProfile,
            IN const wchar_t* fwProcessImageFileName,
            IN const wchar_t* fwName
        );
        HRESULT WindowsFirewallAppIsEnabled(
            IN INetFwProfile* fwProfile,
            IN const wchar_t* fwProcessImageFileName,
            OUT BOOL* fwAppEnabled
        );
        HRESULT WindowsFirewallTurnOff(IN INetFwProfile* fwProfile);
        HRESULT WindowsFirewallTurnOn(IN INetFwProfile* fwProfile);
        HRESULT WindowsFirewallIsOn(IN INetFwProfile* fwProfile, OUT BOOL* fwOn);
        void WindowsFirewallCleanup(IN INetFwProfile* fwProfile);
        HRESULT WindowsFirewallInitialize(OUT INetFwProfile** fwProfile);
};

