#include "StdAfx.h"
#include "XAnimation.h"

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
 
 	
 	CXAnimation *pFrame = new CXAnimation();
	pFrame->Create(pParent, pFrames, nFrameCount, nSwitchInterval, CXFrameXMLFactory::BuildRect(xml), FALSE,
		(CXFrame::WIDTH_MODE)CXFrameXMLFactory::GetWidthMode(xml), (CXFrame::HEIGHT_MODE)CXFrameXMLFactory::GetHeightMode(xml));
 
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

VOID CXAnimation::ChangeFrameRect( const CRect & rcNewFrameRect )
{
	if (GetRect() == rcNewFrameRect)
		return;

	if (m_pFrames)
		m_pFrames->SetDstRect(CRect(0, 0, rcNewFrameRect.Width(), rcNewFrameRect.Height()));

	__super::ChangeFrameRect(rcNewFrameRect);
}


BOOL CXAnimation::PaintForeground( HDC hDC, const CRect &rect )
{
	if (m_pFrames)
		m_pFrames->Draw(hDC, rect);

	return TRUE;
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

	if (GetWidthMode() == WIDTH_MODE_WRAP_CONTENT ||
		GetHeightMode() == HEIGHT_MODE_WRAP_CONTENT)
		RefreashFrameRect();

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

INT CXAnimation::CalculateWrapContentWidth()
{
	INT nParentWrapContentWidth = __super::CalculateWrapContentWidth();

	if (!m_pFrames || !m_nFrameCount)
		return nParentWrapContentWidth;

	return max(nParentWrapContentWidth, m_pFrames->GetImageWidth() / m_nFrameCount);
}

INT CXAnimation::CalculateWrapContentHeight()
{
	INT nParentWrapContentHeight = __super::CalculateWrapContentHeight();

	if (!m_pFrames)
		return nParentWrapContentHeight;

	return max(nParentWrapContentHeight, m_pFrames->GetImageHeight());
}

BOOL CXAnimation::Create( CXFrame * pFrameParent, IXImage *pFrames, UINT nFrameCount, UINT nSwitchFrameInterval, 
						 const CRect & rcRect /*= CRect(0, 0, 0, 0)*/, BOOL bVisible /*= FALSE*/, 
						 WIDTH_MODE aWidthMode /*= WIDTH_MODE_NOT_CHANGE*/, HEIGHT_MODE aHeightMode /*= HEIGHT_MODE_NOT_CHANGE*/ )
{
	BOOL bRtn = __super::Create(pFrameParent, rcRect, bVisible, aWidthMode, aHeightMode);

	SetFrames(pFrames, nFrameCount);

	SetFrameSwitchInterval(nSwitchFrameInterval);

	return bRtn;
}
