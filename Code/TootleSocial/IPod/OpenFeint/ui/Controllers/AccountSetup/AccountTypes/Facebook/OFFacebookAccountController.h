////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFAccountSetupBaseController.h"
#import "FBConnect.h"
@class FBLoginDialog;

@interface OFFacebookAccountController : OFAccountSetupBaseController<FBSessionDelegate, FBDialogDelegate, FBRequestDelegate>
{
@private
	FBUID fbuid;
	FBSession* fbSession;
	NSURL* urlToLaunch;
	UIImageView* fbLoggedInStatusImageView;
	FBLoginDialog* loginDialog;
	UIInterfaceOrientation orientationBeforeFix;
	UIView* invisibleKeyboardTrap;
}

- (void)closeLoginDialog;

@property (nonatomic, assign) FBUID fbuid;
@property (nonatomic, retain) NSURL* urlToLaunch;
@property (nonatomic, retain) IBOutlet UIImageView* fbLoggedInStatusImageView;
@property (nonatomic, retain) FBSession* fbSession;
@end
