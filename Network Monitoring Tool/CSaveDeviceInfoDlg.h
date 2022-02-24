#pragma once
#include "afxdialogex.h"
#include "resource.h"

// CSaveDeviceInfoDlg dialog

class CSaveDeviceInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSaveDeviceInfoDlg)

public:
	CSaveDeviceInfoDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSaveDeviceInfoDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DEVICE_INFO };
#endif
	void SetInformation(CString ip, CString mac, CString dev)
	{
		m_csIPAddress = ip;
		m_csMacAddress = mac;
		m_csDeviceName = dev;
	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	bool SaveDeviceName();
	DECLARE_MESSAGE_MAP()

protected:
	CEdit m_ctrlEditDevicename;
	CEdit m_ctrlEditMACAddress;
	CIPAddressCtrl m_ctrlEditIPAddress;
	CString m_csIPAddress;
	CString m_csMacAddress;
	CString m_csDeviceName;
public:
	afx_msg void OnBnClickedOk();
	CString GetDeviceName()
	{
		return m_csDeviceName;
	}
	virtual BOOL OnInitDialog();
};

