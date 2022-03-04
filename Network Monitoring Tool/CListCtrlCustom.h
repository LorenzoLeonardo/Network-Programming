#pragma once
#include <afxcmn.h>

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

class CHeaderCtrlCustom :
    public CHeaderCtrl
{
public:
    CHeaderCtrlCustom();
    virtual ~CHeaderCtrlCustom();

};

class CListCtrlCustom :
    public CListCtrl
{
public:
    CListCtrlCustom();
    virtual ~CListCtrlCustom();
    COLORREF m_colRow1;
    COLORREF m_colRow2;

    int InsertItem(_In_ UINT nMask, _In_ int nItem, _In_z_ LPCTSTR lpszItem, _In_ UINT nState,
        _In_ UINT nStateMask, _In_ int nImage, _In_ LPARAM lParam);
    void SetAdapterIP(CString ip)
    {
        m_csIP = ip;
    }
protected:
   // afx_msg void PreSubclassWindow();
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    DECLARE_MESSAGE_MAP()
    CString m_csIP;
    CHeaderCtrlCustom m_ctrlHeader;
};

