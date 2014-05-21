#pragma once

#include "XScrollBar.h"

class CXScrollFrame :
	public CXFrame
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXScrollFrame, XScrollFrame)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXScrollFrame@@0VCXFrameXMLRuntime_CXScrollFrame@1@A")

public:
	enum ScrollBarType{ SCROLL_BAR_H = 1, SCROLL_BAR_V = 2 };

public:
	BEGIN_FRAME_MSG_MAP(CXScrollFrame)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	END_FRAME_MSG_MAP()

	BEGIN_FRAME_EVENT_MAP(CXScrollFrame)
		CHAIN_FRAME_EVENT_MAP(CXFrame)
		FRAME_EVENT_FRAME_HANDLER(EVENT_SCROLLBAR_SCROLLCHANGED, m_pScrollH, OnHScroll)
		FRAME_EVENT_FRAME_HANDLER(EVENT_SCROLLBAR_SCROLLCHANGED, m_pScrollV, OnVScroll)
		FRAME_EVENT_FRAME_HANDLER(EVENT_FRAME_RECT_CHANGED, m_pFrameView, OnViewRectChanged)
		FRAME_EVENT_FRAME_HANDLER(EVENT_SCROLLVIEW_BOUND_CHANGED, m_pFrameView, OnContentRectChanged)
	END_FRAME_EVENT_MAP()

public:
	CXScrollFrame(UINT nScrollBar = SCROLL_BAR_H | SCROLL_BAR_V);

public:
	BOOL Create(CXFrame *pParent, LayoutParam * pLayout,  VISIBILITY visibility = VISIBILITY_NONE,
		IXImage *pScrollHImageBG = NULL, IXImage *pScrollHImageFG = NULL,
		IXImage *pScrollVImageBG = NULL, IXImage *pScrollVImageFG = NULL);

public:
	virtual VOID Destroy();
	virtual BOOL OnMeasureWidth(const MeasureParam & param);
	virtual BOOL OnMeasureHeight(const MeasureParam & param);
	virtual BOOL OnLayout(const CRect & rcRect);

public:
	virtual BOOL HandleXMLChildNodes(X_XML_NODE_TYPE xml);

public:
	BOOL AddContentFrame(CXFrame *pFrame);
	UINT GetContentFrameCount();
	CXFrame * RemoveContentFrame(UINT nIndex);

public:
	VOID OnHScroll(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnVScroll(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnViewRectChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnContentRectChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);

private:
	VOID OnViewOrContentChanged(BOOL (CXScrollBar::*pfnSetLen)(INT), 
		INT nOldWidth, INT nOldHeight, INT nNewWidth, INT nNewHeight);

private:
	UINT m_nScrollBar;

private:
	CXFrame *m_pFrameView;
	CXScrollBar *m_pScrollH;
	CXScrollBar *m_pScrollV;
};
