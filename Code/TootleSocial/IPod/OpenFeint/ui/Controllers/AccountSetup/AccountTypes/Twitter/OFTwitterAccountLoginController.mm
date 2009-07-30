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
#import "OFTwitterAccountLoginController.h"
#import "OFViewDataMap.h"
#import "OFISerializer.h"
#import "OpenFeint+Private.h"
#import "OpenFeint+UserOptions.h"
#import "OFProvider.h"
#import "OFShowMessageAndReturnController.h"
#import "OFControllerLoader.h"

@implementation OFTwitterAccountLoginController

@synthesize streamIntegrationSwitch, streamIntegrationLabel, submitButton, contentView, integrationInfoLabel;


- (void)viewWillAppear:(BOOL)animated
{
	if (!self.addingAdditionalCredential)
	{
		self.streamIntegrationSwitch.hidden = YES;
		self.streamIntegrationLabel.hidden = YES;
		UIView* objectToMove = [OpenFeint isInLandscapeMode] ? (UIView*)self.submitButton : (UIView*)self.privacyDisclosure;
		CGRect objectRect = objectToMove.frame;
		objectRect.origin.y = self.streamIntegrationLabel.frame.origin.y;
		objectToMove.frame = objectRect;
		
		integrationInfoLabel.text = @"Enter the twitter username and password you used to secure your OpenFeint account.";
	}
	else
	{
		integrationInfoLabel.text = @"Connect to Twitter to find friends with OpenFeint accounts and import your profile picture.";
	}
	
	[super viewWillAppear:animated];
}

- (bool)shouldUseOAuth
{
	return self.addingAdditionalCredential;
}

- (void)populateViewDataMap:(OFViewDataMap*)dataMap
{
	dataMap->addFieldReference(@"username",	1);
	dataMap->addFieldReference(@"password", 2);
}

- (void)addHiddenParameters:(OFISerializer*)parameterStream
{
	[super addHiddenParameters:parameterStream];
	
	OFRetainedPtr <NSString> credentialsType = @"twitter"; 
	parameterStream->io("credential_type", credentialsType);
	
	if (self.addingAdditionalCredential)
	{
		bool enableStreamIntegration = streamIntegrationSwitch.on;
		parameterStream->io("enable_stream_integration", enableStreamIntegration);
	}
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
	return self.addingAdditionalCredential ? @"users_credentials.xml" : @"session.xml";
}

- (NSString*)getTextToShowWhileSubmitting
{
	return self.addingAdditionalCredential ? @"Connecting To Twitter" : @"Logging In To OpenFeint";
}

- (OFShowMessageAndReturnController*)controllerToPushOnCompletion
{
	if (self.addingAdditionalCredential)
	{
		OFShowMessageAndReturnController* nextController =  (OFShowMessageAndReturnController*)OFControllerLoader::load(@"ShowMessageAndReturn");
		nextController.messageLabel.text = [self getImportingFriendsMessage:@"Twitter"];
		nextController.title = @"Finding Friends";
		return nextController;
	}
	else
	{
		return [self getStandardLoggedInController];
	}	
}

- (void)dealloc
{
	self.streamIntegrationSwitch = nil;
	self.streamIntegrationLabel = nil;
	self.submitButton = nil;
	self.contentView = nil;
	self.integrationInfoLabel = nil;
	[super dealloc];
}

@end