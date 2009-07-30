////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFFormControllerHelper.h"

@class OFShowMessageAndReturnController;

@interface OFAccountSetupBaseController : OFFormControllerHelper
{
@private
	UILabel* privacyDisclosure;
	BOOL mAddingAdditionalCredential;
	
@package
	OFDelegate mCancelDelegate;
	OFDelegate mCompletionDelegate;
}

@property (nonatomic, retain) IBOutlet UILabel* privacyDisclosure;
@property (assign) BOOL addingAdditionalCredential;

- (void)cancelSetup;
- (void)addHiddenParameters:(OFISerializer*)parameterStream;
- (UIViewController*)getControllerToPopTo;
- (OFShowMessageAndReturnController*)getStandardLoggedInController;

- (void)setCancelDelegate:(OFDelegate const&)delegate;
- (void)setCompletionDelegate:(OFDelegate const&)delegate;

- (NSString*)getImportingFriendsMessage:(NSString*)socialNetworkName;


- (void)setCancelDelegate:(OFDelegate const&)delegate;
- (void)setCompletionDelegate:(OFDelegate const&)delegate;

@end
