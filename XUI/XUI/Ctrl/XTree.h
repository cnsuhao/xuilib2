#pragma once

#include "XDock.h"

class CXTree :
	public CXDock
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXTree, XTree)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXTree@@0VCXFrameXMLRuntime_CXTree@1@A")

	BEGIN_FRAME_EVENT_MAP(CXTree)
		CHAIN_FRAME_EVENT_MAP(CXFrame)
		FRAME_EVENT_HANDLER(EVENT_BUTTON_CLICKED, OnFoldUnfoldButtonClicked)
		FRAME_EVENT_FRAME_HANDLER(EVENT_FRAME_RECT_CHANGED, m_pRootItemFrameContainer, OnRootFrameRectChanged)
		FRAME_EVENT_HANDLER(EVENT_FRAME_RECT_CHANGED, OnFoldUnfoldButtonFrameChanged)
	END_FRAME_EVENT_MAP()

public:
	BOOL Create(CXFrame * pFrameParent, LayoutParam * pLayout, VISIBILITY visibility = VISIBILITY_NONE,
		BOOL bUnfolded = TRUE, UINT nChildIndent=20, IXImage *pFoldedButtonFace = NULL, IXImage *pUnfoldedButtonFace = NULL);

public:
	BOOL SetUnfold(BOOL bUnfold);
	BOOL IsUnfold();
	BOOL SetChildItemIndent(UINT nIndent);

	CXFrame * SetRootItemFrame(CXFrame *pFrame);

	BOOL InsertChildItemFrame(CXFrame *pFrame, UINT nIndex);
	BOOL AddChildItemFrame(CXFrame *pFrame);
	UINT GetChildItemFrameCount();
	CXFrame * RemoveChildItemFrame(UINT nIndex);

 	virtual BOOL OnLayout(const CRect & rcRect);

public:
	VOID OnFoldUnfoldButtonClicked(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	VOID OnFoldUnfoldButtonFrameChanged( CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	VOID OnRootFrameRectChanged( CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled );

public:
	virtual BOOL HandleXMLChildNodes(X_XML_NODE_TYPE xml);
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual INT GetVCenter();
	virtual VOID Destroy();

public:
	CXTree(void);

private:
	VOID InvalidateLines();
	VOID UpdateLeftMarginOfChildItemContainer();

private:
	CXDock * m_pRootItemFrameContainer;
	CXDock * m_pChildItemFrameContainer;
	INT m_nChildIndent;
};
 