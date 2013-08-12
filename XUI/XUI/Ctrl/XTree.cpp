#include "StdAfx.h"

#include "XTree.h"

#include "../Base/XResourceMgr.h"
#include "XMultifaceButton.h"

X_IMPLEMENT_FRAME_XML_RUNTIME(CXTree)

#define FOLD_UNFOLD_BUTTON_MARGIN_RIGHT		(10)
#define TREE_LINE_GRAY_DEEP					(0x99)

CXFrame * CXTree::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
{
	if (!xml)
		return NULL;

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
	pFrame->Create(pParent, CXFrameXMLFactory::BuildRect(xml), FALSE,
		bUnfolded, nChildItemIndent, pFoldedButtonFace, pUnfoldedButtonFace,
		(CXFrame::WIDTH_MODE)CXFrameXMLFactory::GetWidthMode(xml), (CXFrame::HEIGHT_MODE)CXFrameXMLFactory::GetHeightMode(xml));

	return pFrame;
}

CXTree::CXTree(void)
	: m_pRootItemFrameContainer(NULL),
	m_pChildItemFrameContainer(NULL)
{
}

BOOL CXTree::Create( CXFrame * pFrameParent, const CRect & rcRect /*= CRect(0, 0, 0, 0)*/, BOOL bVisible /*= FALSE*/, 
					BOOL bUnfolded /*= TRUE*/, UINT nChildIndent/*=20*/, IXImage *pFoldedButtonFace /*= NULL*/, IXImage *pUnfoldedButtonFace /*= NULL*/, 
					WIDTH_MODE aWidthMode /*= WIDTH_MODE_NOT_CHANGE*/, HEIGHT_MODE aHeightMode /*= HEIGHT_MODE_NOT_CHANGE*/ )
{
	__super::Create(pFrameParent, DOCK_TOP2BOTTOM, ALIGN_LOW, rcRect, bVisible, aWidthMode, aHeightMode);

	m_pRootItemFrameContainer = new CXDock();
	m_pRootItemFrameContainer->Create(this, DOCK_LEFT2RIGHT, ALIGN_MIDDLE, 
		CRect(0, 0, 0, 0), TRUE, WIDTH_MODE_WRAP_CONTENT, HEIGHT_MODE_WRAP_CONTENT);
	m_pChildItemFrameContainer = new CXDock();
	m_pChildItemFrameContainer->Create(this, DOCK_TOP2BOTTOM, ALIGN_LOW,
		CRect(0, 0, 0, 0), TRUE, WIDTH_MODE_WRAP_CONTENT, HEIGHT_MODE_WRAP_CONTENT);
	m_pChildItemFrameContainer->AddEventListener(this);

	if (pFoldedButtonFace == NULL)
		pFoldedButtonFace = CXResourceMgr::Instance().GetImage(_T("img/ctrl/folded_button.png"));
	if (pUnfoldedButtonFace == NULL)
		pUnfoldedButtonFace = CXResourceMgr::Instance().GetImage(_T("img/ctrl/unfolded_button.png"));

	IXImage *pButtonFaces[] = {pFoldedButtonFace, pUnfoldedButtonFace};

	CXMultifaceButton *pFoldUnfoldButton = new CXMultifaceButton();
	pFoldUnfoldButton->Create(m_pRootItemFrameContainer, pButtonFaces, 2, 
		CRect(0, 0, 0, 0), TRUE,
		bUnfolded ? 1 : 0, FALSE,
		WIDTH_MODE_ADAPT_BACKGROUND, HEIGHT_MODE_ADAPT_BACKGROUND);

	pFoldUnfoldButton->SetMargin(CRect(0, 0, FOLD_UNFOLD_BUTTON_MARGIN_RIGHT, 0));

	pFoldUnfoldButton->AddEventListener(this);

	SetUnfold(bUnfolded);

	SetChildItemIndent(nChildIndent);

	return TRUE;
}

BOOL CXTree::SetUnfold( BOOL bUnfold )
{
	if (!m_pChildItemFrameContainer)
		return FALSE;

	if (bUnfold && m_pChildItemFrameContainer->IsVisible() && m_pChildItemFrameContainer->IsHoldPlace())
		return TRUE;
	if (!bUnfold && !m_pChildItemFrameContainer->IsVisible() && !m_pChildItemFrameContainer->IsHoldPlace())
		return TRUE;

	m_pChildItemFrameContainer->SetVisible(bUnfold);
	m_pChildItemFrameContainer->SetHoldPlace(bUnfold);

	CXMultifaceButton *pFoldUnfoldButton = NULL;
	if (m_pRootItemFrameContainer && (pFoldUnfoldButton = (CXMultifaceButton *)m_pRootItemFrameContainer->GetFrameByIndex(0)))
		pFoldUnfoldButton->ChangeButtonFaceTo(bUnfold ? 1 : 0);

	InvalidateLines();

	return TRUE;
}

BOOL CXTree::IsUnfold()
{
	if (!m_pChildItemFrameContainer)
		return FALSE;

	if (m_pChildItemFrameContainer->IsVisible() && m_pChildItemFrameContainer->IsHoldPlace())
		return TRUE;

	return FALSE;
}

BOOL CXTree::SetChildItemIndent( UINT nIndent )
{
	if (!m_pRootItemFrameContainer || !m_pChildItemFrameContainer)
		return FALSE;
	CXFrame *pFoldUnfoldButton = m_pRootItemFrameContainer->GetFrameByIndex(0);
	if (!pFoldUnfoldButton)
		return FALSE;

	CRect rcChildItemFrameContainerMargin(m_pChildItemFrameContainer->GetMargin());
	rcChildItemFrameContainerMargin.left = 
		pFoldUnfoldButton->GetRect().right + pFoldUnfoldButton->GetMargin().right + m_pRootItemFrameContainer->GetRect().left +
		nIndent;

	m_pChildItemFrameContainer->SetMargin(rcChildItemFrameContainerMargin);

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

	for (X_XML_NODE_TYPE child = xml->first_node();
		child; child = child->next_sibling())
	{
		if (child->type() != X_XML_NODE_CATEGORY_TYPE::node_element)
			continue;
		
		CXFrame *pFrame = 
			CXFrameXMLFactory::Instance().BuildFrame(child, NULL);

		if (!pFrame)
			continue;

		if (bFirstChild)
		{
			delete SetRootItemFrame(pFrame);
			bFirstChild = FALSE;
		}
		else
			AddChildItemFrame(pFrame);
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
 
 	if (IsUnfold())
 		SetUnfold(FALSE);
 	else
 		SetUnfold(TRUE);
}

BOOL CXTree::PaintForeground( HDC hDC, const CRect &rcUpdate )
{
	if (IsUnfold())
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

			if (!(nLinesXEnd - nLinesXStart < 0 || 
				nLinesXStart >= rcUpdate.right || nLinesXEnd <= rcUpdate.left))
				for (UINT i = 0; i < m_pChildItemFrameContainer->GetFrameCount(); i++)
				{
					CXFrame *pCurrentFrame = m_pChildItemFrameContainer->GetFrameByIndex(i);
					if (!pCurrentFrame)
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

VOID CXTree::OnChildItemFrameRedock( CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	CRect rcInvalid(ParentToChild(GetRect()));

	if (m_pRootItemFrameContainer)
		rcInvalid.top = max(rcInvalid.top, m_pRootItemFrameContainer->GetRect().top);
	if (m_pChildItemFrameContainer)
	{
		CRect rcChildItemFrameContainer(m_pChildItemFrameContainer->GetRect());
		rcInvalid.right = min(rcInvalid.right, rcChildItemFrameContainer.left);
		rcInvalid.bottom = min(rcInvalid.bottom, rcChildItemFrameContainer.bottom);

		CXFrame *pFrame = m_pChildItemFrameContainer->GetFrameByIndex(wParam);
		if (pFrame)
			rcInvalid.top = max(rcInvalid.top, pFrame->GetRect().top + rcChildItemFrameContainer.top);
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

	__super::Destroy();
}
