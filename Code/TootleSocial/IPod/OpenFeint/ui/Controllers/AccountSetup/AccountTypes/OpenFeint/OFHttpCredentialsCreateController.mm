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
#import "OFHttpCredentialsCreateController.h"
#import "OFViewDataMap.h"
#import "OFProvider.h"
#import "OFISerializer.h"
#import "OFProvider.h"
#import "OpenFeint+Private.h"
#import "OFShowMessageAndReturnController.h"
#import "OFControllerLoader.h"

@implementation OFHttpCredentialsCreateController

- (bool)shouldUseOAuth
{
	return true;
}

- (void)populateViewDataMap:(OFViewDataMap*)dataMap
{	
	dataMap->addFieldReference(@"email",				2);
	dataMap->addFieldReference(@"password",				3);
	dataMap->addFieldReference(@"password_confirmation",	4);		
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
	return @"users_credentials.xml";
}

- (OFShowMessageAndReturnController*)controllerToPushOnCompletion
{
	OFShowMessageAndReturnController* nextController =  (OFShowMessageAndReturnController*)OFControllerLoader::load(@"ShowMessageAndReturn");
	nextController.messageLabel.text = @"Your account is secured! You may now login from any device.";
	nextController.title = @"Account Secured";
	return nextController;
}

@end
