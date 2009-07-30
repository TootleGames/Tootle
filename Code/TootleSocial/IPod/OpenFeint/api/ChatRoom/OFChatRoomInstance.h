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

#import "OFResource.h"

@class OFService;

@interface OFChatRoomInstance : OFResource
{
	@package
	NSString* roomName;
	NSString* roomType;
	NSInteger numUsersInRoom;
	NSInteger maxNumUsersInRoom;
}

+ (OFResourceDataMap*)getDataMap;
+ (OFService*)getService;
+ (NSString*)getResourceName;
+ (NSString*)getResourceDiscoveredNotification;

- (BOOL)isDeveloperRoom;
- (BOOL)isGlobalRoom;
- (BOOL)isApplicationRoom;

@property (nonatomic, readonly) NSString* roomName;
@property (nonatomic, readonly) NSString* roomType;
@property (nonatomic, readonly) NSInteger numUsersInRoom;
@property (nonatomic, readonly) NSInteger maxNumUsersInRoom;

@end