////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFProfileNoFriendsHeaderView.h"
#import "OpenFeint+Private.h"

@implementation OFProfileNoFriendsHeaderView

@synthesize messageLabel, target, importFriendsCallback;

- (IBAction)onImportFriendsPressed
{
	[target performSelector:importFriendsCallback];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

@end
