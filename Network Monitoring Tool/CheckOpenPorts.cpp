
// CheckOpenPorst.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "CheckOpenPorts.h"
#include "CheckOpenPortsDlg.h"
#include <tlhelp32.h>
#include "..\EnzNetworkDLL\DebugLog.h"

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

BOOL  CCheckOpenPortsApp::IsAdministrator()
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

void  CCheckOpenPortsApp::ElevateProcess()
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

int CCheckOpenPortsApp::ProcessAppToFirewall(LPCTSTR szAppName)
{
	CFirewall firewall;
	HRESULT hr = S_OK;
	INetFwProfile* fwProfile = NULL;
	TCHAR szFileNamePath[MAX_PATH];
	BOOL bIsAppEnable = false;

	memset(szFileNamePath, 0, sizeof(szFileNamePath));

	GetModuleFileName(NULL, szFileNamePath, sizeof(szFileNamePath));

	// Initialize COM.
	if (firewall.InitializeCOM())
	{
		hr = firewall.WindowsFirewallInitialize(&fwProfile);
		if (FAILED(hr))
		{
		//	DEBUG_LOG(_T("WindowsFirewallInitialize failed: 0x%08lx\n"), hr);
			firewall.UninitializeCOM();
			return false;
		}
		hr = firewall.WindowsFirewallAppIsEnabled(fwProfile, szFileNamePath, &bIsAppEnable);
		if (FAILED(hr))
		{
		//	DEBUG_LOG(_T("WindowsFirewallAddApp failed: 0x%08lx\n"), hr);
			firewall.WindowsFirewallCleanup(fwProfile);
			firewall.UninitializeCOM();
			return false;
		}
		if (!bIsAppEnable)
		{
			CString csMsg;
			csMsg.LoadString(IDS_FIREWALL);
			int bRet = ::MessageBox(GetMainWnd()->GetSafeHwnd(), csMsg, _T("Enzo Tech Network Monitoring Tool"), MB_YESNO | MB_ICONQUESTION);
			if (IDYES == bRet)
			{
				hr = firewall.WindowsFirewallAddApp(fwProfile, szFileNamePath, szAppName);
				if (FAILED(hr))
				{
			//		DEBUG_LOG(_T("WindowsFirewallAddApp failed: 0x%08lx\n"), hr);
					firewall.WindowsFirewallCleanup(fwProfile);
					firewall.UninitializeCOM();
					return false;
				}
			}

		}
	}
	firewall.WindowsFirewallCleanup(fwProfile);
	firewall.UninitializeCOM();

	return true;
}

BOOL CCheckOpenPortsApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
// manifest specifies use of ComCtl32.dll version 6 or later to enable
// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();
	
	CShellManager* pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	if (!IsAdministrator())
	{
		CString csMsg;

		csMsg.Format(IDS_ELEVATE);
		int bRet = ::MessageBox(GetMainWnd()->GetSafeHwnd(), csMsg, _T("Enzo Tech Network Monitoring Tool"), MB_YESNO| MB_ICONQUESTION);

		if (bRet == IDNO)
			return FALSE;
		else
			ElevateProcess();
	}
	ProcessAppToFirewall(_T("Enzo Tech Network Monitoring Tool"));

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
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

