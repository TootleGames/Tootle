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

#import "OFDependencies.h"
#import "OFResourceControllerMap.h"
#import <algorithm>

void OFResourceControllerMap::addResource(Class klass, NSString* controllerName)
{
	ResourceDescription desc;
	desc.klass = klass;
	desc.controllerName = controllerName;
	mResources.push_back(desc);
}

NSString* OFResourceControllerMap::getControllerName(Class lookupClass) const
{
	ResourceDescriptionSeries::const_iterator sit = std::find(mResources.begin(), mResources.end(), lookupClass);
	if(sit == mResources.end())
	{
		return nil;
	}
	
	return sit->controllerName;
}
