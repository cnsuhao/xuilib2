#include "StdAfx.h"
#include "XDialog.h"

#define DEFAULT_DIALOG_WIDTH		470
#define DEFAULT_DIALOG_HEIGHT		220

CXDialog::CXDialog(void) 
	: m_nEndDialogDetail(X_END_DIALOG_ERROR),
	m_bDialogEnd(FALSE)
{
}

INT CXDialog::DoModel( CXWindow *pParent, CRect rcWindow/* = CRect(0, 0, 0, 0)*/, BOOL bNeedCenterWindow/* = TRUE*/, BOOL bVisible/* = TRUE*/,
					  WIDTH_MODE aWidthMode/* = WIDTH_MODE_NOT_CHANGE*/, HEIGHT_MODE aHeightMode/* = HEIGHT_MODE_NOT_CHANGE*/)
{
	if (!pParent)
	{
		ATLASSERT(!_T("No parent specified for the dialog. "));
		return X_END_DIALOG_ERROR;
	}

	if (rcWindow.IsRectEmpty())
	{
		rcWindow.right = rcWindow.left + DEFAULT_DIALOG_WIDTH;
		rcWindow.bottom = rcWindow.top + DEFAULT_DIALOG_HEIGHT;
	}

	Create(pParent->GetHWND(), rcWindow, aWidthMode, aHeightMode, NULL, bVisible ? WS_VISIBLE : 0);
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
