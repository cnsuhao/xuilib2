#include "StdAfx.h"
#include "XFrameMsgMgr.h"
#include <deque>

#include "XFrame.h"


#define HANDLE_MESSAGE_NORMAL(msg, handled)														\
	if (uMsg == msg)																			\
	{																							\
		bHandled = handled;																		\
		LRESULT lResult = 0;																	\
		CXFrame *pTarget = GetMessageTarget(uMsg, wParam, lParam);								\
		if (!pTarget)																			\
			return 0;																			\
		bHandled = bHandled || DispatchFrameMsg(uMsg, wParam, lParam, pTarget, lResult);		\
		return lResult;																			\
	}

#define HANDLE_MESSAGE_SPCIAL(msg, proc, handled)												\
	if (uMsg == msg)																			\
	{																							\
		bHandled = 0;																			\
		LRESULT lResult = 0;																	\
		BOOL bContinueFindHandler = proc(uMsg, wParam, lParam, lResult, bHandled);				\
		bHandled = bHandled || handled;															\
		if (!bContinueFindHandler || bHandled)													\
			return lResult;																		\
	}

CXFrameMsgMgr::CXFrameMsgMgr(CXFrame *pFrame)
{
	if (pFrame)
		m_rFrameBase = pFrame;
}

CXFrameMsgMgr::~CXFrameMsgMgr(void)
{
}



LRESULT CXFrameMsgMgr::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{	
	HANDLE_MESSAGE_SPCIAL(WM_CREATE, HandleCreateMsg, FALSE)

	HANDLE_MESSAGE_SPCIAL(WM_SETCURSOR, HandleSetCursorMessage, FALSE)

	HANDLE_MESSAGE_SPCIAL(WM_MOUSEMOVE, HandleHoverLeaveMessage, FALSE)
	HANDLE_MESSAGE_SPCIAL(WM_LBUTTONDOWN, HandleHoverLeaveMessage, FALSE)
	HANDLE_MESSAGE_SPCIAL(WM_MOUSELEAVE, HandleHoverLeaveMessage, FALSE)

	HANDLE_MESSAGE_NORMAL(WM_LBUTTONDOWN, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_LBUTTONUP, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_RBUTTONDOWN, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_RBUTTONUP, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_LBUTTONDBLCLK, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_MOUSEMOVE, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_MOUSEWHEEL, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_KEYDOWN, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_KEYUP, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_CHAR, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_SETFOCUS, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_KILLFOCUS, FALSE)

	HANDLE_MESSAGE_NORMAL(WM_IME_KEYUP, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_IME_KEYDOWN, FALSE)
 	HANDLE_MESSAGE_NORMAL(WM_IME_CHAR, FALSE)

	HANDLE_MESSAGE_NORMAL(WM_IME_STARTCOMPOSITION, FALSE)
 	HANDLE_MESSAGE_NORMAL(WM_IME_COMPOSITION, FALSE)
 	HANDLE_MESSAGE_NORMAL(WM_IME_COMPOSITIONFULL, FALSE)
  	HANDLE_MESSAGE_NORMAL(WM_IME_ENDCOMPOSITION, FALSE)

	HANDLE_MESSAGE_NORMAL(WM_IME_NOTIFY, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_IME_REQUEST, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_IME_SELECT, FALSE)
	HANDLE_MESSAGE_NORMAL(WM_IME_SETCONTEXT, FALSE)

	return 0;
}

BOOL CXFrameMsgMgr::DispatchFrameMsg( UINT uMsg, WPARAM wParam, LPARAM lParam, CXFrame *pFrame, LRESULT &lResult)
{
	while (pFrame && pFrame->IsFrameActive())
	{
		BOOL bCancleBubble = FALSE;
		DWORD dwFrameInstanceID = pFrame->GetInstanceID();

		WPARAM wMsgParam = wParam;
		LPARAM lMsgParam = lParam;
		
		PrepareMessageForFrame(uMsg, &wMsgParam, &lMsgParam, pFrame);

		if (pFrame->ProcessFrameMessage(uMsg, wMsgParam, lMsgParam, lResult, bCancleBubble))
		{
			return TRUE;
		}

		if (bCancleBubble)
			break;

		if (!CXFrame::IsInstanceActive(dwFrameInstanceID))
			break;

		pFrame = pFrame->GetParent();
	}
	
	return FALSE;
}

CXFrame* CXFrameMsgMgr::GetMessageTarget( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (!m_rFrameBase)
		return NULL;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MOUSEMOVE:
		if (m_rFrameCaptureMouse)
			return m_rFrameCaptureMouse;
		else
			return m_rFrameBase->GetTopFrameFromPoint(CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		break;

	case WM_SETCURSOR:
		if (m_rFrameCaptureMouse)
			return m_rFrameCaptureMouse;
		else
		{
			CPoint pt(0, 0);
			::GetCursorPos(&pt);
			::ScreenToClient(m_rFrameBase->GetHWND(), &pt);
			return m_rFrameBase->GetTopFrameFromPoint(pt);
		}
		break;

	default:
		break;
	}

	return m_rFrameFocus;
}

VOID CXFrameMsgMgr::PrepareMessageForFrame( UINT uMsg, WPARAM *pwParam, LPARAM *plParam, CXFrame *pFrame )
{
	if (!pFrame)
		return;

	if (!pFrame->NeedPrepareMessageForThisFrame())
		return;

	CPoint ptFrame(0, 0);
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MOUSEMOVE:
		ptFrame = 
			pFrame->OtherFrameToThisFrame(m_rFrameBase, CPoint(GET_X_LPARAM(*plParam), GET_Y_LPARAM(*plParam)));
		*plParam = MAKELPARAM(ptFrame.x, ptFrame.y);
		break;

	default:
		break;
	}
}

BOOL CXFrameMsgMgr::GetFocus(CXFrame *pFrame)
{
	if (!pFrame)
		return FALSE;

	if (m_rFrameFocus == pFrame)
		return TRUE;

	if (m_rFrameFocus)
		KillFocus(m_rFrameFocus);

	if (m_rFrameBase)
	{
		m_rFrameFocus = pFrame;

		LRESULT lUnused = 0;

		if (::GetFocus() != m_rFrameBase->GetHWND())
			::SetFocus(m_rFrameBase->GetHWND());
		else
			DispatchFrameMsg(WM_SETFOCUS, 0, 0, pFrame, lUnused);
	}

	return TRUE;
}

BOOL CXFrameMsgMgr::KillFocus( CXFrame *pFrame )
{
	if (!pFrame)
		return FALSE;

	if (m_rFrameFocus != pFrame)
		return FALSE;

	m_rFrameFocus = NULL;

	LRESULT lUnused = 0;
	DispatchFrameMsg(WM_KILLFOCUS, 0, 0, pFrame, lUnused);

	return TRUE;
}

BOOL CXFrameMsgMgr::CaptureMouse( CXFrame *pFrame )
{
	if (!pFrame)
		return FALSE;

	if (!m_rFrameBase)
		return FALSE;

	m_rFrameCaptureMouse = pFrame;

	::SetCapture(m_rFrameBase->GetHWND());

	return TRUE;
}


BOOL CXFrameMsgMgr::ReleaseCaptureMouse( CXFrame *pFrame )
{
	if (!pFrame)
		return FALSE;

	if (!m_rFrameBase)
		return FALSE;

	if (m_rFrameCaptureMouse != pFrame)
		return FALSE;

	::ReleaseCapture();

	m_rFrameCaptureMouse = NULL;

	CPoint ptCursor(0 ,0);
	GetCursorPos(&ptCursor);
	CXFrame *pFrameMouseIn = 
		m_rFrameBase->GetTopFrameFromPoint(m_rFrameBase->ScreenToFrame(ptCursor));
	UpdateMouseIn(pFrameMouseIn);

	return TRUE;
}



BOOL CXFrameMsgMgr::HandleHoverLeaveMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, BOOL &bHandled)
{
	BOOL bRtn = TRUE;

	switch(uMsg)
	{
	case WM_MOUSELEAVE:
		UpdateMouseIn(NULL);
		bRtn = FALSE;
		break;

	case WM_LBUTTONDOWN:
		TrackMouseEvent();
		"nobreak";
	case WM_MOUSEMOVE:
		CXFrame *pFrameMouseIn = NULL;
		if (m_rFrameBase)
			pFrameMouseIn = m_rFrameCaptureMouse ? 
			m_rFrameCaptureMouse :
			m_rFrameBase->GetTopFrameFromPoint(CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		UpdateMouseIn(pFrameMouseIn);
		bRtn = TRUE;
		break;
	}

	return bRtn;
}

VOID CXFrameMsgMgr::TrackMouseEvent()
{
	if (!m_rFrameBase)
		return;

	TRACKMOUSEEVENT tme = {};
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = m_rFrameBase->GetHWND();
	tme.dwHoverTime = 1;

	::TrackMouseEvent(&tme);
}

BOOL CXFrameMsgMgr::HandleCreateMsg( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, BOOL &bHandled)
{
//	TrackMouseEvent();
	return FALSE;
}

BOOL CXFrameMsgMgr::HandleSetCursorMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, BOOL &bHandled )
{
	INT nHitTest = LOWORD(lParam);

	if (nHitTest == HTCLIENT)
	{
 		CXFrame *pTarget = GetMessageTarget(uMsg, wParam, lParam);
 		if (pTarget && DispatchFrameMsg(uMsg, wParam, lParam, pTarget, lResult))
			bHandled = TRUE;
	}

	return FALSE;
}

VOID CXFrameMsgMgr::UpdateMouseIn( CXFrame *pFrameMouseIn )
{
	if (!pFrameMouseIn)
		while (m_vrMouseIn.size())
		{
			LRESULT lUnused = 0;
			BOOL bUnused = FALSE;

			if (m_vrMouseIn.back())
				m_vrMouseIn.back()->ProcessFrameMessage(WM_MOUSELEAVE, 0, 0, lUnused, bUnused);
			m_vrMouseIn.pop_back();
		}
	else
	{
		std::deque<CXFrame *> dqCurrentPath;
		while (pFrameMouseIn)
		{
			dqCurrentPath.push_front(pFrameMouseIn);
			pFrameMouseIn = pFrameMouseIn->GetParent();
		}

		UINT i = 0;
		for (; i < dqCurrentPath.size() && i < m_vrMouseIn.size(); i++)
			if (dqCurrentPath[i] != m_vrMouseIn[i])
				break;

		while (i < m_vrMouseIn.size())
		{
			LRESULT lUnused = 0;
			BOOL bUnused = FALSE;

			if (m_vrMouseIn.back())
				m_vrMouseIn.back()->ProcessFrameMessage(WM_MOUSELEAVE, 0, 0, lUnused, bUnused);
			m_vrMouseIn.pop_back();
		}

		while (i < dqCurrentPath.size())
		{
			LRESULT lUnused = 0;
			BOOL bUnused = FALSE;

			if (dqCurrentPath[i])
				dqCurrentPath[i]->ProcessFrameMessage(WM_X_MOUSEENTER, 0, 0, lUnused, bUnused);
			else
			{
				ATLASSERT(NULL);
				break;
			}

			m_vrMouseIn.push_back(dqCurrentPath[i]);

			i++;
		}
	}
}
