// TestUI.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "TestUI.h"

#include "..\XUI\Draw\XImageGdiPlus.h"

#include "..\XUI\Base\XWindow.h"

#include "..\XUI\Ctrl\XButton.h"
#include "..\XUI\Ctrl\XDock.h"
#include "..\XUI\Ctrl\XScrollBar.h"
#include "..\XUI\Ctrl\XScrollFrame.h"
#include "..\XUI\Ctrl\XSysControl.h"
#include "..\XUI\Draw\XTextGdiPlus.h"
#include "..\XUI\Ctrl\XStatic.h"
#include "..\XUI\Base\XDialog.h"
#include "..\XUI\Ctrl\XAnimation.h"
#include "..\XUI\Ctrl\XEdit.h"
#include "..\XUI\Ctrl\XMultifaceButton.h"
#include "..\XUI\Ctrl\XTree.h"

CAppModule _Module;
 
class CTestDialog :
	public CXDialog
{
	BEGIN_FRAME_EVENT_MAP(CTestDialog)
		FRAME_EVENT_FRAME_HANDLER(EVENT_BUTTON_CLICKED, m_rCloseBtn, OnCloseClicked)
	END_FRAME_EVENT_MAP()

public:
	VOID OnCloseClicked(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		EndDialog(X_END_DIALOG_CANCLE);
	}


public:
	virtual BOOL InitDialog()
	{
		X_XML_DOC_TYPE doc;
		if (CXFrameXMLFactory::LoadXML(SMCRelativePath(_T("Dialog.xml")), &doc))
			ConfigFrameByXML(doc.first_node());

		if (m_rCloseBtn = GetFrameByName(_T("sys_close")))
			m_rCloseBtn->AddEventListener(this);

		return TRUE;
	}

private:
	CRemoteRef<CXFrame> m_rCloseBtn;
};

class CWindowEvent :
	public IXFrameEventListener
{
BEGIN_FRAME_EVENT_MAP(CWindowEvent)
	FRAME_EVENT_FRAME_HANDLER(EVENT_BUTTON_CLICKED, m_rDialogButton, OnDialogButtonClicked)
	FRAME_EVENT_HANDLER(EVENT_BUTTON_CLICKED, OnBtnClicked)
	FRAME_EVENT_FRAME_HANDLER(EVENT_FRAME_RECT_CHANGED, m_rRootFrame , OnRootFrameRectChanged)
END_FRAME_EVENT_MAP()

public:
	CWindowEvent()
		: m_nScrollFrmRootFrmHeightDelta(0)
	{
	}

	VOID OnRootFrameRectChanged(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		if (m_rRootFrame && m_rSysBtnMax)
		{
			if (m_rRootFrame->IsZoomed())
				m_rSysBtnMax->ChangeButtonFaceTo(1);
			else
				m_rSysBtnMax->ChangeButtonFaceTo(0);
		}
	}

	VOID OnDialogButtonClicked(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		if (m_rRootFrame)
		{
			CTestDialog dlg;
			dlg.DoModel(m_rRootFrame, new CXFrame::LayoutParam(), TRUE, TRUE);
		}
		
	}

	VOID OnBtnClicked(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
	{
		if (!pSrcFrame)
			return;

		if (m_rSysBtnMin == pSrcFrame)
		{
			if (m_rRootFrame)
				m_rRootFrame->ShowWindow(SW_MINIMIZE);
		}

		if (m_rSysBtnMax == pSrcFrame)
		{
			if (m_rRootFrame)
			{
				if (m_rRootFrame->IsZoomed())
					m_rRootFrame->ShowWindow(SW_RESTORE);
				else
					m_rRootFrame->ShowWindow(SW_MAXIMIZE);
			}
		}

		if (m_rSysBtnClose == pSrcFrame)
		{
			if (m_rRootFrame)
			{
				m_rRootFrame->Destroy();
				::PostQuitMessage(0);
			}
		}
	}

public:
	BOOL Attach(CXFrame *pFrame)
	{
		if (!pFrame)
			return FALSE;

		m_rRootFrame = (CXWindow *)pFrame;
		m_rSysBtnMin = (CXButton *)pFrame->GetFrameByName(_T("sys_min"));
		m_rSysBtnMax = (CXMultifaceButton *)pFrame->GetFrameByName(_T("sys_max"));
		m_rSysBtnClose = (CXButton *)pFrame->GetFrameByName(_T("sys_close"));
		m_rDialogButton = (CXButton *)pFrame->GetFrameByName(_T("dialog_button"));
		
		if (m_rRootFrame) m_rRootFrame->AddEventListener(this);
		if (m_rSysBtnMin) m_rSysBtnMin->AddEventListener(this);
		if (m_rSysBtnMax) m_rSysBtnMax->AddEventListener(this);
		if (m_rSysBtnClose) m_rSysBtnClose->AddEventListener(this);
		if (m_rDialogButton) m_rDialogButton->AddEventListener(this);
		
		return TRUE;
	}

private:
	CRemoteRef<CXWindow> m_rRootFrame;
	CRemoteRef<CXButton> m_rSysBtnMin;
	CRemoteRef<CXMultifaceButton> m_rSysBtnMax;
	CRemoteRef<CXButton> m_rSysBtnClose;
	CRemoteRef<CXButton> m_rDialogButton;

	INT m_nScrollFrmRootFrmHeightDelta;
};


int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	XUILife xuilife;

	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));
	::DefWindowProc(NULL, 0, 0, 0L);
	AtlInitCommonControls(ICC_BAR_CLASSES);

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CXFrame *pRootFrame = NULL;

	X_XML_DOC_TYPE doc;
	if (CXFrameXMLFactory::LoadXML(SMCRelativePath(_T("UI.xml")), &doc))
		pRootFrame = CXFrameXMLFactory::Instance().BuildFrame(doc.first_node(), NULL);

	CWindowEvent WndEvent;
	if (pRootFrame)
		WndEvent.Attach(pRootFrame);

 	theLoop.Run();
 	_Module.RemoveMessageLoop();

	if (pRootFrame)
		delete pRootFrame;
	
	CoUninitialize();

	_Module.Term();
 

	return 0;
}
