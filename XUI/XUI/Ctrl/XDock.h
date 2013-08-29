#pragma once
#include "../Base/XFrame.h"

class CXDock :
	public CXFrame
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXDock, XDock)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXDock@@0VCXFrameXMLRuntime_CXDock@1@A")

public:
	enum DockType {DOCK_LEFT2RIGHT = 0, DOCK_TOP2BOTTOM, DOCK_RIGHT2LEFT, DOCK_BOTTOM2TOP};
	enum Align {ALIGN_LOW = 0, ALIGN_MIDDLE, ALIGN_HIGH};

public:
	BOOL Create(CXFrame * pFrameParent, DockType dock, Align align, LayoutParam * pLayout, VISIBILITY visibility = VISIBILITY_NONE);

public:
	virtual VOID Destroy();
	virtual BOOL OnMeasureWidth(const MeasureParam & param);
	virtual BOOL OnMeasureHeight(const MeasureParam & param);
	virtual BOOL OnLayout(const CRect & rcRect);


private:
	BOOL OnMeasureAlignDirection(
			const MeasureParam & param, INT *pMeasuredSize,
			BOOL (CXFrame::*pfChildMeasureProc)(const MeasureParam &),
			INT (CXFrame::*pfChildGetMeasurdProc)(),
			INT LayoutParam::*pLayoutParamPos, 
			INT LayoutParam::*pLayoutMarginLow,
			INT LayoutParam::*pLayoutMarginHigh,
			INT LayoutParam::*pnLayoutParamSize, 
			LayoutParam::SPECIAL_METRICS LayoutParam::*pmLayoutParamSize
		);

	BOOL OnMeasureLayoutDirection(
			const MeasureParam & param, INT *pMeasuredSize,
			BOOL (CXFrame::*pfChildMeasureProc)(const MeasureParam &),
			INT (CXFrame::*pfChildGetMeasurdProc)(),
			INT LayoutParam::*pLayoutParamPos, 
			INT LayoutParam::*pLayoutMarginLow,
			INT LayoutParam::*pLayoutMarginHigh,
			INT LayoutParam::*pnLayoutParamSize, 
			LayoutParam::SPECIAL_METRICS LayoutParam::*pmLayoutParamSize
		);

public:
	CXDock();

private:
	DockType m_DockType;
	Align m_Align;
};
