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

@interface OFAccountLoginController : OFAccountSetupBaseController< UIActionSheetDelegate >
{
	UILabel* warningHeader;
	UILabel* warningText;
	UIView* contentView;
	BOOL neverShowLostAccountWarning;
}

@property (nonatomic, retain) IBOutlet UILabel* warningHeader;
@property (nonatomic, retain) IBOutlet UILabel* warningText;

@property (nonatomic, retain) IBOutlet UIView* contentView;

@property (nonatomic, assign) BOOL neverShowLostAccountWarning;

- (IBAction)usedOtherAccountType;

@end
