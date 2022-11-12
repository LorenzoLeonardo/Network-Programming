#include "pch.h"
#include "CCustomClock.h"

CCustomClock *pCCustomClock;
CCustomClock::CCustomClock() {
  m_csFontStyle = _T("Microsoft Sans Serif");
  m_nFontSize = 12;
  m_nFontWeight = FW_NORMAL;
  m_textColor = RGB(0, 0, 0);
  m_textBKColor = RGB(255, 255, 255);
  pCCustomClock = this;
}
CCustomClock::~CCustomClock() {}

void CCustomClock::CreateClock() {
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
                            m_csFontStyle));          // lpszFacename

  VERIFY(m_cFontAMPM.CreateFont(m_nFontSize / 2,          // nHeight
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

  VERIFY(m_cFontDate.CreateFont(m_nFontSize / 3,          // nHeight
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
void CCustomClock::DestroyClock() {
  m_cfont.DeleteObject();
  m_cFontAMPM.DeleteObject();
  m_cFontDate.DeleteObject();
}
CString CCustomClock::GetDateTime() {
  SYSTEMTIME sysTime;
  WORD wHour = 0;
  CString csDateTime;

  GetLocalTime(&sysTime);
  wHour = sysTime.wHour;
  if (sysTime.wHour == 0)
    wHour = 12;
  else if (sysTime.wHour > 12)
    wHour = sysTime.wHour - 12;

  csDateTime.Format(_T("%s %02d, %04d "),
                    pCCustomClock->GetMonthName(sysTime.wMonth).GetBuffer(),
                    sysTime.wDay, sysTime.wYear);

  if (sysTime.wHour >= 12)
    csDateTime.AppendFormat(_T("%d:%02d:%02d PM"), wHour, sysTime.wMinute,
                            sysTime.wSecond);
  else
    csDateTime.AppendFormat(_T("%d:%02d:%02d AM"), wHour, sysTime.wMinute,
                            sysTime.wSecond);

  return csDateTime;
}
void CCustomClock::DrawClock(CClientDC *dcSrc, int x, int y) {
  SYSTEMTIME sysTime;
  SIZE size;
  TEXTMETRIC tm;
  INT xInterval = 0, yInterval = 0;
  CString csTime = _T(""), csAMPM = _T(""), csDate = _T("");
  WORD wHour = 0;

  CFont *pFont = dcSrc->SelectObject(&m_cfont);
  GetLocalTime(&sysTime);
  dcSrc->SetBkMode(OPAQUE);
  dcSrc->SetTextColor(m_textColor);
  dcSrc->SetBkColor(m_textBKColor);
  wHour = sysTime.wHour;
  if (sysTime.wHour == 0)
    wHour = 12;
  if (sysTime.wHour > 12)
    wHour = sysTime.wHour - 12;
  dcSrc->TextOut(x, y, csTime, csTime.GetLength());
  csTime.Format(_T("%d:%02d:%02d"), wHour, sysTime.wMinute, sysTime.wSecond);
  dcSrc->TextOut(x, y, csTime, csTime.GetLength());
  GetTextExtentPoint32(dcSrc->GetSafeHdc(), csTime, csTime.GetLength(), &size);
  GetTextMetrics(dcSrc->GetSafeHdc(), &tm);
  xInterval = x + size.cx;
  yInterval = y;
  dcSrc->SelectObject(pFont);
  //////////////////////////////////////////////
  pFont = dcSrc->SelectObject(&m_cFontAMPM);
  dcSrc->SetTextColor(m_textColor);
  dcSrc->SetBkColor(m_textBKColor);
  dcSrc->TextOut(x, y, csAMPM, csAMPM.GetLength());
  if (sysTime.wHour >= 12)
    csAMPM = _T(" PM");
  else
    csAMPM = _T(" AM");
  dcSrc->TextOut(xInterval, ((tm.tmAscent) / 2) + y, csAMPM,
                 csAMPM.GetLength());
  GetTextExtentPoint32(dcSrc->GetSafeHdc(), csAMPM, csAMPM.GetLength(), &size);
  xInterval += size.cx;
  yInterval += tm.tmHeight;
  dcSrc->SelectObject(pFont);
  //////////////////////////////////////////////
  pFont = dcSrc->SelectObject(&m_cFontDate);
  dcSrc->SetTextColor(m_textColor);
  dcSrc->SetBkColor(m_textBKColor);
  dcSrc->TextOut(x, m_nFontSize + y, csDate, csDate.GetLength());
  csDate.Format(
      _T("%s, %s %02d, %04d"),
      CalcDayOfWeek(sysTime.wYear, sysTime.wMonth, sysTime.wDay).GetBuffer(),
      GetMonthName(sysTime.wMonth).GetBuffer(), sysTime.wDay, sysTime.wYear);
  dcSrc->TextOut(x, m_nFontSize + y, csDate, csDate.GetLength());
  GetTextExtentPoint32(dcSrc->GetSafeHdc(), csDate, csDate.GetLength(), &size);
  GetTextMetrics(dcSrc->GetSafeHdc(), &tm);
  dcSrc->SelectObject(pFont);
  //////////////////////////////////////////////
  m_rectClock.left = x;
  m_rectClock.top = y;
  m_rectClock.right = xInterval;
  m_rectClock.bottom = yInterval + tm.tmHeight;
}

ULONG CCustomClock::CalcDayNumFromDate(UINT y, UINT m, UINT d) {
  m = (m + 9) % 12;
  y -= m / 10;
  ULONG dn = 365 * y + y / 4 - y / 100 + y / 400 + (m * 306 + 5) / 10 + (d - 1);

  return dn;
}

// ----------------------------------------------------------------------
// Given year, month, day, return the day of week (string).
// ----------------------------------------------------------------------
CString CCustomClock::CalcDayOfWeek(WORD y, WORD m, WORD d) {
  CString day[] = {_T("Wednesday"), _T("Thursday"), _T("Friday"),
                   _T("Saturday"),  _T("Sunday"),   _T("Monday"),
                   _T("Tuesday")};

  ULONG dn = CalcDayNumFromDate(y, m, d);

  return day[dn % 7];
}

CString CCustomClock::GetMonthName(WORD wMonth) {
  CString day[] = {_T("Error"),     _T("January"), _T("February"),
                   _T("March"),     _T("April"),   _T("May"),
                   _T("June"),      _T("July"),    _T("August"),
                   _T("September"), _T("October"), _T("November"),
                   _T("December")};

  return day[wMonth];
}