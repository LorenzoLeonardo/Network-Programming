
// CheckOpenPorst.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "CheckOpenPorts.h"
#include "CheckOpenPortsDlg.h"
#include <tlhelp32.h>
#include "..\EnzTCP\DebugLog.h"
#include "CFirewall.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCheckOpenPortsApp

BEGIN_MESSAGE_MAP(CCheckOpenPortsApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCheckOpenPortsApp construction

CCheckOpenPortsApp::CCheckOpenPortsApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CCheckOpenPortsApp object

CCheckOpenPortsApp theApp;

BOOL IsAdministrator()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
		DEBUG_LOG(_T("Network Monitoring Tool : CheckTokenMembership() Error: ") + to_wstring(dwError));
		goto Cleanup;
	}

	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		DEBUG_LOG(_T("Network Monitoring Tool : CheckTokenMembership() Error: ")+ to_wstring(dwError));
		goto Cleanup;
	}

Cleanup:
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsRunAsAdmin;
}

void ElevateProcess()
{
	BOOL bAlreadyRunningAsAdministrator = FALSE;
	try
	{
		bAlreadyRunningAsAdministrator = IsAdministrator();
	}
	catch (...)
	{
		DWORD dwErrorCode = GetLastError();
		TCHAR szMessage[256];
		_stprintf_s(szMessage, ARRAYSIZE(szMessage), L"Error code returned was 0x%08lx", dwErrorCode);
		wstring s(szMessage);
		DEBUG_LOG(_T("Network Monitoring Tool : ") + s);
	}
	if (!bAlreadyRunningAsAdministrator)
	{
		wchar_t szPath[MAX_PATH];
		if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
		{
			// Launch itself as admin
			SHELLEXECUTEINFO sei = { sizeof(sei) };
			sei.lpVerb = L"runas";
			sei.lpFile = szPath;
			sei.hwnd = NULL;
			sei.nShow = SW_NORMAL;

			if (!ShellExecuteEx(&sei))
			{
				DWORD dwError = GetLastError();
				if (dwError == ERROR_CANCELLED)
				{
					DEBUG_LOG(_T("Network Monitoring Tool : Elevation was not allowed."));
				}
			}
			else
			{
				_exit(1);  // Quit itself
			}
		}
	}
}
int ProcessAppToFirewall(LPCTSTR szAppName)
{
	CFirewall firewall;

	HRESULT hr = S_OK;
	INetFwProfile* fwProfile = NULL;
	TCHAR szFileNamePath[MAX_PATH];
	BOOL bIsAppEnable = false;
	GetModuleFileName(NULL, szFileNamePath, sizeof(szFileNamePath));

	// Initialize COM.
	if (firewall.InitializeCOM())
	{
		hr = firewall.WindowsFirewallInitialize(&fwProfile);
		if (FAILED(hr))
		{
			DEBUG_LOG(_T("WindowsFirewallInitialize failed: 0x%08lx\n"), hr);
			firewall.UninitializeCOM();
			return false;
		}
		hr = firewall.WindowsFirewallAppIsEnabled(fwProfile, szFileNamePath, &bIsAppEnable);
		if (FAILED(hr))
		{
			DEBUG_LOG(_T("WindowsFirewallAddApp failed: 0x%08lx\n"), hr);
			firewall.WindowsFirewallCleanup(fwProfile);
			firewall.UninitializeCOM();
			return false;
		}
		if (!bIsAppEnable)
		{
			int bRet = AfxMessageBox(_T("The Network Monitoring Tool is not yet allowed by the Windows firewall. You cannot see all the packets if you will not add it to the Windows firewall. Do you want to add it?"), MB_YESNO);
			
			if (IDYES == bRet)
			{
				hr = firewall.WindowsFirewallAddApp(fwProfile, szFileNamePath, szAppName);
				if (FAILED(hr))
				{
					DEBUG_LOG(_T("WindowsFirewallAddApp failed: 0x%08lx\n"), hr);
					firewall.WindowsFirewallCleanup(fwProfile);
					firewall.UninitializeCOM();
					return false;
				}
			}
			else
			{
				firewall.WindowsFirewallCleanup(fwProfile);
				firewall.UninitializeCOM();
			}
		}
	}
	return true;
}
BOOL CCheckOpenPortsApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!IsAdministrator())
	{
		if (AfxMessageBox(_T("This tool cannot continue if you don't run as administrator. Are you willing to elevate the process?"), MB_YESNO) == IDNO)
			return FALSE;
		else
			ElevateProcess();
	}
	ProcessAppToFirewall(_T("Enzo Tech Network Monitoring Tool"));

	CShellManager *pShellManager = new CShellManager;

	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CCheckOpenPortsDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
//	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

