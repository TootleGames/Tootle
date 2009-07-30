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


#import "OFService.h"

@class OFChatRoomDefinition;
@class OFChatRoomInstance;

@interface OFChatRoomInstanceService : OFService
{
	OFChatRoomInstance* mLastRoom;
	OFChatRoomInstance* mRoomJoining;
	BOOL mRejoiningRoom;
	CFAbsoluteTime mLastUpdateOfLastRoom;
}

@property (retain) OFChatRoomInstance* roomJoining;
@property (retain) OFChatRoomInstance* lastRoom;
@property (assign) BOOL rejoiningRoom;

OPENFEINT_DECLARE_AS_SERVICE(OFChatRoomInstanceService);

+ (void) getPage:(NSInteger)pageIndex forChatRoomDefinition:(OFChatRoomDefinition*)roomDefinition onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure;

+ (void) attemptToJoinRoom:(OFChatRoomInstance*)roomToJoin 
				 rejoining:(BOOL)rejoining 
				 onSuccess:(const OFDelegate&)onSuccess 
				 onFailure:(const OFDelegate&)onFailure;

+ (void) loadLastRoomJoined:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure;

+ (OFChatRoomInstance*) getCachedLastRoomJoined;
@end