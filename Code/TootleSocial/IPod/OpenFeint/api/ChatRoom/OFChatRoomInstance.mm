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
#import "OFChatRoomInstance.h"
#import "OFResourceDataMap.h"
#import "OFChatRoomInstanceService.h"
#import "OFChatRoomDefinition.h"

@implementation OFChatRoomInstance

@synthesize roomName;
@synthesize roomType;
@synthesize numUsersInRoom;
@synthesize maxNumUsersInRoom; 

- (void)setRoomName:(NSString*)value
{
	roomName = [value retain];
}

- (void)setRoomType:(NSString*)value
{
	roomType = [value retain];
}

- (void)setNumUsersInRoom:(NSString*)value
{
	numUsersInRoom = [value intValue];
}

- (void)setMaxNumUsersInRoom:(NSString*)value
{
	maxNumUsersInRoom = [value intValue];
}

+ (OFService*)getService;
{
	return [OFChatRoomInstanceService sharedInstance];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"room_name",					@selector(setRoomName:));
		dataMap->addField(@"num_users_in_room",			@selector(setNumUsersInRoom:));
		dataMap->addField(@"max_num_users_in_room",		@selector(setMaxNumUsersInRoom:));
		dataMap->addField(@"room_type",	@selector(setRoomType:));
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"chat_room_instance";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return @"openfeint_chat_room_instance_discovered";
}

- (BOOL)isDeveloperRoom
{
	return [roomType isEqualToString:[OFChatRoomDefinition getDeveloperRoomTypeId]];
}

- (BOOL)isGlobalRoom
{
	return [roomType isEqualToString:[OFChatRoomDefinition getGlobalRoomTypeId]];
}

- (BOOL)isApplicationRoom
{
	return [roomType isEqualToString:[OFChatRoomDefinition getApplicationRoomTypeId]];
}

- (void) dealloc
{
	[roomName release];
	[roomType release];
	[super dealloc];
}

@end