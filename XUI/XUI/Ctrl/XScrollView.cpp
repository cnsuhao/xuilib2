#include "StdAfx.h"
#include "XScrollView.h"

CXScrollView::CXScrollView(void)
	: m_nMaxX(0), m_nMaxY(0)
{
}

VOID CXScrollView::Destroy()
{
	__super::Destroy();
	m_nMaxX = m_nMaxY = 0;
}

VOID CXScrollView::HandleMaxXY( INT nMaxX, INT nMaxY )
{
	if (m_nMaxX == nMaxX &&
		m_nMaxY == nMaxY)
		return;

	CSize szOld(m_nMaxX, m_nMaxY);
	CSize szNew(nMaxX, nMaxY);
	m_nMaxX = nMaxX;
	m_nMaxY = nMaxY;

	ThrowEvent(EVENT_SCROLLVIEW_BOUND_CHANGED,
		(WPARAM)&szOld, (LPARAM)&szNew);
	
}



BOOL CXScrollView::OnMeasureWidth( const MeasureParam & param )
{
	INT nMeasuredWidth = 0;

	BOOL bRtn = OnMeasureLayoutDirection(param, &nMeasuredWidth, 
		&CXFrame::MeasureWidth, &CXFrame::GetMeasuredWidth, 
		&LayoutParam::m_nX, &LayoutParam::m_nWidth, &LayoutParam::m_mWidth,
		&LayoutParam::m_nMarginRight);

	SetMeasuredWidth(nMeasuredWidth);

	return bRtn;
}

BOOL CXScrollView::OnMeasureHeight( const MeasureParam & param )
{
	INT nMeasuedHeight = 0;

	BOOL bRtn = OnMeasureLayoutDirection(param, &nMeasuedHeight,
		&CXFrame::MeasureHeight, &CXFrame::GetMeasuredHeight, 
		&LayoutParam::m_nY, &LayoutParam::m_nHeight, &LayoutParam::m_mHeight,
		&LayoutParam::m_nMarginBottom);

	SetMeasuredHeight(nMeasuedHeight);

	return bRtn;
}

BOOL CXScrollView::OnMeasureLayoutDirection( const MeasureParam & param, INT *pMeasuredSize, 
									   BOOL (CXFrame::*pfChildMeasureProc)(const MeasureParam &), 
									   INT (CXFrame::*pfChildGetMeasurdProc)(),
									   INT LayoutParam::*pLayoutParamPos, 
									   INT LayoutParam::*pnLayoutParamSize, 
									   LayoutParam::SPECIAL_METRICS LayoutParam::*pmLayoutParamSize,
									   INT LayoutParam::*pLayoutMarginEnd)
{
	if (!pMeasuredSize || !pfChildMeasureProc || !pfChildGetMeasurdProc ||
		!pLayoutParamPos || !pnLayoutParamSize || !pmLayoutParamSize || !pLayoutMarginEnd)
	{
		ATLASSERT(NULL);
		return FALSE;
	}

	INT & nMeasuredSize = *pMeasuredSize;
	if (param.m_Spec == MeasureParam::MEASURE_EXACT)
		nMeasuredSize  = param.m_nNum;

	INT nFrameCount = GetFrameCount();
	for (INT i = 0; i < nFrameCount; i++)
	{
		CXFrame *pCur = GetFrameByIndex(i);

		if (!pCur)
		{
			ATLASSERT(NULL);
			continue;
		}

		if (!pCur->NeedLayout())
			continue;

		LayoutParam *pLayoutParam = pCur->GetLayoutParam();
		if (!pLayoutParam)
		{
			ATLASSERT(NULL);
			continue;
		}		

		MeasureParam ParamForMeasure; 

		switch (pLayoutParam->*pmLayoutParamSize)
		{
		case LayoutParam::METRIC_WRAP_CONTENT:
			ParamForMeasure.m_Spec = MeasureParam::MEASURE_UNRESTRICTED;
			ParamForMeasure.m_nNum = 0;
			break;
		case LayoutParam::METRIC_REACH_PARENT:
			ParamForMeasure.m_Spec = MeasureParam::MEASURE_EXACT;
			ParamForMeasure.m_nNum = max(0,
				nMeasuredSize - pLayoutParam->*pLayoutParamPos - pLayoutParam->*pLayoutMarginEnd);
			break;
		default:
			ParamForMeasure.m_Spec = MeasureParam::MEASURE_EXACT;
			ParamForMeasure.m_nNum = max(0, pLayoutParam->*pnLayoutParamSize);
			break;
		}

		(pCur->*pfChildMeasureProc)(ParamForMeasure);
	}

	return TRUE;
}

BOOL CXScrollView::OnLayout( const CRect & rcRect )
{
	INT nMaxX = 0, nMaxY = 0;

	INT nFrameCount = GetFrameCount();
	for (INT i = 0; i < nFrameCount; i++)
	{
		CXFrame *pCur = GetFrameByIndex(i);

		if (!pCur)
		{
			ATLASSERT(NULL);
			continue;
		}
		if (!pCur->NeedLayout())
			continue;

		LayoutParam *pLayoutParam = pCur->GetLayoutParam();
		if (!pLayoutParam)
		{
			ATLASSERT(NULL);
			continue;
		}

		CRect rc(pLayoutParam->m_nX, pLayoutParam->m_nY,
			pLayoutParam->m_nX + pCur->GetMeasuredWidth(),
			pLayoutParam->m_nY + pCur->GetMeasuredHeight());

		nMaxX = max(nMaxX, rc.right);
		nMaxY = max(nMaxY, rc.bottom);

		pCur->Layout(rc);
	}

	HandleMaxXY(nMaxX, nMaxY);

	return TRUE;
}