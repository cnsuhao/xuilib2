#include "StdAfx.h"
#include "XDock.h"

X_IMPLEMENT_FRAME_XML_RUNTIME(CXDock)

CXFrame * CXDock::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
{
 	if (!xml)
 		return NULL;

	X_XML_ATTR_TYPE attr = NULL;

	DockType eDockType = DOCK_LEFT2RIGHT;
	Align eAlignType = ALIGN_MIDDLE;
	
	attr = xml->first_attribute("dock_type", 0, false);
	if (attr)
	{
		if (!StrCmpIA(attr->value(), "left2right"))
			eDockType = DOCK_LEFT2RIGHT;
		else if (!StrCmpIA(attr->value(), "top2bottom"))
			eDockType = DOCK_TOP2BOTTOM;
		else if (!StrCmpIA(attr->value(), "right2left"))
			eDockType = DOCK_RIGHT2LEFT;
		else if (!StrCmpIA(attr->value(), "bottom2top"))
			eDockType = DOCK_BOTTOM2TOP;
	}

	attr = xml->first_attribute("align_type", 0, false);
	if (attr)
	{
		if (!StrCmpIA(attr->value(), "low"))
			eAlignType = ALIGN_LOW;
		else if (!StrCmpIA(attr->value(), "middle"))
			eAlignType = ALIGN_MIDDLE;
		else if (!StrCmpIA(attr->value(), "high"))
			eAlignType = ALIGN_HIGH;
	}

	CXDock *pDock = new CXDock();
	pDock->Create(pParent, eDockType, eAlignType, CXFrameXMLFactory::BuildRect(xml), FALSE,
		(CXFrame::WIDTH_MODE)CXFrameXMLFactory::GetWidthMode(xml),
		(CXFrame::HEIGHT_MODE)CXFrameXMLFactory::GetHeightMode(xml));

	return pDock;
}

CXDock::CXDock()
	: m_DockType(DOCK_LEFT2RIGHT), 
	m_Align(ALIGN_LOW),
	m_bListenChildFrameRectMarginChange(TRUE)
{
}

BOOL CXDock::InsertFrame( CXFrame * pFrame, UINT nIndex )
{
 	if (!pFrame)
 		return FALSE;
 
 	BOOL bIsFrameVisible = pFrame->IsVisible();
 	pFrame->SetVisible(FALSE);

	__super::InsertFrame(pFrame, nIndex);
 
 	RelayoutFrom(nIndex);
 
 	pFrame->SetVisible(bIsFrameVisible);

	pFrame->AddEventListener(this);
 
 	return TRUE;
}

CXFrame * CXDock::RemoveFrame( UINT nIndex )
{
	CXFrame *pFrame = GetFrameByIndex(nIndex);

 	if (!pFrame)
 		return NULL;

	pFrame->RemoveEventListener(this);
	pFrame = __super::RemoveFrame(nIndex);

	RelayoutFrom(nIndex);
 
	return pFrame;
}

VOID CXDock::RelayoutFrom( UINT nIndex )
{
	const UINT nFrameCount = GetFrameCount();

	if (nIndex >= nFrameCount)
		return;

	ThrowEvent(EVENT_DOCK_ITEMS_REDOCK_BEGIN, nIndex, 0);

	const CRect rcDock(GetRect());
	
	INT nStartPos = 0;

	if (m_DockType == DOCK_LEFT2RIGHT || m_DockType == DOCK_TOP2BOTTOM)
		nStartPos = 0;
	else if (m_DockType == DOCK_RIGHT2LEFT)
		nStartPos = rcDock.Width();
	else if (m_DockType == DOCK_BOTTOM2TOP)
		nStartPos = rcDock.Height();

	for (UINT i = nIndex; i < nFrameCount; i++)
	{
		if (i == nIndex && i > 0)
		{
			CXFrame *pLastHoldPlaceFrame = NULL;
			UINT j = i;
			do 
			{
				j--;
				CXFrame *p = GetFrameByIndex(j);
				if (p && p->IsHoldPlace())
				{
					pLastHoldPlaceFrame = p;
					break;
				}
			} while (j > 0);

			if (pLastHoldPlaceFrame)
			{
				CRect rcLastHoldPlaceFrameRect(pLastHoldPlaceFrame->GetRect());
				CRect rcLastHoldPlaceFrameMargin(pLastHoldPlaceFrame->GetMargin());

				if (m_DockType == DOCK_LEFT2RIGHT)
					nStartPos = rcLastHoldPlaceFrameRect.right + rcLastHoldPlaceFrameMargin.right;
				else if (m_DockType == DOCK_TOP2BOTTOM)
					nStartPos = rcLastHoldPlaceFrameRect.bottom + rcLastHoldPlaceFrameMargin.bottom;
				else if (m_DockType == DOCK_RIGHT2LEFT)
					nStartPos = rcLastHoldPlaceFrameRect.left - rcLastHoldPlaceFrameMargin.left;
				else if (m_DockType == DOCK_BOTTOM2TOP)
					nStartPos = rcLastHoldPlaceFrameRect.top - rcLastHoldPlaceFrameMargin.top;
			}
		}

		CXFrame *pFrameToLay = GetFrameByIndex(i);
		if (!pFrameToLay)
		{
			ATLASSERT(NULL);
			continue;
		}

		if ((m_DockType == DOCK_LEFT2RIGHT || m_DockType == DOCK_RIGHT2LEFT) && 
			pFrameToLay->GetWidthMode() == WIDTH_MODE_REACH_PARENT)
		{
			pFrameToLay->SetWidthHeightMode(CXFrame::WIDTH_MODE_NORMAL, CXFrame::HEIGHT_MODE_NOT_CHANGE);
			ATLASSERT(!_T("L2R or R2L dock can't own a frame with a WIDTH_MODE_REACH_PARENT. "));
		}
		if ((m_DockType == DOCK_TOP2BOTTOM || m_DockType == DOCK_BOTTOM2TOP) &&
			pFrameToLay->GetHeightMode() == HEIGHT_MODE_REACH_PARENT)
		{
			pFrameToLay->SetWidthHeightMode(CXFrame::WIDTH_MODE_NOT_CHANGE, CXFrame::HEIGHT_MODE_NORMAL);
			ATLASSERT(!_T("T2B or B2T dock can't own a frame with a HEIGHT_MODE_REACH_PARENT. "));
		}

		CRect rcSrcFrameToLay(pFrameToLay->GetRect());
		CSize szSizeFrameToLay(rcSrcFrameToLay.Size());
		CRect rcMarginFrameToLay(pFrameToLay->GetMargin());
		CRect rcTargetFrameToLay(0, 0, 0, 0);

		if (m_DockType == DOCK_LEFT2RIGHT)
		{
			rcTargetFrameToLay.left = nStartPos + rcMarginFrameToLay.left;
			rcTargetFrameToLay.right = rcTargetFrameToLay.left + szSizeFrameToLay.cx;
			nStartPos = rcTargetFrameToLay.right + rcMarginFrameToLay.right;
		}
		else if (m_DockType == DOCK_TOP2BOTTOM)
		{
			rcTargetFrameToLay.top = nStartPos + rcMarginFrameToLay.top;
			rcTargetFrameToLay.bottom = rcTargetFrameToLay.top + szSizeFrameToLay.cy;
			nStartPos = rcTargetFrameToLay.bottom + rcMarginFrameToLay.bottom;
		}
		else if (m_DockType == DOCK_RIGHT2LEFT)
		{
			rcTargetFrameToLay.right = nStartPos - rcMarginFrameToLay.right;
			rcTargetFrameToLay.left = rcTargetFrameToLay.right - szSizeFrameToLay.cx;
			nStartPos = rcTargetFrameToLay.left - rcMarginFrameToLay.left;
		}
		else if (m_DockType == DOCK_BOTTOM2TOP)
		{
			rcTargetFrameToLay.bottom = nStartPos - rcMarginFrameToLay.bottom;
			rcTargetFrameToLay.top = rcTargetFrameToLay.bottom - szSizeFrameToLay.cy;
			nStartPos = rcTargetFrameToLay.top - rcMarginFrameToLay.top;
		}

		if (pFrameToLay->GetWidthMode() == WIDTH_MODE_REACH_PARENT)
			rcTargetFrameToLay.left = rcTargetFrameToLay.right = 0;
		else if (pFrameToLay->GetHeightMode() == HEIGHT_MODE_REACH_PARENT)
			rcTargetFrameToLay.top = rcTargetFrameToLay.bottom = 0;
		else if (m_Align == ALIGN_LOW)
		{
			if (m_DockType == DOCK_LEFT2RIGHT || m_DockType == DOCK_RIGHT2LEFT)
			{
				rcTargetFrameToLay.top = rcMarginFrameToLay.top;
				rcTargetFrameToLay.bottom = rcTargetFrameToLay.top + szSizeFrameToLay.cy;
			}
			else
			{
				rcTargetFrameToLay.left = rcMarginFrameToLay.left;
				rcTargetFrameToLay.right = rcTargetFrameToLay.left + szSizeFrameToLay.cx;
			}
		}
		else if (m_Align == ALIGN_MIDDLE)
		{
			if (m_DockType == DOCK_LEFT2RIGHT || m_DockType == DOCK_RIGHT2LEFT)
			{
				rcTargetFrameToLay.top = (rcDock.Height() - szSizeFrameToLay.cy) / 2;
				rcTargetFrameToLay.bottom = rcTargetFrameToLay.top + szSizeFrameToLay.cy;
			}
			else
			{
				rcTargetFrameToLay.left = (rcDock.Width() - szSizeFrameToLay.cx) / 2;
				rcTargetFrameToLay.right = rcTargetFrameToLay.left + szSizeFrameToLay.cx;
			}
		}
		else if (m_Align == ALIGN_HIGH)
		{
			if (m_DockType == DOCK_LEFT2RIGHT || m_DockType == DOCK_RIGHT2LEFT)
			{
				rcTargetFrameToLay.bottom = rcDock.Height() - rcMarginFrameToLay.bottom;
				rcTargetFrameToLay.top = rcTargetFrameToLay.bottom - szSizeFrameToLay.cy;
			}
			else
			{
				rcTargetFrameToLay.right = rcDock.Width() - rcMarginFrameToLay.right;
				rcTargetFrameToLay.left = rcTargetFrameToLay.right - szSizeFrameToLay.cx;
			}
		}

		if (rcSrcFrameToLay.TopLeft() != rcTargetFrameToLay.TopLeft())
		{
			m_bListenChildFrameRectMarginChange = FALSE;
			pFrameToLay->Move(rcTargetFrameToLay.TopLeft());
			m_bListenChildFrameRectMarginChange = TRUE;
		}
	}


	ThrowEvent(EVENT_DOCK_ITEMS_REDOCKED, nIndex, 0);
}

VOID CXDock::OnChildFrameRectChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (!m_bListenChildFrameRectMarginChange)
		return;

	if (!pSrcFrame)
		return;

	UINT nFrameIndex = GetFrameIndex(pSrcFrame);
	if (nFrameIndex == INVALID_FRAME_INDEX)
		return;

	RelayoutFrom(nFrameIndex);
}

VOID CXDock::OnChildFrameMarginChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!m_bListenChildFrameRectMarginChange)
		return;

	if (!pSrcFrame)
		return;

	UINT nFrameIndex = GetFrameIndex(pSrcFrame);
	if (nFrameIndex == INVALID_FRAME_INDEX)
		return;

	RelayoutFrom(nFrameIndex);
}


VOID CXDock::OnChildFrameHoldPlaceStateChanged( CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!pSrcFrame)
		return;

	UINT nFrameIndex = GetFrameIndex(pSrcFrame);
	if (nFrameIndex == INVALID_FRAME_INDEX)
		return;

	RelayoutFrom(nFrameIndex);
}


VOID CXDock::ChangeFrameRect( const CRect & rcNewFrameRect )
{
	CRect rcFrameOld(GetRect());

	if (rcFrameOld == rcNewFrameRect)
		return;

	__super::ChangeFrameRect(rcNewFrameRect);

	BOOL bWidthChange = rcNewFrameRect.Width() != rcFrameOld.Width();
	BOOL bHeightChange = rcNewFrameRect.Height() != rcFrameOld.Height();

	if (m_Align == ALIGN_LOW)
	{
		if (m_DockType == DOCK_LEFT2RIGHT || m_DockType == DOCK_TOP2BOTTOM)
			return;

		if (m_DockType == DOCK_RIGHT2LEFT && !bWidthChange)
			return;

		if (m_DockType == DOCK_BOTTOM2TOP && !bHeightChange)
			return;
	}

	RelayoutFrom(0);
}

VOID CXDock::Destroy()
{
	__super::Destroy();

	m_DockType = DOCK_LEFT2RIGHT;
	m_Align = ALIGN_LOW;
	m_bListenChildFrameRectMarginChange = TRUE;
}

BOOL CXDock::Create( CXFrame * pFrameParent, DockType dock, Align align, const CRect & rcRect /*= CRect(0, 0, 0, 0)*/, BOOL bVisible /*= FALSE*/, 
					WIDTH_MODE aWidthMode /*= WIDTH_MODE_NOT_CHANGE*/, HEIGHT_MODE aHeightMode /*= HEIGHT_MODE_NOT_CHANGE*/ )
{
	BOOL bRtn = __super::Create(pFrameParent, rcRect, bVisible, aWidthMode, aHeightMode);

	m_DockType = dock;
	m_Align = align;

	return bRtn;
}

