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
#import "OFUserSettingCell.h"
#import "OFViewHelper.h"
#import "OFUserSetting.h"
#import "OFTableSequenceControllerHelper.h"
#import "OFUserSettingService.h"

@implementation OFUserSettingCell

@synthesize name;
@synthesize booleanSwitch;
@synthesize owner;

- (void)setControlToSettingValue:(OFUserSetting*)setting
{
	if([setting.valueType isEqualToString:@"bool"])
	{
		booleanSwitch.on = [setting.value boolValue];
		booleanSwitch.enabled = true;
	}
	else
	{
		booleanSwitch.enabled = false;
		booleanSwitch.on = NO;
	}
}

- (void)setSettingToControlValue:(OFUserSetting*)setting
{
	if([setting.valueType isEqualToString:@"bool"])
	{
		setting.value = booleanSwitch.on ? @"1" : @"0";
	}
}

- (void)onResourceChanged:(OFResource*)resource
{	
	OFUserSetting* setting = (OFUserSetting*)resource;
	name.text = setting.name;
	
	[self setControlToSettingValue:setting];
}

- (IBAction)switchToggled
{
	[owner showLoadingScreen];
	
	OFUserSetting* setting = (OFUserSetting*)mResource;
	[OFUserSettingService 
		setUserSettingWithId:setting.resourceId
		toBoolValue:booleanSwitch.on 
		onSuccess:OFDelegate(self, @selector(onSubmitSuccess))
		onFailure:OFDelegate(self, @selector(onSubmitFailure))];
}

- (void)onSubmitFailure
{
	[owner hideLoadingScreen];
	
	[self setControlToSettingValue:(OFUserSetting*)mResource];
	
	[[[[UIAlertView alloc]
		initWithTitle:@"Oops! There was a problem"
		message:@"Something went wrong and we weren't able to save your change. Please try again later."
		delegate:nil
		cancelButtonTitle:@"Ok"
		otherButtonTitles:nil] autorelease] show];
		
	[self setNeedsLayout];
}

- (void)onSubmitSuccess
{
	[owner hideLoadingScreen];
	[self setSettingToControlValue:(OFUserSetting*)mResource];
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

@end
