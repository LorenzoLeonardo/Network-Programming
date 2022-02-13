
// CheckOpenPorstDlg.h : header file
//

#pragma once
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <process.h>
#include <mutex>
#include <vector>
#include "../EnzTCP/EnzTCP.h"
#include <map>
#include <cmath>
#include "CListCtrlCustom.h"


#ifndef _tstring
#ifdef UNICODE
	#define _tstring wstring
#else
	#define _tstring string
#endif
#endif

using namespace std;

#define MAX_PORT 65535

typedef  void(*LPEnumOpenPorts)(const char*, int, FuncFindOpenPort);
typedef  bool(*LPIsPortOpen)(const char*, int, int*);
typedef void (*FNStartLocalAreaListening)(const char* ipAddress, CallbackLocalAreaListener fnpPtr, int nPollingTime);
typedef void (*FNStopLocalAreaListening)();
typedef bool (*FNStartSNMP)(const char* szAgentIPAddress, const char* szCommunity, int nVersion, DWORD & dwLastError);
typedef smiVALUE (*FNSNMPGet)(const char* szOID, DWORD & dwLastError);
typedef void (*FNEndSNMP)();
typedef bool (*FNGetDefaultGateway)(char szDefaultGateway[]);
typedef bool (*FNStopSearchingOpenPorts)();
typedef bool (*FNStartPacketListener)(FNCallbackPacketListener);
typedef void (*FNStopPacketListener)();

inline WCHAR* convert_to_wstring(const char* str);
inline char* convert_from_wstring(const WCHAR* wstr);
inline void GetLastErrorMessageString(_tstring& str, int nGetLastError);
template <typename Map>
inline bool key_compare(Map const& lhs, Map const& rhs);

// CCheckOpenPortsDlg dialog
class CCheckOpenPortsDlg : public CDialogEx
{
// Construction
public:
	CCheckOpenPortsDlg(CWnd* pParent = nullptr);	// standard constructor
	~CCheckOpenPortsDlg();
	FNSNMPGet m_pfnPtrSNMPGet;
	FNEndSNMP m_pfnPtrEndSNMP;
	FNGetDefaultGateway m_pfnPtrGetDefaultGateway;
	FNStartSNMP m_pfnPtrStartSNMP;
	FNStartPacketListener m_pfnPtrStartPacketListener;
	FNStopPacketListener m_pfnPtrStopPacketListener;
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHECKOPENPORST_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	HBRUSH m_hBrushBackGround;
	HBRUSH m_hBrushEditArea;
	LPEnumOpenPorts m_pfnPtrEnumOpenPorts;
	LPIsPortOpen m_pfnPtrIsPortOpen;
	FNStartLocalAreaListening m_pfnPtrStartLocalAreaListening;
	FNStopLocalAreaListening m_pfnPtrStopLocalAreaListening;
	FNStopSearchingOpenPorts m_pfnPtrStopSearchingOpenPorts;

	static void CallbackLANListener(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsopen);
	static void CallBackEnumPort(char* ipAddress, int nPort, bool bIsopen, int nLastError);
	static bool CallPacketListener(char* buffer, int nSize);

	static unsigned __stdcall  RouterThread(void* parg);
	static unsigned __stdcall  SpeedThread(void* parg);

// Implementation
protected:
	HICON m_hIcon;

	bool m_bStopPacketListener;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	bool m_bStopSearchingOpenPorts;
	bool m_bIsRunning;
	vector<CString> m_vList;
	CString m_IPAddress;
	int m_nThread;
	vector<thread*> v_Thread;
	thread* m_tMonitor;
	
	bool m_bHasClickClose;
	CString m_csRouterModel;
	CString m_csRouterBrand;
	CString m_csRouterDescription;
	bool m_bStopLANClicked;
	ULONG m_ulDataSize;

public:
	HMODULE dll_handle;
	int m_nCurrentRowSelected;
	bool HasClickClose()
	{
		return m_bHasClickClose;
	}
	void SetDataSize(ULONG size)
	{
		m_ulDataSize = size;
	}
	ULONG GetDataSize()
	{
		return m_ulDataSize;
	}
	afx_msg void OnBnClickedButtonPort();
	string UnicodeToMultiByte(wstring& wstr);
	HANDLE Handle[MAX_PORT];
	CIPAddressCtrl m_ctrlIPAddress;
	CEdit m_ctrlResult;
	CListCtrlCustom m_ctrlLANConnected;
	map<ULONG, vector<string>> m_mConnected;
	map<ULONG, vector<string>> m_mConnectedBefore;
	vector<thread*> GetHandles()
	{
		return v_Thread;
	}
	void SetThreadRunning(bool b)
	{
		m_bIsRunning = b;
		if(!m_bIsRunning && m_bStopLANClicked)
			::MessageBox(this->GetSafeHwnd(),_T("Listening from Local Area Network has stopped."),_T("Local Area Listener Stopped"), MB_ICONEXCLAMATION);
	}
	void Increment();
	bool IsStopped()
	{
		return m_bStopSearchingOpenPorts;
	}
	bool IsPacketStopped()
	{
		return m_bStopPacketListener;
	}
	bool IsThreadRunning()
	{
		return m_bIsRunning;
	}
	CString GetIPAddress()
	{
		return m_IPAddress;
	}
	afx_msg void OnBnClickedButton2();
	CEdit m_ctrlPortNum;
	afx_msg void OnBnClickedButtonCheckport();
protected:
	CProgressCtrl m_ctrlProgressStatus;
public:
	afx_msg void OnClose();
protected:
	CButton m_ctrlBtnCheckOpenPorts;
	CButton m_ctrlBtnStopSearchingPort;
public:
	afx_msg void OnEnChangeEditArea();

public:
	afx_msg void OnBnClickedButtonListenLan();
	afx_msg void OnBnClickedButtonStopLan();
protected:
	CEdit m_ctrlEditPollingTime;
	HANDLE m_hThreadRouter;
	HANDLE m_hThreadSpeed;
public:
	CButton m_ctrlBtnListen;
	CButton m_ctrlBtnStopListening;
	afx_msg void OnNMClickListLan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHdnItemKeyDownListLan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnKeydownListLan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListLan(NMHDR* pNMHDR, LRESULT* pResult);


	void SetRouterUpTime(CString cs)
	{
		m_ctrlStaticRouterUpTime.SetWindowText(cs);
	}
private:

	CStatic m_ctrlStaticRouterUpTime;
public:
	CEdit m_ctrlEditPacketReport;
	afx_msg void OnBnClickedButtonStartPacket();
	afx_msg void OnBnClickedButtonStopPacket();
	CEdit m_ctrlEditSpeed;
};
