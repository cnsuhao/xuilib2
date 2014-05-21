#include "StdAfx.h"
#include "Util.h"

#include <GdiPlus.h>

static ULONG_PTR g_GdiplusToken = 0;
static DWORD g_dwGdiplusInitCount = 0;

BOOL Util::InitGdiPlus()
{
	if (!g_GdiplusToken)
	{
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartup(&g_GdiplusToken, &gdiplusStartupInput, NULL);
	}

	g_dwGdiplusInitCount++;

	return TRUE;
}

BOOL Util::UninitGdiPlus()
{
	if (g_dwGdiplusInitCount == 0)
		return FALSE;

	g_dwGdiplusInitCount--;

	if (g_dwGdiplusInitCount > 0)
		return TRUE;

	if (g_GdiplusToken)
	{
		Gdiplus::GdiplusShutdown(g_GdiplusToken);
		g_GdiplusToken = 0;
	}

	return TRUE;
}

HBITMAP Util::CreateDIBSection32(int nWidth, int nHeight, PBYTE *ppBytes/* = NULL*/)
{
	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth		= nWidth;
	bmi.bmiHeader.biHeight		= nHeight;
	bmi.bmiHeader.biPlanes		= 1;
	bmi.bmiHeader.biBitCount	= 32;
	bmi.bmiHeader.biCompression	= 0;

	PBYTE pDibBits = NULL;
	HBITMAP bmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pDibBits, NULL, 0);
	if (bmp && pDibBits)
		ZeroMemory(pDibBits, 4 * nWidth * nHeight);

	if (ppBytes)
		*ppBytes = pDibBits;

	return bmp;
}

BOOL Util::ClearRect( HDC dc, const CRect &rect )
{
	HDC dcEmpty = ::CreateCompatibleDC(dc);
	HGDIOBJ hOldBitmap = ::SelectObject(dcEmpty, (HGDIOBJ)CreateDIBSection32(rect.Width(), rect.Height()));

	::BitBlt(dc, rect.left, rect.top, rect.Width(), rect.Height(), dcEmpty, 0, 0, SRCCOPY);

	::DeleteObject(::SelectObject(dcEmpty, hOldBitmap));
	::DeleteDC(dcEmpty);

	return TRUE;
}

BOOL Util::AlaphaBlend( HDC dcSrc, const CRect &srcRect, HDC dcDst, const CRect &dstRect, BYTE cExtraAlpha /*= 255*/)
{
	BLENDFUNCTION blend = {AC_SRC_OVER, 0, cExtraAlpha, AC_SRC_ALPHA};

	return ::AlphaBlend(dcDst, dstRect.left, dstRect.top, dstRect.Width(), dstRect.Height(), 
		dcSrc, srcRect.left, srcRect.top, srcRect.Width(), srcRect.Height(), 
		blend);
}

BOOL Util::BitBlt( HDC dcSrc, const CRect &srcRect, HDC dcDst, const CRect &dstRect )
{
	ATLASSERT(srcRect.Width() == dstRect.Width() && srcRect.Height() == dstRect.Height());

	return ::BitBlt(dcDst, dstRect.left, dstRect.top, dstRect.Width(), dstRect.Height(), 
		dcSrc, srcRect.left, srcRect.top, SRCCOPY);
}

BOOL Util::DrawLine( HDC dc, const CPoint &ptStart, const CPoint &ptEnd, UINT nWidth, 
					BYTE r /*= 0*/, BYTE g /*= 0*/, BYTE b /*= 0*/, BYTE a /*= 0*/, BOOL bDotted /*= FALSE*/ )
{
	Gdiplus::Pen pen(Gdiplus::Color(a, r, g, b), nWidth);

	if (bDotted)
		pen.SetDashStyle(Gdiplus::DashStyleDash);

	Gdiplus::Graphics graph(dc);

	return Gdiplus::Ok == 
		graph.DrawLine(&pen, Gdiplus::Point(ptStart.x, ptStart.y), Gdiplus::Point(ptEnd.x, ptEnd.y));
}

const CString & Util::GetXLibPath()
{
	static CString strXLibPath;
	if (!strXLibPath.IsEmpty())
		return strXLibPath;

	CString strLocalXLib(SMCRelativePath(_T("XLib.dll")));
	if (::PathFileExists(strLocalXLib))
	{
		strXLibPath = strLocalXLib;
		return strXLibPath;
	}

	TCHAR szDevXLib[MAX_PATH] = {};
	::GetModuleFileName(NULL, szDevXLib, MAX_PATH);
	::PathRemoveFileSpec(szDevXLib);
#ifdef _DEBUG
	::PathAppend(szDevXLib, _T("../../XLib/Debug/XLib.dll"));
#else
	::PathAppend(szDevXLib, _T("../../XLib/Release/XLib.dll"));
#endif

	if (::PathFileExists(szDevXLib))
	{
		strXLibPath = szDevXLib;
		return strXLibPath;
	}

	ATLASSERT(!_T("FATAL ERROR: NO XLIB FOUND."));
	return strXLibPath;
}
