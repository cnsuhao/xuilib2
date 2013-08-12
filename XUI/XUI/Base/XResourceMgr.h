#pragma once

#include <vector>
#include "..\..\..\XLib\inc\interface\file\zip\ZipUnpack.h"
#include "..\Draw\XDraw.h"

class CXResourceMgrPackageMapCompareCStringNoCase
{
public:
	bool operator() (const CString & a, const CString & b) const
	{
		return a.CompareNoCase(b) < 0;
	}
};

class CXResourceMgr
{
public:
	BOOL AddSearchPath(LPCTSTR pPath);

public:
	CString GetResourcePath(LPCTSTR pRelativePath);
	BOOL IsPackedPath(LPCTSTR pFullPath);
	SIZE_T GetPackedResourceSize(LPCTSTR pFullPath);
	BOOL GetPackedResource(LPCTSTR pFullPath, BYTE *pBuffer, SIZE_T size);

public:
	static IXImage * GetImage(LPCTSTR pRelativePath);

public:
	static CXResourceMgr & Instance();

private:
	CString GetNormalizedPath(LPCTSTR);
	LPCTSTR FindPackageRelativePathFromFullPath(LPCTSTR);
	LPCTSTR FindPackageRelativePathFromFullPathAndSeprateThem(LPTSTR);

private:
	friend class XUILife;
	VOID OnExit();

private:
	CXResourceMgr();

private:
	std::vector<CString> m_vSearchPath;
	std::map<CString, CSMCPtr<IZipUnpack>, CXResourceMgrPackageMapCompareCStringNoCase> m_mapPackage;
};