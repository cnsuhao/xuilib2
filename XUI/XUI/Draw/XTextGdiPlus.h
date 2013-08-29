#pragma once

#include "XDraw.h"

#include <GdiPlus.h>

class CXTextGdiPlus : public IXText
{
public:
	CXTextGdiPlus(void);
	~CXTextGdiPlus(void);

public:
	virtual BOOL Draw(HDC hDC, const CRect &rcDraw) ;
	virtual BOOL SetAlpha(BYTE cAlpha);
	virtual BOOL SetDstRect(const CRect &rcDst);

	virtual CString GetText();
	virtual CSize Measure(HDC dc, INT nWidthLimit);
	
public:
	BOOL SetText(LPCTSTR szText);

	BOOL SetFont(LPCTSTR szFontName, int nSize, Gdiplus::FontStyle style);
	BOOL SetColor(BYTE alpha, BYTE r, BYTE g, BYTE b);
	BOOL SetAlignment(Gdiplus::StringAlignment AlignmentH, Gdiplus::StringAlignment AlignmentV);
	BOOL SetFormatFlags(Gdiplus::StringFormatFlags FormatFlags);
	BOOL SetRendering(Gdiplus::TextRenderingHint Rendering);

private:
	BOOL RefreashBufferDC(HDC hDCSrc);
	VOID ReleaseBufferDC();

private:
	HDC m_dcBuffer;
	HGDIOBJ m_hBufferOldBmp;
	CRect m_rcDst;

private:
	CString m_strText;

	CString m_strFontName;
	int m_nSize;
	Gdiplus::FontStyle m_FontStyle;
	
	BYTE m_ColorR;
	BYTE m_ColorG;
	BYTE m_ColorB;

	Gdiplus::StringAlignment m_AlignmentH;
	Gdiplus::StringAlignment m_AlignmentV;
	Gdiplus::StringFormatFlags m_FormatFlags;

	Gdiplus::TextRenderingHint m_Rendering;

	BYTE m_cAlpha;
};
