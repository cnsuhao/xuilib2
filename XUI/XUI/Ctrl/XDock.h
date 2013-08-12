#pragma once
#include "../Base/XFrame.h"

class CXDock :
	public CXFrame
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXDock, XDock)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXDock@@0VCXFrameXMLRuntime_CXDock@1@A")

public:
	BEGIN_FRAME_EVENT_MAP(CXDock)
		CHAIN_FRAME_EVENT_MAP(CXFrame)
		FRAME_EVENT_HANDLER(EVENT_FRAME_RECT_CHANGED, OnChildFrameRectChanged)
		FRAME_EVENT_HANDLER(EVENT_FRAME_MARGIN_CHANGED, OnChildFrameMarginChanged)
		FRAME_EVENT_HANDLER(EVENT_FRAME_HOLDPLACE_STATE_CHANGED, OnChildFrameHoldPlaceStateChanged)
	END_FRAME_EVENT_MAP()

public:
	enum DockType {DOCK_LEFT2RIGHT = 0, DOCK_TOP2BOTTOM, DOCK_RIGHT2LEFT, DOCK_BOTTOM2TOP};
	enum Align {ALIGN_LOW = 0, ALIGN_MIDDLE, ALIGN_HIGH};

public:
	BOOL Create(CXFrame * pFrameParent, DockType dock, Align align, const CRect & rcRect = CRect(0, 0, 0, 0), BOOL bVisible = FALSE,
	WIDTH_MODE aWidthMode = WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE aHeightMode = HEIGHT_MODE_NOT_CHANGE);

public:
	virtual CXFrame * RemoveFrame(UINT nIndex);
	virtual BOOL InsertFrame(CXFrame * pFrame, UINT nIndex);
	virtual VOID Destroy();

protected:
	virtual VOID ChangeFrameRect(const CRect & rcNewFrameRect);

public:
	VOID OnChildFrameRectChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnChildFrameMarginChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnChildFrameHoldPlaceStateChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	

public:
	CXDock();

private:
	VOID RelayoutFrom(UINT nIndex);


private:
	DockType m_DockType;
	Align m_Align;
	BOOL m_bListenChildFrameRectMarginChange;
};
