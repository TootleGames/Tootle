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

#include "OFRTTIRepository.h"
#include "OFRTTI.h"

// The serializer micro-architecture should be moved into Common
#include "OFISerializer.h"

OFRTTIRepository::OFRTTIRepository()
{
}

void OFRTTIRepository::RegisterType(OFRTTI* type)
{
	mTypeIds.insert(TypeIdMap::value_type(type->GetTypeId(), type));
}

void* OFRTTIRepository::DeserializeObject(OFISerializer* stream, const OFSdbmHashedString& typeId) const
{
	const OFRTTI* type = getType(typeId);
	if(type == NULL)
	{
		return NULL;
	}
	
	return type->DeserializeObject(stream);
}

const OFRTTI* OFRTTIRepository::getType(const char* name) const
{
	return getType(OFSdbmHashedString(name));
}

const OFRTTI* OFRTTIRepository::getType(const OFSdbmHashedString& typeId) const
{
	TypeIdMap::const_iterator sit = mTypeIds.find(typeId);
	if(sit == mTypeIds.end())
	{
		OFAssert(0, "This is probably an error in your code. Did you setup your hierarchy's RTTI properly?"); 
		return NULL;
	}
	
	return sit->second;
}