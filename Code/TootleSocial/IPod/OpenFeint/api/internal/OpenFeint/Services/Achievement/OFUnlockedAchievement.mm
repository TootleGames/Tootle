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
#import "OFUnlockedAchievement.h"
#import "OFAchievementService.h"
#import "OFResourceDataMap.h"
#import "OFAchievement.h"

@implementation OFUnlockedAchievement

@synthesize achievement, result;

- (void)setResult:(NSString*)value
{
	if ([value isEqualToString:@"invalid"])
		result = kUnlockResult_Invalid;
	else if ([value isEqualToString:@"already_unlocked"])
		result = kUnlockResult_Duplicate;
	else if ([value isEqualToString:@"unlocked"])
		result = kUnlockResult_Success;
}

- (void)setAchievement:(OFResource*)value
{
	achievement = (OFAchievement*)[value retain];
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
		dataMap->addNestedResourceField(@"achievement_definition", @selector(setAchievement:), [OFAchievement class]);
		dataMap->addField(@"result", @selector(setResult:));
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"unlocked_achievement";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return nil;
}

- (void) dealloc
{
	OFSafeRelease(achievement);
	[super dealloc];
}

@end