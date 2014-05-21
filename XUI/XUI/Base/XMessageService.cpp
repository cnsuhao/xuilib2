#include "StdAfx.h"
#include "XMessageService.h"

#include "XFrame.h"



CXMessageService::CXMessageService( void )
	: m_bPostFrameMsgScheduled(FALSE)
{
	Create(HWND_MESSAGE, CRect(0, 0, 0, 0));
	ATLASSERT(m_hWnd);
}

CXMessageService::~CXMessageService( void )
{
	DestroyWindow();
}

BOOL CXMessageService::PostFrameMsg( CXFrame *pFrame, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (!pFrame)
		return FALSE;

	if (!m_hWnd)
	{
		ATLASSERT(m_hWnd);
		return FALSE;
	}

	PostFrameMsgInfo info;
	info.uMsg = uMsg;
	info.wParam = wParam;
	info.lParam = lParam;
	info.rFrame = pFrame;

	m_vPostFrameMsg.push_back(info);
	m_setPendingMsg.insert(uMsg);

	if (!m_bPostFrameMsgScheduled)
	{
		PostMessage(WM_X_POST_FRAME_MSG, 0, 0);
		m_bPostFrameMsgScheduled = TRUE;
	}

	return TRUE;
}

LRESULT CXMessageService::OnPostFrameMsg( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!m_bPostFrameMsgScheduled)
		return 0;

	m_bPostFrameMsgScheduled = FALSE;


	std::vector<PostFrameMsgInfo>::size_type szLen = m_vPostFrameMsg.size();

	for (std::vector<PostFrameMsgInfo>::size_type i = 0;
		i < szLen; i++)
	{
		const PostFrameMsgInfo & info = 
			m_vPostFrameMsg[i];

		LRESULT lUnused = 0;
		BOOL bUnused = 0;

		if (info.rFrame)
			info.rFrame->ProcessFrameMessage(info.uMsg, info.wParam, info.lParam, lUnused, bUnused);
	}

	m_vPostFrameMsg.erase(m_vPostFrameMsg.begin(), m_vPostFrameMsg.begin() + szLen);
	m_setPendingMsg.clear();
	for (std::vector<PostFrameMsgInfo>::size_type i = 0;
		i < m_vPostFrameMsg.size(); i++)
		m_setPendingMsg.insert(m_vPostFrameMsg[i].uMsg);

	return 0;
}

UINT CXMessageService::SetTimer( CXFrame *pFrame, UINT uElapse )
{
	if (!pFrame)
		return 0;

	if (!m_hWnd)
	{
		ATLASSERT(m_hWnd);
		return FALSE;
	}
	

	std::vector<CRemoteRef<CXFrame>>::size_type i = 0;
	for ( ;i < m_vrTimerFrames.size(); i++)
		if (!m_vrTimerFrames[i])
			break;

	if (__super::SetTimer(i + 1, uElapse, NULL))
	{
		if (i < m_vrTimerFrames.size())
			m_vrTimerFrames[i] = pFrame;
		else
			m_vrTimerFrames.push_back(pFrame);

		return i + 1;
	}

	return 0;
}

BOOL CXMessageService::KillTimer( UINT nTimerID )
{
	if (!nTimerID)
		return FALSE;

	if (!m_hWnd)
	{
		ATLASSERT(m_hWnd);
		return FALSE;
	}

	__super::KillTimer(nTimerID);

	UINT nFrameIndex = nTimerID - 1;

	ATLASSERT(nFrameIndex < m_vrTimerFrames.size());
	if (nFrameIndex < m_vrTimerFrames.size())
	{
		m_vrTimerFrames[nFrameIndex] = NULL;

		if (nFrameIndex == m_vrTimerFrames.size() - 1)
			do 
			{
				m_vrTimerFrames.pop_back();
			} while (m_vrTimerFrames.size() && !m_vrTimerFrames.back());	

			return TRUE;
	}

	return FALSE;
}

LRESULT CXMessageService::OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	WPARAM nFrameIndex = wParam - 1;

	if (nFrameIndex >= m_vrTimerFrames.size())
	{
		if (m_hWnd)
			__super::KillTimer(wParam);
		else
			ATLASSERT(NULL);

		return 0;
	}

	if (m_vrTimerFrames[nFrameIndex])
	{
		LRESULT lUnused = 0;
		BOOL bUnused = 0;
		m_vrTimerFrames[nFrameIndex]->ProcessFrameMessage(WM_TIMER, wParam, 0, lUnused, bUnused);
	}
	else
	{
		if (m_hWnd)
			__super::KillTimer(wParam);
		else
			ATLASSERT(NULL);
	}

	return 0;
}

CXMessageService & CXMessageService::Instance()
{
	static CXMessageService instance;
	return instance;
}

BOOL CXMessageService::HasPendingMsg( UINT uMsg )
{
	return m_setPendingMsg.find(uMsg) != m_setPendingMsg.end();
}
