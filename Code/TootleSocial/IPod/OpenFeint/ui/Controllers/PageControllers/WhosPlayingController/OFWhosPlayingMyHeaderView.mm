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
#import "OFWhosPlayingMyHeaderView.h"
#import "OFWhosPlayingController.h"

@implementation OFWhosPlayingMyHeaderView

@synthesize whosPlayingController, gameTitleLabel, overallLabel;

- (void)dealloc
{
	OFSafeRelease(overallLabel);
	OFSafeRelease(gameTitleLabel);
	[super dealloc];
}

@end