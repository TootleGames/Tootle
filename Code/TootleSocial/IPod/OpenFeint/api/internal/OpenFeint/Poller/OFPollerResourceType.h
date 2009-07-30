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

@class OFResource;

@interface OFPollerResourceType : NSObject
{
@private
	NSMutableArray* mNewResources;
	long long mLastSeenId;
	NSString* mName;
	NSString* mIdParameterName;
	NSString* mDiscoveryNotification;
}

@property (nonatomic, retain) NSString* name;
@property (nonatomic, assign) long long lastSeenId;
@property (nonatomic, retain) NSString* idParameterName;
@property (nonatomic, retain) NSString* discoveryNotification;
@property (nonatomic, retain) NSArray* newResources;

- (id)initWithName:(NSString*)name andDiscoveryNotification:(NSString*)discoveryNotification;
- (void)addResource:(OFResource*)resource;
- (void)markNewResourcesOld;
- (void)clearLastSeenId;

@end