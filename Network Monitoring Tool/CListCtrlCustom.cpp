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
	m_NewListFont.DeleteObject();
}




BEGIN_MESSAGE_MAP(CListCtrlCustom, CListCtrl)
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_MEASUREITEM()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
END_MESSAGE_MAP()

void CListCtrlCustom::OnInitialize()
{
	m_NewListFont.CreatePointFont(80, _T("Microsoft Sans Serif"));
	SetFont(&m_NewListFont);

	CHeaderCtrl* pHeader = NULL;
	pHeader = GetHeaderCtrl();

	if (pHeader == NULL)
		return;

	VERIFY(m_ctrlHeader.SubclassWindow(pHeader->m_hWnd));

	HDITEM hdItem;

	hdItem.mask = HDI_FORMAT;

	for (int i = 0; i < m_ctrlHeader.GetItemCount(); i++)
	{
		m_ctrlHeader.GetItem(i, &hdItem);

		hdItem.fmt |= HDF_OWNERDRAW;

		m_ctrlHeader.SetItem(i, &hdItem);
	}
}

LRESULT CListCtrlCustom::OnSetFont(WPARAM wParam, LPARAM)
{
	LRESULT res = Default();

	CRect rc;
	GetWindowRect(&rc);

	WINDOWPOS wp;
	wp.hwnd = m_hWnd;
	wp.cx = rc.Width();
	wp.cy = rc.Height();
	wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
	SendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);

	return res;
}

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
			CString csString;
			csString = this->GetItemText((int)iRow, (int)CCheckOpenPortsDlg::COL_IPADDRESS);
			if (csString.Compare(m_csIP) == 0)
			{
				lplvcd->clrTextBk = RGB(255, 255, 0);
			}
			else
			{
				if (iRow % 2)
				{
					lplvcd->clrTextBk = m_colRow1;
				}
				else
				{
					lplvcd->clrTextBk = m_colRow2;
				}
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

/*	CRect rect;
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
	*/
	return FALSE;
}

void CListCtrlCustom::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO:  Add your code to draw the specified item
	TCHAR  lpBuffer[256];
	LV_ITEM lvi;
	CString csString;
	csString = this->GetItemText((int)lpDrawItemStruct->itemID, (int)CCheckOpenPortsDlg::COL_IPADDRESS);

	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = lpDrawItemStruct->itemID;
	lvi.iSubItem = 0;
	lvi.pszText = lpBuffer;
	lvi.cchTextMax = sizeof(lpBuffer);
	VERIFY(GetItem(&lvi));

	LV_COLUMN lvc, lvcprev;
	::ZeroMemory(&lvc, sizeof(lvc));
	::ZeroMemory(&lvcprev, sizeof(lvcprev));
	lvc.mask = LVCF_WIDTH | LVCF_FMT;
	lvcprev.mask = LVCF_WIDTH | LVCF_FMT;

	
	RECT rectText = lpDrawItemStruct->rcItem;
	rectText.top += 6;
	
	for (int nCol = 0; GetColumn(nCol, &lvc); nCol++)
	{
		if (nCol > 0)
		{
			// Get Previous Column Width in order to move the next display item
			GetColumn(nCol - 1, &lvcprev);
			rectText.left = (lpDrawItemStruct->rcItem.left += lvcprev.cx) + 6;
			rectText.right = lpDrawItemStruct->rcItem.right += lpDrawItemStruct->rcItem.left;
		}
		else
		{
			rectText.left = (lpDrawItemStruct->rcItem.left += 0) + 6;
			rectText.right = lpDrawItemStruct->rcItem.right += lpDrawItemStruct->rcItem.left;
		}

		// Get the text 
		::ZeroMemory(&lvi, sizeof(lvi));
		lvi.iItem = lpDrawItemStruct->itemID;
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.iSubItem = nCol;
		lvi.pszText = lpBuffer;
		lvi.cchTextMax = sizeof(lpBuffer);
		VERIFY(GetItem(&lvi));

		CDC* pDC;
		pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

		if (lpDrawItemStruct->itemState & ODS_SELECTED)
		{
			pDC->FillSolidRect(&lpDrawItemStruct->rcItem, GetSysColor(COLOR_HIGHLIGHT));
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else
		{
			
			if (csString.Compare(m_csIP) == 0)
			{
				pDC->FillSolidRect(&lpDrawItemStruct->rcItem, RGB(255, 255, 0));
			}
			else
			{
				if (lpDrawItemStruct->itemID % 2)
					pDC->FillSolidRect(&lpDrawItemStruct->rcItem, m_colRow1);// GetSysColor(COLOR_WINDOW));
				else
					pDC->FillSolidRect(&lpDrawItemStruct->rcItem, m_colRow2);
				pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			}
		}

		pDC->SelectObject(GetStockObject(DEFAULT_GUI_FONT));

		UINT		uFormat = DT_LEFT;

		//lpDrawItemStruct->rcItem.top = 2;
		::DrawText(lpDrawItemStruct->hDC, lpBuffer, _tcslen(lpBuffer),
			&rectText, uFormat);

		pDC->SelectStockObject(SYSTEM_FONT);
	}
}


void CListCtrlCustom::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO: Add your message handler code here and/or call default

	CListCtrl::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

void CListCtrlCustom::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// Get the LOGFONT for the current font.
	LOGFONT lf;
	::ZeroMemory(&lf, sizeof(lf));

	CFont* pFont = GetFont();
	ASSERT_VALID(pFont);

	if (pFont)
		VERIFY(pFont->GetLogFont(&lf));

	int nAdj(14);
	

	if (lf.lfHeight < 0)
		lpMeasureItemStruct->itemHeight = ((-lf.lfHeight + nAdj));
	else
		lpMeasureItemStruct->itemHeight = ((lf.lfHeight + nAdj) );
}