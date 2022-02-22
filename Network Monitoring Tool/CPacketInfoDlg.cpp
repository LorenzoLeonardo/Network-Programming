// CPacketInfoDlg.cpp : implementation file
//

#include "pch.h"
#include "afxdialogex.h"
#include "CPacketInfoDlg.h"
#include "CheckOpenPortsDlg.h"

// CPacketInfoDlg dialog

IMPLEMENT_DYNAMIC(CPacketInfoDlg, CDialogEx)
mutex g_mtx;

CPacketInfoDlg::CPacketInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CPacketInfoDlg::IDD, pParent)
{
	m_pParent = pParent;
}

CPacketInfoDlg::~CPacketInfoDlg()
{
}

void CPacketInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PACKET_INFO, m_ctrlEditPacketReportArea);
}


BEGIN_MESSAGE_MAP(CPacketInfoDlg, CDialogEx)
END_MESSAGE_MAP()


// CPacketInfoDlg message handlers


void CPacketInfoDlg::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	CDialog::PostNcDestroy();
	if (m_pParent)
	{
		((CCheckOpenPortsDlg*)m_pParent)->m_pmodeless = NULL;
	}
	//CloseHandle(m_hPacketInfoThread);
	delete this;
//	CDialogEx::PostNcDestroy();
}

void CPacketInfoDlg::OnBnClickedCancel()
{
	DestroyWindow();
}

void CPacketInfoDlg::UpdatePacketInfo(CString csPacketInfo)
{
	if (m_ctrlEditPacketReportArea.m_hWnd)
	{
		CString csText;
		if (m_ctrlEditPacketReportArea.m_hWnd)
			m_ctrlEditPacketReportArea.GetWindowText(csText);
		csText += csPacketInfo;
		long nLength = csText.GetLength();
		if (nLength < 5000)
		{
			if (m_ctrlEditPacketReportArea.m_hWnd)
				m_ctrlEditPacketReportArea.SetSel(0, 0);
			if (m_ctrlEditPacketReportArea.m_hWnd)
				m_ctrlEditPacketReportArea.ReplaceSel(csPacketInfo);
		}
		else
		{
			if (m_ctrlEditPacketReportArea.m_hWnd)
				m_ctrlEditPacketReportArea.GetWindowText(csText);
			csText = csText.Left(csText.ReverseFind(_T('\r')));
			if (m_ctrlEditPacketReportArea.m_hWnd)
				m_ctrlEditPacketReportArea.SetWindowText(csText);
		}
	}
}

BOOL CPacketInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	RECT rect, thisRect;
	m_pParent->GetWindowRect(&rect);
	GetClientRect(&thisRect);
	MoveWindow(rect.right-8,rect.top, thisRect.right, thisRect.bottom);


	m_bThisObjDeleted = false;
	// TODO:  Add extra initialization here
	//m_hPacketInfoThread = (HANDLE)_beginthreadex(NULL, 0, PacketInfoThread, this, 0, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

unsigned __stdcall  CPacketInfoDlg::PacketInfoThread(void* parg)
{
	CPacketInfoDlg* pDlg = (CPacketInfoDlg*)parg;
	while (pDlg)
	{
		pDlg->UpdatePacketInfo(_T(""));
	}
	return 0;
}