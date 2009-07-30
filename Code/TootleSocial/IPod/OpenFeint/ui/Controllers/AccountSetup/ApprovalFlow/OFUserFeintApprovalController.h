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

#import "OFCallbackable.h"

@interface OFUserFeintApprovalController : UIViewController<OFCallbackable>
{
@private
	UILabel* gameNameLabel;
}

@property (nonatomic, retain) IBOutlet UILabel* gameNameLabel;

-(IBAction)clickedUseFeint;
-(IBAction)clickedDontUseFeint;
@end
