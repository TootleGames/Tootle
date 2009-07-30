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
#import "OFUserSettingController.h"
#import "OFResourceControllerMap.h"
#import "OFUserSetting.h"
#import "OFUserSettingService.h"
#import "OpenFeint.h"
#import "OFUserSettingPushController.h"

@implementation OFUserSettingController

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFUserSetting class], @"UserSetting");
	resourceMap->addResource([OFUserSettingPushController class], @"UserSettingAction");
	
}

- (OFService*)getService
{
	return [OFUserSettingService sharedInstance];
}

- (bool)shouldAlwaysRefreshWhenShown
{
	return true;
}

- (NSString*)getNoDataFoundMessage
{
	return [NSString stringWithFormat:@"There are no settings available right now"];
}

- (NSString*)getTableHeaderControllerName
{
	return @"UserSettingHeader";
}

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
	if ([cellResource isKindOfClass:[OFUserSettingPushController class]])
	{
		OFUserSettingPushController* pushControllerResource = (OFUserSettingPushController*)cellResource;
		UIViewController* controllerToPush = [pushControllerResource getController];
		if (controllerToPush)
		{
			[self.navigationController pushViewController:controllerToPush animated:YES];
		}
	}
}

@end