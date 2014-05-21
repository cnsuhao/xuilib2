#include "StdAfx.h"
#include "XImageGdiPlus.h"
#include "..\Base\XResourceMgr.h"

CXImageGdiPlus::CXImageGdiPlus(void)
	: m_DrawImageType(DIT_UNKNOW),
	m_dcBuffer(NULL),
	m_hBufferOldBmp(NULL),
	m_rcPart(0, 0, 0, 0),
	m_rcSrc(0, 0, 0, 0),
	m_rcDst(0, 0, 0, 0),
	m_cAlapha(255),
	m_bFormattedImage(FALSE)
{
	Util::InitGdiPlus();
}

CXImageGdiPlus::~CXImageGdiPlus(void)
{
	ReleaseBufferDC();
	Util::UninitGdiPlus();
}

BOOL CXImageGdiPlus::Draw( HDC hDC, const CRect &rcDraw )
{
	if (!m_dcBuffer && !RefreshBufferDC(hDC))
	{
		return FALSE;
	}

	CRect rcDrawDstReal(0, 0, 0, 0);
	if (!::IntersectRect(&rcDrawDstReal, &rcDraw, &m_rcDst) || rcDrawDstReal.IsRectEmpty())
		return TRUE;

	CRect rcDrawSrcReal(rcDrawDstReal);
	rcDrawSrcReal.OffsetRect( - m_rcDst.left, - m_rcDst.top);
	
	Util::AlaphaBlend(m_dcBuffer, rcDrawSrcReal, hDC, rcDrawDstReal, m_cAlapha);

// 	::StretchBlt(hDC, rcDrawReal.left, rcDrawReal.top, rcDrawReal.Width(), rcDrawReal.Height(), 
// 		m_dcBuffer, rcDrawReal.left - m_rcDst.left, rcDrawReal.top - m_rcDst.top, rcDrawReal.Width(), rcDrawReal.Height(), SRCCOPY);

	return TRUE;
}

BOOL CXImageGdiPlus::SetDrawType(DrawImageType dit)
{
	if (m_DrawImageType == dit)
		return TRUE;

	ReleaseBufferDC();
	m_DrawImageType = dit;
	
	return TRUE;
}

BOOL CXImageGdiPlus::SetPartRect(const CRect &rcPart)
{
	if (m_rcPart == rcPart)
		return TRUE;

	ReleaseBufferDC();
	m_rcPart = rcPart;

	return TRUE;
}

BOOL CXImageGdiPlus::DrawNormal(HDC dc)
{
	Gdiplus::Graphics graph(dc);

	if (!m_Image.IsValid())
	{
		return FALSE;
	}

	graph.DrawImage(m_Image, Gdiplus::Rect(0, 0, m_rcSrc.Width(), m_rcSrc.Height()),
		m_rcSrc.left, m_rcSrc.top, m_rcSrc.Width(), m_rcSrc.Height(), Gdiplus::UnitPixel);
	
	return TRUE;
}

BOOL CXImageGdiPlus::DrawStretch(HDC dc)
{
	Gdiplus::Graphics graph(dc);

	if (!m_Image.IsValid())
	{
		return FALSE;
	}

	graph.DrawImage(m_Image, Gdiplus::Rect(0, 0, m_rcDst.Width(), m_rcDst.Height()),
		m_rcSrc.left, m_rcSrc.top, m_rcSrc.Width(), m_rcSrc.Height(), Gdiplus::UnitPixel);

	return TRUE;
}

BOOL CXImageGdiPlus::Draw9Part(HDC dc)
{
	if (!m_Image.IsValid())
	{
		return FALSE;
	}

	CRect rcPart(0, 0, 0, 0);
	CRect rcPartRaw(m_rcPart);
	rcPartRaw.OffsetRect(m_rcSrc.TopLeft());
	if (!rcPart.IntersectRect(m_rcSrc, rcPartRaw))
		rcPart.left = rcPart.top = rcPart.right = rcPart.bottom = 0;

	Gdiplus::Rect rcArrayImage[] =
	{
		Gdiplus::Rect(m_rcSrc.left, m_rcSrc.top, rcPart.left - m_rcSrc.left, rcPart.top - m_rcSrc.top),
		Gdiplus::Rect(rcPart.left, m_rcSrc.top, rcPart.Width(), rcPart.top - m_rcSrc.top),
		Gdiplus::Rect(rcPart.right, m_rcSrc.top, m_rcSrc.right - rcPart.right, rcPart.top - m_rcSrc.top),
		Gdiplus::Rect(m_rcSrc.left, rcPart.top, rcPart.left - m_rcSrc.left, rcPart.Height()),
		Gdiplus::Rect(rcPart.left, rcPart.top, rcPart.Width(), rcPart.Height()),
		Gdiplus::Rect(rcPart.right, rcPart.top, m_rcSrc.right - rcPart.right, rcPart.Height()),
		Gdiplus::Rect(m_rcSrc.left, rcPart.bottom, rcPart.left - m_rcSrc.left, m_rcSrc.bottom - rcPart.bottom),
		Gdiplus::Rect(rcPart.left, rcPart.bottom, rcPart.Width(), m_rcSrc.bottom - rcPart.bottom),
		Gdiplus::Rect(rcPart.right, rcPart.bottom, m_rcSrc.right - rcPart.right, m_rcSrc.bottom - rcPart.bottom)
	};

	CRect rcDstPart(rcPart.left - m_rcSrc.left, rcPart.top - m_rcSrc.top, 
						m_rcDst.right - (m_rcSrc.right - rcPart.right) - m_rcDst.left, 
						m_rcDst.bottom - (m_rcSrc.bottom - rcPart.bottom) - m_rcDst.top);
	Gdiplus::Rect rcArrayDst[] = 
	{
		Gdiplus::Rect(0, 0, rcDstPart.left, rcDstPart.top),
		Gdiplus::Rect(rcDstPart.left, 0, rcDstPart.Width(), rcDstPart.top),
		Gdiplus::Rect(rcDstPart.right, 0, m_rcDst.right - m_rcDst.left - rcDstPart.right, rcDstPart.top),
		Gdiplus::Rect(0, rcDstPart.top, rcDstPart.left, rcDstPart.Height()),
		Gdiplus::Rect(rcDstPart.left, rcDstPart.top, rcDstPart.Width(), rcDstPart.Height()),
		Gdiplus::Rect(rcDstPart.right, rcDstPart.top, m_rcDst.right - m_rcDst.left - rcDstPart.right, rcDstPart.Height()),
		Gdiplus::Rect(0, rcDstPart.bottom, rcDstPart.left, m_rcDst.bottom - m_rcDst.top - rcDstPart.bottom),
		Gdiplus::Rect(rcDstPart.left, rcDstPart.bottom, rcDstPart.Width(), m_rcDst.bottom - m_rcDst.top - rcDstPart.bottom),
		Gdiplus::Rect(rcDstPart.right, rcDstPart.bottom, m_rcDst.right - m_rcDst.left - rcDstPart.right, m_rcDst.bottom - m_rcDst.top - rcDstPart.bottom)
	};

	Gdiplus::Graphics graph(dc);
	for(UINT nIndex = 0; nIndex < ARRAYSIZE(rcArrayImage); ++nIndex)
	{
		if (rcArrayImage[nIndex].IsEmptyArea())
			continue;

		graph.DrawImage(m_Image, rcArrayDst[nIndex], 
			rcArrayImage[nIndex].X, rcArrayImage[nIndex].Y, rcArrayImage[nIndex].Width, rcArrayImage[nIndex].Height, 
			Gdiplus::UnitPixel, NULL, NULL, NULL);
	}

	return TRUE;
}

BOOL CXImageGdiPlus::Draw3PartH(HDC dc)
{
	ATLASSERT(m_rcPart.top == 0 && m_rcPart.bottom == m_Image.GetImageRect().bottom);
	return Draw9Part(dc);
}

BOOL CXImageGdiPlus::Draw3PartV(HDC dc)
{
	ATLASSERT(m_rcPart.left == 0 && m_rcPart.right == m_Image.GetImageRect().right);
	return Draw9Part(dc);
}

BOOL CXImageGdiPlus::DrawCenter(HDC dc)
{
	CRect rcImage = m_Image.GetImageRect();
	ATLASSERT(m_rcDst.Width() >= rcImage.Width() && m_rcDst.Height() >= rcImage.Height());

	CRect rcPaint((m_rcDst.Width() - rcImage.Width()) / 2, (m_rcDst.Height() - rcImage.Height()) / 2,
		0, 0);
	rcPaint.right = rcPaint.left + rcImage.Width();
	rcPaint.bottom = rcPaint.top + rcImage.Height();

	Gdiplus::Graphics graph(dc);
	graph.DrawImage(m_Image, rcPaint.left, rcPaint.right, rcPaint.Width(), rcPaint.Height());

	return TRUE;
}

VOID CXImageGdiPlus::ReleaseBufferDC()
{
	if (m_dcBuffer)
	{
		HGDIOBJ hMyBitmap = ::SelectObject(m_dcBuffer, m_hBufferOldBmp);
		m_hBufferOldBmp = NULL;
		::DeleteObject(hMyBitmap);
		hMyBitmap = NULL;
		::DeleteDC(m_dcBuffer);
		m_dcBuffer = NULL;
	}
}

BOOL CXImageGdiPlus::RefreshBufferDC(HDC hDCSrc)
{
	ReleaseBufferDC();

	m_dcBuffer = ::CreateCompatibleDC(hDCSrc);
	m_hBufferOldBmp = ::SelectObject(m_dcBuffer, 
		(HGDIOBJ)Util::CreateDIBSection32(m_rcDst.Width(), m_rcDst.Height()));

	switch(m_DrawImageType)
	{
		case DIT_NORMAL: return DrawNormal(m_dcBuffer); break;
		case DIT_STRETCH: return DrawStretch(m_dcBuffer); break;
		case DIT_9PART: return Draw9Part(m_dcBuffer); break;
		case DIT_3PARTH: return Draw3PartH(m_dcBuffer); break;
		case DIT_3PARTV: return Draw3PartV(m_dcBuffer); break;
	}

	return FALSE;
}

BOOL CXImageGdiPlus::Load( LPCTSTR szFile )
{
	if (!szFile)
		return FALSE;

	if (szFile[0] == _T('@'))
	{
		szFile++;
		if (!LoadAsResource(szFile))
			return FALSE;
	}
	else
	{
		if (!m_Image.Load(szFile))
			return FALSE;
	}

	ReleaseBufferDC();

	LoadFormattedImageInfo(::PathFindFileName(szFile));

	InitSrcRect();

	return TRUE;
}

BOOL CXImageGdiPlus::SetDstRect( const CRect &rcDst )
{
	if (rcDst == m_rcDst)
		return TRUE;

	if (rcDst.Width() != m_rcDst.Width() || rcDst.Height() != m_rcDst.Height())
		ReleaseBufferDC();
	
	m_rcDst = rcDst;

	return TRUE;
}

BOOL CXImageGdiPlus::SetSrcRect( const CRect &rcSrc )
{
	if (rcSrc == m_rcSrc)
		return TRUE;

	ReleaseBufferDC();
	m_rcSrc = rcSrc;

	return TRUE;
}

INT CXImageGdiPlus::GetImageHeight()
{
	if (m_Image.IsValid())
		return m_Image.GetImageHeight();

	return 0;
}

INT CXImageGdiPlus::GetImageWidth()
{
	if (m_Image.IsValid())
		return m_Image.GetImageWidth();

	return 0;
}

VOID CXImageGdiPlus::InitSrcRect()
{
	m_rcSrc.left = m_rcSrc.top = 0;
	m_rcSrc.right = GetImageWidth();
	m_rcSrc.bottom = GetImageHeight();
}

BOOL CXImageGdiPlus::SetAlpha( BYTE cAlpha )
{
	m_cAlapha = cAlpha;
	return TRUE;
}

VOID CXImageGdiPlus::LoadFormattedImageInfo( LPCTSTR szFileName )
{
	m_bFormattedImage = TRUE;

	if (StrStrI(szFileName, _T(".normal.")))
		m_DrawImageType = DIT_NORMAL;
	else if (StrStrI(szFileName, _T(".stretch.")))
		m_DrawImageType = DIT_STRETCH;
	else if (StrStrI(szFileName, _T(".9.")))
	{
		m_DrawImageType = DIT_9PART;
		LoadFormattedImagePartInfo();
		m_Image.Clip(1, 1, m_Image.GetImageWidth() - 2, m_Image.GetImageHeight() - 2);
	}
	else
		m_bFormattedImage = FALSE;
}

VOID CXImageGdiPlus::LoadFormattedImagePartInfo()
{
	// 9-part images have 1-pixel borders on each side.   
	const INT nImageWidth = m_Image.GetImageWidth() - 2;
	const INT nImageHeight = m_Image.GetImageHeight() - 2;

	// X
	INT i;
	for (i = 0; i < nImageWidth; i++)
		// The part info is on the border. 
		if (m_Image.GetARGB(i + 1, 0) == 0xFF000000)
		{
			m_rcPart.left = i;
			break;
		}

	if (i >= nImageWidth)
	{
		m_rcPart.left = 0;
		m_rcPart.right = nImageWidth;
	}
	else
	{
		for (i++; i < nImageWidth; i++)
			if (m_Image.GetARGB(i + 1, 0) != 0xFF000000)
			{
				m_rcPart.right = i;
				break;
			}

		if (i >= nImageWidth)
			m_rcPart.right = nImageWidth;
	}

	// Y
	INT j;
	for (j = 0; j < nImageHeight; j++)
		// The part info is on the border. 
		if (m_Image.GetARGB(0, j + 1) == 0xFF000000)
		{
			m_rcPart.top = j;
			break;
		}

	if (j >= nImageHeight)
	{
		m_rcPart.top = 0;
		m_rcPart.bottom = nImageHeight;
	}
	else
	{
		for (j++; j < nImageHeight; j++)
			if (m_Image.GetARGB(0, j + 1) != 0xFF000000)
			{
				m_rcPart.bottom = j;
				break;
			}

		if (j >= nImageHeight)
			m_rcPart.bottom = nImageHeight;
	}
}

BOOL CXImageGdiPlus::IsFormattedImage()
{
	return m_bFormattedImage;
}

BOOL CXImageGdiPlus::LoadAsResource( LPCTSTR pRelativePath )
{
	if (!pRelativePath)
		return FALSE;

	CXResourceMgr & ResourceMgr = CXResourceMgr::Instance();

	CString strFullPath = ResourceMgr.GetResourcePath(pRelativePath);

	if (strFullPath.IsEmpty())
		return FALSE;

	if (!ResourceMgr.IsPackedPath(strFullPath))
		return m_Image.Load(strFullPath);

	SIZE_T szResourceSize = ResourceMgr.GetPackedResourceSize(strFullPath);

	if (!szResourceSize)
		return FALSE;

	BYTE *pData = (BYTE *)::GlobalAlloc(GMEM_FIXED, szResourceSize);

	if (!ResourceMgr.GetPackedResource(strFullPath, pData, szResourceSize))
	{
		GlobalFree((HGDIOBJ)pData);
		return FALSE;
	}

	CComPtr<IStream> spDataStream = NULL;
	if (FAILED(CreateStreamOnHGlobal((HGDIOBJ)pData, TRUE, &spDataStream)) || !spDataStream)
	{
		GlobalFree((HGDIOBJ)pData);
		return FALSE;
	}

	return m_Image.Load(spDataStream);
}


