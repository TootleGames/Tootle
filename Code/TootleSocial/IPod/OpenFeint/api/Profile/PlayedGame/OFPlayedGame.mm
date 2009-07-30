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
#import "OFPlayedGame.h"
#import "OFProfileService.h"
#import "OFResourceDataMap.h"

@implementation OFPlayedGame

@synthesize name, iconUrl, applicationId, userGamerscore, totalGamerscore, userAchievements, totalAchievements, userGameStats;

- (void)setName:(NSString*)value
{
	OFSafeRelease(name);
	name = [value retain];
}

- (void)setIconUrl:(NSString*)value
{
	OFSafeRelease(iconUrl);
	iconUrl = [value retain];
}

- (void)setApplicationId:(NSString*)value
{
	applicationId = [value intValue];
}

- (void)setUserGamerscore:(NSString*)value
{
	userGamerscore = [value intValue];
}

- (void)setUserGameStats:(NSMutableArray*)value
{
	OFSafeRelease(userGameStats);
	userGameStats = [value retain];
}

- (void)setTotalGamerscore:(NSString*)value
{
	totalGamerscore = [value intValue];
}

- (void)setUserAchievements:(NSString*)value
{
	userAchievements = [value intValue];
}

- (void)setTotalAchievements:(NSString*)value
{
	totalAchievements = [value intValue];
}

+ (OFService*)getService;
{
	return [OFProfileService sharedInstance];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"name", @selector(setName:));
		dataMap->addField(@"icon_url", @selector(setIconUrl:));
		dataMap->addField(@"client_application_id", @selector(setApplicationId:));
		dataMap->addField(@"user_gamerscore", @selector(setUserGamerscore:));
		dataMap->addField(@"total_gamerscore", @selector(setTotalGamerscore:));
		dataMap->addField(@"user_achievements", @selector(setUserAchievements:));
		dataMap->addField(@"total_achievements", @selector(setTotalAchievements:));
		dataMap->addNestedResourceArrayField(@"user_game_stats", @selector(setUserGameStats:));
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"played_game";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return @"openfeint_played_game_discovered";
}

- (void) dealloc
{
	self.iconUrl = nil;
	self.name = nil;
	self.userGameStats = nil;
	[super dealloc];
}

@end