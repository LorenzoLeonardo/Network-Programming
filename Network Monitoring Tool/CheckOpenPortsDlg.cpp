
// CheckOpenPorstDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "CheckOpenPorts.h"
#include "CheckOpenPortsDlg.h"
#include "afxdialogex.h"



CCheckOpenPortsDlg* g_dlg;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

inline void GetLastErrorMessageString(_tstring& str, int nGetLastError)
{
	DWORD dwSize = 0;
	TCHAR lpMessage[1020];

	dwSize = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
		NULL,                // lpsource
		nGetLastError,                 // message id
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
		lpMessage,              // output buffer
		sizeof(lpMessage) / sizeof(TCHAR),     // size of msgbuf, bytes
		NULL);

	str = lpMessage;
}

template <typename Map>
inline bool key_compare(Map const& lhs, Map const& rhs) 
{

	auto pred = [](decltype(*lhs.begin()) a, decltype(a) b)
	{ 
		return a.first == b.first; 
	};

	return lhs.size() == rhs.size()
		&& std::equal(lhs.begin(), lhs.end(), rhs.begin(), pred);
}

class CAboutDlg : public CDialogEx
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

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCheckOpenPortsDlg dialog

mutex mtx_enumPorts;
mutex mtx_lanlistener;
mutex mtx_packetlistenerDownload;
mutex mtx_packetlistenerUpload;

CCheckOpenPortsDlg::CCheckOpenPortsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHECKOPENPORST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_tMonitor = NULL;
	m_nCurrentRowSelected = -1;
	m_bHasClickClose = FALSE;
	dll_handle = NULL;

	m_hBrushBackGround = CreateSolidBrush(RGB(93, 107, 153));
	m_hBrushEditArea = CreateSolidBrush(RGB(255, 255, 255));
}
CCheckOpenPortsDlg::~CCheckOpenPortsDlg()
{
	DeleteObject(m_hBrushBackGround);
	DeleteObject(m_hBrushEditArea);
}
void CCheckOpenPortsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS_IP, m_ctrlIPAddress);
	DDX_Control(pDX, IDC_EDIT_AREA, m_ctrlResult);
	DDX_Control(pDX, IDC_EDITPORT, m_ctrlPortNum);
	DDX_Control(pDX, IDC_PROGRESS_STATUS, m_ctrlProgressStatus);
	DDX_Control(pDX, IDC_BUTTON_PORT, m_ctrlBtnCheckOpenPorts);
	DDX_Control(pDX, IDC_LIST_LAN, m_ctrlLANConnected);
	DDX_Control(pDX, IDC_EDIT_POLLINGTIME, m_ctrlEditPollingTime);
	DDX_Control(pDX, IDC_BUTTON_LISTEN_LAN, m_ctrlBtnListen);
	DDX_Control(pDX, IDC_BUTTON_STOP_LAN, m_ctrlBtnStopListening);
	DDX_Control(pDX, IDC_STATIC_UPTIME, m_ctrlStaticRouterUpTime);
	DDX_Control(pDX, IDC_BUTTON_STOP_SEARCHINGPORTS, m_ctrlBtnStopSearchingPort);
	DDX_Control(pDX, IDC_EDIT_PACKET_REPORT, m_ctrlEditPacketReportArea);
	DDX_Control(pDX, IDC_EDIT_SPEED_DOWN, m_ctrlEditDownloadSpeed);
	DDX_Control(pDX, IDC_EDIT_SPEED_UP, m_ctrlEditUploadSpeed);
	DDX_Control(pDX, IDC_BUTTON_SHOW_PACKETS, m_ctrlBtnShowPacketInfo);
	DDX_Control(pDX, IDC_BUTTON_START_PACKET, m_ctrlBtnListenPackets);
	DDX_Control(pDX, IDC_BUTTON_STOP_PACKET, m_ctrlBtnUnlistenPackets);
}

BEGIN_MESSAGE_MAP(CCheckOpenPortsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PORT, &CCheckOpenPortsDlg::OnBnClickedButtonPort)
	ON_BN_CLICKED(IDC_BUTTON2, &CCheckOpenPortsDlg::OnBnClickedButtonStopSearchingOpenPorts)
	ON_BN_CLICKED(IDC_BUTTON_CHECKPORT, &CCheckOpenPortsDlg::OnBnClickedButtonCheckIfPortOpen)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_EDIT_AREA, &CCheckOpenPortsDlg::OnEnChangeEditArea)
	ON_BN_CLICKED(IDC_BUTTON_LISTEN_LAN, &CCheckOpenPortsDlg::OnBnClickedButtonListenLan)
	ON_BN_CLICKED(IDC_BUTTON_STOP_LAN, &CCheckOpenPortsDlg::OnBnClickedButtonStopLan)
	ON_NOTIFY(NM_CLICK, IDC_LIST_LAN, &CCheckOpenPortsDlg::OnNMClickListLan)
	ON_WM_KEYDOWN()
	ON_NOTIFY(HDN_ITEMKEYDOWN, 0, &CCheckOpenPortsDlg::OnHdnItemKeyDownListLan)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_LAN, &CCheckOpenPortsDlg::OnLvnKeydownListLan)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_LAN, &CCheckOpenPortsDlg::OnNMDblclkListLan)
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_START_PACKET, &CCheckOpenPortsDlg::OnBnClickedButtonStartPacket)
	ON_BN_CLICKED(IDC_BUTTON_STOP_PACKET, &CCheckOpenPortsDlg::OnBnClickedButtonStopPacket)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_PACKETS, &CCheckOpenPortsDlg::OnBnClickedButtonShowPackets)
END_MESSAGE_MAP()


int CCheckOpenPortsDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//m_ctrlLANConnected.Create(LVS_REPORT | LVS_SHOWSELALWAYS | LBS_NOTIFY, CRect(0, 0, 400, 200), this, IDC_LIST_LAN);
	//m_ctrlLANConnected.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_HEADERDRAGDROP | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	return 0;
}
BOOL CCheckOpenPortsDlg::OnInitDialog()
{
	LPCTSTR lpcRecHeader[] = { _T("No."), _T("IP Address"), _T("HostName"), _T("MAC Address"), _T("Download Speed"), _T("Upload Speed") };
	int nCol = 0;
	char szDefaultGateWay[32];

	CDialogEx::OnInitDialog();
	
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
	::SetWindowTheme(GetDlgItem(IDC_STATIC_ROUTER_INFO)->GetSafeHwnd(), _T(""), _T(""));//To change text Color of Group Box
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	g_dlg = this;
	m_bStopSearchingOpenPorts = TRUE;
	m_bLanStop = TRUE;
	m_ctrlBtnStopSearchingPort.EnableWindow(false);
	m_ctrlProgressStatus.ShowWindow(FALSE);
	m_ctrlPortNum.SetWindowText(_T("80"));
	m_ctrlEditPollingTime.SetWindowText(_T("1000"));
	m_ctrlBtnStopListening.EnableWindow(FALSE);
	m_ctrlLANConnected.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_HEADERDRAGDROP | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	dll_handle = LoadLibrary(_T("EnzTCP.dll"));
	if (dll_handle)
	{
		m_pfnPtrEnumOpenPorts = (LPEnumOpenPorts)GetProcAddress(dll_handle, "EnumOpenPorts");
		m_pfnPtrIsPortOpen = (LPIsPortOpen)GetProcAddress(dll_handle, "IsPortOpen");
		m_pfnPtrStartLocalAreaListening = (FNStartLocalAreaListening)GetProcAddress(dll_handle, "StartLocalAreaListening");
		m_pfnPtrStopLocalAreaListening = (FNStopLocalAreaListening)GetProcAddress(dll_handle, "StopLocalAreaListening");
		m_pfnPtrStartSNMP = (FNStartSNMP)GetProcAddress(dll_handle, "StartSNMP");
		m_pfnPtrSNMPGet = (FNSNMPGet)GetProcAddress(dll_handle, "SNMPGet");
		m_pfnPtrEndSNMP = (FNEndSNMP)GetProcAddress(dll_handle, "EndSNMP");
		m_pfnPtrGetDefaultGateway = (FNGetDefaultGateway)GetProcAddress(dll_handle, "GetDefaultGateway");
		m_pfnPtrStopSearchingOpenPorts = (FNStopSearchingOpenPorts)GetProcAddress(dll_handle, "StopSearchingOpenPorts");
		m_pfnPtrStartPacketListener = (FNStartPacketListener)GetProcAddress(dll_handle, "StartPacketListener");
		m_pfnPtrStopPacketListener = (FNStopPacketListener)GetProcAddress(dll_handle, "StopPacketListener");
	}
	m_ctrlLANConnected.InsertColumn(nCol, lpcRecHeader[nCol++], LVCFMT_FIXED_WIDTH, 30);
	m_ctrlLANConnected.InsertColumn(nCol, lpcRecHeader[nCol++], LVCFMT_LEFT, 120);
	m_ctrlLANConnected.InsertColumn(nCol, lpcRecHeader[nCol++], LVCFMT_LEFT, 120);
	m_ctrlLANConnected.InsertColumn(nCol, lpcRecHeader[nCol++], LVCFMT_LEFT, 120);
	m_ctrlLANConnected.InsertColumn(nCol, lpcRecHeader[nCol++], LVCFMT_LEFT, 120);
	m_ctrlLANConnected.InsertColumn(nCol, lpcRecHeader[nCol++], LVCFMT_LEFT, 120);
	m_bShowPacketInfo = true;
	m_ctrlBtnListenPackets.EnableWindow(TRUE);
	m_ctrlBtnUnlistenPackets.EnableWindow(FALSE);
	if (m_pfnPtrGetDefaultGateway(szDefaultGateWay))
		m_ipFilter = szDefaultGateWay;
	else
		m_ipFilter = "127.0.0.1";
	m_ctrlIPAddress.SetWindowText(m_ipFilter);

	m_hThreadRouter = (HANDLE)_beginthreadex(NULL, 0, RouterThread, this, 0, NULL);
//	m_hThreadDownloadSpeed = (HANDLE)_beginthreadex(NULL, 0, DownloadSpeedThread, this, 0, NULL);
//	m_hThreadUploadSpeed = (HANDLE)_beginthreadex(NULL, 0, UploadSpeedThread, this, 0, NULL);

	OnBnClickedButtonListenLan();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

HBRUSH CCheckOpenPortsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			int id = pWnd->GetDlgCtrlID();

			if (id == IDC_EDIT_AREA || id == IDC_EDIT_PACKET_REPORT || id == IDC_EDIT_SPEED_DOWN || id == IDC_EDIT_SPEED_UP)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 255, 255));
				return m_hBrushEditArea;
			}
			
			else
			{
				pDC->SetTextColor(RGB(255, 255, 255));
				pDC->SetBkColor(RGB(204, 213, 240));
				pDC->SetBkMode(TRANSPARENT);
				return m_hBrushBackGround;
			}
		}

		case CTLCOLOR_DLG:
		{
			//pDC->SetTextColor(ENZO_COLOR_WHITE);
			pDC->SetBkColor(RGB(64, 86, 141));
			//pDC->SetBkMode(TRANSPARENT);
			return m_hBrushBackGround;
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetBkColor(RGB(255, 255, 255));
			return m_hBrushEditArea;
			//pDC->SetBkMode(TRANSPARENT);
		}
		default:
			return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	return hbr;
}
void CCheckOpenPortsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCheckOpenPortsDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCheckOpenPortsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
string CCheckOpenPortsDlg::UnicodeToMultiByte(wstring& wstr)
{
	if (wstr.empty()) 
		return "";

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}
wstring CCheckOpenPortsDlg::MultiByteToUnicode(string& str)
{
	if (str.empty())
		return L"";

	int size_needed =MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

void CCheckOpenPortsDlg::Increment()
{
	m_nThread++;
	m_ctrlProgressStatus.SetPos(m_nThread);
}

void CCheckOpenPortsDlg::OnBnClickedButtonPort()
{
	m_bStopSearchingOpenPorts = false;
	m_ctrlBtnStopSearchingPort.EnableWindow(true);
	m_ctrlIPAddress.EnableWindow(FALSE);
	m_ctrlBtnCheckOpenPorts.EnableWindow(FALSE);
	m_ctrlResult.SetWindowText(_T(""));
	m_ctrlProgressStatus.ShowWindow(TRUE);
	m_ctrlProgressStatus.SetRange32(1, MAX_PORT);
	m_nThread = 0;
	m_ctrlIPAddress.GetWindowText(m_IPAddress);
#ifdef UNICODE
	wstring strIP(m_IPAddress.GetBuffer());
	m_pfnPtrEnumOpenPorts(UnicodeToMultiByte(strIP).c_str(), MAX_PORT, CallBackEnumPort);
#else
	m_pfnPtrEnumOpenPorts(m_IPAddress.GetBuffer(), MAX_PORT, CallBackEnumPort);
#endif
	
}

void CCheckOpenPortsDlg::OnBnClickedButtonStopSearchingOpenPorts()
{

	m_pfnPtrStopSearchingOpenPorts();
}

void CCheckOpenPortsDlg::OnBnClickedButtonCheckIfPortOpen()
{
	CString cs;
	CString csPort;

	m_ctrlIPAddress.GetWindowText(cs);
	m_ctrlPortNum.GetWindowText(csPort);
	int nLastError = 0;
#ifdef UNICODE
	wstring wStr(cs.GetBuffer());

	if (m_pfnPtrIsPortOpen(UnicodeToMultiByte(wStr).c_str(), _ttoi(csPort), &nLastError))
	{
#else
	if (m_pfnPtrIsPortOpen(cs.GetBuffer(), _ttoi(csPort), &nLastError))
	{
#endif
		CString csRes;
		csRes = +_T("Port (") + csPort + _T(") Of (") + cs + _T(") is open.\r\n");
	
		g_dlg->m_ctrlResult.SetSel(0, 0);
		g_dlg->m_ctrlResult.ReplaceSel(csRes);
	}
	else
	{
		CString csRes;
		csRes = +_T("Port (") + csPort + _T(") Of (") + cs + _T(") is closed.\r\n");
		g_dlg->m_ctrlResult.SetSel(0, 0);
		g_dlg->m_ctrlResult.ReplaceSel(csRes);
	}

}

void CCheckOpenPortsDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	m_pfnPtrStopLocalAreaListening();
	m_pfnPtrStopSearchingOpenPorts();
	m_pfnPtrStopPacketListener();
	m_bHasClickClose = TRUE;

	if (IsLANStopped() && IsSearchingOpenPortStopped())
	{
		m_pfnPtrEndSNMP();
		WaitForSingleObject(m_hThreadRouter, INFINITE);
		WaitForSingleObject(m_hThreadDownloadSpeed, INFINITE);
		WaitForSingleObject(m_hThreadUploadSpeed, INFINITE);
		CloseHandle(m_hThreadRouter);
		CloseHandle(m_hThreadDownloadSpeed);
		CloseHandle(m_hThreadUploadSpeed);

		if (m_hThreadUploadSpeedList != NULL)
		{
			WaitForSingleObject(m_hThreadUploadSpeedList, INFINITE);
			CloseHandle(m_hThreadUploadSpeedList);
			m_hThreadUploadSpeedList = NULL;
		}

		if (m_hThreadDownloadSpeedList != NULL)
		{
			WaitForSingleObject(m_hThreadDownloadSpeedList, INFINITE);
			CloseHandle(m_hThreadDownloadSpeedList);
			m_hThreadDownloadSpeedList = NULL;
		}
		FreeLibrary(dll_handle);

		CDialog::OnClose();
	}
	else
	{
	//	m_bStopLANClicked = TRUE;
		::MessageBox(this->GetSafeHwnd(), _T("Application is still busy. Make sure all listening activity has stopped."), _T("Network Monitoring Tool"), MB_ICONEXCLAMATION);
	}
}

void CCheckOpenPortsDlg::OnEnChangeEditArea()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CCheckOpenPortsDlg::OnBnClickedButtonListenLan()
{
	// TODO: Add your control notification handler code here
	CString csText;
	CString csPollTime;

	m_ctrlBtnListen.EnableWindow(FALSE);
	m_ctrlBtnStopListening.EnableWindow(TRUE);
	m_vList.clear();
	SetLANStop(false);
	m_ctrlIPAddress.GetWindowText(csText);

#ifdef UNICODE
	wstring wstr(csText.GetBuffer());
	string str = UnicodeToMultiByte(wstr);
	m_ctrlEditPollingTime.GetWindowText(csPollTime);
	if (csPollTime.IsEmpty())
	{
		m_ctrlEditPollingTime.SetWindowText(_T("1000"));
		if (!m_pfnPtrStartLocalAreaListening(str.c_str(), CallbackLANListener, 1000))
			::MessageBox(this->GetSafeHwnd(), _T("Failed to start Local Area Listener."), _T("Local Area Network Listener"), MB_ICONERROR);
	}
	else
	{
		int nPollTime = _ttoi(csPollTime);
		if (!m_pfnPtrStartLocalAreaListening(str.c_str(), CallbackLANListener, nPollTime))
			::MessageBox(this->GetSafeHwnd(), _T("Failed to start Local Area Listener."), _T("Local Area Network Listener"), MB_ICONERROR);
	}
#else
	if (csPollTime.IsEmpty())
	{
		m_ctrlEditPollingTime.SetWindowText(_T("1000"));
		m_pfnPtrStartLocalAreaListening(csText.GetBuffer(), CallbackLANListener, 1000);
	}
	else
	{
		int nPollTime = _ttoi(csPollTime);
		m_pfnPtrStartLocalAreaListening(csText.GetBuffer(), CallbackLANListener, nPollTime);
	}
#endif
}

void CCheckOpenPortsDlg::OnBnClickedButtonStopLan()
{
	// TODO: Add your control notification handler code here
	m_pfnPtrStopLocalAreaListening();
	m_ctrlBtnStopListening.EnableWindow(FALSE);
}

void CCheckOpenPortsDlg::OnNMClickListLan(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	m_nCurrentRowSelected = pNMItemActivate->iItem;
	
	
	if (pNMItemActivate->iItem > -1)
	{
		m_ipFilter = m_ctrlLANConnected.GetItemText(m_nCurrentRowSelected, 1);
		m_ctrlIPAddress.SetWindowText(m_ipFilter);
		wstring temp = m_ipFilter.GetBuffer();
 		inet_pton(AF_INET, UnicodeToMultiByte(temp).c_str(), &m_ulIPFilter);
	}
}

void CCheckOpenPortsDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	g_dlg->m_ctrlLANConnected.SetItemState(g_dlg->m_nCurrentRowSelected, LVIS_SELECTED, LVIS_SELECTED);
	g_dlg->m_ctrlLANConnected.SetFocus();
	CDialogEx::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCheckOpenPortsDlg::OnHdnItemKeyDownListLan(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here

	OnNMClickListLan(pNMHDR, pResult);
}

void CCheckOpenPortsDlg::OnLvnKeydownListLan(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here

	if (pLVKeyDow->wVKey == VK_UP)
	{
		if(g_dlg->m_nCurrentRowSelected > 0)
		{
			g_dlg->m_nCurrentRowSelected--;
			g_dlg->m_ctrlLANConnected.SetItemState(m_nCurrentRowSelected, LVIS_SELECTED, LVIS_SELECTED);
			g_dlg->m_ctrlLANConnected.SetFocus();
			CString cs = m_ctrlLANConnected.GetItemText(m_nCurrentRowSelected, 1);
			m_ctrlIPAddress.SetWindowText(cs);
		}
	}
	else if (pLVKeyDow->wVKey == VK_DOWN)
	{
		if (g_dlg->m_nCurrentRowSelected < (g_dlg->m_ctrlLANConnected.GetItemCount()-1))
		{
			g_dlg->m_nCurrentRowSelected++;
			g_dlg->m_ctrlLANConnected.SetItemState(m_nCurrentRowSelected, LVIS_SELECTED, LVIS_SELECTED);
			g_dlg->m_ctrlLANConnected.SetFocus();
			CString cs = m_ctrlLANConnected.GetItemText(m_nCurrentRowSelected, 1);
			m_ctrlIPAddress.SetWindowText(cs);
		}
	}

}

void CCheckOpenPortsDlg::OnNMDblclkListLan(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if (pNMItemActivate->iItem >= 0)
	{
		if ((m_nThread >= MAX_PORT) || m_bStopSearchingOpenPorts)
		{
			OnBnClickedButtonPort();
		}
		else
		{
			::MessageBox(this->GetSafeHwnd(), _T("Port Listener is still busy!"), _T("Port Listener Busy"), MB_ICONEXCLAMATION);
		}
		
	}
}
unsigned __stdcall  CCheckOpenPortsDlg::DownloadSpeedThread(void* parg)
{
	CCheckOpenPortsDlg* pDlg = (CCheckOpenPortsDlg*)parg;
	CString cs;
	ULONG prev = 0, current = 0;
	ULONGLONG timePrev = GetTickCount64(), timeCurrent;
	float fDownSpeed = 0;

	pDlg->SetDownloadSize(0);
	prev = pDlg->GetDownloadSize();
	while (!pDlg->HasClickClose())
	{
		timeCurrent = GetTickCount64();
		current = pDlg->GetDownloadSize();
		if ((timeCurrent - timePrev) >= POLLING_TIME)
		{
			fDownSpeed = ((float)(current - prev) / (float)(timeCurrent - timePrev)) * 8;
			if (fDownSpeed < 1000)
				cs.Format(_T("Download: %.2f Kbps"), fDownSpeed);
			else
				cs.Format(_T("Download: %.2f Mbps"), fDownSpeed / 1000);
			pDlg->SetDownloadSpeedText(cs);
			timePrev = timeCurrent;

			pDlg->SetDownloadSize(0);
			prev = pDlg->GetDownloadSize();
		}
	}
	return 0;
}
unsigned __stdcall  CCheckOpenPortsDlg::DownloadSpeedThreadList(void* parg)
{
	CCheckOpenPortsDlg* pDlg = (CCheckOpenPortsDlg*)parg;
	map<ULONG, ULONG> mPrev, mCurrent;
	ULONGLONG timePrev = GetTickCount64(), timeCurrent;
	double fDownSpeed = 0;

	mtx_packetlistenerDownload.lock();
	map<ULONG, ENZ_CONNECTED_DEVICE_DETAILS>::iterator it = pDlg->m_mConnectedBefore.begin();
	while (it != pDlg->m_mConnectedBefore.end())
	{
		pDlg->SetDownloadSize(it->first, 0);
		it++;
	}
	it = pDlg->m_mConnectedBefore.begin();
	while (it != pDlg->m_mConnectedBefore.end())
	{
		mPrev[it->first]= pDlg->GetDownloadSize(it->first);
		it++;
	}
	mtx_packetlistenerDownload.unlock();
	while (!pDlg->HasClickClose() && !pDlg->IsPacketStopped())
	{
		mtx_packetlistenerDownload.lock();
		it = pDlg->m_mConnectedBefore.begin();
		while (it != pDlg->m_mConnectedBefore.end())
		{
			mCurrent[it->first] = pDlg->GetDownloadSize(it->first);
			it++;
		}
		mtx_packetlistenerDownload.unlock();
		timeCurrent = GetTickCount64();

		if ((timeCurrent - timePrev) >= POLLING_TIME)
		{
			mtx_packetlistenerDownload.lock();
			map<ULONG, ENZ_CONNECTED_DEVICE_DETAILS>::iterator it = g_dlg->m_mConnectedBefore.begin();
			int nRow = 0, col = 0;
			WCHAR* temp = NULL;
			
			CString format;

			it = pDlg->m_mConnectedBefore.begin();
			while (it != pDlg->m_mConnectedBefore.end())
			{
				LVFINDINFO lvFindInfo;
				lvFindInfo.flags = LVFI_PARTIAL | LVFI_STRING;
				lvFindInfo.psz = it->second.m_vIPHOSTMAC[0].GetBuffer();
				
				
				LVITEM lvItem;
				lvItem.mask = LVIF_TEXT;
				int findResult = -1;

				for (int i = 0; i < g_dlg->m_ctrlLANConnected.GetItemCount(); ++i)
				{
					CString szText = g_dlg->m_ctrlLANConnected.GetItemText(i, 1);
					if (szText == it->second.m_vIPHOSTMAC[0])
					{
						findResult = i;
						break;
					}
				}

				if (findResult !=-1)
				{
					fDownSpeed = ((double)(mCurrent[it->first] - mPrev[it->first]) / (double)(timeCurrent - timePrev)) * 8;
					pDlg->m_mConnectedBefore[it->first].m_lfDownloadSpeed = fDownSpeed;
					if(fDownSpeed <= 1000)
						format.Format(_T("%.2f Kbps"), fDownSpeed);
					else
						format.Format(_T("%.2f Mbps"), fDownSpeed/1000);
					lvItem.iItem = findResult;
					lvItem.iSubItem = 4;
					lvItem.pszText = format.GetBuffer();
					g_dlg->m_ctrlLANConnected.SetItem(&lvItem);
				}
				it++;
			}
			timePrev = timeCurrent;
			it = pDlg->m_mConnectedBefore.begin();
			while (it != pDlg->m_mConnectedBefore.end())
			{
				pDlg->SetDownloadSize(it->first, 0);
				it++;
			}
			it = pDlg->m_mConnectedBefore.begin();
			while (it != pDlg->m_mConnectedBefore.end())
			{
				mPrev[it->first] = pDlg->GetDownloadSize(it->first);
				it++;
			}
			mtx_packetlistenerDownload.unlock();
		}
	}
	return 0;
}

unsigned __stdcall  CCheckOpenPortsDlg::UploadSpeedThread(void* parg)
{
	CCheckOpenPortsDlg* pDlg = (CCheckOpenPortsDlg*)parg;
	CString cs;
	ULONG prev = 0, current = 0;
	ULONGLONG timePrev = GetTickCount64(), timeCurrent;
	float fUpSpeed = 0;

	pDlg->SetUploadSize(0);
	prev = pDlg->GetUploadSize();
	while (!pDlg->HasClickClose())
	{
		timeCurrent = GetTickCount64();
		current = pDlg->GetUploadSize();
		if ((timeCurrent - timePrev) >= POLLING_TIME)
		{
			fUpSpeed = ((float)(current - prev) / (float)(timeCurrent - timePrev)) * 8;
			if (fUpSpeed < 1000)
				cs.Format(_T("Upload: %.2f Kbps"), fUpSpeed);
			else
				cs.Format(_T("Upload: %.2f Mbps"), fUpSpeed / 1000);
			pDlg->SetUploadSpeedText(cs);
			timePrev = timeCurrent;

			pDlg->SetUploadSize(0);
			prev = pDlg->GetUploadSize();
		}
	}
	return 0;
}

unsigned __stdcall  CCheckOpenPortsDlg::UploadSpeedThreadList(void* parg)
{
	CCheckOpenPortsDlg* pDlg = (CCheckOpenPortsDlg*)parg;
	map<ULONG, ULONG> mPrev, mCurrent;
	ULONGLONG timePrev = GetTickCount64(), timeCurrent;
	double fUpSpeed = 0;

	mtx_packetlistenerUpload.lock();
	map<ULONG, ENZ_CONNECTED_DEVICE_DETAILS>::iterator it = pDlg->m_mConnectedBefore.begin();
	while (it != pDlg->m_mConnectedBefore.end())
	{
		pDlg->SetUploadSize(it->first, 0);
		it++;
	}
	it = pDlg->m_mConnectedBefore.begin();
	while (it != pDlg->m_mConnectedBefore.end())
	{
		mPrev[it->first] = pDlg->GetUploadSize(it->first);
		it++;
	}
	mtx_packetlistenerUpload.unlock();
	while (!pDlg->HasClickClose() && !pDlg->IsPacketStopped())
	{
		mtx_packetlistenerUpload.lock();
		it = pDlg->m_mConnectedBefore.begin();
		while (it != pDlg->m_mConnectedBefore.end())
		{
			mCurrent[it->first] = pDlg->GetUploadSize(it->first);
			it++;
		}
		mtx_packetlistenerUpload.unlock();
		timeCurrent = GetTickCount64();

		if ((timeCurrent - timePrev) >= POLLING_TIME)
		{
			mtx_packetlistenerUpload.lock();
			map<ULONG, ENZ_CONNECTED_DEVICE_DETAILS>::iterator it = g_dlg->m_mConnectedBefore.begin();
			int nRow = 0, col = 0;
			WCHAR* temp = NULL;

			CString format;

			it = pDlg->m_mConnectedBefore.begin();
			while (it != pDlg->m_mConnectedBefore.end())
			{
				LVFINDINFO lvFindInfo;
				lvFindInfo.flags = LVFI_PARTIAL | LVFI_STRING;
				lvFindInfo.psz = it->second.m_vIPHOSTMAC[0].GetBuffer();


				LVITEM lvItem;
				lvItem.mask = LVIF_TEXT;
				int findResult = -1;

				for (int i = 0; i < g_dlg->m_ctrlLANConnected.GetItemCount(); ++i)
				{
					CString szText = g_dlg->m_ctrlLANConnected.GetItemText(i, 1);
					if (szText == it->second.m_vIPHOSTMAC[0])
					{
						findResult = i;
						break;
					}
				}

				if (findResult != -1)
				{
					fUpSpeed = ((double)(mCurrent[it->first] - mPrev[it->first]) / (double)(timeCurrent - timePrev)) * 8;
					pDlg->m_mConnectedBefore[it->first].m_lfUploadSpeed = fUpSpeed;
					if (fUpSpeed <= 1000)
						format.Format(_T("%.2f Kbps"), fUpSpeed);
					else
						format.Format(_T("%.2f Mbps"), fUpSpeed / 1000);
					lvItem.iItem = findResult;
					lvItem.iSubItem = 5;
					lvItem.pszText = format.GetBuffer();
					g_dlg->m_ctrlLANConnected.SetItem(&lvItem);
				}
				it++;
			}
			timePrev = timeCurrent;
			it = pDlg->m_mConnectedBefore.begin();
			while (it != pDlg->m_mConnectedBefore.end())
			{
				pDlg->SetUploadSize(it->first, 0);
				it++;
			}
			it = pDlg->m_mConnectedBefore.begin();
			while (it != pDlg->m_mConnectedBefore.end())
			{
				mPrev[it->first] = pDlg->GetUploadSize(it->first);
				it++;
			}
			mtx_packetlistenerUpload.unlock();
		}
	}
	return 0;
}
// CCheckOpenPortsDlg message handlers
unsigned __stdcall  CCheckOpenPortsDlg::RouterThread(void* parg)
{
	CCheckOpenPortsDlg* pDlg = (CCheckOpenPortsDlg*)parg;
	char szDefaultGateway[40];
	memset(szDefaultGateway, 0, sizeof(szDefaultGateway));
	CString csBrand = _T("");
	CString csModel = _T("");
	CString csDesc = _T("");
	string sTemp;

	if (pDlg->m_pfnPtrGetDefaultGateway(szDefaultGateway))
	{
		DWORD error = 0;
		smiVALUE value;
		if (pDlg->m_pfnPtrStartSNMP(szDefaultGateway, "public", 1, error))
		{

			value = pDlg->m_pfnPtrSNMPGet(".1.3.6.1.2.1.1.6.0", error);//Brand name
			if (error != SNMPAPI_SUCCESS)
			{
				return 0;
			}
#ifdef UNICODE
			sTemp = (char*)value.value.string.ptr;
			csBrand = pDlg->MultiByteToUnicode(sTemp).c_str();
#else
			csBrand = value.value.string.ptr;
#endif
			
			value = pDlg->m_pfnPtrSNMPGet(".1.3.6.1.2.1.1.5.0", error);//Model name
			if (error != SNMPAPI_SUCCESS)
			{
				return 0;
			}
#ifdef UNICODE
			sTemp = (char*)value.value.string.ptr;
			csModel = pDlg->MultiByteToUnicode(sTemp).c_str();
#else
			csModel = value.value.string.ptr;
#endif
			//pDlg->SetRouterBrand(cs);

			value = pDlg->m_pfnPtrSNMPGet(".1.3.6.1.2.1.1.1.0", error);//decription
			if (error != SNMPAPI_SUCCESS)
			{
				return 0;
			}
#ifdef UNICODE
			sTemp = (char*)value.value.string.ptr;
			csDesc = pDlg->MultiByteToUnicode(sTemp).c_str();
#else
			csDesc = value.value.string.ptr;
#endif
		}
		CString csFormat;
		while (!pDlg->HasClickClose())
		{
			value = pDlg->m_pfnPtrSNMPGet(".1.3.6.1.2.1.25.1.1.0", error);//time
			if (error != SNMPAPI_SUCCESS)
			{
				break;
			}
			ULONG ulDays = value.value.uNumber / 8640000;
			double fRem = remainder(value.value.uNumber / (double)8640000, (double)8640000) - ulDays;
			ULONG ulHour = fRem * 24;
			fRem = (double)(fRem * 24) - ulHour;
			ULONG ulMin = fRem * 60;
			fRem = (double)(fRem * 60) - ulMin;
			ULONG ulSec = fRem * 60;
	
			csFormat=_T("");
			csFormat.Format(_T("%s %s %s\r\n\r\nRouter's Up Time\r\n%u days, %u hours, %u min, %u secs"),	csBrand.GetBuffer(), csModel.GetBuffer(), csDesc.GetBuffer(), ulDays, ulHour, ulMin, ulSec);

			pDlg->SetRouterUpTime(csFormat);
			Sleep(500);
		}
	}
	return 0;
}

void CCheckOpenPortsDlg::CallbackLANListener(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsopen)
{
	mtx_lanlistener.lock();
	mtx_packetlistenerDownload.lock();
	mtx_packetlistenerUpload.lock();
	if (bIsopen)
	{
		ULONG ipaddr;
		CString csTemp;
		ENZ_CONNECTED_DEVICE_DETAILS tDeviceDetails;

		inet_pton(AF_INET, ipAddress, &ipaddr);
		
		memset(&tDeviceDetails, 0, sizeof(ENZ_CONNECTED_DEVICE_DETAILS));

		
		csTemp = (char*)ipAddress;
		tDeviceDetails.m_vIPHOSTMAC.push_back(csTemp);
		csTemp = (char*)hostName;
		tDeviceDetails.m_vIPHOSTMAC.push_back(csTemp);
		csTemp = (char*)macAddress;
		tDeviceDetails.m_vIPHOSTMAC.push_back(csTemp);
		g_dlg->m_mConnected[ipaddr] = tDeviceDetails;
	}
	else
	{
		if (strcmp(ipAddress, "end") == 0)
		{
			if ((g_dlg->m_mConnected.size() == g_dlg->m_mConnectedBefore.size()) &&
				key_compare(g_dlg->m_mConnected, g_dlg->m_mConnectedBefore))
			{
				g_dlg->m_mConnected.clear();
				mtx_lanlistener.unlock();
				mtx_packetlistenerDownload.unlock();
				mtx_packetlistenerUpload.unlock();
				return;
			}
			g_dlg->m_mConnectedBefore = g_dlg->m_mConnected;
			g_dlg->m_ctrlLANConnected.DeleteAllItems();

			int col = 0;
			map<ULONG, ENZ_CONNECTED_DEVICE_DETAILS>::iterator it = g_dlg->m_mConnectedBefore.begin();
			int nRow = 0;
			WCHAR* temp = NULL;
			CString format;
			while (it != g_dlg->m_mConnectedBefore.end())
			{
#ifdef UNICODE
				g_dlg->m_ctrlLANConnected.InsertItem(LVIF_TEXT | LVIF_STATE, nRow,
					to_wstring(nRow + 1).c_str(), 0, 0, 0, 0);

				g_dlg->m_ctrlLANConnected.SetItemText(nRow, col + 1, it->second.m_vIPHOSTMAC[0]);
				g_dlg->m_ctrlLANConnected.SetItemText(nRow, col + 2, it->second.m_vIPHOSTMAC[1]);
				g_dlg->m_ctrlLANConnected.SetItemText(nRow, col + 3, it->second.m_vIPHOSTMAC[2]);
				if (it->second.m_lfDownloadSpeed <= 1000)
					format.Format(_T("%.2f Kbps"), it->second.m_lfDownloadSpeed);
				else
					format.Format(_T("%.2f Mbps"), it->second.m_lfDownloadSpeed / 1000);
				g_dlg->m_ctrlLANConnected.SetItemText(nRow, col + 4, format);

				if (it->second.m_lfUploadSpeed <= 1000)
					format.Format(_T("%.2f Kbps"), it->second.m_lfUploadSpeed);
				else
					format.Format(_T("%.2f Mbps"), it->second.m_lfUploadSpeed / 1000);
				g_dlg->m_ctrlLANConnected.SetItemText(nRow, col + 5, format);
#else 
				g_dlg->m_ctrlLANConnected.InsertItem(LVIF_TEXT | LVIF_STATE, nRow,
					to_string(nRow + 1).c_str(), 0, 0, 0, 0);

				g_dlg->m_ctrlLANConnected.SetItemText(nRow, col + 1, it->second.m_vIPHOSTMAC[0]);
				g_dlg->m_ctrlLANConnected.SetItemText(nRow, col + 2, it->second.m_vIPHOSTMAC[1]);
				g_dlg->m_ctrlLANConnected.SetItemText(nRow, col + 3, it->second.m_vIPHOSTMAC[2]);
#endif
				it++;
				nRow++;
			}
			if ((g_dlg->m_mConnectedBefore.size() - 1) < g_dlg->m_nCurrentRowSelected)
				g_dlg->m_nCurrentRowSelected = (int)g_dlg->m_mConnectedBefore.size() - 1;

			g_dlg->m_ctrlLANConnected.SetItemState(g_dlg->m_nCurrentRowSelected, LVIS_SELECTED, LVIS_SELECTED);
			g_dlg->m_ctrlLANConnected.SetFocus();
			g_dlg->m_mConnected.clear();
		}
		else if (strcmp(ipAddress, "stop") == 0)
		{
			g_dlg->m_ctrlBtnListen.EnableWindow(TRUE);
			g_dlg->m_ctrlBtnStopListening.EnableWindow(FALSE);
			g_dlg->SetLANStop(true);
		}
	}
	mtx_lanlistener.unlock();
	mtx_packetlistenerDownload.unlock();
	mtx_packetlistenerUpload.unlock();
}

void CCheckOpenPortsDlg::CallBackEnumPort(char* ipAddress, int nPort, bool bIsopen, int nLastError)
{
	if (ipAddress != NULL)
	{
		if(strcmp(ipAddress,"DONE") == 0)
		{
			g_dlg->m_ctrlIPAddress.EnableWindow(TRUE);
			g_dlg->m_ctrlBtnCheckOpenPorts.EnableWindow(TRUE);
			g_dlg->m_ctrlProgressStatus.ShowWindow(FALSE);
			g_dlg->m_ctrlBtnStopSearchingPort.EnableWindow(false);
			g_dlg->SetStopSearchingOpenPort();
		}
		else
		{
			mtx_enumPorts.lock();
			if (bIsopen)
			{
				CString csStr;

				string sTemp = ipAddress;

				_tstring wsLastError;
				GetLastErrorMessageString(wsLastError, nLastError);
				if (bIsopen)
					csStr.Format(_T("%s %d is open.\r\n"), g_dlg->MultiByteToUnicode(sTemp).c_str(), nPort);

				long nLength = g_dlg->m_ctrlResult.GetWindowTextLength();
				g_dlg->m_ctrlResult.SetSel(0, 0);
				g_dlg->m_ctrlResult.ReplaceSel(csStr);
			}
			g_dlg->Increment();
			mtx_enumPorts.unlock();
		}
	}
}

bool CCheckOpenPortsDlg::CallPacketListener(unsigned char* buffer, int nSize)
{
	CString csText, csSrcPort, csDestPort, sourceIP, destIP, cs, csTemp, ipFilter;
	int iphdrlen = 0, nDataSize = 0;
	IPV4_HDR* iphdr;
	TCP_HDR* tcpheader = NULL;
	UDP_HDR* udpheader = NULL;
	string sTemp;
	char sztemp[32];
	memset(sztemp, 0, sizeof(sztemp));

	iphdr = (IPV4_HDR*)buffer;
	iphdrlen = iphdr->ucIPHeaderLen * 4;
	
	if (g_dlg->ShowPacketInfo())
	{
		inet_ntop(AF_INET, (const void*)&iphdr->unDestaddress, sztemp, sizeof(sztemp));
		destIP = sztemp;
		inet_ntop(AF_INET, (const void*)&iphdr->unSrcaddress, sztemp, sizeof(sztemp));
		sourceIP = sztemp;

		switch (iphdr->ucIPProtocol)
		{
			case ICMP_PROTOCOL:
			{
				csText = _T("ICMP : ");
				break;
			}
			case IGMP_PROTOCOL:
			{
				csText = _T("IGMP : ");
				break;
			}
			case TCP_PROTOCOL:
			{
				tcpheader = (TCP_HDR*)(buffer + iphdrlen);
				csSrcPort = to_wstring(ntohs(tcpheader->usSourcePort)).c_str();
				csDestPort = to_wstring(ntohs(tcpheader->usDestPort)).c_str();
				csText = _T("TCP : ");
				break;
			}
			case UDP_PROTOCOL:
			{
				udpheader = (UDP_HDR*)(buffer + iphdrlen);
				csSrcPort = to_wstring(ntohs(udpheader->usSourcePort)).c_str();
				csDestPort = to_wstring(ntohs(udpheader->usDestPort)).c_str();
				csText = _T("UDP : ");
				break;
			}
			default:
			{
				csText = _T("OTHERS : ");
				break;
			}
		}
	}
	
	g_dlg->SetDownloadSize(iphdr->unDestaddress, g_dlg->GetDownloadSize(iphdr->unDestaddress) + nSize);
	g_dlg->SetUploadSize(iphdr->unDestaddress, g_dlg->GetUploadSize(iphdr->unSrcaddress) + nSize);

	if (g_dlg->GetIPFilterULONG() == iphdr->unDestaddress)
	{
		g_dlg->SetDownloadSize(g_dlg->GetDownloadSize() + nSize);
		if (g_dlg->ShowPacketInfo())
		{
			csText += sourceIP + _T(":") + csSrcPort + _T(" -> ") + destIP + _T(":") + csDestPort + _T(" Size: ") + to_wstring(nSize).c_str() + _T(" bytes\r\n");
			g_dlg->m_ctrlEditPacketReportArea.GetWindowText(csTemp);
			long nLength = csTemp.GetLength();
			if (nLength < 5000)
			{
				g_dlg->m_ctrlEditPacketReportArea.SetSel(0, 0);
				g_dlg->m_ctrlEditPacketReportArea.ReplaceSel(csText);
			}
			else
			{
				g_dlg->m_ctrlEditPacketReportArea.GetWindowText(csTemp);
				csTemp = csTemp.Left(csTemp.ReverseFind(_T('\r')));
				g_dlg->m_ctrlEditPacketReportArea.SetWindowText(csTemp);
			}
		}
	}
	if (g_dlg->GetIPFilterULONG() == iphdr->unSrcaddress)
	{
		g_dlg->SetUploadSize(g_dlg->GetUploadSize() + nSize);
		if (g_dlg->ShowPacketInfo())
		{
			csText += sourceIP + _T(":") + csSrcPort + _T(" -> ") + destIP + _T(":") + csDestPort + _T(" Size: ") + to_wstring(nSize).c_str() + _T(" bytes\r\n");
			g_dlg->m_ctrlEditPacketReportArea.GetWindowText(csTemp);
			long nLength = csTemp.GetLength();
			if (nLength < 5000)
			{
				g_dlg->m_ctrlEditPacketReportArea.SetSel(0, 0);
				g_dlg->m_ctrlEditPacketReportArea.ReplaceSel(csText);
			}
			else
			{
				g_dlg->m_ctrlEditPacketReportArea.GetWindowText(csTemp);
				csTemp = csTemp.Left(csTemp.ReverseFind(_T('\r')));
				g_dlg->m_ctrlEditPacketReportArea.SetWindowText(csTemp);
			}
		}
	}
	return true;
}
void CCheckOpenPortsDlg::OnBnClickedButtonStartPacket()
{
	// TODO: Add your control notification handler code here
	m_bStopPacketListener = false;
	m_ctrlBtnListenPackets.EnableWindow(FALSE);
	m_ctrlBtnUnlistenPackets.EnableWindow(TRUE);
	if (!m_pfnPtrStartPacketListener(CallPacketListener))
		::MessageBox(this->GetSafeHwnd(), _T("Packet Listener failed to start. Please run the tool as Administrator. To run as administrator, right click on the executable file and click run as administrator."), _T("Run as Administrator"), MB_ICONEXCLAMATION);
	
	m_hThreadDownloadSpeedList = (HANDLE)_beginthreadex(NULL, 0, DownloadSpeedThreadList, this, 0, NULL);
	m_hThreadUploadSpeedList = (HANDLE)_beginthreadex(NULL, 0, UploadSpeedThreadList, this, 0, NULL);
}


void CCheckOpenPortsDlg::OnBnClickedButtonStopPacket()
{
	// TODO: Add your control notification handler code here
	m_bStopPacketListener = true;
	m_pfnPtrStopPacketListener();
	m_ctrlBtnListenPackets.EnableWindow(TRUE);
	m_ctrlBtnUnlistenPackets.EnableWindow(FALSE);


	WaitForSingleObject(m_hThreadUploadSpeedList, INFINITE);
	WaitForSingleObject(m_hThreadDownloadSpeedList, INFINITE);
	CloseHandle(m_hThreadUploadSpeedList);
	CloseHandle(m_hThreadDownloadSpeedList);

	m_hThreadUploadSpeedList = NULL;
	m_hThreadDownloadSpeedList = NULL;
}


void CCheckOpenPortsDlg::OnBnClickedButtonShowPackets()
{
	// TODO: Add your control notification handler code here
	if (m_bShowPacketInfo)
	{
		m_ctrlBtnShowPacketInfo.SetWindowText(_T("Hide Packet Info"));
		m_bShowPacketInfo = false;
		m_ctrlEditPacketReportArea.ShowWindow(false);
	}
	else
	{
		m_ctrlBtnShowPacketInfo.SetWindowText(_T("Show Packet Info"));
		m_bShowPacketInfo = true;
		m_ctrlEditPacketReportArea.ShowWindow(true);
	}
}
