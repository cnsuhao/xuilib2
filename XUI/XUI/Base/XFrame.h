#pragma once

#include "../Draw/XDraw.h"

#include "RemoteRef.h"
#include "XFrameMsgMgr.h"
#include "XFrameEvent.h"
#include "XFrameXMLFactory.h"

#include <set>

#include <mmsystem.h>

#define INVALID_FRAME_INDEX ((UINT)-1)

class CXFrame :
	public IXFrameEventListener,
	SUPPORT_REMOTE_REFERENCE
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXFrame, XFrame)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXFrame@@0VCXFrameXMLRuntime_CXFrame@1@A")

	BEGIN_FRAME_EVENT_MAP(CXFrame)
		FRAME_EVENT_FRAME_HANDLER(EVENT_FRAME_RECT_CHANGED, m_pFrameParent, OnParentRectChanged)
		FRAME_EVENT_HANDLER(EVENT_FRAME_RECT_CHANGED, OnChildRectChanged)
		FRAME_EVENT_HANDLER(EVENT_FRAME_MARGIN_CHANGED, OnChildMarginChanged)
		FRAME_EVENT_HANDLER(EVENT_FRAME_HOLDPLACE_STATE_CHANGED, OnChildHoldPlaceStateChanged)
	END_FRAME_EVENT_MAP()

	BEGIN_FRAME_MSG_MAP(CXFrame)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		FRAME_MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		FRAME_MESSAGE_HANDLER(WM_X_MOUSEENTER, OnMouseEnter)
	END_FRAME_MSG_MAP()

public:
	enum WIDTH_MODE	
		{WIDTH_MODE_NORMAL = 0, WIDTH_MODE_REACH_PARENT, WIDTH_MODE_WRAP_CONTENT, WIDTH_MODE_ADAPT_BACKGROUND, 
		WIDTH_MODE_NOT_CHANGE};
	enum HEIGHT_MODE 
		{HEIGHT_MODE_NORMAL = 0, HEIGHT_MODE_REACH_PARENT, HEIGHT_MODE_WRAP_CONTENT, HEIGHT_MODE_ADAPT_BACKGROUND, 
		HEIGHT_MODE_NOT_CHANGE};

public:
	virtual BOOL ConfigFrameByXML(X_XML_NODE_TYPE xml);
	virtual BOOL HandleXMLChildNodes(X_XML_NODE_TYPE xml);

public:
	BOOL SetName(LPCTSTR pName);
	CXFrame * GetFrameByName(LPCTSTR pName);
	BOOL GetFrameByName(LPCTSTR pName, std::vector<CXFrame *> *pvFrames);

public:
	CXFrame(void);
	// We can't call the virtual Destroy in the C++ destructor, so, we MUST destroy a frame before destroy the frame object. 
	virtual ~CXFrame(void);

public:
	VOID OnParentRectChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	VOID OnChildRectChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	VOID OnChildMarginChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	VOID OnChildHoldPlaceStateChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled );

public:
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnMouseEnter(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);

public:
	BOOL Create(CXFrame * pFrameParent, const CRect & rcRect = CRect(0, 0, 0, 0), BOOL bVisible = FALSE,
		WIDTH_MODE aWidthMode = WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE aHeightMode = HEIGHT_MODE_NOT_CHANGE);
	virtual VOID Destroy();
	BOOL IsFrameActive();

	UINT GetFrameCount();
	CXFrame * GetFrameByIndex(UINT nIndex);
	UINT GetFrameIndex(CXFrame * pFrame);
	virtual BOOL InsertFrame(CXFrame * pFrame, UINT nIndex);
	BOOL AddFrame(CXFrame * pFrame);
	virtual CXFrame * RemoveFrame(UINT nIndex);
	BOOL RemoveFrame(CXFrame * pFrame);

public:
	BOOL SetParent(CXFrame * pFrameParent);
	CXFrame * GetParent();

	virtual BOOL SetWidthHeightMode(WIDTH_MODE aWidthMode, HEIGHT_MODE aHeightMode);
	WIDTH_MODE GetWidthMode();
	HEIGHT_MODE GetHeightMode();
	virtual BOOL SetRect(const CRect &rcFrame);
	BOOL Move(const CPoint &pt);
	CRect GetRect();
	virtual INT GetVCenter();

	CRect GetMargin();
	BOOL SetMargin(const CRect &rect);

	CXFrame * GetTopFrameFromPoint(const CPoint &pt);

public:
	virtual BOOL SetVisible(BOOL bVisible);
	BOOL IsVisible();
	BOOL SetHoldPlace(BOOL bHoldPlace);
	BOOL IsHoldPlace();

	BOOL GetFocus();
	BOOL KillFocus();

public:
	virtual IXImage * SetBackground(IXImage * pDrawBackground);
	IXImage * GetBackground();

public:
	BOOL SetTouchable(BOOL b);
	IXImage * SetMouseOverLayer(IXImage *pLayer);
	IXImage * SetMouseDownLayer(IXImage *pLayer);
	BOOL SetSelectable(BOOL b);
	BOOL SetSelectWhenMouseClick(BOOL b);
	BOOL SetUnselectWhenMouseClick(BOOL b);
	BOOL SetSelectedState(BOOL b);
	IXImage * SetSelectedLayer(IXImage *pLayer);

public:
	virtual BOOL InvalidateRect(const CRect & rect);
	BOOL InvalidateRect();
	virtual BOOL Update();

public:
	virtual	HWND GetHWND();
	virtual CXFrameMsgMgr *GetFrameMsgMgr();

public:
	virtual BOOL PaintUI(HDC hDC, const CRect &rect);
	virtual BOOL PaintBackground(HDC hDC, const CRect &rect);
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual HDC GetDC();
	virtual BOOL ReleaseDC(HDC dc);

public:
	CPoint ParentToChild(const CPoint &pt);
	CRect ParentToChild(const CRect &rc);
	CPoint ChildToParent(const CPoint &pt);
	CRect ChildToParent(const CRect &rc);

	CPoint OtherFrameToThisFrame(CXFrame * pFrameOther, const CPoint &pt);
	CRect OtherFrameToThisFrame(CXFrame * pFrameOther, const CRect &rect);
	
	CRect FrameToWindow(const CRect &rc);
	virtual CPoint FrameToWindow(const CPoint &pt);
	CRect FrameToScreen(const CRect &rc);
	virtual CPoint FrameToScreen(const CPoint &pt);
	CRect ScreenToFrame(const CRect &rc);
	virtual CPoint ScreenToFrame(const CPoint &pt);

public:
	BOOL PostFrameMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
	UINT SetTimer(UINT uElapse);
	BOOL KillTimer(UINT nID);

public:
	BOOL AddEventListener(IXFrameEventListener *pListener);
	BOOL RemoveEventListener(IXFrameEventListener *pListener);
	VOID ThrowEvent(UINT uEvent, WPARAM wParam, LPARAM lParam);

public:
	virtual BOOL NeedPrepareMessageForThisFrame();

public:
	BOOL SetToolTip(CString strToolTip);
	BOOL ToolTipInitial(CPoint pt);
	BOOL ToolTipDestroy();
	BOOL ToolTipInitialEx(CPoint pt);
	BOOL ToolTipDestroyEx();

protected:
	virtual VOID OnDetachedFromParent();
	virtual VOID OnAttachedToParent(CXFrame *pParent);

	VOID RefreashFrameRect();
	virtual INT CalculateAdaptBackgroundWidth();
	virtual INT CalculateAdaptBackgroundHeight();
	virtual INT CalculateWrapContentWidth();
	virtual INT CalculateWrapContentHeight();

	virtual VOID ChangeFrameRect(const CRect & rcNewFrameRect);

private:
	VOID CalculateFrameRect(CRect *pRect);
	BOOL FillMouseOverLayer();
	BOOL FillMouseDownLayer();
	BOOL FillSelectedLayer();

private:
	CString m_strFrameName;

	std::vector<CXFrame *> m_vecFrameChild;
	CXFrame * m_pFrameParent;
	IXImage * m_pDrawBackground;

	BOOL m_bTouchable;
	IXImage * m_pMouseOverLayer;
	IXImage * m_pMouseDownLayer;
	BOOL m_bSelectable;
	BOOL m_bSelectWhenMouseCilck;
	BOOL m_bUnselectWhenMouseCilck;
	BOOL m_bSelectedState;
	IXImage * m_pSelectedLayer;

	BOOL m_bMouseOver;
	BOOL m_bMouseDown;

	CRect m_rcFrame;
	CRect m_rcMargin;
	BOOL m_bVisible;
	BOOL m_bHoldPlace;

	WIDTH_MODE m_WidthMode;
	HEIGHT_MODE m_HeightMode;

	BOOL m_bIsFrameAlive;

	CToolTipCtrl m_ToolTip;
	CString m_strToolTip;
	HWND m_hToolTip;

private:
	std::map<CRemoteRef<IXFrameEventListener>, DWORD> /* listener -> add count. */ m_mapEventListener;

private:
	static DWORD RegisterInstanceID();
	static BOOL UnregisterInstanceID(DWORD dwInstanceID);
	DWORD m_dwInstanceID;

public:
	static BOOL IsInstanceActive(DWORD dwInstanceID);
	DWORD GetInstanceID();	

private:
	static std::vector<BOOL> s_bActiveInstances;
};
