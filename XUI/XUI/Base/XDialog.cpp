#include "StdAfx.h"
#include "XDialog.h"

#define DEFAULT_DIALOG_WIDTH		470
#define DEFAULT_DIALOG_HEIGHT		220

CXDialog::CXDialog(void) 
	: m_nEndDialogDetail(X_END_DIALOG_ERROR),
	m_bDialogEnd(FALSE)
{
}

INT CXDialog::DoModel( CXWindow *pParent, LayoutParam * pLayout ,BOOL bNeedCenterWindow/* = TRUE*/, BOOL bVisible/* = TRUE*/)
{
	if (!pLayout) 
	{
		ATLASSERT(!_T("No layout parameter. "));
		return X_END_DIALOG_ERROR;
	}

	if (!pParent)
	{
		ATLASSERT(!_T("No parent specified for the dialog. "));
		return X_END_DIALOG_ERROR;
	}

	if (pLayout->m_nWidth * pLayout->m_nHeight == 0)
	{
		pLayout->m_nWidth = DEFAULT_DIALOG_WIDTH;
		pLayout->m_nHeight =  DEFAULT_DIALOG_HEIGHT;
	}

	Create(pParent->GetHWND(), pLayout, NULL, bVisible ? WS_VISIBLE : 0);
	::EnableWindow(pParent->GetHWND(), FALSE);

	InitDialog();

	if (bNeedCenterWindow)
		CenterWindow();

	m_bDialogEnd = FALSE;
	MessageLoop();

	::EnableWindow(pParent->GetHWND(), TRUE);

	INT nEndDialogDetail = m_nEndDialogDetail;

	Destroy();

	return nEndDialogDetail;
}

BOOL CXDialog::EndDialog( INT nDetail )
{
	m_nEndDialogDetail = nDetail;

	m_bDialogEnd = TRUE;

	return TRUE;
}

BOOL CXDialog::MessageLoop()
{
	while (true)
	{
		MSG msg;
		if (!::GetMessage(&msg, NULL, 0, 0))
		{
			::PostQuitMessage(msg.wParam);
			break;
		}

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);

		if (m_bDialogEnd)
			break;
	}

	return TRUE;
}

VOID CXDialog::Destroy()
{
	m_nEndDialogDetail = X_END_DIALOG_ERROR;
	m_bDialogEnd = FALSE;

	__super::Destroy();
}
