
// CheckOpenPorst.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "CheckOpenPorts.h"
#include "CheckOpenPortsDlg.h"
#include <tlhelp32.h>
#include "..\EnzTCP\DebugLog.h"

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
	CFirewall* firewall = new CFirewall();
	HRESULT hr = S_OK;
	INetFwProfile* fwProfile = NULL;
	TCHAR* szFileNamePath = NULL;
	BOOL bIsAppEnable = false;
	DWORD dwSize = sizeof(TCHAR) * (MAX_PATH + 1);

	szFileNamePath = (TCHAR*)malloc(dwSize);
	if (!szFileNamePath)
		return 0;
	memset(szFileNamePath, 0, dwSize);
	GetModuleFileName(NULL, szFileNamePath, dwSize);

	// Initialize COM.
	if (firewall->InitializeCOM())
	{
		hr = firewall->WindowsFirewallInitialize(&fwProfile);
		if (FAILED(hr))
		{
			DEBUG_LOG(_T("WindowsFirewallInitialize failed: 0x%08lx\n"), hr);
			firewall->UninitializeCOM();
			free(szFileNamePath);
			delete firewall;
			return false;
		}
		hr = firewall->WindowsFirewallAppIsEnabled(fwProfile, szFileNamePath, &bIsAppEnable);
		if (FAILED(hr))
		{
			DEBUG_LOG(_T("WindowsFirewallAddApp failed: 0x%08lx\n"), hr);
			firewall->WindowsFirewallCleanup(fwProfile);
			firewall->UninitializeCOM();
			free(szFileNamePath);
			delete firewall;
			return false;
		}
		if (!bIsAppEnable)
		{
			CString csMsg;
			csMsg.LoadString(IDS_FIREWALL);
			int bRet = ::MessageBox(GetMainWnd()->GetSafeHwnd(), csMsg, _T("Enzo Tech Network Monitoring Tool"), MB_YESNO | MB_ICONQUESTION);
			if (IDYES == bRet)
			{
				hr = firewall->WindowsFirewallAddApp(fwProfile, szFileNamePath, szAppName);
				if (FAILED(hr))
				{
					DEBUG_LOG(_T("WindowsFirewallAddApp failed: 0x%08lx\n"), hr);
					firewall->WindowsFirewallCleanup(fwProfile);
					firewall->UninitializeCOM();
					free(szFileNamePath);
					delete firewall;
					return false;
				}
			}

		}
	}
	free(szFileNamePath);
	firewall->WindowsFirewallCleanup(fwProfile);
	firewall->UninitializeCOM();
	delete firewall;
	return true;
}

BOOL CCheckOpenPortsApp::InitInstance()
{
	CWinApp::InitInstance();
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
	CShellManager *pShellManager = new CShellManager;

	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	CCheckOpenPortsDlg dlg;
	
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	
	// Delete the shell manager created above.
	if (pShellManager != nullptr)
		delete pShellManager;
	

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

