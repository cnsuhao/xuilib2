#include "StdAfx.h"
#include "XScrollContentContainer.h"

CXScrollContentContainer::CXScrollContentContainer(void)
{
}

CXScrollContentContainer::~CXScrollContentContainer(void)
{
}

BOOL CXScrollContentContainer::OnMeasureWidth( 
	const MeasureParam & param )
{
	MeasureParam ModifiedParam(param);

	if (ModifiedParam.m_Spec == MeasureParam::MEASURE_ATMOST)
		ModifiedParam.m_Spec = MeasureParam::MEASURE_UNRESTRICTED;

	return __super::OnMeasureWidth(ModifiedParam);
}

BOOL CXScrollContentContainer::OnMeasureHeight( const MeasureParam & param )
{
	MeasureParam ModifiedParam(param);

	if (ModifiedParam.m_Spec == MeasureParam::MEASURE_ATMOST)
		ModifiedParam.m_Spec = MeasureParam::MEASURE_UNRESTRICTED;

	return __super::OnMeasureHeight(ModifiedParam);

}
