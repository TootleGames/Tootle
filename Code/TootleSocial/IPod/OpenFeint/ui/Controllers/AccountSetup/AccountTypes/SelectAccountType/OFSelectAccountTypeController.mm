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
#import "OFSelectAccountTypeController.h"
#import "OFControllerLoader.h"
#import "OFFacebookAccountController.h"
#import "OpenFeint+UserOptions.h"

@implementation OFSelectAccountTypeController

@synthesize socialNetworkNotice;
@synthesize openFeintNotice;	
@synthesize neverShowLostAccountWarning;

- (BOOL)shouldShowLostAccountWarning
{
	return !neverShowLostAccountWarning && ![OpenFeint loggedInUserHasNonDeviceCredential];
}

- (void)_pushAccountSetupControllerFor:(NSString*)containerName
{
	OFAccountSetupBaseController* accountController = (OFAccountSetupBaseController*)OFControllerLoader::load([NSString stringWithFormat:@"%@AccountLogin", containerName]);
	[accountController setCancelDelegate:mCancelDelegate];
	[accountController setCompletionDelegate:mCompletionDelegate];
	[[self navigationController] pushViewController:accountController animated:YES];
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
	if (buttonIndex == 0)
	{
		[self _pushAccountSetupControllerFor:mControllerToOpen.get()];
	}
}

- (void)_showAccountSetupFor:(NSString*)containerName
{
	if ([self shouldShowLostAccountWarning])
	{
		mControllerToOpen = containerName;
		NSString* warning = [NSString stringWithFormat:@"Warning! %@ is not secured and will be lost if you change account. You may secure your account from the settings page.", [OpenFeint lastLoggedInUserName]];
		UIActionSheet* warningSheet = [[[UIActionSheet alloc] initWithTitle:warning delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:@"Change Account Anyway" otherButtonTitles:nil] autorelease];
		[warningSheet showInView:self.view];
	}
	else
	{
		[self _pushAccountSetupControllerFor:containerName];
	}
}

- (void)_onPressedTwitter
{
	[self _showAccountSetupFor:@"Twitter"];
}

- (void)_onPressedFacebook
{
	[self _showAccountSetupFor:@"Facebook"];
}

- (void)registerActionsNow
{
	[self registerAction:OFDelegate(self, @selector(_onPressedTwitter)) forTag:2];	
	[self registerAction:OFDelegate(self, @selector(_onPressedFacebook)) forTag:3];		
}

- (void)dealloc
{
	self.socialNetworkNotice = nil;
	self.openFeintNotice = nil;
	[super dealloc];
}

@end
