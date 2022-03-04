#include "pch.h"
#include "CListCtrlCustom.h"
#include "CheckOpenPortsDlg.h"

CHeaderCtrlCustom::CHeaderCtrlCustom()
{


}
CHeaderCtrlCustom:: ~CHeaderCtrlCustom()
{

}



CListCtrlCustom::CListCtrlCustom()
{
	m_colRow1 = RGB(240, 255, 255);
	m_colRow2 = RGB(0, 255, 255);
}

CListCtrlCustom::~CListCtrlCustom()
{
}




BEGIN_MESSAGE_MAP(CListCtrlCustom, CListCtrl)
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)

END_MESSAGE_MAP()

void CListCtrlCustom::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

	size_t iRow = lplvcd->nmcd.dwItemSpec;
	bool bHighlighted = false;

	switch (lplvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
			return;
		}
		// Modify item text and or background
		case CDDS_ITEMPREPAINT:
		{
		
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			return;
		}

		case CDDS_SUBITEM | CDDS_PREPAINT | CDDS_ITEM:
		{
			if (iRow % 2)
			{
				lplvcd->clrTextBk = m_colRow1;
			}
			else
			{
				lplvcd->clrTextBk = m_colRow2;
			}

			CString csString;
			csString = this->GetItemText((int)iRow, (int)CCheckOpenPortsDlg::COL_IPADDRESS);
			if (csString.Compare(m_csIP) == 0)
			{
				lplvcd->clrTextBk = RGB(255, 255, 0);
			}

			*pResult = CDRF_DODEFAULT;
			return;
		}
	}
}
int CListCtrlCustom::InsertItem(_In_ UINT nMask, _In_ int nItem, _In_z_ LPCTSTR lpszItem, _In_ UINT nState,
	_In_ UINT nStateMask, _In_ int nImage, _In_ LPARAM lParam)
{

	return CListCtrl::InsertItem(nMask, nItem, lpszItem, nState, nStateMask, nImage, lParam);
}

BOOL CListCtrlCustom::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	CRect rect;
	GetClientRect(rect);


	POINT mypoint;

	memset(&mypoint, 0, sizeof(mypoint));
	CBrush brush0(m_colRow1);
	CBrush brush1(m_colRow2);
	
	int chunk_height = GetCountPerPage();
	pDC->FillRect(&rect, &brush0);

	for (int i = 0; i <= chunk_height; i++)
	{
		GetItemPosition(i, &mypoint);
		rect.top = mypoint.y;
		GetItemPosition(i + 1, &mypoint);
		rect.bottom = mypoint.y;
		pDC->FillRect(&rect, i % 2 ? &brush0 : &brush1);
	}

	brush0.DeleteObject();
	brush1.DeleteObject();

	return FALSE;
}