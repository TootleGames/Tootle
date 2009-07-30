////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFChangeNameController.h"
#import "OpenFeint+UserOptions.h"
#import "OFViewDataMap.h"
#import "OFISerializer.h"
#import "OFViewHelper.h"
#import "OFSelectAccountTypeController.h"
#import "OFControllerLoader.h"

namespace 
{
	const int kNameInputFieldTag = 1;
}
@implementation OFChangeNameController

@synthesize currentNameLabel;


- (void)populateViewDataMap:(OFViewDataMap*)dataMap
{
	dataMap->addFieldReference(@"name", kNameInputFieldTag);	
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	
	currentNameLabel.text = [OpenFeint lastLoggedInUserName];	
	
	if ([OpenFeint loggedInUserHasNonDeviceCredential])
	{
		UIView* oldUserButton = OFViewHelper::findViewByTag(self.view, 5);
		oldUserButton.hidden = YES;
	}
	
}

- (void)onBeforeFormSubmitted
{
	[nameAttemptingToClaim release];
	UITextField* nameField = (UITextField*)OFViewHelper::findViewByTag(self.view, kNameInputFieldTag);
	nameAttemptingToClaim = [nameField.text retain];
}

- (void)addHiddenParameters:(OFISerializer*)parameterStream
{
	OFRetainedPtr<NSString> me = @"me";
	parameterStream->io("id", me);
}

- (void)registerActionsNow
{
}

- (void)onFormSubmitted
{
	[OpenFeint setLastLoggedInUserName:nameAttemptingToClaim];
	[OpenFeint setLoggedInUserHasSetName:YES];
	currentNameLabel.text = nameAttemptingToClaim;
	OFSafeRelease(nameAttemptingToClaim);
	OFControllerLoader::push(self, @"ChangeNameComplete");
}

- (NSString*)singularResourceName
{
	return @"user";
}

- (NSString*)getFormSubmissionUrl
{
	return @"users/update_name.xml";
}

- (NSString*)getHTTPMethod
{
	return @"POST";
}

-(IBAction)clickedAlreadyHaveAccount
{
	OFControllerLoader::push(self, @"OpenFeintAccountLogin");
}

- (void)dealloc
{
	OFSafeRelease(nameAttemptingToClaim);
	[super dealloc];
}


@end
