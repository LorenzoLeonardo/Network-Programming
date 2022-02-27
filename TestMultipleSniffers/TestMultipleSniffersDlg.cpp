
// TestMultipleSniffersDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "TestMultipleSniffers.h"
#include "TestMultipleSniffersDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CTestMultipleSniffersDlg* g_dlg;
// CAboutDlg dialog used for App About

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


// CTestMultipleSniffersDlg dialog



CTestMultipleSniffersDlg::CTestMultipleSniffersDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TESTMULTIPLESNIFFERS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestMultipleSniffersDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_ctrlEdit1);
	DDX_Control(pDX, IDC_EDIT2, m_ctrlEdit2);
	DDX_Control(pDX, IDC_BUTTON_START_STOP, m_ctrlBtnStartStopSniff);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ctrlIPAddress);
}

BEGIN_MESSAGE_MAP(CTestMultipleSniffersDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START_STOP, &CTestMultipleSniffersDlg::OnBnClickedButtonStartStop)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CTestMultipleSniffersDlg message handlers

BOOL CTestMultipleSniffersDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.
	g_dlg = this;
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

	m_hModuleDLL = LoadLibrary(_T("EnzTCP.dll"));
	if (m_hModuleDLL)
	{
		m_fnptrStartPacketListenerEx = (FNPTRStartPacketListenerEx)GetProcAddress(m_hModuleDLL, "StartPacketListenerEx");
		m_fnptrStopPacketListenerEx = (FNPTRStopPacketListenerEx)GetProcAddress(m_hModuleDLL, "StopPacketListenerEx");
		m_fnptrCreatePacketListenerEx = (FNPTRCreatePacketListenerEx)GetProcAddress(m_hModuleDLL, "CreatePacketListenerEx");
		m_fnptrDeletePacketListenerEx = (FNPTRDeletePacketListenerEx)GetProcAddress(m_hModuleDLL, "DeletePacketListenerEx");
	}


	// TODO: Add extra initialization here
	m_bStart = false;
	m_ctrlIPAddress.SetWindowTextW(_T("192.168.0.101"));
	CDeviceConnected* obj = new CDeviceConnected();

	obj->m_ulDataSizeDownload = 0;
	obj->m_ulDataSizeUpload = 0;
	obj->m_lfDownloadSpeed = 0;
	obj->m_lfUploadSpeed = 0;
	obj->m_lfMaxDownloadSpeed = 0;
	obj->m_lfMaxUploadSpeed = 0;
	obj->m_szHostName = _T("192.168.0.101");
	obj->m_szIPAddress = _T("192.168.0.101");
	obj->m_szMACAddress = _T("192.168.0.101");
	obj->m_ullDownloadStartTime = GetTickCount64();
	obj->m_ullUploadStartTime = GetTickCount64();
	obj->m_hPacketListener = m_fnptrCreatePacketListenerEx(CallbackPacketListenerEx, (void*)obj);
	m_mapListConnected[obj->m_szIPAddress] = obj;

	obj = new CDeviceConnected();

	obj->m_ulDataSizeDownload = 0;
	obj->m_ulDataSizeUpload = 0;
	obj->m_lfDownloadSpeed = 0;
	obj->m_lfUploadSpeed = 0;
	obj->m_lfMaxDownloadSpeed = 0;
	obj->m_lfMaxUploadSpeed = 0;
	obj->m_szHostName = _T("192.168.0.101");
	obj->m_szIPAddress = _T("192.168.0.109");
	obj->m_szMACAddress = _T("192.168.0.101");
	obj->m_ullDownloadStartTime = GetTickCount64();
	obj->m_ullUploadStartTime = GetTickCount64();
	obj->m_hPacketListener = m_fnptrCreatePacketListenerEx(CallbackPacketListenerEx, (void*)obj);
	m_mapListConnected[obj->m_szIPAddress] = obj;

	obj = new CDeviceConnected();

	obj->m_ulDataSizeDownload = 0;
	obj->m_ulDataSizeUpload = 0;
	obj->m_lfDownloadSpeed = 0;
	obj->m_lfUploadSpeed = 0;
	obj->m_lfMaxDownloadSpeed = 0;
	obj->m_lfMaxUploadSpeed = 0;
	obj->m_szHostName = _T("192.168.0.101");
	obj->m_szIPAddress = _T("192.168.0.104");
	obj->m_szMACAddress = _T("192.168.0.101");
	obj->m_ullDownloadStartTime = GetTickCount64();
	obj->m_ullUploadStartTime = GetTickCount64();
	obj->m_hPacketListener = m_fnptrCreatePacketListenerEx(CallbackPacketListenerEx, (void*)obj);
	m_mapListConnected[obj->m_szIPAddress] = obj;
	

	/*CDeviceConnected* obj1 = new CDeviceConnected();

	obj1->m_ulDataSizeDownload = 0;
	obj1->m_ulDataSizeUpload = 0;
	obj1->m_lfDownloadSpeed = 0;
	obj1->m_lfUploadSpeed = 0;
	obj1->m_lfMaxDownloadSpeed = 0;
	obj1->m_lfMaxUploadSpeed = 0;
	obj1->m_szHostName = _T("192.168.0.1");
	obj1->m_szIPAddress = _T("192.168.0.1");
	obj1->m_szMACAddress = _T("192.168.0.1");
	obj1->m_ullDownloadStartTime = GetTickCount64();
	obj1->m_ullUploadStartTime = GetTickCount64();

	m_hPacketHandle2 = m_fnptrCreatePacketListenerEx(CallbackPacketListenerEx, (void*)obj);*/

	return TRUE;  // return TRUE  unless you set the focus to a control
}
bool CTestMultipleSniffersDlg::CallbackPacketListenerEx(unsigned char* buffer, int nSize, void* pObject)
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
	destIP = sztemp;
	inet_ntop(AF_INET, (const void*)&iphdr->unSrcaddress, sztemp, sizeof(sztemp));
	sourceIP = sztemp;

	if (pDevice->m_szIPAddress == sourceIP)
	{
		pDevice->m_ulDataSizeUpload += nSize;
		ULONGLONG timeCurrent = GetTickCount64();
		if ((timeCurrent - pDevice->m_ullUploadStartTime) >= 500)
		{
			pDevice->m_lfUploadSpeed = ((double)pDevice->m_ulDataSizeUpload / (double)(timeCurrent - pDevice->m_ullUploadStartTime)) * 8;
			if (pDevice->m_lfMaxUploadSpeed < pDevice->m_lfUploadSpeed)
				pDevice->m_lfMaxUploadSpeed = pDevice->m_lfUploadSpeed;
			pDevice->m_ulDataSizeUpload = 0;
			g_dlg->DisplayUploadSpeed(pDevice);
			pDevice->m_ullUploadStartTime = GetTickCount64();
		}
	}
	if (pDevice->m_szIPAddress == destIP)
	{
		pDevice->m_ulDataSizeDownload += nSize;
		ULONGLONG timeCurrent = GetTickCount64();
		if ((timeCurrent - pDevice->m_ullDownloadStartTime) >= 500)
		{
			pDevice->m_lfDownloadSpeed = ((double)pDevice->m_ulDataSizeDownload / (double)(timeCurrent - pDevice->m_ullDownloadStartTime)) * 8;
			if (pDevice->m_lfMaxDownloadSpeed < pDevice->m_lfDownloadSpeed)
				pDevice->m_lfMaxDownloadSpeed = pDevice->m_lfDownloadSpeed;
			pDevice->m_ulDataSizeDownload = 0;
			g_dlg->DisplayDownloadSpeed(pDevice);
			pDevice->m_ullDownloadStartTime = GetTickCount64();
		}
	}
	return true;
}

void CTestMultipleSniffersDlg::DisplayDownloadSpeed(CDeviceConnected* pDeviceConnected)
{
	CString csFormat;
	if (pDeviceConnected->m_lfDownloadSpeed <= 1000)
		csFormat.Format(_T("%.2lf Kbps"), pDeviceConnected->m_lfDownloadSpeed);
	else
		csFormat.Format(_T("%.2lf Mbps"), pDeviceConnected->m_lfDownloadSpeed / 1000);
	m_ctrlEdit1.SetWindowText(csFormat);
}

void CTestMultipleSniffersDlg::DisplayUploadSpeed(CDeviceConnected *pDeviceConnected)
{
	CString csFormat;
	if (pDeviceConnected->m_lfUploadSpeed <= 1000)
		csFormat.Format(_T("%.2lf Kbps"), pDeviceConnected->m_lfUploadSpeed);
	else
		csFormat.Format(_T("%.2lf Mbps"), pDeviceConnected->m_lfUploadSpeed / 1000);
	m_ctrlEdit2.SetWindowText(csFormat);
}

void CTestMultipleSniffersDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTestMultipleSniffersDlg::OnPaint()
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
HCURSOR CTestMultipleSniffersDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTestMultipleSniffersDlg::OnBnClickedButtonStartStop()
{
	// TODO: Add your control notification handler code here
	if (!m_bStart)
	{
		map<CString, CDeviceConnected*>::iterator it = m_mapListConnected.begin();
		while (it != m_mapListConnected.end())
		{
			m_fnptrStartPacketListenerEx(it->second->m_hPacketListener);
			it++;
		}
		m_bStart = true;
	}
	else
	{
		map<CString, CDeviceConnected*>::iterator it = m_mapListConnected.begin();
		while (it != m_mapListConnected.end())
		{
			m_fnptrStopPacketListenerEx(it->second->m_hPacketListener);
			it++;
		}
		m_bStart = false;
	}
}

void CTestMultipleSniffersDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	map<CString, CDeviceConnected*>::iterator it = m_mapListConnected.begin();

	while (it != m_mapListConnected.end())
	{
		m_fnptrStopPacketListenerEx(it->second->m_hPacketListener);
		m_fnptrDeletePacketListenerEx(it->second->m_hPacketListener);

		delete it->second;
		it++;
	}
	m_mapListConnected.clear();
	CDialogEx::OnClose();
}
