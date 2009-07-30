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

@class OFAchievementListController;

@interface OFAchievementListGetGameHeaderController : UIViewController<OFTableControllerHeader>
{
@private
	OFAchievementListController* achievementListController;
}

@property (nonatomic, readwrite, assign) IBOutlet OFAchievementListController* achievementListController;

@end