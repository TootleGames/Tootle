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

@class OFHighScoreController;
// OF2.0UI
//@class OFToggleControl;

@interface OFHighScoreHeaderController : UIViewController
{
@private
	OFHighScoreController* highScoreController;
}

@property (nonatomic, readwrite, assign) IBOutlet OFHighScoreController* highScoreController;

// OF2.0UI - New toggle control -- also need to update the XIB
//- (IBAction)selectorHasChanged:(OFToggleControl*)sender;
- (IBAction)selectorHasChanged:(UISegmentedControl*)sender;

@end
