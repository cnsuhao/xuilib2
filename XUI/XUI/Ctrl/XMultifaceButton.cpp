#include "StdAfx.h"
#include "XMultifaceButton.h"

#define NOT_DELETE

X_IMPLEMENT_FRAME_XML_RUNTIME(CXMultifaceButton)

CXFrame * CXMultifaceButton::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
{
 	if (!xml)
 		return NULL;
 
 	X_XML_ATTR_TYPE attr = NULL;
 
 	BOOL bDisabled = FALSE;
	UINT nStartButtonFace = 0;
 
 	attr = xml->first_attribute("bg", 0, false);
 	if (attr) xml->remove_attribute(attr);

	std::vector<IXImage *> vImageList;
	CXFrameXMLFactory::BuildImageList(&vImageList, xml, "faces", "face_types_", "stretch", "face_parts_");
 
 	if (!vImageList.size())
 		CXFrameXMLFactory::ReportError("WARNING: No face specified for the multi-face button, will use default button face. ");

	attr = xml->first_attribute("start_face", 0, false);
	if (attr) nStartButtonFace = strtoul(attr->value(), NULL, 10);
 
 	attr = xml->first_attribute("disabled", 0, false);
 	if (attr && !StrCmpIA(attr->value(), "true"))
 		bDisabled = TRUE;
 
 	CXMultifaceButton *pFrame = new CXMultifaceButton();
	pFrame->Create(pParent, vImageList.size() ? &vImageList[0] : NULL, vImageList.size(), 
		CXFrameXMLFactory::BuildRect(xml), FALSE,
		nStartButtonFace, bDisabled,
		(CXFrame::WIDTH_MODE)CXFrameXMLFactory::GetWidthMode(xml), (CXFrame::HEIGHT_MODE)CXFrameXMLFactory::GetHeightMode(xml));
 
 	return pFrame;

}

CXMultifaceButton::CXMultifaceButton(void)
	: m_nCurrentButtonFace(0)
{
}

BOOL CXMultifaceButton::Create(CXFrame * pFrameParent, 
							   IXImage **pButtonFaces, UINT nButtonFaceCount, 
							   const CRect & rc /*= CRect(0, 0, 0, 0)*/, BOOL bVisible /*= FALSE*/, 
							   UINT nStartButtonFace /*= 0*/, BOOL bDisabled /*= FALSE*/, 
							   WIDTH_MODE aWidthMode /*= WIDTH_MODE_NOT_CHANGE*/, HEIGHT_MODE aHeightMode /*= HEIGHT_MODE_NOT_CHANGE*/ )
{
	if (pButtonFaces && nButtonFaceCount > 0)
		m_vButtonFaces.insert(m_vButtonFaces.end(), pButtonFaces, pButtonFaces + nButtonFaceCount);

	IXImage *pCurrentButtonFace = m_vButtonFaces.size() ? pButtonFaces[0] : NULL;
	if (nStartButtonFace < nButtonFaceCount)
	{
		m_nCurrentButtonFace = nStartButtonFace;
		pCurrentButtonFace = pButtonFaces[nStartButtonFace];
	}

	return __super::Create(pFrameParent, rc, bVisible, bDisabled, pCurrentButtonFace, aWidthMode, aHeightMode);
}

BOOL CXMultifaceButton::ChangeButtonFaceTo( UINT nButtonFaceIndex )
{
	if (nButtonFaceIndex >= m_vButtonFaces.size())
		return FALSE;

	if (m_nCurrentButtonFace == nButtonFaceIndex)
		return TRUE;

	m_nCurrentButtonFace = nButtonFaceIndex;

	NOT_DELETE SetBackground(m_vButtonFaces[nButtonFaceIndex]);

	return TRUE;
}

VOID CXMultifaceButton::Destroy()
{
	NOT_DELETE SetBackground(NULL);

	for (std::vector<IXImage *>::size_type i = 0;
		i < m_vButtonFaces.size(); i++)
		delete m_vButtonFaces[i];

	m_vButtonFaces.clear();

	m_nCurrentButtonFace = 0;

	__super::Destroy();
}

