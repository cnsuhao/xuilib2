#pragma once
#include "..\Base\XFrame.h"


class CXAnimation :
	public CXFrame
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXAnimation, XAnimation)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXAnimation@@0VCXFrameXMLRuntime_CXAnimation@1@A")

	BEGIN_FRAME_MSG_MAP(CXAnimation)
		CHAIN_FRAME_MSG_MAP(CXFrame)
		FRAME_MESSAGE_HANDLER(WM_TIMER, OnTimer)
	END_FRAME_MSG_MAP()

public:
	BOOL Create(CXFrame * pFrameParent, IXImage *pFrames, UINT nFrameCount, UINT nSwitchFrameInterval,
		const CRect & rcRect = CRect(0, 0, 0, 0), BOOL bVisible = FALSE,
		WIDTH_MODE aWidthMode = WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE aHeightMode = HEIGHT_MODE_NOT_CHANGE);

public:
	BOOL SetFrames(IXImage *pFrames, UINT nFrameCount);
	BOOL SetFrameSwitchInterval(UINT nSwitchFrameInterval);

public:
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual VOID Destroy();

protected:
	virtual INT CalculateWrapContentWidth();
	virtual INT CalculateWrapContentHeight();
	virtual VOID ChangeFrameRect(const CRect & rcNewFrameRect);

public:
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);

public:
	CXAnimation(void);

private:
	IXImage * m_pFrames;
	UINT m_nCurrentFrame;
	UINT m_nFrameCount;
	UINT m_nSwitchInterval;
	UINT m_nTimerID;
};
