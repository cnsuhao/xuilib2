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
	m_Visibility(VISIBILITY_NONE),
	m_rcFrame(0, 0, 0, 0),
	m_nOffsetX(0),
	m_nOffsetY(0),
	m_nScrollX(0),
	m_nScrollY(0),
	m_bManualLayoutMode(FALSE),
	m_pLayoutParam(NULL),
	m_pDelayLayoutParam(NULL),
	m_bDelayUpdateLayoutParamScheduled(FALSE),
	m_nMeasuredWidth(0),
	m_nMeasuredHeight(0),
	m_bLayoutInvaild(TRUE),
	m_bNeedInvalidateAfterLayout(FALSE),
	m_dwInstanceID(RegisterInstanceID()),
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

BOOL CXFrame::Create( CXFrame * pFrameParent, LayoutParam * pLayout, VISIBILITY visibility /*= VISIBILITY_NONE*/)
{
	ATLASSERT(!m_bIsFrameAlive || !_T("Already created. "));
	m_bIsFrameAlive = TRUE;

 	if (!pLayout) 
 		OpenManualLayoutMode();
 	else
 	{
		BeginUpdateLayoutParam(pLayout);
		EndUpdateLayoutParam();
	}	

	SetParent(pFrameParent);

	SetVisibility(visibility);

	return TRUE;
}

VOID CXFrame::Destroy()
{
	ATLASSERT(m_bIsFrameAlive || !_T("Already destroyed or not created. "));
	m_bIsFrameAlive = FALSE;

	SetVisibility(VISIBILITY_NONE);

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

	m_LastMeasureWidthParam.Reset();
	m_LastMeasureHeightParam.Reset();

	SetRect(CRect(0, 0, 0, 0));
	m_nOffsetX = m_nOffsetY = 0;
	m_nScrollX = m_nScrollY = 0;

	delete SetBackground(NULL);

	SetTouchable(FALSE);
	delete SetMouseOverLayer(NULL);
	delete SetMouseDownLayer(NULL);
	
	SetSelectable(FALSE);
	SetSelectWhenMouseClick(FALSE);
	SetUnselectWhenMouseClick(FALSE);
	SetSelectedState(FALSE);
	delete SetSelectedLayer(NULL);

	m_bManualLayoutMode = FALSE;

	delete m_pLayoutParam;
	m_pLayoutParam = NULL;
	delete m_pDelayLayoutParam;
	m_pDelayLayoutParam = NULL;
	m_bDelayUpdateLayoutParamScheduled = FALSE;

	m_nMeasuredWidth = m_nMeasuredHeight = 0;

	m_bLayoutInvaild = TRUE;
	m_bNeedInvalidateAfterLayout = FALSE;

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
	
	switch (pFrame->GetVisibility())
	{
	case VISIBILITY_SHOW:
		InvalidateRect(pFrame->GetRect());
	case VISIBILITY_HIDE:
		InvalidateLayout();
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

	VISIBILITY visibility = pFrame->GetVisibility();

	switch (visibility)
	{
	case VISIBILITY_HIDE:
		InvalidateLayout();
		break;
	case VISIBILITY_SHOW:
		InvalidateLayout();
		pFrame->InvalidateAfterLayout();
		break;
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

IXImage * CXFrame::SetBackground(IXImage * pDrawBackground)
{
	if (m_pDrawBackground == pDrawBackground)
		return NULL;

	IXImage *pOldBackground = m_pDrawBackground;

	m_pDrawBackground = pDrawBackground;

	if (m_pDrawBackground)
		m_pDrawBackground->SetDstRect(CRect(0, 0, m_rcFrame.Width(), m_rcFrame.Height()));

	if (m_Visibility == VISIBILITY_SHOW)
		InvalidateRect();

	return pOldBackground;
}

IXImage * CXFrame::GetBackground()
{
	return m_pDrawBackground;
}

BOOL CXFrame::SetRect( const CRect & rcNewFrameRect )
{
	if (m_rcFrame == rcNewFrameRect)
		return TRUE;

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

	if (m_Visibility == VISIBILITY_SHOW)
	{
		if (m_pFrameParent)
		{
			m_pFrameParent->InvalidateRect(rcFrameRectOld);
			m_pFrameParent->InvalidateRect(rcNewFrameRect);
		}
		else
		{
			if (rcFrameRectOld.Width() != rcNewFrameRect.Width() ||
				rcFrameRectOld.Height() != rcNewFrameRect.Height())
			InvalidateRect();
		}
	}

	ThrowEvent(EVENT_FRAME_RECT_CHANGED, (WPARAM)&rcFrameRectOld, (LPARAM)&rcNewFrameRect);

	return TRUE;
}

CRect CXFrame::GetRect()
{
	return m_rcFrame;
}

BOOL CXFrame::SetVisibility( VISIBILITY visibility )
{
	if (m_Visibility == visibility)
		return TRUE;

	VISIBILITY OldVisibility = m_Visibility;
	m_Visibility = visibility;

	switch (visibility)
	{
	case VISIBILITY_SHOW:
		if (OldVisibility == VISIBILITY_NONE && m_pFrameParent)
		{
			m_pFrameParent->InvalidateLayout();
			InvalidateAfterLayout();
		}
		else
			InvalidateRect();
		break;

	case VISIBILITY_HIDE:
		if (OldVisibility == VISIBILITY_NONE)
		{
			if (m_pFrameParent) m_pFrameParent->InvalidateLayout();
		}
		else
		{
			if (m_pFrameParent) m_pFrameParent->InvalidateRect(m_rcFrame);
		}
		break;

	case VISIBILITY_NONE:
		if (OldVisibility == VISIBILITY_SHOW)
			if (m_pFrameParent) m_pFrameParent->InvalidateRect(m_rcFrame);
		if (m_pFrameParent) m_pFrameParent->InvalidateLayout();
		break;
	}

	ThrowEvent(EVENT_FRAME_VISIBILITY_CHANGED, (WPARAM)visibility, (LPARAM)OldVisibility);

	return TRUE;
}

CXFrame::VISIBILITY CXFrame::GetVisibility()
{
	return m_Visibility;
}

BOOL CXFrame::Update()
{
	if (m_Visibility != VISIBILITY_SHOW)
		return TRUE;

	CXFrame * pFrameParent = GetParent();
	if (!pFrameParent)
		return FALSE;

	return pFrameParent->Update();
}


BOOL CXFrame::InvalidateLayout()
{
	m_bLayoutInvaild = TRUE;

 	CXFrame * pFrameParent = GetParent();
 	if (!pFrameParent)
 		return TRUE;
 
 	return pFrameParent->InvalidateLayout();
}


BOOL CXFrame::InvalidateRect(const CRect & rect)
{
	if (m_Visibility != VISIBILITY_SHOW)
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
	if (m_Visibility != VISIBILITY_SHOW) return NULL;
	if (!::PtInRect(&CRect(0, 0, m_rcFrame.Width(), m_rcFrame.Height()), pt))
		return NULL;

	CXFrame * pFrame = this;

	std::vector<CXFrame *>::const_reverse_iterator iter = m_vecFrameChild.rbegin();
	for (; iter != m_vecFrameChild.rend(); ++iter)
		if(*iter)
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
	if (m_Visibility != VISIBILITY_SHOW)
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
		if(*iter && (*iter)->GetVisibility() == VISIBILITY_SHOW)
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
	if (!attr || !StrCmpIA(attr->value(), "show"))
		SetVisibility(VISIBILITY_SHOW);
	else if (!StrCmpIA(attr->value(), "hide"))
		SetVisibility(VISIBILITY_HIDE);
	else if (!StrCmpIA(attr->value(), "none"))
		SetVisibility(VISIBILITY_NONE);
	else
		SetVisibility(VISIBILITY_SHOW);

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


CString CXFrame::GetName()
{
	return m_strFrameName;
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

BOOL CXFrame::Move( const CPoint &pt )
{
	if (m_rcFrame.TopLeft() == pt)
		return TRUE;

	CRect rcTarget(m_rcFrame);
	rcTarget.MoveToXY(pt);

	return SetRect(rcTarget);
}

BOOL CXFrame::PostFrameMsg( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return CXMessageService::Instance().PostFrameMsg(this, uMsg, wParam, lParam);
}

VOID CXFrame::OnAttachedToParent( CXFrame *pParent )
{
	if (GetParent())
		SetParent(NULL);

	m_pFrameParent = pParent;

	ThrowEvent(EVENT_FRAME_ATTACHED_TO_PARENT, (WPARAM)pParent, 0);
}

VOID CXFrame::OnDetachedFromParent()
{
	CXFrame *pParent = m_pFrameParent;

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

BOOL CXFrame::ReleaseDC( HDC dc, BOOL bUpdate/* = TRUE */)
{
	if (!m_pFrameParent)
		return FALSE;

	return m_pFrameParent->ReleaseDC(dc, bUpdate);
}

BOOL CXFrame::NeedPrepareMessageForThisFrame()
{
	return TRUE;
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

CXFrame::LayoutParam * CXFrame::GenerateLayoutParam( LayoutParam *pCopyFrom /*=NULL*/ )
{
	if (!pCopyFrom)
		return new LayoutParam();

	return new LayoutParam(*pCopyFrom);
}

CXFrame::LayoutParam * CXFrame::GenerateLayoutParam( X_XML_NODE_TYPE xml )
{	
	LayoutParam *p = GenerateLayoutParam();
	if (p) p->FillByXML(xml);
	return p;
}

BOOL CXFrame::InvalidateAfterLayout()
{
	m_bNeedInvalidateAfterLayout = TRUE;
	return TRUE;
}

CXFrame::LayoutParam * CXFrame::BeginUpdateLayoutParam()
{
	if (!m_pLayoutParam) {
		ATLASSERT(NULL);
		return NULL;
	}

	if (IsLayouting())
	{
		if (m_pDelayLayoutParam)
			return m_pDelayLayoutParam;

		if (m_pFrameParent)
			m_pDelayLayoutParam = 
				m_pFrameParent->GenerateLayoutParam(m_pLayoutParam);
		else
			m_pDelayLayoutParam =
				new CXFrame::LayoutParam(*m_pLayoutParam);

		return m_pDelayLayoutParam;
	}
	else
	{
		if (m_pDelayLayoutParam)
		{
			if (m_pDelayLayoutParam != m_pLayoutParam)
			{
				delete m_pLayoutParam;
				m_pLayoutParam = NULL;

				m_pLayoutParam = m_pDelayLayoutParam;
			}
			
			m_pDelayLayoutParam = NULL;
		}

		return m_pLayoutParam;
	}
}

BOOL CXFrame::BeginUpdateLayoutParam( LayoutParam *pLayoutParam )
{
	if (!pLayoutParam)
	{
		return FALSE;
		ATLASSERT(NULL);
	}

	if (IsLayouting())
	{
		if (m_pDelayLayoutParam != pLayoutParam)
		{
			delete m_pDelayLayoutParam;
			m_pDelayLayoutParam = NULL;

			m_pDelayLayoutParam = pLayoutParam;
		}
	}
	else
	{
		if (m_pDelayLayoutParam == m_pLayoutParam)
			m_pDelayLayoutParam = NULL;

		delete m_pDelayLayoutParam;
		m_pDelayLayoutParam = NULL;

		if (m_pLayoutParam != pLayoutParam)
		{
			delete m_pLayoutParam;
			m_pLayoutParam = NULL;

			m_pLayoutParam = pLayoutParam;
		}
	}

	return TRUE;
}

BOOL CXFrame::EndUpdateLayoutParam()
{
	if (m_bManualLayoutMode)
		return FALSE;

	if (m_pDelayLayoutParam)
	{
		if (m_bDelayUpdateLayoutParamScheduled)
			return TRUE;

		m_bDelayUpdateLayoutParamScheduled = TRUE;

		CXMessageService::Instance().
			PostFrameMsg(this, WM_DELAY_UPDATE_LAYOUT_PARAM, 0, 0);

		return TRUE;
	}

	if (m_pFrameParent)
		m_pFrameParent->InvalidateLayout();

	return TRUE;
}

CXFrame::LayoutParam * CXFrame::GetLayoutParam()
{
	return m_pLayoutParam;
}



INT CXFrame::GetMeasuredWidth()
{
	return m_nMeasuredWidth;
}

INT CXFrame::GetMeasuredHeight()
{
	return m_nMeasuredHeight;
}

BOOL CXFrame::MeasureWidth( const MeasureParam & param )
{
	if (param == m_LastMeasureWidthParam && !m_bLayoutInvaild)
		return TRUE;

	return OnMeasureWidth(param);
}

BOOL CXFrame::MeasureHeight( const MeasureParam & param )
{
	if (param == m_LastMeasureHeightParam && !m_bLayoutInvaild)
		return TRUE;

	return OnMeasureHeight(param);
}

BOOL CXFrame::OnMeasureWidth( const MeasureParam & param )
{
	return OnMeasureLayoutDirection(param, &m_nMeasuredWidth, 
		&CXFrame::MeasureWidth, &CXFrame::GetMeasuredWidth, 
		&LayoutParam::m_nX, &LayoutParam::m_nWidth, &LayoutParam::m_mWidth,
		&LayoutParam::m_nMarginRight);
}

BOOL CXFrame::OnMeasureHeight( const MeasureParam & param )
{
	return OnMeasureLayoutDirection(param, &m_nMeasuredHeight,
		&CXFrame::MeasureHeight, &CXFrame::GetMeasuredHeight, 
		&LayoutParam::m_nY, &LayoutParam::m_nHeight, &LayoutParam::m_mHeight,
		&LayoutParam::m_nMarginBottom);
}

BOOL CXFrame::OnMeasureLayoutDirection( const MeasureParam & param, INT *pMeasuredSize, 
								 BOOL (CXFrame::*pfChildMeasureProc)(const MeasureParam &), 
								 INT (CXFrame::*pfChildGetMeasurdProc)(),
								 INT LayoutParam::*pLayoutParamPos, 
								 INT LayoutParam::*pnLayoutParamSize, 
								 LayoutParam::SPECIAL_METRICS LayoutParam::*pmLayoutParamSize,
								 INT LayoutParam::*pLayoutMarginEnd)
{
	if (!pMeasuredSize || !pfChildMeasureProc || !pfChildGetMeasurdProc ||
		!pLayoutParamPos || !pnLayoutParamSize || !pmLayoutParamSize || !pLayoutMarginEnd)
	{
		ATLASSERT(NULL);
		return FALSE;
	}

	INT & nMeasuredSize = *pMeasuredSize;

	INT nMaxEnd = 0;

	for (std::vector<CXFrame *>::const_iterator it = m_vecFrameChild.begin();
		it != m_vecFrameChild.end(); it++)
	{
		if (!*it)
		{
			ATLASSERT(NULL);
			continue;
		}

		CXFrame *pCur = *it;
		if (!pCur->NeedLayout())
			continue;

		LayoutParam *pLayoutParam = pCur->GetLayoutParam();

		if (!pLayoutParam)
		{
			ATLASSERT(NULL);
			continue;
		}

		if (pLayoutParam->*pmLayoutParamSize 
			== LayoutParam::METRIC_REACH_PARENT)
			continue;

		MeasureParam ParamForMeasure; 

		switch (pLayoutParam->*pmLayoutParamSize)
		{
		case LayoutParam::METRIC_WRAP_CONTENT:
			if (param.m_Spec == MeasureParam::MEASURE_ATMOST ||
				param.m_Spec == MeasureParam::MEASURE_EXACT)
			{
				ParamForMeasure.m_Spec = MeasureParam::MEASURE_ATMOST;
				ParamForMeasure.m_nNum = max(0, param.m_nNum - pLayoutParam->*pLayoutParamPos);
			}
			else
			{
				ParamForMeasure.m_Spec = MeasureParam::MEASURE_UNRESTRICTED;
				ParamForMeasure.m_nNum = 0;
			}
			break;
		default:
			ParamForMeasure.m_Spec = MeasureParam::MEASURE_EXACT;
			ParamForMeasure.m_nNum = max(0, pLayoutParam->*pnLayoutParamSize);
			break;
		}

		(pCur->*pfChildMeasureProc)(ParamForMeasure);

		nMaxEnd = max(nMaxEnd, pLayoutParam->*pLayoutParamPos + (pCur->*pfChildGetMeasurdProc)()
			+ pLayoutParam->*pLayoutMarginEnd);
	}

	switch (param.m_Spec)
	{
	case MeasureParam::MEASURE_EXACT:
		nMeasuredSize = param.m_nNum;
		break;
	default:
		nMeasuredSize = nMaxEnd;
		if (param.m_Spec == MeasureParam::MEASURE_ATMOST)
			nMeasuredSize = min(nMeasuredSize, param.m_nNum);
		break;
	}

	for (std::vector<CXFrame *>::const_iterator it = m_vecFrameChild.begin();
		it != m_vecFrameChild.end(); it++)
	{
		if (!*it)
		{
			ATLASSERT(NULL);
			continue;
		}

		CXFrame *pCur = *it;
		if (!pCur->NeedLayout())
			continue;

		LayoutParam *pLayoutParam = pCur->GetLayoutParam();

		if (!pLayoutParam)
		{
			ATLASSERT(NULL);
			continue;
		}

		if (pLayoutParam->*pmLayoutParamSize 
			!= LayoutParam::METRIC_REACH_PARENT)
			continue;

		MeasureParam ParamForMeasure;
		ParamForMeasure.m_Spec = MeasureParam::MEASURE_EXACT;
		ParamForMeasure.m_nNum = max(0, 
			nMeasuredSize - pLayoutParam->*pLayoutParamPos - pLayoutParam->*pLayoutMarginEnd);

		(pCur->*pfChildMeasureProc)(ParamForMeasure);
	}

	return TRUE;
}

BOOL CXFrame::Layout( const CRect & rc )
{
	CRect rcRect(rc);

	if (m_pFrameParent)
		rcRect.OffsetRect(-m_pFrameParent->GetScrollX(), -m_pFrameParent->GetScrollY());
	rcRect.OffsetRect(m_nOffsetX, m_nOffsetY);

	if (rcRect != m_rcFrame || m_bLayoutInvaild) 
	{
		m_bLayoutInvaild = FALSE;

		OnLayout(rcRect);

		SetRect(rcRect);
		
	}

	if (m_bNeedInvalidateAfterLayout) 
	{
		m_bNeedInvalidateAfterLayout = FALSE;
		InvalidateRect();
	}
		

	return TRUE;
}

BOOL CXFrame::OnLayout( const CRect & rcRect )
{
	for (std::vector<CXFrame *>::const_iterator it = m_vecFrameChild.begin();
		it != m_vecFrameChild.end(); it++)
	{
		if (!*it)
		{
			ATLASSERT(NULL);
			continue;
		}

		CXFrame *pCur = *it;
		if (!pCur->NeedLayout())
			continue;

		LayoutParam *pLayoutParam = pCur->GetLayoutParam();
		if (!pLayoutParam)
		{
			ATLASSERT(NULL);
			continue;
		}

		CRect rc(pLayoutParam->m_nX, pLayoutParam->m_nY,
			pLayoutParam->m_nX + pCur->GetMeasuredWidth(),
			pLayoutParam->m_nY + pCur->GetMeasuredHeight());

		pCur->Layout(rc);
	}

	return TRUE;
}

BOOL CXFrame::SetMeasuredWidth(INT width)
{
	m_nMeasuredWidth = width;
	return TRUE;
}

BOOL CXFrame::SetMeasuredHeight( INT height )
{
	m_nMeasuredHeight = height;
	return TRUE;
}

BOOL CXFrame::IsLayouting()
{
	if (m_pFrameParent)
		return m_pFrameParent->IsLayouting();

	return FALSE;
}

LRESULT CXFrame::OnDelayUpdateLayoutParam( UINT uMsg, WPARAM wParam, LPARAM lParam, 
									  BOOL& bHandled, BOOL& bCancelBabble )
{
	if (!m_bDelayUpdateLayoutParamScheduled)
		return -1;

	m_bDelayUpdateLayoutParamScheduled = FALSE;

	if (!m_pDelayLayoutParam) return 0;

	if (m_pLayoutParam != m_pDelayLayoutParam)
	{
		delete m_pLayoutParam;
		m_pLayoutParam = NULL;

		m_pLayoutParam = m_pDelayLayoutParam;
	}
	
	m_pDelayLayoutParam = NULL;

	EndUpdateLayoutParam();

	return 0;
}

INT CXFrame::GetOffsetX()
{
	return m_nOffsetX;
}

INT CXFrame::GetOffsetY()
{
	return m_nOffsetY;
}

INT CXFrame::GetScrollX()
{
	return m_nScrollX;
}

INT CXFrame::GetScrollY()
{
	return m_nScrollY;
}

BOOL CXFrame::UpdateScroll(INT nDeltaX,INT nDeltaY )
{
	for (std::vector<CXFrame *>::const_iterator it = m_vecFrameChild.begin();
		it != m_vecFrameChild.end(); it++)
	{
		ATLASSERT(*it);
		if (*it) {
			CRect rcNew((*it)->GetRect());
			rcNew.OffsetRect(-nDeltaX, -nDeltaY);
			(*it)->SetRect(rcNew);
		}
	}

	return TRUE;
}

BOOL CXFrame::UpdateOffset( INT nDeltaX, INT nDeltaY )
{
	CRect rcNew(m_rcFrame);
	rcNew.OffsetRect(nDeltaX, nDeltaY);
	return SetRect(rcNew);
}

BOOL CXFrame::SetScrollX( INT x )
{
	if (m_nScrollX == x) return TRUE;
	UpdateScroll(x - m_nScrollX, 0);
	m_nScrollX = x;
	return TRUE;
}

BOOL CXFrame::SetScrollY( INT y )
{
	if (m_nScrollY == y) return TRUE;
	UpdateScroll(0, y - m_nScrollY);
	m_nScrollY = y;
	return TRUE;
}

BOOL CXFrame::SetOffsetX( INT x )
{
	if (m_nOffsetX == x) return TRUE;
	UpdateOffset(x - m_nOffsetX, 0);
	m_nOffsetX = x;
	return TRUE;
}

BOOL CXFrame::SetOffsetY( INT y )
{
	if (m_nOffsetY == y) return TRUE;
	UpdateOffset(0, y - m_nOffsetY);
	m_nOffsetY = y;
	return TRUE;
}

BOOL CXFrame::NeedLayout()
{
	return m_Visibility != VISIBILITY_NONE && !m_bManualLayoutMode;
}

BOOL CXFrame::OpenManualLayoutMode()
{
	delete m_pDelayLayoutParam;
	delete m_pLayoutParam;
	m_pDelayLayoutParam = m_pLayoutParam = NULL;

	m_bManualLayoutMode = TRUE;

	return TRUE;
}

BOOL CXFrame::CloseManualLayoutMode( LayoutParam *pLayoutParam )
{
	if (!pLayoutParam) {
		ATLASSERT(NULL);
		return FALSE;
	}

	m_bManualLayoutMode = FALSE;

	BeginUpdateLayoutParam(pLayoutParam);
	EndUpdateLayoutParam();
}

BOOL CXFrame::GetManualLayoutMode()
{
	return m_bManualLayoutMode;
}

CXFrame::LayoutParam::LayoutParam( X_XML_NODE_TYPE xml)
	: m_nX(0), m_nY(0), m_nWidth(0), m_nHeight(0),
	 m_nMarginLeft(0), m_nMarginTop(0), m_nMarginRight(0), m_nMarginBottom(0)
{
	FillByXML(xml);
}

INT CXFrame::LayoutParam::GetSpecialMetrics( LPCSTR pStr )
{
	if (!pStr)
		return 0;

	if (!StrCmpIA("reach_parent", pStr))
		return METRIC_REACH_PARENT;
	else if (!StrCmpIA("wrap_content", pStr))
		return METRIC_WRAP_CONTENT;

	return atoi(pStr);
}

BOOL CXFrame::LayoutParam::FillByXML( X_XML_NODE_TYPE xml)
{
	if (!xml)
		return FALSE;

	X_XML_ATTR_TYPE attr = NULL;

	attr = xml->first_attribute("left", 0, false);
	if (attr) m_nX = atoi(attr->value());
	attr = xml->first_attribute("top", 0, false);
	if (attr) m_nY = atoi(attr->value());
	attr = xml->first_attribute("width", 0, false);
	if (attr) m_nWidth = GetSpecialMetrics(attr->value());
	attr = xml->first_attribute("height", 0, false);
	if (attr) m_nHeight = GetSpecialMetrics(attr->value());

	attr = xml->first_attribute("margin_left", 0, false);
	if (attr) m_nMarginLeft = atoi(attr->value());
	attr = xml->first_attribute("margin_top", 0, false);
	if (attr) m_nMarginTop = atoi(attr->value());
	attr = xml->first_attribute("margin_right", 0, false);
	if (attr) m_nMarginRight = atoi(attr->value());
	attr = xml->first_attribute("margin_bottom", 0, false);
	if (attr) m_nMarginBottom = atoi(attr->value());

	return TRUE;
}

BOOL CXFrame::MeasureParam::operator==( const MeasureParam & other ) const
{
	return m_Spec == other.m_Spec && m_nNum == other.m_nNum;
}

