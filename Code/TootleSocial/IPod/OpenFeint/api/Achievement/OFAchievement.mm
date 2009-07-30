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
#import "OFAchievement.h"
#import "OFAchievementService.h"
#import "OFResourceDataMap.h"

@implementation OFAchievement

@synthesize title, description, gamerscore, iconUrl, isSecret, isUnlocked, isUnlockedByComparedToUser, comparedToUserId, unlockDate;

- (void)setTitle:(NSString*)value
{
	OFSafeRelease(title);
	title = [value retain];
}

- (void)setDescription:(NSString*)value
{
	OFSafeRelease(description);
	description = [value retain];
}

- (void)setGamerscore:(NSString*)value
{
	gamerscore = [value intValue];
}

- (void)setIconUrl:(NSString*)value
{
	OFSafeRelease(iconUrl);
	iconUrl = [value retain];
}

- (void)setIsSecret:(NSString*)value
{
	isSecret = [value boolValue];
}

- (void)setIsUnlocked:(NSString*)value
{
	isUnlocked = [value boolValue];
}

- (void)setIsUnlockedByComparedToUser:(NSString*)value
{
	isUnlockedByComparedToUser = [value boolValue];
}

- (void)setComparedToUserId:(NSString*)value
{
	comparedToUserId = [value retain];
}

- (void)setUnlockDate:(NSString*)value
{
	OFSafeRelease(unlockDate);
	
	if (value != nil)
	{
		NSDateFormatter* dateFormatter = [[[NSDateFormatter alloc] init]  autorelease];
		[dateFormatter setDateFormat:@"yyy-MM-dd HH:mm:ss zzz"];
		unlockDate = [[dateFormatter dateFromString:value] retain];
	}
}

+ (OFService*)getService;
{
	return [OFAchievementService sharedInstance];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"title", @selector(setTitle:));
		dataMap->addField(@"description", @selector(setDescription:));
		dataMap->addField(@"gamerscore", @selector(setGamerscore:));
		dataMap->addField(@"icon_url", @selector(setIconUrl:));
		dataMap->addField(@"is_secret", @selector(setIsSecret:));
		dataMap->addField(@"is_unlocked", @selector(setIsUnlocked:));
		dataMap->addField(@"is_unlocked_by_compared_to_user", @selector(setIsUnlockedByComparedToUser:));
		dataMap->addField(@"compared_to_user_id", @selector(setComparedToUserId:));
		dataMap->addField(@"unlock_date", @selector(setUnlockDate:));
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"achievement";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return @"openfeint_achievement_discovered";
}

- (void) dealloc
{
	self.title = nil;
	self.description = nil;
	self.iconUrl = nil;
	self.unlockDate = nil;
	self.comparedToUserId = nil;
	[super dealloc];
}

@end
