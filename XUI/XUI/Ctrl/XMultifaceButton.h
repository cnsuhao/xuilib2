#pragma once
#include "xbutton.h"

class CXMultifaceButton :
	public CXButton
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXMultifaceButton, XMultifaceButton)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXMultifaceButton@@0VCXFrameXMLRuntime_CXMultifaceButton@1@A")

public:
	BOOL Create( CXFrame * pFrameParent, 
		IXImage **pButtonFaces, UINT nButtonFaceCount,
		const CRect & rc = CRect(0, 0, 0, 0), BOOL bVisible = FALSE,
		UINT nStartButtonFace = 0, BOOL bDisabled = FALSE, 
		WIDTH_MODE aWidthMode = WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE aHeightMode = HEIGHT_MODE_NOT_CHANGE);

public:
	BOOL ChangeButtonFaceTo(UINT nButtonFaceIndex);

public:
	virtual VOID Destroy();

public:
	CXMultifaceButton(void);

private:
	std::vector<IXImage *> m_vButtonFaces;
	UINT m_nCurrentButtonFace;
};
