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

#import "OFWhosPlayingController.h"

#import "OFDependencies.h"
#import "OFResourceControllerMap.h"
#import "OFControllerLoader.h"
#import "OFFriendsService.h"
#import "OFGamePlayer.h"
#import "OFUser.h"
#import "OFAchievementListController.h"
#import "OFTableSectionDescription.h"
#import "OFWhosPlayingFriendsHeaderView.h"
#import "OFWhosPlayingMyHeaderView.h"
#import "OpenFeint+Settings.h"
#import "OFFullScreenImportFriendsMessage.h"
#import "OFUsersCredential.h"

@implementation OFWhosPlayingController

@synthesize userId, applicationName, applicationId, applicationIconUrl;

- (void)dealloc
{
	self.applicationName = nil;
	self.applicationId = nil;
	self.applicationIconUrl = nil;
	self.userId = nil;

	[super dealloc];
}

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFGamePlayer class], @"WhosPlaying");
}

- (OFService*)getService
{
	return [OFFriendsService sharedInstance];
}

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
	OFGamePlayer* resource = (OFGamePlayer*)cellResource;

	OFAchievementListController* achievementListController = (OFAchievementListController*)OFControllerLoader::load(@"AchievementList");
	achievementListController.userId = resource.user.resourceId;
	achievementListController.applicationName = applicationName;
	achievementListController.applicationId = applicationId;
	achievementListController.applicationIconUrl = applicationIconUrl;
	achievementListController.doesUserHaveApplication = YES;
	[[self navigationController] pushViewController:achievementListController animated:YES];
}

- (NSString*)getNoDataFoundMessage
{
	return [NSString stringWithFormat:@"None of your friends are playing %@.", applicationName];
}

- (UIViewController*)getNoDataFoundViewController
{
	NSMutableArray* missingCredentialTypes = [self getMetaDataOfType:[OFUsersCredential class]];	
	bool mayImportFriends = [missingCredentialTypes count] > 0;
	if (mayImportFriends)
	{	
		NSString* notice = [self getNoDataFoundMessage];
		OFFullScreenImportFriendsMessage* noDataController = (OFFullScreenImportFriendsMessage*)OFControllerLoader::load(@"FullscreenImportFriendsMessage");	
		[noDataController setMissingCredentials:missingCredentialTypes withNotice:notice];
		noDataController.owner = self;
		return noDataController;
	}
	return nil;
}

- (void)doIndexActionWithPage:(NSUInteger)pageIndex onSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	[OFFriendsService getFriendsPlayingApp:applicationId forUser:userId pageIndex:pageIndex onSuccess:success onFailure:failure];
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure;
{
	[OFFriendsService getFriendsPlayingApp:applicationId forUser:userId pageIndex:1 onSuccess:success onFailure:failure];
}

- (void)onSectionsCreated
{
	OFTableSectionDescription* mySection = [self getSectionWithIdentifier:@"myself"];
	if (mySection)
	{
		OFWhosPlayingMyHeaderView* header = (OFWhosPlayingMyHeaderView*)OFControllerLoader::loadView(@"WhosPlayingMyHeaderView");
		header.gameTitleLabel.text = [OpenFeint applicationShortDisplayName];

		CGSize gameNameSize = [header.gameTitleLabel.text sizeWithFont:header.gameTitleLabel.font constrainedToSize:header.gameTitleLabel.frame.size];
		
		CGRect gameTitleFrame = header.gameTitleLabel.frame;
		gameTitleFrame.origin.y = MAX(0.f, header.frame.size.height - gameNameSize.height);
		gameTitleFrame.size.height = MIN(gameNameSize.height, header.frame.size.height);
		[header.gameTitleLabel setFrame:gameTitleFrame];		
		
		CGSize overallTextSize = [header.overallLabel.text sizeWithFont:header.overallLabel.font constrainedToSize:header.overallLabel.frame.size];

		CGRect overallLabelFrame = header.overallLabel.frame;
		overallLabelFrame.origin.y = MAX(0.f, header.frame.size.height - overallTextSize.height);
		overallLabelFrame.size.height = MIN(overallTextSize.height, header.frame.size.height);
		[header.overallLabel setFrame:overallLabelFrame];		

		mySection.headerView = header;
	}
	
	OFTableSectionDescription* friendsSection = [self getSectionWithIdentifier:@"friends"];
	if (friendsSection)
	{
		if ([friendsSection.page count] > 0)
		{
			OFWhosPlayingMyHeaderView* header = (OFWhosPlayingMyHeaderView*)OFControllerLoader::loadView(@"WhosPlayingFriendsHeaderView");
			friendsSection.headerView = header;
		}
		else
		{
			friendsSection.title = nil;	
		}
	}
	
	[self.tableView reloadData];
}


@end