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

@interface OFTwitterAccountLoginController : OFAccountSetupBaseController 
{
	UISwitch* streamIntegrationSwitch;
	UILabel* streamIntegrationLabel;
	UIButton* submitButton;
	UIView* contentView;
	UILabel* integrationInfoLabel;
}

@property (nonatomic, retain) IBOutlet UISwitch* streamIntegrationSwitch;
@property (nonatomic, retain) IBOutlet UILabel* streamIntegrationLabel;
@property (nonatomic, retain) IBOutlet UIButton* submitButton;
@property (nonatomic, retain) IBOutlet UIView* contentView;
@property (nonatomic, retain) IBOutlet UILabel* integrationInfoLabel;

@end
