#include "StdAfx.h"

#include "XFrameXMLFactory.h"

#include "..\..\..\XLib\inc\interfaceS\string\StringCode.h"
#include "..\..\..\XLib\inc\interfaceS\string\StringHelper.h"
#include "XFrame.h"
#include "../Draw/XImageGdiPlus.h"
#include "../Draw/XTextGdiPlus.h"


BOOL CXFrameXMLFactory::RegisterFrameType( const CHAR *pName, IXFrameXMLRuntimeInfo * pRuntimeInfo )
{
	if (!pName || !pRuntimeInfo)
		return FALSE;

	m_mapFrameNameToFactory.insert(std::make_pair(pName, pRuntimeInfo));
	return TRUE;
}

CXFrame * CXFrameXMLFactory::BuildFrame( X_XML_NODE_TYPE xml, CXFrame *pParent)
{
	if (!xml)
		return FALSE;

	char *pNodeName = xml->name();

	FRAME_FACTORY_MAP_TYPE::iterator it =
		m_mapFrameNameToFactory.find(pNodeName);

	if (it == m_mapFrameNameToFactory.end() || it->second == NULL)
	{
		CStringA strError;
		strError.Format("WARNING: No frame named %s. Skip this frame and its subframes. ", pNodeName);
		ReportError(pNodeName);
		return NULL;
	}

	CXFrame *pFrame = NULL;
	if (it->second)
	{
		X_FRAME_XML_FACTORY_TYPE factory = it->second->GetFrameXMLFactory();
		if (factory) pFrame = (*factory)(xml, pParent);
	}

	if (pFrame)
		pFrame->ConfigFrameByXML(xml);

	return pFrame;
}

BOOL CXFrameXMLFactory::ReportError( const CHAR *pError )
{
	if (!pError)
		return FALSE;

	::OutputDebugStringA(pError);
	return TRUE;
}

CXFrameXMLFactory & CXFrameXMLFactory::Instance()
{
	static CXFrameXMLFactory instance;
	return instance;
}

BOOL CXFrameXMLFactory::LoadXML( LPCTSTR szPath, X_XML_DOC_TYPE *pDocument)
{
	CStringA strError;

	if (!pDocument)
	{
		ReportError("ERROR: NULL document. ");
		return FALSE;
	}

	if (!szPath)
	{
		ReportError("ERROR: NULL path. ");
		return FALSE;
	}

	if (!::PathFileExists(szPath))
	{
#ifdef _UNICODE
		strError.Format("ERROR: Can't find the file %s. ", (LPCSTR)XLibS::StringCode::ConvertWideStrToAnsiStr(szPath));
#else
		strError.Format("ERROR: Can't find the file %s. ", szPath);
#endif
		ReportError(strError);

		return FALSE;
	}

	HANDLE hFile = ::CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (!hFile || hFile == INVALID_HANDLE_VALUE)
	{
#ifdef _UNICODE
		strError.Format("ERROR: Failed to open the file %s. ", (LPCSTR)XLibS::StringCode::ConvertWideStrToAnsiStr(szPath));
#else
		strError.Format("ERROR: Failed to open the file %s. ", szPath);
#endif
		ReportError(strError);

		return FALSE;
	}

	BOOL bResult = FALSE;
	
	DWORD dwFileSize = ::GetFileSize(hFile, NULL);
	if (dwFileSize)
	{
		CHAR *pBuffer = new CHAR[dwFileSize + 2]();
		DWORD dwRead = 0;
		if (::ReadFile(hFile, pBuffer, dwFileSize, &dwRead, NULL) && dwRead == dwFileSize)
		{
			try
			{
				pDocument->parse<0>(pBuffer);
				bResult = TRUE;
			}
			catch (const rapidxml::parse_error & error)
			{
				CHAR szErrorContext[50] = {};
				strncpy(szErrorContext, error.where<char>(), 48);
				
				strError.Format("ERROR: Parse failed. %s. ( %s @ %u ) ", error.what(), szErrorContext, 
					(int)(error.where<char>() - pBuffer));
			}
		}
		else
		{
#ifdef _UNICODE
			strError.Format("ERROR: Failed to read the file %s. ", (LPCSTR)XLibS::StringCode::ConvertWideStrToAnsiStr(szPath));
#else
			strError.Format("ERROR: Failed to read the file %s. ", szPath);
#endif
			ReportError(strError);
		}

	}

	::CloseHandle(hFile);
	return bResult;
}

VOID CXFrameXMLFactory::BuildImageList( std::vector<IXImage *> *pVecImageList, 
									   X_XML_NODE_TYPE xml, const char * pPathListName, const char * pTypeNameList /*= NULL*/ , const char *pDefaultType /*= NULL*/ , 
									   const char * pPartRectListPrefix /*= NULL*/ )
{
	if (!pVecImageList || !xml || !pPathListName)
		return;

	pVecImageList->clear();

	X_XML_ATTR_TYPE attr = NULL;

	std::vector<CStringA> vPathList;
	std::vector<CStringA> vTypeList;
	std::vector<CStringA> vPartLeft;
	std::vector<CStringA> vPartTop;
	std::vector<CStringA> vPartWidth;
	std::vector<CStringA> vPartHeight;

	attr = xml->first_attribute(pPathListName, 0, false);
	if (attr) XLibS::StringHelper::SplitStringA(attr->value(), "|", &vPathList);
	if (!vPathList.size())
		return;

	if (pTypeNameList && (attr = xml->first_attribute(pTypeNameList, 0, false)))
		XLibS::StringHelper::SplitStringA(attr->value(), "|", &vTypeList);

	CStringA strPartRectPrefix(pPartRectListPrefix);
	if (pPartRectListPrefix && (attr = xml->first_attribute(strPartRectPrefix + "left", 0, false)))
		XLibS::StringHelper::SplitStringA(attr->value(), "|", &vPartLeft);
	if (pPartRectListPrefix && (attr = xml->first_attribute(strPartRectPrefix + "top", 0, false)))
		XLibS::StringHelper::SplitStringA(attr->value(), "|", &vPartTop);
	if (pPartRectListPrefix && (attr = xml->first_attribute(strPartRectPrefix + "width", 0, false)))
		XLibS::StringHelper::SplitStringA(attr->value(), "|", &vPartWidth);
	if (pPartRectListPrefix && (attr = xml->first_attribute(strPartRectPrefix + "height", 0, false)))
		XLibS::StringHelper::SplitStringA(attr->value(), "|", &vPartHeight);

	for (std::vector<CStringA>::size_type i = 0; i < vPathList.size(); i++)
	{
		CXImageGdiPlus * pImage = new CXImageGdiPlus();
		if (!pImage->Load(MakePath(XLibSA2T(vPathList[i]))))
		{
			CStringA strError;
			strError.Format("WARNING: Load the file %s for the image failed. This image will be NULL in the image list. ", vPathList[i]);
			ReportError(strError);
			delete pImage;
			pVecImageList->push_back(NULL);
			continue;
		}

		const char *pType = "";
		if (i < vTypeList.size())
			pType = vTypeList[i];
		else if (!pImage->IsFormattedImage())
			pType = pDefaultType ? pDefaultType : "normal";

		if (!StrCmpIA(pType, "normal"))
			pImage->SetDrawType(IXImage::DIT_NORMAL);
		else if (!StrCmpIA(pType, "stretch"))
			pImage->SetDrawType(IXImage::DIT_STRETCH);
		else if (!StrCmpIA(pType, "center"))
			pImage->SetDrawType(IXImage::DIT_CENTER);
		else if (!StrCmpIA(pType, "3partH"))
			pImage->SetDrawType(IXImage::DIT_3PARTH);
		else if (!StrCmpIA(pType, "3partV"))
			pImage->SetDrawType(IXImage::DIT_3PARTV);
		else if (!StrCmpIA(pType, "9part"))
			pImage->SetDrawType(IXImage::DIT_9PART);

			
		CRect rcPartRect(0, 0, pImage->GetImageWidth(), pImage->GetImageHeight());
		BOOL bHasAttr = FALSE;
		if (i < vPartLeft.size()) { rcPartRect.left = atoi(vPartLeft[i]); bHasAttr = TRUE; }
		if (i < vPartTop.size()) { rcPartRect.top = atoi(vPartTop[i]); bHasAttr = TRUE; }
		if (i < vPartWidth.size()) { rcPartRect.right = rcPartRect.left + atoi(vPartWidth[i]); bHasAttr = TRUE; }
		if (i < vPartHeight.size()) { rcPartRect.bottom = rcPartRect.top + atoi(vPartHeight[i]); bHasAttr = TRUE; }
		if (bHasAttr)
			pImage->SetPartRect(rcPartRect);

		pVecImageList->push_back(pImage);
	}
}


IXImage * CXFrameXMLFactory::BuildImage( X_XML_NODE_TYPE xml, const char * pPathName, const char * pTypeName /*= NULL*/, const char * pDefaultType /*= NULL*/, 
										const char * pPartRectPrefix /*= NULL*/ )
{
	if (!xml || !pPathName)
		return NULL;

	X_XML_ATTR_TYPE attr = NULL;
	

	attr = xml->first_attribute(pPathName, 0, false);
	if (!attr)
		return NULL;

	CXImageGdiPlus * pImage = new CXImageGdiPlus();
	if (!pImage->Load(MakePath(XLibSA2T(attr->value()))))
	{
		CStringA strError;
		strError.Format("WARNING: Load the file %s for the image failed. Create the image failed. ", attr->value());
		ReportError(strError);
		delete pImage;
		return NULL;
	}

	const char *pType = "";
	if (pTypeName && (attr = xml->first_attribute(pTypeName, 0, false)))
		pType = attr->value();
	else if (!pImage->IsFormattedImage())
		pType = pDefaultType ? pDefaultType : "normal";

	if (!StrCmpIA(pType, "normal"))
		pImage->SetDrawType(IXImage::DIT_NORMAL);
	else if (!StrCmpIA(pType, "stretch"))
		pImage->SetDrawType(IXImage::DIT_STRETCH);
	else if (!StrCmpIA(pType, "center"))
		pImage->SetDrawType(IXImage::DIT_CENTER);
	else if (!StrCmpIA(pType, "3partH"))
		pImage->SetDrawType(IXImage::DIT_3PARTH);
	else if (!StrCmpIA(pType, "3partV"))
		pImage->SetDrawType(IXImage::DIT_3PARTV);
	else if (!StrCmpIA(pType, "9part"))
		pImage->SetDrawType(IXImage::DIT_9PART);

	if (pPartRectPrefix)
	{
		CRect rcPartRect(0, 0, pImage->GetImageWidth(), pImage->GetImageHeight());

		BOOL bHasAttr = FALSE;
		CStringA strPartRectPrefix(pPartRectPrefix);
		attr = xml->first_attribute(strPartRectPrefix + "left", 0, false);
		if (attr) { rcPartRect.left = atoi(attr->value()); bHasAttr = TRUE; }
		attr = xml->first_attribute(strPartRectPrefix + "top", 0, false);
		if (attr) { rcPartRect.top = atoi(attr->value()); bHasAttr = TRUE; }
		attr = xml->first_attribute(strPartRectPrefix + "width", 0, false);
		if (attr) { rcPartRect.right = rcPartRect.left + atoi(attr->value()); bHasAttr = TRUE; }
		attr = xml->first_attribute(strPartRectPrefix + "height", 0, false);
		if (attr) { rcPartRect.bottom = rcPartRect.top + atoi(attr->value()); bHasAttr = TRUE; }

		if (bHasAttr)
			pImage->SetPartRect(rcPartRect);
	}

	return pImage;
}

CRect CXFrameXMLFactory::BuildRect( X_XML_NODE_TYPE xml, const char * pRectPrefix /*= NULL*/ )
{
	CRect rtn(0, 0, 0, 0);

	if (!xml)
		return rtn;

	X_XML_ATTR_TYPE attr = NULL;

	CStringA strRectPrefix;
	if (pRectPrefix)
		strRectPrefix = pRectPrefix;

	attr = xml->first_attribute(strRectPrefix + "left", 0, false);
	if (attr) rtn.left = atoi(attr->value());
	attr = xml->first_attribute(strRectPrefix + "top", 0, false);
	if (attr) rtn.top = atoi(attr->value());
	attr = xml->first_attribute(strRectPrefix + "width", 0, false);
	if (attr) rtn.right = rtn.left + atoi(attr->value());
	attr = xml->first_attribute(strRectPrefix + "height", 0, false);
	if (attr) rtn.bottom = rtn.top + atoi(attr->value());

	return rtn;
}

IXText * CXFrameXMLFactory::BuildText( X_XML_NODE_TYPE xml, const char * pTextPrefix /*= NULL*/ )
{
	if (!xml)
		return NULL;

	CStringA strTextPrefix;
	X_XML_ATTR_TYPE attr = NULL;

	if (pTextPrefix)
		strTextPrefix = pTextPrefix;

	CXTextGdiPlus *pText = new CXTextGdiPlus();
	
	attr = xml->first_attribute(strTextPrefix + "text", 0, false);
	if (attr)
#ifdef _UNICODE
		pText->SetText(XLibS::StringCode::ConvertAnsiStrToWideStr(attr->value()));
#else
		pText->SetText(attr->value());
#endif

	const char *pFontName = "Arial";
	INT nFontSize = 12;
	Gdiplus::FontStyle fsFontStyle = Gdiplus::FontStyleRegular;

	attr = xml->first_attribute(strTextPrefix + "font", 0, false);
	if (attr) pFontName = attr->value();
	attr = xml->first_attribute(strTextPrefix + "font_size", 0, false);
	if (attr) nFontSize = atoi(attr->value());
	attr = xml->first_attribute(strTextPrefix + "font_style", 0, false);
	if (attr)
	{
		const char *pValue = attr->value();
		if (StrStrIA(pValue, "bold"))
			fsFontStyle = (Gdiplus::FontStyle)(fsFontStyle | Gdiplus::FontStyleBold);
		if (StrStrIA(pValue, "italic"))
			fsFontStyle = (Gdiplus::FontStyle)(fsFontStyle | Gdiplus::FontStyleItalic);
		if (StrStrIA(pValue, "strikeout"))
			fsFontStyle = (Gdiplus::FontStyle)(fsFontStyle | Gdiplus::FontStyleStrikeout);
		if (StrStrIA(pValue, "underline"))
			fsFontStyle = (Gdiplus::FontStyle)(fsFontStyle | Gdiplus::FontStyleUnderline);;
	}
#ifdef _UNICODE
	pText->SetFont(XLibS::StringCode::ConvertAnsiStrToWideStr(pFontName), nFontSize, fsFontStyle);
#else
	pText->SetFont(pFontName, nFontSize, fsFontStyle);
#endif

	Gdiplus::StringAlignment align_h = Gdiplus::StringAlignmentNear;
	Gdiplus::StringAlignment align_v = Gdiplus::StringAlignmentNear;

	attr = xml->first_attribute(strTextPrefix + "text_align_h", 0, false);
	if (attr)
	{
		const char *pValue = attr->value();
		if (!StrCmpIA(pValue, "center"))
			align_h = Gdiplus::StringAlignmentCenter;
		if (!StrCmpIA(pValue, "right"))
			align_h = Gdiplus::StringAlignmentFar;
	}
	attr = xml->first_attribute(strTextPrefix + "text_align_v", 0, false);
	if (attr)
	{
		const char *pValue = attr->value();
		if (!StrCmpIA(pValue, "middle"))
			align_v = Gdiplus::StringAlignmentCenter;
		if (!StrCmpIA(pValue, "bottom"))
			align_v = Gdiplus::StringAlignmentFar;
	}
	pText->SetAlignment(align_h, align_v);

	attr = xml->first_attribute(strTextPrefix + "text_color", 0, false);
	if (attr)
	{
		UINT32 color = strtoul(attr->value(), NULL, 16);
		pText->SetColor(color >> 24, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
	}
	
	return pText;
}

CString CXFrameXMLFactory::MakePath( LPCTSTR pPath )
{
	if (!pPath)
		return _T("");

	if (pPath[0] == _T('@'))
		return pPath;

	if (::PathIsRelative(pPath))
		return SMCRelativePath(pPath);

	return pPath;
}

