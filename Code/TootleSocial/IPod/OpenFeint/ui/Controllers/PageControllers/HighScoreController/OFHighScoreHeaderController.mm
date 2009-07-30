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
#import "OFHighScoreHeaderController.h"
#import "OFHighScoreController.h"
#import "OFViewHelper.h"
#import "OpenFeint+Private.h"
// OF2.0UI
//#import "OFToggleControl.h"

@implementation OFHighScoreHeaderController

@synthesize highScoreController;
	
- (void)viewDidLoad
{
	[super viewDidLoad];

	// OF2.0UI
	UISegmentedControl* toggle = (UISegmentedControl*)OFViewHelper::findViewByTag(self.view, 1);
//	OFToggleControl* toggle = (OFToggleControl*)OFViewHelper::findViewByTag(self.view, 1);
//	[toggle setImageForLeftSelected:[UIImage imageNamed:@"OFHighScoreGlobal.png"]];
//	[toggle setImageForRightSelected:[UIImage imageNamed:@"OFHighScoreFriends.png"]];
//	[toggle setLeftSelected:NO];
	
	float segmentHeight = toggle.frame.size.height;
	CGRect myRect = CGRectMake(0.0f, 0.0f, [OpenFeint getDashboardBounds].size.width, segmentHeight + 5.f);
	self.view.frame = myRect;
}

// OF2.0UI - New toggle control -- also need to update the XIB
//- (IBAction)selectorHasChanged:(OFToggleControl*)sender
//	if(sender.isLeftSelected == YES)
//		[highScoreController showGlobalLeaderboard];
//	else if(sender.isLeftSelected == NO)
//		[highScoreController showFriendsLeaderboard];	
- (IBAction)selectorHasChanged:(UISegmentedControl*)sender;
{
	if (sender.selectedSegmentIndex == 0)
	{
		[highScoreController showGlobalLeaderboard];
	}
	else if (sender.selectedSegmentIndex == 1)
	{
		[highScoreController showFriendsLeaderboard];	
	}
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

@end