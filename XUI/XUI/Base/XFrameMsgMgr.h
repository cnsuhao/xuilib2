#pragma once

#include <vector>

#include "RemoteRef.h"



#define FRAME_MSG_MGR_HANDLE_MSG(obj)										\
	if (TRUE)																\
	{																		\
		bHandled = FALSE;													\
		lResult = obj.HandleMessage(uMsg, wParam, lParam, bHandled);		\
		if(bHandled)														\
			return TRUE;													\
	}

#define BEGIN_FRAME_MSG_MAP(theClass)																						\
public:																														\
	virtual BOOL ProcessFrameMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, BOOL& bCancelBubble)		\
	{																														\
		BOOL bHandled = FALSE;																								\
		(uMsg);																												\
		(wParam);																											\
		(lParam);																											\
		(lResult);																											\
		(bHandled);																											\
		(bCancelBubble);

#define FRAME_MESSAGE_HANDLER(msg, func)																					\
		if(uMsg == msg)																										\
		{																													\
			bHandled = FALSE;																								\
			lResult = func(uMsg, wParam, lParam, bHandled, bCancelBubble);													\
			if(bHandled)																									\
				return TRUE;																								\
		}

#define FRAME_ALL_MESSAGE_HANDLER(func)																						\
		{																													\
			bHandled = FALSE;																								\
			lResult = func(uMsg, wParam, lParam, bHandled, bCancelBubble);													\
			if(bHandled)																									\
				return TRUE;																								\
		}

#define CHAIN_FRAME_MSG_MAP(theChainClass)																					\
		{																													\
			if(theChainClass::ProcessFrameMessage(uMsg, wParam, lParam, lResult, bCancelBubble))							\
				return TRUE;																								\
		}


#define END_FRAME_MSG_MAP()																									\
		return FALSE;																										\
	}

class CXFrame;

typedef struct tagPostFrameMsgInfo
{
	UINT uMsg;
	WPARAM wParam;
	LPARAM lParam;
	CRemoteRef<CXFrame> rFrame;
}PostFrameMsgInfo;


class CXFrameMsgMgr
{
public:
	CXFrameMsgMgr(CXFrame *pFrame);
	~CXFrameMsgMgr(void);

public:
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	BOOL GetFocus(CXFrame *pFrame);
	BOOL KillFocus(CXFrame *pFrame);

	BOOL CaptureMouse(CXFrame *pFrame);
	BOOL ReleaseCaptureMouse(CXFrame *pFrame);

private:
	BOOL DispatchFrameMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, CXFrame *pFrame, LRESULT& lResult);
	
private:
	CRemoteRef<CXFrame> m_rFrameBase;
	CRemoteRef<CXFrame> m_rFrameFocus;
	CRemoteRef<CXFrame> m_rFrameCaptureMouse;
	std::vector<CRemoteRef<CXFrame>> m_vrMouseIn;

private:
	BOOL HandleHoverLeaveMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, BOOL &bHandled);
	BOOL HandleCreateMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, BOOL &bHandled);
	BOOL HandleSetCursorMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, BOOL &bHandled);

private:
	CXFrame* GetMessageTarget(UINT uMsg, WPARAM wParam, LPARAM lParam);
	VOID PrepareMessageForFrame(UINT uMsg, WPARAM *pwParam, LPARAM *plParam, CXFrame *pFrame);
	VOID TrackMouseEvent();
	VOID UpdateMouseIn(CXFrame *pFrameMouseIn);
};