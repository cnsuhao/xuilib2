#include "StdAfx.h"
#include "XFrame.h"

#include <algorithm>

#include "..\..\..\XLib\inc\interfaceS\string\StringCode.h"

#include "XMessageService.h"
#include "XResourceMgr.h"

#include <list>

std::vector<BOOL> CXFrame::s_bActiveInstances;

X_IMPLEMENT_FRAME_XML_RUNTIME(CXFrame)
X_IMPLEMENT_DEFAULT_FRAME_XML_FACTORY(CXFrame)

CXFrame::CXFrame(void)
	: m_pFrameParent(NULL),
	m_pDrawBackground(NULL),
	m_bTouchable(FALSE),
	m_pMouseOverLayer(NULL),
	m_pMouseDownLayer(NULL),
	m_bSelectable(FALSE),
	m_bSelectWhenMouseCilck(FALSE),
	m_bUnselectWhenMouseCilck(FALSE),
	m_bSelectedState(FALSE),
	m_pSelectedLayer(FALSE),
	m_bMouseOver(FALSE),
	m_bMouseDown(FALSE),
	m_bVisible(FALSE),
	m_bHoldPlace(TRUE),
	m_rcFrame(0, 0, 0, 0),
	m_rcMargin(0, 0, 0, 0),
	m_dwInstanceID(RegisterInstanceID()),
	m_WidthMode(WIDTH_MODE_NORMAL),
	m_HeightMode(HEIGHT_MODE_NORMAL),
	m_bIsFrameAlive(FALSE)
{
	Util::InitGdiPlus();
}

CXFrame:: ~CXFrame(void)
{
	ATLASSERT(!m_bIsFrameAlive || !_T("We MUST destroy a frame before destroy the frame object. "));

	Util::UninitGdiPlus();
	UnregisterInstanceID(m_dwInstanceID);
}

BOOL CXFrame::Create( CXFrame * pFrameParent, const CRect & rcRect, BOOL bVisible, 
					 WIDTH_MODE aWidthMode, HEIGHT_MODE aHeightMode)
{
	ATLASSERT(!m_bIsFrameAlive || !_T("Already created. "));
	m_bIsFrameAlive = TRUE;

	SetWidthHeightMode(aWidthMode, aHeightMode);

	SetRect(rcRect);

	SetParent(pFrameParent);

	SetVisible(bVisible);

	return TRUE;
}

VOID CXFrame::Destroy()
{
	ATLASSERT(m_bIsFrameAlive || !_T("Already destroyed or not created. "));
	m_bIsFrameAlive = FALSE;

	SetVisible(FALSE);

	SetParent(NULL);

	while (UINT nFrameCount = GetFrameCount())
	{
		CXFrame *pFrame = GetFrameByIndex(nFrameCount - 1);
		if (pFrame)
		{
			pFrame->Destroy();
			delete pFrame;
		}
	}

	SetWidthHeightMode(WIDTH_MODE_NORMAL, HEIGHT_MODE_NORMAL);

	SetRect(CRect(0, 0, 0, 0));

	SetMargin(CRect(0, 0, 0, 0));

	delete SetBackground(NULL);

	SetHoldPlace(TRUE);

	SetTouchable(FALSE);
	delete SetMouseOverLayer(NULL);
	delete SetMouseDownLayer(NULL);
	
	SetSelectable(FALSE);
	SetSelectWhenMouseClick(FALSE);
	SetUnselectWhenMouseClick(FALSE);
	SetSelectedState(FALSE);
	delete SetSelectedLayer(NULL);

	m_bMouseOver = FALSE;
	m_bMouseDown = FALSE;

	m_mapEventListener.clear();
}

UINT CXFrame::GetFrameCount()
{
	return m_vecFrameChild.size();
}

CXFrame * CXFrame::GetFrameByIndex( UINT nIndex )
{
	if (nIndex >= GetFrameCount())
		return NULL;

	return m_vecFrameChild[nIndex];
}

UINT CXFrame::GetFrameIndex( CXFrame * pFrame )
{
	if (!pFrame)
		return INVALID_FRAME_INDEX;

	std::vector<CXFrame *>::iterator iterFind = 
		std::find(m_vecFrameChild.begin(), m_vecFrameChild.end(), pFrame);
	
	if (iterFind == m_vecFrameChild.end())
		return INVALID_FRAME_INDEX;

	return iterFind - m_vecFrameChild.begin();
}

BOOL CXFrame::AddFrame(CXFrame * pFrame)
{
	return InsertFrame(pFrame, GetFrameCount());
}

BOOL CXFrame::RemoveFrame(CXFrame * pFrame)
{
	if (pFrame == NULL)
		return FALSE;

	std::vector<CXFrame *>::iterator iterFind = 
		std::find(m_vecFrameChild.begin(), m_vecFrameChild.end(), pFrame);
	if (iterFind == m_vecFrameChild.end())
		return FALSE;

	return RemoveFrame(iterFind - m_vecFrameChild.begin()) != NULL;
}

CXFrame * CXFrame::RemoveFrame( UINT nIndex )
{
	if (nIndex >= m_vecFrameChild.size())
		return NULL;

	CXFrame *pFrame = m_vecFrameChild[nIndex];

	if (!pFrame)
	{
		ATLASSERT(NULL);
		return NULL;
	}

	m_vecFrameChild.erase(m_vecFrameChild.begin() + nIndex);
	pFrame->OnDetachedFromParent();
	
	if (m_WidthMode == WIDTH_MODE_WRAP_CONTENT ||
		m_HeightMode == HEIGHT_MODE_WRAP_CONTENT)
	{
		pFrame->RemoveEventListener(this);
		RefreashFrameRect();
	}

	if (pFrame->IsVisible() && IsVisible())
	{
		InvalidateRect(pFrame->GetRect());
	}

	return pFrame;
}

BOOL CXFrame::InsertFrame(CXFrame * pFrame, UINT nIndex)
{
	if (pFrame == NULL)
		return FALSE;

	std::vector<CXFrame *>::iterator iterFind = 
		std::find(m_vecFrameChild.begin(), m_vecFrameChild.end(), pFrame);
	if (iterFind != m_vecFrameChild.end())
		return FALSE;

	if (nIndex > m_vecFrameChild.size())
		nIndex = m_vecFrameChild.size();

	m_vecFrameChild.insert(m_vecFrameChild.begin() + nIndex, pFrame);
	pFrame->OnAttachedToParent(this);

	if (m_WidthMode == WIDTH_MODE_WRAP_CONTENT || 
		m_HeightMode == HEIGHT_MODE_WRAP_CONTENT)
	{
		RefreashFrameRect();
		pFrame->AddEventListener(this);
	}


	if (pFrame->IsVisible() && IsVisible())
	{
		InvalidateRect(pFrame->GetRect());
	}

	return TRUE;
}

BOOL CXFrame::SetParent(CXFrame * pFrameParent)
{
	if (m_pFrameParent == pFrameParent)
		return TRUE;

	if (m_pFrameParent)
		m_pFrameParent->RemoveFrame(this);

	if (pFrameParent)
		pFrameParent->AddFrame(this);

	return TRUE;
}

CXFrame * CXFrame::GetParent()
{
	return m_pFrameParent;
}

IXImage *  CXFrame::SetBackground(IXImage * pDrawBackground)
{
	if (m_pDrawBackground == pDrawBackground)
		return NULL;

	IXImage *pOldBackground = m_pDrawBackground;

	m_pDrawBackground = pDrawBackground;

	if (m_pDrawBackground)
		m_pDrawBackground->SetDstRect(CRect(0, 0, m_rcFrame.Width(), m_rcFrame.Height()));

	if (m_WidthMode == WIDTH_MODE_ADAPT_BACKGROUND ||
		m_HeightMode == HEIGHT_MODE_ADAPT_BACKGROUND)
		RefreashFrameRect();

	if (IsVisible())
		InvalidateRect();

	return pOldBackground;
}

IXImage * CXFrame::GetBackground()
{
	return m_pDrawBackground;
}


VOID CXFrame::CalculateFrameRect( CRect *pRect )
{
	if (!pRect)
		return;

	CRect & rcFrame = *pRect;

	switch (m_WidthMode)
	{
	case WIDTH_MODE_REACH_PARENT:
		rcFrame.right = m_pFrameParent ? 
			max(m_pFrameParent->GetRect().Width() - m_rcMargin.right, rcFrame.left) : rcFrame.left;
		break;
	case WIDTH_MODE_WRAP_CONTENT:
		rcFrame.right = rcFrame.left + CalculateWrapContentWidth();
		break;
	case WIDTH_MODE_ADAPT_BACKGROUND:
		rcFrame.right = rcFrame.left + CalculateAdaptBackgroundWidth();
		break;
	}

	switch (m_HeightMode)
	{
	case HEIGHT_MODE_REACH_PARENT:
		rcFrame.bottom = m_pFrameParent ? 
			max(m_pFrameParent->GetRect().Height() - m_rcMargin.bottom, rcFrame.top) : rcFrame.top;
		break;
	case HEIGHT_MODE_WRAP_CONTENT:
		rcFrame.bottom = rcFrame.top + CalculateWrapContentHeight();
		break;
	case HEIGHT_MODE_ADAPT_BACKGROUND:
		rcFrame.bottom = rcFrame.top + CalculateAdaptBackgroundHeight();
		break;
	}
}

VOID CXFrame::ChangeFrameRect( const CRect & rcNewFrameRect )
{
	if (m_rcFrame == rcNewFrameRect)
		return;

	CRect rcFrameRectOld(m_rcFrame);
	m_rcFrame = rcNewFrameRect;

	CRect rcFrameDrawArea(0, 0, rcNewFrameRect.Width(), rcNewFrameRect.Height());
	if (m_pDrawBackground)
		m_pDrawBackground->SetDstRect(rcFrameDrawArea);
	if (m_pMouseOverLayer)
		m_pMouseOverLayer->SetDstRect(rcFrameDrawArea);
	if (m_pMouseDownLayer)
		m_pMouseDownLayer->SetDstRect(rcFrameDrawArea);
	if (m_pSelectedLayer)
		m_pSelectedLayer->SetDstRect(rcFrameDrawArea);

	if (IsVisible())
	{
		if (m_pFrameParent)
		{
			if (m_pFrameParent->IsVisible())
			{
				if (!rcFrameRectOld.IsRectEmpty())
					m_pFrameParent->InvalidateRect(rcFrameRectOld);
				if (!rcNewFrameRect.IsRectEmpty())
					m_pFrameParent->InvalidateRect(rcNewFrameRect);
			}
		}
		else
		{
			if (rcFrameRectOld.Width() != rcNewFrameRect.Width() ||
				rcFrameRectOld.Height() != rcNewFrameRect.Height())
			InvalidateRect();
		}
	}

	ThrowEvent(EVENT_FRAME_RECT_CHANGED, (WPARAM)&rcFrameRectOld, 0);
}

BOOL CXFrame::SetRect(const CRect &rc)
{
	CRect rcFrame(rc);

	CalculateFrameRect(&rcFrame);
	
	if (m_rcFrame == rcFrame)
		return TRUE;

	ChangeFrameRect(rcFrame);

	return TRUE;
}

CRect CXFrame::GetRect()
{
	return m_rcFrame;
}

BOOL CXFrame::SetVisible(BOOL bVisible)
{
	if( (m_bVisible && bVisible) || (!m_bVisible && !bVisible))
		return TRUE;

	m_bVisible = bVisible;

	if (m_bVisible)
	{
		InvalidateRect();
	}
	else
	{
		if (m_pFrameParent && m_pFrameParent->IsVisible())
		{
			m_pFrameParent->InvalidateRect(m_rcFrame);
		}
	}

	ThrowEvent(EVENT_FRAME_SHOWHIDE_CHANGED, (WPARAM)m_bVisible, 0);
	
	return TRUE;
}

BOOL CXFrame::IsVisible()
{
	return m_bVisible;
}


BOOL CXFrame::Update()
{
	if (!IsVisible())
		return TRUE;

	CXFrame * pFrameParent = GetParent();
	if (!pFrameParent)
		return FALSE;

	return pFrameParent->Update();
}


BOOL CXFrame::InvalidateRect(const CRect & rect)
{
	if (!IsVisible())
		return TRUE;

	if (rect.IsRectEmpty())
		return TRUE;

	CRect rcReal(0, 0, 0, 0);
	if (!IntersectRect(&rcReal, &rect, &CRect(0, 0, m_rcFrame.Width(), m_rcFrame.Height())) || rcReal.IsRectEmpty())
		return TRUE;

	CXFrame * pFrameParent = GetParent();
	if (!pFrameParent)
		return FALSE;

	return pFrameParent->InvalidateRect(ChildToParent(rcReal));
}

BOOL CXFrame::InvalidateRect()
{
	return InvalidateRect(CRect(0, 0, m_rcFrame.Width(), m_rcFrame.Height()));
}

HWND CXFrame::GetHWND()
{
	CXFrame * pFrameParent = GetParent();
	if (!pFrameParent)
		return NULL;

	return pFrameParent->GetHWND();
}

CRect CXFrame::FrameToWindow(const CRect &rc)
{
	CPoint ptLeftTop(rc.TopLeft());
	CPoint ptRightBottom(rc.BottomRight());

	ptLeftTop = FrameToWindow(ptLeftTop);
	ptRightBottom = FrameToWindow(ptRightBottom);

	return CRect(ptLeftTop, ptRightBottom);
}

CPoint CXFrame::FrameToWindow( const CPoint &pt )
{
	CXFrame * pFrameParent = GetParent();
	if (!pFrameParent)
		return CPoint(0, 0);

	return pFrameParent->FrameToWindow(ChildToParent(pt));
}


CRect CXFrame::ScreenToFrame( const CRect &rc )
{
	CPoint ptLeftTop(rc.TopLeft());
	CPoint ptRightBottom(rc.BottomRight());

	ptLeftTop = ScreenToFrame(ptLeftTop);
	ptRightBottom = ScreenToFrame(ptRightBottom);

	return CRect(ptLeftTop, ptRightBottom);
}

CPoint CXFrame::ScreenToFrame( const CPoint &pt )
{
	CXFrame * pFrameParent = GetParent();
	if (!pFrameParent)
		return CPoint(0, 0);

	return ParentToChild(pFrameParent->ScreenToFrame(pt));
}

CPoint CXFrame::FrameToScreen( const CPoint &pt )
{
	CXFrame * pFrameParent = GetParent();
	if (!pFrameParent)
		return CPoint(0, 0);

	return pFrameParent->FrameToScreen(ChildToParent(pt));
}

CRect CXFrame::FrameToScreen( const CRect &rc )
{
	CPoint ptLeftTop(rc.TopLeft());
	CPoint ptRightBottom(rc.BottomRight());

	ptLeftTop = FrameToScreen(ptLeftTop);

	ptRightBottom = FrameToScreen(ptRightBottom);

	return CRect(ptLeftTop, ptRightBottom);
}

CXFrameMsgMgr *CXFrame::GetFrameMsgMgr()
{
	CXFrame * pFrameParent = GetParent();
	if (!pFrameParent)
		return NULL;

	return pFrameParent->GetFrameMsgMgr();
}

CXFrame * CXFrame::GetTopFrameFromPoint(const CPoint &pt)
{
	if (!::PtInRect(&CRect(0, 0, m_rcFrame.Width(), m_rcFrame.Height()), pt))
		return NULL;

	CXFrame * pFrame = this;

	std::vector<CXFrame *>::const_reverse_iterator iter = m_vecFrameChild.rbegin();
	for (; iter != m_vecFrameChild.rend(); ++iter)
		if(*iter && (*iter)->IsVisible() && ::PtInRect(&(*iter)->GetRect(), pt))
		{
			CXFrame *pFrameChild = (*iter)->GetTopFrameFromPoint((*iter)->ParentToChild(pt));
			if (pFrameChild)
			{
				pFrame = pFrameChild;
				break;
			}
		}

	return pFrame;
}

inline static CRect LRectToDRect(HDC dc, const CRect &rc)
{
	CRect rcRst(rc);
	::LPtoDP(dc, (LPPOINT)&rcRst, 2);
	return rcRst;
}

BOOL CXFrame::PaintUI(HDC hDC, const CRect &rect)
{
	if (!IsVisible())
		return TRUE;

	return PaintBackground(hDC, rect) && PaintForeground(hDC, rect);
}


CPoint CXFrame::ParentToChild(const CPoint &pt)
{
	return CPoint(pt.x - m_rcFrame.left, pt.y - m_rcFrame.top);
}

CRect CXFrame::ParentToChild(const CRect &rc)
{
	return CRect(rc.left - m_rcFrame.left, rc.top - m_rcFrame.top, rc.right - m_rcFrame.left, rc.bottom - m_rcFrame.top);
}

CPoint CXFrame::ChildToParent(const CPoint &pt)
{
	return CPoint(pt.x + m_rcFrame.left, pt.y + m_rcFrame.top);
}

CRect CXFrame::ChildToParent(const CRect &rc)
{
	return CRect(rc.left + m_rcFrame.left, rc.top + m_rcFrame.top, rc.right + m_rcFrame.left, rc.bottom + m_rcFrame.top);
}

BOOL CXFrame::ToolTipInitial(CPoint pt)
{
	if (m_strToolTip.IsEmpty()) 
	{
		return FALSE;
	}

	HWND hWnd = GetHWND();
	if (!m_ToolTip.IsWindow()) 
	{			
		m_ToolTip.Create( hWnd, NULL, NULL, TTS_ALWAYSTIP | TTS_NOPREFIX);

		ATLASSERT(m_ToolTip.IsWindow());
		m_ToolTip.SetMaxTipWidth(245);
		m_ToolTip.AddTool(hWnd, _T(""));

		m_ToolTip.SetDelayTime( TTDT_AUTOPOP, GetDoubleClickTime() * 10 ) ;
		m_ToolTip.SetDelayTime( TTDT_RESHOW, GetDoubleClickTime() * 10 ) ;
		m_ToolTip.Activate(TRUE);
	}

	MSG msg = { hWnd, WM_MOUSEMOVE, 0, MAKELONG (pt.x, pt.y)};
	m_ToolTip.RelayEvent(&msg);
	m_ToolTip.UpdateTipText(m_strToolTip.GetString(), hWnd);
	return TRUE;
}

BOOL CXFrame::SetToolTip(CString strToolTip)
{
	m_strToolTip = strToolTip;
	return TRUE;
}

BOOL CXFrame::ToolTipDestroy()
{
	if (!m_strToolTip.IsEmpty() && m_ToolTip.IsWindow()) 
	{
		HWND hWnd = GetHWND();
		m_ToolTip.UpdateTipText(_T(""), hWnd);
		m_ToolTip.DestroyWindow();
	}

	return TRUE;
}

BOOL CXFrame::ToolTipInitialEx(CPoint pt)
{
	InitCommonControls();
	if (m_strToolTip.IsEmpty()) 
	{
		return FALSE;
	}

	HWND hWnd = GetHWND();
	if(!IsWindow(m_ToolTip))
	{
		HINSTANCE hInst =GetModuleHandle(NULL);		
		m_hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, L"", TTS_ALWAYSTIP |TTS_NOPREFIX, 0, 0, 0, 0, NULL, NULL, hInst, NULL );
		if(m_hToolTip == NULL)
		{
			return FALSE;
		}

		::SendMessage(m_hToolTip, TTM_SETMAXTIPWIDTH, 0, 245);

		TOOLINFO ti;
		ZeroMemory(&ti, sizeof ti);
		ti.cbSize = sizeof ti;
		ti.uFlags = TTF_IDISHWND;
		ti.hinst = NULL;
		ti.hwnd = ::GetParent(hWnd);
		ti.uId = (UINT_PTR)hWnd;
		ti.lpszText = _T("");

		::SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
		::SendMessage(m_hToolTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, MAKELPARAM(GetDoubleClickTime() * 10, 0));
		::SendMessage(m_hToolTip, TTM_SETDELAYTIME, TTDT_RESHOW, MAKELPARAM(GetDoubleClickTime() * 10, 0));
		::SendMessage(m_hToolTip, TTM_ACTIVATE, TRUE, 0L);
	}

	MSG msg = {hWnd, WM_MOUSEMOVE, 0, MAKELONG (pt.x, pt.y)};
	::SendMessage(m_hToolTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);

	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_IDISHWND;
	ti.hinst = NULL;
	ti.hwnd = ::GetParent(hWnd);
	ti.uId = (UINT_PTR)hWnd;
	ti.lpszText = m_strToolTip.GetBuffer();
	::SendMessage(m_hToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	m_strToolTip.ReleaseBuffer();
	return TRUE;
}

BOOL CXFrame::ToolTipDestroyEx()
{
	if (!m_strToolTip.IsEmpty() || !IsWindow(m_hToolTip)) 
	{
		return FALSE;
	}

	HWND hWnd = GetHWND();

	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_IDISHWND;
	ti.hinst = NULL;
	ti.hwnd = ::GetParent(hWnd);
	ti.uId = (UINT_PTR)hWnd;
	ti.lpszText = _T("");

	::SendMessage(m_hToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	DestroyWindow(m_hToolTip);
	m_hToolTip = NULL;
	return TRUE;
}

CPoint CXFrame::OtherFrameToThisFrame( CXFrame * pFrameOther, const CPoint &pt )
{
	if (!pFrameOther)
		return CPoint(0, 0);

	CXFrame *pThisFrameRoot = this;
	CXFrame *pOtherFrameRoot = pFrameOther;

	CPoint ptThisOrg(0, 0);
	CPoint ptTarget(pt);
	while (pThisFrameRoot->GetParent())
	{
		ptThisOrg = pThisFrameRoot->ChildToParent(ptThisOrg);
		pThisFrameRoot = pThisFrameRoot->GetParent();
	}

	while (pOtherFrameRoot->GetParent())
	{
		ptTarget = pOtherFrameRoot->ChildToParent(ptTarget);
		pOtherFrameRoot = pOtherFrameRoot->GetParent();
	}

	if (pThisFrameRoot != pOtherFrameRoot)
		return CPoint(0, 0);

	ptTarget = CPoint(ptTarget.x - ptThisOrg.x , ptTarget.y - ptThisOrg.y);

	return ptTarget;
}


CRect CXFrame::OtherFrameToThisFrame( CXFrame * pFrameOther, const CRect &rect )
{
	CPoint ptTopLeft(rect.TopLeft());
	CPoint ptRightBottom(rect.BottomRight());

	ptTopLeft = OtherFrameToThisFrame(pFrameOther, ptTopLeft);
	ptRightBottom =	OtherFrameToThisFrame(pFrameOther, ptRightBottom);

	return CRect(ptTopLeft.x, ptTopLeft.y, ptRightBottom.x, ptRightBottom.y);
}

BOOL CXFrame::AddEventListener(IXFrameEventListener *pListener)
{
	if (!pListener)
		return FALSE;

	std::pair<std::map<CRemoteRef<IXFrameEventListener>, DWORD>::iterator, bool> rst =
		m_mapEventListener.insert(std::make_pair(pListener, 1));

	if (!rst.second)
		rst.first->second++;

	return TRUE;
}


VOID CXFrame::ThrowEvent(UINT uEvent, WPARAM wParam, LPARAM lParam)
{
 	DWORD dwInstanceID = m_dwInstanceID;

	std::list<std::pair<const CRemoteRef<IXFrameEventListener>, UINT>> 
		lListeners(m_mapEventListener.begin(), m_mapEventListener.end());

  	for (std::list<std::pair<const CRemoteRef<IXFrameEventListener>, UINT>>::const_iterator it = lListeners.begin();
  		it != lListeners.end(); it++)
  	{
  		const CRemoteRef<IXFrameEventListener> & r = it->first;
  
  		if (r)
  			r->ProcessFrameEvent(this, uEvent, wParam, lParam);
  		else
  			if (IsInstanceActive(dwInstanceID))
  				m_mapEventListener.erase(r);
  	}
}

BOOL CXFrame::GetFocus()
{
	CXFrameMsgMgr *pMgr = GetFrameMsgMgr();
	if (pMgr)
		return pMgr->GetFocus(this);

	return FALSE;
}

BOOL CXFrame::KillFocus()
{
	CXFrameMsgMgr *pMgr = GetFrameMsgMgr();
	if (pMgr)
		return pMgr->KillFocus(this);

	return FALSE;
}

CRect CXFrame::GetMargin()
{
	return m_rcMargin;
}

BOOL CXFrame::SetMargin( const CRect &rect )
{
	CRect rcMarginOld(m_rcMargin);

	if (rcMarginOld == rect)
		return TRUE;

	m_rcMargin = rect;

	if ((rcMarginOld.right != rect.right && m_WidthMode == WIDTH_MODE_REACH_PARENT) ||
		(rcMarginOld.bottom != rect.bottom) && m_HeightMode == HEIGHT_MODE_REACH_PARENT)
		RefreashFrameRect();

	ThrowEvent(EVENT_FRAME_MARGIN_CHANGED, (WPARAM)&rect, 0);

	return TRUE;
}

BOOL CXFrame::PaintBackground( HDC hDC, const CRect &rect )
{
	if (m_pDrawBackground)
		m_pDrawBackground->Draw(hDC, rect);

	return TRUE;
}

BOOL CXFrame::PaintForeground( HDC hDC, const CRect &rect )
{
	if (m_bSelectable && m_bSelectedState)
	{
		if (FillSelectedLayer())
			m_pSelectedLayer->Draw(hDC, rect);
		else ATLASSERT(NULL);
	}

	if (m_bTouchable && m_bMouseOver)
		if (m_bMouseDown)
		{
			if (FillMouseDownLayer())
				m_pMouseDownLayer->Draw(hDC, rect);
			else ATLASSERT(NULL);
		}
		else
		{
			if (FillMouseOverLayer())
				m_pMouseOverLayer->Draw(hDC, rect);
			else ATLASSERT(NULL);
		}

	std::vector<CXFrame *>::const_iterator iter = m_vecFrameChild.begin();
	for(; iter != m_vecFrameChild.end(); ++iter)
		if(*iter && (*iter)->IsVisible())
		{
			CRect rcPaint(0, 0, 0, 0);
			if (!::IntersectRect(&rcPaint, &rect, &((*iter)->GetRect())))
				continue;

			HRGN hRgnOldClip = ::CreateRectRgn(0, 0, 0, 0);
			::GetClipRgn(hDC, hRgnOldClip);

			HRGN hRgn = ::CreateRectRgnIndirect(&LRectToDRect(hDC, rcPaint));
			::ExtSelectClipRgn(hDC, hRgn, RGN_AND);
			::DeleteObject(hRgn);
			hRgn = NULL;

			CPoint ptOrgOld(0, 0);
			::GetViewportOrgEx(hDC, &ptOrgOld);
			::SetViewportOrgEx(hDC, ptOrgOld.x + (*iter)->GetRect().left, ptOrgOld.y + (*iter)->GetRect().top, NULL);

			(*iter)->PaintUI(hDC, (*iter)->ParentToChild(rcPaint));

			::SetViewportOrgEx(hDC, ptOrgOld.x, ptOrgOld.y, NULL);

			::SelectClipRgn(hDC, hRgnOldClip);
			::DeleteObject(hRgnOldClip);	
		}


	return TRUE;
}

BOOL CXFrame::RemoveEventListener( IXFrameEventListener *pListener )
{
	if (!pListener)
		return FALSE;

	std::map<CRemoteRef<IXFrameEventListener>, DWORD>::iterator it =
		m_mapEventListener.find(pListener);

	if (it != m_mapEventListener.end())
	{
		if (--it->second == 0)
			m_mapEventListener.erase(it);
	}

	return TRUE;
}


BOOL CXFrame::ConfigFrameByXML( X_XML_NODE_TYPE xml )
{
	if (!xml)
		return FALSE;

	X_XML_ATTR_TYPE attr = NULL;

	IXImage *pBackgroud = CXFrameXMLFactory::BuildImage(xml, "bg", "bg_type", "stretch", "bg_part_");
	if (pBackgroud)
		delete SetBackground(pBackgroud);

	CRect rcMargin(0, 0, 0, 0);
	attr = xml->first_attribute("margin_left", 0, false);
	if (attr) rcMargin.left = atoi(attr->value());
	attr = xml->first_attribute("margin_top", 0, false);
	if (attr) rcMargin.top = atoi(attr->value());
	attr = xml->first_attribute("margin_right", 0, false);
	if (attr) rcMargin.right = atoi(attr->value());
	attr = xml->first_attribute("margin_bottom", 0, false);
	if (attr) rcMargin.bottom = atoi(attr->value());
	SetMargin(rcMargin);

 	attr = xml->first_attribute("touchable", 0, false);
 	if (attr && !StrCmpIA(attr->value(), "true"))
 		SetTouchable(TRUE);
	IXImage *pMouseOverLayer = CXFrameXMLFactory::BuildImage(xml, "mouse_over_layer", "mouse_over_layer_type", "stretch", "mouse_over_layer_part_");
 	if (pMouseOverLayer) delete SetMouseOverLayer(pMouseOverLayer);
	IXImage *pMouseDownLayer = CXFrameXMLFactory::BuildImage(xml, "mouse_down_layer", "mouse_down_layer_type", "stretch", "mouse_down_layer_part_");
 	if (pMouseDownLayer) delete SetMouseDownLayer(pMouseDownLayer);
 
 	attr = xml->first_attribute("selectable", 0, false);
 	if (attr && !StrCmpIA(attr->value(), "true"))
 		SetSelectable(TRUE);
 	attr = xml->first_attribute("mouse_click_select", 0, false);
 	if (attr && !StrCmpIA(attr->value(), "true"))
 		SetSelectWhenMouseClick(TRUE);
 	attr = xml->first_attribute("mouse_click_unselect", 0, false);
 	if (attr && !StrCmpIA(attr->value(), "true"))
 		SetUnselectWhenMouseClick(TRUE);
 	attr = xml->first_attribute("selected", 0, false);
 	if (attr && !StrCmpIA(attr->value(), "true"))
 		SetSelectedState(TRUE);
	IXImage *pSelectedLayer = CXFrameXMLFactory::BuildImage(xml, "selected_layer", "selected_layer_type", "stretch", "selected_layer_part_");
 	if (pSelectedLayer) delete SetSelectedLayer(pSelectedLayer);

	attr = xml->first_attribute("name", 0, false);
	if (attr)
		SetName(XLibSA2T(attr->value()));

	HandleXMLChildNodes(xml);

	attr = xml->first_attribute("visible", 0, false);
	if (!attr || StrCmpIA(attr->value(), "false"))
		SetVisible(TRUE);

	return TRUE;
}

BOOL CXFrame::HandleXMLChildNodes( X_XML_NODE_TYPE xml )
{
	for (X_XML_NODE_TYPE child = xml->first_node();
		child; child = child->next_sibling())
	{
		if (child->type() != X_XML_NODE_CATEGORY_TYPE::node_element)
			continue;
		CXFrame *pChildFrame = 
			CXFrameXMLFactory::Instance().BuildFrame(child, this);
	}

	return TRUE;
}


BOOL CXFrame::SetName( LPCTSTR pName )
{
	if (!pName)
		return FALSE;

	m_strFrameName = pName;
	return TRUE;
}

CXFrame * CXFrame::GetFrameByName(LPCTSTR pName)
{
	if (!pName)
		return NULL;

	if (m_strFrameName == pName)
		return this;

	for (std::vector<CXFrame *>::const_iterator it = m_vecFrameChild.begin();
		it != m_vecFrameChild.end(); it++)
	{
		CXFrame *pRst = NULL;
		if (*it && (pRst = (*it)->GetFrameByName(pName)))
			return pRst;
	}

	return NULL;
}

BOOL CXFrame::GetFrameByName( LPCTSTR pName, std::vector<CXFrame *> *pvFrames )
{
	if (!pName || !pvFrames)
		return FALSE;
	
	if (m_strFrameName == pName)
		pvFrames->push_back(this);

	for (std::vector<CXFrame *>::const_iterator it = m_vecFrameChild.begin();
		it != m_vecFrameChild.end(); it++)
			if (*it)
				(*it)->GetFrameByName(pName, pvFrames);

	return TRUE;
}

DWORD CXFrame::RegisterInstanceID()
{
 	for (std::vector<BOOL>::size_type i = 0;
 		i < s_bActiveInstances.size(); i++)
 	{
 		if (!s_bActiveInstances[i])
 		{
 			s_bActiveInstances[i] = TRUE;
 			return i;
 		}
 	}
 
 	s_bActiveInstances.push_back(TRUE);
 	return s_bActiveInstances.size() - 1;
}

BOOL CXFrame::UnregisterInstanceID( DWORD dwInstanceID )
{
	s_bActiveInstances[dwInstanceID] = FALSE;
	return TRUE;
}

BOOL CXFrame::IsInstanceActive( DWORD dwInstanceID )
{
	return s_bActiveInstances[dwInstanceID];
}

DWORD CXFrame::GetInstanceID()
{
	return m_dwInstanceID;
}

VOID CXFrame::OnParentRectChanged( CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (m_WidthMode != WIDTH_MODE_REACH_PARENT && m_HeightMode != HEIGHT_MODE_REACH_PARENT)
		return;

	if (!m_pFrameParent)
		return;

	CRect *pParentRectOld = (CRect *)wParam;
	if (!pParentRectOld)
		return;

	CRect rcParentRect(m_pFrameParent->GetRect());

	if ((m_WidthMode == WIDTH_MODE_REACH_PARENT && pParentRectOld->Width() != rcParentRect.Width()) ||
		(m_HeightMode == HEIGHT_MODE_REACH_PARENT && pParentRectOld->Height() != rcParentRect.Height()))
		RefreashFrameRect();
}

BOOL CXFrame::Move( const CPoint &pt )
{
	if (m_rcFrame.TopLeft() == pt)
		return TRUE;

	CRect rcTarget(m_rcFrame);
	rcTarget.MoveToXY(pt);

	return SetRect(rcTarget);
}

INT CXFrame::CalculateAdaptBackgroundWidth()
{
	if (!m_pDrawBackground)
		return 0;

	return m_pDrawBackground->GetImageWidth();
}

INT CXFrame::CalculateAdaptBackgroundHeight()
{
	if (!m_pDrawBackground)
		return 0;

	return m_pDrawBackground->GetImageHeight();
}

VOID CXFrame::RefreashFrameRect()
{
	if (m_WidthMode == WIDTH_MODE_NORMAL && m_HeightMode == HEIGHT_MODE_NORMAL)
		return;

	CRect rcFrame(m_rcFrame);

	CalculateFrameRect(&rcFrame);

	if (m_rcFrame == rcFrame)
		return;

	ChangeFrameRect(rcFrame);
}

BOOL CXFrame::SetWidthHeightMode( WIDTH_MODE aWidthMode, HEIGHT_MODE aHeightMode )
{
	if (m_pFrameParent)
	{
		ATLASSERT(!"a WRAP_CONTENT parent can't own a REACH_PARENT child. ");
		if (aWidthMode == WIDTH_MODE_REACH_PARENT && m_pFrameParent->GetWidthMode() == WIDTH_MODE_WRAP_CONTENT)
			aWidthMode = WIDTH_MODE_NOT_CHANGE;
		if (aHeightMode == HEIGHT_MODE_REACH_PARENT && m_pFrameParent->GetHeightMode() == HEIGHT_MODE_WRAP_CONTENT)
			aHeightMode = HEIGHT_MODE_NOT_CHANGE;
	}

	BOOL bListenParentForReachParentCurrent = 
		m_WidthMode == WIDTH_MODE_REACH_PARENT || m_HeightMode == HEIGHT_MODE_REACH_PARENT;
	BOOL bListenChildrenForWrapContentCurrent =
		m_WidthMode == WIDTH_MODE_WRAP_CONTENT || m_HeightMode == HEIGHT_MODE_WRAP_CONTENT;

	BOOL bNeedRecaculateFrameRect = FALSE;

	if (aWidthMode != WIDTH_MODE_NOT_CHANGE && m_WidthMode != aWidthMode)
	{	
		m_WidthMode = aWidthMode; 
		if (m_WidthMode != WIDTH_MODE_NORMAL)
			bNeedRecaculateFrameRect = TRUE;	
	}
	if (aHeightMode != HEIGHT_MODE_NOT_CHANGE && m_HeightMode != aHeightMode)
	{	
		m_HeightMode = aHeightMode;	
		if (m_HeightMode != HEIGHT_MODE_NORMAL)
			bNeedRecaculateFrameRect = TRUE; 
	}

	BOOL bListenParentForReachParent = 
		m_WidthMode == WIDTH_MODE_REACH_PARENT || m_HeightMode == HEIGHT_MODE_REACH_PARENT;
	BOOL bListenChildrenForWrapContent =
		m_WidthMode == WIDTH_MODE_WRAP_CONTENT || m_HeightMode == HEIGHT_MODE_WRAP_CONTENT;

	if (bListenParentForReachParentCurrent && !bListenParentForReachParent)
		if (m_pFrameParent) m_pFrameParent->RemoveEventListener(this);
	if (!bListenParentForReachParentCurrent && bListenParentForReachParent)
		if (m_pFrameParent) m_pFrameParent->AddEventListener(this);

	if (bListenChildrenForWrapContentCurrent && !bListenChildrenForWrapContent)
		for (std::vector<CXFrame *>::size_type i = 0; i < m_vecFrameChild.size(); i++)
			if (m_vecFrameChild[i]) m_vecFrameChild[i]->RemoveEventListener(this);
	if (!bListenChildrenForWrapContentCurrent && bListenChildrenForWrapContent)
		for (std::vector<CXFrame *>::size_type i = 0; i < m_vecFrameChild.size(); i++)
			if (m_vecFrameChild[i]) m_vecFrameChild[i]->AddEventListener(this);

	if (bNeedRecaculateFrameRect)
		RefreashFrameRect();

	return TRUE;
}

CXFrame::WIDTH_MODE CXFrame::GetWidthMode()
{
	return m_WidthMode;
}

CXFrame::HEIGHT_MODE CXFrame::GetHeightMode()
{
	return m_HeightMode;
}

INT CXFrame::CalculateWrapContentWidth()
{
	INT nWidth = 0;
	
	for (std::vector<CXFrame *>::size_type i = 0; i < m_vecFrameChild.size(); i++)
	{
		CXFrame *pChild = m_vecFrameChild[i];
		if (pChild && pChild->IsHoldPlace())
		{
			INT nRightWithMargin = 
				pChild->GetRect().right + pChild->GetMargin().right;
			if (nRightWithMargin > nWidth)
				nWidth = nRightWithMargin;
		}
	}

	return nWidth;
}

INT CXFrame::CalculateWrapContentHeight()
{
	INT nHeight = 0;

	for (std::vector<CXFrame *>::size_type i = 0; i < m_vecFrameChild.size(); i++)
	{
		CXFrame *pChild = m_vecFrameChild[i];
		if (pChild && pChild->IsHoldPlace())
		{
			INT nBottomWithMargin = 
				pChild->GetRect().bottom + pChild->GetMargin().bottom;
			if (nBottomWithMargin  > nHeight)
				nHeight = nBottomWithMargin;
		}
	}

	return nHeight;
}

VOID CXFrame::OnChildRectChanged( CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!pSrcFrame || std::find(m_vecFrameChild.begin(), m_vecFrameChild.end(), pSrcFrame) == m_vecFrameChild.end())
		return;

	CRect *pRectOld = (CRect *)wParam;
	if (!pRectOld)
		return;

	CRect rcRect(pSrcFrame->GetRect());

	if ((m_WidthMode == WIDTH_MODE_WRAP_CONTENT && pRectOld->right != rcRect.right) ||
		(m_HeightMode == HEIGHT_MODE_WRAP_CONTENT && pRectOld->bottom != rcRect.bottom))
		RefreashFrameRect();
}

VOID CXFrame::OnChildMarginChanged( CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!pSrcFrame || std::find(m_vecFrameChild.begin(), m_vecFrameChild.end(), pSrcFrame) == m_vecFrameChild.end())
		return;

	CRect *pMarginOld = (CRect *)wParam;
	if (!pMarginOld)
		return;

	CRect rcMargin(pSrcFrame->GetMargin());

	if ((m_WidthMode == WIDTH_MODE_WRAP_CONTENT && pMarginOld->right != rcMargin.right) ||
		(m_HeightMode == HEIGHT_MODE_WRAP_CONTENT && pMarginOld->bottom != rcMargin.bottom))
		RefreashFrameRect();
}

VOID CXFrame::OnChildHoldPlaceStateChanged( CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!pSrcFrame || std::find(m_vecFrameChild.begin(), m_vecFrameChild.end(), pSrcFrame) == m_vecFrameChild.end())
		return;

	if (m_WidthMode == WIDTH_MODE_WRAP_CONTENT || m_HeightMode == HEIGHT_MODE_WRAP_CONTENT)
		RefreashFrameRect();
}

BOOL CXFrame::PostFrameMsg( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return CXMessageService::Instance().PostFrameMsg(this, uMsg, wParam, lParam);
}

VOID CXFrame::OnAttachedToParent( CXFrame *pParent )
{
	if (GetParent())
		SetParent(NULL);

	if (pParent)
	{
		if (GetWidthMode() == WIDTH_MODE_REACH_PARENT && pParent->GetWidthMode() == WIDTH_MODE_WRAP_CONTENT)
		{
			ATLASSERT(!_T("a WRAP_CONTENT parent can't own a REACH_PARENT child. "));
			SetWidthHeightMode(WIDTH_MODE_NORMAL, HEIGHT_MODE_NOT_CHANGE);
		}
		if (GetHeightMode() == HEIGHT_MODE_REACH_PARENT && pParent->GetHeightMode() == HEIGHT_MODE_WRAP_CONTENT)
		{
			ATLASSERT(!_T("a WRAP_CONTENT parent can't own a REACH_PARENT child. "));
			SetWidthHeightMode(WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE_NORMAL);
		}
	}

	m_pFrameParent = pParent;

	if (pParent)
	{
		if (GetWidthMode() == WIDTH_MODE_REACH_PARENT || 
			GetHeightMode() == HEIGHT_MODE_REACH_PARENT)
		{
			RefreashFrameRect();
			pParent->AddEventListener(this);
		}
	}

	ThrowEvent(EVENT_FRAME_ATTACHED_TO_PARENT, (WPARAM)pParent, 0);
}

VOID CXFrame::OnDetachedFromParent()
{
	CXFrame *pParent = m_pFrameParent;

	if (pParent &&
		(m_WidthMode == WIDTH_MODE_REACH_PARENT || 
		m_HeightMode == HEIGHT_MODE_REACH_PARENT))
		pParent->RemoveEventListener(this);

	m_pFrameParent = NULL;

	ThrowEvent(EVENT_FRAME_DETACHED_FROM_PARENT, (WPARAM)pParent, 0);
}

UINT CXFrame::SetTimer( UINT uElapse )
{
	return CXMessageService::Instance().SetTimer(this, uElapse);
}

BOOL CXFrame::KillTimer( UINT nID )
{
	return CXMessageService::Instance().KillTimer(nID);
}

BOOL CXFrame::IsFrameActive()
{
	return m_bIsFrameAlive;
}

HDC CXFrame::GetDC()
{
	if (!m_pFrameParent)
		return NULL;

	HDC dc = m_pFrameParent->GetDC();

	HRGN hRgn = ::CreateRectRgnIndirect(&LRectToDRect(dc, m_rcFrame));
	::ExtSelectClipRgn(dc, hRgn, RGN_AND);
	::DeleteObject(hRgn);
	hRgn = NULL;

	CPoint ptOrgOld(0, 0);
	::GetViewportOrgEx(dc, &ptOrgOld);
	::SetViewportOrgEx(dc, ptOrgOld.x + GetRect().left, ptOrgOld.y + GetRect().top, NULL);

	return dc;
}

BOOL CXFrame::ReleaseDC( HDC dc )
{
	if (!m_pFrameParent)
		return NULL;

	return m_pFrameParent->ReleaseDC(dc);
}

BOOL CXFrame::NeedPrepareMessageForThisFrame()
{
	return TRUE;
}

BOOL CXFrame::SetHoldPlace( BOOL bHoldPlace )
{
	if (m_bHoldPlace == bHoldPlace)
		return TRUE;

	m_bHoldPlace = bHoldPlace;

	ThrowEvent(EVENT_FRAME_HOLDPLACE_STATE_CHANGED, (WPARAM)m_bHoldPlace, 0);

	return TRUE;
}

BOOL CXFrame::IsHoldPlace()
{
	return m_bHoldPlace;
}

INT CXFrame::GetVCenter()
{
	return GetRect().Height() / 2;
}

BOOL CXFrame::FillMouseOverLayer()
{
	if (m_pMouseOverLayer)
		return TRUE;

	SetMouseOverLayer(CXResourceMgr::Instance().GetImage(_T("img/layer/mouse_over.9.png")));

	return m_pMouseOverLayer != NULL;
}

BOOL CXFrame::FillMouseDownLayer()
{
	if (m_pMouseDownLayer)
		return TRUE;

	SetMouseDownLayer(CXResourceMgr::Instance().GetImage(_T("img/layer/mouse_down.9.png")));

	return m_pMouseDownLayer != NULL;
}

BOOL CXFrame::FillSelectedLayer()
{
	if (m_pSelectedLayer)
		return TRUE;

	SetSelectedLayer(CXResourceMgr::Instance().GetImage(_T("img/layer/selected.9.png")));

	return m_pSelectedLayer != NULL;
}

LRESULT CXFrame::OnMouseEnter( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	m_bMouseOver = TRUE;

	if (!m_bTouchable)
		return 0;

	InvalidateRect();

	return 0;
}

LRESULT CXFrame::OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	m_bMouseOver = FALSE;

	if (!m_bTouchable)
		return 0;

	InvalidateRect();

	return 0;
}

LRESULT CXFrame::OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	if (!m_bMouseDown)
		return 0;

	m_bMouseDown = FALSE;

	CXFrameMsgMgr *pMgr = GetFrameMsgMgr();
	if (pMgr)
		pMgr->ReleaseCaptureMouse(this);

	if (m_bTouchable)
		InvalidateRect();

	if (!::PtInRect(&ParentToChild(m_rcFrame), CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))))
		return 0;

	if (m_bSelectable)
		if (m_bSelectedState)
		{
			if (m_bUnselectWhenMouseCilck) SetSelectedState(FALSE);
		}
		else
		{
			if (m_bSelectWhenMouseCilck) SetSelectedState(TRUE);
		}

	return 0;
}

LRESULT CXFrame::OnLButtonDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	if (!m_bTouchable && !m_bSelectWhenMouseCilck && !m_bUnselectWhenMouseCilck)
		return 0;

	m_bMouseDown = TRUE;

	CXFrameMsgMgr *pMgr = GetFrameMsgMgr();
	if (pMgr)
		pMgr->CaptureMouse(this);

	InvalidateRect();

	return 0;
}

BOOL CXFrame::SetTouchable( BOOL b )
{
	if ((m_bTouchable && b) || (!m_bTouchable && !b))
		return TRUE;

	m_bTouchable = b;

	InvalidateRect();

	return TRUE;
}

IXImage * CXFrame::SetMouseOverLayer( IXImage *pLayer )
{
	IXImage *pOldLayer = m_pMouseOverLayer;

	m_pMouseOverLayer = pLayer;

	if (m_pMouseOverLayer)
		m_pMouseOverLayer->SetDstRect(ParentToChild(GetRect()));

	return pOldLayer;
}

IXImage * CXFrame::SetMouseDownLayer( IXImage *pLayer )
{
	IXImage *pOldLayer = m_pMouseDownLayer;

	m_pMouseDownLayer = pLayer;

	if (m_pMouseDownLayer)
		m_pMouseDownLayer->SetDstRect(ParentToChild(GetRect()));

	return pOldLayer;
}

BOOL CXFrame::SetSelectable( BOOL b )
{
	if ((m_bSelectable && b) || (!m_bSelectable && !b))
		return TRUE;

	m_bSelectable = b;

	m_bSelectedState = FALSE;

	InvalidateRect();

	return TRUE;
}

BOOL CXFrame::SetSelectWhenMouseClick( BOOL b )
{
	m_bSelectWhenMouseCilck = b;

	return TRUE;
}

BOOL CXFrame::SetUnselectWhenMouseClick( BOOL b )
{
	m_bUnselectWhenMouseCilck = b;

	return TRUE;
}

BOOL CXFrame::SetSelectedState( BOOL b )
{
	if (!m_bSelectable)
		return FALSE;

	if ((m_bSelectedState && b) || (!m_bSelectedState && !b))
		return TRUE;

	m_bSelectedState = b;

	InvalidateRect();

	return TRUE;
}

IXImage * CXFrame::SetSelectedLayer( IXImage *pLayer )
{
	IXImage *pOldLayer = m_pSelectedLayer;

	m_pSelectedLayer = pLayer;

	if (m_pSelectedLayer)
		m_pSelectedLayer->SetDstRect(ParentToChild(GetRect()));

	return pOldLayer;
}

