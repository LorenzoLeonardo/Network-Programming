// CSaveDeviceInfoDlg.cpp : implementation file
//

#include "pch.h"
#include "afxdialogex.h"
#include "CSaveDeviceInfoDlg.h"


// CSaveDeviceInfoDlg dialog

IMPLEMENT_DYNAMIC(CSaveDeviceInfoDlg, CDialogEx)

CSaveDeviceInfoDlg::CSaveDeviceInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DEVICE_INFO, pParent)
{

}

CSaveDeviceInfoDlg::~CSaveDeviceInfoDlg()
{
}

void CSaveDeviceInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DEVICE_NAME, m_ctrlEditDevicename);
	DDX_Control(pDX, IDC_EDIT_MAC, m_ctrlEditMACAddress);
	DDX_Control(pDX, IDC_IPADDRESS_IPINFO, m_ctrlEditIPAddress);
}


BEGIN_MESSAGE_MAP(CSaveDeviceInfoDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSaveDeviceInfoDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSaveDeviceInfoDlg message handlers


void CSaveDeviceInfoDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	if (!SaveDeviceName())
		AfxMessageBox(_T("Error in saving device name."));
	else
		CDialogEx::OnOK();
}

bool CSaveDeviceInfoDlg::SaveDeviceName()
{
	CString csDevName,csMacAddress;
	HKEY hKey;
	DWORD dwError = 0;

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
		LONG setRes = RegSetValueEx(hKey, csMacAddress, 0, REG_SZ, (LPBYTE)csDevName.GetBuffer(), sizeof(TCHAR)* csDevName.GetLength());

		if (setRes == ERROR_SUCCESS)
			return true;
		else
			return false;
	}
	return false;
}

BOOL CSaveDeviceInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	m_ctrlEditDevicename.SetWindowText(m_csDeviceName);
	m_ctrlEditMACAddress.SetWindowText(m_csMacAddress);
	m_ctrlEditIPAddress.SetWindowText(m_csIPAddress);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
