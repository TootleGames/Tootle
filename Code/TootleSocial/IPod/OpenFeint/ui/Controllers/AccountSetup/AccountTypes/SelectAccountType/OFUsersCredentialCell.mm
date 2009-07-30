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
#import "OFUsersCredentialCell.h"
#import "OFViewHelper.h"
#import "OFUsersCredential.h"
#import "OFTableSequenceControllerHelper.h"
#import "OFUsersCredentialService.h"

struct OFKnownCredential
{
	OFKnownCredential(NSString* _credentialType, NSString* _imageName, NSString* _controllerName, NSString* _labelText)
	: credentialType(_credentialType)
	, imageName(_imageName)
	, controllerName(_controllerName)
	, labelText(_labelText)
	{
	}
	
	OFRetainedPtr<NSString> credentialType;
	OFRetainedPtr<NSString> imageName;
	OFRetainedPtr<NSString> controllerName;
	OFRetainedPtr<NSString> labelText;
};

namespace  
{
	static OFKnownCredential sKnownCredentials[] = 
	{
		OFKnownCredential(@"twitter", @"OpenFeintAccountSetupTwitterLogo.png", @"TwitterAccountLogin", nil),
		OFKnownCredential(@"fbconnect", @"OpenFeintAccountSetupFBConnectButton.png", @"FacebookAccountLogin", nil),
		OFKnownCredential(@"http_basic", nil, @"OFHttpCredentialCreate", @"Create New Password"),
	};
}

@implementation OFUsersCredentialCell

@synthesize owner;

+(OFKnownCredential*)getKnownCredential:(NSString*)credentialName
{
	for (int i = 0; i < sizeof(sKnownCredentials) / sizeof(OFKnownCredential); i++)
	{
		if ([credentialName isEqualToString:sKnownCredentials[i].credentialType.get()])
		{
			return &sKnownCredentials[i];
		}
	}
	return NULL;
}

+ (NSString*)getCredentialImage:(NSString*)credentialName
{
	OFKnownCredential* knownCredential = [OFUsersCredentialCell getKnownCredential:credentialName];
	return knownCredential ? knownCredential->imageName.get() : nil;
}

+ (NSString*)getCredentialControllerName:(NSString*)credentialName
{
	OFKnownCredential* knownCredential = [OFUsersCredentialCell getKnownCredential:credentialName];
	return knownCredential ? knownCredential->controllerName.get() : nil;
}

+ (NSString*)getCredentialCellText:(NSString*)credentialName
{
	OFKnownCredential* knownCredential = [OFUsersCredentialCell getKnownCredential:credentialName];
	return knownCredential ? knownCredential->labelText.get() : nil;
}

- (void)onResourceChanged:(OFResource*)resource
{	
	OFUsersCredential* credential = (OFUsersCredential*)resource;
	UIImageView* credentialImageView = (UIImageView*)OFViewHelper::findViewByTag(self, 1);
	NSString* imageName = [OFUsersCredentialCell getCredentialImage:credential.credentialType];
	credentialImageView.image = imageName ? [UIImage imageNamed:imageName] : nil;
	UILabel* credentialLabel = (UILabel*)OFViewHelper::findViewByTag(self, 2);
	NSString* credentialText = [OFUsersCredentialCell getCredentialCellText:credential.credentialType];
	credentialLabel.text = credentialText;
	credentialLabel.hidden = credentialText == nil;
}

@end
