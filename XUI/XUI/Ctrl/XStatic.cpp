#include "StdAfx.h"
#include "XStatic.h"
#include "..\..\..\XLib\inc\interfaceS\string\StringCode.h"

X_IMPLEMENT_FRAME_XML_RUNTIME(CXStatic)

CXFrame * CXStatic::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
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

	IXText *pText = CXFrameXMLFactory::BuildText(xml);

	CXStatic *pStatic = new CXStatic();
	pStatic->Create(pParent, pLayout, VISIBILITY_NONE, pText);

	return pStatic;
}

CXStatic::CXStatic(void)
	: m_pText(NULL),
	m_nMeasredHeight(-1)
{
}


BOOL CXStatic::PaintForeground( HDC hDC, const CRect &rect )
{
	if (m_pText)
		m_pText->Draw(hDC, rect);

	return __super::PaintForeground(hDC, rect);
}

IXText * CXStatic::SetText( IXText *pText )
{
	if (pText == m_pText)
		return NULL;

	IXText *pOldText = m_pText;

	m_pText = pText;

	if (m_pText)
	{
		CRect rcFrame(GetRect());
		m_pText->SetDstRect(CRect(0, 0, rcFrame.Width(), rcFrame.Height()));
	}

	InvalidateLayout();
	InvalidateAfterLayout();

	return pOldText;
}

CString CXStatic::GetText()
{
	if (!m_pText)
		return CString(_T(""));

	return m_pText->GetText();
}

BOOL CXStatic::SetRect( const CRect & rcNewFrameRect )
{
	if (GetRect() == rcNewFrameRect)
		return TRUE;

	if (m_pText)
		m_pText->SetDstRect(CRect(0, 0, rcNewFrameRect.Width(), rcNewFrameRect.Height()));

	return __super::SetRect(rcNewFrameRect);
}
	
BOOL CXStatic::Create( CXFrame * pFrameParent, LayoutParam * pLayout,  VISIBILITY visibility /*= VISIBILITY_NONE*/, 
					  IXText *pText /*= NULL*/)
{
	BOOL bRtn =  __super::Create(pFrameParent, pLayout, visibility);

	delete SetText(pText);

	return bRtn;
}

VOID CXStatic::Destroy()
{
	delete SetText(NULL);

	m_nMeasredHeight = -1;

	__super::Destroy();
}

BOOL CXStatic::OnMeasureWidth( const MeasureParam & param )
{
	BOOL bRtn = __super::OnMeasureWidth(param);

	if (!m_pText)
		return bRtn;

	HDC dc = GetDC();

	CSize szText(0, 0);

	INT nMeasured = GetMeasuredWidth();

	if (param.m_Spec != MeasureParam::MEASURE_EXACT)
	{
		if (param.m_Spec == MeasureParam::MEASURE_ATMOST)
			szText = m_pText->Measure(dc, param.m_nNum);
		else
			szText = m_pText->Measure(dc);

		INT nNewMeasured = max(nMeasured, szText.cx);
		if (param.m_Spec == MeasureParam::MEASURE_ATMOST)
			nNewMeasured = min(nNewMeasured, param.m_nNum);

		SetMeasuredWidth(nNewMeasured);
	}
	else
		szText = m_pText->Measure(dc, nMeasured);

	m_nMeasredHeight = szText.cy;

	ReleaseDC(dc, FALSE);

	return bRtn;
}

BOOL CXStatic::OnMeasureHeight( const MeasureParam & param )
{
	BOOL bRtn = __super::OnMeasureHeight(param);

	if (!m_pText)
		return bRtn;

	if (param.m_Spec != MeasureParam::MEASURE_EXACT)
	{
		int nMeasured = GetMeasuredHeight();
		nMeasured = max(nMeasured, m_nMeasredHeight);
		
		if (param.m_Spec == MeasureParam::MEASURE_ATMOST)
			nMeasured = min(nMeasured, param.m_nNum);

		SetMeasuredHeight(nMeasured);
	}

	m_nMeasredHeight = 0;

	return bRtn;
}
