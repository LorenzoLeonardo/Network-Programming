
// CheckOpenPorst.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "CheckOpenPorts.h"
#include "CheckOpenPortsDlg.h"
#include <tlhelp32.h>
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

	CWinApp::InitInstance();

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

