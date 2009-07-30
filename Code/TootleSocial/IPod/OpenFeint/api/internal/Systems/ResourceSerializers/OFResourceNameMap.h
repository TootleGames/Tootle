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

#import "OFRetainedPtr.h"
#import "OFSmartObject.h"

class OFResourceNameMap : public OFSmartObject
{
public:
	struct ResourceDescription
	{
		OFRetainedPtr<NSString> resourceName;
		Class klass;
		
		bool operator==(NSString* nameString) const
		{
			return [resourceName.get() isEqualToString:nameString];
		}
	};
	
	void addResource(NSString* name, Class klass);
	Class getTypeNamed(NSString* name) const;
	
private:
	typedef std::vector<ResourceDescription> ResourceDescriptionSeries;
	ResourceDescriptionSeries mResources;
};