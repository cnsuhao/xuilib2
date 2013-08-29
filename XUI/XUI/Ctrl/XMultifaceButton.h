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
		LayoutParam * pLayout,  VISIBILITY visibility = VISIBILITY_NONE,
		UINT nStartButtonFace = 0, BOOL bDisabled = FALSE);

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
