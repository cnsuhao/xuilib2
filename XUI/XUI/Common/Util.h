#pragma once

namespace Util
{
	BOOL InitGdiPlus();

	BOOL UninitGdiPlus();

	const CString & GetXLibPath();

	HBITMAP CreateDIBSection32(int nWidth, int nHeight, PBYTE *pBytes = NULL);

	BOOL ClearRect(HDC dc, const CRect &rect);

	BOOL BitBlt(HDC dcSrc, const CRect &srcRect, HDC dcDst, const CRect &dstRect);

	BOOL AlaphaBlend(HDC dcSrc, const CRect &srcRect, HDC dcDst, const CRect &dstRect, BYTE cExtraAlpha = 255);

	BOOL DrawLine(HDC dc, const CPoint &ptStart, const CPoint &ptEnd, UINT nWidth, 
		BYTE r = 0, BYTE g = 0, BYTE b = 0, BYTE a = 0, BOOL bDotted = FALSE);
}