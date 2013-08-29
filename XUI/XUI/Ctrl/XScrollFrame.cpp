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
	pFrame->Create(pParent, pLayout, VISIBILITY_NONE, 
		pBarBGH, pBarFGH, pBarBGV, pBarFGV);

	return pFrame;
}

CXScrollFrame::CXScrollFrame(UINT nScrollBar)
	: m_pScrollH(NULL), 
	m_pScrollV(NULL),
	m_pFrameView(NULL),
	m_pFrameContent(NULL),
	m_nScrollBar(nScrollBar)
{

}

BOOL CXScrollFrame::Create( CXFrame *pParent, LayoutParam * pLayout,  VISIBILITY visibility /*= VISIBILITY_NONE*/,
						   IXImage *pScrollHImageBG/* = NULL*/, IXImage *pScrollHImageFG/* = NULL*/,
						   IXImage *pScrollVImageBG/* = NULL*/, IXImage *pScrollVImageFG/* = NULL*/)
{
	if (!pLayout) 
	{
		ATLASSERT(!_T("No layout parameter. "));
		return FALSE;
	}

	BOOL bRtn =  __super::Create(pParent, pLayout, visibility);

	if ((m_nScrollBar & SCROLL_BAR_H) && !m_pScrollH)
	{
		m_pScrollH = new CXScrollBar();
		m_pScrollH->Create(this, CXScrollBar::SCROLL_H, GenerateLayoutParam(), 
			VISIBILITY_SHOW, pScrollHImageBG, pScrollHImageFG);
		m_pScrollH->AddEventListener(this);
	}

	if ((m_nScrollBar & SCROLL_BAR_V) && !m_pScrollV)
	{
		m_pScrollV = new CXScrollBar();
		m_pScrollV->Create(this, CXScrollBar::SCROLL_V, GenerateLayoutParam(), 
			VISIBILITY_SHOW, pScrollVImageBG, pScrollVImageFG);
		m_pScrollV->AddEventListener(this);
	}

	if (!m_pFrameView)
	{
		m_pFrameView = new CXFrame();
		m_pFrameView->Create(this, GenerateLayoutParam(), VISIBILITY_SHOW);
		m_pFrameView->AddEventListener(this);
	}

	if (!m_pFrameView->GetFrameCount())
	{
		LayoutParam * pLayout = GenerateLayoutParam();
		if (pLayout)
		{
			pLayout->m_mWidth = pLayout->m_mHeight 
				= LayoutParam::METRIC_WRAP_CONTENT;

			CXScrollContentContainer *pContentContainer = new CXScrollContentContainer();
			pContentContainer->Create(m_pFrameView, pLayout, VISIBILITY_SHOW);
			m_pFrameContent = pContentContainer;
			m_pFrameContent->AddEventListener(this);
		}
		else
		{
			ATLASSERT(NULL);
			return FALSE;
		}
	}

	return bRtn;
}

VOID CXScrollFrame::OnHScroll(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	ATLASSERT(m_pFrameContent);

	INT nPos = (INT)wParam;

	if (m_pFrameContent)
	{
		LayoutParam *pLayoutParma = 
			m_pFrameContent->BeginUpdateLayoutParam();
		ATLASSERT(pLayoutParma);
		if (pLayoutParma)
		{
			pLayoutParma->m_nX = -nPos;
			m_pFrameContent->EndUpdateLayoutParam();
		}
	}
}

VOID CXScrollFrame::OnVScroll(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	ATLASSERT(m_pFrameContent);

	INT nPos = (INT)wParam;

	if (m_pFrameContent)
	{
		LayoutParam *pLayoutParma = 
			m_pFrameContent->BeginUpdateLayoutParam();
		if (pLayoutParma)
		{
			pLayoutParma->m_nY = -nPos;
			m_pFrameContent->EndUpdateLayoutParam();
		}
	}
}

VOID CXScrollFrame::Destroy()
{
	m_pFrameView = m_pFrameContent = m_pScrollH = m_pScrollV = NULL;

	return __super::Destroy();
}

BOOL CXScrollFrame::AddContentFrame( CXFrame *pFrame )
{
	if (!pFrame)
		return FALSE;

	if (!m_pFrameView)
	{
		ATLASSERT(NULL);
		return FALSE;
	}

	CXFrame * pContentContainer = m_pFrameView->GetFrameByIndex(0);
	if (!pContentContainer)
	{
		ATLASSERT(NULL);
		return FALSE;
	}

	return pContentContainer->AddFrame(pFrame);
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
	if (!m_pFrameView)
	{
		ATLASSERT(NULL);
		return FALSE;
	}
	CXFrame * pContentContainer = m_pFrameView->GetFrameByIndex(0);
	if (!pContentContainer)
	{
		ATLASSERT(NULL);
		return FALSE;
	}

	for (X_XML_NODE_TYPE child = xml->first_node();
		child; child = child->next_sibling())
	{
		if (child->type() != X_XML_NODE_CATEGORY_TYPE::node_element)
			continue;
		CXFrame *pFrame = 
			CXFrameXMLFactory::Instance().BuildFrame(child, pContentContainer);
	}

	return TRUE;
}

LRESULT CXScrollFrame::OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	if (m_pScrollV && m_pScrollV->GetVisibility() == VISIBILITY_SHOW)
		m_pScrollV->GetFocus();
	else if (m_pScrollH && m_pScrollH->GetVisibility() == VISIBILITY_SHOW)
		m_pScrollH->GetFocus();

	return 0;
}

BOOL CXScrollFrame::OnMeasureWidth( const MeasureParam & param )
{
	ATLASSERT(m_pFrameView);
	ATLASSERT(!(m_nScrollBar & SCROLL_BAR_H) || m_pScrollH);
	ATLASSERT(!(m_nScrollBar & SCROLL_BAR_V) || m_pScrollV);

	int nMeasured = 0;

	if (param.m_Spec == MeasureParam::MEASURE_EXACT)
		nMeasured = param.m_nNum;

	MeasureParam ParamForMeasure;
	ParamForMeasure.m_Spec = MeasureParam::MEASURE_EXACT;
	
	if (m_pFrameView)
	{
		ParamForMeasure.m_nNum = nMeasured;
		if (m_nScrollBar & SCROLL_BAR_V)
			ParamForMeasure.m_nNum = max(0, ParamForMeasure.m_nNum - SCROLL_V_WIDTH);
		m_pFrameView->MeasureWidth(ParamForMeasure);
	}

	if (m_pScrollV)
	{
		ParamForMeasure.m_nNum = SCROLL_V_WIDTH;
		m_pScrollV->MeasureWidth(ParamForMeasure);
	}

	if (m_pScrollH)
	{
		ParamForMeasure.m_nNum = nMeasured;
		if (m_nScrollBar & SCROLL_BAR_V)
			ParamForMeasure.m_nNum = max(0, ParamForMeasure.m_nNum - SCROLL_V_WIDTH);
		m_pScrollH->MeasureWidth(ParamForMeasure);
	}

	SetMeasuredWidth(nMeasured);

	return TRUE;
}

BOOL CXScrollFrame::OnMeasureHeight( const MeasureParam & param )
{
	ATLASSERT(m_pFrameView);
	ATLASSERT(!(m_nScrollBar & SCROLL_BAR_H) || m_pScrollH);
	ATLASSERT(!(m_nScrollBar & SCROLL_BAR_V) || m_pScrollV);

	int nMeasured = 0;

	if (param.m_Spec == MeasureParam::MEASURE_EXACT)
		nMeasured = param.m_nNum;

	MeasureParam ParamForMeasure;
	ParamForMeasure.m_Spec = MeasureParam::MEASURE_EXACT;

	if (m_pFrameView)
	{
		ParamForMeasure.m_nNum = nMeasured;
		if (m_nScrollBar & SCROLL_BAR_H)
			ParamForMeasure.m_nNum = max(0, ParamForMeasure.m_nNum - SCROLL_H_HEIGHT);
		m_pFrameView->MeasureHeight(ParamForMeasure);
	}

	if (m_pScrollV)
	{
		ParamForMeasure.m_nNum = nMeasured;
		if (m_nScrollBar & SCROLL_BAR_H)
			ParamForMeasure.m_nNum = max(0, ParamForMeasure.m_nNum - SCROLL_H_HEIGHT);
		m_pScrollV->MeasureHeight(ParamForMeasure);
	}

	if (m_pScrollH)
	{
		ParamForMeasure.m_nNum = SCROLL_H_HEIGHT;
		m_pScrollH->MeasureHeight(ParamForMeasure);
	}

	SetMeasuredHeight(nMeasured);

	return TRUE;
}

BOOL CXScrollFrame::OnLayout( const CRect & rcRect )
{
	int nWidth = rcRect.Width();
	int nHeight = rcRect.Height();

	ATLASSERT(m_pFrameView);
	ATLASSERT(!(m_nScrollBar & SCROLL_BAR_H) || m_pScrollH);
	ATLASSERT(!(m_nScrollBar & SCROLL_BAR_V) || m_pScrollV);

	if (m_pFrameView)
		m_pFrameView->Layout(CRect(0, 0, 
			0 + m_pFrameView->GetMeasuredWidth(), 
			0 + m_pFrameView->GetMeasuredHeight()));

	if (m_pScrollV)
		m_pScrollV->Layout(CRect(nWidth - m_pScrollV->GetMeasuredWidth(), 0, 
			nWidth, 0 + m_pScrollV->GetMeasuredHeight()));

	if (m_pScrollH)
		m_pScrollH->Layout(CRect(0, nHeight - m_pScrollH->GetMeasuredHeight(),
			0 + m_pScrollH->GetMeasuredWidth(), nHeight));

	return TRUE;
}

VOID CXScrollFrame::OnViewRectChanged( CXFrame *pSrcFrame,
											 UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	const CRect &rcOld = *(CRect *)wParam;
	const CRect &rcNew = *(CRect *)lParam;

	OnViewOrContentChanged(&CXScrollBar::SetViewLen, rcOld, rcNew);
}

VOID CXScrollFrame::OnContentRectChanged( CXFrame *pSrcFrame, 
										 UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	const CRect &rcOld = *(CRect *)wParam;
	const CRect &rcNew = *(CRect *)lParam;

	OnViewOrContentChanged(&CXScrollBar::SetContentLen, rcOld, rcNew);
}

VOID CXScrollFrame::OnViewOrContentChanged( BOOL (CXScrollBar::*pfnSetLen)(INT),
										   const CRect &rcOld, const CRect &rcNew)
{
	ATLASSERT(m_pFrameView && m_pFrameContent);
	ATLASSERT(!(m_nScrollBar & SCROLL_BAR_H) || m_pScrollH);
	ATLASSERT(!(m_nScrollBar & SCROLL_BAR_V) || m_pScrollV);

	if (m_pScrollV && rcOld.Height() != rcNew.Height())
	{
		int nOldPos = m_pScrollV->GetScrollPos();
		(m_pScrollV->*pfnSetLen)(rcNew.Height());
		int nNewPos = m_pScrollV->GetScrollPos();
		if (nOldPos != nNewPos && m_pFrameContent)
		{
			LayoutParam *pLayoutParam = 
				m_pFrameContent->BeginUpdateLayoutParam();
			ATLASSERT(pLayoutParam);
			if (pLayoutParam)
			{
				pLayoutParam->m_nY = -nNewPos;
				m_pFrameContent->EndUpdateLayoutParam();
			}
		}
	}

	if (m_pScrollH && rcOld.Width() != rcNew.Width())
	{
		int nOldPos = m_pScrollH->GetScrollPos();
		(m_pScrollH->*pfnSetLen)(rcNew.Width());
		int nNewPos = m_pScrollH->GetScrollPos();
		if (nOldPos != nNewPos && m_pFrameContent)
		{
			LayoutParam *pLayoutParam = 
				m_pFrameContent->BeginUpdateLayoutParam();
			ATLASSERT(pLayoutParam);
			if (pLayoutParam)
			{
				pLayoutParam->m_nX = -nNewPos;
				m_pFrameContent->EndUpdateLayoutParam();
			}
		}
	}
}

