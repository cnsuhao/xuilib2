#pragma once

#define SUPPORT_REMOTE_REFERENCE virtual public CRemoteRefSupport

#include <set>

class CRemoteRefBase
{
protected:
	CRemoteRefBase(BOOL bValid);

protected:
	BOOL m_bValid;

	friend class CRemoteRefSupport;
};

template<class T>
class CRemoteRef :
	public CRemoteRefBase
{
public:
	operator T* () const;
	T* operator->() const;

public:
	CRemoteRef(T * const & ptr);
	
public:
	CRemoteRef(const CRemoteRef<T> &other);
	CRemoteRef<T> & operator= (const CRemoteRef<T> & other);

public:
	bool operator< (const CRemoteRef<T> &other) const;

public:
	CRemoteRef();
	~CRemoteRef();

private:
	T* m_p;
};

class CRemoteRefSupport
{
public:
	~CRemoteRefSupport();

private:
	VOID RegisterRemoteRef(CRemoteRefBase *pRemoteRef);
	VOID UnregisterRemoteRef(CRemoteRefBase *pRemoteRef);

private:
	std::set<CRemoteRefBase *> m_setRemoteRef;

	template <class T> 
	friend class CRemoteRef;
};

#include "RemoteRef.inl"