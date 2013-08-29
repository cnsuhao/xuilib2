#include "stdafx.h"

#include "../Base/XFrameXMLFactory.h"
#include "XSysControl.h"
#include "../../../XLib/inc/interfaceS/string/StringCode.h"

CXFrame * XFrameXMLFactoryForSysControl(X_XML_NODE_TYPE xml, CXFrame *pParent)
{
	X_XML_ATTR_TYPE attr = NULL;

	CXFrame::LayoutParam *pLayout = pParent ?
		pParent->GenerateLayoutParam(xml) : new CXFrame::LayoutParam(xml);
	if (!pLayout)
	{
		CStringA strError;
		strError.Format("WARNING: Generating the layout parameter for the parent %s failed. \
						Building the frame failed. ", XLibST2A(pParent->GetName()));
		CXFrameXMLFactory::ReportError(strError);
		return NULL;
	}

	CRect rcFrame(0, 0, 0, 0);

	CStringA strError;

	attr = xml->first_attribute("type", 0, false);
	if (!attr)
	{
		CXFrameXMLFactory::ReportError("WARNING: No type specified for the system control. Create the system control failed. ");
		return NULL;
	}
	const char *pType = attr->value();

	if (!StrCmpIA(pType, "edit"))
	{
		CXSysControl<CEdit> *pFrame = new CXSysControl<CEdit>;
		pFrame->Create(pParent, pLayout, CXFrame::VISIBILITY_NONE);
		return pFrame;
	}
	else if (FALSE)
	{
		// .....
	}

	strError.Format("WARNING: Type (%s) not supported for a system control. Create the system control failed. ", pType);
	return NULL;
}

class CXFrameXMLRuntimeInfo_CXSysControl :
	public IXFrameXMLRuntimeInfo
{
public:
	virtual X_FRAME_XML_FACTORY_TYPE GetFrameXMLFactory()
	{
		return &XFrameXMLFactoryForSysControl;
	}

	CXFrameXMLRuntimeInfo_CXSysControl()
	{
		CXFrameXMLFactory::Instance().RegisterFrameType("XSysControl", this);
	}
};

CXFrameXMLRuntimeInfo_CXSysControl s_xml_runtime_info_cxsyscontrol;