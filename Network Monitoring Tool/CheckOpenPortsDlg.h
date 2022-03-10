
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
#include "CFirewall.h"

#ifndef _tstring
#ifdef UNICODE
	#define _tstring wstring
#else
	#define _tstring string
#endif
#endif

using namespace std;


#define WM_RESET_CONNECTION WM_USER + 1

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
typedef void (*FNPTRWaitPacketListenerEx)(HANDLE);
typedef void (*FNPTRDeletePacketListenerEx)(HANDLE);
typedef HANDLE (*FNPTRCreateLocalAreaListenerEx)();
typedef	bool (*FNPTRStartLocalAreaListenerEx)(HANDLE, const char* ipAddress, const char* subNetMask, CallbackLocalAreaListener fnpPtr, int nPollingTimeMS);
typedef void (*FNPTRStopLocalAreaListenerEx)(HANDLE);
typedef void (*FNPTRDeleteLocalAreaListenerEx)(HANDLE);
typedef void (*FNPTRSetNICAdapterToUse)(const char* szAdapterName, ULONG ipAddress);

template <typename Map>
inline bool key_compare(Map const& lhs, Map const& rhs);

class CNetworkInterfaceInfo
{
public:
	IP_ADAPTER_INFO AdapterInfo;
	HANDLE hLANListener;
};

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
	LPEnumOpenPorts m_pfnPtrEnumOpenPorts;
	LPIsPortOpen m_pfnPtrIsPortOpen;
	FNStopSearchingOpenPorts m_pfnPtrStopSearchingOpenPorts;
	FNGetNetworkDeviceStatus m_pfnPtrGetNetworkDeviceStatus;
	FNEnumNetworkAdapters m_pfnPtrEnumNetworkAdapters;
	FNGetDefaultGatewayEx m_pfnPtrGetDefaultGatewayEx;
	FNPTRCreatePacketListenerEx m_fnptrCreatePacketListenerEx;
	FNPTRStartPacketListenerEx m_fnptrStartPacketListenerEx;
	FNPTRStopPacketListenerEx m_fnptrStopPacketListenerEx;
	FNPTRWaitPacketListenerEx m_fnptrWaitPacketListenerEx;
	FNPTRDeletePacketListenerEx m_fnptrDeletePacketListenerEx;
	FNPTRCreateLocalAreaListenerEx m_fnptrCreateLocalAreaListenerEx;
	FNPTRStartLocalAreaListenerEx m_fnptrStartLocalAreaListenerEx;
	FNPTRStopLocalAreaListenerEx m_fnptrStopLocalAreaListenerEx;
	FNPTRDeleteLocalAreaListenerEx m_fnptrDeleteLocalAreaListenerEx;
	FNPTRSetNICAdapterToUse m_fnptrSetNICAdapterToUse;
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
	CIPAddressCtrl m_ctrlIPAddress;
	CEdit m_ctrlResult;
	CListCtrlCustom m_ctrlLANConnected;
	map<ULONG, CDeviceConnected*> m_mConnected;
	map<ULONG, int> m_mMonitorDeviceCurrent;
	map<ULONG, int> m_mMonitorDeviceBefore;
	vector<CNetworkInterfaceInfo> m_vAdapterInfo;
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
	bool m_bInitNIC;
	int m_nCurrentNICSelect;
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
	void UpdateDeviceConnected();
	void EnableCloseButton(bool bEnable);
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
	bool ShowPacketInfo()
	{
		return m_bShowPacketInfo;
	}
	DWORD GetIPFilterULONG()
	{
		return m_ulIPFilter;
	}
	void InitAdapterUI();
	void UpdateAdapterChanges();

	bool IsInternetOnly()
	{
		return m_bIsInternetOnly;
	}
protected:
	volatile bool m_bIsInternetOnly;
	HBRUSH m_hBrushBackGround;
	HBRUSH m_hBrushEditArea;
	HICON m_hIcon;
	bool m_bStopPacketListener;
	bool m_bStopSearchingOpenPorts;
	bool m_bLanStop;
	CString m_IPAddress;
	int m_nThread;
	volatile bool m_bHasClickClose;
	CString m_csRouterModel;
	CString m_csRouterBrand;
	CString m_csRouterDescription;
	CEdit m_ctrlEditPollingTime;

	HANDLE m_hThreadRouter;
	HANDLE m_hThreadLANListener;
	HANDLE m_hNICPacketListener;
	HANDLE m_hThreadClock;
	HANDLE m_hThreadOpenPortListener;
	HANDLE m_hThreadNICListener;
	HANDLE m_hWaitEvent;
	HANDLE m_hThisMainThread;
	CButton m_ctrlBtnCheckOpenPorts;
	CButton m_ctrlBtnStopSearchingPort;
	CProgressCtrl m_ctrlProgressStatus;
	CStatic m_ctrlStaticRouterUpTime;
	volatile bool m_bShowPacketInfo;
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
	afx_msg void OnBnClickedButtonStartListenLan();
	afx_msg void OnBnClickedButtonStopListenLan();
	afx_msg void OnNMClickListLan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnHdnItemKeyDownListLan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnKeydownListLan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkListLan(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonStartPacket();
	afx_msg void OnBnClickedButtonStopPacket();
	afx_msg void OnBnClickedButtonShowPackets();
	DECLARE_MESSAGE_MAP()

	static void CallbackLANListenerEx(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsopen);
	void ProcessLANListener(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsopen);
	static void CallBackEnumPort(char* ipAddress, int nPort, bool bIsopen, int nLastError);
	static bool CallbackNICPacketListener(unsigned char* buffer, int nSize, void* obj);
	bool ProcessNICPacketListener(unsigned char* buffer, int nSize, void* obj);
	static void CallBackEnumAdapters(void*);
	static bool CallbackPacketListenerDownloadEx(unsigned char* buffer, int nSize, void* pObject);
	bool ProcessPacketListenerDownloadEx(unsigned char* buffer, int nSize, void* pObject);
	static bool CallbackPacketListenerUploadEx(unsigned char* buffer, int nSize, void* pObject);
	bool ProcessPacketListenerUploadEx(unsigned char* buffer, int nSize, void* pObject);
	static unsigned __stdcall  RouterThread(void* parg);
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
	void PumpWaitingMessages();
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
	CStatic m_ctrlStaticNICListen;
	afx_msg void OnBnClickedCheckInternetOnly();
	CStatic m_ctrlStaticRouterImage;
	CStatic m_ctrlStaticNumDevice;
	virtual void OnCancel();
	virtual void OnOK();
};
