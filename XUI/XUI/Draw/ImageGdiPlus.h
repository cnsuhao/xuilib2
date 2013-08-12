#pragma once 

#include <GdiPlus.h>

class CImageGdiPlus
{ 
public: 
	CImageGdiPlus(void)	: m_pBitmap(NULL)								{} 
	virtual ~CImageGdiPlus(void)										{Empty();} 
	operator Gdiplus::Bitmap *()										{return m_pBitmap;} 

	void Empty()														{if (m_pBitmap) {delete m_pBitmap; m_pBitmap = NULL;}} 
	BOOL IsValid()														{return m_pBitmap && m_pBitmap->GetLastStatus() == Gdiplus::Ok;}

	BOOL Load(LPCTSTR szFile)											{Empty(); m_pBitmap = Gdiplus::Bitmap::FromFile(szFile); return IsValid();} 
	BOOL Load(HMODULE hInst, LPCTSTR szName)							{Empty(); m_pBitmap = Gdiplus::Bitmap::FromResource(hInst, szName); return IsValid();} 
	BOOL Load(HMODULE hInst, UINT uID)									{Empty(); m_pBitmap = Gdiplus::Bitmap::FromResource(hInst, MAKEINTRESOURCE(uID)); return IsValid();} 
	BOOL Load(HICON hIcon)												{Empty(); m_pBitmap = Gdiplus::Bitmap::FromHICON(hIcon); return IsValid();} 
	BOOL Load(IStream *pStream)											{Empty(); m_pBitmap = Gdiplus::Bitmap::FromStream(pStream); return IsValid();}

	DWORD GetARGB(INT x, INT y)											{if (!IsValid()) return 0; 
																			Gdiplus::Color color; m_pBitmap->GetPixel(x, y, &color); return color.GetValue();} 
	BOOL Clip(INT x, INT y, INT width, INT height)						{if (!IsValid()) return FALSE; 
																			Gdiplus::Image *pTemp = m_pBitmap; m_pBitmap = m_pBitmap->Clone(x, y, width, height, m_pBitmap->GetPixelFormat());
																			delete pTemp; pTemp = NULL;
																			return TRUE;}

	int GetImageWidth();
	int GetImageHeight();
	CRect GetImageRect();
	Gdiplus::Bitmap * GetBitmap();

private: 
	Gdiplus::Bitmap * m_pBitmap;
}; 

inline int CImageGdiPlus::GetImageWidth()
{
	if (!IsValid())
	{
		return 0;
	}

	return m_pBitmap->GetWidth();
}

inline int CImageGdiPlus::GetImageHeight()
{
	if (!IsValid())
	{
		return 0;
	}

	return m_pBitmap->GetHeight();
}

inline CRect CImageGdiPlus::GetImageRect()
{
	return CRect(0, 0, GetImageWidth(), GetImageHeight());
}

inline Gdiplus::Bitmap * CImageGdiPlus::GetBitmap()
{
	return m_pBitmap;
}