
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
	afx_msg void OnCbnSelchangeComboListAdapter();
	afx_msg void OnBnClickedCheckInternetOnly();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnBnClickedCheckDebug();
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

	inline string UnicodeToMultiByte(wstring& wstr);
	inline wstring MultiByteToUnicode(string& wstr);
	inline void DisplayUploadSpeed(CString szIPAddress, int nColumn, double ldData);
	inline void DisplayDownloadSpeed(CString szIPAddress, int nColumn, double ldData);

	void Increment();
	int IsInTheList(CString csIPAddress);
	void UpdateClock();
	void UpdateDeviceConnected();
	void EnableCloseButton(bool bEnable);
	bool InitDLL();
	void InitAdapterUI();
	void UpdateAdapterChanges();

	bool HasClickClose()
	{
		return m_bHasClickClose;
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
	bool ShowPacketInfo()
	{
		return m_bShowPacketInfo;
	}
	DWORD GetIPFilterULONG()
	{
		return m_ulIPFilter;
	}


	bool IsInternetOnly()
	{
		return m_bIsInternetOnly;
	}
protected:
	volatile bool m_bIsInternetOnly;
	volatile bool m_bHasClickClose;
	volatile bool m_bShowPacketInfo;
	bool m_bStopPacketListener;
	bool m_bStopSearchingOpenPorts;
	bool m_bLanStop;
	bool m_bOnCloseWasCalled;
	bool m_bInitNIC;

	HBRUSH m_hBrushBackGround;
	HBRUSH m_hBrushEditArea;

	HICON m_hIcon;

	CString m_IPAddress;
	CString m_csRouterModel;
	CString m_csRouterBrand;
	CString m_csRouterDescription;
	CString m_ipFilter;

	int m_nThread;
	int m_nCurrentRowSelected;
	int m_nCurrentNICSelect;

	HMODULE m_hDLLhandle;
	
	map<ULONG, CDeviceConnected*> m_mConnected;
	map<ULONG, int> m_mMonitorDeviceCurrent;
	map<ULONG, int> m_mMonitorDeviceBefore;

	vector<CNetworkInterfaceInfo> m_vAdapterInfo;

	ULONG   m_ulIPFilter;

	RECT m_rectModeless;
	
	HANDLE m_hThreadRouter;
	HANDLE m_hThreadLANListener;
	HANDLE m_hNICPacketListener;
	HANDLE m_hThreadClock;
	HANDLE m_hThreadOpenPortListener;
	HANDLE m_hThreadNICListener;

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

	static void __stdcall CallbackLANListenerEx(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsopen);
	static void __stdcall CallBackEnumPort(char* ipAddress, int nPort, bool bIsopen, int nLastError);
	static bool __stdcall CallbackNICPacketListener(unsigned char* buffer, int nSize, void* obj);
	static void __stdcall CallBackEnumAdapters(void*);
	static bool __stdcall CallbackPacketListenerDownloadEx(unsigned char* buffer, int nSize, void* pObject);
	static bool __stdcall CallbackPacketListenerUploadEx(unsigned char* buffer, int nSize, void* pObject);
	void ProcessLANListener(const char* ipAddress, const char* hostName, const char* macAddress, bool bIsopen);
	bool ProcessPacketListenerDownloadEx(unsigned char* buffer, int nSize, void* pObject);
	bool ProcessNICPacketListener(unsigned char* buffer, int nSize, void* obj);
	bool ProcessPacketListenerUploadEx(unsigned char* buffer, int nSize, void* pObject);

	static unsigned __stdcall  RouterThread(void* parg);
	static unsigned __stdcall  ClockThread(void* parg);
	static unsigned __stdcall  OpenPortListenerThread(void* parg);
	static unsigned __stdcall  NICListenerThread(void* parg);
	// Generated message map functions


	CButton m_ctrlBtnDebug;
	CButton m_ctrlBtnShowPacketInfo;
	CButton m_ctrlBtnListenPackets;
	CButton m_ctrlBtnUnlistenPackets;
	CButton m_ctrlBtnCheckOpenPorts;
	CButton m_ctrlBtnStopSearchingPort;
	CButton m_ctrlBtnListen;
	CButton m_ctrlBtnStopListening;

	CStatic m_ctrlStaticRouterUpTime;
	CStatic m_ctrlStaticNICListen;
	CStatic m_ctrlStaticRouterImage;
	CStatic m_ctrlStaticNumDevice;
	CStatic m_ctrlStaticLogo;

	CEdit m_ctrlResult;
	CEdit m_ctrlEditAdapterInfo;
	CEdit m_ctrlEditPollingTime;
	CEdit m_ctrlEditPacketReportArea;
	CEdit m_ctrlEditDownloadSpeed;
	CEdit m_ctrlEditUploadSpeed;
	CEdit m_ctrlEditPacketReportUpload;
	CEdit m_ctrlPortNum;

	CComboBox m_ctrlComboAdapterList;
	
	CProgressCtrl m_ctrlProgressStatus;

	CIPAddressCtrl m_ctrlIPAddress;
	
	CListCtrlCustom m_ctrlLANConnected;

	CCustomClock m_customClock;
};
