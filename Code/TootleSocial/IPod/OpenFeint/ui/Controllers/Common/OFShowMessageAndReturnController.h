////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#import "OFFormControllerHelper.h"

@interface OFShowMessageAndReturnController : UIViewController
{
	UIViewController* controllerToPopTo;
	UILabel* messageLabel;
	UIButton* continueButton;
}
@property (nonatomic, retain) IBOutlet UILabel* messageLabel;
@property (nonatomic, retain) IBOutlet UIButton* continueButton;
@property (retain) UIViewController* controllerToPopTo;

-(IBAction)clickedContinue;

@end

