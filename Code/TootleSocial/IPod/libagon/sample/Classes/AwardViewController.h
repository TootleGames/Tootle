//  AwardViewController.h

//  Copyright Aptocore ApS 2009. All rights reserved.

#import <UIKit/UIKit.h>
#import "LuckyAgonIds.h"

/**
 Simple example of how to display awards that have been unlocked.
 Unlocked awards are stored in a queue and showed using a small UIView dropping in from the top of the screen.
 You are free to use and modify that example code if it is useful for your scenario.
 */
@interface AwardViewController : UIViewController {
	// List of the awards that have been unlocked and are ready to be displayed.
	NSMutableArray* _pendingAwards;
	
	// The UI elements that are used to display the awards as they are unlocked.
	UIImageView* _awardBar;
	UIImageView* _awardIcon;
	UILabel* _awardLabel;
	UILabel* _awardTitle;
}

// Unlocks the award associated with the given index. If the award has already been unlocked, the
// method returns immediately without attempting to unlock the same award twice.
- (void)unlockAwardWithId:(AgonAwardId)awardId;

@end
