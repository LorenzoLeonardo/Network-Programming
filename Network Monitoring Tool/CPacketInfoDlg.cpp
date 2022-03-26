// CPacketInfoDlg.cpp : implementation file
//

#include "pch.h"
#include "afxdialogex.h"
#include "CPacketInfoDlg.h"
#include "CheckOpenPortsDlg.h"
#include "../EnzNetworkDLL/DebugLog.h"
// CPacketInfoDlg dialog

IMPLEMENT_DYNAMIC(CPacketInfoDlg, CDialogEx)
mutex g_mtx;

CPacketInfoDlg::CPacketInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CPacketInfoDlg::IDD, pParent)
{
	m_pParent = pParent;
	m_hBrushBackGround = CreateSolidBrush(RGB(93, 107, 153));
	m_hBrushEditArea = CreateSolidBrush(RGB(255, 255, 255));
}

CPacketInfoDlg::~CPacketInfoDlg()
{
	DeleteObject(m_hBrushBackGround);
	DeleteObject(m_hBrushEditArea);
}

void CPacketInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PACKET_INFO, m_ctrlEditPacketReportArea);
}


BEGIN_MESSAGE_MAP(CPacketInfoDlg, CDialogEx)
	ON_WM_CTLCOLOR()
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
	delete this;
}

void CPacketInfoDlg::OnBnClickedCancel()
{
	m_bIsCanceled = true;
	DestroyWindow();
}

void CPacketInfoDlg::UpdatePacketInfo(CString csPacketInfo, int nProtocol)
{
	try
	{
		if (m_bIsCanceled)
			return;
		if (m_ctrlEditPacketReportArea.m_hWnd)
		{
			if (m_ctrlEditPacketReportArea.m_hWnd && ((CButton*)GetDlgItem(IDC_CHECK_IGMP))->GetCheck() == BST_UNCHECKED
				&& nProtocol == IGMP_PROTOCOL)
				return;
			else if (m_ctrlEditPacketReportArea.m_hWnd && ((CButton*)GetDlgItem(IDC_CHECK_TCP))->GetCheck() == BST_UNCHECKED
				&& nProtocol == TCP_PROTOCOL)
				return;
			else if (m_ctrlEditPacketReportArea.m_hWnd && ((CButton*)GetDlgItem(IDC_CHECK_UDP))->GetCheck() == BST_UNCHECKED
				&& nProtocol == UDP_PROTOCOL)
				return;
			else if (m_ctrlEditPacketReportArea.m_hWnd && ((CButton*)GetDlgItem(IDC_CHECK_ICMP))->GetCheck() == BST_UNCHECKED
				&& nProtocol == ICMP_PROTOCOL)
				return;



			CString csText;
			int nLineLimit = 50;
			long nLength = 0;
			int len = 0;
			int nBegin = 0;
			int nEnd = 0;
			if (m_ctrlEditPacketReportArea.m_hWnd)
				m_ctrlEditPacketReportArea.GetWindowText(csText);
			csText += csPacketInfo;

			if (m_ctrlEditPacketReportArea.m_hWnd)
				m_ctrlEditPacketReportArea.SetSel(0, 0);
			if (m_ctrlEditPacketReportArea.m_hWnd)
				m_ctrlEditPacketReportArea.ReplaceSel(csPacketInfo);
			if (m_ctrlEditPacketReportArea.m_hWnd)
				nLength = m_ctrlEditPacketReportArea.GetLineCount();
			if (nLength == nLineLimit)
			{
				if (m_ctrlEditPacketReportArea.m_hWnd)
					m_ctrlEditPacketReportArea.GetWindowText(csText);

				len = csText.ReverseFind(_T('\r'));
				if (m_ctrlEditPacketReportArea.m_hWnd)
					nBegin = m_ctrlEditPacketReportArea.LineIndex(nLength - 2);
				nEnd = nBegin + len;
				if (m_ctrlEditPacketReportArea.m_hWnd)
					m_ctrlEditPacketReportArea.SetSel(nBegin, nEnd, TRUE);
				if (m_ctrlEditPacketReportArea.m_hWnd)
					m_ctrlEditPacketReportArea.ReplaceSel(_T(""));
				if (m_ctrlEditPacketReportArea.m_hWnd)
					m_ctrlEditPacketReportArea.SetSel(0, 0, TRUE);
			}
			//}
		}
	}
	catch (exception e)
	{
		DEBUG_LOG(CA2W(e.what()).m_szBuffer);
	}
}

BOOL CPacketInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_bIsCanceled = false;
	RECT rect, thisRect;
	((CCheckOpenPortsDlg*)m_pParent)->GetWindowRect(&rect);

	((CCheckOpenPortsDlg*)m_pParent)->GetDlgItem(IDC_LIST_LAN)->GetWindowRect(&thisRect);
	//GetClientRect(&thisRect);
	MoveWindow(thisRect.left, thisRect.top, thisRect.right - thisRect.left, thisRect.bottom- thisRect.top);

	//::SetWindowTheme(GetDlgItem(IDC_CHECK_IGMP)->GetSafeHwnd(), _T(""), _T(""));
	//::SetWindowTheme(GetDlgItem(IDC_CHECK_ICMP)->GetSafeHwnd(), _T(""), _T(""));
	//::SetWindowTheme(GetDlgItem(IDC_CHECK_TCP)->GetSafeHwnd(), _T(""), _T(""));
	//::SetWindowTheme(GetDlgItem(IDC_CHECK_UDP)->GetSafeHwnd(), _T(""), _T(""));
	m_bThisObjDeleted = false;
	// TODO:  Add extra initialization here
	//m_hPacketInfoThread = (HANDLE)_beginthreadex(NULL, 0, PacketInfoThread, this, 0, NULL);

	((CButton*)GetDlgItem(IDC_CHECK_IGMP))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_TCP))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_UDP))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_ICMP))->SetCheck(BST_CHECKED);
	SetForegroundWindow();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

unsigned __stdcall  CPacketInfoDlg::PacketInfoThread(void* parg)
{
	CPacketInfoDlg* pDlg = (CPacketInfoDlg*)parg;
	while (pDlg)
	{
		pDlg->UpdatePacketInfo(_T(""),0);
	}
	return 0;
}

HBRUSH CPacketInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (nCtlColor)
	{
		case CTLCOLOR_DLG:
		{
			pDC->SetTextColor(RGB(0, 0, 0));
			pDC->SetBkColor(RGB(255, 255, 255));//RGB(64, 86, 141));
			pDC->SetBkMode(OPAQUE);
			return m_hBrushEditArea;
		}
		case CTLCOLOR_STATIC:
		{
			int id = pWnd->GetDlgCtrlID();

			if (id == IDC_EDIT_PACKET_INFO)
			{
				pDC->SetTextColor(RGB(0, 0, 0));
				pDC->SetBkColor(RGB(255, 255, 255));
				return m_hBrushEditArea;
			}

			else
			{
				pDC->SetTextColor(RGB(255, 255, 255));
				pDC->SetBkColor(RGB(255, 255, 255));//RGB(204, 213, 240));
				pDC->SetBkMode(OPAQUE);
				return m_hBrushEditArea;
			}
		}
		
		case CTLCOLOR_BTN: 
		{
			int id = pWnd->GetDlgCtrlID();
			if (id == IDC_CHECK_IGMP || id == IDC_CHECK_ICMP || id == IDC_CHECK_TCP || id == IDC_CHECK_UDP)
			{
				//pDC->SetTextColor(RGB(255, 255, 255));
				//pDC->SetBkColor(RGB(93, 107, 153));
				pDC->SetBkMode(TRANSPARENT);
				return m_hBrushBackGround;
			}
			break;
		}
		default:
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	return hbr;
}
