#include "pch.h"
#include "CCustomText.h"

CCustomText::CCustomText(int nFontSize, int nFontWeight, CString csFontStyle) {
  m_nFontSize = nFontSize;
  m_nFontWeight = nFontWeight;
  m_csFontStyle = csFontStyle;

  VERIFY(m_cfont.CreateFont(m_nFontSize,              // nHeight
                            0,                        // nWidth
                            0,                        // nEscapement
                            0,                        // nOrientation
                            m_nFontWeight,            // nWeight
                            FALSE,                    // bItalic
                            FALSE,                    // bUnderline
                            0,                        // cStrikeOut
                            ANSI_CHARSET,             // nCharSet
                            OUT_DEFAULT_PRECIS,       // nOutPrecision
                            CLIP_DEFAULT_PRECIS,      // nClipPrecision
                            DEFAULT_QUALITY,          // nQuality
                            DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily
                            m_csFontStyle));
}
CCustomText::~CCustomText() { m_cfont.DeleteObject(); }

void CCustomText::DrawCustomText(CClientDC *dcSrc, int x, int y,
                                 CString csInputString) {
  CFont *pFont = dcSrc->SelectObject(&m_cfont);

  // COLORREF ref = dcSrc->GetBkColor();
  // ref = RGB(255, 255, 255);
  dcSrc->SetBkColor(::GetSysColor(COLOR_3DFACE));
  dcSrc->SetTextColor(m_textColor);
  dcSrc->SetBkMode(OPAQUE);

  CString csEraser = _T("                        ");
  dcSrc->TextOut(x, y, _T("                       "), csEraser.GetLength());
  dcSrc->TextOut(x, y, csInputString, csInputString.GetLength());
  dcSrc->SelectObject(pFont);
}