
// CheckOpenPorstDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "CheckOpenPorts.h"
#include "CheckOpenPortsDlg.h"
#include "afxdialogex.h"
#include "CSaveDeviceInfoDlg.h"
#include "../EnzTCP/DebugLog.h"

CCheckOpenPortsDlg* g_pCCheckOpenPortsDlg;
mutex mtx_enumPorts;
mutex mtx_lanlistener;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
CCheckOpenPortsDlg::CCheckOpenPortsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHECKOPENPORST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nCurrentRowSelected = -1;
	m_bHasClickClose = FALSE;
	m_hDLLhandle = NULL;
	m_hThreadRouter = NULL;
	m_hThreadLANListener = NULL;
	m_hNICPacketListener = NULL;
	m_hThreadClock = NULL;
	m_hThreadOpenPortListener = NULL;
	m_hThreadNICListener = NULL;
	m_bOnCloseWasCalled = false;
	m_hBrushBackGround = CreateSolidBrush(RGB(93, 107, 153));
	m_hBrushEditArea = CreateSolidBrush(RGB(255, 255, 255));
	m_pmodeless = NULL;
	m_pfnPtrSNMPGet = NULL;
	m_pfnPtrEndSNMP = NULL;
	m_pfnPtrGetDefaultGateway = NULL;
	m_pfnPtrStartSNMP = NULL;
	m_pfnPtrEnumOpenPorts = NULL;
	m_pfnPtrIsPortOpen = NULL;
	m_pfnPtrStopSearchingOpenPorts = NULL;
	m_pfnPtrGetNetworkDeviceStatus = NULL;
	m_pfnPtrEnumNetworkAdapters = NULL;
	m_pfnPtrGetDefaultGatewayEx = NULL;
	m_fnptrCreatePacketListenerEx = NULL;
	m_fnptrStartPacketListenerEx = NULL;
	m_fnptrStopPacketListenerEx = NULL;
	m_fnptrDeletePacketListenerEx = NULL;
	m_fnptrCreateLocalAreaListenerEx = NULL;
	m_fnptrStartLocalAreaListenerEx = NULL;
	m_fnptrStopLocalAreaListenerEx = NULL;
	m_fnptrDeleteLocalAreaListenerEx = NULL;
	m_fnptrSetNICAdapterToUse = NULL;
	g_pCCheckOpenPortsDlg = this;
	m_bStopSearchingOpenPorts = TRUE;
	m_bLanStop = TRUE;
	m_bIsInternetOnly = false;
	m_bShowPacketInfo = false;
	m_customClock.SetFontStyle(_T("Yu Gothic UI"));
	m_customClock.SetFontSize(40);
	m_customClock.SetFontWeight(FW_NORMAL);
	m_customClock.SetTextColor(RGB(255, 255, 255));
	m_customClock.SetTextBKColor(RGB(93, 107, 153));
	m_customClock.CreateClock();
}
CCheckOpenPortsDlg::~CCheckOpenPortsDlg()
{
	DeleteObject(m_hBrushBackGround);
	DeleteObject(m_hBrushEditArea);
	m_customClock.DestroyClock();
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
	//DDX_Control(pDX, IDC_EDIT_POLLINGTIME, m_ctrlEditPollingTime);
	DDX_Control(pDX, IDC_BUTTON_LISTEN_LAN, m_ctrlBtnListen);
	DDX_Control(pDX, IDC_BUTTON_STOP_LAN, m_ctrlBtnStopListening);
	DDX_Control(pDX, IDC_STATIC_UPTIME, m_ctrlStaticRouterUpTime);
	DDX_Control(pDX, IDC_BUTTON_STOP_SEARCHINGPORTS, m_ctrlBtnStopSearchingPort);
	DDX_Control(pDX, IDC_BUTTON_SHOW_PACKETS, m_ctrlBtnShowPacketInfo);
	DDX_Control(pDX, IDC_BUTTON_START_PACKET, m_ctrlBtnListenPackets);
	DDX_Control(pDX, IDC_BUTTON_STOP_PACKET, m_ctrlBtnUnlistenPackets);
	DDX_Control(pDX, IDC_STATIC_PICTURE, m_ctrlStaticLogo);
	DDX_Control(pDX, IDC_CHECK_DEBUG, m_ctrlBtnDebug);
	DDX_Control(pDX, IDC_EDIT_ADAPTER_INFO, m_ctrlEditAdapterInfo);
	DDX_Control(pDX, IDC_COMBO_LIST_ADAPTER, m_ctrlComboAdapterList);
	DDX_Control(pDX, IDC_STATIC_NIC_LISTEN, m_ctrlStaticNICListen);
	DDX_Control(pDX, IDC_STATIC_ROUTER_PIC, m_ctrlStaticRouterImage);
	DDX_Control(pDX, IDC_STATIC_NUM_DEVICE, m_ctrlStaticNumDevice);
}

BEGIN_MESSAGE_MAP(CCheckOpenPortsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PORT, &CCheckOpenPortsDlg::OnBnClickedButtonStartSearchingOpenPort)
	ON_BN_CLICKED(IDC_BUTTON2, &CCheckOpenPortsDlg::OnBnClickedButtonStopSearchingOpenPorts)
	ON_BN_CLICKED(IDC_BUTTON_CHECKPORT, &CCheckOpenPortsDlg::OnBnClickedButtonCheckIfPortOpen)
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_EDIT_AREA, &CCheckOpenPortsDlg::OnEnChangeEditArea)
	ON_BN_CLICKED(IDC_BUTTON_LISTEN_LAN, &CCheckOpenPortsDlg::OnBnClickedButtonStartListenLan)
	ON_BN_CLICKED(IDC_BUTTON_STOP_LAN, &CCheckOpenPortsDlg::OnBnClickedButtonStopListenLan)
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
	ON_WM_MOUSEMOVE()
	ON_WM_MOVE()	
	ON_BN_CLICKED(IDC_CHECK_DEBUG, &CCheckOpenPortsDlg::OnBnClickedCheckDebug)
	ON_CBN_SELCHANGE(IDC_COMBO_LIST_ADAPTER, &CCheckOpenPortsDlg::OnCbnSelchangeComboListAdapter)
	ON_WM_MOVING()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_NCMOUSELEAVE()
	ON_BN_CLICKED(IDC_CHECK_INTERNET_ONLY, &CCheckOpenPortsDlg::OnBnClickedCheckInternetOnly)
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCMOUSEHOVER()
	ON_WM_NCMOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()


int CCheckOpenPortsDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}
void CCheckOpenPortsDlg::UpdateClock()
{
	CClientDC cdc(this);
	m_customClock.DrawClock(&cdc,450, 50);
}
unsigned __stdcall  CCheckOpenPortsDlg::ClockThread(void* parg)
{
	DEBUG_LOG(_T("CCheckOpenPortsDlg::ClockThread() has started."));
	CCheckOpenPortsDlg* pDlg = (CCheckOpenPortsDlg*)parg;
	while(!pDlg->HasClickClose())
	{
		pDlg->UpdateClock();
		Sleep(POLLING_TIME);
	}
	DEBUG_LOG(_T("CCheckOpenPortsDlg::ClockThread() has ended."));
	return 0;
}
unsigned __stdcall  CCheckOpenPortsDlg::OpenPortListenerThread(void* parg)
{
	DEBUG_LOG(_T("CCheckOpenPortsDlg::OpenPortListenerThread() has started."));
	CCheckOpenPortsDlg* pDlg = (CCheckOpenPortsDlg*)parg;

	pDlg->m_ctrlIPAddress.GetWindowText(pDlg->m_IPAddress);

	pDlg->m_pfnPtrEnumOpenPorts(CW2A(pDlg->m_IPAddress), MAX_PORT, CallBackEnumPort);

	DEBUG_LOG(_T("CCheckOpenPortsDlg::OpenPortListenerThread() has ended."));
	return 0;
}

void CCheckOpenPortsDlg::InitAdapterUI()
{
	m_ctrlComboAdapterList.SetCurSel(0);
	OnCbnSelchangeComboListAdapter();
}
bool CCheckOpenPortsDlg::InitDLL()
{
	m_hDLLhandle = LoadLibrary(_T("EnzTCP.dll"));
	if (m_hDLLhandle)
	{
		m_pfnPtrEnumOpenPorts = (LPEnumOpenPorts)GetProcAddress(m_hDLLhandle, "EnumOpenPorts");
		m_pfnPtrIsPortOpen = (LPIsPortOpen)GetProcAddress(m_hDLLhandle, "IsPortOpen");
		m_pfnPtrStartSNMP = (FNStartSNMP)GetProcAddress(m_hDLLhandle, "StartSNMP");
		m_pfnPtrSNMPGet = (FNSNMPGet)GetProcAddress(m_hDLLhandle, "SNMPGet");
		m_pfnPtrEndSNMP = (FNEndSNMP)GetProcAddress(m_hDLLhandle, "EndSNMP");
		m_pfnPtrGetDefaultGateway = (FNGetDefaultGateway)GetProcAddress(m_hDLLhandle, "GetDefaultGateway");
		m_pfnPtrStopSearchingOpenPorts = (FNStopSearchingOpenPorts)GetProcAddress(m_hDLLhandle, "StopSearchingOpenPorts");
		m_pfnPtrGetNetworkDeviceStatus = (FNGetNetworkDeviceStatus)GetProcAddress(m_hDLLhandle, "GetNetworkDeviceStatus");
		m_pfnPtrEnumNetworkAdapters = (FNEnumNetworkAdapters)GetProcAddress(m_hDLLhandle, "EnumNetworkAdapters");
		m_pfnPtrGetDefaultGatewayEx = (FNGetDefaultGatewayEx)GetProcAddress(m_hDLLhandle, "GetDefaultGatewayEx");
		m_fnptrCreatePacketListenerEx = (FNPTRCreatePacketListenerEx)GetProcAddress(m_hDLLhandle, "CreatePacketListenerEx");
		m_fnptrStartPacketListenerEx = (FNPTRStartPacketListenerEx)GetProcAddress(m_hDLLhandle, "StartPacketListenerEx");
		m_fnptrStopPacketListenerEx = (FNPTRStopPacketListenerEx)GetProcAddress(m_hDLLhandle, "StopPacketListenerEx");
		m_fnptrWaitPacketListenerEx = (FNPTRWaitPacketListenerEx)GetProcAddress(m_hDLLhandle, "WaitPacketListenerEx");
		m_fnptrDeletePacketListenerEx = (FNPTRDeletePacketListenerEx)GetProcAddress(m_hDLLhandle, "DeletePacketListenerEx");
		m_fnptrCreateLocalAreaListenerEx = (FNPTRCreateLocalAreaListenerEx)GetProcAddress(m_hDLLhandle, "CreateLocalAreaListenerEx");
		m_fnptrStartLocalAreaListenerEx = (FNPTRStartLocalAreaListenerEx)GetProcAddress(m_hDLLhandle, "StartLocalAreaListenerEx");
		m_fnptrStopLocalAreaListenerEx = (FNPTRStopLocalAreaListenerEx)GetProcAddress(m_hDLLhandle, "StopLocalAreaListenerEx");
		m_fnptrDeleteLocalAreaListenerEx = (FNPTRDeleteLocalAreaListenerEx)GetProcAddress(m_hDLLhandle, "DeleteLocalAreaListenerEx");
		m_fnptrSetNICAdapterToUse = (FNPTRSetNICAdapterToUse)GetProcAddress(m_hDLLhandle, "SetNICAdapterToUse");
		return true;
	}
	CString csMsg;
	csMsg.Format(IDS_ENZDLL);
	AfxMessageBox(csMsg, MB_ICONERROR);
	OnOK();
	return false;
}
bool CCheckOpenPortsDlg::InitGUIControls()
{
	LPCTSTR lpcRecHeader[] = { _T("No."), _T("IP Address"), _T("Device Name"), _T("MAC Address"), _T("Download Speed"), _T("Upload Speed"),  _T("Max Download Speed"), _T("Max Upload Speed") };
	DWORD value = 0;
	DWORD BufferSize = 4;

	::SetWindowTheme(GetDlgItem(IDC_STATIC_ROUTER_INFO)->GetSafeHwnd(), _T(""), _T(""));//To change text Color of Group Box
	::SetWindowTheme(GetDlgItem(IDC_STATIC_ADAPTER_INFO)->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_STATIC_OPEN_PORTS)->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_CHECK_DEBUG)->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_CHECK_INTERNET_ONLY)->GetSafeHwnd(), _T(""), _T(""));
	::SetWindowTheme(GetDlgItem(IDC_STATIC_NET_MON)->GetSafeHwnd(), _T(""), _T(""));

	m_ctrlLANConnected.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_ctrlLANConnected.InsertColumn(COL_NUMBER, lpcRecHeader[COL_NUMBER], LVCFMT_FIXED_WIDTH, 30);
	m_ctrlLANConnected.InsertColumn(COL_IPADDRESS, lpcRecHeader[COL_IPADDRESS], LVCFMT_LEFT, 100);
	m_ctrlLANConnected.InsertColumn(COL_DEVICENAME, lpcRecHeader[COL_DEVICENAME], LVCFMT_LEFT, 110);
	m_ctrlLANConnected.InsertColumn(COL_MACADDRESS, lpcRecHeader[COL_MACADDRESS], LVCFMT_LEFT, 0);
	m_ctrlLANConnected.InsertColumn(COL_DOWNLOADSPEED, lpcRecHeader[COL_DOWNLOADSPEED], LVCFMT_LEFT, 100);
	m_ctrlLANConnected.InsertColumn(COL_UPLOADSPEED, lpcRecHeader[COL_UPLOADSPEED], LVCFMT_LEFT, 100);
	m_ctrlLANConnected.InsertColumn(COL_DOWNLOADMAXSPEED, lpcRecHeader[COL_DOWNLOADMAXSPEED], LVCFMT_LEFT, 120);
	m_ctrlLANConnected.InsertColumn(COL_UPLOADMAXSPEED, lpcRecHeader[COL_UPLOADMAXSPEED], LVCFMT_LEFT, 120);
	m_ctrlBtnStopSearchingPort.EnableWindow(false);
	m_ctrlProgressStatus.ShowWindow(FALSE);
	m_ctrlPortNum.SetWindowText(_T("80"));
	m_ctrlBtnStopListening.EnableWindow(FALSE);
	m_ctrlBtnListenPackets.EnableWindow(FALSE);
	m_ctrlBtnUnlistenPackets.EnableWindow(TRUE);

	RegGetValue(HKEY_LOCAL_MACHINE, REGISTRY_PATH, _T("DebugLog"), /*RRF_RT_ANY*/RRF_RT_DWORD, NULL, (PVOID)&value, &BufferSize);
	if (value)
		m_ctrlBtnDebug.SetCheck(BST_CHECKED);
	else
		m_ctrlBtnDebug.SetCheck(BST_UNCHECKED);
	return true;
}
bool CCheckOpenPortsDlg::InitNIC()
{
	char szDefaultGateWay[32];
	m_bInitNIC = true;
	m_pfnPtrEnumNetworkAdapters(CallBackEnumAdapters);
	if (m_vAdapterInfo.empty())
	{
		CString csMsg;
		csMsg.Format(IDS_NIC_DISCONNECTED);
		AfxMessageBox(csMsg, MB_ICONERROR);
		OnOK();
		return false;
	}
	m_bInitNIC = false;
	InitAdapterUI();

	m_nCurrentNICSelect = m_ctrlComboAdapterList.GetCurSel();
	m_fnptrSetNICAdapterToUse(m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.AdapterName, m_vAdapterInfo[m_ctrlComboAdapterList.GetCurSel()].AdapterInfo.IpAddressList.Context);
	if (m_pfnPtrGetDefaultGatewayEx(m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.AdapterName, szDefaultGateWay, sizeof(szDefaultGateWay)))
	{
		m_ipFilter = m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.IpAddressList.IpAddress.String;
		inet_pton(AF_INET, CW2A(m_ipFilter), &m_ulIPFilter);
	}
	else
	{
		CString csMsg;

		csMsg.Format(IDS_NIC_DISCONNECTED);
		AfxMessageBox(csMsg, MB_ICONERROR);
		OnOK();
		return false;
	}

	m_ctrlIPAddress.SetWindowText(CA2W(szDefaultGateWay));
	m_ctrlStaticNICListen.SetWindowText(CA2W(m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.Description));
	m_ctrlLANConnected.SetAdapterIP(CA2W(m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.IpAddressList.IpAddress.String).m_szBuffer);
	
	for (int i = 0; i < m_vAdapterInfo.size(); i++)
		m_vAdapterInfo[i].hLANListener = m_fnptrCreateLocalAreaListenerEx();
	return true;
}
BOOL CCheckOpenPortsDlg::OnInitDialog()
{
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
	// Set the icon for this dialog.  The framework does this automatically
//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	InitGUIControls();
	InitDLL();
	InitNIC();

	m_hNICPacketListener = m_fnptrCreatePacketListenerEx(CallbackNICPacketListener, NULL);
	m_hThreadRouter = (HANDLE)_beginthreadex(NULL, 0, RouterThread, this, 0, NULL);
	m_hThreadNICListener = (HANDLE)_beginthreadex(NULL, 0, NICListenerThread, this, 0, NULL);
	m_hThreadClock = (HANDLE)_beginthreadex(NULL, 0, ClockThread, this, 0, NULL);
	OnBnClickedButtonStartListenLan();
	OnBnClickedButtonShowPackets();

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

			if (id == IDC_EDIT_AREA || id == IDC_EDIT_PACKET_REPORT || id == IDC_EDIT_SPEED_DOWN || id == IDC_EDIT_SPEED_UP || id == IDC_EDIT_ADAPTER_INFO)
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
		case CTLCOLOR_BTN:
		{
			int id = pWnd->GetDlgCtrlID();
			if (id == IDC_CHECK_DEBUG)
			{
				pDC->SetTextColor(RGB(255, 255, 255));
				pDC->SetBkColor(RGB(93, 107, 153));
				pDC->SetBkMode(TRANSPARENT);
				return m_hBrushBackGround;
			}
			break;
		}
		case CTLCOLOR_DLG:
		{
			pDC->SetBkColor(RGB(64, 86, 141));
			return m_hBrushBackGround;
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetBkColor(RGB(255, 255, 255));
			return m_hBrushEditArea;
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
		if (m_pmodeless)
			m_pmodeless->SetFocus();
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCheckOpenPortsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCheckOpenPortsDlg::Increment()
{
	m_nThread++;
	m_ctrlProgressStatus.SetPos(m_nThread);
}

void CCheckOpenPortsDlg::OnBnClickedButtonStartSearchingOpenPort()
{
	m_bStopSearchingOpenPorts = false;
	m_ctrlBtnStopSearchingPort.EnableWindow(true);
	m_ctrlIPAddress.EnableWindow(FALSE);
	m_ctrlBtnCheckOpenPorts.EnableWindow(FALSE);
	m_ctrlResult.SetWindowText(_T(""));
	m_ctrlProgressStatus.ShowWindow(TRUE);
	m_ctrlProgressStatus.SetRange32(1, MAX_PORT);
	m_nThread = 0;


	if (m_hThreadOpenPortListener)
	{
		WaitForSingleObject(m_hThreadOpenPortListener, INFINITE);
		CloseHandle(m_hThreadOpenPortListener);
		m_hThreadOpenPortListener = NULL;
	}
	m_hThreadOpenPortListener = (HANDLE)_beginthreadex(NULL, 0, OpenPortListenerThread, this, 0, NULL);

	
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
	if (m_pfnPtrIsPortOpen(CW2A(cs), _ttoi(csPort), &nLastError))
	{
#else
	if (m_pfnPtrIsPortOpen(cs.GetBuffer(), _ttoi(csPort), &nLastError))
	{
#endif
		CString csRes;
		csRes = +_T("Port (") + csPort + _T(") Of (") + cs + _T(") is open.\r\n");
	
		g_pCCheckOpenPortsDlg->m_ctrlResult.SetSel(0, 0);
		g_pCCheckOpenPortsDlg->m_ctrlResult.ReplaceSel(csRes);
	}
	else
	{
		CString csRes;
		csRes = +_T("Port (") + csPort + _T(") Of (") + cs + _T(") is closed.\r\n");
		g_pCCheckOpenPortsDlg->m_ctrlResult.SetSel(0, 0);
		g_pCCheckOpenPortsDlg->m_ctrlResult.ReplaceSel(csRes);
	}

}

void CCheckOpenPortsDlg::OnClose()
{
	OnBnClickedButtonStopListenLan();
	OnBnClickedButtonStopSearchingOpenPorts();
	OnBnClickedButtonStopPacket();
	
	if (m_pmodeless)
	{
		if(m_hNICPacketListener)
			m_fnptrStopPacketListenerEx(m_hNICPacketListener);
		m_pmodeless->OnBnClickedCancel();
	}
	if (IsLANStopped() && IsSearchingOpenPortStopped())
	{
		EnableCloseButton(false);
		m_bHasClickClose = TRUE;

		//Tjhrea for checking if additional NIC is added
		if (m_hThreadNICListener)
		{
			WaitForSingleObject(m_hThreadNICListener, INFINITE);
			CloseHandle(m_hThreadNICListener);
		}

		//Thread for listening LAN devices connected
		if (m_hThreadLANListener)
		{
			WaitForSingleObject(m_hThreadLANListener, INFINITE);
			CloseHandle(m_hThreadLANListener);
		}
		//Thread for checking open ports
		if (m_hThreadOpenPortListener)
		{
			WaitForSingleObject(m_hThreadOpenPortListener, INFINITE);
			CloseHandle(m_hThreadOpenPortListener);
		}

		if (m_hNICPacketListener)
		{
			m_fnptrDeletePacketListenerEx(m_hNICPacketListener);
		}
		map<ULONG, CDeviceConnected*>::iterator it = m_mConnected.begin();

		for (int i = 0; i < m_vAdapterInfo.size(); i++)
		{
			if (m_vAdapterInfo[i].hLANListener)
			{
				m_fnptrDeleteLocalAreaListenerEx(m_vAdapterInfo[i].hLANListener);
			}
		}
		m_vAdapterInfo.clear();
		while (it != m_mConnected.end())
		{
			if (it->second->m_hPacketListenerDownload)
			{
				m_fnptrDeletePacketListenerEx(it->second->m_hPacketListenerDownload);
			}
			if (it->second->m_hPacketListenerUpload)
			{
				m_fnptrDeletePacketListenerEx(it->second->m_hPacketListenerUpload);
			}

			delete it->second;
			it++;
		}
		m_mConnected.clear();

		//Listening of Main Router Information
		if (m_hThreadRouter)
		{
			WaitForSingleObject(m_hThreadRouter, INFINITE);
			CloseHandle(m_hThreadRouter);
			m_pfnPtrEndSNMP();
		}
		//Clock Thread
		if (m_hThreadClock)
		{
			WaitForSingleObject(m_hThreadClock, INFINITE);
			CloseHandle(m_hThreadClock);
		}


		if(m_hDLLhandle)
			FreeLibrary(m_hDLLhandle);
	
		
		OnOK();
	}
	else
	{
		//m_bStopLANClicked = TRUE;
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

void CCheckOpenPortsDlg::OnBnClickedButtonStartListenLan()
{
	m_ctrlBtnListen.EnableWindow(FALSE);
	m_ctrlBtnStopListening.EnableWindow(TRUE);
	SetLANStop(false);
	
	CString csText;
	CString csPollTime;
	
	string ip = m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.IpAddressList.IpAddress.String;
	string mask = m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.IpAddressList.IpMask.String;

	m_fnptrStartLocalAreaListenerEx(m_vAdapterInfo[m_nCurrentNICSelect].hLANListener, ip.c_str(), mask.c_str(), CallbackLANListenerEx, POLLING_TIME);
}

void CCheckOpenPortsDlg::OnBnClickedButtonStopListenLan()
{
	// TODO: Add your control notification handler code here
	//m_pfnPtrStopLocalAreaListening();
	m_ctrlBtnStopListening.EnableWindow(FALSE);


	for(int i = 0; i < m_vAdapterInfo.size();i++)
		m_fnptrStopLocalAreaListenerEx(m_vAdapterInfo[i].hLANListener);

}

void CCheckOpenPortsDlg::OnNMClickListLan(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	m_nCurrentRowSelected = pNMItemActivate->iItem;
	
	
	if (pNMItemActivate->iItem > -1)
	{
		//m_ipFilter = ;
		m_ctrlIPAddress.SetWindowText(m_ctrlLANConnected.GetItemText(m_nCurrentRowSelected, 1));
		//wstring temp = m_ipFilter.GetBuffer();
 		//inet_pton(AF_INET, UnicodeToMultiByte(temp).c_str(), &m_ulIPFilter);
	}
}

void CCheckOpenPortsDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	m_ctrlLANConnected.SetItemState(m_nCurrentRowSelected, LVIS_SELECTED, LVIS_SELECTED);
	m_ctrlLANConnected.SetFocus();
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
	
	if (pLVKeyDow->wVKey == VK_UP)
	{
		m_nCurrentRowSelected = m_ctrlLANConnected.GetSelectionMark()-1;
		if (m_nCurrentRowSelected >= 0)
		{
			m_ctrlLANConnected.SetItemState(m_nCurrentRowSelected, LVIS_SELECTED, LVIS_SELECTED);
			m_ctrlLANConnected.SetFocus();
			CString cs = m_ctrlLANConnected.GetItemText(m_nCurrentRowSelected, 1);
			m_ctrlIPAddress.SetWindowText(cs);
		}
	}
	else if (pLVKeyDow->wVKey == VK_DOWN)
	{
		m_nCurrentRowSelected = m_ctrlLANConnected.GetSelectionMark()+1;
		if (m_nCurrentRowSelected < m_ctrlLANConnected.GetItemCount())
		{
			m_ctrlLANConnected.SetItemState(m_nCurrentRowSelected, LVIS_SELECTED, LVIS_SELECTED);
			m_ctrlLANConnected.SetFocus();
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

		OnBnClickedButtonStopPacket();

		CSaveDeviceInfoDlg saveDlg;
		CString csIP = m_ctrlLANConnected.GetItemText(pNMItemActivate->iItem, COL_IPADDRESS);
		ULONG ulIP;
		
		inet_pton(AF_INET, CW2A(csIP.GetBuffer()), &ulIP);
	
		
		saveDlg.m_fnptrCreatePacketListenerEx = m_fnptrCreatePacketListenerEx;
		saveDlg.m_fnptrStartPacketListenerEx = m_fnptrStartPacketListenerEx;
		saveDlg.m_fnptrStopPacketListenerEx = m_fnptrStopPacketListenerEx;
		saveDlg.m_fnptrDeletePacketListenerEx = m_fnptrDeletePacketListenerEx;

		saveDlg.SetInformation(csIP,
			m_ctrlLANConnected.GetItemText(pNMItemActivate->iItem, COL_MACADDRESS),
			m_ctrlLANConnected.GetItemText(pNMItemActivate->iItem, COL_DEVICENAME));

		INT_PTR nPtr = saveDlg.DoModal();
		if (nPtr == IDOK)
		{
			if (m_mConnected.find(ulIP) != m_mConnected.end())
			{
				m_mConnected[ulIP]->m_szHostName = saveDlg.GetDeviceName();
				m_ctrlLANConnected.SetItemText(pNMItemActivate->iItem, COL_DEVICENAME, saveDlg.GetDeviceName());
			}
		}
		OnBnClickedButtonStartPacket();
	}
}

// CCheckOpenPortsDlg message handlers
unsigned __stdcall  CCheckOpenPortsDlg::RouterThread(void* parg)
{
	DEBUG_LOG(_T("CCheckOpenPortsDlg::RouterThread() has started."));
	CCheckOpenPortsDlg* pDlg = (CCheckOpenPortsDlg*)parg;
	char szDefaultGateway[32];
	memset(szDefaultGateway, 0, sizeof(szDefaultGateway));
	CString csBrand = _T("");
	CString csModel = _T("");
	CString csDesc = _T("");
	char* sTemp = NULL;

	DWORD error = 0;
	smiVALUE value;

	if (pDlg->m_vAdapterInfo.empty())
		return 0;
	if (pDlg->m_pfnPtrGetDefaultGatewayEx(pDlg->m_vAdapterInfo[pDlg->m_ctrlComboAdapterList.GetCurSel()].AdapterInfo.AdapterName, szDefaultGateway, sizeof(szDefaultGateway)))
	{
		if (pDlg->m_pfnPtrStartSNMP(szDefaultGateway, "public", 1, error))
		{

			value = pDlg->m_pfnPtrSNMPGet(".1.3.6.1.2.1.1.6.0", error);//Brand name
			if (error != SNMPAPI_SUCCESS)
			{
				return 0;
			}
	#ifdef UNICODE
			sTemp = (char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, sizeof(char) * (value.value.sNumber + 1));
			if (sTemp)
			{
				memset(sTemp, 0, sizeof(char) * (value.value.sNumber + 1));
				memcpy_s(sTemp, sizeof(char) * (value.value.sNumber), value.value.string.ptr, value.value.sNumber);
				csBrand = CA2W(sTemp);
				HeapFree(GetProcessHeap(), 0, sTemp);
				sTemp = NULL;
			}
	#else		
			csBrand = value.value.string.ptr;
	#endif

			value = pDlg->m_pfnPtrSNMPGet(".1.3.6.1.2.1.1.5.0", error);//Model name
			if (error != SNMPAPI_SUCCESS)
			{
				return 0;
			}
	#ifdef UNICODE
			sTemp = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * (value.value.sNumber + 1));
			if (sTemp)
			{
				memset(sTemp, 0, sizeof(char) * (value.value.sNumber + 1));
				memcpy_s(sTemp, sizeof(char) * (value.value.sNumber), value.value.string.ptr, value.value.sNumber);
				csModel = CA2W(sTemp);
				HeapFree(GetProcessHeap(), 0, sTemp);
				sTemp = NULL;
			}
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

			sTemp = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * (value.value.sNumber + 1));
			if (sTemp)
			{
				memset(sTemp, 0, sizeof(char) * (value.value.sNumber + 1));
				memcpy_s(sTemp, sizeof(char) * (value.value.sNumber), value.value.string.ptr, value.value.sNumber);
				csDesc = CA2W(sTemp);
				HeapFree(GetProcessHeap(), 0, sTemp);
				sTemp = NULL;
			}
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
			double fRem = remainder(value.value.uNumber / (float)8640000, (float)8640000) - ulDays;
			ULONG ulHour = fRem * 24;
			fRem = (double)(fRem * 24) - ulHour;
			ULONG ulMin = fRem * 60;
			fRem = (double)(fRem * 60) - ulMin;
			ULONG ulSec = fRem * 60;

			csFormat = _T("");
			csFormat.Format(_T("%s %s %s\r\n\r\nRouter's Up Time\r\n%u days, %u hours, %u min, %u secs"), csBrand.GetBuffer(), csModel.GetBuffer(), csDesc.GetBuffer(), ulDays, ulHour, ulMin, ulSec);

			pDlg->SetRouterUpTime(csFormat);
			Sleep(POLLING_TIME);
		}
	}
	DEBUG_LOG(_T("CCheckOpenPortsDlg::RouterThread() has ended."));
	return 0;
}
unsigned __stdcall  CCheckOpenPortsDlg::NICListenerThread(void* parg)
{
	DEBUG_LOG(_T("CCheckOpenPortsDlg::NICListenerThread() has started."));
	CCheckOpenPortsDlg* pDlg = (CCheckOpenPortsDlg*)parg;

	while (!pDlg->HasClickClose())
	{
		pDlg->m_pfnPtrEnumNetworkAdapters(CallBackEnumAdapters);
		Sleep(500);
	}
	DEBUG_LOG(_T("CCheckOpenPortsDlg::NICListenerThread() has ended."));
	return 0;
}

int CCheckOpenPortsDlg::IsInTheList(CString csIPAddress)
{
	int index = -1;
	bool bRet = false;

	
	for (int i = 0; i < m_ctrlLANConnected.GetItemCount(); ++i)
	{
		CString szText = g_pCCheckOpenPortsDlg->m_ctrlLANConnected.GetItemText(i, 1);
		if (szText == csIPAddress)
		{
			index = i;
			break;
		}
	}

	return index;
}
void CCheckOpenPortsDlg::UpdateDeviceConnected()
{
	CString csConn;
	csConn.Format(_T("Device Connected: %d"), m_ctrlLANConnected.GetItemCount());
	m_ctrlStaticNumDevice.SetWindowTextW(csConn);
}

void CCheckOpenPortsDlg::ProcessLANListener(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsopen)
{
	if (strcmp(ipAddress, "start") == 0)
	{
		m_ctrlBtnListen.EnableWindow(FALSE);
		m_ctrlBtnStopListening.EnableWindow(TRUE);
		SetLANStop(false);
		//EnableCloseButton(false);
	}
	if (bIsopen)
	{
		int col = 0;
		CString format;
		DWORD dwError = 0;
		string ipAdd;
		ULONG ipaddr;
		int nRow = 0;

		inet_pton(AF_INET, ipAddress, &ipaddr);
		m_mMonitorDeviceCurrent[ipaddr]++;
		if ((m_mConnected.find(ipaddr) == m_mConnected.end()) && strlen(ipAddress))
		{
			CString csTemp, format;
			CDeviceConnected* tDeviceDetails = new CDeviceConnected();

			tDeviceDetails->m_szIPAddress.Format(_T("%s"), CA2W(ipAddress).m_szBuffer);
			tDeviceDetails->m_szMACAddress.Format(_T("%s"), CA2W(macAddress).m_szBuffer);
			tDeviceDetails->m_szHostName.Format(_T("%s"), CA2W(hostName).m_szBuffer);
			tDeviceDetails->m_ulDataSizeDownload = 0;
			tDeviceDetails->m_ulDataSizeUpload = 0;
			tDeviceDetails->m_lfDownloadSpeed = 0;
			tDeviceDetails->m_lfUploadSpeed = 0;
			tDeviceDetails->m_lfMaxDownloadSpeed = 0;
			tDeviceDetails->m_lfMaxUploadSpeed = 0;
			tDeviceDetails->m_ullDownloadStartTime = tDeviceDetails->m_ullUploadStartTime = GetTickCount64();
			tDeviceDetails->m_hPacketListenerDownload = m_fnptrCreatePacketListenerEx(CallbackPacketListenerDownloadEx, (void*)tDeviceDetails);
			tDeviceDetails->m_hPacketListenerUpload = m_fnptrCreatePacketListenerEx(CallbackPacketListenerUploadEx, (void*)tDeviceDetails);

			if (!m_bStopPacketListener)
			{
				m_fnptrStartPacketListenerEx(tDeviceDetails->m_hPacketListenerDownload);
				m_fnptrStartPacketListenerEx(tDeviceDetails->m_hPacketListenerUpload);
				m_ctrlBtnListenPackets.EnableWindow(FALSE);
				m_ctrlBtnUnlistenPackets.EnableWindow(TRUE);
			}

			if (tDeviceDetails->m_szIPAddress == tDeviceDetails->m_szHostName)
			{
				TCHAR value[MAX_PATH];
				DWORD BufferSize = sizeof(value);
				DWORD dwRet = RegGetValue(HKEY_LOCAL_MACHINE, REGISTRY_PATH, tDeviceDetails->m_szMACAddress, /*RRF_RT_ANY*/RRF_RT_REG_SZ, NULL, (PVOID)&value, &BufferSize);
				if (dwRet == ERROR_SUCCESS)
					tDeviceDetails->m_szHostName = value;
			}
			m_mConnected[ipaddr] = tDeviceDetails;


			nRow = m_ctrlLANConnected.GetItemCount();

			if (nRow == 0)
			{
				m_ctrlLANConnected.InsertItem(LVIF_TEXT | LVIF_STATE, nRow,
					to_wstring(nRow + 1).c_str(), 0, 0, 0, 0);
				m_ctrlLANConnected.SetItemText(nRow, col + 1, tDeviceDetails->m_szIPAddress);
				m_ctrlLANConnected.SetItemText(nRow, col + 2, tDeviceDetails->m_szHostName);
				m_ctrlLANConnected.SetItemText(nRow, col + 3, tDeviceDetails->m_szMACAddress);
				if (tDeviceDetails->m_lfDownloadSpeed <= 1000)
					format.Format(_T("%.2f Kbps"), tDeviceDetails->m_lfDownloadSpeed);
				else
					format.Format(_T("%.2f Mbps"), tDeviceDetails->m_lfDownloadSpeed / 1000);
				m_ctrlLANConnected.SetItemText(nRow, col + 4, format);
				if (tDeviceDetails->m_lfUploadSpeed <= 1000)
					format.Format(_T("%.2f Kbps"), tDeviceDetails->m_lfUploadSpeed);
				else
					format.Format(_T("%.2f Mbps"), tDeviceDetails->m_lfUploadSpeed / 1000);
				m_ctrlLANConnected.SetItemText(nRow, col + 5, format);
				if (tDeviceDetails->m_lfMaxDownloadSpeed <= 1000)
					format.Format(_T("%.2f Kbps"), tDeviceDetails->m_lfMaxDownloadSpeed);
				else
					format.Format(_T("%.2f Mbps"), tDeviceDetails->m_lfMaxDownloadSpeed / 1000);
				m_ctrlLANConnected.SetItemText(nRow, col + 6, format);
				if (tDeviceDetails->m_lfMaxUploadSpeed <= 1000)
					format.Format(_T("%.2f Kbps"), tDeviceDetails->m_lfMaxUploadSpeed);
				else
					format.Format(_T("%.2f Mbps"), tDeviceDetails->m_lfMaxUploadSpeed / 1000);
				m_ctrlLANConnected.SetItemText(nRow, col + 7, format);
			}
			else
			{
				for (int i = 0; i < m_ctrlLANConnected.GetItemCount(); i++)
				{
					ULONG ulItemBefore = 0, UlIP = 0, ulItemAfter = 0;
					InetPtonW(AF_INET, m_ctrlLANConnected.GetItemText(i, 1), &ulItemBefore);
					InetPtonW(AF_INET, m_ctrlLANConnected.GetItemText(i + 1, 1), &ulItemAfter);
					InetPtonW(AF_INET, tDeviceDetails->m_szIPAddress, &UlIP);
					if ((htonl(ulItemBefore) < htonl(UlIP)) && (htonl(UlIP) < htonl(ulItemAfter)))
					{
						nRow = i + 1;
						break;
					}
					else if (htonl(ulItemBefore) < htonl(UlIP) && i == m_ctrlLANConnected.GetItemCount() - 1)
					{
						nRow = i + 1;
						break;
					}
					else if ((htonl(ulItemBefore) > htonl(UlIP)) && (htonl(UlIP) < htonl(ulItemAfter)))
					{
						nRow = i;
						break;
					}
					else if (htonl(ulItemBefore) > htonl(UlIP) && i == m_ctrlLANConnected.GetItemCount() - 1)
					{
						nRow = i;
						break;
					}
				}

				m_ctrlLANConnected.InsertItem(LVIF_TEXT | LVIF_STATE, nRow,
					to_wstring(nRow + 1).c_str(), 0, 0, 0, 0);
				m_ctrlLANConnected.SetItemText(nRow, col + 1, tDeviceDetails->m_szIPAddress);
				m_ctrlLANConnected.SetItemText(nRow, col + 2, tDeviceDetails->m_szHostName);
				m_ctrlLANConnected.SetItemText(nRow, col + 3, tDeviceDetails->m_szMACAddress);
				if (tDeviceDetails->m_lfDownloadSpeed <= 1000)
					format.Format(_T("%.2f Kbps"), tDeviceDetails->m_lfDownloadSpeed);
				else
					format.Format(_T("%.2f Mbps"), tDeviceDetails->m_lfDownloadSpeed / 1000);
				m_ctrlLANConnected.SetItemText(nRow, col + 4, format);
				if (tDeviceDetails->m_lfUploadSpeed <= 1000)
					format.Format(_T("%.2f Kbps"), tDeviceDetails->m_lfUploadSpeed);
				else
					format.Format(_T("%.2f Mbps"), tDeviceDetails->m_lfUploadSpeed / 1000);
				m_ctrlLANConnected.SetItemText(nRow, col + 5, format);
				if (tDeviceDetails->m_lfMaxDownloadSpeed <= 1000)
					format.Format(_T("%.2f Kbps"), tDeviceDetails->m_lfMaxDownloadSpeed);
				else
					format.Format(_T("%.2f Mbps"), tDeviceDetails->m_lfMaxDownloadSpeed / 1000);
				m_ctrlLANConnected.SetItemText(nRow, col + 6, format);
				if (tDeviceDetails->m_lfMaxUploadSpeed <= 1000)
					format.Format(_T("%.2f Kbps"), tDeviceDetails->m_lfMaxUploadSpeed);
				else
					format.Format(_T("%.2f Mbps"), tDeviceDetails->m_lfMaxUploadSpeed / 1000);
				m_ctrlLANConnected.SetItemText(nRow, col + 7, format);


				for (int j = nRow; j < g_pCCheckOpenPortsDlg->m_ctrlLANConnected.GetItemCount(); j++)
				{
					m_ctrlLANConnected.SetItemText(j, 0, to_wstring(j + 1).c_str());
				}
			}
			UpdateDeviceConnected();
		}

	}
	else
	{
		if (strcmp(ipAddress, "end") == 0)
		{
			if (!m_mMonitorDeviceBefore.empty())
			{
				map<ULONG, int>::iterator it = m_mMonitorDeviceCurrent.begin();
				while (it != m_mMonitorDeviceCurrent.end())
				{
					if (m_mMonitorDeviceBefore.find(it->first) != m_mMonitorDeviceBefore.end())
						m_mMonitorDeviceBefore.erase(it->first);
					it++;
				}
				it = m_mMonitorDeviceBefore.begin();
				while (it != m_mMonitorDeviceBefore.end())
				{
					char szIP[32];
					inet_ntop(AF_INET, &(it->first), szIP, sizeof(szIP));
					CString cs(szIP);
					int nRow = IsInTheList(cs);
					if (nRow != -1)
					{
						m_ctrlLANConnected.DeleteItem(nRow);
						for (int j = nRow; j < m_ctrlLANConnected.GetItemCount(); j++)
						{
							m_ctrlLANConnected.SetItemText(j, 0, to_wstring(j + 1).c_str());
						}

						m_fnptrDeletePacketListenerEx(m_mConnected[it->first]->m_hPacketListenerDownload);
						m_fnptrDeletePacketListenerEx(m_mConnected[it->first]->m_hPacketListenerUpload);

						CDeviceConnected* pDevice;
						pDevice = m_mConnected[it->first];
						m_mConnected[it->first] = NULL;
						delete pDevice;
						pDevice = NULL;
						m_mConnected.erase(it->first);
					}
					it++;
				}
			}

			m_mMonitorDeviceBefore = m_mMonitorDeviceCurrent;
			m_mMonitorDeviceCurrent.clear();

			CString csIP(CA2W(m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.IpAddressList.IpAddress.String));
			m_ctrlLANConnected.SetAdapterIP(csIP);
			m_fnptrSetNICAdapterToUse(m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.AdapterName, m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.IpAddressList.Context);
			m_ctrlStaticNICListen.SetWindowText(CA2W(m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.Description));
			m_ctrlComboAdapterList.EnableWindow(TRUE);
			UpdateDeviceConnected();
		}
		else if (strcmp(ipAddress, "stop") == 0)
		{
			m_ctrlBtnListen.EnableWindow(TRUE);
			m_ctrlBtnStopListening.EnableWindow(FALSE);
			SetLANStop(true);
			//EnableCloseButton(true);
		}
	}
}
void CCheckOpenPortsDlg::CallbackLANListenerEx(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsopen)
{
	mtx_lanlistener.lock();
	g_pCCheckOpenPortsDlg->ProcessLANListener(ipAddress, hostName, macAddress, bIsopen);
	mtx_lanlistener.unlock();
}

void CCheckOpenPortsDlg::CallBackEnumPort(char* ipAddress, int nPort, bool bIsopen, int nLastError)
{
	if (ipAddress != NULL)
	{
		if(strcmp(ipAddress,"DONE") == 0)
		{
			g_pCCheckOpenPortsDlg->m_ctrlIPAddress.EnableWindow(TRUE);
			g_pCCheckOpenPortsDlg->m_ctrlBtnCheckOpenPorts.EnableWindow(TRUE);
			g_pCCheckOpenPortsDlg->m_ctrlProgressStatus.ShowWindow(FALSE);
			g_pCCheckOpenPortsDlg->m_ctrlBtnStopSearchingPort.EnableWindow(false);
			g_pCCheckOpenPortsDlg->SetStopSearchingOpenPort();
		}
		else
		{
			mtx_enumPorts.lock();
			if (bIsopen)
			{
				CString csStr;

				if (bIsopen)
					csStr.Format(_T("%s %d is open.\r\n"), CA2W(ipAddress).m_szBuffer, nPort);

				long nLength = g_pCCheckOpenPortsDlg->m_ctrlResult.GetWindowTextLength();
				g_pCCheckOpenPortsDlg->m_ctrlResult.SetSel(0, 0);
				g_pCCheckOpenPortsDlg->m_ctrlResult.ReplaceSel(csStr);
			}
			g_pCCheckOpenPortsDlg->Increment();
			mtx_enumPorts.unlock();
		}
	}
}
void CCheckOpenPortsDlg::CallBackEnumAdapters(void* args)
{
	PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)args;
	bool bFound = false;
	int nIndexFound = -1;
	for (int i = 0; i < g_pCCheckOpenPortsDlg->m_vAdapterInfo.size(); i++)
	{
		if (strcmp(g_pCCheckOpenPortsDlg->m_vAdapterInfo[i].AdapterInfo.AdapterName, pAdapterInfo->AdapterName) == 0)
		{
			bFound = true;
			nIndexFound = i;
			break;
		}
	}
	if (!bFound)
	{
		if (pAdapterInfo->IpAddressList.Context)
		{
			CNetworkInterfaceInfo nicInfo;
			nicInfo.AdapterInfo = *pAdapterInfo;
			nicInfo.hLANListener = g_pCCheckOpenPortsDlg->m_fnptrCreateLocalAreaListenerEx();
			g_pCCheckOpenPortsDlg->m_vAdapterInfo.push_back(nicInfo);
			g_pCCheckOpenPortsDlg->m_ctrlComboAdapterList.AddString(CA2W(pAdapterInfo->Description));
			g_pCCheckOpenPortsDlg->UpdateAdapterChanges();
		}
		if (g_pCCheckOpenPortsDlg->m_vAdapterInfo.empty())
		{
			g_pCCheckOpenPortsDlg->UpdateAdapterChanges();
		}
	}
	else
	{
		if (!pAdapterInfo->IpAddressList.Context)
		{
			g_pCCheckOpenPortsDlg->m_fnptrStopLocalAreaListenerEx(g_pCCheckOpenPortsDlg->m_vAdapterInfo[nIndexFound].hLANListener);
			g_pCCheckOpenPortsDlg->m_fnptrDeleteLocalAreaListenerEx(g_pCCheckOpenPortsDlg->m_vAdapterInfo[nIndexFound].hLANListener);
			g_pCCheckOpenPortsDlg->m_vAdapterInfo.erase(g_pCCheckOpenPortsDlg->m_vAdapterInfo.begin() + nIndexFound);
			g_pCCheckOpenPortsDlg->m_ctrlComboAdapterList.DeleteString(nIndexFound);
			g_pCCheckOpenPortsDlg->UpdateAdapterChanges();
		}
	}
}
void CCheckOpenPortsDlg::UpdateAdapterChanges()
{
	CStringA csWindowText;
	char szDefaultGateWay[32];
	m_ctrlComboAdapterList.SetCurSel(0);
	int i = m_ctrlComboAdapterList.GetCurSel();
	
	memset(szDefaultGateWay, 0, sizeof(szDefaultGateWay));
	if (!m_vAdapterInfo.empty())
	{
		if (m_pfnPtrGetDefaultGatewayEx(m_vAdapterInfo[i].AdapterInfo.AdapterName, szDefaultGateWay, sizeof(szDefaultGateWay)))
		{
			m_ipFilter = CA2W(m_vAdapterInfo[i].AdapterInfo.IpAddressList.IpAddress.String);
			inet_pton(AF_INET, m_vAdapterInfo[i].AdapterInfo.IpAddressList.IpAddress.String, &m_ulIPFilter);

			csWindowText.Format("Adapter Name: \t%s\r\n", m_vAdapterInfo[i].AdapterInfo.AdapterName);
			csWindowText.AppendFormat("Adapter Desc: \t%s\r\n", m_vAdapterInfo[i].AdapterInfo.Description);
			//memcpy_s(p, sizeof(p), m_vAdapterInfo[i].AdapterInfo.Address, sizeof(p));
			csWindowText.AppendFormat("Adapter Addr: \t%X:%X:%X:%X:%X:%X\r\n", 
				m_vAdapterInfo[i].AdapterInfo.Address[0], m_vAdapterInfo[i].AdapterInfo.Address[1], m_vAdapterInfo[i].AdapterInfo.Address[2], 
				m_vAdapterInfo[i].AdapterInfo.Address[3], m_vAdapterInfo[i].AdapterInfo.Address[4], m_vAdapterInfo[i].AdapterInfo.Address[5]);
			csWindowText.AppendFormat("IP Addr: \t\t%s\r\n", m_vAdapterInfo[i].AdapterInfo.IpAddressList.IpAddress.String);
			csWindowText.AppendFormat("IP Mask: \t\t%s\r\n", m_vAdapterInfo[i].AdapterInfo.IpAddressList.IpMask.String);
			csWindowText.AppendFormat("IP Gateway: \t%s\r\n",  m_vAdapterInfo[i].AdapterInfo.GatewayList.IpAddress.String);
			if (m_vAdapterInfo[i].AdapterInfo.DhcpEnabled)
			{
				csWindowText.AppendFormat("DHCP Enable: \tYes\r\n");
				csWindowText.AppendFormat("Lease Obtained: \t%lld\r\n", m_vAdapterInfo[i].AdapterInfo.LeaseObtained);
			}
			else
				csWindowText.AppendFormat("DHCP Enable: \tNo\r\n");

			if (m_vAdapterInfo[i].AdapterInfo.HaveWins)
			{
				csWindowText.AppendFormat("Have Wins: \tYes\r\n");
				csWindowText.AppendFormat("Primary Wins Server: \t%s\r\n",  m_vAdapterInfo[i].AdapterInfo.PrimaryWinsServer.IpAddress.String);
				csWindowText.AppendFormat("Secondary Wins Server: \t%s\r\n", m_vAdapterInfo[i].AdapterInfo.SecondaryWinsServer.IpAddress.String);
			}
			else
				csWindowText.AppendFormat("Have Wins: \tNo\r\n");

			m_ctrlEditAdapterInfo.SetWindowText(CA2W(csWindowText));

			if (!m_bInitNIC)
			{
				if (m_nCurrentNICSelect != i)
				{
					OnBnClickedButtonStopListenLan();
					m_nCurrentNICSelect = i;
					OnBnClickedButtonStartListenLan();
				}
			}

		}
		
	}
	else
	{
		m_ctrlEditAdapterInfo.SetWindowText(_T(""));
		OnBnClickedButtonStopListenLan();
		OnBnClickedButtonStopSearchingOpenPorts();
		OnBnClickedButtonStopPacket();
		m_nCurrentNICSelect = -1;
		m_bStopPacketListener = false;
		AfxMessageBox(_T("All Network Interfaces are disconnected. Please check your ethernet ports or WIFI adapters."));
	}
}
bool CCheckOpenPortsDlg::CallbackNICPacketListener(unsigned char* buffer, int nSize, void *obj)
{
	return g_pCCheckOpenPortsDlg->ProcessNICPacketListener(buffer, nSize, obj);
}
bool CCheckOpenPortsDlg::ProcessNICPacketListener(unsigned char* buffer, int nSize, void* obj)
{
	CString csText, csSrcPort, csDestPort, sourceIP, destIP, cs, csTemp, ipFilter, csReport;
	int iphdrlen = 0, nDataSize = 0;
	IPV4_HDR* iphdr;
	TCP_HDR* tcpheader = NULL;
	UDP_HDR* udpheader = NULL;
	string sTemp;
	char sztemp[32];

	memset(sztemp, 0, sizeof(sztemp));
	iphdr = (IPV4_HDR*)buffer;
	iphdrlen = iphdr->ucIPHeaderLen * 4;

	if (m_bShowPacketInfo)
		return false;
	if (IsInternetOnly())
	{
		DWORD dwMask = 0;
		DWORD dwBeginIP = 0;
		DWORD dwEndIP = 0;

		inet_pton(AF_INET, m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.IpAddressList.IpMask.String, &dwMask);
		dwMask = ntohl(dwMask);
		inet_pton(AF_INET, m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.IpAddressList.IpAddress.String, &dwBeginIP);
		dwBeginIP = dwMask & ntohl(dwBeginIP);
		dwEndIP = (0xFFFFFFFF - dwMask) + dwBeginIP;

		if (((ntohl(iphdr->unDestaddress) > dwBeginIP) && (ntohl(iphdr->unDestaddress) < dwEndIP)) && ((ntohl(iphdr->unSrcaddress) > dwBeginIP) && (ntohl(iphdr->unSrcaddress) < dwEndIP)))
		{
			return false;
		}
	}


	if (m_pmodeless == NULL)
	{
		return false;
	}

	else
	{
		inet_ntop(AF_INET, (const void*)&iphdr->unDestaddress, sztemp, sizeof(sztemp));
		destIP = sztemp;
		inet_ntop(AF_INET, (const void*)&iphdr->unSrcaddress, sztemp, sizeof(sztemp));
		sourceIP = sztemp;

		switch (iphdr->ucIPProtocol)
		{
			case ICMP_PROTOCOL:
			{
				csText = _T("ICMP:\t");
				break;
			}
			case IGMP_PROTOCOL:
			{
				csText = _T("IGMP:\t");
				break;
			}
			case TCP_PROTOCOL:
			{
				tcpheader = (TCP_HDR*)(buffer + iphdrlen);
				csSrcPort = to_wstring(ntohs(tcpheader->usSourcePort)).c_str();
				csDestPort = to_wstring(ntohs(tcpheader->usDestPort)).c_str();
				csText = _T("TCP:\t");
				break;
			}
			case UDP_PROTOCOL:
			{
				udpheader = (UDP_HDR*)(buffer + iphdrlen);
				csSrcPort = to_wstring(ntohs(udpheader->usSourcePort)).c_str();
				csDestPort = to_wstring(ntohs(udpheader->usDestPort)).c_str();
				csText = _T("UDP:\t");
				break;
			}
			default:
			{
				csText = _T("OTHERS:\t");
				break;
			}
		}
		if (GetIPFilterULONG() == iphdr->unDestaddress)
		{
			csReport = csText + sourceIP + _T(":") + csSrcPort + _T("->") + destIP + _T(":") + csDestPort + _T("\t\tSize: ") + to_wstring(nSize).c_str() + _T(" bytes\r\n");
			if (m_pmodeless)
				m_pmodeless->UpdatePacketInfo(csReport, iphdr->ucIPProtocol);
		}
		else if (GetIPFilterULONG() == iphdr->unSrcaddress)
		{
			csReport = csText + sourceIP + _T(":") + csSrcPort + _T("->") + destIP + _T(":") + csDestPort + _T("\t\tSize: ") + to_wstring(nSize).c_str() + _T(" bytes\r\n");
			if (m_pmodeless)
				m_pmodeless->UpdatePacketInfo(csReport, iphdr->ucIPProtocol);
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

	if (m_hNICPacketListener)
	{
		map<ULONG, CDeviceConnected*>::iterator it = m_mConnected.begin();
		while (it != m_mConnected.end())
		{
			m_fnptrStartPacketListenerEx(it->second->m_hPacketListenerDownload);
			m_fnptrStartPacketListenerEx(it->second->m_hPacketListenerUpload);
			it++;
		}
	}
}


void CCheckOpenPortsDlg::OnBnClickedButtonStopPacket()
{
	// TODO: Add your control notification handler code here
	m_bStopPacketListener = true;
	m_ctrlBtnListenPackets.EnableWindow(TRUE);
	m_ctrlBtnUnlistenPackets.EnableWindow(FALSE);

	map<ULONG, CDeviceConnected*>::iterator it = m_mConnected.begin();
	while (it != m_mConnected.end())
	{
		if (it->second->m_hPacketListenerDownload)
		{
			m_fnptrStopPacketListenerEx(it->second->m_hPacketListenerDownload);
		}
		if (it->second->m_hPacketListenerUpload)
		{
			m_fnptrStopPacketListenerEx(it->second->m_hPacketListenerUpload);
		}
		it++;
	}
}


void CCheckOpenPortsDlg::OnBnClickedButtonShowPackets()
{
	// TODO: Add your control notification handler code here
	
	if (m_bShowPacketInfo)
	{
		m_fnptrStartPacketListenerEx(m_hNICPacketListener);
		m_ctrlBtnShowPacketInfo.SetWindowText(_T("Hide Packet Info"));
		m_bShowPacketInfo = false;
	
		if (m_pmodeless)
		{
			m_pmodeless->SetForegroundWindow();
		}
		else
		{
			m_pmodeless = new CPacketInfoDlg(this);
			m_pmodeless->Create(CPacketInfoDlg::IDD, GetDesktopWindow());
			m_pmodeless->ShowWindow(SW_SHOW);
			m_pmodeless->SetFocus();
		}

		m_pmodeless->GetClientRect(&m_rectModeless);

	}
	else
	{
		m_bShowPacketInfo = true;
		m_fnptrStopPacketListenerEx(m_hNICPacketListener);

		m_ctrlBtnShowPacketInfo.SetWindowText(_T("Show Packet Info"));
		
	
		if (m_pmodeless)
		{
			m_pmodeless->OnBnClickedCancel();
	
		}
	}
}


void CCheckOpenPortsDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	RECT rect,rectDlg,translatedRect;
	this->GetWindowRect(&rectDlg);
	m_ctrlStaticLogo.GetWindowRect(&rect);
	m_ctrlStaticLogo.GetClientRect(&translatedRect);

	translatedRect.left = rect.left - rectDlg.left - 10;
	translatedRect.top = rect.top - rectDlg.top - 30;
	translatedRect.right = rect.right - rectDlg.left - 10;
	translatedRect.bottom = rect.bottom - rectDlg.top - 30;

	if ((translatedRect.left <= (point.x)) && ((point.x) <= translatedRect.right) &&
		((translatedRect.top) <= point.y) && (point.y <= (translatedRect.bottom)))
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
	}
	else
	{
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		this->GetWindowRect(&rectDlg);
		m_ctrlStaticRouterImage.GetWindowRect(&rect);
		m_ctrlStaticRouterImage.GetClientRect(&translatedRect);

		translatedRect.left = rect.left - rectDlg.left - 10;
		translatedRect.top = rect.top - rectDlg.top - 30;
		translatedRect.right = rect.right - rectDlg.left - 10;
		translatedRect.bottom = rect.bottom - rectDlg.top - 30;

		if ((translatedRect.left <= (point.x)) && ((point.x) <= translatedRect.right) &&
			((translatedRect.top) <= point.y) && (point.y <= (translatedRect.bottom)))
		{
			SetCursor(LoadCursor(NULL, IDC_HAND));
		}
		else
		{
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
	}

	
	CDialogEx::OnMouseMove(nFlags, point);
}


//void CCheckOpenPortsDlg::OnLButtonDown(UINT nFlags, CPoint point)
//{
//	// TODO: Add your message handler code here and/or call default
//	RECT rect, rectDlg, translatedRect;
//	this->GetWindowRect(&rectDlg);
//	m_ctrlStaticLogo.GetWindowRect(&rect);
//	m_ctrlStaticLogo.GetClientRect(&translatedRect);
//
//	translatedRect.left = rect.left - rectDlg.left - 10;
//	translatedRect.top = rect.top - rectDlg.top - 30;
//	translatedRect.right = rect.right - rectDlg.left - 10;
//	translatedRect.bottom = rect.bottom - rectDlg.top - 30;
//
//	if ((translatedRect.left <= (point.x)) && ((point.x) <= translatedRect.right) &&
//		((translatedRect.top) <= point.y) && (point.y <= (translatedRect.bottom)))
//	{
//		ShellExecute(NULL, _T("open"), _T("https://m.me/Lorenzo.Leonardo.92"), NULL, NULL, SW_SHOWNORMAL);
//	}
//
//	this->GetWindowRect(&rectDlg);
//	m_ctrlStaticRouterImage.GetWindowRect(&rect);
//	m_ctrlStaticRouterImage.GetClientRect(&translatedRect);
//
//	translatedRect.left = rect.left - rectDlg.left - 10;
//	translatedRect.top = rect.top - rectDlg.top - 30;
//	translatedRect.right = rect.right - rectDlg.left - 10;
//	translatedRect.bottom = rect.bottom - rectDlg.top - 30;
//	if((translatedRect.left <= (point.x)) && ((point.x) <= translatedRect.right) &&
//		((translatedRect.top) <= point.y) && (point.y <= (translatedRect.bottom)))
//	{
//		CString csLink;
//		char szDefaultGateway[32];
//		memset(szDefaultGateway, 0, sizeof(szDefaultGateway));
//
//		m_pfnPtrGetDefaultGatewayEx(m_vAdapterInfo[m_nCurrentNICSelect].AdapterInfo.AdapterName, szDefaultGateway, sizeof(szDefaultGateway));
//		csLink = _T("http://");
//		csLink.AppendFormat(_T("%s"), CA2W(szDefaultGateway).m_szBuffer);
//		ShellExecute(NULL, _T("open"), csLink, NULL, NULL, SW_SHOWNORMAL);
//	}
//	if (m_pmodeless)
//		m_pmodeless->SetForegroundWindow();
//	CDialogEx::OnLButtonDown(nFlags, point);
//}


void CCheckOpenPortsDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);


	if (m_pmodeless)
	{
		RECT  rectParent;
		GetClientRect(&rectParent);
		m_pmodeless->MoveWindow(x + rectParent.right, y-32, m_rectModeless.right, m_rectModeless.bottom+3);
		m_pmodeless->SetForegroundWindow();
		// TODO: Add your message handler code here
	}
}


void CCheckOpenPortsDlg::OnBnClickedCheckDebug()
{
	// TODO: Add your control notification handler code here
	int ChkBox = m_ctrlBtnDebug.GetCheck();
	CString str;
	CString csDevName, csMacAddress;
	HKEY hKey;
	DWORD dwError = 0;

	dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PATH, 0, KEY_ALL_ACCESS, &hKey);
	if (dwError == ERROR_FILE_NOT_FOUND)
		dwError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PATH, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	else if (dwError == ERROR_ACCESS_DENIED)
		AfxMessageBox(_T("Access denied. Please run the tool as administrator."));



	if (dwError == ERROR_SUCCESS)
	{
		DWORD value = 0;
		DWORD BufferSize = 4;
		if (ChkBox == BST_UNCHECKED)
		{
			
			value = 0;
			m_ctrlBtnDebug.SetCheck(BST_UNCHECKED);
			LONG setRes = RegSetValueEx(hKey, _T("DebugLog"), 0, RRF_RT_DWORD, (LPBYTE)&value, BufferSize);
			AfxMessageBox(_T("Debugging log of the tool is disabled."), MB_ICONINFORMATION);
			if (setRes == ERROR_SUCCESS)
				goto REGCLOSE;
			else
				goto REGCLOSE;
		}
		else if (ChkBox == BST_CHECKED)
		{
			value = 1;
			m_ctrlBtnDebug.SetCheck(BST_CHECKED);
			LONG setRes = RegSetValueEx(hKey, _T("DebugLog"), 0, RRF_RT_REG_DWORD, (LPBYTE)&value, BufferSize);
			AfxMessageBox(_T("Debugging log of the tool is enabled."), MB_ICONINFORMATION);
			if (setRes == ERROR_SUCCESS)
				goto REGCLOSE;
			else
				goto REGCLOSE;
		}
	}
REGCLOSE:
	RegCloseKey(hKey);
}


void CCheckOpenPortsDlg::OnCbnSelchangeComboListAdapter()
{
	// TODO: Add your control notification handler code here
	CStringA csWindowText;
	char szDefaultGateWay[32];
	u_char p[8];
	int i = m_ctrlComboAdapterList.GetCurSel();

	memset(szDefaultGateWay, 0, sizeof(szDefaultGateWay));
	if (!m_vAdapterInfo.empty())
	{
		if (m_pfnPtrGetDefaultGatewayEx(m_vAdapterInfo[i].AdapterInfo.AdapterName, szDefaultGateWay, sizeof(szDefaultGateWay)))
		{
			m_ipFilter = CA2W(m_vAdapterInfo[i].AdapterInfo.IpAddressList.IpAddress.String);
			inet_pton(AF_INET, m_vAdapterInfo[i].AdapterInfo.IpAddressList.IpAddress.String, &m_ulIPFilter);

			csWindowText.Format("Adapter Name: \t%s\r\n", m_vAdapterInfo[i].AdapterInfo.AdapterName);
			csWindowText.AppendFormat("Adapter Desc: \t%s\r\n", m_vAdapterInfo[i].AdapterInfo.Description);
			memcpy(p, m_vAdapterInfo[i].AdapterInfo.Address, sizeof(m_vAdapterInfo[i].AdapterInfo.Address));
			csWindowText.AppendFormat("Adapter Addr: \t%X:%X:%X:%X:%X:%X\r\n", 
				p[0], p[1], p[2], p[3], p[4], p[5]);
			csWindowText.AppendFormat("IP Addr: \t\t%s\r\n",  m_vAdapterInfo[i].AdapterInfo.IpAddressList.IpAddress.String);
			csWindowText.AppendFormat("IP Mask: \t\t%s\r\n", m_vAdapterInfo[i].AdapterInfo.IpAddressList.IpMask.String);
			csWindowText.AppendFormat("IP Gateway: \t%s\r\n", m_vAdapterInfo[i].AdapterInfo.GatewayList.IpAddress.String);
			if (m_vAdapterInfo[i].AdapterInfo.DhcpEnabled)
			{
				csWindowText.AppendFormat("DHCP Enable: \tYes\r\n");
				csWindowText.AppendFormat("Lease Obtained: \t%lld\r\n", m_vAdapterInfo[i].AdapterInfo.LeaseObtained);
			}
			else
				csWindowText.AppendFormat("DHCP Enable: \tNo\r\n");

			if (m_vAdapterInfo[i].AdapterInfo.HaveWins)
			{
				csWindowText.AppendFormat("Have Wins: \tYes\r\n");
				csWindowText.AppendFormat("Primary Wins Server: \t%s\r\n",  m_vAdapterInfo[i].AdapterInfo.PrimaryWinsServer.IpAddress.String);
				csWindowText.AppendFormat("Secondary Wins Server: \t%s\r\n",  m_vAdapterInfo[i].AdapterInfo.SecondaryWinsServer.IpAddress.String);
			}
			else
				csWindowText.AppendFormat("Have Wins: \tNo\r\n");

			if (i != m_nCurrentNICSelect)
			{
				OnBnClickedButtonStopListenLan();
				m_nCurrentNICSelect = i;
				m_ctrlComboAdapterList.EnableWindow(FALSE);
				OnBnClickedButtonStartListenLan();
			}
			m_ctrlEditAdapterInfo.SetWindowText(CA2W(csWindowText));
		}
	}
	else
	{
		m_ctrlEditAdapterInfo.SetWindowText(_T(""));
		OnBnClickedButtonStopListenLan();
		OnBnClickedButtonStopSearchingOpenPorts();
		OnBnClickedButtonStopPacket();
		m_nCurrentNICSelect = -1;
		m_bStopPacketListener = false;
		AfxMessageBox(_T("All Network Interfaces are disconnected. Please check your ethernet ports or WIFI adapters."));
	}
}


inline void CCheckOpenPortsDlg::DisplayDownloadSpeed(CString szIPAddress, int nColumn, double ldData)
{
	CString csFormat;
	LVFINDINFO lvFindInfo;
	LVITEM lvItem;
	int findResult = -1;

	lvFindInfo.flags = LVFI_PARTIAL | LVFI_STRING;
	lvFindInfo.psz = szIPAddress;
	lvItem.mask = LVIF_TEXT;

	for (int i = 0; i < m_ctrlLANConnected.GetItemCount(); ++i)
	{
		CString szText = m_ctrlLANConnected.GetItemText(i, COL_IPADDRESS);
		if (szText == szIPAddress)
		{
			findResult = i;
			break;
		}
	}
	if (findResult != -1)
	{
		if (ldData < 1000)
			csFormat.Format(_T("%.2lf Kbps"), ldData);
		else
			csFormat.Format(_T("%.2lf Mbps"), ldData / 1000);

		if (m_ctrlLANConnected.GetItemText(findResult, nColumn).Compare(csFormat) != 0)
		{
			lvItem.iItem = findResult;
			lvItem.iSubItem = nColumn;
			lvItem.pszText = csFormat.GetBuffer();
			m_ctrlLANConnected.EnsureVisible(findResult, TRUE);
			m_ctrlLANConnected.SetItem(&lvItem);
			m_ctrlLANConnected.RedrawItems(findResult, findResult);
		}
	}
}

inline void CCheckOpenPortsDlg::DisplayUploadSpeed(CString szIPAddress, int nColumn, double ldData)
{
	CString csFormat;
	LVFINDINFO lvFindInfo;
	LVITEM lvItem;
	int findResult = -1;

	lvFindInfo.flags = LVFI_PARTIAL | LVFI_STRING;
	lvFindInfo.psz = szIPAddress;
	lvItem.mask = LVIF_TEXT;

	for (int i = 0; i < m_ctrlLANConnected.GetItemCount(); ++i)
	{
		CString szText = m_ctrlLANConnected.GetItemText(i, COL_IPADDRESS);
		if (szText == szIPAddress)
		{
			findResult = i;
			break;
		}
	}
	if (findResult != -1)
	{
		if (ldData < 1000)
			csFormat.Format(_T("%.2lf Kbps"), ldData);
		else
			csFormat.Format(_T("%.2lf Mbps"), ldData / 1000);

		if (m_ctrlLANConnected.GetItemText(findResult, nColumn).Compare(csFormat) != 0)
		{
			lvItem.iItem = findResult;
			lvItem.iSubItem = nColumn;
			lvItem.pszText = csFormat.GetBuffer();
			m_ctrlLANConnected.EnsureVisible(findResult, TRUE);
			m_ctrlLANConnected.SetItem(&lvItem);
			m_ctrlLANConnected.RedrawItems(findResult, findResult);
		}
	}
}

bool CCheckOpenPortsDlg::CallbackPacketListenerDownloadEx(unsigned char* buffer, int nSize, void* pObject)
{
	return g_pCCheckOpenPortsDlg->ProcessPacketListenerDownloadEx(buffer, nSize, pObject);
}
bool CCheckOpenPortsDlg::ProcessPacketListenerDownloadEx(unsigned char* buffer, int nSize, void* pObject)
{
	CDeviceConnected* pDevice = (CDeviceConnected*)pObject;
	CString sourceIP, destIP, csText, csSrcPort, csDestPort, csReport;
	int iphdrlen = 0;
	IPV4_HDR* iphdr;
	TCP_HDR* tcpheader = NULL;
	UDP_HDR* udpheader = NULL;
	char sztemp[32];

	memset(sztemp, 0, sizeof(sztemp));
	iphdr = (IPV4_HDR*)buffer;
	iphdrlen = iphdr->ucIPHeaderLen * 4;
	inet_ntop(AF_INET, (const void*)&iphdr->unDestaddress, sztemp, sizeof(sztemp));
	destIP = CA2W(sztemp);
	inet_ntop(AF_INET, (const void*)&iphdr->unSrcaddress, sztemp, sizeof(sztemp));
	sourceIP = CA2W(sztemp);

	if (IsInternetOnly())
	{
		DWORD dwMask = 0;
		DWORD dwBeginIP = 0;
		DWORD dwEndIP = 0;

		inet_pton(AF_INET, m_vAdapterInfo[m_ctrlComboAdapterList.GetCurSel()].AdapterInfo.IpAddressList.IpMask.String, &dwMask);
		dwMask = ntohl(dwMask);
		inet_pton(AF_INET, m_vAdapterInfo[m_ctrlComboAdapterList.GetCurSel()].AdapterInfo.IpAddressList.IpAddress.String, &dwBeginIP);
		dwBeginIP = dwMask & ntohl(dwBeginIP);
		dwEndIP = (0xFFFFFFFF - dwMask) + dwBeginIP;

		if (((ntohl(iphdr->unDestaddress) > dwBeginIP) && (ntohl(iphdr->unDestaddress) < dwEndIP)) && ((ntohl(iphdr->unSrcaddress) > dwBeginIP) && (ntohl(iphdr->unSrcaddress) < dwEndIP)))
			return false;
	}

	ULONGLONG timeCurrent = GetTickCount64();
	if (pDevice->m_szIPAddress == destIP)
		pDevice->m_ulDataSizeDownload += nSize;

	if ((timeCurrent - pDevice->m_ullDownloadStartTime) >= POLLING_TIME)
	{
		pDevice->m_lfDownloadSpeed = ((double)pDevice->m_ulDataSizeDownload / (double)(timeCurrent - pDevice->m_ullDownloadStartTime)) * 8;
		if (pDevice->m_lfMaxDownloadSpeed < pDevice->m_lfDownloadSpeed)
		{
			pDevice->m_lfMaxDownloadSpeed = pDevice->m_lfDownloadSpeed;
			DisplayDownloadSpeed(pDevice->m_szIPAddress, COL_DOWNLOADMAXSPEED, pDevice->m_lfMaxDownloadSpeed);
		}
		pDevice->m_ulDataSizeDownload = 0;
		DisplayDownloadSpeed(pDevice->m_szIPAddress, COL_DOWNLOADSPEED, pDevice->m_lfDownloadSpeed);
		pDevice->m_ullDownloadStartTime = GetTickCount64();
	}
	return true;
}
bool CCheckOpenPortsDlg::CallbackPacketListenerUploadEx(unsigned char* buffer, int nSize, void* pObject)
{
	return g_pCCheckOpenPortsDlg->ProcessPacketListenerUploadEx(buffer, nSize, pObject);
}

bool CCheckOpenPortsDlg::ProcessPacketListenerUploadEx(unsigned char* buffer, int nSize, void* pObject)
{
	CDeviceConnected* pDevice = (CDeviceConnected*)pObject;
	CString sourceIP, destIP;
	int iphdrlen = 0;
	IPV4_HDR* iphdr;
	TCP_HDR* tcpheader = NULL;
	UDP_HDR* udpheader = NULL;
	char sztemp[32];

	memset(sztemp, 0, sizeof(sztemp));
	iphdr = (IPV4_HDR*)buffer;
	iphdrlen = iphdr->ucIPHeaderLen * 4;
	inet_ntop(AF_INET, (const void*)&iphdr->unDestaddress, sztemp, sizeof(sztemp));
	destIP = CA2W(sztemp);
	inet_ntop(AF_INET, (const void*)&iphdr->unSrcaddress, sztemp, sizeof(sztemp));
	sourceIP = CA2W(sztemp);

	if (IsInternetOnly())
	{
		DWORD dwMask = 0;
		DWORD dwBeginIP = 0;
		DWORD dwEndIP = 0;

		inet_pton(AF_INET, m_vAdapterInfo[m_ctrlComboAdapterList.GetCurSel()].AdapterInfo.IpAddressList.IpMask.String, &dwMask);
		dwMask = ntohl(dwMask);
		inet_pton(AF_INET, m_vAdapterInfo[m_ctrlComboAdapterList.GetCurSel()].AdapterInfo.IpAddressList.IpAddress.String, &dwBeginIP);
		dwBeginIP = dwMask & ntohl(dwBeginIP);
		dwEndIP = (0xFFFFFFFF - dwMask) + dwBeginIP;

		if (((ntohl(iphdr->unDestaddress) > dwBeginIP) && (ntohl(iphdr->unDestaddress) < dwEndIP)) && ((ntohl(iphdr->unSrcaddress) > dwBeginIP) && (ntohl(iphdr->unSrcaddress) < dwEndIP)))
			return false;
	}

	ULONGLONG timeCurrent = GetTickCount64();
	if (pDevice->m_szIPAddress == sourceIP)
		pDevice->m_ulDataSizeUpload += nSize;

	if ((timeCurrent - pDevice->m_ullUploadStartTime) >= POLLING_TIME)
	{
		pDevice->m_lfUploadSpeed = ((double)pDevice->m_ulDataSizeUpload / (double)(timeCurrent - pDevice->m_ullUploadStartTime)) * 8;
		if (pDevice->m_lfMaxUploadSpeed < pDevice->m_lfUploadSpeed)
		{
			pDevice->m_lfMaxUploadSpeed = pDevice->m_lfUploadSpeed;
			DisplayUploadSpeed(pDevice->m_szIPAddress, COL_UPLOADMAXSPEED, pDevice->m_lfMaxUploadSpeed);
		}
		pDevice->m_ulDataSizeUpload = 0;
		DisplayUploadSpeed(pDevice->m_szIPAddress, COL_UPLOADSPEED, pDevice->m_lfUploadSpeed);
		pDevice->m_ullUploadStartTime = GetTickCount64();
	}
	return true;
}
//void CCheckOpenPortsDlg::OnSetFocus(CWnd* pOldWnd)
//{
//	if (m_pmodeless)
//		m_pmodeless->SetForegroundWindow();
//	CDialogEx::OnSetFocus(pOldWnd);
//	// TODO: Add your message handler code here
//}


//void CCheckOpenPortsDlg::OnNcLButtonUp(UINT nHitTest, CPoint point)
//{
//	// TODO: Add your message handler code here and/or call default
//	if (m_pmodeless)
//		m_pmodeless->SetForegroundWindow();
//	CDialogEx::OnNcLButtonUp(nHitTest, point);
//}


//void CCheckOpenPortsDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
//{
//	// TODO: Add your message handler code here and/or call default
//	if (m_pmodeless)
//		m_pmodeless->SetForegroundWindow();
//	CDialogEx::OnNcLButtonDown(nHitTest, point);
//}


//void CCheckOpenPortsDlg::OnLButtonUp(UINT nFlags, CPoint point)
//{
//	// TODO: Add your message handler code here and/or call default
//	if(m_pmodeless)
//		m_pmodeless->SetForegroundWindow();
//	CDialogEx::OnLButtonUp(nFlags, point);
//}


void CCheckOpenPortsDlg::OnMoving(UINT fwSide, LPRECT pRect)
{

	if (m_pmodeless)
		m_pmodeless->SetFocus();
	// TODO: Add your message handler code here
	CDialogEx::OnMoving(fwSide, pRect);
}


void CCheckOpenPortsDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CCheckOpenPortsDlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnRButtonDblClk(nFlags, point);
}


void CCheckOpenPortsDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnRButtonDown(nFlags, point);
}


void CCheckOpenPortsDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnRButtonUp(nFlags, point);
}


void CCheckOpenPortsDlg::OnBnClickedCheckInternetOnly()
{
	// TODO: Add your control notification handler code here
	int ChkBox = ((CButton*)GetDlgItem(IDC_CHECK_INTERNET_ONLY))->GetCheck();
	if (ChkBox == BST_UNCHECKED)
	{
		((CButton*)GetDlgItem(IDC_CHECK_INTERNET_ONLY))->SetCheck(BST_UNCHECKED);
		m_bIsInternetOnly = false;
		AfxMessageBox(_T("The tool will be able to listen both local area and internet packets."), MB_ICONINFORMATION);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHECK_INTERNET_ONLY))->SetCheck(BST_CHECKED);
		m_bIsInternetOnly = true;
		AfxMessageBox(_T("The tool will be able to listen internet packets only."), MB_ICONINFORMATION);
	}
}

void CCheckOpenPortsDlg::EnableCloseButton(bool bEnable)
{
	UINT nMenuf = bEnable ? (MF_BYCOMMAND) : (MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);

	CMenu* pSysMenu = g_pCCheckOpenPortsDlg->GetSystemMenu(FALSE);
	if (pSysMenu)
		pSysMenu->EnableMenuItem(SC_CLOSE, nMenuf);
}

void CCheckOpenPortsDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

//	CDialogEx::OnCancel();
}


void CCheckOpenPortsDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialogEx::OnOK();
}


void CCheckOpenPortsDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnNcLButtonDown(nHitTest, point);
}


void CCheckOpenPortsDlg::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnNcLButtonUp(nHitTest, point);
}


void CCheckOpenPortsDlg::OnNcMouseHover(UINT nFlags, CPoint point)
{
	// This feature requires Windows 2000 or greater.
	// The symbols _WIN32_WINNT and WINVER must be >= 0x0500.
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnNcMouseHover(nFlags, point);
}


void CCheckOpenPortsDlg::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnNcMouseMove(nHitTest, point);
}



void CCheckOpenPortsDlg::OnNcMouseLeave()
{
	// This feature requires Windows 2000 or greater.
	// The symbols _WIN32_WINNT and WINVER must be >= 0x0500.
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnNcMouseLeave();
}


void CCheckOpenPortsDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnLButtonUp(nFlags, point);
}


void CCheckOpenPortsDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CCheckOpenPortsDlg::OnMouseHover(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnMouseHover(nFlags, point);
}


void CCheckOpenPortsDlg::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default
	if (m_pmodeless)
		m_pmodeless->SetFocus();
	CDialogEx::OnMouseLeave();
}
