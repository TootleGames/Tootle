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
#ifndef OF_SMART_OBJECT_H
#define OF_SMART_OBJECT_H

#include "OFRTTI.h"

//////////////////////////////////////////////////////////////////////////
/// An interface by which objects can be self-managed through the
/// use of reference counting and auto-pointers to wrap the reference
/// counting. 
///
/// @note	To derive a new such object, one would have to inherit from
///			OFSmartObject, as well as declare and implement the RTTI system 
///			either by hand or using the predefined macros OFDeclareRTTI and 
///			OFImplementRTTI. 
///
/// @note	To avoid the requirement of calling AddRef/Release
///			upon creation/deletion of a pointer you may also make use of 
///			the	onSmartPointer class.
///
/// @note	RTTI implementation is optional but recommended.
///
/// @note	Use the onSharedPtr type for non-intrusive shared pointers.
///
/// @remarks	Prefer the onIntrusivePtr datatype for declarations of
///				shared pointers for smart objects.
//////////////////////////////////////////////////////////////////////////

class OFSmartObject
{
	OFDeclareRootRTTI
public:
	virtual ~OFSmartObject();

	int GetObjectID();

	void AddRef();
	void AddRef() const;
	
	void Release();
	void Release() const;

	int RefCount() const;

	bool unique() const;

protected:
	OFSmartObject();

	mutable int	m_iRefCount;
};

#include "OFSmartObject.inl"

#endif