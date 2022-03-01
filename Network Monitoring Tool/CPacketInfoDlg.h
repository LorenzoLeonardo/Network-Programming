#pragma once
#include "afxdialogex.h"
#include "resource.h"

// CPacketInfoDlg dialog

class CPacketInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPacketInfoDlg)

public:
	CPacketInfoDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CPacketInfoDlg();


	enum { IDD = IDD_PACKETINFO_DIALOG };

	CWnd* m_pParent;
	afx_msg void OnBnClickedCancel();
	void UpdatePacketInfo(CString, int nProtocol);
	void SetPacketInfo(CString csInfo)
	{
		m_csPacketInfo = csInfo;
	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual void PostNcDestroy();
	CEdit m_ctrlEditPacketReportArea;
	CString m_csPacketInfo;
	HANDLE m_hPacketInfoThread;
	HBRUSH 	m_hBrushBackGround;
	HBRUSH	m_hBrushEditArea;
	bool m_bThisObjDeleted;
	virtual BOOL OnInitDialog();
	static unsigned __stdcall  PacketInfoThread(void* parg);
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
