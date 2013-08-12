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
	BOOL Create(CXFrame * pFrameParent, ScrollType type, const CRect & rcRect = CRect(0, 0, 0, 0), BOOL bVisible = FALSE,
		IXImage * pBarBackground = NULL, IXImage * pBarImage = NULL,
		WIDTH_MODE aWidthMode = WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE aHeightMode = HEIGHT_MODE_NOT_CHANGE);

public:
	BOOL SetFrameLen(INT nLen);
	BOOL SetViewLen(INT nLen);
	BOOL SetScrollPos(INT nPos);
	INT GetScrollPos();
	IXImage * SetBarImage(IXImage *pImage);

public:
	virtual BOOL SetVisible(BOOL bVisible);
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual VOID Destroy();

protected:
	virtual VOID ChangeFrameRect(const CRect & rcNewFrameRect);

public:
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	
public:
	CXScrollBar(void);

private:
	VOID UpdateScrollBar();

private:
	ScrollType m_Type;
	IXImage *m_pImageBar;
	INT m_nFrameLen;
	INT m_nViewLen;
	INT m_nPos;

private:
	CRect m_rcBar;

private:
	BOOL m_bVisibleState;

private:
	BOOL m_bMouseDown;
	CPoint m_ptLastMousePt;
};
