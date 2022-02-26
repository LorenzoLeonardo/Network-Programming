
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
	m_ctrlIPAddress.SetWindowTextW(_T("192.168.0.101"));

	// TODO: Add extra initialization here
	m_bStart = false;
	m_hPacketHandle1 = m_fnptrCreatePacketListenerEx(CallbackPacketListener1);
	m_hPacketHandle2 = m_fnptrCreatePacketListenerEx(CallbackPacketListener2);

	return TRUE;  // return TRUE  unless you set the focus to a control
}
bool CTestMultipleSniffersDlg::CallbackPacketListener1(unsigned char* buffer, int nSize)
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
		g_dlg->m_ctrlEdit1.GetWindowText(csText);
		csReport = csText + sourceIP + _T(":") + csSrcPort + _T(" -> ") + destIP + _T(":") + csDestPort + _T(" Size: ") + to_wstring(nSize).c_str() + _T(" bytes\r\n");
		g_dlg->m_ctrlEdit1.SetWindowText(csReport);

	return true;
}
bool CTestMultipleSniffersDlg::CallbackPacketListener2(unsigned char* buffer, int nSize)
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
	g_dlg->m_ctrlEdit2.GetWindowText(csText);
	csReport = csText + sourceIP + _T(":") + csSrcPort + _T(" -> ") + destIP + _T(":") + csDestPort + _T(" Size: ") + to_wstring(nSize).c_str() + _T(" bytes\r\n");
	g_dlg->m_ctrlEdit2.SetWindowText(csReport);

	return true;
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
		m_fnptrStartPacketListenerEx(m_hPacketHandle1);
		m_fnptrStartPacketListenerEx(m_hPacketHandle2);

		m_bStart = true;
	}
	else
	{
		m_bStart = false;
		m_fnptrStopPacketListenerEx(m_hPacketHandle1);
		m_fnptrStopPacketListenerEx(m_hPacketHandle2);
	}
}

void CTestMultipleSniffersDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	m_fnptrStopPacketListenerEx(m_hPacketHandle1);
	m_fnptrStopPacketListenerEx(m_hPacketHandle2);

	m_fnptrDeletePacketListenerEx(m_hPacketHandle1);
	m_fnptrDeletePacketListenerEx(m_hPacketHandle2);
	CDialogEx::OnClose();
}
