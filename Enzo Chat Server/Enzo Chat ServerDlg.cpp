
// Enzo Chat ServerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Enzo Chat Server.h"
#include "Enzo Chat ServerDlg.h"
#include "afxdialogex.h"
#include <string>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


typedef void (*NewConnection)(void*);
typedef HANDLE(WINAPI* LPOpenServer)(const char*, NewConnection);
typedef VOID(WINAPI* LPRunServer)(HANDLE);
typedef VOID(WINAPI* LPCloseServer)(HANDLE);
typedef VOID(WINAPI* LPCloseClientConnection)(HANDLE);

LPOpenServer pfnPtrOpenServer;
LPRunServer pfnPtrRunServer;
LPCloseServer pfnPtrCloseServer;
LPCloseClientConnection pfnPtrCloseClientConnection;

CEnzoChatServerDlg* g_dlgPtr = NULL;
// CAboutDlg dialog used for App About



char* convert_from_wstring(const WCHAR* wstr)
{
	int wstr_len = (int)wcslen(wstr);
	int num_chars = WideCharToMultiByte(CP_UTF8, 0, wstr, wstr_len, NULL, 0, NULL, NULL);
	CHAR* strTo = (CHAR*)malloc((num_chars + 1) * sizeof(CHAR));
	if (strTo)
	{
		WideCharToMultiByte(CP_UTF8, 0, wstr, wstr_len, strTo, num_chars, NULL, NULL);
		strTo[num_chars] = '\0';
	}
	return strTo;
}
WCHAR* convert_to_wstring(const char* str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str), NULL, 0);
	WCHAR* wstrTo = (WCHAR*)malloc(size_needed);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)strlen(str), wstrTo, size_needed);
	return wstrTo;
}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CEnzoChatServerDlg dialog



CEnzoChatServerDlg::CEnzoChatServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_ENZO_CHAT_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_ServerHandle = NULL;
	m_vSocket = new vector<ISocket*>;
}

void CEnzoChatServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INPUT_AREA, m_ctrlInputArea);
	DDX_Control(pDX, IDC_EDIT_CHAT_AREA, m_ctrlChatArea);
	DDX_Control(pDX, IDC_LIST_CONNECTED, m_ctrlListConnected);
}

BEGIN_MESSAGE_MAP(CEnzoChatServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CEnzoChatServerDlg::OnBnClickedOk)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CEnzoChatServerDlg message handlers
UINT HandleClientThreadFunc(LPVOID pParam)
{
	ISocket* pSocket = (ISocket*)pParam;
	string sData = "";
	vector<ISocket*> *list = g_dlgPtr->GetVectorList();

	if (pSocket == NULL)
		return 0;
	try
	{
		string text = pSocket->GetIP();
		text += "(" + to_string(pSocket->GetSocket()) + ")";
		text+= " has joined the conversation.\r\n";
		
		CString csText(text.c_str());
		g_dlgPtr->UpdateChatAreaText(csText);

		for (int i = 0; i < list->size(); i++)
		{
			(*list)[i]->Send((char *)text.c_str());
		}
		while (!(sData = pSocket->Receive()).empty())
		{
			string text = pSocket->GetIP();
			text +="(" + to_string(pSocket->GetSocket()) + ")" + " : " + sData + "\r\n";
			
			CString csText(text.c_str());
			g_dlgPtr->UpdateChatAreaText(csText);

			for (int i = 0; i < list->size(); i++)
			{
				if ((*list)[i] != pSocket)
				{
					(*list)[i]->Send((char*)text.c_str());
				}
			}
		}
		for (int i = 0; i < list->size(); i++)
		{
			if ((*list)[i] != pSocket)
			{
				string text = pSocket->GetIP();
				text += "(" + to_string(pSocket->GetSocket()) + ")" + " has left the conversation.\r\n";
				(*list)[i]->Send((char*)text.c_str());
			}
		}
	
	}
	catch (int nError)
	{
		CString csError;
		csError.FormatMessage(_T("%d"), nError);
//		MessageBox(NULL, TEXT("ERROR"), csError, 0);
	}
	
	string text = pSocket->GetIP();
	text +="(" + to_string(pSocket->GetSocket()) + ")" + " has left the conversation.\r\n";
	
	CString csText(text.c_str());

	g_dlgPtr->UpdateChatAreaText(csText);
	list->erase(std::remove(list->begin(), list->end(), pSocket), list->end());
	g_dlgPtr->DisplayConnectedClients();

	pfnPtrCloseClientConnection(pSocket);
	return 0;   // thread completed successfully
}

void NewClientConnection(void* pData)
{
	if (pData != NULL)
	{
		try
		{
			((ISocket*)pData)->Send("Welcome to Lorenzo Leonardo's Awesome Chat\r\n");
			g_dlgPtr->GetVectorList()->push_back((ISocket*)pData);
			g_dlgPtr->DisplayConnectedClients();
			AfxBeginThread(HandleClientThreadFunc, (ISocket*)pData);
		}
		catch (int nError)
		{
			CString csError;
			csError.FormatMessage(_T("%d"), nError);
			MessageBox(NULL, TEXT("ERROR"), csError, 0);

		}
	}
}


UINT ServerThreadFunc(LPVOID pParam)
{
	//m_ServerHandle =
	CEnzoChatServerDlg* dlg = (CEnzoChatServerDlg*)pParam;

	const char* temp = (const char*)convert_from_wstring(_T("0611"));
	HANDLE handle = pfnPtrOpenServer(temp, NewClientConnection);

	g_dlgPtr = dlg;
	dlg->SetConnectionHandle(handle);
	pfnPtrRunServer(handle);
	free((void*)temp);
	return 0;
}
BOOL CEnzoChatServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	// InitCommonControlsEx() is required on Windows XP if an application
// manifest specifies use of ComCtl32.dll version 6 or later to enable
// visual styles.  Otherwise, any window creation will fail.
#ifndef _UNICODE
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
#endif	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	dll_handle = LoadLibrary(_T("EnzTCP.dll"));
	if (dll_handle)
	{

		pfnPtrOpenServer = (LPOpenServer)GetProcAddress(dll_handle, "OpenServer");
		pfnPtrRunServer = (LPRunServer)GetProcAddress(dll_handle, "RunServer");
		pfnPtrCloseServer = (LPCloseServer)GetProcAddress(dll_handle, "CloseServer");
		pfnPtrCloseClientConnection = (LPCloseClientConnection)GetProcAddress(dll_handle, "CloseClientConnection");
	}

	LPCTSTR lpcRecHeader[] = { _T("Client Connected") };
	int nCol = 0;

	m_ctrlListConnected.InsertColumn(nCol, lpcRecHeader[nCol++], LVCFMT_FIXED_WIDTH, 150);
	//m_ctrlListConnected.InsertColumn(nCol, lpcRecHeader[nCol++], LVCFMT_LEFT, 250);
	//m_ctrlListConnected.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_TRACKSELECT | LVS_EDITLABELS);

	AfxBeginThread(ServerThreadFunc, this);
	// TODO: Add extra initialization here
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEnzoChatServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEnzoChatServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEnzoChatServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CEnzoChatServerDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//CDialog::OnOK();
	CString input;

	m_ctrlInputArea.GetWindowText(input);


	input = _T("Server : ")+ input + _T("\r\n");

	UpdateChatAreaText(input);
	try
	{
		for (int i = 0; i < m_vSocket->size(); i++)
		{
			char* sInput = convert_from_wstring(input);
			(*m_vSocket)[i]->Send(sInput);
			free(sInput);
		}
	}
	catch (int nError)
	{

	}
	m_ctrlInputArea.SetWindowText(_T(""));
}


void CEnzoChatServerDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	pfnPtrCloseServer(m_ServerHandle);

	FreeLibrary(dll_handle);
	CDialog::OnClose();
}
