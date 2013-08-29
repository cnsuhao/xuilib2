#pragma once 

#include "../../../XLib/Lib/include/RapidXml/rapidxml.hpp"
#include "../Draw/XDraw.h"

#include <map>
#include <vector>

class CXFrame;

typedef rapidxml::xml_node<> * X_XML_NODE_TYPE;
typedef rapidxml::xml_attribute<> * X_XML_ATTR_TYPE;
typedef rapidxml::xml_document<>  X_XML_DOC_TYPE;
typedef CXFrame * (* X_FRAME_XML_FACTORY_TYPE)(X_XML_NODE_TYPE, CXFrame *);
typedef rapidxml::node_type X_XML_NODE_CATEGORY_TYPE;

class CXFrameXMLFactory_CompareRawString
{
public:
	bool operator()(const CHAR * const &l, const CHAR * const &r) const
	{
		return StrCmpA(l, r) < 0;
	}
};

class IXFrameXMLRuntimeInfo
{
public:
	virtual X_FRAME_XML_FACTORY_TYPE GetFrameXMLFactory() = 0;
};

class CXFrameXMLFactory
{
public:
	static CXFrameXMLFactory& Instance();

public:
	static BOOL ReportError(const CHAR *pError);
	static BOOL LoadXML(LPCTSTR szPath, X_XML_DOC_TYPE *pDocument);

public:
	BOOL RegisterFrameType(const CHAR *pName, IXFrameXMLRuntimeInfo *pRuntimeInfo);
	CXFrame * BuildFrame(X_XML_NODE_TYPE xml, CXFrame *pParent);

public:
	static CString MakePath(LPCTSTR pPath);
	static IXImage * BuildImage(X_XML_NODE_TYPE xml, const char * pPathName, const char * pTypeName = NULL, const char * pDefaultType = NULL, 
		const char * pPartRectPrefix = NULL);
	static VOID BuildImageList(std::vector<IXImage *> *pVecImageList, X_XML_NODE_TYPE xml, const char * pPathListName, const char * pTypeListName = NULL, const char *pDefaultType = NULL,
		const char * pPartRectListPrefix = NULL);
	static CRect BuildRect(X_XML_NODE_TYPE xml, const char * pRectPrefix = NULL);
	static IXText * BuildText(X_XML_NODE_TYPE xml, const char * pTextPrefix = NULL);

private:
	typedef std::map<const CHAR *, IXFrameXMLRuntimeInfo *, CXFrameXMLFactory_CompareRawString> FRAME_FACTORY_MAP_TYPE; 

private:
	FRAME_FACTORY_MAP_TYPE m_mapFrameNameToFactory;
};

#define X_DECLEAR_FRAME_XML_RUNTIME(classname, name)													\
																										\
private:																								\
																										\
	class CXFrameXMLRuntime_##classname																	\
		: public IXFrameXMLRuntimeInfo																	\
	{																									\
	public:																								\
		CXFrameXMLRuntime_##classname()																	\
		{																								\
			CXFrameXMLFactory::Instance()																\
				.RegisterFrameType(#name, this);														\
		}																								\
																										\
		virtual X_FRAME_XML_FACTORY_TYPE GetFrameXMLFactory()											\
		{																								\
			return &classname::CreateFrameFromXML;														\
		}																								\
																										\
	};																									\
																										\
	static CXFrameXMLRuntime_##classname s_xml_runtime_info;											\
																										\
public:																									\
	static CXFrame * CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent);
	

#define X_IMPLEMENT_FRAME_XML_RUNTIME(classname)														\
																										\
classname::CXFrameXMLRuntime_##classname classname::s_xml_runtime_info;


#define X_IMPLEMENT_DEFAULT_FRAME_XML_FACTORY(classname)												\
																										\
CXFrame * classname::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)							\
{																										\
	if (!xml)																							\
		return FALSE;																					\
																										\
	LayoutParam *pLayout = pParent ?																	\
		pParent->GenerateLayoutParam(xml) : new CXFrame::LayoutParam(xml);								\
	if (!pLayout)																						\
	{																									\
		CStringA strError;																				\
		strError.Format("WARNING: Generating the layout parameter for the parent %s failed. \\			\
			Building the frame failed. ", XLibST2A(pParent->GetName()));								\
		CXFrameXMLFactory::ReportError(strError);														\
		return NULL;																					\
	}																									\
																										\
	classname *pFrame = new classname();																\
	pFrame->Create(pParent, pLayout, VISIBILITY_NONE);													\
																										\
	return pFrame;																						\
}