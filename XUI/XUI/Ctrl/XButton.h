#pragma once

#include "../Base/XFrame.h"

#define APAPT_BACKGROUND -3

class CXButton : public CXFrame
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXButton, XButton)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXButton@@0VCXFrameXMLRuntime_CXButton@1@A")

public:
	BEGIN_FRAME_MSG_MAP(CXButton)
		CHAIN_FRAME_MSG_MAP(CXFrame)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		FRAME_MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		FRAME_MESSAGE_HANDLER(WM_X_MOUSEENTER, OnMouseEnter)
	END_FRAME_MSG_MAP()

public:
	BOOL Create( CXFrame * pFrameParent, LayoutParam * pLayout,  VISIBILITY visibility = VISIBILITY_NONE,
		BOOL bDisabled = FALSE, IXImage *pBackground = NULL);

public:
	virtual VOID Destroy();
	virtual BOOL OnMeasureWidth(const MeasureParam & param);
	virtual BOOL OnMeasureHeight(const MeasureParam & param);
	virtual BOOL OnLayout(const CRect & rcRect);
	virtual IXImage * SetBackground(IXImage * pDrawBackground);

public:
	BOOL EnableButton(BOOL bEnable = TRUE);

private:
	enum BtnState {BTN_NORMAL = 0, BTN_HOVER, BTN_DOWN, BTN_DISABLED};

private:
	VOID RefreashButtonFace();

public:
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnMouseEnter(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnLButtonDbClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);

public:
	CXButton(void);

private:
	BOOL m_bMouseIn;
	BOOL m_bMouseDown;
	BOOL m_bDisabled;
};
