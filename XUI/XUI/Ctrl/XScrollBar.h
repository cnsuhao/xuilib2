#pragma once

#include "../Base/XFrame.h"

class CXScrollBar :
	public CXFrame
{
public:
	BEGIN_FRAME_MSG_MAP(CXScrollBar)
		CHAIN_FRAME_MSG_MAP(CXFrame)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		FRAME_MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		FRAME_MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	END_FRAME_MSG_MAP()

public:
	enum ScrollType{SCROLL_H, SCROLL_V};

public:
	BOOL Create(CXFrame * pFrameParent, ScrollType type, LayoutParam * pLayout, VISIBILITY visibility = VISIBILITY_NONE,
		IXImage * pBarBackground = NULL, IXImage * pBarImage = NULL);

public:
	BOOL SetContentLen(INT nLen);
	BOOL SetViewLen(INT nLen);
	BOOL SetScrollPos(INT nPos);
	INT GetScrollPos();
	IXImage * SetBarImage(IXImage *pImage);

public:
	virtual BOOL SetRect(const CRect & rcNewFrameRect);
	virtual BOOL SetVisibility(VISIBILITY visibility);
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual VOID Destroy();
	

public:
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	
public:
	CXScrollBar(void);

private:
	VOID UpdateScrollBar();
	VOID NotifyScrollChange();
	INT AdjustScrollPos(INT nPos);

private:
	ScrollType m_Type;
	IXImage *m_pImageBar;
	INT m_nContentLen;
	INT m_nViewLen;
	INT m_nPos;

private:
	CRect m_rcBar;

private:
	BOOL m_bVisibleState;

private:
	BOOL m_bMouseDown;
	CPoint m_ptMouseDownPt;
	INT m_nMouseDownScrollPos;
};
