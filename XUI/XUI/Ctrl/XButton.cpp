#include "StdAfx.h"

#include "XButton.h"
#include "..\..\..\XLib\inc\interfaceS\string\StringCode.h"
#include "..\Base\XResourceMgr.h"

X_IMPLEMENT_FRAME_XML_RUNTIME(CXButton)

CXFrame * CXButton::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
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

	IXImage *pFace = NULL;
	BOOL bDisabled = FALSE;
	
	attr = xml->first_attribute("bg", 0, false);
	if (attr)
		xml->remove_attribute(attr);

	pFace = CXFrameXMLFactory::BuildImage(xml, "face", "face_type", "stretch", "face_part_");

	attr = xml->first_attribute("disabled", 0, false);
	if (attr && !StrCmpIA(attr->value(), "true"))
		bDisabled = TRUE;

 	CXButton *pFrame = new CXButton();
	pFrame->Create(pParent, pLayout, VISIBILITY_NONE, bDisabled, pFace);

 	return pFrame;
}


CXButton::CXButton(void)
	: m_bMouseIn(FALSE),
	m_bMouseDown(FALSE),
	m_bDisabled(FALSE)
{
}

BOOL CXButton::Create(  CXFrame * pFrameParent, LayoutParam * pLayout,  VISIBILITY visibility /*= VISIBILITY_NONE*/ ,
					  BOOL bDisabled /* = FALSE*/, IXImage *pBackground /*= NULL*/ )
{
	if (!pLayout) 
	{
		ATLASSERT(!_T("No layout parameter. "));
		return FALSE;
	}

	BOOL bRtn = __super::Create(pFrameParent, pLayout, visibility);

	m_bDisabled = bDisabled;

	if (!pBackground)
		pBackground = CXResourceMgr::GetImage(_T("img/ctrl/button.9.png"));

	ATLASSERT(!pBackground || pBackground->GetImageWidth() % 4 == 0);

	delete SetBackground(pBackground);

	RefreashButtonFace();

	return bRtn;
}

VOID CXButton::Destroy()
{
	m_bMouseIn = FALSE;
	m_bMouseDown = FALSE;
	m_bDisabled = FALSE;

	__super::Destroy();
}

VOID CXButton::RefreashButtonFace()
{
	IXImage *pBackground = GetBackground();

	if (!pBackground)
		return;

	ATLASSERT(pBackground->GetImageWidth() % 4 == 0);

	BtnState state = m_bDisabled ? BTN_DISABLED : BTN_NORMAL;

	if (!m_bDisabled && m_bMouseIn)
		state = m_bMouseDown ? BTN_DOWN : BTN_HOVER;

	pBackground->SetSrcRect(CRect( CPoint(state * pBackground->GetImageWidth() / 4, 0), 
		CSize(pBackground->GetImageWidth() / 4, pBackground->GetImageHeight())));

	InvalidateRect();
}

LRESULT CXButton::OnLButtonDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	bHandled = TRUE;

	if (m_bDisabled)
		return 0;

	m_bMouseDown = TRUE;

	RefreashButtonFace();

	CXFrameMsgMgr *pMgr = GetFrameMsgMgr();
	if (pMgr)
		pMgr->CaptureMouse(this);

	return 0;
}

LRESULT CXButton::OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	bHandled = TRUE;

	if (!m_bMouseDown)
		return 0;

	m_bMouseDown = FALSE;

	CXFrameMsgMgr *pMgr = GetFrameMsgMgr();
	if (pMgr)
		pMgr->ReleaseCaptureMouse(this);

	if (m_bDisabled)
		return 0;

	RefreashButtonFace();

	if (::PtInRect(&ParentToChild(GetRect()), CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))))
		ThrowEvent(EVENT_BUTTON_CLICKED, 0, 0);

	return 0;
}

LRESULT CXButton::OnMouseEnter( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	m_bMouseIn = TRUE;

	if (m_bDisabled)
		return 0;

	RefreashButtonFace();

	return 0;
}

LRESULT CXButton::OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	m_bMouseIn = FALSE;

	if (m_bDisabled)
		return 0;

	RefreashButtonFace();

	return 0;
}

BOOL CXButton::EnableButton( BOOL bEnable /*= TRUE*/ )
{
	if ((!bEnable && m_bDisabled) || (bEnable && !m_bDisabled))
		return TRUE;

	m_bDisabled = !bEnable;

	RefreashButtonFace();

	return TRUE;
}

BOOL CXButton::OnMeasureWidth( const MeasureParam & param )
{
	if (param.m_Spec != MeasureParam::MEASURE_ATMOST
		&& param.m_Spec != MeasureParam::MEASURE_UNRESTRICTED )
		return __super::OnMeasureWidth(param);

	IXImage *pBg = GetBackground();
	int nWrapSize = 0;
	if (pBg) nWrapSize = pBg->GetImageWidth() / 4;

	if (param.m_Spec == MeasureParam::MEASURE_ATMOST)
		nWrapSize = min(nWrapSize, param.m_nNum);
	
	MeasureParam WrapParam;
	WrapParam.m_Spec = MeasureParam::MEASURE_EXACT;
	WrapParam.m_nNum = nWrapSize;

	return __super::OnMeasureWidth(WrapParam);
}

BOOL CXButton::OnMeasureHeight( const MeasureParam & param )
{
	if (param.m_Spec != MeasureParam::MEASURE_ATMOST
		&& param.m_Spec != MeasureParam::MEASURE_UNRESTRICTED )
		return __super::OnMeasureHeight(param);

	IXImage *pBg = GetBackground();
	int nWrapSize = 0;
	if (pBg) nWrapSize = pBg->GetImageHeight();

	if (param.m_Spec == MeasureParam::MEASURE_ATMOST)
		nWrapSize = min(nWrapSize, param.m_nNum);

	MeasureParam WrapParam;
	WrapParam.m_Spec = MeasureParam::MEASURE_EXACT;
	WrapParam.m_nNum = nWrapSize;

	return __super::OnMeasureHeight(WrapParam);
}

IXImage * CXButton::SetBackground( IXImage * pDrawBackground )

{
	IXImage *pImage = __super::SetBackground(pDrawBackground);
	RefreashButtonFace();

	InvalidateLayout();
	
	return pImage;
}

BOOL CXButton::OnLayout( const CRect & rcRect )
{
	return __super::OnLayout(rcRect);
}

