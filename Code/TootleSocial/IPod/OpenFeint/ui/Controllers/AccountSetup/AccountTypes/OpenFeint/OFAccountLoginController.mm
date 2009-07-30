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
#import "OFAccountLoginController.h"
#import "OFViewDataMap.h"
#import "OFProvider.h"
#import "OFISerializer.h"
#import "OFProvider.h"
#import "OpenFeint+Private.h"
#import "OpenFeint+UserOptions.h"
#import "OFShowMessageAndReturnController.h"
#import "OFFormControllerHelper+Submit.h"
#import "OFSelectAccountTypeController.h"
#import "OFControllerLoader.h"
#import "OFViewHelper.h"

@implementation OFAccountLoginController

@synthesize warningHeader;
@synthesize warningText;
@synthesize neverShowLostAccountWarning;
@synthesize contentView;

- (BOOL)shouldShowLostAccountWarning
{
	return !neverShowLostAccountWarning && ![OpenFeint loggedInUserHasNonDeviceCredential];
}

- (IBAction)onSubmitForm:(UIView*)sender
{
	if ([self shouldShowLostAccountWarning])
	{
		NSString* warning = [NSString stringWithFormat:@"Warning! %@ is not secured and will be lost if you change account. You may secure your account from the settings page.", [OpenFeint lastLoggedInUserName]];
		UIActionSheet* warningSheet = [[[UIActionSheet alloc] initWithTitle:warning delegate:self cancelButtonTitle:@"Cancel" destructiveButtonTitle:@"Change Account Anyway" otherButtonTitles:nil] autorelease];
		[warningSheet showInView:[OpenFeint getTopLevelView]];
	}
	else
	{
		[super onSubmitForm:nil];
	}
}

- (IBAction)usedOtherAccountType
{
	OFSelectAccountTypeController* accountController = (OFSelectAccountTypeController*)OFControllerLoader::load(@"SelectAccountType");
	[accountController setCancelDelegate:mCancelDelegate];
	[accountController setCompletionDelegate:mCompletionDelegate];
	accountController.neverShowLostAccountWarning = neverShowLostAccountWarning;
	[[self navigationController] pushViewController:accountController animated:YES];
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];

	if ([self shouldShowLostAccountWarning])
	{
		self.warningHeader.hidden = NO;
		self.warningText.hidden = NO;
		self.warningText.text = [NSString stringWithFormat:@"%@ has not been password protected and will be lost if you change account.", [OpenFeint lastLoggedInUserName]];
	}
	else
	{
		CGRect frame = self.contentView.frame;
		frame.origin.y -= self.warningText.frame.size.height + self.warningHeader.frame.size.height;
		self.contentView.frame = frame;
	}
}

- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];

	if (![self shouldShowLostAccountWarning])
	{
		CGRect frame = self.contentView.frame;
		frame.origin.y += self.warningText.frame.size.height + self.warningHeader.frame.size.height;
		self.contentView.frame = frame;
	}
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
	if (buttonIndex == 0)
	{
		[super onSubmitForm:nil];
	}
}

- (bool)shouldUseOAuth
{
	return false;
}

- (void)populateViewDataMap:(OFViewDataMap*)dataMap
{
	dataMap->addFieldReference(@"email",	1);
	dataMap->addFieldReference(@"password", 2);
}

- (void)addHiddenParameters:(OFISerializer*)parameterStream
{
	[super addHiddenParameters:parameterStream];
	
	OFRetainedPtr <NSString> credentialsType = @"http_basic"; 
	parameterStream->io("credential_type", credentialsType);	
	
}

- (void)registerActionsNow
{
}

- (NSString*)singularResourceName
{
	return @"credential";
}

- (NSString*)getFormSubmissionUrl
{
	return @"session.xml";
}

- (OFShowMessageAndReturnController*)controllerToPushOnCompletion
{
	return [self getStandardLoggedInController];
}

- (void)dealloc
{
	[super dealloc];
}

@end
