#pragma once

#include "../Base/XWindow.h"

#define FRAME_MSG_UPDATE_POSITION (0x10)

template<class TCtrl>
class CXSysControl
	: public CXFrame, 
	public CWindowImpl<CXSysControl<TCtrl>, TCtrl>
{
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info_cxsyscontrol@@3VCXFrameXMLRuntimeInfo_CXSysControl@@A")

	BEGIN_FRAME_EVENT_MAP(CXSysControl<TCtrl>)
		CHAIN_FRAME_EVENT_MAP(CXFrame)
		FRAME_EVENT_HANDLER(EVENT_FRAME_RECT_CHANGED, OnAncestorRectChanged)
		FRAME_EVENT_HANDLER(EVENT_FRAME_SHOWHIDE_CHANGED, OnAncestorShowHide)
		FRAME_EVENT_HANDLER(EVENT_WND_MOVED, OnAncestorRectChanged)
		FRAME_EVENT_HANDLER(EVENT_WND_ENABLED, OnParentWndEnabled)
		FRAME_EVENT_HANDLER(EVENT_FRAME_ATTACHED_TO_PARENT, OnOneAncestorAttachedToParent)
		FRAME_EVENT_HANDLER(EVENT_FRAME_DETACHED_FROM_PARENT, OnOneAncestorDetachedFromaParent)
	END_FRAME_EVENT_MAP()

	BEGIN_FRAME_MSG_MAP(CXSysControl<TCtrl>)
		CHAIN_FRAME_MSG_MAP(CXFrame)
		FRAME_MESSAGE_HANDLER(FRAME_MSG_UPDATE_POSITION, OnUpdatePositon)
	END_FRAME_MSG_MAP()

	BEGIN_MSG_MAP(CXSysControl<TCtrl>)
	END_MSG_MAP()

public:
	BOOL Create(CXFrame * pFrameParent, const CRect & rcRect = CRect(0, 0, 0, 0),
		BOOL bVisible = FALSE, WIDTH_MODE aWidthMode = WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE aHeightMode = HEIGHT_MODE_NOT_CHANGE);

public:
	virtual VOID Destroy();

	virtual CXFrame * RemoveFrame(UINT nIndex);
	virtual BOOL InsertFrame(CXFrame * pFrame, UINT nIndex);

	virtual BOOL SetWidthHeightMode(WIDTH_MODE aWidthMode, HEIGHT_MODE aHeightMode);
	
	virtual BOOL SetVisible(BOOL bVisible);
	
	virtual IXImage * SetBackground(IXImage * pDrawBackground);
	
	virtual BOOL InvalidateRect(const CRect & rect);
	
	virtual BOOL PaintUI(HDC hDC, const CRect &rect);

protected:
	virtual VOID OnDetachedFromParent();
	virtual VOID OnAttachedToParent(CXFrame *pParent);
	virtual VOID ChangeFrameRect(const CRect & rcNewFrameRect);
	

private:
	BOOL UpdateSysControlPosition();
	BOOL UpdateShowState();

public:
	VOID OnAncestorRectChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnAncestorShowHide(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnParentWndEnabled(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnOneAncestorAttachedToParent(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnOneAncestorDetachedFromaParent(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	LRESULT OnUpdatePositon(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);

public:
	CXSysControl(void);

private:
	BOOL IsAncester(CXFrame *pFrame);

private:
	BOOL m_bNeedRestoreWhenParentEnable;
	BOOL m_bUpdatePositionScheduled;
};


#include "XSysControl.inl"
