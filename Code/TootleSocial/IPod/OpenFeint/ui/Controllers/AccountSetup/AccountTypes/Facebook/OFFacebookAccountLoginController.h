////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFFacebookAccountController.h"

@interface OFFacebookAccountLoginController : OFFacebookAccountController
{
	UISwitch* streamIntegrationSwitch;
	UILabel* streamIntegrationLabel;
	UILabel* findFriendsLabel;
}

@property (nonatomic, retain) IBOutlet UISwitch* streamIntegrationSwitch;
@property (nonatomic, retain) IBOutlet UILabel* streamIntegrationLabel;
@property (nonatomic, retain) IBOutlet UILabel* findFriendsLabel;

@end
