#pragma once
#include "..\Base\XFrame.h"

class CXCaret :
	public CXFrame
{
BEGIN_FRAME_MSG_MAP(CXCaret)
	CHAIN_FRAME_MSG_MAP(CXFrame)
	FRAME_MESSAGE_HANDLER(WM_TIMER, OnTimer)
END_FRAME_MSG_MAP()

public:
	BOOL Create(CXFrame * pFrameParent, const CRect & rcRect = CRect(0, 0, 0, 0), BOOL bVisible = FALSE,
		UINT nBlinkTime = 500,
		WIDTH_MODE aWidthMode = WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE aHeightMode = HEIGHT_MODE_NOT_CHANGE);
	// Conforms to the windows API CreateCaret.
	// See http://msdn.microsoft.com/zh-cn/library/windows/desktop/ms648399(v=vs.85).aspx
	BOOL SetCaretShape(HBITMAP hBitmap, int nWidth, int nHeight);
	BOOL SetCaretBlinkTime(UINT uMSeconds);

public:
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);

public:
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual VOID Destroy();

protected:
	virtual VOID ChangeFrameRect(const CRect & rcNewFrameRect);

public:
	CXCaret(void);

private:
	HBITMAP m_hBitmap;
	BYTE *m_pBitmapBits;
	INT m_nBitmapWidth;
	INT m_nBitmapHeight;

	UINT m_nBlinkTimer;
	BOOL m_bBlinkShow;
};
