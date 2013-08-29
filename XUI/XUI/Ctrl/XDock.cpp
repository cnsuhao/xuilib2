#include "StdAfx.h"
#include "XDock.h"
#include "..\..\..\XLib\inc\interfaceS\string\StringCode.h"

X_IMPLEMENT_FRAME_XML_RUNTIME(CXDock)

CXFrame * CXDock::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
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

	DockType eDockType = DOCK_LEFT2RIGHT;
	Align eAlignType = ALIGN_MIDDLE;
	
	attr = xml->first_attribute("dock_type", 0, false);
	if (attr)
	{
		if (!StrCmpIA(attr->value(), "left2right"))
			eDockType = DOCK_LEFT2RIGHT;
		else if (!StrCmpIA(attr->value(), "top2bottom"))
			eDockType = DOCK_TOP2BOTTOM;
		else if (!StrCmpIA(attr->value(), "right2left"))
			eDockType = DOCK_RIGHT2LEFT;
		else if (!StrCmpIA(attr->value(), "bottom2top"))
			eDockType = DOCK_BOTTOM2TOP;
	}

	attr = xml->first_attribute("align_type", 0, false);
	if (attr)
	{
		if (!StrCmpIA(attr->value(), "low"))
			eAlignType = ALIGN_LOW;
		else if (!StrCmpIA(attr->value(), "middle"))
			eAlignType = ALIGN_MIDDLE;
		else if (!StrCmpIA(attr->value(), "high"))
			eAlignType = ALIGN_HIGH;
	}

	CXDock *pDock = new CXDock();
	pDock->Create(pParent, eDockType, eAlignType, pLayout, VISIBILITY_NONE);

	return pDock;
}

CXDock::CXDock()
	: m_DockType(DOCK_LEFT2RIGHT), 
	m_Align(ALIGN_LOW)
{
}

VOID CXDock::Destroy()
{
	__super::Destroy();

	m_DockType = DOCK_LEFT2RIGHT;
	m_Align = ALIGN_LOW;
}

BOOL CXDock::Create( CXFrame * pFrameParent, DockType dock, Align align, LayoutParam * pLayout,  VISIBILITY visibility /*= VISIBILITY_NONE*/)
{
	if (!pLayout) 
	{
		ATLASSERT(!_T("No layout parameter. "));
		return FALSE;
	}

	BOOL bRtn = __super::Create(pFrameParent, pLayout, visibility);

	m_DockType = dock;
	m_Align = align;

	return bRtn;
}

BOOL CXDock::OnMeasureWidth( const MeasureParam & param )
{
	int nMeasurd = 0;

	if (m_DockType == DOCK_TOP2BOTTOM || m_DockType == DOCK_BOTTOM2TOP)
		OnMeasureAlignDirection(param, &nMeasurd, 
			&CXFrame::MeasureWidth, &CXFrame::GetMeasuredWidth, 
			&LayoutParam::m_nX,
			&LayoutParam::m_nMarginLeft, &LayoutParam::m_nMarginRight,
			&LayoutParam::m_nWidth, &LayoutParam::m_mWidth);
	else
		OnMeasureLayoutDirection(param, &nMeasurd, 
			&CXFrame::MeasureWidth, &CXFrame::GetMeasuredWidth, 
			&LayoutParam::m_nX,
			&LayoutParam::m_nMarginLeft, &LayoutParam::m_nMarginRight,
			&LayoutParam::m_nWidth, &LayoutParam::m_mWidth);

	SetMeasuredWidth(nMeasurd);

	return TRUE;
}

BOOL CXDock::OnMeasureHeight( const MeasureParam & param )
{
	int nMeasurd = 0;

	if (m_DockType == DOCK_LEFT2RIGHT || m_DockType == DOCK_RIGHT2LEFT)
		OnMeasureAlignDirection(param, &nMeasurd,
			&CXFrame::MeasureHeight, &CXFrame::GetMeasuredHeight, 
			&LayoutParam::m_nY, 
			&LayoutParam::m_nMarginTop, &LayoutParam::m_nMarginBottom,
			&LayoutParam::m_nHeight, &LayoutParam::m_mHeight);
	else
		OnMeasureLayoutDirection(param, &nMeasurd,
			&CXFrame::MeasureHeight, &CXFrame::GetMeasuredHeight, 
			&LayoutParam::m_nY, 
			&LayoutParam::m_nMarginTop, &LayoutParam::m_nMarginBottom,
			&LayoutParam::m_nHeight, &LayoutParam::m_mHeight);

	SetMeasuredHeight(nMeasurd);

	return TRUE;
}
BOOL CXDock::OnMeasureAlignDirection( const MeasureParam & param, INT *pMeasuredSize, 
									 BOOL (CXFrame::*pfChildMeasureProc)(const MeasureParam &), 
									 INT (CXFrame::*pfChildGetMeasurdProc)(), 
									 INT LayoutParam::*pLayoutParamPos, 
									 INT LayoutParam::*pLayoutMarginLow, 
									 INT LayoutParam::*pLayoutMarginHigh, 
									 INT LayoutParam::*pnLayoutParamSize, 
									 LayoutParam::SPECIAL_METRICS LayoutParam::*pmLayoutParamSize )
{
	if (!pMeasuredSize || !pfChildMeasureProc || !pfChildGetMeasurdProc ||
		!pLayoutMarginLow || !pLayoutMarginHigh ||
		!pLayoutParamPos || !pnLayoutParamSize || !pmLayoutParamSize)
	{
		ATLASSERT(NULL);
		return FALSE;
	}

	INT & nMeasuredSize = *pMeasuredSize;

	int nMaxSize = 0;

	INT nFrameCount = GetFrameCount();
	for (INT i = 0; i < nFrameCount; i++)
	{
		CXFrame *pCur = GetFrameByIndex(i);
		if (!pCur)
		{
			ATLASSERT(NULL);
			continue;
		}

		LayoutParam *pLayoutParam = pCur->GetLayoutParam();
		if (!pLayoutParam)
		{
			ATLASSERT(NULL);
			continue;
		}

		if (pCur->GetVisibility() == VISIBILITY_NONE)
			continue;

		if (pLayoutParam->*pmLayoutParamSize 
			== LayoutParam::METRIC_REACH_PARENT)
			continue;

		MeasureParam ParamForMeasure;
		switch (pLayoutParam->*pmLayoutParamSize)
		{
		case LayoutParam::METRIC_WRAP_CONTENT:
			if (param.m_Spec == MeasureParam::MEASURE_ATMOST ||
				param.m_Spec == MeasureParam::MEASURE_EXACT)
			{
				ParamForMeasure.m_Spec = MeasureParam::MEASURE_ATMOST;
				ParamForMeasure.m_nNum = max(0, param.m_nNum - 
					pLayoutParam->*pLayoutMarginLow - pLayoutParam->*pLayoutMarginHigh);
			}
			else
			{
				ParamForMeasure.m_Spec = MeasureParam::MEASURE_UNRESTRICTED;
				ParamForMeasure.m_nNum = 0;
			}
			break;
		default:
			ParamForMeasure.m_Spec = MeasureParam::MEASURE_EXACT;
			ParamForMeasure.m_nNum = max(0, pLayoutParam->*pnLayoutParamSize);
			break;
		}

		(pCur->*pfChildMeasureProc)(ParamForMeasure);

		nMaxSize = max(nMaxSize, 
			(pCur->*pfChildGetMeasurdProc)() + 
			pLayoutParam->*pLayoutMarginLow +
			pLayoutParam->*pLayoutMarginHigh);
	}

	switch (param.m_Spec)
	{
	case MeasureParam::MEASURE_EXACT:
		nMeasuredSize = param.m_nNum;
		break;
	default:
		nMeasuredSize = nMaxSize;
		if (param.m_Spec == MeasureParam::MEASURE_ATMOST)
			nMeasuredSize = min(nMeasuredSize, param.m_nNum);
		break;
	}

	for (INT i = 0; i < nFrameCount; i++)
	{
		CXFrame *pCur = GetFrameByIndex(i);
		if (!pCur)
		{
			ATLASSERT(NULL);
			continue;
		}

		LayoutParam *pLayoutParam = pCur->GetLayoutParam();
		if (!pLayoutParam)
		{
			ATLASSERT(NULL);
			continue;
		}

		if (pCur->GetVisibility() == VISIBILITY_NONE)
			continue;

		if (pLayoutParam->*pmLayoutParamSize 
			!= LayoutParam::METRIC_REACH_PARENT)
			continue;

		MeasureParam ParamForMeasure;
		ParamForMeasure.m_Spec = MeasureParam::MEASURE_EXACT;
		ParamForMeasure.m_nNum = max(0, 
			nMeasuredSize - pLayoutParam->*pLayoutMarginLow - pLayoutParam->*pLayoutMarginHigh);

		(pCur->*pfChildMeasureProc)(ParamForMeasure);
	}

	return TRUE;
}


BOOL CXDock::OnMeasureLayoutDirection( const MeasureParam & param, INT *pMeasuredSize, 
								BOOL (CXFrame::*pfChildMeasureProc)(const MeasureParam &), 
								INT (CXFrame::*pfChildGetMeasurdProc)(), 
								INT LayoutParam::*pLayoutParamPos, 
								INT LayoutParam::*pLayoutMarginLow,
								INT LayoutParam::*pLayoutMarginHigh,
								INT LayoutParam::*pnLayoutParamSize, 
								LayoutParam::SPECIAL_METRICS LayoutParam::*pmLayoutParamSize)
{
	if (!pMeasuredSize || !pfChildMeasureProc || !pfChildGetMeasurdProc ||
		!pLayoutMarginLow || !pLayoutMarginHigh ||
		!pLayoutParamPos || !pnLayoutParamSize || !pmLayoutParamSize)
	{
		ATLASSERT(NULL);
		return FALSE;
	}

	INT & nMeasuredSize = *pMeasuredSize;

	INT nCurrentPos = 0;

	INT nFrameCount = GetFrameCount();
	for (INT i = 0; i < nFrameCount; i++)
	{
		CXFrame *pCur = GetFrameByIndex(i);
		if (!pCur)
		{
			ATLASSERT(NULL);
			continue;
		}

		LayoutParam *pLayoutParam = pCur->GetLayoutParam();
		if (!pLayoutParam)
		{
			ATLASSERT(NULL);
			continue;
		}

		if (pCur->GetVisibility() == VISIBILITY_NONE)
			continue;

		MeasureParam ParamForMeasure; 

		switch (pLayoutParam->*pmLayoutParamSize)
		{
		case LayoutParam::METRIC_WRAP_CONTENT:
			if (param.m_Spec == MeasureParam::MEASURE_ATMOST)
			{
				ParamForMeasure.m_Spec = MeasureParam::MEASURE_ATMOST;
				ParamForMeasure.m_nNum = max(0, 
					param.m_nNum - nCurrentPos - 
					pLayoutParam->*pLayoutMarginLow - pLayoutParam->*pLayoutMarginHigh);
			}
			else
			{
				ParamForMeasure.m_Spec = MeasureParam::MEASURE_UNRESTRICTED;
				ParamForMeasure.m_nNum = 0;
			}
			break;
		case LayoutParam::METRIC_REACH_PARENT:
			ParamForMeasure.m_Spec = MeasureParam::MEASURE_EXACT;
			if (param.m_Spec == MeasureParam::MEASURE_EXACT)
				ParamForMeasure.m_nNum = max(0, 
					param.m_nNum - nCurrentPos - 
					pLayoutParam->*pLayoutMarginLow - pLayoutParam->*pLayoutMarginHigh);
			else
				ParamForMeasure.m_nNum = 0;
			break;
		default:
			ParamForMeasure.m_Spec = MeasureParam::MEASURE_EXACT;
			ParamForMeasure.m_nNum = max(0, pLayoutParam->*pnLayoutParamSize);
			break;
		}

		(pCur->*pfChildMeasureProc)(ParamForMeasure);

		nCurrentPos = nCurrentPos + (pCur->*pfChildGetMeasurdProc)() +
			pLayoutParam->*pLayoutMarginLow + pLayoutParam->*pLayoutMarginHigh;
	}

	switch (param.m_Spec)
	{
	case MeasureParam::MEASURE_EXACT:
		nMeasuredSize = param.m_nNum;
		break;
	default:
		nMeasuredSize = nCurrentPos;
		if (param.m_Spec == MeasureParam::MEASURE_ATMOST)
			nMeasuredSize = min(nMeasuredSize, param.m_nNum);
		break;
	}

	return TRUE;
}

BOOL CXDock::OnLayout( const CRect & rcRect )
{
	INT nDirection = 0;
	INT nCurrentPos = 0;
	LONG CRect::*pLayoutDirectionStart = NULL;
	LONG CRect::*pLayoutDirectionEnd = NULL;
	INT LayoutParam::*pLayoutDirectionMarginStart = NULL;
	INT LayoutParam::*pLayoutDirectionMarginEnd = NULL;
	LONG CRect::*pAlignDirectionLow = NULL;
	LONG CRect::*pAlignDirectionHigh = NULL;
	INT (CXFrame::*pfnLayoutDirectionSize)() = NULL;
	INT (CXFrame::*pfnAlignDirectionSize)() = NULL;
	INT LayoutParam::*pAlignDirectionMarginStart = NULL;
	INT LayoutParam::*pAlignDirectionMarginEnd = NULL;

	INT nAlignDirectionSize = 0;

	switch (m_DockType)
	{
	case DOCK_LEFT2RIGHT:
		nDirection = 1;
		nCurrentPos = 0;
		pLayoutDirectionStart = &CRect::left;
		pLayoutDirectionEnd = &CRect::right;
		pLayoutDirectionMarginStart = &LayoutParam::m_nMarginLeft;
		pLayoutDirectionMarginEnd = &LayoutParam::m_nMarginRight;
		pAlignDirectionMarginStart = &LayoutParam::m_nMarginTop;
		pAlignDirectionMarginEnd = &LayoutParam::m_nMarginBottom;
		pAlignDirectionLow = &CRect::top;
		pAlignDirectionHigh = &CRect::bottom;
		pfnLayoutDirectionSize = &CXFrame::GetMeasuredWidth;
		pfnAlignDirectionSize = &CXFrame::GetMeasuredHeight;
		nAlignDirectionSize = rcRect.Height();
		break;
	case DOCK_RIGHT2LEFT:
		nDirection = -1;
		nCurrentPos = GetMeasuredWidth();
		pLayoutDirectionStart = &CRect::right;
		pLayoutDirectionEnd = &CRect::left;
		pLayoutDirectionMarginStart = &LayoutParam::m_nMarginRight;
		pLayoutDirectionMarginEnd = &LayoutParam::m_nMarginLeft;
		pAlignDirectionMarginStart = &LayoutParam::m_nMarginTop;
		pAlignDirectionMarginEnd = &LayoutParam::m_nMarginBottom;
		pAlignDirectionLow = &CRect::top;
		pAlignDirectionHigh = &CRect::bottom;
		pfnLayoutDirectionSize = &CXFrame::GetMeasuredWidth;
		pfnAlignDirectionSize = &CXFrame::GetMeasuredHeight;
		nAlignDirectionSize = rcRect.Height();
		break;
	case DOCK_TOP2BOTTOM:
		nDirection = 1;
		nCurrentPos = 0;
		pLayoutDirectionStart = &CRect::top;
		pLayoutDirectionEnd = &CRect::bottom;
		pLayoutDirectionMarginStart = &LayoutParam::m_nMarginTop;
		pLayoutDirectionMarginEnd = &LayoutParam::m_nMarginBottom;
		pAlignDirectionMarginStart = &LayoutParam::m_nMarginLeft;
		pAlignDirectionMarginEnd = &LayoutParam::m_nMarginRight;
		pAlignDirectionLow = &CRect::left;
		pAlignDirectionHigh = &CRect::right;
		pfnLayoutDirectionSize = &CXFrame::GetMeasuredHeight;
		pfnAlignDirectionSize = &CXFrame::GetMeasuredWidth;
		nAlignDirectionSize = rcRect.Width();
		break;
	case DOCK_BOTTOM2TOP:
		nDirection = -1;
		nCurrentPos = GetMeasuredHeight();
		pLayoutDirectionStart = &CRect::bottom;
		pLayoutDirectionEnd = &CRect::top;
		pLayoutDirectionMarginStart = &LayoutParam::m_nMarginBottom;
		pLayoutDirectionMarginEnd = &LayoutParam::m_nMarginTop;
		pAlignDirectionMarginStart = &LayoutParam::m_nMarginLeft;
		pAlignDirectionMarginEnd = &LayoutParam::m_nMarginRight;
		pAlignDirectionLow = &CRect::left;
		pAlignDirectionHigh = &CRect::right;
		pfnLayoutDirectionSize = &CXFrame::GetMeasuredHeight;
		pfnAlignDirectionSize = &CXFrame::GetMeasuredWidth;
		nAlignDirectionSize = rcRect.Width();
		break;
	}

	INT nFrameCount = GetFrameCount();
	for (INT i = 0; i < nFrameCount; i++)
	{
		CXFrame *pCur = GetFrameByIndex(i);
		if (!pCur)
		{
			ATLASSERT(NULL);
			continue;
		}

		LayoutParam *pLayoutParam = pCur->GetLayoutParam();
		if (!pLayoutParam)
		{
			ATLASSERT(NULL);
			continue;
		}

		if (pCur->GetVisibility() == VISIBILITY_NONE)
			continue;

		CRect rc;
		rc.*pLayoutDirectionStart = 
			nCurrentPos + nDirection * pLayoutParam->*pLayoutDirectionMarginStart;
		rc.*pLayoutDirectionEnd = 
			rc.*pLayoutDirectionStart + nDirection * (pCur->*pfnLayoutDirectionSize)();
		nCurrentPos = rc.*pLayoutDirectionEnd + nDirection * pLayoutParam->*pLayoutDirectionMarginEnd;

		switch (m_Align)
		{
		case ALIGN_LOW:
			rc.*pAlignDirectionLow = pLayoutParam->*pAlignDirectionMarginStart;
			rc.*pAlignDirectionHigh = rc.*pAlignDirectionLow + (pCur->*pfnAlignDirectionSize)();
			break;

		case ALIGN_MIDDLE:
			{
				INT nSize = (pCur->*pfnAlignDirectionSize)();
				INT nSizeWithMargin = nSize + pLayoutParam->*pAlignDirectionMarginStart + pLayoutParam->*pAlignDirectionMarginEnd;
				rc.*pAlignDirectionLow = (nAlignDirectionSize - nSizeWithMargin) / 2 + pLayoutParam->*pAlignDirectionMarginStart;
				rc.*pAlignDirectionHigh = rc.*pAlignDirectionLow + nSize;
			}
			break;

		case ALIGN_HIGH:
			rc.*pAlignDirectionHigh = nAlignDirectionSize - pLayoutParam->*pAlignDirectionMarginEnd;
			rc.*pAlignDirectionLow = rc.*pAlignDirectionHigh - (pCur->*pfnAlignDirectionSize)();
			break;
		}

		pCur->Layout(rc);
	}



	return TRUE;
}

