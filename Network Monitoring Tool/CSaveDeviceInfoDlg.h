#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "CCustomText.h"
#include "CCustomClock.h"
#include "CDeviceConnected.h"
// CSaveDeviceInfoDlg dialog
#include <mutex>

typedef bool (*FNCallbackPacketListenerEx)(unsigned char* buffer, int nSize, void* pObject);
typedef HANDLE(*FNPTRCreatePacketListenerEx)(FNCallbackPacketListenerEx, void*);
typedef bool (*FNPTRStartPacketListenerEx)(HANDLE);
typedef void (*FNPTRStopPacketListenerEx)(HANDLE);
typedef void (*FNPTRDeletePacketListenerEx)(HANDLE);

class CSpeedObj
{
public:
	double m_lfDownloadSpeed;
	double m_lfUploadSpeed;
	double m_lfMaxDownloadSpeed;
	double m_lfMaxUploadSpeed;
	ULONGLONG m_ullDownloadSize;
	ULONGLONG m_ullUploadSize;
	ULONGLONG m_ullTimeStarted;
};
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
	FNPTRCreatePacketListenerEx m_fnptrCreatePacketListenerEx;
	FNPTRStartPacketListenerEx m_fnptrStartPacketListenerEx;
	FNPTRStopPacketListenerEx m_fnptrStopPacketListenerEx;
	FNPTRDeletePacketListenerEx m_fnptrDeletePacketListenerEx;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	bool SaveDeviceName();
	DECLARE_MESSAGE_MAP()

protected:
	CSpeedObj m_speedObj;

	CEdit m_ctrlEditDevicename;
	CEdit m_ctrlEditMACAddress;
	CIPAddressCtrl m_ctrlEditIPAddress;
	CString m_csIPAddress;
	CString m_csMacAddress;
	CString m_csDeviceName;
	HANDLE m_hPacketListener;
public:
	afx_msg void OnBnClickedOk();
	void DisplaySpeed(unsigned char* buffer, int nSize, void* pObject);
	CString GetDeviceName()
	{
		return m_csDeviceName;
	}
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	static bool CallbackPacketListener(unsigned char* buffer, int nSize, void* pObject);
	afx_msg void OnClose();
	CButton m_ctrlStaticArea;
};

