#pragma once

#include "ImageGdiPlus.h"
#include "XDraw.h"

class CXImageGdiPlus : public IXImage
{
public:
	CXImageGdiPlus(void);
	~CXImageGdiPlus(void);

public:
	virtual BOOL Draw(HDC hDC, const CRect &rcDraw);
	virtual BOOL SetAlpha(BYTE cAlpha);
	virtual BOOL SetSrcRect(const CRect &rcSrc);
	virtual BOOL SetDstRect(const CRect &rcDst);
	virtual BOOL SetDrawType(DrawImageType dit);
	virtual BOOL SetPartRect(const CRect &rcPart);
	virtual INT GetImageHeight();
	virtual INT GetImageWidth();

public:
	BOOL Load(LPCTSTR szFile);

public:
	BOOL IsFormattedImage();

private:
	BOOL RefreashBufferDC(HDC hDCSrc);
	VOID ReleaseBufferDC();
	VOID InitSrcRect();

private:
	BOOL LoadAsResource(LPCTSTR pRelativePath);
	VOID LoadFormattedImageInfo(LPCTSTR szFileName);
	VOID LoadFormattedImagePartInfo();

private:
	BOOL DrawNormal(HDC dc);
	BOOL DrawStretch(HDC dc);
	BOOL Draw9Part(HDC dc);
	BOOL Draw3PartH(HDC dc);
	BOOL Draw3PartV(HDC dc);
	BOOL DrawCenter(HDC dc);


private:
	CImageGdiPlus m_Image;
	HDC m_dcBuffer;
	HGDIOBJ m_hBufferOldBmp;

	DrawImageType m_DrawImageType;
	
	CRect m_rcPart;
	CRect m_rcSrc;
	CRect m_rcDst;

	BYTE m_cAlapha;

	BOOL m_bFormattedImage;
};
