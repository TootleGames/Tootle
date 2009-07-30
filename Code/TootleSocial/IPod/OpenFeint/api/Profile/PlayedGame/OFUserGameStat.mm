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
#import "OFUserGameStat.h"
#import "OFProfileService.h"
#import "OFResourceDataMap.h"

@implementation OFUserGameStat

@synthesize userHasGame, userId, userGamerScore;

- (void)setUserHasGame:(NSString*)value
{
	userHasGame = [value boolValue];
}

- (void)setUserId:(NSString*)value
{
	OFSafeRelease(userId);
	userId = [value retain];
}

- (void)setUserGamerscore:(NSString*)value
{
	userGamerScore = [value intValue];
}

+ (OFService*)getService;
{
	return nil;
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"user_has_game", @selector(setUserHasGame:));
		dataMap->addField(@"user_id", @selector(setUserId:));
		dataMap->addField(@"user_gamerscore", @selector(setUserGamerscore:));
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"user_game_stat";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return @"openfeint_user_game_stat_discovered";
}

- (void) dealloc
{
	self.userId = nil;
	[super dealloc];
}

@end