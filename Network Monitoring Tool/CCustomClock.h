#pragma once
class CCustomClock
{
public:
	CCustomClock();
	~CCustomClock();

	void SetFontStyle(CString csFontStyle)
	{
		m_csFontStyle = csFontStyle;
	}
	void SetFontSize(INT nFontSize)
	{
		m_nFontSize = nFontSize;
	}
	void SetFontWeight(INT nFontWeight)
	{
		m_nFontWeight = nFontWeight;
	}
	void SetTextColor(COLORREF color)
	{
		m_textColor = color;
	}
	void SetTextBKColor(COLORREF color)
	{
		m_textBKColor = color;
	}
	void CreateClock();
	void DestroyClock();
	void DrawClock(CClientDC* , int x, int y);
	void GetClockRect(LPRECT rect)
	{
		*rect = m_rectClock;
	}
	static CString GetDateTime();
protected:
	CFont			m_cfont;
	CFont			m_cFontAMPM;
	CFont			m_cFontDate;
	CString			m_csFontStyle;
	INT				m_nFontSize;
	INT				m_nFontWeight;
	COLORREF		m_textColor;
	COLORREF		m_textBKColor;
	RECT			m_rectClock;

	ULONG CalcDayNumFromDate(UINT y, UINT m, UINT d);
	CString CalcDayOfWeek(WORD y, WORD m, WORD d);
	CString GetMonthName(WORD wMonth);
};

