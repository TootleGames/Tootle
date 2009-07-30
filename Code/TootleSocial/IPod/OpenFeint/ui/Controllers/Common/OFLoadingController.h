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

#import <Foundation/Foundation.h>

@interface OFLoadingController : UIViewController

+ (OFLoadingController*)loadingControllerWithText:(NSString*)loadingText;
- (void)setLoadingText:(NSString*)loadingText;

@end
