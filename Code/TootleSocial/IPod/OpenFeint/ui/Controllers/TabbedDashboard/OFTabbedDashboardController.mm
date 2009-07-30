////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFDependencies.h"
#import "OFTabbedDashboardController.h"
#import "OFTabbedDashboardPageController.h"
#import "OFSelectChatRoomDefinitionController.h"
#import "OFControllerLoader.h"
#import "OFNavigationController.h"
#import "OFNowPlayingController.h"
#import "OFPlayedGameController.h"
#import "OFFriendsController.h"
#import "OpenFeint+Private.h"
#import "OpenFeint+Settings.h"

@implementation OFTabbedDashboardController

- (void) viewDidLoad
{
	NSMutableArray* tabs = [NSMutableArray arrayWithCapacity:5];
	OFSelectChatRoomDefinitionController* globalChatController = (OFSelectChatRoomDefinitionController*)OFControllerLoader::load(@"SelectChatRoomDefinition");
	globalChatController.includeGlobalRooms = YES;
	globalChatController.tabBarItem = [[[UITabBarItem alloc] initWithTitle:@"Feint Lobby" image:[UIImage imageNamed:@"OpenFeintTabIconFeintChat.png"] tag:5] autorelease];
	globalChatController.navigationItem.title = @"Select Lobby";

	OFNowPlayingController* nowPlayingController = (OFNowPlayingController*)OFControllerLoader::load(@"NowPlaying");
	nowPlayingController.tabBarItem = [[[UITabBarItem alloc] initWithTitle:[OpenFeint applicationShortDisplayName] image:[UIImage imageNamed:@"OpenFeintTabIconGame.png"] tag:7] autorelease];
	nowPlayingController.title = [OpenFeint applicationShortDisplayName];
	
	OFPlayedGameController* feintLibraryController = (OFPlayedGameController*)OFControllerLoader::load(@"PlayedGame");
	feintLibraryController.tabBarItem = [[[UITabBarItem alloc] initWithTitle:@"My Games" image:[UIImage imageNamed:@"OpenFeintTabIconLibrary.png"] tag:8] autorelease];
	feintLibraryController.navigationItem.title = @"My Games";

	OFFriendsController* friendsController = (OFFriendsController*)OFControllerLoader::load(@"Friends");
	friendsController.tabBarItem = [[[UITabBarItem alloc] initWithTitle:@"Friends" image:[UIImage imageNamed:@"OpenFeintTabIconFriends.png"] tag:9] autorelease];

	[tabs addObject:[OFTabbedDashboardPageController pageWithInstantiatedController:globalChatController]];
	[tabs addObject:[OFTabbedDashboardPageController pageWithInstantiatedController:nowPlayingController]];
	[tabs addObject:[OFTabbedDashboardPageController pageWithInstantiatedController:friendsController]];
	[tabs addObject:[OFTabbedDashboardPageController pageWithInstantiatedController:feintLibraryController]];
	[tabs addObject:[OFTabbedDashboardPageController pageWithController:@"UserSetting"]];
							
	self.viewControllers = tabs;
	self.delegate = self;
}

- (void)tabBarController:(UITabBarController *)tabBarController didSelectViewController:(UIViewController *)viewController
{
	for (OFNavigationController* navController in self.viewControllers)
	{
		OFAssert([navController isKindOfClass:[OFNavigationController class]], @"all tab bar items need to be navigation controllers");
		navController.isInHiddenTab = (navController != viewController);
	}
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

@end
