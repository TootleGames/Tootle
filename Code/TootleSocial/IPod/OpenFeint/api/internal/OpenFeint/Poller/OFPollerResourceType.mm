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
#import "OFPollerResourceType.h"
#import "OFResource.h"

@implementation OFPollerResourceType

@synthesize lastSeenId = mLastSeenId;
@synthesize name = mName;
@synthesize idParameterName = mIdParameterName;
@synthesize discoveryNotification = mDiscoveryNotification;
@synthesize newResources = mNewResources;

- (id)initWithName:(NSString*)name andDiscoveryNotification:(NSString*)discoveryNotification
{
	if(self = [super init])
	{
		self.name = name;
		self.idParameterName = [NSString stringWithFormat:@"last_seen_%@_id", self.name];
		self.discoveryNotification = discoveryNotification;
		self.newResources = [NSMutableArray array];		
	}
	return self;
}

- (void) dealloc
{
	self.newResources = nil;
	self.discoveryNotification = nil;
	self.name = nil;
	self.idParameterName = nil;
	[super dealloc];
}

- (void)addResource:(OFResource*)resource
{
	long long resourceId = [resource.resourceId longLongValue];

	if(resourceId > mLastSeenId)
	{
		mLastSeenId = resourceId;
	}
	
	[mNewResources addObject:resource];
}

- (void)markNewResourcesOld
{
	[mNewResources removeAllObjects];
}

- (void)clearLastSeenId
{
	mLastSeenId = 0;
}

@end