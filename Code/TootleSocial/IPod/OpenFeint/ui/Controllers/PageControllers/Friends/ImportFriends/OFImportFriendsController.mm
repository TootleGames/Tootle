////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFImportFriendsController.h"
#import "OFControllerLoader.h"
#import "OFResourceControllerMap.h"
#import "OFDeadEndErrorController.h"
#import "OFUsersCredential.h"
#import "OFUsersCredentialService.h"
#import "OFAccountSetupBaseController.h"
#import "OFViewHelper.h"
#import "OFTableSectionDescription.h"
#import "OFShowMessageAndReturnController.h"

@implementation OFImportFriendsController

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFUsersCredential class], @"UsersCredential");
}

- (OFService*)getService
{
	return [OFUsersCredentialService sharedInstance];
}

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
	OFUsersCredential* credentialResource = (OFUsersCredential*)cellResource;
	
	if (credentialResource.isLinked)
	{
		[self showLoadingScreen];
		mImportCompleteMessage = [NSString stringWithFormat:@"Please allow some time for us to find your %@ friends with OpenFeint accounts. Any matches will appear in the friends tab.", 
								  [OFUsersCredential getDisplayNameForCredentialType:credentialResource.credentialType]];
		
		OFDelegate success(self, @selector(importFriendsSucceeded));
		OFDelegate failure(self, @selector(importFriendsFailed));
		[OFUsersCredentialService importFriendsFromCredentialType:credentialResource.credentialType onSuccess:success onFailure:failure];
	}
	else
	{
		OFAccountSetupBaseController* linkCredentialController = nil;
		if ([credentialResource.credentialType isEqualToString:@"twitter"])
		{
			linkCredentialController = (OFAccountSetupBaseController*)OFControllerLoader::load(@"TwitterAccountLogin");
		}
		else if ([credentialResource.credentialType isEqualToString:@"fbconnect"])
		{
			linkCredentialController = (OFAccountSetupBaseController*)OFControllerLoader::load(@"FacebookAccountLogin");
		}
		else if ([credentialResource.credentialType isEqualToString:@"http_basic"])
		{
			linkCredentialController = (OFAccountSetupBaseController*)OFControllerLoader::load(@"HttpCredentialsCreate");
		}
		
		if (linkCredentialController)
		{
			linkCredentialController.addingAdditionalCredential = YES;
			[self.navigationController pushViewController:linkCredentialController animated:YES];
		}
	}
}

- (void)pushCompleteControllerWithMessage:(NSString*)message andTitle:(NSString*)controllerTitle
{
	[self hideLoadingScreen];
	OFShowMessageAndReturnController* completeController = (OFShowMessageAndReturnController*)OFControllerLoader::load(@"ShowMessageAndReturn");
	completeController.messageLabel.text = message;
	completeController.title = controllerTitle;
	[completeController.continueButton setTitle:@"OK" forState:UIControlStateNormal];
	[completeController.continueButton setTitle:@"OK" forState:UIControlStateSelected];
	if ([self.navigationController.viewControllers count] > 1)
	{
		completeController.controllerToPopTo = [self.navigationController.viewControllers objectAtIndex:[self.navigationController.viewControllers count] - 2];
	}	
	[self.navigationController pushViewController:completeController animated:YES];
}

- (void)importFriendsSucceeded
{
	[self pushCompleteControllerWithMessage:mImportCompleteMessage.get() andTitle:@"Finding Friends"];
}

- (void)importFriendsFailed
{
	[self pushCompleteControllerWithMessage:@"An error occured when trying to import your friends. Please try again later." andTitle:@"Error"];
}

- (NSString*)getNoDataFoundMessage
{
	return @"An error has occurred. Please try again later.";
}

- (NSString*)getTableHeaderControllerName
{
	return @"ImportFriendsHeader";
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	[OFUsersCredentialService getIndexOnSuccess:success 
									  onFailure:failure
				onlyIncludeNotLinkedCredentials:NO
				  onlyIncludeFriendsCredentials:YES];
}

- (void)viewDidLoad
{
	[super viewDidLoad];
	
	//self.hidesBottomBarWhenPushed = YES;
}

- (void)dealloc
{
	[super dealloc];
}
@end

