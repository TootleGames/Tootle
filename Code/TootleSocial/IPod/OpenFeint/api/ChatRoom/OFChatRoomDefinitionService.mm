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
#import "OFChatRoomDefinitionService.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFService+Private.h"
#import "OFChatRoomDefinition.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFChatRoomDefinitionService);

@implementation OFChatRoomDefinitionService

OPENFEINT_DEFINE_SERVICE(OFChatRoomDefinitionService);

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFChatRoomDefinition getResourceName], [OFChatRoomDefinition class]);
}

+ (void) getIndexOnSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	[OFChatRoomDefinitionService getPage:1 includeGlobalRooms:true includeDeveloperRooms:true includeApplicationRooms:true onSuccess:onSuccess onFailure:onFailure];
}

+ (void) getPage:(NSInteger)pageIndex 
includeGlobalRooms:(bool)includeGlobalRooms 
includeDeveloperRooms:(bool)includeDeveloperRooms 
includeApplicationRooms:(bool)includeApplicationRooms 
	   onSuccess:(const OFDelegate&)onSuccess 
	   onFailure:(const OFDelegate&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	params->io("include_global_rooms", includeGlobalRooms);
	params->io("include_application_rooms", includeApplicationRooms);
	params->io("include_developer_rooms", includeDeveloperRooms);
	
	[[self sharedInstance] 
	 getAction:@"chat_room_definitions.xml"
	 withParameters:params
	 withSuccess:onSuccess
	 withFailure:onFailure
	 withRequestType:OFActionRequestForeground
	 withNotice:@"Downloading Chat Rooms"];
}

@end
