#pragma once

#include "..\Base\XFrame.h"


class CXScrollContentContainer : public CXFrame
{

public:
	virtual BOOL OnMeasureWidth(const MeasureParam & param);
	virtual BOOL OnMeasureHeight(const MeasureParam & param);

public:
	CXScrollContentContainer(void);
	~CXScrollContentContainer(void);
};
