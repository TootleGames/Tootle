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

#import "OFAchievementService.h"
#import "OFAchievementService+Private.h"
#import "OFSqlQuery.h"
#import "OFReachability.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFActionRequestType.h"
#import "OFService+Private.h"
#import "OpenFeint+Private.h"
#import "OpenFeint+UserOptions.h"
#import <sqlite3.h>

namespace
{
	static OFSqlQuery sUnlockQuery;
	static OFSqlQuery sPendingUnlocksQuery;
	static OFSqlQuery sDeleteRowQuery;
}

@implementation OFAchievementService (Private)

- (id) init
{
	self = [super init];
	
	if (self != nil)
	{
		[OFAchievementService setupOfflineSupport];
	}
	
	return self;
}

- (void) dealloc
{
	sUnlockQuery.destroyQueryNow();
	sPendingUnlocksQuery.destroyQueryNow();
	sDeleteRowQuery.destroyQueryNow();
	[super dealloc];
}

+ (void) setupOfflineSupport
{
	OFSqlQuery(
		[OpenFeint getOfflineDatabaseHandle],
		 "CREATE TABLE IF NOT EXISTS unlocked_achievements("
		 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
		 "achievement_definition_id INTEGER NOT NULL,"
		 "user_id INTEGER NOT NULL,"
		 "UNIQUE(achievement_definition_id, user_id))"
		 ).execute();	

	OFSqlQuery(
		 [OpenFeint getOfflineDatabaseHandle], 
		 "CREATE UNIQUE INDEX IF NOT EXISTS unlocked_achievements_index "
		 "ON unlocked_achievements (achievement_definition_id, user_id)"
		 ).execute();

	sUnlockQuery.reset(
		[OpenFeint getOfflineDatabaseHandle], 
		"INSERT INTO unlocked_achievements "
		"(achievement_definition_id, user_id) "
		"VALUES(:achievement_definition_id, :user_id)"
	);
	
	sPendingUnlocksQuery.reset(
		[OpenFeint getOfflineDatabaseHandle],
		"SELECT achievement_definition_id, user_id "
		"FROM unlocked_achievements "
		"WHERE user_id = :user_id"
	);
	
	sDeleteRowQuery.reset(
		[OpenFeint getOfflineDatabaseHandle],
		"DELETE FROM unlocked_achievements "
		"WHERE user_id = :user_id AND "
		"achievement_definition_id = :achievement_definition_id"
	);
}

- (void) _successfullyCommittedAchievementsWithIgnoredResources:(NSArray*)resources andAchievements:(NSArray*)achievementIdList
{
	NSString* lastLoggedInUser = [OpenFeint lastLoggedInUserId];
	sDeleteRowQuery.bind("user_id", lastLoggedInUser);

	for (NSString* achievementId in achievementIdList)
	{
		sDeleteRowQuery.bind("achievement_definition_id", achievementId);
		sDeleteRowQuery.execute();
	}
}

- (void) userDidLogIn:(NSString*)userId;
{
	NSString* lastLoggedInUser = [OpenFeint lastLoggedInUserId];
	if (OFReachability::Instance()->isGameServerReachable() && [lastLoggedInUser longLongValue] > 0)
	{
		NSMutableArray* achievementIdList = [NSMutableArray new];
		
		sPendingUnlocksQuery.bind("user_id", lastLoggedInUser);
		for (sPendingUnlocksQuery.execute(); !sPendingUnlocksQuery.hasReachedEnd(); sPendingUnlocksQuery.step())
		{
			NSString* achievementId = [NSString stringWithFormat:@"%d", sPendingUnlocksQuery.getInt("achievement_definition_id")];
			[achievementIdList addObject:achievementId];
		}

		if ([achievementIdList count] > 0)
		{
			[OFAchievementService 
				unlockAchievements:achievementIdList 
				onSuccess:OFDelegate(
					[OFAchievementService sharedInstance], 
					@selector(_successfullyCommittedAchievementsWithIgnoredResources:andAchievements:), 
					achievementIdList) 
				onFailure:OFDelegate()];
		}
	}
}

+ (bool) offlineUnlockAchievement:(NSString*)achievementId forUser:(NSString*)userId
{
	sUnlockQuery.bind("achievement_definition_id", achievementId);
	sUnlockQuery.bind("user_id", userId);		
	sUnlockQuery.execute();
	bool success = (sUnlockQuery.getLastStepResult() == SQLITE_OK);
	sUnlockQuery.resetQuery();
	return success;
}

+ (void) unlockAchievements:(NSArray*)achievementIdList onSuccess:(OFDelegate const&)onSuccess onFailure:(OFDelegate const&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;

	{
		OFISerializer::Scope high_score(params, "achievement_list", true);

		for (NSString* achievementId in achievementIdList)
		{
			OFISerializer::Scope high_score(params, "achievement");
			OFRetainedPtr<NSString> resourceId = achievementId;
			params->io("achievement_definition_id", resourceId);		
		}
	}
	
	[[self sharedInstance] 
		postAction:@"users/@me/unlocked_achievements.xml"
		withParameters:params
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestSilent
		withNotice:@"Submitting Unlocked Achivements"];
}

@end