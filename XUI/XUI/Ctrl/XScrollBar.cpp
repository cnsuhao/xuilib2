#include "StdAfx.h"

#include "XScrollBar.h"
#include "..\Base\XResourceMgr.h"

CXScrollBar::CXScrollBar()
	: m_pImageBar(NULL),
	m_nContentLen(0), m_nViewLen(0), m_nPos(0),
	m_rcBar(0, 0, 0, 0), 
	m_bVisibleState(FALSE),
	m_bMouseDown(FALSE), m_ptMouseDownPt(0, 0),
	m_nMouseDownScrollPos(0)
{

}


BOOL CXScrollBar::SetContentLen( INT nLen )
{
	if (m_nContentLen == nLen)
		return TRUE;

	m_nContentLen = nLen;
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

	if (m_nPos > m_nContentLen - m_nViewLen)
		m_nPos = m_nContentLen - m_nViewLen;
	if (m_nPos < 0)
		m_nPos = 0;

	if (!m_bVisibleState)
	{
		__super::SetVisibility(VISIBILITY_HIDE);
		return;
	}

	if (m_nViewLen == 0)
	{
		__super::SetVisibility(VISIBILITY_HIDE);
		return;
	}

	if (m_nViewLen >= m_nContentLen)
	{
		__super::SetVisibility(VISIBILITY_HIDE);
		return;
	}

	CSize szBox = GetRect().Size();
	CSize szBar(m_Type == SCROLL_H ? szBox.cx * m_nViewLen / m_nContentLen : szBox.cx , 
		m_Type == SCROLL_V ? szBox.cy * m_nViewLen / m_nContentLen : szBox.cy );

	CRect rcBarOld(m_rcBar);

	if (m_Type == SCROLL_H)
	{
		m_rcBar.top = 0;
		m_rcBar.bottom = szBar.cy;
		m_rcBar.left = m_nPos * szBox.cx / m_nContentLen;
		m_rcBar.right = m_rcBar.left + szBar.cx;
	}
	else
	{
		m_rcBar.left = 0;
		m_rcBar.right = szBar.cx;
		m_rcBar.top = m_nPos * szBox.cy / m_nContentLen;
		m_rcBar.bottom = m_rcBar.top + szBar.cy;
	}

	if (m_pImageBar)
		m_pImageBar->SetDstRect(m_rcBar);

	if (GetVisibility() == VISIBILITY_SHOW)
	{
		InvalidateRect(rcBarOld);
		InvalidateRect(m_rcBar);
	}
	else
	{
		__super::SetVisibility(VISIBILITY_SHOW);
	}
}

BOOL CXScrollBar::PaintForeground( HDC hDC, const CRect &rect )
{
	if (m_pImageBar)
		m_pImageBar->Draw(hDC, rect);

	__super::PaintForeground(hDC, rect);

	return 0;
}

BOOL CXScrollBar::SetVisibility( VISIBILITY visibility )
{
	if ((m_bVisibleState && visibility == VISIBILITY_SHOW) || 
		(!m_bVisibleState && visibility != VISIBILITY_SHOW))
		return TRUE;

	m_bVisibleState =  visibility == VISIBILITY_SHOW;

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
			m_ptMouseDownPt = CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			m_nMouseDownScrollPos = GetScrollPos();
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

		NotifyScrollChange();
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
		nOffset = ptMouse.x - m_ptMouseDownPt.x;
	else
		nOffset = ptMouse.y - m_ptMouseDownPt.y;

	if (nOffset != 0)
	{
		if (m_Type == SCROLL_H)
			SetScrollPos(m_nMouseDownScrollPos + nOffset * m_nContentLen / GetRect().Width());
		else
			SetScrollPos(m_nMouseDownScrollPos + nOffset * m_nContentLen / GetRect().Height());

		NotifyScrollChange();
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

	NotifyScrollChange();

	return 0;
}

VOID CXScrollBar::Destroy()
{
	delete SetBarImage(NULL);

	m_nContentLen = 0;
	m_nViewLen = 0; 
	m_nPos = 0;
	m_rcBar.left = m_rcBar.top = m_rcBar.right = m_rcBar.bottom = 0;
	m_bVisibleState = FALSE;
	m_bMouseDown = FALSE;
	m_ptMouseDownPt.x = m_ptMouseDownPt.y = 0;
	m_nMouseDownScrollPos = 0;
	
	return __super::Destroy();
}

BOOL CXScrollBar::Create( CXFrame * pFrameParent,  ScrollType type, LayoutParam * pLayout,  VISIBILITY visibility /*= VISIBILITY_NONE*/, 
						 IXImage * pBarBackground /*= NULL*/, IXImage * pBarImage /*= NULL*/)
{
	if (!pLayout) 
	{
		ATLASSERT(!_T("No layout parameter. "));
		return NULL;
	}

	BOOL bRtn = __super::Create(pFrameParent, pLayout, visibility);

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

BOOL CXScrollBar::SetRect( const CRect & rcNewFrameRect )
{
	if (GetRect() == rcNewFrameRect)
		return TRUE;

	BOOL bRtn = __super::SetRect(rcNewFrameRect);

	UpdateScrollBar();

	return bRtn;
}

VOID CXScrollBar::NotifyScrollChange()
{
	ThrowEvent(EVENT_SCROLLBAR_SCROLLCHANGED, m_nPos, 0);
}

