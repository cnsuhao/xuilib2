#include "StdAfx.h"

#include "XTree.h"

#include "../Base/XResourceMgr.h"
#include "XMultifaceButton.h"
#include "../../../XLib/inc/interfaceS/string/StringCode.h"

X_IMPLEMENT_FRAME_XML_RUNTIME(CXTree)

#define FOLD_UNFOLD_BUTTON_MARGIN_RIGHT		(10)
#define TREE_LINE_GRAY_DEEP					(0x99)

CXFrame * CXTree::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
{
	if (!xml)
		return NULL;

	LayoutParam *pLayout = pParent ?
		pParent->GenerateLayoutParam(xml) : new CXFrame::LayoutParam(xml);
	if (!pLayout)
	{
		CStringA strError;
		strError.Format("WARNING: Generating the layout parameter for the parent %s failed. \
						Building the frame failed. ", XLibST2A(pParent->GetName()));
		CXFrameXMLFactory::ReportError(strError);
		return NULL;
	}

	X_XML_ATTR_TYPE attr = NULL;

	BOOL bUnfolded = TRUE;
	UINT nChildItemIndent = 20;
	IXImage *pFoldedButtonFace = NULL;
	IXImage *pUnfoldedButtonFace = NULL;

	attr = xml->first_attribute("start_state", 0, false);
	if (attr && !StrCmpIA(attr->value(), "folded")) bUnfolded = FALSE;

	attr = xml->first_attribute("child_item_indent", 0 ,false);
	if (attr) nChildItemIndent = strtoul(attr->value(), NULL, 10);

	pFoldedButtonFace = CXFrameXMLFactory::BuildImage(xml, "folded_button", "folded_button_type", "normal", "folded_button_part_");
	pUnfoldedButtonFace = CXFrameXMLFactory::BuildImage(xml, "unfolded_button", "unfolded_button_type", "normal", "unfolded_button_part_");

	CXTree *pFrame = new CXTree();
	pFrame->Create(pParent, pLayout, VISIBILITY_NONE,
		bUnfolded, nChildItemIndent, pFoldedButtonFace, pUnfoldedButtonFace);

	return pFrame;
}

CXTree::CXTree(void)
	: m_pRootItemFrameContainer(NULL),
	m_pChildItemFrameContainer(NULL),
	m_nChildIndent(0)
{
}

BOOL CXTree::Create( CXFrame * pFrameParent, LayoutParam * pLayout, VISIBILITY visibility /*= VISIBILITY_NONE*/, 
					BOOL bUnfolded /*= TRUE*/, UINT nChildIndent/*=20*/, IXImage *pFoldedButtonFace /*= NULL*/, IXImage *pUnfoldedButtonFace /*= NULL*/)
{
	__super::Create(pFrameParent, CXDock::DOCK_TOP2BOTTOM, CXDock::ALIGN_LOW, pLayout, visibility);

	LayoutParam *pRootItemFrameContainerLayout = GenerateLayoutParam();
	if (!pRootItemFrameContainerLayout) {ATLASSERT(NULL); return FALSE;}
	pRootItemFrameContainerLayout->m_mWidth = pRootItemFrameContainerLayout->m_mHeight 
		= LayoutParam::METRIC_WRAP_CONTENT;

 	LayoutParam *pChildItemFrameContainerLayout = GenerateLayoutParam();
 	if (!pChildItemFrameContainerLayout) {ATLASSERT(NULL); return FALSE;}
 	pChildItemFrameContainerLayout->m_mWidth = pChildItemFrameContainerLayout->m_mHeight 
 		= LayoutParam::METRIC_WRAP_CONTENT;

	LayoutParam *pFoldUnfoldButtonLayout = GenerateLayoutParam();
	if (!pFoldUnfoldButtonLayout) {ATLASSERT(NULL); return FALSE;}
	pFoldUnfoldButtonLayout->m_mWidth = pFoldUnfoldButtonLayout->m_mHeight 
		= LayoutParam::METRIC_WRAP_CONTENT;
	pFoldUnfoldButtonLayout->m_nMarginRight = FOLD_UNFOLD_BUTTON_MARGIN_RIGHT;

	m_pRootItemFrameContainer = new CXDock();
	m_pRootItemFrameContainer->Create(this, CXDock::DOCK_LEFT2RIGHT, CXDock::ALIGN_MIDDLE, 
		pRootItemFrameContainerLayout, VISIBILITY_SHOW);
	m_pRootItemFrameContainer->AddEventListener(this);
	m_pChildItemFrameContainer = new CXDock();
	m_pChildItemFrameContainer->Create(this, CXDock::DOCK_TOP2BOTTOM, CXDock::ALIGN_LOW,
		pChildItemFrameContainerLayout, VISIBILITY_SHOW);

	if (pFoldedButtonFace == NULL)
		pFoldedButtonFace = CXResourceMgr::GetImage(_T("img/ctrl/folded_button.png"));
	if (pUnfoldedButtonFace == NULL)
		pUnfoldedButtonFace = CXResourceMgr::GetImage(_T("img/ctrl/unfolded_button.png"));

	IXImage *pButtonFaces[] = {pFoldedButtonFace, pUnfoldedButtonFace};

	CXMultifaceButton *pFoldUnfoldButton = new CXMultifaceButton();
	pFoldUnfoldButton->Create(m_pRootItemFrameContainer, pButtonFaces, 2, 
		pFoldUnfoldButtonLayout, VISIBILITY_SHOW,
		bUnfolded ? 1 : 0, FALSE);

	pFoldUnfoldButton->AddEventListener(this);

	SetUnfold(bUnfolded);

	SetChildItemIndent(nChildIndent);

	return TRUE;
}

BOOL CXTree::SetUnfold( BOOL bUnfold )
{
 	if (!m_pChildItemFrameContainer)
 		return FALSE;
 
 	if (bUnfold && m_pChildItemFrameContainer->GetVisibility() == VISIBILITY_SHOW)
 		return TRUE;
 	if (!bUnfold && m_pChildItemFrameContainer->GetVisibility() == VISIBILITY_NONE)
 		return TRUE;
 
 	m_pChildItemFrameContainer->SetVisibility(bUnfold ? VISIBILITY_SHOW : VISIBILITY_NONE);
 
 	CXMultifaceButton *pFoldUnfoldButton = NULL;
 	if (m_pRootItemFrameContainer && (pFoldUnfoldButton = (CXMultifaceButton *)m_pRootItemFrameContainer->GetFrameByIndex(0)))
 		pFoldUnfoldButton->ChangeButtonFaceTo(bUnfold ? 1 : 0);

	return TRUE;
}

BOOL CXTree::IsUnfolded()
{
	if (!m_pChildItemFrameContainer)
		return FALSE;

	return m_pChildItemFrameContainer->GetVisibility() == VISIBILITY_SHOW;
}

BOOL CXTree::SetChildItemIndent( UINT nIndent )
{
	if (m_nChildIndent == nIndent)
		return TRUE;

	m_nChildIndent = nIndent;

	UpdateLeftMarginOfChildItemContainer();

	return TRUE;
}

CXFrame * CXTree::SetRootItemFrame( CXFrame *pFrame )
{
	if (!m_pRootItemFrameContainer)
		return pFrame;

	CXFrame *pOldFrame = 
		m_pRootItemFrameContainer->RemoveFrame(1);

	if (pFrame)
		m_pRootItemFrameContainer->AddFrame(pFrame);

	return pOldFrame;
}

BOOL CXTree::AddChildItemFrame( CXFrame *pFrame )
{
	return InsertChildItemFrame(pFrame, GetChildItemFrameCount());
}

BOOL CXTree::InsertChildItemFrame( CXFrame *pFrame, UINT nIndex)
{
	if (!pFrame)
		return FALSE;

	if (!m_pChildItemFrameContainer)
		return FALSE;

	return m_pChildItemFrameContainer->InsertFrame(pFrame, nIndex);
}


CXFrame * CXTree::RemoveChildItemFrame( UINT nIndex )
{
	if (!m_pChildItemFrameContainer)
		return NULL;

	return m_pChildItemFrameContainer->RemoveFrame(nIndex);
}

UINT CXTree::GetChildItemFrameCount()
{
	if (!m_pChildItemFrameContainer)
		return 0;

	return m_pChildItemFrameContainer->GetFrameCount();
}

BOOL CXTree::HandleXMLChildNodes( X_XML_NODE_TYPE xml )
{
	BOOL bFirstChild = TRUE;

	if (!m_pRootItemFrameContainer)
	{
		ATLASSERT(NULL);
		return FALSE;
	}

	if (!m_pChildItemFrameContainer)
	{
		ATLASSERT(NULL);
		return FALSE;
	}

	for (X_XML_NODE_TYPE child = xml->first_node();
		child; child = child->next_sibling())
	{
		if (child->type() != X_XML_NODE_CATEGORY_TYPE::node_element)
			continue;
		
		if (bFirstChild)
		{
			delete SetRootItemFrame(NULL);

			CXFrame *pChild = 
				CXFrameXMLFactory::Instance().BuildFrame(child, m_pRootItemFrameContainer);

			if (!pChild)
				continue;

			bFirstChild = FALSE;
		}
		else
		{
			CXFrame *pChild = 
				CXFrameXMLFactory::Instance().BuildFrame(child, m_pChildItemFrameContainer);
		}
	}

	return TRUE;
}

VOID CXTree::OnFoldUnfoldButtonClicked( CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
 	if (!m_pRootItemFrameContainer)
 		return;

 	CXFrame *pFoldUnfoldButton = m_pRootItemFrameContainer->GetFrameByIndex(0);
 	if (!pFoldUnfoldButton)
 		return;
 
 	if (pSrcFrame != pFoldUnfoldButton)
 		return;
 
 	if (IsUnfolded())
 		SetUnfold(FALSE);
 	else
 		SetUnfold(TRUE);
}

BOOL CXTree::PaintForeground( HDC hDC, const CRect &rcUpdate )
{
	if (IsUnfolded())
	{
		CXFrame *pFoldUnFoldButton = NULL;
		if (m_pRootItemFrameContainer && (pFoldUnFoldButton = m_pRootItemFrameContainer->GetFrameByIndex(0))
			&& m_pChildItemFrameContainer && m_pChildItemFrameContainer->GetFrameCount())
		{
			CRect rcFoldUnFoldButton(pFoldUnFoldButton->GetRect());

			INT nChildItemFrameContainerTop = m_pChildItemFrameContainer->GetRect().top;
			
			INT nLinesXStart = 
				rcFoldUnFoldButton.left + rcFoldUnFoldButton.Width() / 2
				+ m_pRootItemFrameContainer->GetRect().left;
			INT nLinesXEnd =
				m_pChildItemFrameContainer->GetRect().left;
			INT nVLineYStart = m_pRootItemFrameContainer->GetRect().bottom;

			if (!(nLinesXEnd - nLinesXStart <= 0 || 
				nLinesXStart >= rcUpdate.right || nLinesXEnd <= rcUpdate.left))
				for (UINT i = 0; i < m_pChildItemFrameContainer->GetFrameCount(); i++)
				{
					CXFrame *pCurrentFrame = m_pChildItemFrameContainer->GetFrameByIndex(i);
					if (!pCurrentFrame)
						continue;

					if (pCurrentFrame->GetVisibility() != VISIBILITY_SHOW)
						continue;

					INT nHLineY = pCurrentFrame->GetRect().top + max(0, pCurrentFrame->GetVCenter()) 
						+ nChildItemFrameContainerTop;

					nVLineYStart = max(nVLineYStart, rcUpdate.top);
					INT nVLineYEnd = min(nHLineY + 1, rcUpdate.bottom);

					if (nLinesXStart >= rcUpdate.left &&
						nLinesXStart < rcUpdate.right &&
						nVLineYEnd - nVLineYStart > 0)
						//Draw V Line
						Util::DrawLine(hDC, CPoint(nLinesXStart, nVLineYStart), CPoint(nLinesXStart, nVLineYEnd), 1, 
						TREE_LINE_GRAY_DEEP, TREE_LINE_GRAY_DEEP, TREE_LINE_GRAY_DEEP, 255, 
						TRUE);

					nVLineYStart = nVLineYEnd;

					INT nHLineXStart = max(rcUpdate.left, nLinesXStart);
					INT nHLineXEnd = min(rcUpdate.right, nLinesXEnd);

					if (nHLineY >= rcUpdate.top &&
						nHLineY < rcUpdate.bottom &&
						nHLineXEnd - nHLineXStart > 0)
						// Draw H Line
						Util::DrawLine(hDC, CPoint(nHLineXStart, nHLineY), CPoint(nHLineXEnd, nHLineY), 1, 
						TREE_LINE_GRAY_DEEP, TREE_LINE_GRAY_DEEP, TREE_LINE_GRAY_DEEP, 255, 
						TRUE);

					if (nHLineY >= rcUpdate.bottom)
						break;
				}
		}
	}

	return __super::PaintForeground(hDC, rcUpdate);
}

VOID CXTree::InvalidateLines()
{
	CRect rcInvalid(ParentToChild(GetRect()));

	if (m_pRootItemFrameContainer)
		rcInvalid.top = max(rcInvalid.top, m_pRootItemFrameContainer->GetRect().top);
	if (m_pChildItemFrameContainer)
	{
		CRect rcChildItemFrameContainer(m_pChildItemFrameContainer->GetRect());
		rcInvalid.right = min(rcInvalid.right, rcChildItemFrameContainer.left);
		rcInvalid.bottom = min(rcInvalid.bottom, rcChildItemFrameContainer.bottom);
	}

	InvalidateRect(rcInvalid);
}

INT CXTree::GetVCenter()
{
	CXFrame *pFoldUnfoldButton = NULL;
	if (m_pRootItemFrameContainer && (pFoldUnfoldButton = m_pRootItemFrameContainer->GetFrameByIndex(0)))
	{
		CRect rcFoldUnfoldButton(pFoldUnfoldButton->GetRect());
		return rcFoldUnfoldButton.top + rcFoldUnfoldButton.Height() / 2 + m_pRootItemFrameContainer->GetRect().top;
	}

	return __super::GetVCenter();
}

VOID CXTree::Destroy()
{
	m_pRootItemFrameContainer = m_pChildItemFrameContainer = NULL;

	m_nChildIndent = 0;

	__super::Destroy();
}

VOID CXTree::OnFoldUnfoldButtonFrameChanged( CXFrame *pSrcFrame, UINT uEvent, 
											WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!m_pRootItemFrameContainer)
 		return;

 	CXFrame *pFoldUnfoldButton = m_pRootItemFrameContainer->GetFrameByIndex(0);
 	if (!pFoldUnfoldButton)
 		return;
 
 	if (pSrcFrame != pFoldUnfoldButton)
 		return;

	const CRect &rcOld = *(CRect *)wParam;
	const CRect &rcNew = *(CRect *)lParam;

	if (rcOld.right == rcNew.right)
		return;

	UpdateLeftMarginOfChildItemContainer();
}


VOID CXTree::OnRootFrameRectChanged( CXFrame *pSrcFrame, UINT uEvent, 
									WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	const CRect &rcOld = *(CRect *)wParam;
	const CRect &rcNew = *(CRect *)lParam;

	if (rcOld.left == rcNew.left)
		return;

	UpdateLeftMarginOfChildItemContainer();
}

VOID CXTree::UpdateLeftMarginOfChildItemContainer()
{
	ATLASSERT(m_pRootItemFrameContainer && m_pChildItemFrameContainer);

	int nChildMarginLeft = m_nChildIndent;

	if (m_pRootItemFrameContainer)
	{
		nChildMarginLeft += m_pRootItemFrameContainer->GetRect().left;
		CXFrame * pFoldUnfoldButton = NULL;
		if (pFoldUnfoldButton = m_pRootItemFrameContainer->GetFrameByIndex(0))
			nChildMarginLeft += pFoldUnfoldButton->GetRect().right + FOLD_UNFOLD_BUTTON_MARGIN_RIGHT;
	}

	if (m_pChildItemFrameContainer)
	{
		LayoutParam *pLayoutParam = m_pChildItemFrameContainer->BeginUpdateLayoutParam();
		ATLASSERT(pLayoutParam);
		if (pLayoutParam)
		{
			pLayoutParam->m_nMarginLeft = nChildMarginLeft;
			m_pChildItemFrameContainer->EndUpdateLayoutParam();
		}
	}
}

BOOL CXTree::OnLayout( const CRect & rcRect )
{
	InvalidateLines();

	BOOL bRtn = __super::OnLayout(rcRect);

	InvalidateLines();

	return bRtn;
}