#include "StdAfx.h"

#include "XScrollBar.h"
#include "..\Base\XResourceMgr.h"

CXScrollBar::CXScrollBar()
	: m_pImageBar(NULL),
	m_nFrameLen(0), m_nViewLen(0), m_nPos(0),
	m_rcBar(0, 0, 0, 0), 
	m_bVisibleState(FALSE),
	m_bMouseDown(FALSE), m_ptLastMousePt(0, 0)
{

}


BOOL CXScrollBar::SetFrameLen( INT nLen )
{
	if (m_nFrameLen == nLen)
		return TRUE;

	m_nFrameLen = nLen;
	UpdateScrollBar();

	return TRUE;
}

BOOL CXScrollBar::SetViewLen( INT nLen )
{
	if (m_nViewLen == nLen)
		return TRUE;

	m_nViewLen = nLen;
	UpdateScrollBar();

	return TRUE;
}


BOOL CXScrollBar::SetScrollPos( INT nPos )
{
	if (m_nPos == nPos)
		return TRUE;

	if (nPos > m_nFrameLen - m_nViewLen)
		nPos = m_nFrameLen - m_nViewLen;
	if (nPos < 0)
		nPos = 0;

	m_nPos = nPos;
	UpdateScrollBar();

	ThrowEvent(EVENT_SCROLLBAR_SCROLLCHANGED, nPos, 0);

	return TRUE;
}


IXImage * CXScrollBar::SetBarImage( IXImage *pImage )
{
	if (m_pImageBar == pImage)
		return NULL;

	IXImage *pOldImage = m_pImageBar;

	m_pImageBar = pImage;

	if (m_pImageBar)
		m_pImageBar->SetDstRect(m_rcBar);

	InvalidateRect(m_rcBar);

	return pOldImage;
}

VOID CXScrollBar::UpdateScrollBar()
{
	if (!m_bVisibleState)
	{
		__super::SetVisible(FALSE);
		return;
	}

	if (m_nViewLen == 0)
	{
		__super::SetVisible(FALSE);
		return;
	}

	if (m_nViewLen >= m_nFrameLen)
	{
		__super::SetVisible(FALSE);
		return;
	}

	CSize szBox = GetRect().Size();
	CSize szBar(m_Type == SCROLL_H ? szBox.cx * m_nViewLen / m_nFrameLen : szBox.cx , 
		m_Type == SCROLL_V ? szBox.cy * m_nViewLen / m_nFrameLen : szBox.cy );

	CRect rcBarOld(m_rcBar);

	if (m_Type == SCROLL_H)
	{
		m_rcBar.top = 0;
		m_rcBar.bottom = szBar.cy;
		m_rcBar.left = m_nPos * szBox.cx / m_nFrameLen;
		m_rcBar.right = m_rcBar.left + szBar.cx;
	}
	else
	{
		m_rcBar.left = 0;
		m_rcBar.right = szBar.cx;
		m_rcBar.top = m_nPos * szBox.cy / m_nFrameLen;
		m_rcBar.bottom = m_rcBar.top + szBar.cy;
	}

	if (m_pImageBar)
		m_pImageBar->SetDstRect(m_rcBar);

	if (IsVisible())
	{
		InvalidateRect(rcBarOld);
		InvalidateRect(m_rcBar);
	}
	else
	{
		__super::SetVisible(TRUE);
	}
}

BOOL CXScrollBar::PaintForeground( HDC hDC, const CRect &rect )
{
	if (m_pImageBar)
		m_pImageBar->Draw(hDC, rect);
	__super::PaintForeground(hDC, rect);

	return 0;
}

BOOL CXScrollBar::SetVisible( BOOL bVisible )
{
	if ((m_bVisibleState && bVisible) || (!m_bVisibleState && !bVisible))
		return TRUE;

	m_bVisibleState = bVisible;

	UpdateScrollBar();

	return TRUE;
}

LRESULT CXScrollBar::OnLButtonDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	bHandled = TRUE;

	CXFrameMsgMgr *pMsgMgr = GetFrameMsgMgr();

	CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	if (m_rcBar.PtInRect(pt))
	{	
		if (pMsgMgr)
		{
			m_ptLastMousePt = CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			pMsgMgr->CaptureMouse(this);
			m_bMouseDown = TRUE;
		}
	}
	else
	{
		INT nDirection = -1;

		if (m_Type == SCROLL_H && pt.x >= m_rcBar.right)
			nDirection = 1;
		if (m_Type == SCROLL_V && pt.y >= m_rcBar.bottom)
			nDirection = 1;

		SetScrollPos(GetScrollPos() + nDirection * m_nViewLen);
	}

	return 0;
}

INT CXScrollBar::GetScrollPos()
{
	return m_nPos;
}

LRESULT CXScrollBar::OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	if (!m_bMouseDown)
		return 0;

	INT nOffset = 0;

	CPoint ptMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	if (m_Type == SCROLL_H)
		nOffset = ptMouse.x - m_ptLastMousePt.x;
	else
		nOffset = ptMouse.y - m_ptLastMousePt.y;
	m_ptLastMousePt = ptMouse;

	if (nOffset != 0)
	{
		if (m_Type == SCROLL_H)
			SetScrollPos(GetScrollPos() + nOffset * m_nFrameLen / GetRect().Width());
		else
			SetScrollPos(GetScrollPos() + nOffset * m_nFrameLen / GetRect().Height());
	}

	return 0;
}

LRESULT CXScrollBar::OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	bHandled = TRUE;

	CXFrameMsgMgr *pMsgMgr = GetFrameMsgMgr();
	if (pMsgMgr)
		pMsgMgr->ReleaseCaptureMouse(this);

	if (m_bMouseDown)
		m_bMouseDown = FALSE;

	if (pMsgMgr)
		pMsgMgr->GetFocus(this);

	return 0;
}

LRESULT CXScrollBar::OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	SHORT nDis = HIWORD(wParam);
	nDis = nDis / WHEEL_DELTA;

	if (m_Type == SCROLL_H)
		SetScrollPos(GetScrollPos() - nDis * 10);
	else
		SetScrollPos(GetScrollPos() - nDis * 10);

	return 0;
}

VOID CXScrollBar::Destroy()
{
	delete SetBarImage(NULL);	
	
	return __super::Destroy();
}

BOOL CXScrollBar::Create( CXFrame * pFrameParent,  ScrollType type, const CRect & rcRect /*= CRect(0, 0, 0, 0)*/, BOOL bVisible /*= FALSE*/, 
						 IXImage * pBarBackground /*= NULL*/, IXImage * pBarImage /*= NULL*/, 
						 WIDTH_MODE aWidthMode /*= WIDTH_MODE_NOT_CHANGE*/, HEIGHT_MODE aHeightMode /*= HEIGHT_MODE_NOT_CHANGE*/ )
{
	BOOL bRtn = __super::Create(pFrameParent, rcRect, bVisible, aWidthMode, aHeightMode);

	m_Type = type;

	if (!pBarBackground)
	{
		if (type == SCROLL_H)
			pBarBackground = CXResourceMgr::GetImage(_T("img/ctrl/scroll_bkgH.9.png"));
		else
			pBarBackground = CXResourceMgr::GetImage(_T("img/ctrl/scroll_bkg.9.png"));
	}

	if (!pBarImage)
	{
		if (type == SCROLL_H)
			pBarImage = CXResourceMgr::GetImage(_T("img/ctrl/scrollH.9.png"));
		else
			pBarImage = CXResourceMgr::GetImage(_T("img/ctrl/scroll.9.png"));
	}

	delete SetBarImage(pBarImage);

	delete SetBackground(pBarBackground);

	return bRtn;
}

VOID CXScrollBar::ChangeFrameRect( const CRect & rcNewFrameRect )
{
	if (GetRect() == rcNewFrameRect)
		return;

	__super::ChangeFrameRect(rcNewFrameRect);

	UpdateScrollBar();
}