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

#import "OFNowPlayingController.h"
#import "OFNowPlayingOptionVirtualResource.h"

#import "OFApplicationDescription.h"
#import "OFApplicationDescriptionService.h"

#import "OFTableSectionDescription.h"
#import "OFTableSectionCellDescription.h"

#import "OFSelectChatRoomDefinitionController.h"
#import "OFLeaderboardController.h"
#import "OFAchievementListController.h"
#import "OFWhosPlayingController.h"
#import "OFControllerLoader.h"
#import "OFViewHelper.h"
#import "OFImageView.h"
#import "OFProfileFramedView.h"

#import "OpenFeint+Private.h"
#import "OpenFeint+Settings.h"
#import "OpenFeint+UserOptions.h"

@implementation OFNowPlayingController

- (void)dealloc
{
	[super dealloc];
}

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFNowPlayingOptionVirtualResource class], @"NowPlaying");
}

- (OFService*)getService
{
	return nil;
}

- (NSString*)getNoDataFoundMessage
{
	return @"No game data was found!";
}

- (NSString*)getTableHeaderControllerName
{
	return @"NowPlayingHeader";
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	// this table only has virtual resources
	
	OFPaginatedSeries* cells = [OFPaginatedSeries paginatedSeries];
	OFNowPlayingOptionVirtualResource* option = nil;

	option = [OFNowPlayingOptionVirtualResource withTitle:@"Chat" andControllerName:@"SelectChatRoomDefinition" andImageName:@"OFChatIcon.png"];
	[cells addObject:option];

	option = [OFNowPlayingOptionVirtualResource withTitle:@"Leaderboards" andControllerName:@"Leaderboard" andImageName:@"OFLeaderboardsIcon.png"];
	[cells addObject:option];

	option = [OFNowPlayingOptionVirtualResource withTitle:@"Achievements" andControllerName:@"AchievementList" andImageName:@"OFAchievementsIcon.png"];
	[cells addObject:option];

	option = [OFNowPlayingOptionVirtualResource withTitle:@"Who's Playing" andControllerName:@"WhosPlaying" andImageName:@"OFWhosPlayingIcon.png"];
	[cells addObject:option];
	

	OFTableSectionDescription* section = [OFTableSectionDescription sectionWithTitle:[OpenFeint applicationDisplayName] andPage:cells];
	OFPaginatedSeries* resources = [OFPaginatedSeries paginatedSeriesWithObject:section];
	success.invoke(resources);
}

- (void)onCellWasClicked:(OFNowPlayingOptionVirtualResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
	UIViewController* controllerToPush = OFControllerLoader::load(cellResource.controllerName);
	controllerToPush.navigationItem.title = cellResource.title;
	
	if ([controllerToPush isKindOfClass:[OFSelectChatRoomDefinitionController class]])
	{
		OFSelectChatRoomDefinitionController* chatController = (OFSelectChatRoomDefinitionController*)controllerToPush;
		chatController.includeDeveloperRooms = YES;
		chatController.includeApplicationRooms = YES;
	}
	else if ([controllerToPush isKindOfClass:[OFLeaderboardController class]])
	{
		OFLeaderboardController* leaderboardController = (OFLeaderboardController*)controllerToPush;
		leaderboardController.applicationId = [OpenFeint clientApplicationId];
	}
	else if ([controllerToPush isKindOfClass:[OFAchievementListController class]])
	{
		OFAchievementListController* achievementController = (OFAchievementListController*)controllerToPush;
		achievementController.applicationName = [OpenFeint applicationDisplayName];
		achievementController.applicationId = [OpenFeint clientApplicationId];
		achievementController.applicationIconUrl = [OpenFeint clientApplicationIconUrl];
		achievementController.doesUserHaveApplication = YES;
	}
	else if ([controllerToPush isKindOfClass:[OFWhosPlayingController class]])
	{
		OFWhosPlayingController* whosPlayingController = (OFWhosPlayingController*)controllerToPush;
		whosPlayingController.applicationName = [OpenFeint applicationDisplayName];
		whosPlayingController.applicationId = [OpenFeint clientApplicationId];
		whosPlayingController.applicationIconUrl = [OpenFeint clientApplicationIconUrl];
	}

	OFAssert(controllerToPush != nil, "Must have a controller by now!");
	[self.navigationController pushViewController:controllerToPush animated:YES];
}

- (float)getBannerWidth
{
	UIView* myContent = self.view;
	if ([myContent isKindOfClass:[OFProfileFramedView class]])
	{
		myContent = ((OFProfileFramedView*)myContent).contentView;
	}
	
	return myContent.frame.size.width;
}

@end