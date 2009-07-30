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
#import "OFTableSingleControllerHelper.h"
#import "OFTableSingleControllerHelper+Overridables.h"
#import "OFService+Overridables.h"
#import "OFPaginatedSeries.h"

@implementation OFTableSingleControllerHelper

@synthesize resourceId;

- (void)_onDataLoaded:(OFPaginatedSeries*)resources isIncremental:(BOOL)isIncremental
{
	OFAssert(isIncremental == false, "Incremental loading is not supported yet for single controllers");
	
	OFSafeRelease(mSections);
	
	if([resources count])
	{
		mSections = [[self tableSectionDescriptionsForResource:(OFResource*)[resources objectAtIndex:0]] retain];
	}
	
	[self.tableView reloadData];
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	OFAssert(self.resourceId != nil, "Must specify a resource for the TableSingleController to display");
	OFService* service = [self getService];
	[[service class] getShowWithId:self.resourceId onSuccess:success onFailure:failure];
}

- (void)dealloc
{
	self.resourceId = nil;
	[super dealloc];
}

@end
