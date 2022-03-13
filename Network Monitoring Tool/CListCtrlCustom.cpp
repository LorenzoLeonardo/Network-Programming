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
	m_colRow1 = RGB(250, 250, 250);
	m_colRow2 = RGB(204, 213, 240);//RGB(0, 255, 255);
	m_nColumnHeight = 0;
	m_rect = { 0,0,0,0 };
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
	ON_NOTIFY(HDN_ITEMCHANGINGA, 0, &CListCtrlCustom::OnHdnItemchanging)
	ON_NOTIFY(HDN_ITEMCHANGINGW, 0, &CListCtrlCustom::OnHdnItemchanging)
END_MESSAGE_MAP()

void CListCtrlCustom::OnInitialize()
{
	m_NewListFont.CreatePointFont(80, _T("Microsoft Sans Serif"));
	SetFont(&m_NewListFont);
	SetBkColor(RGB(128, 128, 128));
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

		hdItem.fmt |= HDF_OWNERDRAW| HDF_CENTER;

		m_ctrlHeader.SetItem(i, &hdItem);
	}
	GetClientRect(&m_rect);
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
			CDC* pDC = CDC::FromHandle(lplvcd->nmcd.hdc);
			RECT rect = m_rect;

			rect.bottom = m_nColumnHeight;
			CBrush brush0(m_colRow1);
			CBrush brush1(m_colRow2);

			int chunk_height = GetCountPerPage();

			for (int i = 0; i <= chunk_height+1; i++)
			{
				pDC->FillRect(&rect, i % 2 ? &brush1 : &brush0);
				rect.top += m_nColumnHeight;
				rect.bottom += m_nColumnHeight;
			}
			return;
		}
		// Modify item text and or background
		
		case CDDS_ITEMPREPAINT:
		{
			*pResult = CDRF_NOTIFYITEMDRAW;//CDRF_NOTIFYSUBITEMDRAW;

			return;
		}

		case CDDS_POSTPAINT://CDDS_SUBITEM | CDDS_PREPAINT | CDDS_ITEM:
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
			CDC* pDC = CDC::FromHandle(lplvcd->nmcd.hdc);
			RECT rect = m_rect;

			rect.bottom = m_nColumnHeight;
			CBrush brush0(m_colRow1);
			CBrush brush1(m_colRow2);

			int chunk_height = GetCountPerPage();

			for (int i = 0; i <= chunk_height + 1; i++)
			{
				pDC->FillRect(&rect, i % 2 ? &brush1 : &brush0);
				rect.top += m_nColumnHeight;
				rect.bottom += m_nColumnHeight;
			}
			return;
		}
	}
}
int CListCtrlCustom::InsertItem(_In_ UINT nMask, _In_ int nItem, _In_z_ LPCTSTR lpszItem, _In_ UINT nState,
	_In_ UINT nStateMask, _In_ int nImage, _In_ LPARAM lParam)
{

	return CListCtrl::InsertItem(nMask, nItem, lpszItem, nState, nStateMask, nImage, lParam);
}

BOOL CListCtrlCustom::DeleteItem(_In_ int nItem)
{
	LockWindowUpdate();
	BOOL bRet =  CListCtrl::DeleteItem(nItem);
	UnlockWindowUpdate();
	//RedrawItems(nItem, GetItemCount());
	RedrawWindow();
	return bRet;
}
BOOL CListCtrlCustom::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

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

	lpDrawItemStruct->rcItem.bottom = lpDrawItemStruct->rcItem.top + m_nColumnHeight;
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
			}
			pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
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

	m_nColumnHeight = lpMeasureItemStruct->itemHeight;
}

BEGIN_MESSAGE_MAP(CHeaderCtrlCustom, CHeaderCtrl)
ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CHeaderCtrlCustom::OnNMCustomdraw)
ON_NOTIFY(HDN_ITEMCHANGINGA, 0, &CHeaderCtrlCustom::OnHdnItemchanging)
ON_NOTIFY(HDN_ITEMCHANGINGW, 0, &CHeaderCtrlCustom::OnHdnItemchanging)
END_MESSAGE_MAP()


void CHeaderCtrlCustom::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = CDRF_DODEFAULT;

	if (pNMCD->dwDrawStage == CDDS_PREPAINT)
	{
	//	CDC* pDC = CDC::FromHandle(pNMCD->hdc);
	//	CRect rect(0, 0, 0, 0);
	//	GetClientRect(&rect);
	//	pDC->FillSolidRect(&rect, RGB(64, 86, 141));
	//	pDC->SetTextColor(RGB(255, 255, 255));
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (pNMCD->dwDrawStage == CDDS_ITEMPREPAINT)
	{

		*pResult = CDRF_NOTIFYPOSTPAINT;
	}
	else if (pNMCD->dwDrawStage == CDDS_ITEMPOSTPAINT)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	/*	HDITEM hditem;
		TCHAR buffer[MAX_PATH] = { 0 };
		SecureZeroMemory(&hditem, sizeof(HDITEM));
		hditem.mask = HDI_TEXT;
		hditem.pszText = buffer;
		hditem.cchTextMax = MAX_PATH;
		GetItem(pNMCD->dwItemSpec, &hditem);
		CRect rect(0, 0, 0, 0);
		GetClientRect(&rect);
		CDC* pDC = CDC::FromHandle(pNMCD->hdc);
		pDC->FillSolidRect(&rect, RGB(64, 86, 141));

	
		pDC->SetBkColor(RGB(64, 86, 141));
		pDC->SetTextColor(RGB(255, 255, 255));

		CString str(buffer);
		pDC->DrawText(str, CRect(pNMCD->rc), DT_VCENTER | DT_LEFT);*/
	}
}


void CHeaderCtrlCustom::OnHdnItemchanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	if ((phdr->pitem->mask & HDI_WIDTH) != 0)
	{
		if(phdr->iItem == 0)
			phdr->pitem->cxy = 30;
	}
}


void CListCtrlCustom::OnHdnItemchanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	if ((phdr->pitem->mask & HDI_WIDTH) != 0)
	{
		if (phdr->iItem == 0)
			phdr->pitem->cxy = 30;
	}
}
