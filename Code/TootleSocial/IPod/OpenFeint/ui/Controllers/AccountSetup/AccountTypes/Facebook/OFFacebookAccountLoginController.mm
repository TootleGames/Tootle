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
#import "OFFacebookAccountLoginController.h"
#import "OFViewDataMap.h"
#import "OFProvider.h"
#import "OFISerializer.h"
#import "OFProvider.h"
#import "OpenFeint+Private.h"
#import "OFControllerLoader.h"
#import "OFShowMessageAndReturnController.h"

@implementation OFFacebookAccountLoginController

@synthesize streamIntegrationSwitch, streamIntegrationLabel, findFriendsLabel;

- (void)viewWillAppear:(BOOL)animated
{
	if (!self.addingAdditionalCredential)
	{
		self.streamIntegrationSwitch.hidden = YES;
		self.streamIntegrationLabel.hidden = YES;
		CGRect disclosureRect = self.privacyDisclosure.frame;
		disclosureRect.origin.y = self.streamIntegrationLabel.frame.origin.y;
		self.privacyDisclosure.frame = disclosureRect;
		findFriendsLabel.hidden = YES;
	}
	
	[super viewWillAppear:animated];
}

- (void)populateViewDataMap:(OFViewDataMap*)dataMap
{	
}

- (void)addHiddenParameters:(OFISerializer*)parameterStream
{
	[super addHiddenParameters:parameterStream];
	
	OFRetainedPtr<NSString> facebookUId = [NSString stringWithFormat:@"%lld", self.fbuid];
	OFRetainedPtr<NSString> facebookSessionKey = [self.fbSession sessionKey];
	if(self.fbSession == nil)
	{
		facebookSessionKey = @"";
	}
	
	parameterStream->io("credential[fbuid]",	   facebookUId);
	parameterStream->io("credential[session_key]", facebookSessionKey);
	
	if (self.addingAdditionalCredential)
	{
		bool enableStreamIntegration = streamIntegrationSwitch.on;
		parameterStream->io("enable_stream_integration", enableStreamIntegration);
	}
}

- (NSString*)singularResourceName
{
	return self.addingAdditionalCredential ? @"users_credential" : @"credential";
}

- (NSString*)getFormSubmissionUrl
{
	return self.addingAdditionalCredential ? @"users_credentials.xml" : @"session.xml";
}

- (NSString*)getTextToShowWhileSubmitting
{
	return self.addingAdditionalCredential ? @"Connecting To Facebook" : @"Logging In To OpenFeint";
}

- (OFShowMessageAndReturnController*)controllerToPushOnCompletion
{
	if (self.addingAdditionalCredential)
	{
		OFShowMessageAndReturnController* nextController =  (OFShowMessageAndReturnController*)OFControllerLoader::load(@"ShowMessageAndReturn");
		nextController.messageLabel.text = [self getImportingFriendsMessage:@"Facebook"];
		nextController.title = @"Finding Friends";
		return nextController;
	}
	else
	{
		return [super getStandardLoggedInController];
	}	
}

- (void)dealloc
{
	self.streamIntegrationLabel = nil;
	self.streamIntegrationSwitch = nil;
	self.findFriendsLabel = nil;
	[super dealloc];
}
@end
