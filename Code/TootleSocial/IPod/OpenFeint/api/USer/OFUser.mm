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
#import "OFUser.h"
#import "OFResourceDataMap.h"
#import "OpenFeint+UserOptions.h"

@implementation OFUser

@synthesize name;
@synthesize profilePictureUrl;
@synthesize usesFacebookProfilePicture;
@synthesize lastPlayedGameId;
@synthesize lastPlayedGameName;
@synthesize gamerScore;

- (void)setName:(NSString*)value
{
	OFSafeRelease(name);
	name = [value retain];
}

- (void)setProfilePictureUrl:(NSString*)value
{
	OFSafeRelease(profilePictureUrl);
	if (![value isEqualToString:@""])
	{
		profilePictureUrl = [value retain];
	}
}

- (void)setUsesFacebookProfilePicture:(NSString*)value
{
	usesFacebookProfilePicture = [value boolValue];
}

- (void)setLastPlayedGameId:(NSString*)value
{
	lastPlayedGameId = [value intValue];
}

- (void)setLastPlayedGameName:(NSString*)value
{
	OFSafeRelease(lastPlayedGameName);
	lastPlayedGameName = [value retain];
}

- (void)setGamerScore:(NSString*)value
{
	gamerScore = [value intValue];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"name",							@selector(setName:));
		dataMap->addField(@"profile_picture_url",			@selector(setProfilePictureUrl:));
		dataMap->addField(@"uses_facebook_profile_picture",	@selector(setUsesFacebookProfilePicture:));
		dataMap->addField(@"last_played_game_id",			@selector(setLastPlayedGameId:));
		dataMap->addField(@"last_played_game_name",			@selector(setLastPlayedGameName:));
		dataMap->addField(@"gamer_score",					@selector(setGamerScore:));
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"user";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return nil;
}

- (bool)isLocalUser
{
	return [self.resourceId isEqualToString:[OpenFeint lastLoggedInUserId]];
}

- (void) dealloc
{
	OFSafeRelease(name);
	OFSafeRelease(profilePictureUrl);
	OFSafeRelease(lastPlayedGameName);
	[super dealloc];
}

@end