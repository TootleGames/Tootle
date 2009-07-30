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

#pragma once

#include "OFBase.h"
#include "OFHashedString.h"
#include <map>

class OFInputSerializer;

class OFRTTIRepository
{
OFDeclareNonCopyable(OFRTTIRepository);

public:
	static OFRTTIRepository* Instance()
	{
		static std::auto_ptr<OFRTTIRepository> sInstance(0);
	
		if(sInstance.get() == NULL)
		{
			sInstance.reset(new OFRTTIRepository());
		}
		
		return sInstance.get();
	}


	void RegisterType(OFRTTI* type);
	
	const OFRTTI* getType(const char* name) const;
	const OFRTTI* getType(const OFSdbmHashedString& typeId) const;
	
	// WARNING: The returned memory is not managed.
	void* DeserializeObject(OFISerializer* stream, const OFSdbmHashedString& typeId) const;
	
private:	
	OFRTTIRepository();
	
	typedef std::map<OFSdbmHashedString, const OFRTTI*> TypeIdMap;
	
	TypeIdMap mTypeIds;
};