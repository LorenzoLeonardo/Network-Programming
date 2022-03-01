
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
#include "CPacketInfoDlg.h"
#include "CDeviceConnected.h"
#include "CCustomClock.h"

#ifndef _tstring
#ifdef UNICODE
	#define _tstring wstring
#else
	#define _tstring string
#endif
#endif

using namespace std;

#define MAX_PORT 65535
#define POLLING_TIME 1000
#define WM_CLEAR_TREADS WM_USER + 1

typedef  void(*LPEnumOpenPorts)(const char*, int, FuncFindOpenPort);
typedef  bool(*LPIsPortOpen)(const char*, int, int*);
typedef bool (*FNStartLocalAreaListening)(const char* ipAddress, const char* subnetMask, CallbackLocalAreaListener fnpPtr, int nPollingTime);
typedef void (*FNStopLocalAreaListening)();
typedef bool (*FNStartSNMP)(const char* szAgentIPAddress, const char* szCommunity, int nVersion, DWORD & dwLastError);
typedef smiVALUE (*FNSNMPGet)(const char* szOID, DWORD & dwLastError);
typedef void (*FNEndSNMP)();
typedef bool (*FNGetDefaultGateway)(char szDefaultGateway[]);
typedef bool (*FNGetDefaultGatewayEx)(const char* szAdapterName, char* szDefaultGateway, int nSize);
typedef bool (*FNStopSearchingOpenPorts)();
typedef bool (*FNStartPacketListener)(FNCallbackPacketListener);
typedef void (*FNStopPacketListener)();
typedef bool (*FNGetNetworkDeviceStatus)(const char* ipAddress, char* hostname, int nSizeHostName, char* macAddress, int nSizeMacAddress, DWORD * pError);
typedef bool (*FNEnumNetworkAdapters)(FuncAdapterList);
typedef HANDLE(*FNPTRCreatePacketListenerEx)(FNCallbackPacketListenerEx, void*);
typedef bool (*FNPTRStartPacketListenerEx)(HANDLE);
typedef void (*FNPTRStopPacketListenerEx)(HANDLE);
typedef void (*FNPTRDeletePacketListenerEx)(HANDLE);

inline void GetLastErrorMessageString(_tstring& str, int nGetLastError);
template <typename Map>
inline bool key_compare(Map const& lhs, Map const& rhs);

typedef struct _tOBJ
{
	ULONG m_ulDataSizeDownload;
	ULONG m_ulDataSizeUpload;
	double m_lfDownloadSpeed;
	double m_lfUploadSpeed;
	double m_lfMaxDownloadSpeed;
	double m_lfMaxUploadSpeed;
	vector<CString> m_vIPHOSTMAC;//IP, Host, MAC
	HANDLE m_hThreadDownload;
	HANDLE m_hThreadUpload;
	HANDLE m_eventDownload;
	HANDLE m_eventUpload;
}ENZ_CONNECTED_DEVICE_DETAILS;



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
	LPEnumOpenPorts m_pfnPtrEnumOpenPorts;
	LPIsPortOpen m_pfnPtrIsPortOpen;
	FNStartLocalAreaListening m_pfnPtrStartLocalAreaListening;
	FNStopLocalAreaListening m_pfnPtrStopLocalAreaListening;
	FNStopSearchingOpenPorts m_pfnPtrStopSearchingOpenPorts;
	FNGetNetworkDeviceStatus m_pfnPtrGetNetworkDeviceStatus;
	FNEnumNetworkAdapters m_pfnPtrEnumNetworkAdapters;
	FNGetDefaultGatewayEx m_pfnPtrGetDefaultGatewayEx;
	FNPTRCreatePacketListenerEx m_fnptrCreatePacketListenerEx;
	FNPTRStartPacketListenerEx m_fnptrStartPacketListenerEx;
	FNPTRStopPacketListenerEx m_fnptrStopPacketListenerEx;
	FNPTRDeletePacketListenerEx m_fnptrDeletePacketListenerEx;
	CPacketInfoDlg* m_pmodeless;
// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHECKOPENPORST_DIALOG };
#endif
	enum
	{
		COL_NUMBER =0,
		COL_IPADDRESS,
		COL_DEVICENAME,
		COL_MACADDRESS,
		COL_DOWNLOADSPEED,
		COL_UPLOADSPEED,
		COL_DOWNLOADMAXSPEED,
		COL_UPLOADMAXSPEED
	};
	HMODULE m_hDLLhandle;
	int m_nCurrentRowSelected;
	HANDLE Handle[MAX_PORT];
	CIPAddressCtrl m_ctrlIPAddress;
	CEdit m_ctrlResult;
	CListCtrlCustom m_ctrlLANConnected;
	map<ULONG, CDeviceConnected*> m_mConnected;
	map<ULONG, CDeviceConnected> m_mConnectedBefore;
	vector<IP_ADAPTER_INFO> m_vAdapterInfo;
	CEdit m_ctrlEditPacketReportArea;
	CEdit m_ctrlEditDownloadSpeed;
	CEdit m_ctrlEditUploadSpeed;
	CEdit m_ctrlEditPacketReportUpload;
	CButton m_ctrlBtnShowPacketInfo;
	CButton m_ctrlBtnListenPackets;
	CButton m_ctrlBtnUnlistenPackets;
	CString m_ipFilter;
	ULONG   m_ulIPFilter;
	CEdit m_ctrlPortNum;
	CButton m_ctrlBtnListen;
	CButton m_ctrlBtnStopListening;
	CCustomClock m_customClock;
	RECT m_rectModeless;
	
	inline string UnicodeToMultiByte(wstring& wstr);
	inline wstring MultiByteToUnicode(string& wstr);
	void Increment();
	int IsInTheList(CString csIPAddress);
	void UpdateClock();
	inline void DisplayUploadSpeed(CString szIPAddress, int nColumn, double ldData);
	inline void DisplayDownloadSpeed(CString szIPAddress, int nColumn, double ldData);
	bool HasClickClose()
	{
		return m_bHasClickClose;
	}
	void SetDownloadSize(ULONG size)
	{
		m_ulDataSizeDownload = size;
	}

	void SetDownloadSize(ULONG ulIPAdd, ULONG size)
	{
		
		if (!m_mConnectedBefore.empty())
		{
			if (m_mConnectedBefore.find(ulIPAdd) != m_mConnectedBefore.end())
				m_mConnectedBefore[ulIPAdd].m_ulDataSizeDownload = size;
		}
	
	}

	void SetUploadSize(ULONG size)
	{
		m_ulDataSizeUpload = size;
	}

	void SetUploadSize(ULONG ulIPAdd, ULONG size)
	{
		
		if (!m_mConnectedBefore.empty())
		{
			if(m_mConnectedBefore.find(ulIPAdd)!= m_mConnectedBefore.end())
				m_mConnectedBefore[ulIPAdd].m_ulDataSizeUpload = size;
		}
		
	}
	ULONG GetDownloadSize()
	{
		return m_ulDataSizeDownload;
	}
	ULONG GetDownloadSize(ULONG ulIPAdd)
	{
		
		ULONG ulSize = 0;
		if (!m_mConnectedBefore.empty())
		{
			if (m_mConnectedBefore.find(ulIPAdd) != m_mConnectedBefore.end())
				ulSize = m_mConnectedBefore[ulIPAdd].m_ulDataSizeDownload;
		}
		
		return ulSize;
	}
	ULONG GetUploadSize()
	{
		return m_ulDataSizeUpload;
	}
	ULONG GetUploadSize(ULONG ulIPAdd)
	{
		
		ULONG ulSize = 0;
		if (!m_mConnectedBefore.empty())
		{
			if (m_mConnectedBefore.find(ulIPAdd) != m_mConnectedBefore.end())
				ulSize = m_mConnectedBefore[ulIPAdd].m_ulDataSizeUpload;
		}
		
		return ulSize;
	}
	vector<thread*> GetHandles()
	{
		return v_Thread;
	}
	void SetLANStop(bool b)
	{
		m_bLanStop = b;
	}
	void SetStopSearchingOpenPort()
	{
		m_bStopSearchingOpenPorts = true;
	}
	bool IsSearchingOpenPortStopped()
	{
		return m_bStopSearchingOpenPorts;
	}
	bool IsPacketStopped()
	{
		return m_bStopPacketListener;
	}
	bool IsLANStopped()
	{
		return m_bLanStop;
	}
	CString GetIPAddress()
	{
		return m_IPAddress;
	}
	void SetRouterUpTime(CString cs)
	{
		m_ctrlStaticRouterUpTime.SetWindowText(cs);
	}
	void SetDownloadSpeedText(CString cs)
	{
		m_ctrlEditDownloadSpeed.SetWindowText(cs);
	}
	void SetUploadSpeedText(CString cs)
	{
		m_ctrlEditUploadSpeed.SetWindowText(cs);
	}
	bool ShowPacketInfo()
	{
		return m_bShowPacketInfo;
	}
	CString GetIPFilterString()
	{
		return m_ipFilter;
	}
	DWORD GetIPFilterULONG()
	{
		return m_ulIPFilter;
	}
	void InitAdapterUI();
	void EndProgram();
protected:
	HBRUSH m_hBrushBackGround;
	HBRUSH m_hBrushEditArea;
	HICON m_hIcon;
	bool m_bStopPacketListener;
	bool m_bStopSearchingOpenPorts;
	bool m_bLanStop;
	CString m_IPAddress;
	int m_nThread;
	vector<thread*> v_Thread;
	thread* m_tMonitor;
	bool m_bHasClickClose;
	CString m_csRouterModel;
	CString m_csRouterBrand;
	CString m_csRouterDescription;
	ULONG m_ulDataSizeDownload;
	ULONG m_ulDataSizeUpload;
	CEdit m_ctrlEditPollingTime;

	HANDLE m_hThreadRouter;
	HANDLE m_hThreadDownloadSpeedList;
	HANDLE m_hThreadUploadSpeedList;
	HANDLE m_hThreadLANListener;
	HANDLE m_hThreadPacketListener;
	HANDLE m_hThreadClock;
	HANDLE m_hThreadOpenPortListener;
	HANDLE m_hThreadNICListener;

	CButton m_ctrlBtnCheckOpenPorts;
	CButton m_ctrlBtnStopSearchingPort;
	CProgressCtrl m_ctrlProgressStatus;
	CStatic m_ctrlStaticRouterUpTime;
	bool m_bShowPacketInfo;
	bool m_bOnCloseWasCalled;

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonStopSearchingOpenPorts();
	afx_msg void OnBnClickedButtonCheckIfPortOpen();
	afx_msg void OnBnClickedButtonStartSearchingOpenPort();
	afx_msg void OnClose();
	afx_msg void OnEnChangeEditArea();
	afx_msg void OnBnClickedButtonListenLan();
	afx_msg void OnBnClickedButtonStopLan();
	afx_msg void OnNMClickListLan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHdnItemKeyDownListLan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnKeydownListLan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListLan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonStartPacket();
	afx_msg void OnBnClickedButtonStopPacket();
	afx_msg void OnBnClickedButtonShowPackets();
	afx_msg LRESULT OnClearThreads(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	static void CallbackLANListener(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsopen);
	static void CallbackLANListenerEx(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsopen);
	static void CallBackEnumPort(char* ipAddress, int nPort, bool bIsopen, int nLastError);
	static bool CallPacketListener(unsigned char* buffer, int nSize);
	static void CallBackEnumAdapters(void*);
	static bool CallbackPacketListenerDownloadEx(unsigned char* buffer, int nSize, void* pObject);
	static bool CallbackPacketListenerUploadEx(unsigned char* buffer, int nSize, void* pObject);
	static unsigned __stdcall  RouterThread(void* parg);
	static unsigned __stdcall  DownloadSpeedThreadList(void* parg);
	static unsigned __stdcall  UploadSpeedThreadList(void* parg);
	static unsigned __stdcall  LANListenerThread(void* parg);
	static unsigned __stdcall  PacketListenerThread(void* parg);
	static unsigned __stdcall  ClockThread(void* parg);
	static unsigned __stdcall  OpenPortListenerThread(void* parg);
	static unsigned __stdcall  NICListenerThread(void* parg);
	// Generated message map functions
	virtual BOOL OnInitDialog();
	bool InitDLL();
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	CStatic m_ctrlStaticLogo;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnBnClickedCheckDebug();
protected:
	CButton m_ctrlBtnDebug;
public:
	afx_msg void OnCbnSelchangeComboListAdapter();
protected:
	CEdit m_ctrlEditAdapterInfo;
	CComboBox m_ctrlComboAdapterList;
public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
};
