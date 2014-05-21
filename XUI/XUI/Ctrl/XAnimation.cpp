#include "StdAfx.h"
#include "XAnimation.h"
#include "..\..\..\XLib\inc\interfaceS\string\StringCode.h"

X_IMPLEMENT_FRAME_XML_RUNTIME(CXAnimation)

CXFrame * CXAnimation::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
{
	if (!xml)
		return NULL;

 	X_XML_ATTR_TYPE attr = NULL;
 
 	IXImage *pFrames = NULL;
 	UINT nFrameCount = 0;
 	UINT nSwitchInterval = 0;
 
 	pFrames = CXFrameXMLFactory::BuildImage(xml, "frames", "frames_type", "stretch", "frames_part_");
 	if (!pFrames)
 	{
 		CXFrameXMLFactory::ReportError("WARNING: Load frames for the XAnimation failed, Create the XAnimation failed. ");
 		return NULL;
 	}
 
 	attr = xml->first_attribute("frame_count", 0, false);
 	if (attr) nFrameCount = strtoul(attr->value(), NULL, 10);
 	if (!nFrameCount)
 	{
 		CXFrameXMLFactory::ReportError("WARNING: Wrong frame count specified for the XAnimation, Create the XAnimation failed. ");
 		return NULL;
 	}
 
 	attr = xml->first_attribute("switch_interval", 0, false);
 	if (attr) nSwitchInterval = strtoul(attr->value(), NULL, 10);
	if (!nSwitchInterval)
	{
		CXFrameXMLFactory::ReportError("WARNING: Wrong frame switch interval specified for the XAnimation, Create the XAnimation failed. ");
		return NULL;
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
 	
 	CXAnimation *pFrame = new CXAnimation();
	pFrame->Create(pParent, pFrames, nFrameCount, nSwitchInterval, pLayout, VISIBILITY_NONE);
 
 	return pFrame;
}


CXAnimation::CXAnimation(void)
	: m_pFrames(NULL),
	m_nFrameCount(0),
	m_nSwitchInterval(0),
	m_nTimerID(0),
	m_nCurrentFrame(0)
{
}

BOOL CXAnimation::SetRect( const CRect & rcNewFrameRect )
{
	if (GetRect() == rcNewFrameRect)
		return TRUE;

	if (m_pFrames)
		m_pFrames->SetDstRect(CRect(0, 0, rcNewFrameRect.Width(), rcNewFrameRect.Height()));

	return __super::SetRect(rcNewFrameRect);
}


BOOL CXAnimation::PaintForeground( HDC hDC, const CRect &rect )
{
	if (m_pFrames)
		m_pFrames->Draw(hDC, rect);

	return __super::PaintForeground(hDC, rect);
}

BOOL CXAnimation::SetFrames( IXImage *pFrames, UINT nFrameCount )
{
	if (m_pFrames == pFrames)
		return TRUE;

	if (m_pFrames)
		delete m_pFrames;

	m_pFrames = pFrames;
	m_nFrameCount = nFrameCount;

	if (m_pFrames)
	{
		CRect rcFrame(GetRect());
		m_pFrames->SetDstRect(CRect(0, 0, rcFrame.Width(), rcFrame.Height()));
		
		ATLASSERT(m_nFrameCount);
		if (m_nFrameCount)
		{
			ATLASSERT(pFrames->GetImageWidth() % m_nFrameCount == 0);
			m_pFrames->SetSrcRect(CRect(0, 0, pFrames->GetImageWidth() / m_nFrameCount, pFrames->GetImageHeight()));
		}
	}

	m_nCurrentFrame = 0;

	InvalidateLayout();
	InvalidateRect();

	return TRUE;
}

BOOL CXAnimation::SetFrameSwitchInterval( UINT nSwitchFrameInterval )
{
	if (m_nSwitchInterval == nSwitchFrameInterval)
		return TRUE;

	if (m_nTimerID)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}

	m_nSwitchInterval = nSwitchFrameInterval;

	if (m_nSwitchInterval)
		m_nTimerID = SetTimer(m_nSwitchInterval);

	return !m_nSwitchInterval || m_nTimerID;
}

LRESULT CXAnimation::OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	if (wParam != m_nTimerID)
		return 0;

	bHandled = TRUE;

	if (!m_nFrameCount)
		return 0;

	m_nCurrentFrame = (m_nCurrentFrame + 1) % m_nFrameCount;

	if (m_pFrames)
	{
		INT nFrameWidth = m_pFrames->GetImageWidth() / m_nFrameCount;
		INT nCurrentFrameLeft = nFrameWidth * m_nCurrentFrame;
		m_pFrames->SetSrcRect(CRect(nCurrentFrameLeft, 0, nCurrentFrameLeft + nFrameWidth, m_pFrames->GetImageHeight()));
		InvalidateRect();
	}

	return 0;
}

VOID CXAnimation::Destroy()
{
	SetFrames(NULL, 0);

	SetFrameSwitchInterval(0);

	return __super::Destroy();
}

BOOL CXAnimation::Create( CXFrame * pFrameParent, IXImage *pFrames, UINT nFrameCount, UINT nSwitchFrameInterval, 
						 LayoutParam * pLayout,  VISIBILITY visibility /*= VISIBILITY_NONE*/)
{
	BOOL bRtn = __super::Create(pFrameParent, pLayout, visibility);

	SetFrames(pFrames, nFrameCount);

	SetFrameSwitchInterval(nSwitchFrameInterval);

	return bRtn;
}

BOOL CXAnimation::OnMeasureWidth( const MeasureParam & param )
{
	BOOL bRtn = __super::OnMeasureWidth(param);

	if (param.m_Spec == MeasureParam::MEASURE_EXACT 
		|| !m_pFrames || !m_nFrameCount)
		return bRtn;

	int nMeasured = max(GetMeasuredWidth(), m_pFrames->GetImageWidth() / m_nFrameCount);
	if (param.m_Spec == MeasureParam::MEASURE_ATMOST)
		nMeasured = min(nMeasured, param.m_nNum);

	SetMeasuredWidth(nMeasured);

	return bRtn;
}

BOOL CXAnimation::OnMeasureHeight( const MeasureParam & param )
{
	BOOL bRtn = __super::OnMeasureHeight(param);

	if (param.m_Spec == MeasureParam::MEASURE_EXACT
		|| !m_pFrames || !m_nFrameCount)
		return bRtn;

	int nMeasured = max(GetMeasuredHeight(), m_pFrames->GetImageHeight());
	if (param.m_Spec == MeasureParam::MEASURE_ATMOST)
		nMeasured = min(nMeasured, param.m_nNum);

	SetMeasuredHeight(nMeasured);

	return bRtn;
}
