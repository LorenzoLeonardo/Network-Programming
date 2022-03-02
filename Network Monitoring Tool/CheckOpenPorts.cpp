
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

BOOL IsRunAsAdministrator()
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

	// Determine whether the SID of administrators group is enabled in 
	// the primary access token of the process.
	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		DEBUG_LOG(_T("Network Monitoring Tool : CheckTokenMembership() Error: ")+ to_wstring(dwError));
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsRunAsAdmin;
}

void ElevateNow()
{
	BOOL bAlreadyRunningAsAdministrator = FALSE;
	try
	{
		bAlreadyRunningAsAdministrator = IsRunAsAdministrator();
	}
	catch (...)
	{
		//std::cout << "Failed to determine if application was running with admin rights" << std::endl;
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
					// The user refused to allow privileges elevation.
					//std::cout << "End user did not allow elevation" << std::endl;
					DEBUG_LOG(_T("Network Monitoring Tool : End user did not allow elevation"));
				}
			}
			else
			{
				_exit(1);  // Quit itself
			}
		}
	}
}

// CCheckOpenPortsApp initialization
/*BOOL EnableWindowsPrivilege(WCHAR* Privilege)
{
	LUID luid = {};
	TOKEN_PRIVILEGES tp;
	HANDLE currentProcess = GetCurrentProcess();
	HANDLE currentToken = {};
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!LookupPrivilegeValue(NULL, Privilege, &luid)) 
		return FALSE;
	if (!OpenProcessToken(currentProcess, TOKEN_ALL_ACCESS, &currentToken)) 
		return FALSE;
	if (!AdjustTokenPrivileges(currentToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) 
		return FALSE;
	return TRUE;
}
HANDLE GetAccessToken(DWORD pid)
{

	
	HANDLE currentProcess = {};
	HANDLE AccessToken = {};
	DWORD LastError;

	if (pid == 0)
	{
		currentProcess = GetCurrentProcess();
	}
	else
	{
		currentProcess = OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, pid);
		if (!currentProcess)
		{
			LastError = GetLastError();
			wprintf(L"ERROR: OpenProcess(): %d\n", LastError);
			return (HANDLE)NULL;
		}
	}
	if (!OpenProcessToken(currentProcess, TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE | TOKEN_QUERY, &AccessToken))
	{
		LastError = GetLastError();
		wprintf(L"ERROR: OpenProcessToken(): %d\n", LastError);
		return (HANDLE)NULL;
	}
	return AccessToken;
}*/
BOOL CCheckOpenPortsApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
//	INITCOMMONCONTROLSEX InitCtrls;
//	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
//	InitCtrls.dwICC = ICC_WIN95_CLASSES;
//	InitCommonControlsEx(&InitCtrls);
	CFirewall firewall;
	TCHAR szpath[MAX_PATH];
	GetModuleFileName(NULL, szpath, sizeof(szpath));

	firewall.ImplementFirewall(szpath,_T("Enzo Tech Network Monitoring Tool"));
	CWinApp::InitInstance();

	if (!IsRunAsAdministrator())
	{
		if (AfxMessageBox(_T("This tool cannot continue if you don't run as administrator. Are you willing to elevate the process?"), MB_YESNO) == IDNO)
		{
			return FALSE;
		}
		else
		{
			ElevateNow();
		}
	}

	/*if (!EnableWindowsPrivilege(SE_ASSIGNPRIMARYTOKEN_NAME))
	{
		return FALSE;
	}
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	DWORD dwID = 0;
	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (_wcsicmp(entry.szExeFile, L"explorer.exe") == 0)
			{
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
				dwID = entry.th32ProcessID;

				CloseHandle(hProcess);
			}
		}
	}

	CloseHandle(snapshot);
	
	HANDLE pToken = GetAccessToken(dwID);

	//These are required to call DuplicateTokenEx.
	SECURITY_IMPERSONATION_LEVEL seImpersonateLevel = SecurityImpersonation;
	TOKEN_TYPE tokenType = TokenPrimary;
	HANDLE pNewToken = new HANDLE;
	if (!DuplicateTokenEx(pToken, MAXIMUM_ALLOWED, NULL, seImpersonateLevel, tokenType, &pNewToken))
	{
		DWORD LastError = GetLastError();
		wprintf(L"ERROR: Could not duplicate process token [%d]\n", LastError);
		return 1;
	}
	DWORD dwSessionId = GetCurrentProcessId();
	
	if (!SetTokenInformation(pNewToken, TokenSessionId, &dwSessionId, sizeof(dwSessionId)))
	{
		DWORD LastError = GetLastError();
		wprintf(L"ERROR: Could not set token [%d]\n", LastError);
		return 1;
	}
	delete pNewToken;*/
	//AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	//CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
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

