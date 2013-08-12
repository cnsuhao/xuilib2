#pragma once

#include "XWindow.h"

#define X_END_DIALOG_OK			0
#define X_END_DIALOG_CANCLE		1

#define X_END_DIALOG_YES		0
#define X_END_DIALOG_NO			1
#define X_END_DIALOG_IGNORE		2

#define X_END_DIALOG_ERROR		-1

class CXDialog :
	public CXWindow
{
public:
	INT DoModel(CXWindow *pParent, CRect rcWindow = CRect(0, 0, 0, 0), BOOL bNeedCenterWindow = TRUE, BOOL bVisible = TRUE,
		WIDTH_MODE aWidthMode = WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE aHeightMode = HEIGHT_MODE_NOT_CHANGE);
	BOOL EndDialog(INT nDetail);

public:
	virtual BOOL InitDialog() = 0;

public:
	virtual VOID Destroy();

private:
	BOOL MessageLoop();

public:
	virtual LPCTSTR GetClassName(){return _T("XDialog");}

public:
	CXDialog(void);

private:
	INT m_nEndDialogDetail;
	BOOL m_bDialogEnd;
};
