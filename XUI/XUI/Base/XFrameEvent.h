#pragma once

#define BEGIN_FRAME_EVENT_MAP(theClass)																						\
public:																														\
	virtual BOOL ProcessFrameEvent(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam)							\
	{																														\
		BOOL bHandled = FALSE;																								\
		(uEvent);																											\
		(wParam);																											\
		(lParam);																											\
		(bHandled);

#define FRAME_EVENT_HANDLER(event, func)																					\
		if(uEvent == event)																									\
		{																													\
			bHandled = FALSE;																								\
			func(pSrcFrame, uEvent, wParam, lParam, bHandled);																\
			if(bHandled)																									\
				return TRUE;																								\
		}

#define FRAME_EVENT_FRAME_HANDLER(event, frame, func)																		\
		if(uEvent == event && pSrcFrame == frame)																			\
		{																													\
			bHandled = FALSE;																								\
			func(pSrcFrame, uEvent, wParam, lParam, bHandled);																\
			if(bHandled)																									\
				return TRUE;																								\
		}

#define CHAIN_FRAME_EVENT_MAP(theChainClass)																				\
		{																													\
			if(theChainClass::ProcessFrameEvent(pSrcFrame, uEvent, wParam, lParam))											\
				return TRUE;																								\
		}


#define END_FRAME_EVENT_MAP()																								\
		return FALSE;																										\
	}


class IXFrameEventListener :
	SUPPORT_REMOTE_REFERENCE
{

public:
	virtual BOOL ProcessFrameEvent(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam) = 0;

};