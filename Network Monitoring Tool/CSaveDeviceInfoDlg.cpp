// CSaveDeviceInfoDlg.cpp : implementation file
//

#include "pch.h"
#include "afxdialogex.h"
#include "CSaveDeviceInfoDlg.h"
#include <ws2tcpip.h>


CSaveDeviceInfoDlg* g_pCSaveDeviceInfoDlg = NULL;
// CSaveDeviceInfoDlg dialog

IMPLEMENT_DYNAMIC(CSaveDeviceInfoDlg, CDialogEx)

CSaveDeviceInfoDlg::CSaveDeviceInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DEVICE_INFO, pParent)
{

}

CSaveDeviceInfoDlg::~CSaveDeviceInfoDlg()
{
//	m_fnptrDeletePacketListenerEx(m_hPacketListener);
	CloseHandle(m_hThreadStop);
	CloseHandle(m_hThread);
}

void CSaveDeviceInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DEVICE_NAME, m_ctrlEditDevicename);
	DDX_Control(pDX, IDC_EDIT_MAC, m_ctrlEditMACAddress);
	DDX_Control(pDX, IDC_IPADDRESS_IPINFO, m_ctrlEditIPAddress);
	DDX_Control(pDX, IDC_STATIC_DEVAREA, m_ctrlStaticArea);
	DDX_Control(pDX, IDC_EDIT_SITE_VISIT, m_ctrlEditSiteVisited);
	DDX_Control(pDX, IDC_LIST_VISITED, m_ctrlListVisited);
}


BEGIN_MESSAGE_MAP(CSaveDeviceInfoDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSaveDeviceInfoDlg::OnBnClickedOk)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CSaveDeviceInfoDlg::OnBnClickedCancel)
	
//	ON_REGISTERED_MESSAGE(WM_SITE_VISIT, &CSaveDeviceInfoDlg::OnSiteVisit)
ON_MESSAGE(WM_SITE_VISITED, &CSaveDeviceInfoDlg::OnSiteVisited)
END_MESSAGE_MAP()


// CSaveDeviceInfoDlg message handlers


unsigned _stdcall CSaveDeviceInfoDlg::SiteVisitedThread(void* args)
{
	CSaveDeviceInfoDlg* pDlg = (CSaveDeviceInfoDlg*)args;
	pDlg->SiteVisitedThread();
	
	return 0;
}
int CSaveDeviceInfoDlg::IsInTheList(CString csIPAddress)
{
	int index = -1;
	bool bRet = false;


	for (int i = 0; i < m_ctrlListVisited.GetItemCount(); ++i)
	{
		CString szText = m_ctrlListVisited.GetItemText(i, 2);
		if (szText == csIPAddress)
		{
			index = i;
			break;
		}
	}

	return index;
}
void CSaveDeviceInfoDlg::SiteVisitedThread()
{
	DWORD dwRet = 0;
	CString csText;
	int nRow = 0;
	do
	{
		dwRet = WaitForSingleObject(m_hThreadStop, 0);
		
		map<CString, CString>::iterator it = (*m_pmSiteVisited)[m_csIPAddress].begin();
		while ((it != (*m_pmSiteVisited)[m_csIPAddress].end()) && (dwRet != WAIT_OBJECT_0))
		{
			if (IsInTheList(csText = GetHostName(it->first)) == -1)
			{
				dwRet = WaitForSingleObject(m_hThreadStop, 0);
					if (WAIT_OBJECT_0 == dwRet)
						break;

					m_ctrlListVisited.InsertItem(LVIF_TEXT | LVIF_STATE, nRow, to_wstring(nRow + 1).c_str(), 0, 0, 0, 0);
	
					m_ctrlListVisited.SetItemText(nRow, 1, it->first);
		
					m_ctrlListVisited.SetItemText(nRow, 2, csText);
				nRow++;
			}
			it++;
		}
		//if (m_ctrlEditSiteVisited.m_hWnd)
		//	m_ctrlEditSiteVisited.SetWindowText(csText);
	
	} while (dwRet != WAIT_OBJECT_0);

}


afx_msg LRESULT CSaveDeviceInfoDlg::OnSiteVisited(WPARAM wParam, LPARAM lParam)
{
	
	
	return 0;
}
BOOL CSaveDeviceInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	LPCTSTR lpcRecHeader[] = { _T("No."), _T("IP Address"), _T("Site")};
	DWORD value = 0;
	DWORD BufferSize = 4;

	::SetWindowTheme(GetDlgItem(IDC_STATIC_ROUTER_INFO)->GetSafeHwnd(), NULL, _T(""));//To change text Color of Group Box
	::SetWindowTheme(GetDlgItem(IDC_STATIC_ADAPTER_INFO)->GetSafeHwnd(), NULL, _T(""));
	::SetWindowTheme(GetDlgItem(IDC_STATIC_OPEN_PORTS)->GetSafeHwnd(), NULL, _T(""));
	::SetWindowTheme(GetDlgItem(IDC_CHECK_DEBUG)->GetSafeHwnd(), NULL, _T(""));
	::SetWindowTheme(GetDlgItem(IDC_CHECK_INTERNET_ONLY)->GetSafeHwnd(), NULL, _T(""));
	::SetWindowTheme(GetDlgItem(IDC_STATIC_NET_MON)->GetSafeHwnd(), NULL, _T(""));

	m_ctrlListVisited.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_ctrlListVisited.OnInitialize();
	m_ctrlListVisited.InsertColumn(0, lpcRecHeader[0], LVCFMT_FIXED_WIDTH, 30);
	m_ctrlListVisited.InsertColumn(1, lpcRecHeader[1], LVCFMT_LEFT, 100);
	m_ctrlListVisited.InsertColumn(2, lpcRecHeader[2], LVCFMT_LEFT, 400);
	// TODO:  Add extra initialization here

	m_ctrlEditDevicename.SetWindowText(m_csDeviceName);
	m_ctrlEditMACAddress.SetWindowText(m_csMacAddress);
	m_ctrlEditIPAddress.SetWindowText(m_csIPAddress);

	g_pCSaveDeviceInfoDlg = this;
	m_speedObj.m_lfDownloadSpeed = 0;
	m_speedObj.m_lfUploadSpeed = 0;
	m_speedObj.m_ullDownloadSize = 0;
	m_speedObj.m_ullUploadSize = 0;
	m_speedObj.m_ullTimeStarted = GetTickCount64();
	
	m_hThreadStop = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThreadWait = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, SiteVisitedThread, this, 0, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

bool CSaveDeviceInfoDlg::CallbackPacketListener(unsigned char* buffer, int nSize, void* pObject)
{
	g_pCSaveDeviceInfoDlg->DisplaySpeed(buffer,nSize, pObject);
	return 0;
}
CString CSaveDeviceInfoDlg::GetHostName(CString ip)
{
	char hostname[NI_MAXHOST];
	struct addrinfo hints = { 0 }, * addrs;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	const int status = getaddrinfo(CW2A(ip).m_szBuffer, NULL, &hints, &addrs);
	if (status != 0)
	{
		return _T("");
	}
	getnameinfo(addrs->ai_addr, (socklen_t)addrs->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);

	
	CString csHostName(hostname);
	freeaddrinfo(addrs);
	return csHostName;
}
void CSaveDeviceInfoDlg::DisplaySpeed(unsigned char* buffer, int nSize, void* pObj)
{
	CSpeedObj* pDevice = (CSpeedObj*)pObj;
	CClientDC cdc(this);
	CString csFormat;
	CString csTitle = _T("");
	CCustomText cText(14, FW_BOLD);
	cText.SetTextColor(RGB(0, 0, 0));
	int nRow = 220;
	CString sourceIP, destIP;
	int iphdrlen = 0;
	IPV4_HDR* iphdr;

	char sztemp[32];

	memset(sztemp, 0, sizeof(sztemp));
	iphdr = (IPV4_HDR*)buffer;
	iphdrlen = iphdr->ucIPHeaderLen * 4;
	inet_ntop(AF_INET, (const void*)&iphdr->unDestaddress, sztemp, sizeof(sztemp));
	destIP = CA2W(sztemp);
	inet_ntop(AF_INET, (const void*)&iphdr->unSrcaddress, sztemp, sizeof(sztemp));
	sourceIP = CA2W(sztemp);

	ULONGLONG timeCurrent = GetTickCount64();
	if (m_csIPAddress == sourceIP)
		pDevice->m_ullUploadSize += nSize;
	else if(m_csIPAddress == destIP)
		pDevice->m_ullDownloadSize += nSize;

	if ((timeCurrent - pDevice->m_ullTimeStarted) >= POLLING_TIME)
	{
		pDevice->m_lfUploadSpeed = ((double)pDevice->m_ullUploadSize / (double)(timeCurrent - pDevice->m_ullTimeStarted)) * 8;
		pDevice->m_ullUploadSize = 0;

		pDevice->m_lfDownloadSpeed = ((double)pDevice->m_ullDownloadSize / (double)(timeCurrent - pDevice->m_ullTimeStarted)) * 8;
		pDevice->m_ullDownloadSize = 0;
			
		m_ctrlStaticArea.RedrawWindow();
		if (pDevice->m_lfUploadSpeed < 1000)
			csFormat.Format(_T("%.2lf Kbps"), pDevice->m_lfUploadSpeed);
		else
			csFormat.Format(_T("%.2lf Mbps"), pDevice->m_lfUploadSpeed / 1000);

		csTitle = csTitle + _T(" ↑") + csFormat +_T("    ");
		if (pDevice->m_lfDownloadSpeed < 1000)
			csFormat.Format(_T("%.2lf Kbps"), pDevice->m_lfDownloadSpeed);
		else
			csFormat.Format(_T("%.2lf Mbps"), pDevice->m_lfDownloadSpeed / 1000);
		cText.SetTextBKColor(GetSysColor(COLOR_3DFACE));
		cText.SetTextColor(RGB(0, 0, 0));
		
		csTitle = csTitle + _T("↓") + csFormat;
		this->SetWindowText(csTitle);
		cText.DrawCustomText(&cdc, 30, nRow, csTitle);
		pDevice->m_ullTimeStarted = GetTickCount64();

		CString csText;
		map<CString, CString> mSite = (*m_pmSiteVisited)[m_csIPAddress];
		map<CString, CString>::iterator it = mSite.begin();
		while (it != mSite.end())
		{
			csText.AppendFormat(_T("%s : %s\r\n"), it->second, GetHostName(it->first));
			it++;
		}
		m_ctrlEditSiteVisited.SetWindowText(csText);
	}
}
void CSaveDeviceInfoDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CDialogEx::OnPaint() for painting messages

}


void CSaveDeviceInfoDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	//m_fnptrStopPacketListenerEx(m_hPacketListener);
	SetEvent(m_hThreadStop);

	while (::MsgWaitForMultipleObjects(1, &m_hThread, FALSE, INFINITE,
		QS_SENDMESSAGE) == WAIT_OBJECT_0 + 1)
	{
		MSG message;
		::PeekMessage(&message, 0, 0, 0, PM_NOREMOVE);
	}
	CDialogEx::OnClose();
}

void CSaveDeviceInfoDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//m_fnptrStopPacketListenerEx(m_hPacketListener);
	SetEvent(m_hThreadStop);
	while (::MsgWaitForMultipleObjects(1, &m_hThread, FALSE, INFINITE,
		QS_SENDMESSAGE) == WAIT_OBJECT_0 + 1)
	{
		MSG message;
		::PeekMessage(&message, 0, 0, 0, PM_NOREMOVE);
	}
	if (!SaveDeviceName())
		AfxMessageBox(_T("Error in saving device name."));
	else
		CDialogEx::OnOK();
}

bool CSaveDeviceInfoDlg::SaveDeviceName()
{
	CString csDevName, csMacAddress;
	HKEY hKey;
	DWORD dwError = 0;
	bool bRet = false;
	m_fnptrStopPacketListenerEx(m_hPacketListener);
	m_ctrlEditDevicename.GetWindowText(csDevName);
	m_ctrlEditMACAddress.GetWindowText(csMacAddress);
	m_csDeviceName = csDevName;
	dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PATH, 0, KEY_ALL_ACCESS, &hKey);
	if (dwError == ERROR_FILE_NOT_FOUND)
		dwError = RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_PATH, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	else if (dwError == ERROR_ACCESS_DENIED)
		AfxMessageBox(_T("Access denied. Please run the tool as administrator."));


	if (dwError == ERROR_SUCCESS)
	{
		LONG setRes = RegSetValueEx(hKey, csMacAddress, 0, REG_SZ, (LPBYTE)csDevName.GetBuffer(), sizeof(TCHAR) * csDevName.GetLength());

		if (setRes == ERROR_SUCCESS)
			bRet = true;
		else
			bRet = false;
	}
	RegCloseKey(hKey);
	return bRet;
}


void CSaveDeviceInfoDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
//	m_fnptrStopPacketListenerEx(m_hPacketListener);
	SetEvent(m_hThreadStop);
	while (::MsgWaitForMultipleObjects(1, &m_hThread, FALSE, INFINITE,
		QS_SENDMESSAGE) == WAIT_OBJECT_0 + 1)
	{
		MSG message;
		::PeekMessage(&message, 0, 0, 0, PM_NOREMOVE);
	}
	CDialogEx::OnCancel();
}


//afx_msg LRESULT CSaveDeviceInfoDlg::OnSiteVisit(WPARAM wParam, LPARAM lParam)
//{
//	return 0;
//}




BOOL CSaveDeviceInfoDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	//WaitForSingleObject(m_hThreadWait, INFINITE);

	return CDialogEx::DestroyWindow();
}
