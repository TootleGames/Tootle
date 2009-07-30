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
#import "OFAccountSetupBaseController.h"
#import "OFShowMessageAndReturnController.h"
#import "OFFormControllerHelper+Submit.h"
#import "OpenFeint+Private.h"
#import "OpenFeint+UserOptions.h"
#import "OFProvider.h"
#import "OFISerializer.h"
#import "OFControllerLoader.h"

@implementation OFAccountSetupBaseController

@synthesize privacyDisclosure;
@synthesize addingAdditionalCredential = mAddingAdditionalCredential;

- (void)viewDidLoad
{
	[super viewDidLoad];
	
	self.navigationItem.rightBarButtonItem = [[[UIBarButtonItem alloc]	
					initWithBarButtonSystemItem:UIBarButtonSystemItemCancel 
					target:self
					action:@selector(cancelSetup)] autorelease];
}

- (BOOL)isInModalController
{
	return self.navigationController.parentViewController && self.navigationController.parentViewController.modalViewController != nil;
}

- (void)cancelSetup
{
	if (mCancelDelegate.isValid())
	{
		mCancelDelegate.invoke();
	}
	else if ([self isInModalController])
	{
		[self dismissModalViewControllerAnimated:YES];
	}
}

- (void)addHiddenParameters:(OFISerializer*)parameterStream
{
	OFRetainedPtr<NSString> udid = [UIDevice currentDevice].uniqueIdentifier;
	parameterStream->io("udid", udid);
}

- (OFShowMessageAndReturnController*)getStandardLoggedInController
{
	OFShowMessageAndReturnController* nextController =  (OFShowMessageAndReturnController*)OFControllerLoader::load(@"ShowMessageAndReturn");
	nextController.messageLabel.text = [NSString stringWithFormat:@"You are now logged in to OpenFeint as %@", [OpenFeint lastLoggedInUserName]];;
	return nextController;
}

- (OFShowMessageAndReturnController*)controllerToPushOnCompletion
{
	return nil;
}

- (UIViewController*)getControllerToPopTo
{
	// When logging in we always pop all the way back to the root. This is to make sure you don't pop back into a chat room you're no longer part of
	if (self.addingAdditionalCredential)
	{
		for (int i = [self.navigationController.viewControllers count] - 1; i >= 1; i--)
		{
			UIViewController* curController = [self.navigationController.viewControllers objectAtIndex:i];
			if (![curController isKindOfClass:[OFAccountSetupBaseController class]])
			{
				return [self.navigationController.viewControllers objectAtIndex:i - 1];
			}
		}
	}
	return nil;
}

- (void)popOutOfAccountFlow
{
	if ([self isInModalController])
	{
		[self dismissModalViewControllerAnimated:YES];
	}
	else
	{
		UIViewController* controllerToPopTo = [self getControllerToPopTo];
		if (controllerToPopTo)
		{
			[self.navigationController popToViewController:controllerToPopTo animated:YES];
		}
		else
		{
			[self.navigationController popToRootViewControllerAnimated:YES];
		}
	}
}

- (void)pushCompletionControllerOrPopOut
{
	if (mCompletionDelegate.isValid())
	{
		mCompletionDelegate.invoke();
	}
	else
	{
		OFShowMessageAndReturnController* controllerToPush = [self controllerToPushOnCompletion];
		if (controllerToPush)
		{
			controllerToPush.navigationItem.hidesBackButton = YES;
			controllerToPush.controllerToPopTo = [self getControllerToPopTo];
			[self.navigationController pushViewController:controllerToPush animated:YES];
		}
		else
		{
			[self popOutOfAccountFlow];
		
		}

	}
}

- (void)onFormSubmitted
{
	if (self.addingAdditionalCredential)
	{
		[OpenFeint setLoggedInUserHasNonDeviceCredential:YES];
		[self pushCompletionControllerOrPopOut];
	}
	else
	{
		[self showLoadingScreen];
		[[OpenFeint provider] destroyLocalCredentials];
		[[OpenFeint provider] loginAndBootstrap:OFDelegate(self, @selector(onBootstrapDone)) onFailure:OFDelegate(self, @selector(onBootstrapDone))];
	}	
}

- (void)onBootstrapDone
{
	[self hideLoadingScreen];
	[OpenFeint reloadInactiveTabBars];
	[self pushCompletionControllerOrPopOut];
}

- (void)dealloc
{
	self.privacyDisclosure = nil;
	[super dealloc];
}

- (void)setCancelDelegate:(OFDelegate const&)delegate
{
	mCancelDelegate = delegate;
}

- (void)setCompletionDelegate:(OFDelegate const&)delegate
{
	mCompletionDelegate = delegate;
}

- (NSString*)getImportingFriendsMessage:(NSString*)socialNetworkName
{
	return [NSString stringWithFormat:@"Your OpenFeint account is now connected to %@. Please allow some time for us to find your %@ friends with OpenFeint accounts. Any matches will appear in the Friends tab.", socialNetworkName, socialNetworkName];
}

@end