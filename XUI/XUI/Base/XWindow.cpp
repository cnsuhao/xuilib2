#include "StdAfx.h"

#include "XWindow.h"
#include "../../../XLib/inc/interfaceS/string/StringCode.h"
#include "XMessageService.h"

X_IMPLEMENT_FRAME_XML_RUNTIME(CXWindow)

#define WINDOW_BORDER_WIDTH 10

CXFrame * CXWindow::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
{
	if (!xml)
		return NULL;

	ATLASSERT(!pParent);
																						
	CRect rcFrame(0, 0, 0, 0);
	CString strWndName(_T("XUI´°¿Ú"));

	X_XML_ATTR_TYPE attr = NULL;

	attr = xml->first_attribute("title", 0, false);
	if (attr)
	{
#ifdef _UNICODE 
		strWndName = XLibS::StringCode::ConvertAnsiStrToWideStr(attr->value());
#else
		strWndName = attr->value();
#endif
	}

	LayoutParam *pLayout = pParent ?
		pParent->GenerateLayoutParam(xml) : new CXFrame::LayoutParam(xml);
	if (!pLayout)
	{
		CStringA strError;
		strError.Format("WARNING: Generating the layout parameter for the parent %s failed. \
			Building the frame failed. ", XLibST2A(pParent->GetName()));
		CXFrameXMLFactory::ReportError(strError);
		return NULL;
	}

	CXWindow *pFrame = new CXWindow();
	pFrame->Create(NULL, pLayout, strWndName, 0);

	return pFrame;
}

CXWindow::CXWindow(void)
	: m_dcBuffer(NULL),
	m_dcBufferForDirectDraw(NULL),
	m_hBufferOldBmp(NULL),
	m_bLayoutScheduled(FALSE),
	m_bIsLayouting(FALSE),
	m_bUpdateScheduled(FALSE),
	m_cAlpha(255),
	m_FrameMsgMgr(this),
	m_rcCaption(0, 0, 0, 0),
	m_bResizable(FALSE),
	m_bCaptionWidthReachParent(FALSE),
	m_bCaptionHeightReachParnet(FALSE)
{
}

ATL::CWndClassInfo  CXWindow::GetWndClassInfo()
{
	ATL::CWndClassInfo wc = 
	{ 
		{ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc, 
		0, 0, NULL, NULL, NULL, (HBRUSH)(COLOR_WINDOW + 1), NULL, GetClassName(), NULL }, 
		GetWndClassName(), NULL, IDC_ARROW, TRUE, 0, _T("") 
	}; 

	return wc; 
}

HWND CXWindow::Create(HWND hWndParent, LayoutParam * pLayout, 
					  LPCTSTR szWindowName, DWORD dwStyle, DWORD dwExStyle, _U_MENUorID MenuOrID, LPVOID lpCreateParam)
{
	if (!pLayout) 
	{
		ATLASSERT(!_T("No layout parameter. "));
		return NULL;
	}

	ATOM atom = GetWndClassInfo().Register(&m_pfnSuperWindowProc);
	if (szWindowName == NULL)
		szWindowName = GetWndCaption();

	dwStyle |= WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	dwExStyle |= WS_EX_LAYERED;

	BOOL bVisible = dwStyle & WS_VISIBLE;
	dwStyle &= ~WS_VISIBLE;

	HWND hWnd = CWindowImplBaseT<CWindow, CControlWinTraits>::
		Create(hWndParent, CRect(0, 0, 0, 0), szWindowName, dwStyle, dwExStyle, MenuOrID, atom, lpCreateParam);

	if (hWnd)
		__super::Create(NULL, pLayout, bVisible ? VISIBILITY_SHOW : VISIBILITY_NONE);

	return hWnd;
}

LRESULT CXWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CXWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CXWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CRect rcWnd(0, 0, 0, 0);
	GetWindowRect(&rcWnd);

	ReleaseBufferDC();
	__super::SetRect(rcWnd);
	InvalidateLayout();

	if (m_bCaptionWidthReachParent || m_bCaptionHeightReachParnet)
		RefreashCaptionRect();

	return 0;
}

LRESULT CXWindow::OnShowWindow( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	__super::SetVisibility(wParam ? VISIBILITY_SHOW : VISIBILITY_NONE);
	return 0;
}

LRESULT CXWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ValidateRect(NULL);
	return 0;
}

LRESULT CXWindow::OnEraseBkgrnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;
}

LRESULT CXWindow::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CXWindow::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

BOOL CXWindow::SetAlpha(BYTE cAlpha)
{
	if (m_cAlpha != cAlpha)
	{
		m_cAlpha = cAlpha;
		UpdateLayeredWindow(m_dcBuffer);
	}

	return TRUE;
}

BYTE CXWindow::GetAlpha()
{
	return m_cAlpha;
}

BOOL CXWindow::InvalidateRect(const CRect & rect)
{
	if (!IsWindow())
		return FALSE;

	if (GetVisibility() != VISIBILITY_SHOW)
		return FALSE;

	CRect rcFrame(GetRect());

	CRect rcReal(0,0,0,0);
	rcReal.IntersectRect(CRect(0,0,rcFrame.Width(),rcFrame.Height()), rect);

	if (rcReal.IsRectEmpty())
		return FALSE;

	m_vRectInvalidated.push_back(rcReal);

	if (!m_bUpdateScheduled)
	{
		m_bUpdateScheduled = TRUE;
		PostMessage(WM_X_UPDATE);
	}

	return TRUE;
}


BOOL CXWindow::Update()
{
	if (!IsWindow())
		return FALSE;

	if (!m_bUpdateScheduled)
		return TRUE;

	SendMessage(WM_X_UPDATE);

	return TRUE;
}


HWND CXWindow::GetHWND()
{
	return m_hWnd;
}

VOID CXWindow::ReleaseBufferDC()
{
	if (m_dcBuffer)
	{
		HGDIOBJ hSharedBitmap = ::SelectObject(m_dcBuffer, m_hBufferOldBmp);
		::SelectObject(m_dcBufferForDirectDraw, m_hBufferForDirectDrawOldBmp);
		m_hBufferOldBmp = NULL;
		m_hBufferForDirectDrawOldBmp = NULL;
		::DeleteObject(hSharedBitmap);
		hSharedBitmap = NULL;
		::DeleteDC(m_dcBuffer);
		::DeleteDC(m_dcBufferForDirectDraw);
		m_dcBuffer = NULL;
		m_dcBufferForDirectDraw = NULL;
	}
}

BOOL CXWindow::RecreateBufferDC(INT nWidth, INT nHeight )
{
	if (!IsWindow())
		return FALSE;

	ReleaseBufferDC();

	HDC hWndDC = ::GetDC(m_hWnd);
	m_dcBuffer = ::CreateCompatibleDC(hWndDC);
	m_dcBufferForDirectDraw = ::CreateCompatibleDC(hWndDC);
	HBITMAP hBitmapToShare = Util::CreateDIBSection32(nWidth, nHeight);
	m_hBufferOldBmp = ::SelectObject(m_dcBuffer, (HGDIOBJ)hBitmapToShare);
	m_hBufferForDirectDrawOldBmp = ::SelectObject(m_dcBufferForDirectDraw, (HGDIOBJ)hBitmapToShare);
	::ReleaseDC(m_hWnd, hWndDC);

	return TRUE;
}

BOOL CXWindow::UpdateLayeredWindow(HDC dcSrc)
{
	if (!IsWindow())
		return FALSE;

	CRect rcClient(0, 0, 0, 0);
	GetClientRect(&rcClient);

	BLENDFUNCTION blendUpdate = {AC_SRC_OVER, 0, m_cAlpha, AC_SRC_ALPHA};
	BOOL bRes = ::UpdateLayeredWindow(m_hWnd, NULL, NULL, &rcClient.Size(), dcSrc, &CPoint(0,0), 0, &blendUpdate, ULW_ALPHA);

	return bRes;
}


BOOL CXWindow::SetVisibility( VISIBILITY visibility )
{
	if (visibility == VISIBILITY_HIDE)
		visibility = VISIBILITY_NONE;

	if (GetVisibility() == visibility)
		return TRUE;

	if (IsWindow())
		ShowWindow( visibility == VISIBILITY_SHOW ? SW_SHOW : SW_HIDE );
	else
		__super::SetVisibility(visibility);

	return TRUE;
}

CXFrameMsgMgr * CXWindow::GetFrameMsgMgr()
{
	return &m_FrameMsgMgr;
}

LRESULT CXWindow::OnNCHitTest( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	CPoint ptMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	::ScreenToClient(m_hWnd, &ptMouse);

	if (!IsZoomed() && m_bResizable)
	{
		CRect rcClient(0, 0, 0, 0);
		GetClientRect(&rcClient);

		BOOL bLeftBorder = FALSE;
		BOOL bRightBorder = FALSE;
		BOOL bTopBorder = FALSE;
		BOOL bBottomBorder = FALSE;

		if (ptMouse.x < WINDOW_BORDER_WIDTH)
			bLeftBorder = TRUE;
		if (ptMouse.y < WINDOW_BORDER_WIDTH)
			bTopBorder = TRUE;
		if (ptMouse.x >= rcClient.right - WINDOW_BORDER_WIDTH)
			bRightBorder = TRUE;
		if (ptMouse.y >= rcClient.bottom - WINDOW_BORDER_WIDTH)
			bBottomBorder = TRUE;

		if (bTopBorder)
		{
			if (bLeftBorder)
				return HTTOPLEFT;
			if (bRightBorder)
				return HTTOPRIGHT;
			return HTTOP;
		}

		if (bBottomBorder)
		{
			if (bLeftBorder)
				return HTBOTTOMLEFT;
			if (bRightBorder)
				return HTBOTTOMRIGHT;
			return HTBOTTOM;
		}

		if (bLeftBorder)
			return HTLEFT;
		if (bRightBorder)
			return HTRIGHT;
	}

	return HTCLIENT;
}

CPoint CXWindow::ScreenToFrame(const CPoint &pt)
{
	CPoint ptRes(pt);

	if (!IsWindow())
		ptRes.x = ptRes.y = 0;
	else
	{
		if (IsIconic())
			ptRes.x = ptRes.y = 0;
		else
			ScreenToClient(&ptRes);
	}

	return ptRes;
}


CPoint CXWindow::FrameToScreen( const CPoint &pt )
{
	CPoint ptRes(pt);

	if (!IsWindow())
		ptRes.x = ptRes.y = 0;
	else
	{
		if (IsIconic())
			ptRes.x = ptRes.y = 0;
		else
			ClientToScreen(&ptRes);
	}

	return ptRes;
}

LRESULT CXWindow::OnLButtonDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	CPoint ptMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	if (::PtInRect(&m_rcCaption, ptMouse))
	{
		CPoint ptMouseInScreen(ptMouse);
		::ClientToScreen(m_hWnd, &ptMouseInScreen);
		SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(ptMouseInScreen.x, ptMouseInScreen.y));
	}

	return 0;
}

LRESULT CXWindow::OnLButtonDbClk( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	CPoint ptMouse(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	if (::PtInRect(&m_rcCaption, ptMouse))
	{
		CPoint ptMouseInScreen(ptMouse);
		::ClientToScreen(m_hWnd, &ptMouseInScreen);
		SendMessage(WM_NCLBUTTONDBLCLK, HTCAPTION, MAKELPARAM(ptMouseInScreen.x, ptMouseInScreen.y));
	}

	return 0;
}


BOOL CXWindow::SetResizable( BOOL bResizable )
{
	m_bResizable = bResizable;

	return TRUE;
}

LRESULT CXWindow::OnSetCursor( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	INT nHitTest = LOWORD(lParam);

	switch (nHitTest)
	{
	case HTCAPTION:
	case HTSYSMENU:
	case HTMENU:
	case HTCLIENT:
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
		break;

	case HTTOP:
	case HTBOTTOM:
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS)));
		break;

	case HTLEFT:
	case HTRIGHT:
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
		break;

	case HTTOPLEFT:
	case HTBOTTOMRIGHT:
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENWSE)));
		break;

	case HTTOPRIGHT:
	case HTBOTTOMLEFT:
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENESW)));
		break;

	default:
		bHandled = FALSE;
		break;
	}

	return 0;
}

LRESULT CXWindow::OnNCLButtonDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	INT nHitTest = wParam;

	switch (nHitTest)
	{
	case HTTOP:
		SendMessage( WM_SYSCOMMAND, SC_SIZE | WMSZ_TOP, lParam);
		break;
	case HTBOTTOM:
		SendMessage( WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOM, lParam);
		break;
	case HTLEFT:
		SendMessage( WM_SYSCOMMAND, SC_SIZE | WMSZ_LEFT, lParam);
		break;
	case HTRIGHT:
		SendMessage( WM_SYSCOMMAND, SC_SIZE | WMSZ_RIGHT, lParam);
		break;
	case HTTOPLEFT:
		SendMessage( WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPLEFT, lParam);
		break;
	case HTTOPRIGHT:
		SendMessage( WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPRIGHT, lParam);
		break;
	case HTBOTTOMLEFT:
		SendMessage( WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMLEFT, lParam);
		break;
	case HTBOTTOMRIGHT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMRIGHT, lParam);
		break;
	default:
		bHandled = FALSE;
		break;
	}

	return 0;
}

BOOL CXWindow::SetCaptionRect( const CRect &rc )
{
	m_rcCaption = rc;

	if (m_bCaptionWidthReachParent || m_bCaptionHeightReachParnet)
	{
		if (!IsWindow())
			return FALSE;

		CRect rcClient(0, 0, 0, 0);
		GetClientRect(&rcClient);

		if (m_bCaptionWidthReachParent)
			m_rcCaption.right = rcClient.right;
		if (m_bCaptionHeightReachParnet)
			m_rcCaption.bottom = rcClient.bottom;
	}

	return TRUE;
}


LRESULT CXWindow::OnMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	CRect rcWnd(0, 0, 0, 0);
	GetWindowRect(&rcWnd);

	__super::SetRect(rcWnd);

	ThrowEvent(EVENT_WND_MOVED, wParam, 0);
	return 0;
}


BOOL CXWindow::ConfigFrameByXML( X_XML_NODE_TYPE xml )
{
	BOOL bRtn = __super::ConfigFrameByXML(xml);

	X_XML_ATTR_TYPE attr = NULL;

	SetCaptionRect(CXFrameXMLFactory::BuildRect(xml, "title_"));

	attr = xml->first_attribute("resizable", 0, false);
	if (attr && !StrCmpIA(attr->value(), "true"))
		SetResizable(TRUE);

	attr = xml->first_attribute("title_width");
	if (attr && !StrCmpIA(attr->value(), "reach_window"))
		SetCaptionWidthReachWnd(TRUE);
	attr = xml->first_attribute("title_height");
	if (attr && !StrCmpIA(attr->value(), "reach_window"))
		SetCaptionHeightReachWnd(TRUE);

	return bRtn;
}

VOID CXWindow::Destroy()
{
	__super::Destroy();

	m_bLayoutScheduled = FALSE;
	m_bIsLayouting = FALSE;
	m_bUpdateScheduled = FALSE;
	m_vRectInvalidated.clear();

	if(IsWindow())
		DestroyWindow();

	ReleaseBufferDC();
}

CRect CXWindow::GetCaptionRect()
{
	return m_rcCaption;
}

LRESULT CXWindow::OnMinMaxInfo( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	DefWindowProc(uMsg, wParam, lParam);

	MINMAXINFO *pInfo = (MINMAXINFO *)lParam;
	if (pInfo)
	{
		pInfo->ptMaxSize.x = ::GetSystemMetrics(SM_CXFULLSCREEN);
		pInfo->ptMaxSize.y = ::GetSystemMetrics(SM_CYFULLSCREEN) + ::GetSystemMetrics(SM_CYCAPTION);
	}

	return 0;
}


LRESULT CXWindow::OnEnable( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	ThrowEvent(EVENT_WND_ENABLED, wParam, 0);

	bHandled = FALSE;
	return 0;
}

LRESULT CXWindow::OnXLayout( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!m_bLayoutScheduled)
		return -1;

	m_bLayoutScheduled = FALSE;

	m_bIsLayouting = TRUE;

	CRect rc(GetRect());

	MeasureParam param;
	param.m_Spec = MeasureParam::MEASURE_EXACT;
	
	param.m_nNum = rc.Width();
	OnMeasureWidth(param);
	param.m_nNum = rc.Height();
	OnMeasureHeight(param);

	OnLayout(rc);

	m_bIsLayouting = FALSE;

	return 0;
}


LRESULT CXWindow::OnXUpdate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!m_bUpdateScheduled)
		return -1;

	if (m_bLayoutScheduled || 
		CXMessageService::Instance().HasPendingMsg(WM_DELAY_UPDATE_LAYOUT_PARAM))
	{
		PostMessage(WM_X_UPDATE);
		return -1;
	}

	m_bUpdateScheduled = FALSE;

	if (m_vRectInvalidated.empty())
		return 0;

	INT nAreaBound = 0;
	INT nAreaSum = 0;

	CRect rcBound(m_vRectInvalidated[0]);
	for (std::vector<CRect>::size_type i = 0; i < m_vRectInvalidated.size(); i++)
	{
		if (i != 0)
		{
			if (m_vRectInvalidated[i].left < rcBound.left) rcBound.left = m_vRectInvalidated[i].left;
			if (m_vRectInvalidated[i].top < rcBound.top) rcBound.top =  m_vRectInvalidated[i].top;
			if (m_vRectInvalidated[i].right > rcBound.right) rcBound.right = m_vRectInvalidated[i].right;
			if (m_vRectInvalidated[i].bottom > rcBound.bottom) rcBound.bottom =  m_vRectInvalidated[i].bottom;
		}

		nAreaSum += m_vRectInvalidated[i].Width() * m_vRectInvalidated[i].Height();
	}

	if (nAreaSum == 0)
	{
		m_vRectInvalidated.clear();
		return 0;
	}

	CRect rcClient;
	GetClientRect(&rcClient);
	if (!m_dcBuffer && !RecreateBufferDC(rcClient.Width(), rcClient.Height()))
		return -1;

	HRGN hRgn = ::CreateRectRgnIndirect(&m_vRectInvalidated[0]);
	::SelectClipRgn(m_dcBuffer, hRgn);
	::DeleteObject(hRgn);
	for (std::vector<CRect>::size_type i = 1; i < m_vRectInvalidated.size(); i++)
	{
		hRgn = ::CreateRectRgnIndirect(&m_vRectInvalidated[i]);
		::ExtSelectClipRgn(m_dcBuffer, hRgn, RGN_OR);
		::DeleteObject(hRgn);
	}
	::SetViewportOrgEx(m_dcBuffer, 0, 0, NULL);

	nAreaBound = rcBound.Width() * rcBound.Height();

	if (nAreaBound > nAreaSum)
		for (std::vector<CRect>::size_type i = 0; i < m_vRectInvalidated.size(); i++)
		{
			Util::ClearRect(m_dcBuffer, m_vRectInvalidated[i]);
			PaintUI(m_dcBuffer, m_vRectInvalidated[i]);
		}
	else
	{
		Util::ClearRect(m_dcBuffer, rcBound);
		PaintUI(m_dcBuffer, rcBound);
	}

	UpdateLayeredWindow(m_dcBuffer);

	m_vRectInvalidated.clear();

	return 0;
}

VOID CXWindow::RefreashCaptionRect()
{
	if (!m_bCaptionWidthReachParent && !m_bCaptionHeightReachParnet)
		return;

	SetCaptionRect(m_rcCaption);
}

BOOL CXWindow::SetCaptionWidthReachWnd( BOOL b )
{
	if ((!m_bCaptionWidthReachParent && !b) || 
		(m_bCaptionWidthReachParent && b))
		return TRUE;

	m_bCaptionWidthReachParent = b;

	if (b) RefreashCaptionRect();

	return TRUE;
}

BOOL CXWindow::SetCaptionHeightReachWnd( BOOL b )
{
	if ((!m_bCaptionHeightReachParnet && !b) ||
		(m_bCaptionHeightReachParnet && b))
		return TRUE;

	m_bCaptionHeightReachParnet = b;

	if (b) RefreashCaptionRect();

	return TRUE;
}

BOOL CXWindow::SetRect( const CRect & rcNewFrameRect )
{
	if (GetRect() == rcNewFrameRect)
		return TRUE;

	if (IsWindow())
		return MoveWindow(&rcNewFrameRect, FALSE);
	else
	{
		return __super::SetRect(rcNewFrameRect);
		InvalidateLayout();
	}
}

CPoint CXWindow::FrameToWindow( const CPoint &pt )
{
	return pt;
}

HDC CXWindow::GetDC()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	if (!m_dcBuffer && !RecreateBufferDC(rcClient.Width(), rcClient.Height()))
		return NULL;

	HRGN hRgn = ::CreateRectRgnIndirect(&rcClient);
	::SelectClipRgn(m_dcBufferForDirectDraw, hRgn);
	::DeleteObject(hRgn);
	::SetViewportOrgEx(m_dcBufferForDirectDraw, 0, 0, NULL);

	return m_dcBufferForDirectDraw;
}

BOOL CXWindow::ReleaseDC( HDC dc, BOOL bUpdate /*= TRUE*/ )
{
	ATLASSERT(dc == m_dcBufferForDirectDraw);

	if (bUpdate && IsWindow())
		UpdateLayeredWindow(m_dcBufferForDirectDraw);

	return TRUE;
}

BOOL CXWindow::EndUpdateLayoutParam()
{
	LayoutParam *pLayout = GetLayoutParam();
	if (!pLayout)
		return FALSE;

	CRect rc(pLayout->m_nX, pLayout->m_nY,
		pLayout->m_nX + max(0, pLayout->m_nWidth),
		pLayout->m_nY + max(0, pLayout->m_nHeight));

	SetRect(rc);

	return TRUE;
}

BOOL CXWindow::InvalidateLayout()
{
	if (m_bLayoutScheduled)
		return TRUE;

	m_bLayoutScheduled = TRUE;

	PostMessage(WM_X_LAYOUT);

	return TRUE;
}

BOOL CXWindow::IsLayouting()
{
	return m_bIsLayouting;
}
