//  LuckyViewController.h

//  Copyright Aptocore ApS 2009. All rights reserved.


#import <UIKit/UIKit.h>
#import "AwardViewController.h"
#import "LuckyAgonIds.h"

@interface LuckyViewController : UIViewController<UIAlertViewDelegate, UIActionSheetDelegate> {
	AwardViewController* _awardViewController;
	UILabel* _randomNumberLabel;
	UIButton* _playGameButton;
	UIButton* _difficultyButton;
	NSTimer* _timer;
	UIButton* _showLeaderboardsButton;
	UIButton* _showAwardsButton;
	UIButton* _showFriendsButton;
	UIButton* _showProfileButton;
	UILabel* _bestScoreLabel;
	int _theScore;
	AgonLeaderboardId _difficulty;
}

@end

