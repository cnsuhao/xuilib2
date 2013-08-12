#include "StdAfx.h"
#include "XCaret.h"

CXCaret::CXCaret(void)
	: m_hBitmap(NULL),
	m_pBitmapBits(NULL),
	m_nBitmapWidth(0),
	m_nBitmapHeight(0),
	m_nBlinkTimer(0),
	m_bBlinkShow(FALSE)
{
}

VOID CXCaret::Destroy()
{
	SetCaretBlinkTime(0);

	m_bBlinkShow = FALSE;

	m_hBitmap = 0;
	if (m_pBitmapBits)
	{
		delete [] m_pBitmapBits;
		m_pBitmapBits = NULL;
	}
	m_nBitmapWidth = m_nBitmapHeight = 0;

	__super::Destroy();
}

BOOL CXCaret::SetCaretBlinkTime( UINT uMSeconds )
{
	if (m_nBlinkTimer)
	{
		KillTimer(m_nBlinkTimer);
		m_nBlinkTimer = 0;
	}

	if (uMSeconds)
		m_nBlinkTimer = SetTimer(uMSeconds);

	return TRUE;
}

BOOL CXCaret::SetCaretShape( HBITMAP hBitmap, int nWidth, int nHeight )
{
	if (hBitmap != 0 && hBitmap != (HBITMAP)1 && m_hBitmap == hBitmap)
		return TRUE;

	m_hBitmap = 0;
	if (m_pBitmapBits)
	{
		delete [] m_pBitmapBits;
		m_pBitmapBits = NULL;
	}
	m_nBitmapWidth = 0;
	m_nBitmapHeight = 0;

	if (hBitmap != 0 && hBitmap != (HBITMAP)1)
	{
 		BITMAP info;
 		ZeroMemory(&info, sizeof(info));
 		if (::GetObject(hBitmap, sizeof(info), &info) && // A valid bitmap. 
			info.bmBits == 0 && // A DDB. 
			info.bmBitsPixel == 1) // Only support 1-bit-pixel bitmap.
		{
			nWidth = m_nBitmapWidth = info.bmWidth;
			nHeight = m_nBitmapHeight = info.bmHeight;
			const UINT nBitmapByteCount = info.bmWidth * info.bmHeight / 8 + 1;
			m_pBitmapBits = new BYTE[nBitmapByteCount];
			::GetBitmapBits(hBitmap, nBitmapByteCount, m_pBitmapBits);
			m_hBitmap = hBitmap;
		}
		else
			nWidth = nHeight = 0;
	}
	else
		m_hBitmap = hBitmap;

	CRect rcFrame(GetRect());
	SetRect(CRect(rcFrame.left, rcFrame.top, rcFrame.left + nWidth, rcFrame.top + nHeight));

	InvalidateRect();

	return TRUE;
}

VOID CXCaret::ChangeFrameRect( const CRect & rcNewFrameRect )
{
	if (GetRect() == rcNewFrameRect)
		return;

	if (m_hBitmap != 0 && m_hBitmap != (HBITMAP)1 &&
		(rcNewFrameRect.Width() != m_nBitmapWidth || rcNewFrameRect.Height() != m_nBitmapHeight))
		return;

	__super::ChangeFrameRect(rcNewFrameRect);
}

LRESULT CXCaret::OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	if (wParam != m_nBlinkTimer)
		return -1;

	m_bBlinkShow = !m_bBlinkShow;

	InvalidateRect();

	return 0;
}

BOOL CXCaret::PaintForeground( HDC hDC, const CRect &rcUpdate )
{
	if (rcUpdate.IsRectEmpty())
		return TRUE;

	if (m_bBlinkShow)
	{
		BYTE *pBmpBufferByte = NULL;
		CRect rcBmpBuffer(0, 0, rcUpdate.Width(), rcUpdate.Height());
		HBITMAP hBmpBuffer = Util::CreateDIBSection32(rcBmpBuffer.Width(), rcBmpBuffer.Height(), &pBmpBufferByte);
		if (pBmpBufferByte)
		{
			const UINT nBmpBufferSize = rcUpdate.Width() * rcUpdate.Height() * 4;
			HDC dcBmpBuffer = ::CreateCompatibleDC(hDC);
			HGDIOBJ hOldBmp = ::SelectObject(dcBmpBuffer, (HGDIOBJ)hBmpBuffer);

			Util::BitBlt(hDC, rcUpdate, dcBmpBuffer, rcBmpBuffer);

			if (m_hBitmap == 0 || m_hBitmap == (HBITMAP)1)
				for (UINT i = 0; i < nBmpBufferSize;)
				{
					UINT32 *p = (UINT32 *)(pBmpBufferByte + i);
					*p = ~*p;
					pBmpBufferByte[i + 3] = ~pBmpBufferByte[i + 3];

					if (m_hBitmap == 0)
						i += 4; // solid caret
					else
						i += 8; // gray caret
				}
			else
			{
				if (m_pBitmapBits)
					for (UINT i = 0; i < nBmpBufferSize; i += 4)
					{
						UINT nLine = (i / 4 / rcBmpBuffer.Width()) + (m_nBitmapHeight - rcUpdate.bottom);
						UINT nCow = (i / 4 % rcBmpBuffer.Width()) + rcUpdate.left;
						UINT nIndex = nLine * m_nBitmapWidth + nCow;
						if (m_pBitmapBits[nIndex / 8] & (  (UINT8)0x80 >> (nIndex % 8) ))
						{
							UINT32 *p = (UINT32 *)(pBmpBufferByte + i);
							*p = ~*p;
							pBmpBufferByte[i + 3] = ~pBmpBufferByte[i + 3];
						}
					}
			}

			Util::BitBlt(dcBmpBuffer, rcBmpBuffer, hDC, rcUpdate);

			::SelectObject(dcBmpBuffer, hOldBmp);
			::DeleteObject((HGDIOBJ)dcBmpBuffer);
			::DeleteDC(dcBmpBuffer);
		}

	}

	return __super::PaintBackground(hDC, rcUpdate);
}

BOOL CXCaret::Create( CXFrame * pFrameParent, const CRect & rcRect /*= CRect(0, 0, 0, 0)*/, BOOL bVisible /*= FALSE*/, 
					 UINT nBlinkTime /*= 500*/, 
					 WIDTH_MODE aWidthMode /*= WIDTH_MODE_NOT_CHANGE*/, HEIGHT_MODE aHeightMode /*= HEIGHT_MODE_NOT_CHANGE*/ )
{
	BOOL bRtn = __super::Create(pFrameParent, rcRect, bVisible, aWidthMode, aHeightMode);

	SetCaretBlinkTime(nBlinkTime);

	return bRtn;
}
