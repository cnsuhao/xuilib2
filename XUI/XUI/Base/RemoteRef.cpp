#include "stdafx.h"
#include "RemoteRef.h"


CRemoteRefBase::CRemoteRefBase( BOOL bValid )
	: m_bValid(bValid)
{
}

CRemoteRefSupport::~CRemoteRefSupport()
{
	for (std::set<CRemoteRefBase *>::const_iterator it = m_setRemoteRef.begin();
		it != m_setRemoteRef.end(); it++)
	{
		if (*it)
			// Not to change m_p to NULL, instead, we make 
			// m_bValid FALSE. That makes sure that the operator< of 
			// CRemoteRef is stable. 
			(*it)->m_bValid = FALSE;
	}
	m_setRemoteRef.clear();
}

VOID CRemoteRefSupport::RegisterRemoteRef( CRemoteRefBase *pRemoteRef )
{
	if (!pRemoteRef)
		return;

	m_setRemoteRef.insert(pRemoteRef);
}

VOID CRemoteRefSupport::UnregisterRemoteRef( CRemoteRefBase *pRemoteRef )
{
	if (!pRemoteRef)
		return;

	m_setRemoteRef.erase(pRemoteRef);
}