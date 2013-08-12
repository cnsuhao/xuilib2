#include "StdAfx.h"
#include "XStatic.h"

X_IMPLEMENT_FRAME_XML_RUNTIME(CXStatic)

CXFrame * CXStatic::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
{
	if (!xml)
		return NULL;

	X_XML_ATTR_TYPE attr = NULL;

	IXText *pText = CXFrameXMLFactory::BuildText(xml);


	CXStatic *pStatic = new CXStatic();
	pStatic->Create(pParent, CXFrameXMLFactory::BuildRect(xml), FALSE,
		pText,
		(CXFrame::WIDTH_MODE)CXFrameXMLFactory::GetWidthMode(xml),
		(CXFrame::HEIGHT_MODE)CXFrameXMLFactory::GetHeightMode(xml));

	return pStatic;
}

CXStatic::CXStatic(void)
	: m_pText(NULL)
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

	InvalidateRect();

	return pOldText;
}

CString CXStatic::GetText()
{
	if (!m_pText)
		return CString(_T(""));

	return m_pText->GetText();
}

VOID CXStatic::ChangeFrameRect( const CRect & rcNewFrameRect )
{
	if (GetRect() == rcNewFrameRect)
		return;

	if (m_pText)
		m_pText->SetDstRect(CRect(0, 0, rcNewFrameRect.Width(), rcNewFrameRect.Height()));

	__super::ChangeFrameRect(rcNewFrameRect);
}
	
BOOL CXStatic::Create( CXFrame * pFrameParent, const CRect & rcRect /*= CRect(0, 0, 0, 0)*/, BOOL bVisible /*= FALSE*/, 
					  IXText *pText /*= NULL*/, WIDTH_MODE aWidthMode /*= WIDTH_MODE_NOT_CHANGE*/, HEIGHT_MODE aHeightMode /*= HEIGHT_MODE_NOT_CHANGE*/ )
{
	BOOL bRtn =  __super::Create(pFrameParent, rcRect, bVisible, aWidthMode, aHeightMode);

	delete SetText(pText);

	return bRtn;
}

VOID CXStatic::Destroy()
{
	delete SetText(NULL);

	__super::Destroy();
}