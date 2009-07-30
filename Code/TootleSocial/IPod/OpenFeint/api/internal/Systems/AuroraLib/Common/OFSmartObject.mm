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

#include "OFPointer.h"

OFImplementRootRTTI(OFSmartObject)

// OFSmartObject
OFSmartObject::OFSmartObject()
{
	m_iRefCount = 0;
}

// ~OFSmartObject
OFSmartObject::~OFSmartObject()
{
	m_iRefCount = 0;
}

void OFSmartObject::Release() const
{
	OFAssert(m_iRefCount > 0, "If this fails, there is a serious problem. This should never happen!");

	if (--m_iRefCount == 0)
	{
		delete this;
	}
}

// Release
void OFSmartObject::Release()
{
	OFAssert(m_iRefCount > 0, "If this fails, there is a serious problem. This should never happen!");

	if (--m_iRefCount == 0)
	{
		delete this;
	}
}