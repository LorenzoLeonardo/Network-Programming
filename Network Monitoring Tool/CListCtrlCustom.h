#pragma once
#include <afxcmn.h>
#include <vector>

using namespace std;
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(                                                               \
    linker,                                                                    \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(                                                               \
    linker,                                                                    \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(                                                               \
    linker,                                                                    \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(                                                               \
    linker,                                                                    \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

class CHeaderCtrlCustom : public CHeaderCtrl {
public:
  CHeaderCtrlCustom();
  virtual ~CHeaderCtrlCustom();

  DECLARE_MESSAGE_MAP()
  afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
};

class CListCtrlCustom : public CListCtrl {
public:
  CListCtrlCustom();
  virtual ~CListCtrlCustom();
  COLORREF m_colRow1;
  COLORREF m_colRow2;

  int InsertItem(_In_ UINT nMask, _In_ int nItem, _In_z_ LPCTSTR lpszItem,
                 _In_ UINT nState, _In_ UINT nStateMask, _In_ int nImage,
                 _In_ LPARAM lParam);
  BOOL DeleteItem(_In_ int nItem);

  int InsertColumn(_In_ int nCol, _In_ const LVCOLUMN *pColumn);
  int InsertColumn(_In_ int nCol, _In_z_ LPCTSTR lpszColumnHeading,
                   _In_ int nFormat = LVCFMT_LEFT, _In_ int nWidth = -1,
                   _In_ int nSubItem = -1);

  void SetAdapterIP(CString ip) { m_csIP = ip; }

protected:
  // afx_msg void PreSubclassWindow();
  afx_msg void OnCustomDraw(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg BOOL OnEraseBkgnd(CDC *pDC);
  afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
  afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
  DECLARE_MESSAGE_MAP()
  CString m_csIP;
  CHeaderCtrlCustom m_ctrlHeader;
  CFont m_NewListFont;
  int m_nColumnHeight;
  RECT m_rect;
  struct header_format {
    int nColWidth;
    int nFormat;
  };
  vector<struct header_format> m_vFormat;

public:
  virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
  afx_msg void OnMeasureItem(int nIDCtl,
                             LPMEASUREITEMSTRUCT lpMeasureItemStruct);
  void OnInitialize();
  afx_msg void OnHdnItemchanging(NMHDR *pNMHDR, LRESULT *pResult);
};
