template<class T>
CRemoteRef<T>::operator T*() const
{
	if (m_bValid)
		return m_p;

	return NULL;
}


template<class T>
T* CRemoteRef<T>::operator->() const
{
	if (m_bValid)
		return m_p;

	return NULL;
}

template<class T>
CRemoteRef<T>::CRemoteRef( T * const & ptr )
	: CRemoteRefBase(TRUE),
	m_p(ptr)
{
	if (ptr)
		ptr->RegisterRemoteRef(this);
}

template<class T>
CRemoteRef<T>::CRemoteRef( const CRemoteRef<T> &other )
	: CRemoteRefBase(other.m_bValid),
	m_p(other.m_p)
	
{
	if (other.m_bValid)
		if (other.m_p)
			other.m_p->RegisterRemoteRef(this);
}

template<class T>
CRemoteRef<T> & CRemoteRef<T>::operator=( const CRemoteRef<T> & other)
{
	if (m_bValid == other.m_bValid && m_p == other.m_p)
		return *this;

	if (m_bValid && m_p)
		m_p->UnregisterRemoteRef(this);

	m_bValid = other.m_bValid;
	m_p = other.m_p;
	
	if (other.m_bValid)
		if (other.m_p)
			other.m_p->RegisterRemoteRef(this);

	return *this;
}

template<class T>
bool CRemoteRef<T>::operator<( const CRemoteRef<T> &other ) const
{
	return m_p < other.m_p;
}

template<class T>
CRemoteRef<T>::CRemoteRef()
	: CRemoteRefBase(TRUE),
	m_p(NULL)
{
}

template<class T>
CRemoteRef<T>::~CRemoteRef()
{
	if (m_bValid && m_p)
		m_p->UnregisterRemoteRef(this);
}