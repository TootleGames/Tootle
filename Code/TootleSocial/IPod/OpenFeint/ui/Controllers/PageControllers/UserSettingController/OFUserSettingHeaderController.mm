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
#import "OFUserSettingHeaderController.h"
#import "OFUserSettingController.h"
#import "OFViewHelper.h"
#import "OpenFeint+Private.h"

@implementation OFUserSettingHeaderController
	
- (void)viewDidLoad
{
	[super viewDidLoad];

	float segmentHeight = OFViewHelper::findViewByTag(self.view, 1).frame.size.height;
	CGRect myRect = CGRectMake(0.0f, 0.0f, [OpenFeint getDashboardBounds].size.width, segmentHeight);
	self.view.frame = myRect;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

@end