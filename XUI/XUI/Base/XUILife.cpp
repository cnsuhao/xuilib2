#include "StdAfx.h"
#include "XUILife.h"

#include "XResourceMgr.h"

XUILife::XUILife(void)
{
	OnInit();
}

XUILife::~XUILife(void)
{
	OnExit();
}

VOID XUILife::OnInit()
{
	return;
}

VOID XUILife::OnExit()
{
	CXResourceMgr::Instance().OnExit();

	SMCReleaseAllDLL();
}
