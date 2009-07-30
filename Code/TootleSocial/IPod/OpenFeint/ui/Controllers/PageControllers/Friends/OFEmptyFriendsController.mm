////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFEmptyFriendsController.h"
#import "OFControllerLoader.h"
#import "OpenFeint+Private.h"

@implementation OFEmptyFriendsController

@synthesize owner;

- (IBAction)onImportFriendsPressed
{
	[self.owner.navigationController pushViewController:OFControllerLoader::load(@"ImportFriends") animated:YES];
}

@end