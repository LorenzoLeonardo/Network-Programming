// CSaveDeviceInfoDlg.cpp : implementation file
//

#include "pch.h"
#include "afxdialogex.h"
#include "CSaveDeviceInfoDlg.h"
#include <ws2tcpip.h>
typedef struct _tIPV4HDR
{
	UCHAR ucIPHeaderLen : 4;
	UCHAR ucIPVersion : 4;
	UCHAR ucIPTos;
	USHORT usIPTotalLength;
	USHORT usIPID;
	UCHAR ucIPFragOffset : 5;
	UCHAR ucIPMoreFragment : 1;
	UCHAR ucIPDontFragment : 1;
	UCHAR ucIPReservedZero : 1;
	UCHAR ucIPFragOffset1;
	UCHAR ucIPTTL;
	UCHAR ucIPProtocol;
	USHORT usIPChecksum;
	UINT unSrcaddress;
	UINT unDestaddress;
} IPV4_HDR;

CSaveDeviceInfoDlg* g_pCSaveDeviceInfoDlg = NULL;
// CSaveDeviceInfoDlg dialog

IMPLEMENT_DYNAMIC(CSaveDeviceInfoDlg, CDialogEx)

CSaveDeviceInfoDlg::CSaveDeviceInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DEVICE_INFO, pParent)
{

}

CSaveDeviceInfoDlg::~CSaveDeviceInfoDlg()
{
	m_fnptrDeletePacketListenerEx(m_hPacketListener);
}

void CSaveDeviceInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DEVICE_NAME, m_ctrlEditDevicename);
	DDX_Control(pDX, IDC_EDIT_MAC, m_ctrlEditMACAddress);
	DDX_Control(pDX, IDC_IPADDRESS_IPINFO, m_ctrlEditIPAddress);
	DDX_Control(pDX, IDC_STATIC_DEVAREA, m_ctrlStaticArea);
}


BEGIN_MESSAGE_MAP(CSaveDeviceInfoDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSaveDeviceInfoDlg::OnBnClickedOk)
	ON_WM_PAINT()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CSaveDeviceInfoDlg message handlers



BOOL CSaveDeviceInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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
	m_hPacketListener = m_fnptrCreatePacketListenerEx(CallbackPacketListener, &m_speedObj);
	if (!m_fnptrStartPacketListenerEx(m_hPacketListener))
	{
		m_fnptrDeletePacketListenerEx(m_hPacketListener);
		OnClose();
		return FALSE;
	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

bool CSaveDeviceInfoDlg::CallbackPacketListener(unsigned char* buffer, int nSize, void* pObject)
{
	g_pCSaveDeviceInfoDlg->DisplaySpeed(buffer,nSize, pObject);
	return 0;
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
	if(m_csIPAddress == destIP)
		pDevice->m_ullDownloadSize += nSize;

	if ((timeCurrent - pDevice->m_ullTimeStarted) >= 500)
	{
		pDevice->m_lfUploadSpeed = ((double)pDevice->m_ullUploadSize / (double)(timeCurrent - pDevice->m_ullTimeStarted)) * 8;
		pDevice->m_ullUploadSize = 0;

		pDevice->m_lfDownloadSpeed = ((double)pDevice->m_ullDownloadSize / (double)(timeCurrent - pDevice->m_ullTimeStarted)) * 8;
		pDevice->m_ullDownloadSize = 0;
	
		pDevice->m_ullTimeStarted = GetTickCount64();
		m_ctrlStaticArea.RedrawWindow();
		if (pDevice->m_lfUploadSpeed < 1000)
			csFormat.Format(_T("%.2lf Kbps"), pDevice->m_lfUploadSpeed);
		else
			csFormat.Format(_T("%.2lf Mbps"), pDevice->m_lfUploadSpeed / 1000);
		//cText.SetTextBKColor(GetSysColor(COLOR_3DFACE));
		//cText.SetTextColor(RGB(0, 0, 0));
	//	cText.DrawCustomText(&cdc, 20, 20+ nRow, csFormat);

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
	m_fnptrStopPacketListenerEx(m_hPacketListener);
	CDialogEx::OnClose();
}

void CSaveDeviceInfoDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	m_fnptrStopPacketListenerEx(m_hPacketListener);
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
			return true;
		else
			return false;
	}
	return false;
}
