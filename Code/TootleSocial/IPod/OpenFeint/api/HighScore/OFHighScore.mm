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
#import "OFHighScore.h"
#import "OFHighScoreService.h"
#import "OFResourceDataMap.h"
#import "OFUser.h"

@implementation OFHighScore

@synthesize user;
@synthesize score;
@synthesize rank;
@synthesize leaderboardId;

- (void)setUser:(OFUser*)value
{
	OFSafeRelease(user);
	user = [value retain];
}

- (void)setScore:(NSString*)value
{
	score = [value longLongValue];
}

- (void)setRank:(NSString*)value
{
	rank = [value intValue];
}

- (void)setLeaderboardId:(NSString*)value
{
	leaderboardId = [value integerValue];
}

+ (OFService*)getService;
{
	return [OFHighScoreService sharedInstance];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"score",			@selector(setScore:));
		dataMap->addField(@"rank",			@selector(setRank:));
		dataMap->addField(@"leaderboard_id", @selector(setLeaderboardId:));
		dataMap->addNestedResourceField(@"user", @selector(setUser:), [OFUser class]);
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"high_score";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return @"openfeint_high_score_discovered";
}

- (void) dealloc
{
	OFSafeRelease(user);
	[super dealloc];
}

@end
