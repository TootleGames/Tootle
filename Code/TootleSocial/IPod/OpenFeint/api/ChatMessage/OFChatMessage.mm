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
#import "OFChatMessage.h"
#import "OFChatMessageService.h"
#import "OFResourceDataMap.h"
#import "OFChatMessageService.h"

@implementation OFChatMessage

@synthesize date;
@synthesize playerName;
@synthesize playerCurrentGameId;
@synthesize playerCurrentGame;
@synthesize message;
@synthesize playerId;
@synthesize playerProfilePictureUrl;
@synthesize playerUsesFacebookProfilePicture;
@synthesize playerCurrentGameIconUrl;
@synthesize doesLocalPlayerOwnGame;

- (void)setDate:(NSString*)value
{
	date = [value retain];
}

- (void)setPlayerName:(NSString*)value
{
	playerName = [value retain];
}

- (void)setPlayerCurrentGame:(NSString*)value
{
	playerCurrentGame = [value retain];
}

- (void)setPlayerCurrentGameId:(NSString*)value
{
	playerCurrentGameId = [value retain];
}

- (void)setMessage:(NSString*)value
{
	message = [value retain];
}

- (void)setPlayerId:(NSString*)value
{
	playerId = [value retain];
}

- (void)setPlayerProfilePictureUrl:(NSString*)value
{
	playerProfilePictureUrl = [value retain];
}

- (void)setPlayerUsesFacebookProfilePicture:(NSString*)value
{
	playerUsesFacebookProfilePicture = [value boolValue];
}

- (void)setPlayerCurrentGameIconUrl:(NSString*)value
{
	playerCurrentGameIconUrl = [value retain];
}

- (void)setDoesLocalPlayerOwnGame:(NSString*)value
{
	doesLocalPlayerOwnGame = [value boolValue];
}

+ (OFService*)getService;
{
	return [OFChatMessageService sharedInstance];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"player_name",							@selector(setPlayerName:));
		dataMap->addField(@"player_current_game_id",				@selector(setPlayerCurrentGameId:));
		dataMap->addField(@"player_current_game",					@selector(setPlayerCurrentGame:));
		dataMap->addField(@"message",								@selector(setMessage:));
		dataMap->addField(@"user_id",								@selector(setPlayerId:));	
		dataMap->addField(@"date",									@selector(setDate:));				
		dataMap->addField(@"player_profile_picture_url",			@selector(setPlayerProfilePictureUrl:));
		dataMap->addField(@"player_uses_facebook_profile_picture",	@selector(setPlayerUsesFacebookProfilePicture:));
		dataMap->addField(@"player_current_game_icon_url",			@selector(setPlayerCurrentGameIconUrl:));				
		dataMap->addField(@"local_user_owns_application",			@selector(setDoesLocalPlayerOwnGame:));				
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"chat_message";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return @"openfeint_chat_message_discovered";
}

- (void) dealloc
{
	OFSafeRelease(playerName);
	OFSafeRelease(playerId);
	OFSafeRelease(playerCurrentGameId);
	OFSafeRelease(playerCurrentGame);
	OFSafeRelease(message);
	OFSafeRelease(date);
	OFSafeRelease(playerProfilePictureUrl);
	OFSafeRelease(playerCurrentGameIconUrl);
	[super dealloc];
}

@end