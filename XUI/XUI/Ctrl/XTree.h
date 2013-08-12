#pragma once

#include "XDock.h"

class CXTree :
	public CXDock
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXTree, XTree)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXTree@@0VCXFrameXMLRuntime_CXTree@1@A")

	BEGIN_FRAME_EVENT_MAP(CXTree)
		CHAIN_FRAME_EVENT_MAP(CXDock)
		FRAME_EVENT_HANDLER(EVENT_BUTTON_CLICKED, OnFoldUnfoldButtonClicked)
		FRAME_EVENT_FRAME_HANDLER(EVENT_DOCK_ITEMS_REDOCK_BEGIN, m_pChildItemFrameContainer, OnChildItemFrameRedock)
		FRAME_EVENT_FRAME_HANDLER(EVENT_DOCK_ITEMS_REDOCKED, m_pChildItemFrameContainer, OnChildItemFrameRedock)
	END_FRAME_EVENT_MAP()

public:
	BOOL Create(CXFrame * pFrameParent, const CRect & rcRect = CRect(0, 0, 0, 0), BOOL bVisible = FALSE,
		BOOL bUnfolded = TRUE, UINT nChildIndent=20, IXImage *pFoldedButtonFace = NULL, IXImage *pUnfoldedButtonFace = NULL,
		WIDTH_MODE aWidthMode = WIDTH_MODE_NOT_CHANGE, HEIGHT_MODE aHeightMode = HEIGHT_MODE_NOT_CHANGE);

public:
	BOOL SetUnfold(BOOL bUnfold);
	BOOL IsUnfold();
	BOOL SetChildItemIndent(UINT nIndent);

	CXFrame * SetRootItemFrame(CXFrame *pFrame);

	BOOL InsertChildItemFrame(CXFrame *pFrame, UINT nIndex);
	BOOL AddChildItemFrame(CXFrame *pFrame);
	UINT GetChildItemFrameCount();
	CXFrame * RemoveChildItemFrame(UINT nIndex);

public:
	VOID OnFoldUnfoldButtonClicked(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	VOID OnChildItemFrameRedock(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled );

public:
	virtual BOOL HandleXMLChildNodes(X_XML_NODE_TYPE xml);
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual INT GetVCenter();
	virtual VOID Destroy();

public:
	CXTree(void);

private:
	VOID InvalidateLines();

private:
	CXDock * m_pRootItemFrameContainer;
	CXDock * m_pChildItemFrameContainer;
};
 