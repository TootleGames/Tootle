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

@interface OFWhosPlayingMyHeaderView : UIView
{
@private
	OFWhosPlayingController* whosPlayingController;
	UILabel* gameTitleLabel;
	UILabel* overallLabel;
}

@property (nonatomic, assign) IBOutlet OFWhosPlayingController* whosPlayingController;
@property (nonatomic, retain) IBOutlet UILabel* gameTitleLabel;
@property (nonatomic, retain) IBOutlet UILabel* overallLabel;

@end