#pragma once

#include "..\Base\XFrame.h"

class CXScrollView :
	public CXFrame
{
public:
	CXScrollView::CXScrollView(void);

public:
	virtual BOOL OnMeasureWidth(const MeasureParam & param);
	virtual BOOL OnMeasureHeight(const MeasureParam & param);
	virtual BOOL OnLayout(const CRect & rcRect);
	virtual VOID Destroy();

private:
	VOID HandleMaxXY(INT nMaxX, INT nMaxY);

	BOOL OnMeasureLayoutDirection (
		const MeasureParam & param, INT *pMeasuredSize,
		BOOL (CXFrame::*pfChildMeasureProc)(const MeasureParam &),
		INT (CXFrame::*pfChildGetMeasurdProc)(),
		INT LayoutParam::*pLayoutParamPos, 
		INT LayoutParam::*pnLayoutParamSize, 
		LayoutParam::SPECIAL_METRICS LayoutParam::*pmLayoutParamSize,
		INT LayoutParam::*pLayoutMarginEnd);

private:
	INT m_nMaxX;
	INT m_nMaxY;
};
