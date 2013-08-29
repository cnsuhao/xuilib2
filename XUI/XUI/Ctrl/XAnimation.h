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
		LayoutParam * pLayout, VISIBILITY visibility = VISIBILITY_NONE);

public:
	BOOL SetFrames(IXImage *pFrames, UINT nFrameCount);
	BOOL SetFrameSwitchInterval(UINT nSwitchFrameInterval);

public:
	virtual BOOL SetRect(const CRect & rcNewFrameRect);
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual VOID Destroy();
	virtual BOOL OnMeasureWidth(const MeasureParam & param);
	virtual BOOL OnMeasureHeight(const MeasureParam & param);

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
