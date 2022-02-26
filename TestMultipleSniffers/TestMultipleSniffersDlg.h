
// TestMultipleSniffersDlg.h : header file
//

#pragma once

#include "../EnzTCP/EnzTCP.h"
#include "CDeviceConnected.h"
#include <string>

using namespace std;

typedef HANDLE(*FNPTRCreatePacketListenerEx)(FNCallbackPacketListenerEx, void*);
typedef bool (*FNPTRStartPacketListenerEx)(HANDLE);
typedef void (*FNPTRStopPacketListenerEx)(HANDLE);
typedef void (*FNPTRDeletePacketListenerEx)(HANDLE);

// CTestMultipleSniffersDlg dialog
class CTestMultipleSniffersDlg : public CDialogEx
{
// Construction
public:
	CTestMultipleSniffersDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTMULTIPLESNIFFERS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	FNPTRCreatePacketListenerEx m_fnptrCreatePacketListenerEx;
	FNPTRStartPacketListenerEx m_fnptrStartPacketListenerEx;
	FNPTRStopPacketListenerEx m_fnptrStopPacketListenerEx;
	FNPTRDeletePacketListenerEx m_fnptrDeletePacketListenerEx;
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	bool m_bStart;
public:
	CEdit m_ctrlEdit1;
	CEdit m_ctrlEdit2;
	CButton m_ctrlBtnStartStopSniff;
	CIPAddressCtrl m_ctrlIPAddress;
	HMODULE m_hModuleDLL;
	HANDLE m_hPacketHandle1;
	HANDLE m_hPacketHandle2;


	static bool CallbackPacketListener(unsigned char* buffer, int nSize, void* pObject);
	

	afx_msg void OnBnClickedButtonStartStop();
	afx_msg void OnClose();
};
