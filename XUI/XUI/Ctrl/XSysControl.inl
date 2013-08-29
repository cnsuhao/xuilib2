
template<class TCtrl>
BOOL CXSysControl<TCtrl>::Create( CXFrame * pFrameParent, LayoutParam * pLayout, VISIBILITY visibility /*= VISIBILITY_NONE*/)
{
	if (!pLayout) 
	{
		ATLASSERT(!_T("No layout parameter. "));
		return FALSE;
	}

	return __super::Create(NULL, &CRect(0, 0, 0, 0), NULL, WS_POPUP, WS_EX_TOOLWINDOW, 0U, NULL) && 
		__super::Create(pFrameParent, pLayout, visibility);
}


template<class TCtrl>
VOID CXSysControl<TCtrl>::Destroy()
{
	if (IsWindow())
		DestroyWindow();

	__super::Destroy();
}


template<class TCtrl>
CXFrame * CXSysControl<TCtrl>::RemoveFrame( UINT nIndex )
{
	return FALSE;
}

template<class TCtrl>
BOOL CXSysControl<TCtrl>::InsertFrame( CXFrame * pFrame, UINT nIndex )
{
	return FALSE;
}


template<class TCtrl>
BOOL CXSysControl<TCtrl>::SetRect( const CRect & rcNewFrameRect )
{
	if (CXFrame::GetRect() == rcNewFrameRect)
		return TRUE;

	BOOL bRtn = __super::SetRect(rcNewFrameRect);

	if (IsWindow() && !m_bUpdatePositionScheduled)
	{
		PostFrameMsg(FRAME_MSG_UPDATE_POSITION, 0, 0);
		m_bUpdatePositionScheduled = TRUE;
	}

	return bRtn;
}


template<class TCtrl>
BOOL CXSysControl<TCtrl>::UpdateSysControlPosition()
{
	if (!IsWindow())
		return FALSE;

	CRect rcThisFrame(CXFrame::GetRect());
	CRect rcWnd(FrameToScreen(ParentToChild(rcThisFrame)));

	MoveWindow(rcWnd.left, rcWnd.top, rcWnd.Width(), rcWnd.Height(), FALSE);

	return TRUE;
}

template<class TCtrl>
BOOL CXSysControl<TCtrl>::SetVisibility( VISIBILITY visibility )
{
	if (GetVisibility() == visibility)
		return TRUE;

	BOOL bRtn = __super::SetVisibility(visibility);

	if (IsWindow())
		UpdateShowState();

	return bRtn;
}

template<class TCtrl>
IXImage *  CXSysControl<TCtrl>::SetBackground( IXImage * pDrawBackground )
{
	return pDrawBackground;
}


template<class TCtrl>
BOOL CXSysControl<TCtrl>::InvalidateRect( const CRect & rect )
{
	return FALSE;
}


template<class TCtrl>
BOOL CXSysControl<TCtrl>::PaintUI( HDC hDC, const CRect &rect )
{
	return FALSE;
}

template<class TCtrl>
VOID CXSysControl<TCtrl>::OnAncestorRectChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!IsAncester(pSrcFrame))
		return;

	if (IsWindow() && !m_bUpdatePositionScheduled)
	{
		PostFrameMsg(FRAME_MSG_UPDATE_POSITION, 0, 0);
		m_bUpdatePositionScheduled = TRUE;
	}
}


template<class TCtrl>
CXSysControl<TCtrl>::CXSysControl( void )
	: m_bNeedRestoreWhenParentEnable(FALSE),
	m_bUpdatePositionScheduled(FALSE)
{
}

template<class TCtrl>
BOOL CXSysControl<TCtrl>::UpdateShowState()
{
	if (!IsWindow())
		return FALSE;

	BOOL bParentFrameShow = FALSE;
	CXFrame *p = CXFrame::GetParent();
	if (p && p->GetHWND())
	{
		bParentFrameShow = p->GetVisibility() == VISIBILITY_SHOW;
		p = p->GetParent();
	}
	
	while (bParentFrameShow && p)
	{
		if (p->GetVisibility() != VISIBILITY_SHOW)
			bParentFrameShow = FALSE;
		p = p->GetParent();
	}

	BOOL bWindowShowed = IsWindowVisible();

	BOOL bFrameShow = GetVisibility() == VISIBILITY_SHOW;
	BOOL bShow = bFrameShow && bParentFrameShow;

	if ((bWindowShowed && bShow) || (!bWindowShowed && !bShow ))
		return TRUE;

	ShowWindow(bShow ? SW_SHOW : SW_HIDE);

	return TRUE;
}	

template<class TCtrl>
VOID CXSysControl<TCtrl>::OnAncestorShowHide(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!IsAncester(pSrcFrame))
		return;

	if (!IsWindow())
		return;

	BOOL bShow = (BOOL)wParam;

	if (!bShow)
		ShowWindow(SW_HIDE);
	else
		UpdateShowState();
}

template<class TCtrl>
VOID CXSysControl<TCtrl>::OnParentWndEnabled(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	CXFrame *pRoot = this;
	while (pRoot && pRoot->GetParent())
		pRoot = pRoot->GetParent();

	if (pSrcFrame != pRoot)
		return;

	if (!IsWindow())
		return;

	if (!wParam)
	{
		if (IsWindowEnabled())
		{
			EnableWindow(FALSE);
			m_bNeedRestoreWhenParentEnable = TRUE;
		}
	}
	else
	{
		if (m_bNeedRestoreWhenParentEnable)
		{
			EnableWindow(TRUE);
			m_bNeedRestoreWhenParentEnable = FALSE;
		}
	}
}

template<class TCtrl>
BOOL CXSysControl<TCtrl>::IsAncester( CXFrame *pFrame )
{
	CXFrame *p = CXFrame::GetParent();
	while (p)
	{
		if (pFrame == p)
			return TRUE;

		p = p->GetParent();
	}

	return FALSE;
}

template<class TCtrl>
VOID CXSysControl<TCtrl>::OnOneAncestorDetachedFromaParent(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!IsAncester(pSrcFrame))
		return;

	CXFrame *pParentDetachedFrom = (CXFrame *)wParam;

	while (pParentDetachedFrom)
	{
		pParentDetachedFrom->RemoveEventListener(this);
		pParentDetachedFrom = pParentDetachedFrom->GetParent();
	}

	if (!IsWindow())
		return;

	CWindowImpl<CXSysControl<TCtrl>, TCtrl>::
		SetWindowLongPtr(GWL_HWNDPARENT, (LONG_PTR)NULL);

	UpdateSysControlPosition();

	UpdateShowState();
}

template<class TCtrl>
VOID CXSysControl<TCtrl>::OnOneAncestorAttachedToParent(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!IsAncester(pSrcFrame))
		return;

	CXFrame *pParentAttachedTo = (CXFrame *)wParam;

	while (pParentAttachedTo)
	{
		pParentAttachedTo->AddEventListener(this);
		pParentAttachedTo = pParentAttachedTo->GetParent();
	}

	if (!IsWindow())
		return;

	CWindowImpl<CXSysControl<TCtrl>, TCtrl>::
		SetWindowLongPtr(GWL_HWNDPARENT, (LONG_PTR)GetHWND());

	UpdateSysControlPosition();

	UpdateShowState();

}

template<class TCtrl>
VOID CXSysControl<TCtrl>::OnDetachedFromParent()
{
	CXFrame *pCurrentParent = CXFrame::GetParent();
	ATLASSERT(pCurrentParent);

	while (pCurrentParent)
	{
		pCurrentParent->RemoveEventListener(this);
		pCurrentParent = pCurrentParent->GetParent();
	}

	__super::OnDetachedFromParent();

	if (!IsWindow())
		return;

	CWindowImpl<CXSysControl<TCtrl>, TCtrl>::
		SetWindowLongPtr(GWL_HWNDPARENT, NULL);

	UpdateSysControlPosition();

	UpdateShowState();
}

template<class TCtrl>
VOID CXSysControl<TCtrl>::OnAttachedToParent(CXFrame *pParent)
{
	__super::OnAttachedToParent(pParent);

	while (pParent)
	{
		pParent->AddEventListener(this);
		pParent = pParent->GetParent();
	}

	if (!IsWindow())
		return;

	CWindowImpl<CXSysControl<TCtrl>, TCtrl>::
		SetWindowLongPtr(GWL_HWNDPARENT, (LONG_PTR)GetHWND());

	UpdateSysControlPosition();

	UpdateShowState();
}

template<class TCtrl>
LRESULT CXSysControl<TCtrl>::OnUpdatePositon( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	if (!m_bUpdatePositionScheduled)
		return 0;

	m_bUpdatePositionScheduled = FALSE;

	if (IsWindow())
		UpdateSysControlPosition();

	return 0;
}