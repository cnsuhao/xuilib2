#include "stdafx.h"

#include "..\..\..\XLib\inc\interfaceS\string\StringCode.h"
#include "..\Draw\XImageGdiPlus.h"
#include "XResourceMgr.h"

#define PACKED_RESOURCE_EXT _T(".XP")

BOOL CXResourceMgr::AddSearchPath( LPCTSTR pPath )
{
	if (!pPath)
		return FALSE;

	CString strPath(GetNormalizedPath(pPath));

	::PathRemoveBackslash(strPath.GetBuffer());
	strPath.ReleaseBuffer();

	for (std::vector<CString>::size_type i = 0; i < m_vSearchPath.size(); i++)
	{
		if (m_vSearchPath[i].CompareNoCase(strPath) == 0)
			return TRUE;
	}

	if (!PathFileExists(strPath))
		return FALSE;

	if (!StrCmpI(PathFindExtension(strPath), PACKED_RESOURCE_EXT)
		&& !PathIsDirectory(strPath))
	{
		IZipUnpack * pUnpack = NULL;
		if (!SMCCreateInstance(Util::GetXLibPath(), SMC_IDOF(IZipUnpack), (ISMCInterface**)&pUnpack) || !pUnpack)
			return FALSE;
		if (!pUnpack->Open(strPath))
			return FALSE;

		m_mapPackage.insert(std::make_pair(strPath, pUnpack));

		pUnpack->Release();
	}

	m_vSearchPath.push_back(strPath);

	return TRUE;
}

CString CXResourceMgr::GetResourcePath( LPCTSTR pRelativePath )
{
	if (!pRelativePath)
		return _T("");

	CString strRelativePath(GetNormalizedPath(pRelativePath));

	if (!::PathIsRelative(strRelativePath))
		return _T("");

	CString strRst;

	for (std::vector<CString>::size_type i = 0; 
		i < m_vSearchPath.size(); i++)
	{
		if (!StrCmpI(PathFindExtension(m_vSearchPath[i]), PACKED_RESOURCE_EXT)
			&& !PathIsDirectory(m_vSearchPath[i]))
		{
			std::map<CString, CSMCPtr<IZipUnpack>, CXResourceMgrPackageMapCompareCStringNoCase>::const_iterator it
				= m_mapPackage.find(m_vSearchPath[i]);
			if (it == m_mapPackage.end() || !it->second)
			{
				ATLASSERT(NULL);
				continue;
			}

			if (!it->second->IsFileExists(XLibST2W(pRelativePath)))
				continue;

			strRst = m_vSearchPath[i];
			::PathAppend(strRst.GetBuffer(MAX_PATH), strRelativePath);
			strRst.ReleaseBuffer();
			break;
		}

		strRst = m_vSearchPath[i];
		::PathAppend(strRst.GetBuffer(MAX_PATH), strRelativePath);
		strRst.ReleaseBuffer();

		if (!PathFileExists(strRst))
		{
			strRst.Empty();
			continue;
		}

		break;
	}

	return strRst;
}

BOOL CXResourceMgr::IsPackedPath( LPCTSTR pFullPath )
{
	if (!pFullPath)
		return FALSE;

	return FindPackageRelativePathFromFullPath(pFullPath) != NULL;

}

CString CXResourceMgr::GetNormalizedPath(LPCTSTR pPath)
{
	if (!pPath)
		return _T("");

	CString strRtn(pPath);
	strRtn.Replace(_T("/"), _T("\\"));

	return strRtn;
}

LPCTSTR CXResourceMgr::FindPackageRelativePathFromFullPath( LPCTSTR pPath )
{
 	if (!pPath)
 		return NULL;
 
 	LPCTSTR pIndex = NULL;
 	if ((pIndex = StrStrI(pPath, PACKED_RESOURCE_EXT)) == NULL)
 		return NULL;
 
 	LPCTSTR pRelativePath =  pIndex + _tcslen(PACKED_RESOURCE_EXT);
 	if (*pRelativePath != '\\' && *pRelativePath != '/')
 		return NULL;

	pRelativePath++;
 
 	CString strPackagePath(pPath, pRelativePath - pPath - 1);

	if (!::PathFileExists(strPackagePath) || ::PathIsDirectory(strPackagePath))
		return NULL;
 
 	return pRelativePath;
}

LPCTSTR CXResourceMgr::FindPackageRelativePathFromFullPathAndSeprateThem( LPTSTR pPath)
{
	if (!pPath)
		return NULL;

	LPTSTR pRelativePath = (LPTSTR)FindPackageRelativePathFromFullPath(pPath);

	if (!pRelativePath)
		return NULL;

	*(pRelativePath - 1) = '\0';

	return pRelativePath;
}

SIZE_T CXResourceMgr::GetPackedResourceSize( LPCTSTR pFullPath )
{
	if (!pFullPath)
		return 0;

	CString strFullPath(pFullPath);
	LPTSTR pNonConstFullPath = strFullPath.GetBuffer();

	SIZE_T szRtn = 0;
	do 
	{
		LPCTSTR pPackagePath = pNonConstFullPath;
		LPCTSTR pRelativePath = NULL;
		if (!(pRelativePath = FindPackageRelativePathFromFullPathAndSeprateThem(pNonConstFullPath)))
			break;

		std::map<CString, CSMCPtr<IZipUnpack>, CXResourceMgrPackageMapCompareCStringNoCase>::const_iterator it
			= m_mapPackage.find(pPackagePath);
		if (it == m_mapPackage.end())
			break;
		if (!it->second) {ATLASSERT(NULL); break;}
		
		szRtn = it->second->GetFileSize(XLibST2W(pRelativePath));
		
	} while (FALSE);

	strFullPath.ReleaseBuffer();

	return szRtn;
}

BOOL CXResourceMgr::GetPackedResource( LPCTSTR pFullPath, BYTE *pBuffer, SIZE_T size )
{
	if (!pFullPath || !pBuffer || !size)
		return FALSE;

	CString strFullPath(pFullPath);
	LPTSTR pNonConstFullPath = strFullPath.GetBuffer();

	BOOL bRtn = FALSE;
	do 
	{
		LPCTSTR pPackagePath = pNonConstFullPath;
		LPCTSTR pRelativePath = NULL;
		if (!(pRelativePath = FindPackageRelativePathFromFullPathAndSeprateThem(pNonConstFullPath)))
			break;

		std::map<CString, CSMCPtr<IZipUnpack>, CXResourceMgrPackageMapCompareCStringNoCase>::const_iterator it
			= m_mapPackage.find(pPackagePath);
		if (it == m_mapPackage.end())
			break;
		if (!it->second) {ATLASSERT(NULL); break;}

		bRtn = it->second->UnpackFile(XLibST2W(pRelativePath), pBuffer, size);

	} while (FALSE);

	strFullPath.ReleaseBuffer();

	return bRtn;
}

CXResourceMgr & CXResourceMgr::Instance()
{
	static CXResourceMgr instance;
	return instance;
}

CXResourceMgr::CXResourceMgr()
{
	AddSearchPath(SMCRelativePath(_T("XUILib.xp")));
}

IXImage * CXResourceMgr::GetImage( LPCTSTR pRelativePath )
{
	if (!pRelativePath || !::PathIsRelative(pRelativePath))
		return NULL;

	CXImageGdiPlus *pImage = new CXImageGdiPlus();
	if (!pImage->Load(CString(_T("@")) + pRelativePath))
	{
		delete pImage;
		pImage = NULL;
	}

	return pImage;
}

VOID CXResourceMgr::OnExit()
{
	m_vSearchPath.clear();
	m_mapPackage.clear();
}