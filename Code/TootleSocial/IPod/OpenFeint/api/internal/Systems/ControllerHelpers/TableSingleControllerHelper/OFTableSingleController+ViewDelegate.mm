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
#import "OFTableControllerHelper+Overridables.h"
#import "OFTableSingleControllerHelper.h"
#import "OFTableSingleController+ViewDelegate.h"
#import "OFControllerLoader.h"
#import "OFTableCellHelper.h"
#import <objc/runtime.h>
#import "OFTableSectionDescription.h"
#import "OFTableSectionCellDescription.h"

@implementation OFTableSingleControllerHelper (ViewDelegate)

- (OFResource*) getResourceFromSection:(NSArray*)sectionCells atRow:(NSUInteger)row
{
	OFTableSectionCellDescription* cellDescription = (OFTableSectionCellDescription*)[sectionCells objectAtIndex:row];	
	return cellDescription.resource;
}

- (NSString*) getCellControllerNameFromSection:(NSArray*)sectionCells atRow:(NSUInteger)row
{
	OFTableSectionCellDescription* cellDescription = (OFTableSectionCellDescription*)[sectionCells objectAtIndex:row];	
	return cellDescription.controllerName;
}

@end