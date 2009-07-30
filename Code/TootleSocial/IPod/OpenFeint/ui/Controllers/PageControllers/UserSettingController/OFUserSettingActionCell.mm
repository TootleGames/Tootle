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
#import "OFUserSettingActionCell.h"
#import "OFViewHelper.h"
#import "OFTableSequenceControllerHelper.h"
#import "OFUserSettingPushController.h"


@implementation OFUserSettingActionCell

@synthesize owner;

- (void)hideCell
{
	UILabel* titleLabel = (UILabel*)OFViewHelper::findViewByTag(self, 1);
	titleLabel.text = @"";
}

- (void)onResourceChanged:(OFResource*)resource
{	
	if ([resource isKindOfClass:[OFUserSettingPushController class]])
	{
		OFUserSettingPushController* pushControllerResource = (OFUserSettingPushController*)resource;
		if ([pushControllerResource verifyControllerExists])
		{
			UILabel* titleLabel = (UILabel*)OFViewHelper::findViewByTag(self, 1);
			titleLabel.text = pushControllerResource.settingTitle;
		}
		else
		{
			[self hideCell];
		}
	}
	else
	{
		[self hideCell];
	}
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

- (void)dealloc
{
	[super dealloc];
}

@end
