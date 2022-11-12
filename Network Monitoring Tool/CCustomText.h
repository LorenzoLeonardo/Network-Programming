#pragma once
class CCustomText {
public:
	CCustomText(int nFontSize, int nFontWeight,
		CString csFontStyle = _T("Microsoft Sans Serif"));
	~CCustomText();

	void SetFontStyle(CString csFontStyle) { m_csFontStyle = csFontStyle; }
	void SetFontSize(INT nFontSize) { m_nFontSize = nFontSize; }
	void SetFontWeight(INT nFontWeight) { m_nFontWeight = nFontWeight; }
	void SetTextColor(COLORREF color) { m_textColor = color; }
	void SetTextBKColor(COLORREF color) { m_textBKColor = color; }
	void DrawCustomText(CClientDC* dcSrc, int x, int y, CString csInputString);

protected:
	CFont m_cfont;
	CString m_csFontStyle;
	INT m_nFontSize;
	INT m_nFontWeight;
	COLORREF m_textColor;
	COLORREF m_textBKColor;
};
