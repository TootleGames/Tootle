////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Copyright (c) 2009 Aurora Feint Inc.
///
///  This library is free software; you can redistribute it and/or
///  modify it under the terms of the GNU Lesser General Public
///  License as published by the Free Software Foundation; either
///  
///  version 3 of the License, or (at your option) any later version.
///  
///  This library is distributed in the hope that it will be useful,
///  
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
///  Lesser General Public License for more details.
///  
///  
///  You should have received a copy of the GNU Lesser General Public
///  License along with this library; if not, write to the Free Software
///  
///  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef OF_POINTER_H
#define OF_POINTER_H

#include "OFBase.h"

//////////////////////////////////////////////////////////////////////////
/// OFPointer is a templated class which wraps the functionality of a
/// pointer and automates reference counting of OFSmartObject derived
/// objects.
//////////////////////////////////////////////////////////////////////////
template <class T>
class OFPointer
{
public:
	/// Ctors and Dtors
	OFPointer(T* pObj = 0);
	OFPointer(const OFPointer& pPtr);
	~OFPointer();

	template <class D>
	explicit OFPointer(const OFPointer<D>& pPtr);

	template <class D>
	explicit OFPointer(OFPointer<D>& pPtr);

	/// Assigment
	OFPointer& operator=(const OFPointer& pPtr);

	/// @TODO Remove this!
	OFPointer& operator=(T* pObj);

	// Assignment from one smart pointer type to another
	template <class D>
	OFPointer<T>& operator=(OFPointer<D>& pPtr);

	/// Equality
	bool operator==(const OFPointer& pPtr) const;
	bool operator==(T* pObj) const;

	/// Inequality
	bool operator!=(const OFPointer& pPtr) const;
	bool operator!=(T* pObj) const;

	// Pointer access
	T* get();
	const T* get() const;

	// Resets the smart pointer
	void reset(T* ptr);

	/// Implicit conversions
	/// @TODO Remove this!
	operator T*() const;

	operator bool() const;

	T* operator->() const;
	T& operator*() const;

	template <class D>
	bool operator<(const OFPointer<D>& Ptr) const;

private:
	/// Actual object pointer
	T*	m_pObject;
};

template <class T> template <class D>
inline OFPointer<T>::OFPointer(const OFPointer<D>& pPtr)
{
	m_pObject = static_cast<const T*>(pPtr.get());

	if(m_pObject)
		m_pObject->AddRef();
}

template <class T> template <class D>
inline OFPointer<T>::OFPointer(OFPointer<D>& pPtr)
{
	m_pObject = static_cast<T*>(pPtr.get());

	if(m_pObject)
		m_pObject->AddRef();
}

template <class T> template<class D>
inline OFPointer<T>& OFPointer<T>::operator=(OFPointer<D>& pPtr)
{
	if(pPtr.get() != m_pObject)
	{
		if(m_pObject)
			m_pObject->Release();

		m_pObject = static_cast<T*>(pPtr.get());

		if(m_pObject)
			m_pObject->AddRef();
	}

	return *this;
}

template <class T>
inline T* OFPointer<T>::get()
{
	return m_pObject;
}

template <class T>
inline const T* OFPointer<T>::get() const
{
	return m_pObject;
}

template <class T>
inline void OFPointer<T>::reset(T* ptr)
{
	if(ptr != m_pObject)
	{
		if(m_pObject)
			m_pObject->Release();

		m_pObject = ptr;

		if(m_pObject)
			m_pObject->AddRef();
	}
}

template <class T>
inline OFPointer<T>::OFPointer(T* pObj) :
	m_pObject(pObj)
{
	if (m_pObject)
		m_pObject->AddRef();
}

template <class T>
inline OFPointer<T>::OFPointer(const OFPointer<T>& pPtr) :
	m_pObject(pPtr.m_pObject)
{
	if (m_pObject)
		m_pObject->AddRef();
}

template <class T>
inline OFPointer<T>::~OFPointer()
{
	if (m_pObject)
		m_pObject->Release();
}

template <class T>
inline OFPointer<T>& OFPointer<T>::operator=(const OFPointer<T>& pPtr)
{
	if(m_pObject != pPtr.m_pObject)
	{
		if (m_pObject)
			m_pObject->Release();

		m_pObject = pPtr.m_pObject;

		if (m_pObject)
			m_pObject->AddRef();
	}

	return *this;
}

template <class T>
inline OFPointer<T>& OFPointer<T>::operator=(T* pObj)
{
	if (m_pObject != pObj)
	{
		if (m_pObject)
			m_pObject->Release();

		m_pObject = pObj;

		if (m_pObject)
			m_pObject->AddRef();
	}

	return *this;
}

template <class T>
inline bool OFPointer<T>::operator==(const OFPointer<T>& pPtr) const
{
	return m_pObject == pPtr.m_pObject;
}

template <class T>
inline bool OFPointer<T>::operator==(T* pObj) const
{
	return m_pObject == pObj;
}

template <class T>
inline bool OFPointer<T>::operator!=(const OFPointer<T>& pPtr) const
{
	return m_pObject != pPtr.m_pObject;
}

template <class T>
inline bool OFPointer<T>::operator!=(T* pObj) const
{
	return m_pObject != pObj;
}

template <class T>
inline OFPointer<T>::operator T*() const
{
	return m_pObject;
}

template <class T>
inline OFPointer<T>::operator bool() const
{
	return m_pObject != 0;
}

template <class T>
inline T* OFPointer<T>::operator->() const
{
	return m_pObject;
}

template <class T>
inline T& OFPointer<T>::operator*() const
{
	return *m_pObject;
}

template <class T> template <class D>
inline bool OFPointer<T>::operator<(const OFPointer<D>& Ptr) const
{
	return m_pObject < Ptr.m_pObject;
}

#endif