#pragma once

#include "XFrame.h"

#define WM_X_UPDATE (WM_USER + 0x100)
#define WM_X_LAYOUT (WM_USER + 0x101)

class CXWindow : public CXFrame, public CWindowImpl<CXWindow>
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXWindow, XWindow)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXWindow@@0VCXFrameXMLRuntime_CXWindow@1@A")

public:
	virtual BOOL ConfigFrameByXML(X_XML_NODE_TYPE xml);

public:
	CXWindow(void);

public:
	/// 支持子类自定义类名
	virtual LPCTSTR GetClassName(){return _T("XWindow");}
	ATL::CWndClassInfo GetWndClassInfo();
	HWND Create(HWND hWndParent,  LayoutParam * pLayout,
		LPCTSTR szWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, _U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL);

	BEGIN_MSG_MAP(CXWindow)
		FRAME_MSG_MGR_HANDLE_MSG(m_FrameMsgMgr)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgrnd)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDbClk)
		MESSAGE_HANDLER(WM_MOUSELEAVE,OnMouseLeave)
		MESSAGE_HANDLER(WM_NCHITTEST, OnNCHitTest)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_NCLBUTTONDOWN, OnNCLButtonDown)
		MESSAGE_HANDLER(WM_MOVE, OnMove)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnMinMaxInfo)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_X_UPDATE, OnXUpdate)
		MESSAGE_HANDLER(WM_X_LAYOUT, OnXLayout)
	END_MSG_MAP()

public:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgrnd( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDbClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNCHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNCLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEnable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	LRESULT OnXUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnXLayout(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
public:
	BOOL SetAlpha(BYTE cAlpha);
	BYTE GetAlpha();

public:
	BOOL SetCaptionRect(const CRect &rcCaption);
	BOOL SetCaptionWidthReachWnd(BOOL b);
	BOOL SetCaptionHeightReachWnd(BOOL b);
	CRect GetCaptionRect();
	BOOL SetResizable(BOOL bResizable);
	
public:
	virtual BOOL SetRect(const CRect & rcNewFrameRect);

	virtual HDC GetDC();
	virtual BOOL ReleaseDC(HDC dc, BOOL bUpdate = TRUE);

	virtual BOOL EndUpdateLayoutParam();

	virtual BOOL InvalidateLayout();
	virtual BOOL IsLayouting();

	virtual BOOL InvalidateRect(const CRect & rect);
	virtual BOOL Update();

	virtual BOOL SetVisibility(VISIBILITY visibility);

	virtual CPoint FrameToWindow(const CPoint &pt);
	virtual CPoint FrameToScreen(const CPoint &pt);
	virtual CPoint ScreenToFrame(const CPoint &pt);

	virtual	HWND GetHWND();
	virtual CXFrameMsgMgr *GetFrameMsgMgr();

	virtual VOID Destroy();

private:
	VOID RefreashCaptionRect();

private:
	BOOL UpdateLayeredWindow(HDC dcSrc);

private:
	BOOL RecreateBufferDC(INT nWidth, INT nHeight);
	VOID ReleaseBufferDC();
	
private:
	HDC m_dcBuffer;
	HDC m_dcBufferForDirectDraw;
	HGDIOBJ m_hBufferOldBmp;
	HGDIOBJ m_hBufferForDirectDrawOldBmp;
	std::vector<CRect> m_vRectInvalidated;
	BOOL m_bLayoutScheduled;
	BOOL m_bIsLayouting;
	BOOL m_bUpdateScheduled;

	BYTE m_cAlpha;

private:
	CXFrameMsgMgr m_FrameMsgMgr;

private:
	BOOL m_bResizable;
	CRect m_rcCaption;
	BOOL m_bCaptionWidthReachParent;
	BOOL m_bCaptionHeightReachParnet;
};