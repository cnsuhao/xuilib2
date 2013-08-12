#include "StdAfx.h"

#include "XScrollFrame.h"

#include "..\..\..\XLib\inc\interfaceS\string\StringCode.h"

X_IMPLEMENT_FRAME_XML_RUNTIME(CXScrollFrame)

#define SCROLL_H_HEIGHT 15
#define SCROLL_V_WIDTH 15

CXFrame * CXScrollFrame::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
{
	if (!xml)
		return NULL;

	X_XML_ATTR_TYPE attr = NULL;

	UINT nScrollType = 0;
	IXImage *pBarBGH = NULL;
	IXImage *pBarFGH = NULL;
	IXImage *pBarBGV = NULL;
	IXImage *pBarFGV = NULL;

	attr = xml->first_attribute("h_scroll", 0, false);
	if (attr && !StrCmpIA(attr->value(), "true"))
		nScrollType |= SCROLL_BAR_H;
	attr = xml->first_attribute("v_scroll", 0, false);
	if (attr && !StrCmpIA(attr->value(), "true"))
		nScrollType |= SCROLL_BAR_V;

	if (!nScrollType)
	{
		CXFrameXMLFactory::ReportError("WARNING: No scroll bar specified for the scroll frame. Create the scroll frame failed. ");
		return NULL;
	}

	if (nScrollType & SCROLL_BAR_H)
	{
		pBarBGH = CXFrameXMLFactory::BuildImage(xml, "scroll_bar_h_face_bg", NULL, "3partH", "scroll_bar_h_bg_part_");
		pBarFGH = CXFrameXMLFactory::BuildImage(xml, "scroll_bar_h_face_fg", NULL, "3partH", "scroll_bar_h_fg_part_");
	}

	if (nScrollType & SCROLL_BAR_V)
	{
		pBarBGV = CXFrameXMLFactory::BuildImage(xml, "scroll_bar_v_face_bg", NULL, "3partV", "scroll_bar_v_bg_part_");
		pBarFGV = CXFrameXMLFactory::BuildImage(xml, "scroll_bar_v_face_fg", NULL, "3partV", "scroll_bar_v_fg_part_");
	}

	CXScrollFrame *pFrame = new CXScrollFrame(nScrollType);
	pFrame->Create(pParent,CXFrameXMLFactory::BuildRect(xml), FALSE, 
		pBarBGH, pBarFGH, pBarBGV, pBarFGV, 
		(CXFrame::WIDTH_MODE)CXFrameXMLFactory::GetWidthMode(xml),
		(CXFrame::HEIGHT_MODE)CXFrameXMLFactory::GetHeightMode(xml));

	return pFrame;
}

CXScrollFrame::CXScrollFrame(UINT nScrollBar)
	: m_pScrollH(NULL), 
	m_pScrollV(NULL),
	m_pFrameView(NULL),
	m_nScrollBar(nScrollBar)
{

}

VOID CXScrollFrame::ChangeFrameRect( const CRect & rcNewFrameRect )
{
	if (GetRect() == rcNewFrameRect)
		return;

	__super::ChangeFrameRect(rcNewFrameRect);

	UpdateViewAndScrollBars();
}

VOID CXScrollFrame::UpdateViewAndScrollBars()
{
	if ((m_nScrollBar & SCROLL_BAR_H) && !m_pScrollH)
		return;
	if ((m_nScrollBar & SCROLL_BAR_V) && !m_pScrollV)
		return;

	if (!m_pFrameView || !m_pFrameView->GetFrameByIndex(0)
		|| !m_pFrameView->GetFrameByIndex(0)->GetFrameCount())
	{
		if (m_nScrollBar & SCROLL_BAR_H)
			m_pScrollH->SetVisible(FALSE);
		if (m_nScrollBar & SCROLL_BAR_V)
			m_pScrollV->SetVisible(FALSE);

		if (m_pFrameView)
			m_pFrameView->SetVisible(FALSE);

		return;
	}

	CRect rcFrame(GetRect());

	if (m_nScrollBar & SCROLL_BAR_H)
	{
		CRect rcScrollBarH(0, rcFrame.Height() - SCROLL_H_HEIGHT, 
			(m_nScrollBar & SCROLL_BAR_V) ? rcFrame.Width() - SCROLL_V_WIDTH : rcFrame.Width(), 
			rcFrame.Height());

		m_pScrollH->SetRect(rcScrollBarH);
	}
	if (m_nScrollBar & SCROLL_BAR_V)
	{
		CRect rcScrollBarV(rcFrame.Width() - SCROLL_V_WIDTH, 0, 
			rcFrame.Width(), 
			(m_nScrollBar & SCROLL_BAR_H) ? rcFrame.Height() - SCROLL_H_HEIGHT : rcFrame.Height());

		m_pScrollV->SetRect(rcScrollBarV);
	}

	CXFrame *pInnerFrame = m_pFrameView->GetFrameByIndex(0);

	CRect rcScrollSpace(0, 0, 
		(m_nScrollBar & SCROLL_BAR_V) ? rcFrame.Width() - SCROLL_V_WIDTH : rcFrame.Width(),
		(m_nScrollBar & SCROLL_BAR_H) ? rcFrame.Height() - SCROLL_H_HEIGHT : rcFrame.Height());

	if (rcScrollSpace.right < 0)
		rcScrollSpace.right = 0;
	if (rcScrollSpace.bottom < 0)
		rcScrollSpace.bottom = 0;
	
	CRect rcInnerFrame(pInnerFrame->GetRect());

	CRect rcInnerFrameTarget(rcInnerFrame);
	if (rcInnerFrameTarget.right < rcScrollSpace.Width())
		rcInnerFrameTarget.OffsetRect(rcScrollSpace.Width() - rcInnerFrameTarget.right, 0);
	if (rcInnerFrameTarget.left > 0)
		rcInnerFrameTarget.OffsetRect(-rcInnerFrameTarget.left, 0);
	if (rcInnerFrameTarget.bottom < rcScrollSpace.Height())
		rcInnerFrameTarget.OffsetRect(0, rcScrollSpace.Height() - rcInnerFrameTarget.bottom);
	if (rcInnerFrameTarget.top > 0)
		rcInnerFrameTarget.OffsetRect(0, -rcInnerFrameTarget.top);

	if (rcInnerFrameTarget != rcInnerFrame)
	{
		pInnerFrame->SetRect(rcInnerFrameTarget);
		return;
	}
	
	if (m_nScrollBar & SCROLL_BAR_H)
	{
		m_pScrollH->SetViewLen(rcScrollSpace.Width());
		m_pScrollH->SetFrameLen(rcInnerFrameTarget.Width());
		m_pScrollH->SetScrollPos(-rcInnerFrameTarget.left);
		m_pScrollH->SetVisible(TRUE);
	}
	if (m_nScrollBar & SCROLL_BAR_V)
	{
		m_pScrollV->SetViewLen(rcScrollSpace.Height());
		m_pScrollV->SetFrameLen(rcInnerFrameTarget.Height());
		m_pScrollV->SetScrollPos(-rcInnerFrameTarget.top);
		m_pScrollV->SetVisible(TRUE);
	}

	m_pFrameView->SetRect(rcScrollSpace);
	m_pFrameView->SetVisible(TRUE);
}

VOID CXScrollFrame::OnFrameRectChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!m_pFrameView || pSrcFrame != m_pFrameView->GetFrameByIndex(0))
		return;

	UpdateViewAndScrollBars();
}

BOOL CXScrollFrame::Create( CXFrame *pParent, const CRect & rcRect /*= CRect(0, 0, 0, 0)*/, BOOL bVisible /*= FALSE*/,
						   IXImage *pScrollHImageBG/* = NULL*/, IXImage *pScrollHImageFG/* = NULL*/,
						   IXImage *pScrollVImageBG/* = NULL*/, IXImage *pScrollVImageFG/* = NULL*/, 
						   WIDTH_MODE aWidthMode /*= WIDTH_MODE_NOT_CHANGE*/, HEIGHT_MODE aHeightMode /*= HEIGHT_MODE_NOT_CHANGE*/)
{
	BOOL bRtn =  __super::Create(pParent, rcRect, bVisible, aWidthMode, aHeightMode);

	if ((m_nScrollBar & SCROLL_BAR_H) && !m_pScrollH)
	{
		m_pScrollH = new CXScrollBar();
		m_pScrollH->Create(this, CXScrollBar::SCROLL_H, CRect(0, 0, 0, 0), FALSE, pScrollHImageBG, pScrollHImageFG);
		m_pScrollH->AddEventListener(this);
	}

	if ((m_nScrollBar & SCROLL_BAR_V) && !m_pScrollV)
	{
		m_pScrollV = new CXScrollBar();
		m_pScrollV->Create(this, CXScrollBar::SCROLL_V, CRect(0, 0, 0, 0), FALSE, pScrollVImageBG, pScrollVImageFG);
		m_pScrollV->AddEventListener(this);
	}

	return bRtn;
}

VOID CXScrollFrame::OnHScroll(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	INT nPos = (INT)wParam;

	CXFrame *pInnerFrame = NULL;
	if (m_pFrameView && m_pFrameView->GetFrameCount())
		pInnerFrame = m_pFrameView->GetFrameByIndex(0);

	if (pInnerFrame)
	{
		CRect rcTarget(pInnerFrame->GetRect());
		rcTarget.MoveToX(-nPos);
		pInnerFrame->SetRect(rcTarget);
	}
}

VOID CXScrollFrame::OnVScroll(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	INT nPos = (INT)wParam;

	CXFrame *pInnerFrame = NULL;
	if (m_pFrameView && m_pFrameView->GetFrameCount())
		pInnerFrame = m_pFrameView->GetFrameByIndex(0);

	if (pInnerFrame)
	{
		CRect rcTarget(pInnerFrame->GetRect());
		rcTarget.MoveToY(-nPos);
		pInnerFrame->SetRect(rcTarget);
	}
}

VOID CXScrollFrame::Destroy()
{
	m_pFrameView = m_pScrollH = m_pScrollV = NULL;

	return __super::Destroy();
}

BOOL CXScrollFrame::AddContentFrame( CXFrame *pFrame )
{
	if (!pFrame)
		return FALSE;

	if (!m_pFrameView)
	{
		m_pFrameView = new CXFrame();
		m_pFrameView->Create(this);
	}

	if (!m_pFrameView->GetFrameCount())
	{
		CXFrame *pContentContainer = new CXFrame();
		pContentContainer->Create(m_pFrameView, CRect(0, 0, 0, 0), 
			TRUE, WIDTH_MODE_WRAP_CONTENT, HEIGHT_MODE_WRAP_CONTENT);
		pContentContainer->AddEventListener(this);
	}

	return m_pFrameView->GetFrameByIndex(0)->AddFrame(pFrame);
}

UINT CXScrollFrame::GetContentFrameCount()
{
	if (!m_pFrameView)
		return 0;

	CXFrame *pContentContainer = 
		m_pFrameView->GetFrameByIndex(0);

	if (!pContentContainer)
		return 0;

	return pContentContainer->GetFrameCount();
}

CXFrame * CXScrollFrame::RemoveContentFrame( UINT nIndex )
{
	if (!m_pFrameView)
		return NULL;

	if (nIndex >= GetContentFrameCount())
		return NULL;

	CXFrame *pContentContainer = 
		m_pFrameView->GetFrameByIndex(0);

	if (!pContentContainer)
		return NULL;

	return pContentContainer->RemoveFrame(nIndex);
}

BOOL CXScrollFrame::HandleXMLChildNodes( X_XML_NODE_TYPE xml )
{
	for (X_XML_NODE_TYPE child = xml->first_node();
		child; child = child->next_sibling())
	{
		if (child->type() != X_XML_NODE_CATEGORY_TYPE::node_element)
			continue;
		CXFrame *pFrame = 
			CXFrameXMLFactory::Instance().BuildFrame(child, NULL);

		AddContentFrame(pFrame);
	}

	return TRUE;
}

LRESULT CXScrollFrame::OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	if (m_pScrollV && m_pScrollV->IsVisible())
		m_pScrollV->GetFocus();
	else if (m_pScrollH && m_pScrollH->IsVisible())
		m_pScrollH->GetFocus();

	return 0;
}

