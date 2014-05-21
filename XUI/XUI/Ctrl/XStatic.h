#pragma once
#include "../Base/XFrame.h"

#include "..\Draw\XDraw.h"

class CXStatic :
	public CXFrame
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXStatic, XStatic)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXStatic@@0VCXFrameXMLRuntime_CXStatic@1@A")

public:
	BOOL Create(CXFrame * pFrameParent, LayoutParam * pLayout, VISIBILITY visibility = VISIBILITY_NONE,
		IXText *pText = NULL);

public:
	virtual BOOL OnMeasureWidth(const MeasureParam & param);
	virtual BOOL OnMeasureHeight(const MeasureParam & param);

public:
	CXStatic(void);

public:
	IXText * SetText(IXText *pText);
	CString GetText();

public:
	virtual BOOL SetRect(const CRect & rcNewFrameRect);
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual VOID Destroy();
	

private:
	IXText *m_pText;
	INT m_nMeasredHeight;
};
