#pragma once

#include <vector>
#include <map>

#include "RemoteRef.h"

#define WM_X_POST_FRAME_MSG		(WM_USER + 0x0101)

class CXFrame;
class CXMessageService 
	: public CWindowImpl<CXMessageService, CWindow, CWinTraits<0, 0>>
{
	BEGIN_MSG_MAP(CXMessageService)
		MESSAGE_HANDLER(WM_X_POST_FRAME_MSG, OnPostFrameMsg)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
	END_MSG_MAP()

	DECLARE_WND_CLASS(_T("XMessageService"))

public:
	static CXMessageService & Instance();

public:
	BOOL PostFrameMsg(CXFrame *pFrame, UINT uMsg, WPARAM wParam, LPARAM lParam);
	/* return value of 0 indicates a failure, and timer IDs are always greater then zero. */
	UINT SetTimer(CXFrame *pFrame, UINT uElapse); 
	BOOL KillTimer(UINT nTimerID);

public:
	LRESULT OnPostFrameMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


private:
	CXMessageService(void);
	~CXMessageService(void);

private:
	typedef struct tagPostFrameMsgInfo
	{
		UINT uMsg;
		WPARAM wParam;
		LPARAM lParam;
		CRemoteRef<CXFrame> rFrame;
	}PostFrameMsgInfo;

public:
	std::vector<PostFrameMsgInfo> m_vPostFrameMsg;
	BOOL m_bPostFrameMsgScheduled;
	std::vector<CRemoteRef<CXFrame>> m_vrTimerFrames;
};
