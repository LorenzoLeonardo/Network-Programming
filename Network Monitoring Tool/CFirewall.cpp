#include "pch.h"
#include "CFirewall.h"
#include "../EnzTCP/DebugLog.h"

CFirewall::CFirewall()
{
    m_comInit = E_FAIL;
}
CFirewall::~CFirewall()
{

}

bool CFirewall::InitializeCOM()
{
    HRESULT hr = S_OK;

    m_comInit = CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (m_comInit != RPC_E_CHANGED_MODE)
    {
        hr = m_comInit;
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("CoInitializeEx failed: 0x%08lx\n"), hr);
            return false;
        }
    }
    return true;
}

void CFirewall::UninitializeCOM()
{
    if (SUCCEEDED(m_comInit))
        CoUninitialize();

}
HRESULT CFirewall::WindowsFirewallInitialize(OUT INetFwProfile** fwProfile)
{
    HRESULT hr = S_OK;
    INetFwMgr* fwMgr = NULL;
    INetFwPolicy* fwPolicy = NULL;

    _ASSERT(fwProfile != NULL);

    *fwProfile = NULL;

    // Create an instance of the firewall settings manager.
    hr = CoCreateInstance(
        __uuidof(NetFwMgr),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwMgr),
        (void**)&fwMgr
    );
    if (FAILED(hr))
    {
        DEBUG_LOG(_T("CoCreateInstance failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Retrieve the local firewall policy.
    hr = fwMgr->get_LocalPolicy(&fwPolicy);
    if (FAILED(hr))
    {
        DEBUG_LOG(_T("get_LocalPolicy failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Retrieve the firewall profile currently in effect.
    hr = fwPolicy->get_CurrentProfile(fwProfile);
    if (FAILED(hr))
    {
        DEBUG_LOG(_T("get_CurrentProfile failed: 0x%08lx\n"), hr);
        goto error;
    }

error:

    // Release the local firewall policy.
    if (fwPolicy != NULL)
    {
        fwPolicy->Release();
    }

    // Release the firewall settings manager.
    if (fwMgr != NULL)
    {
        fwMgr->Release();
    }

    return hr;
}


void CFirewall::WindowsFirewallCleanup(IN INetFwProfile* &fwProfile)
{
    // Release the firewall profile.
    if (fwProfile != NULL)
    {
        fwProfile->Release();
    }
}



HRESULT CFirewall::WindowsFirewallAppIsEnabled(
    IN INetFwProfile* fwProfile,
    IN const wchar_t* fwProcessImageFileName,
    OUT BOOL* fwAppEnabled
)
{
    HRESULT hr = S_OK;
    BSTR fwBstrProcessImageFileName = NULL;
    VARIANT_BOOL fwEnabled;
    INetFwAuthorizedApplication* fwApp = NULL;
    INetFwAuthorizedApplications* fwApps = NULL;

    _ASSERT(fwProfile != NULL);
    _ASSERT(fwProcessImageFileName != NULL);
    _ASSERT(fwAppEnabled != NULL);

    *fwAppEnabled = FALSE;

    // Retrieve the authorized application collection.
    hr = fwProfile->get_AuthorizedApplications(&fwApps);
    if (FAILED(hr))
    {
        DEBUG_LOG(_T("get_AuthorizedApplications failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Allocate a BSTR for the process image file name.
    fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
    if (fwBstrProcessImageFileName == NULL)
    {
        hr = E_OUTOFMEMORY;
        DEBUG_LOG(_T("SysAllocString failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Attempt to retrieve the authorized application.
    hr = fwApps->Item(fwBstrProcessImageFileName, &fwApp);
    if (SUCCEEDED(hr))
    {
        // Find out if the authorized application is enabled.
        hr = fwApp->get_Enabled(&fwEnabled);
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("get_Enabled failed: 0x%08lx\n"), hr);
            goto error;
        }

        if (fwEnabled != VARIANT_FALSE)
        {
            // The authorized application is enabled.
            *fwAppEnabled = TRUE;

            DEBUG_LOG(
                _T("Authorized application %lS is enabled in the firewall.\n"),
                fwProcessImageFileName
            );
        }
        else
        {
            DEBUG_LOG(
                _T("Authorized application %lS is disabled in the firewall.\n"),
                fwProcessImageFileName
            );
        }
    }
    else
    {
        // The authorized application was not in the collection.
        hr = S_OK;

        DEBUG_LOG(
            _T("Authorized application %lS is disabled in the firewall.\n"),
            fwProcessImageFileName
        );
    }

error:

    // Free the BSTR.
    SysFreeString(fwBstrProcessImageFileName);

    // Release the authorized application instance.
    if (fwApp != NULL)
    {
        fwApp->Release();
    }

    // Release the authorized application collection.
    if (fwApps != NULL)
    {
        fwApps->Release();
    }

    return hr;
}

HRESULT CFirewall::WindowsFirewallAddApp(
    IN INetFwProfile* fwProfile,
    IN const wchar_t* fwProcessImageFileName,
    IN const wchar_t* fwName
)
{
    HRESULT hr = S_OK;
    BOOL fwAppEnabled;
    BSTR fwBstrName = NULL;
    BSTR fwBstrProcessImageFileName = NULL;
    INetFwAuthorizedApplication* fwApp = NULL;
    INetFwAuthorizedApplications* fwApps = NULL;

    _ASSERT(fwProfile != NULL);
    _ASSERT(fwProcessImageFileName != NULL);
    _ASSERT(fwName != NULL);

    // First check to see if the application is already authorized.
    hr = WindowsFirewallAppIsEnabled(
        fwProfile,
        fwProcessImageFileName,
        &fwAppEnabled
    );
    if (FAILED(hr))
    {
        DEBUG_LOG(_T("WindowsFirewallAppIsEnabled failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Only add the application if it isn't already authorized.
    if (!fwAppEnabled)
    {
        // Retrieve the authorized application collection.
        hr = fwProfile->get_AuthorizedApplications(&fwApps);
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("get_AuthorizedApplications failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Create an instance of an authorized application.
        hr = CoCreateInstance(
            __uuidof(NetFwAuthorizedApplication),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(INetFwAuthorizedApplication),
            (void**)&fwApp
        );
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("CoCreateInstance failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Allocate a BSTR for the process image file name.
        fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
        if (fwBstrProcessImageFileName == NULL)
        {
            hr = E_OUTOFMEMORY;
            DEBUG_LOG(_T("SysAllocString failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Set the process image file name.
        hr = fwApp->put_ProcessImageFileName(fwBstrProcessImageFileName);
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("put_ProcessImageFileName failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Allocate a BSTR for the application friendly name.
        fwBstrName = SysAllocString(fwName);
        if (SysStringLen(fwBstrName) == 0)
        {
            hr = E_OUTOFMEMORY;
            DEBUG_LOG(_T("SysAllocString failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Set the application friendly name.
        hr = fwApp->put_Name(fwBstrName);
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("put_Name failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Add the application to the collection.
        hr = fwApps->Add(fwApp);
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("Add failed: 0x%08lx\n"), hr);
            goto error;
        }

        DEBUG_LOG(
            _T("Authorized application %lS is now enabled in the firewall.\n"),
            fwProcessImageFileName
        );
    }

error:

    // Free the BSTRs.
    SysFreeString(fwBstrName);
    SysFreeString(fwBstrProcessImageFileName);

    // Release the authorized application instance.
    if (fwApp != NULL)
    {
        fwApp->Release();
    }

    // Release the authorized application collection.
    if (fwApps != NULL)
    {
        fwApps->Release();
    }

    return hr;
}

HRESULT CFirewall::WindowsFirewallPortIsEnabled(
    IN INetFwProfile* fwProfile,
    IN LONG portNumber,
    IN NET_FW_IP_PROTOCOL ipProtocol,
    OUT BOOL* fwPortEnabled
)
{
    HRESULT hr = S_OK;
    VARIANT_BOOL fwEnabled;
    INetFwOpenPort* fwOpenPort = NULL;
    INetFwOpenPorts* fwOpenPorts = NULL;

    _ASSERT(fwProfile != NULL);
    _ASSERT(fwPortEnabled != NULL);

    *fwPortEnabled = FALSE;

    // Retrieve the globally open ports collection.
    hr = fwProfile->get_GloballyOpenPorts(&fwOpenPorts);
    if (FAILED(hr))
    {
        DEBUG_LOG(_T("get_GloballyOpenPorts failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Attempt to retrieve the globally open port.
    hr = fwOpenPorts->Item(portNumber, ipProtocol, &fwOpenPort);
    if (SUCCEEDED(hr))
    {
        // Find out if the globally open port is enabled.
        hr = fwOpenPort->get_Enabled(&fwEnabled);
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("get_Enabled failed: 0x%08lx\n"), hr);
            goto error;
        }

        if (fwEnabled != VARIANT_FALSE)
        {
            // The globally open port is enabled.
            *fwPortEnabled = TRUE;

            DEBUG_LOG(_T("Port %ld is open in the firewall.\n"), portNumber);
        }
        else
        {
            DEBUG_LOG(_T("Port %ld is not open in the firewall.\n"), portNumber);
        }
    }
    else
    {
        // The globally open port was not in the collection.
        hr = S_OK;

        DEBUG_LOG(_T("Port %ld is not open in the firewall.\n"), portNumber);
    }

error:

    // Release the globally open port.
    if (fwOpenPort != NULL)
    {
        fwOpenPort->Release();
    }

    // Release the globally open ports collection.
    if (fwOpenPorts != NULL)
    {
        fwOpenPorts->Release();
    }

    return hr;
}

HRESULT CFirewall::WindowsFirewallPortAdd(
    IN INetFwProfile* fwProfile,
    IN LONG portNumber,
    IN NET_FW_IP_PROTOCOL ipProtocol,
    IN const wchar_t* name
)
{
    HRESULT hr = S_OK;
    BOOL fwPortEnabled;
    BSTR fwBstrName = NULL;
    INetFwOpenPort* fwOpenPort = NULL;
    INetFwOpenPorts* fwOpenPorts = NULL;

    _ASSERT(fwProfile != NULL);
    _ASSERT(name != NULL);

    // First check to see if the port is already added.
    hr = WindowsFirewallPortIsEnabled(
        fwProfile,
        portNumber,
        ipProtocol,
        &fwPortEnabled
    );
    if (FAILED(hr))
    {
        DEBUG_LOG(_T("WindowsFirewallPortIsEnabled failed: 0x%08lx\n"), hr);
        goto error;
    }

    // Only add the port if it isn't already added.
    if (!fwPortEnabled)
    {
        // Retrieve the collection of globally open ports.
        hr = fwProfile->get_GloballyOpenPorts(&fwOpenPorts);
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("get_GloballyOpenPorts failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Create an instance of an open port.
        hr = CoCreateInstance(
            __uuidof(NetFwOpenPort),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(INetFwOpenPort),
            (void**)&fwOpenPort
        );
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("CoCreateInstance failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Set the port number.
        hr = fwOpenPort->put_Port(portNumber);
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("put_Port failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Set the IP protocol.
        hr = fwOpenPort->put_Protocol(ipProtocol);
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("put_Protocol failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Allocate a BSTR for the friendly name of the port.
        fwBstrName = SysAllocString(name);
        if (SysStringLen(fwBstrName) == 0)
        {
            hr = E_OUTOFMEMORY;
            DEBUG_LOG(_T("SysAllocString failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Set the friendly name of the port.
        hr = fwOpenPort->put_Name(fwBstrName);
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("put_Name failed: 0x%08lx\n"), hr);
            goto error;
        }

        // Opens the port and adds it to the collection.
        hr = fwOpenPorts->Add(fwOpenPort);
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("Add failed: 0x%08lx\n"), hr);
            goto error;
        }

        DEBUG_LOG(_T("Port %ld is now open in the firewall.\n"), portNumber);
    }

error:

    // Free the BSTR.
    SysFreeString(fwBstrName);

    // Release the open port instance.
    if (fwOpenPort != NULL)
    {
        fwOpenPort->Release();
    }

    // Release the globally open ports collection.
    if (fwOpenPorts != NULL)
    {
        fwOpenPorts->Release();
    }

    return hr;
}


int CFirewall::ProcessAppToFirewall(LPCTSTR szProgramName)
{
    HRESULT hr = S_OK;
    HRESULT comInit = E_FAIL;
    INetFwProfile* fwProfile = NULL;
    TCHAR szFileNamePath[MAX_PATH];

    GetModuleFileName(NULL, szFileNamePath, sizeof(szFileNamePath));

    // Initialize COM.
    comInit = CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );

    if (comInit != RPC_E_CHANGED_MODE)
    {
        hr = comInit;
        if (FAILED(hr))
        {
            DEBUG_LOG(_T("CoInitializeEx failed: 0x%08lx\n"), hr);
            goto error;
        }
    }

    hr = WindowsFirewallInitialize(&fwProfile);
    if (FAILED(hr))
    {
        DEBUG_LOG(_T("WindowsFirewallInitialize failed: 0x%08lx\n"), hr);
        goto error;
    }

    hr = WindowsFirewallAddApp(fwProfile, szFileNamePath, szProgramName);
    if (FAILED(hr))
    {
        DEBUG_LOG(_T("WindowsFirewallAddApp failed: 0x%08lx\n"), hr);
        goto error;
    }

error:
    WindowsFirewallCleanup(fwProfile);

    if (SUCCEEDED(comInit))
        CoUninitialize();

    return 0;
}
