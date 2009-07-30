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

@interface OFChangeNameController : OFFormControllerHelper
{
@private
	UILabel* currentNameLabel;
	NSString* nameAttemptingToClaim;
}

@property (nonatomic, retain) IBOutlet UILabel* currentNameLabel;

-(IBAction)clickedAlreadyHaveAccount;
@end
