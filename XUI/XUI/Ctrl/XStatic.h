#pragma once
#include "../Base/XFrame.h"

#include "..\Draw\XDraw.h"

class CXStatic :
	public CXFrame
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXStatic, XStatic)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXStatic@@0VCXFrameXMLRuntime_CXStatic@1@A")

public:
	BOOL Create(CXFrame * pFrameParent, const CRect & rcRect = CRect(0, 0, 0, 0), BOOL bVisible = FALSE,
		IXText *pText = NULL,
		WIDTH_MODE aWidthMode = WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE aHeightMode = HEIGHT_MODE_NOT_CHANGE);

public:
	CXStatic(void);

public:
	IXText * SetText(IXText *pText);
	CString GetText();

public:
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual VOID Destroy();

protected:
	virtual VOID ChangeFrameRect(const CRect & rcNewFrameRect);

private:
	IXText *m_pText;
};
