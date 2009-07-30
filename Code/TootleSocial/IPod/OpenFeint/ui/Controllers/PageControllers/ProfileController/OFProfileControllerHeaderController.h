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

#import "OFTableControllerHeader.h"

@class OFProfileController;

@interface OFProfileControllerHeaderController : UIViewController<OFTableControllerHeader>
{
@private
	OFProfileController* profileController;
}

@property (nonatomic, readwrite, assign) IBOutlet OFProfileController* profileController;

@end