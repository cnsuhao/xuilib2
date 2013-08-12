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
		FRAME_EVENT_HANDLER(EVENT_FRAME_RECT_CHANGED, OnFrameRectChanged)
		FRAME_EVENT_FRAME_HANDLER(EVENT_SCROLLBAR_SCROLLCHANGED, m_pScrollH, OnHScroll)
		FRAME_EVENT_FRAME_HANDLER(EVENT_SCROLLBAR_SCROLLCHANGED, m_pScrollV, OnVScroll)
	END_FRAME_EVENT_MAP()

public:
	CXScrollFrame(UINT nScrollBar = SCROLL_BAR_H | SCROLL_BAR_V);

public:
	BOOL Create(CXFrame *pParent, const CRect & rcRect = CRect(0, 0, 0, 0), BOOL bVisible = FALSE,
		IXImage *pScrollHImageBG = NULL, IXImage *pScrollHImageFG = NULL,
		IXImage *pScrollVImageBG = NULL, IXImage *pScrollVImageFG = NULL,
		WIDTH_MODE aWidthMode = WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE aHeightMode = HEIGHT_MODE_NOT_CHANGE);

public:
	virtual VOID Destroy();

protected:
	virtual VOID ChangeFrameRect(const CRect & rcNewFrameRect);

public:
	virtual BOOL HandleXMLChildNodes(X_XML_NODE_TYPE xml);

public:
	BOOL AddContentFrame(CXFrame *pFrame);
	UINT GetContentFrameCount();
	CXFrame * RemoveContentFrame(UINT nIndex);

public:
	VOID OnFrameRectChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnHScroll(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnVScroll(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);

private:
	VOID UpdateViewAndScrollBars();

private:
	UINT m_nScrollBar;

private:
	CXFrame *m_pFrameView;
	CXScrollBar *m_pScrollH;
	CXScrollBar *m_pScrollV;
};
