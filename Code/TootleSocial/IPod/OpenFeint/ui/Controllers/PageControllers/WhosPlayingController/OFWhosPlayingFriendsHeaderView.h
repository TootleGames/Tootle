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

@class OFWhosPlayingController;

@interface OFWhosPlayingFriendsHeaderView : UIView
{
@private
	OFWhosPlayingController* whosPlayingController;
}

@property (nonatomic, assign) IBOutlet OFWhosPlayingController* whosPlayingController;

@end