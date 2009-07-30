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

#import "OFProfileFrame.h"

@class OFAchievement;

@interface OFAchievementDetailController : UIViewController< OFProfileFrame >
{
@package
	OFAchievement* achievement;
	NSString* userId;
}

@property (nonatomic, retain) NSString* userId;
@property (nonatomic, retain) OFAchievement* achievement;

- (NSString*)getProfileUserId;

@end