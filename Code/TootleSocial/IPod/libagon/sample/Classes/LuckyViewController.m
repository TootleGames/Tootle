//  LuckyViewController.m

//  Copyright Aptocore ApS 2009. All rights reserved.


#import "LuckyViewController.h"
#import "AGON.h"

const int MAXSCORE = 1000000;

@interface LuckyViewController (LuckyViewControllerPrivate)
- (void)stopGame;
- (NSString*)getDifficultyString;
@end

@implementation LuckyViewController

- (id)init {
	_difficulty = LEADERBOARD_MEDIUM;
	_theScore = 0;
	
	_awardViewController = nil;
	_randomNumberLabel = nil;
	_playGameButton = nil;
	_difficultyButton = nil;
	_timer = nil;
	_showLeaderboardsButton = nil;
	_showAwardsButton = nil;
	_showFriendsButton = nil;
	_showProfileButton = nil;
	_bestScoreLabel = nil;
	
	return [super init];
}

- (void)dealloc {
	[self stopGame];
	
	[_awardViewController release];
	[_randomNumberLabel release];
	[_playGameButton release];
	[_difficultyButton release];
	[_showLeaderboardsButton release];
	[_showAwardsButton release];
	[_showFriendsButton release];
	[_showProfileButton release];
    [_bestScoreLabel release];
	[super dealloc];
}

// This timer event, generates a new random number every tick.
// When the user stops the timer, the latest number will be the score.
- (void)timerEvent:(NSTimer*)theTimer {
	_theScore = MAXSCORE * (rand() / (float)RAND_MAX);
	_randomNumberLabel.text = [NSString stringWithFormat:@"%d",_theScore];
}

// Start the Game timer 
- (void)startGame {
	_timer = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(timerEvent:) userInfo:nil repeats:YES];
	srand(time(NULL));
	[_playGameButton setTitle:@"I Feel Lucky" forState:UIControlStateNormal];
}

// Stop the game timer
- (void)stopGame {
	[_timer invalidate];
	_timer = nil;
	[_playGameButton setTitle:@"Start" forState:UIControlStateNormal];
}

- (BOOL)isStarted {
	return _timer != nil;
}

// The AGON view has been hidden
- (void)agonDidHide {
	[self startGame];
}

// Try to unlock award
- (void)unlockAwardWithId:(AgonAwardId)awardId {
	[_awardViewController unlockAwardWithId:awardId];
}

- (AgonAwardId)getAwardIdFromIndex:(int)index {
	const AgonAwardId awards[] = {
		AWARD_CLOVER_FINGERS,
		AWARD_HORSE_SHOE,
		AWARD_RED_RABBIT_FOOT,
		AWARD_BLACK_CAT_CROSSING, 
		AWARD_BELOW_THE_LADDER,
		AWARD_WINNING_SMILE,
		AWARD_ON_A_ROLL,
		AWARD_MAGIC_8_BALL,
		AWARD_BULLSEYE,
		AWARD_LUCKY_LUKE,
		AWARD_POT_OF_GOLD,
	};
	
	return awards[index];
}

// Check if a score unlocks an award.
- (void)checkAwardsAtScore:(int)score {
	// This very simple award system calculates what award to unlock.
	float normalized = ((float)score / (float)MAXSCORE );
	int awardIdx = normalized * 11.0f;
	// Try and unlock it.
	[self unlockAwardWithId:[self getAwardIdFromIndex:awardIdx]];
}


// The user pressed "I Feel Lucky" button
- (void)getScorePressed:(id)sender {
	if ([self isStarted]) {
		[self stopGame];
		
		// Create the string to display for the score in AGON.
		NSString* s = [NSString stringWithFormat:@"%d", _theScore];
		
		// Submit to AGON's local storage which synchronizes with the
		// AGON backend when going online (when a blade is shown).
		AgonSubmitScore(_theScore, s, _difficulty);
		
		// Check if the score unlocks an award.
		[self checkAwardsAtScore:_theScore];
		
		// Update label with best score.
		NSString* bestScore = AgonGetActiveProfileBestDisplayScore(_difficulty);
		if (!bestScore) {
			bestScore = @"None";
		}
		_bestScoreLabel.text = [NSString stringWithFormat:@"Best Score: %@", bestScore];
	}
	else {
		[self startGame];
	}
}

// Show difficulty blade
- (void)difficultyPressed:(id)sender {
	UIActionSheet* s = [[UIActionSheet alloc] initWithTitle:@"Select Difficulty"
												   delegate:self 
										  cancelButtonTitle:@"Cancel" 
									 destructiveButtonTitle:nil 
										  otherButtonTitles:@"Easy",@"Medium",@"Hard",@"Extreme",nil];
	
	[s showInView:self.view ];
	[s release];
}

// Change difficulty. Notice that the difficulty has no "gameplay" influence,
// it is merely to give an example of using multiple leaderboards.
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {	
	if(buttonIndex == 0) {
		_difficulty = LEADERBOARD_EASY;
	}
	else if(buttonIndex == 1) {
		_difficulty = LEADERBOARD_MEDIUM;
	}
	else if(buttonIndex == 2) {
		_difficulty = LEADERBOARD_HARD;
	}
	else if(buttonIndex == 3) {
		_difficulty = LEADERBOARD_EXTREME;
	}
	
	// Update title on difficulty button.
	[_difficultyButton setTitle:[self getDifficultyString] forState:UIControlStateNormal];
	
	// Update label with best score.
	NSString* bestScore = AgonGetActiveProfileBestDisplayScore(_difficulty);
	if (!bestScore) {
		bestScore = @"None";
	}
	_bestScoreLabel.text = [NSString stringWithFormat:@"Best Score: %@", bestScore];
}

// Show leaderboards blade
- (void)showHighscorePressed:(id)sender {
	[self stopGame];
	// Show the leaderboard corresponding to the current difficulty.
	AgonShowLeaderboard(self, @selector(agonDidHide), _difficulty);
}

// Show awards blade
- (void)showAwardsPressed:(id)sender {
	[self stopGame];
	// We Show AGON with the profile view
	AgonShow(self, @selector(agonDidHide), AgonBladeAwards);
}

// Show friends blade
- (void)showFriendsPressed:(id)sender {
	[self stopGame];
	// We Show AGON with the profile view
	AgonShow(self, @selector(agonDidHide), AgonBladeFriends);
}

// Show profile blade
- (void)showProfilePressed:(id)sender {
	[self stopGame];
	// We Show AGON with the profile view
	AgonShow(self, @selector(agonDidHide), AgonBladeProfile);
}

// Setup the game view manually.
- (void)loadView {
	// Simple white background
	self.view = [[[UIView alloc] initWithFrame:[UIScreen mainScreen].bounds] autorelease];
	self.view.backgroundColor = [UIColor whiteColor];
	
	// Initiate the award handler that will be displaying awards as they are unlocked.
	_awardViewController = [[AwardViewController alloc] init];
	[self.view addSubview:_awardViewController.view];
	
	// The label holding the random number
	_randomNumberLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 70, 320, 80)];
	_randomNumberLabel.text = @"0000000";
	_randomNumberLabel.textAlignment = UITextAlignmentCenter;
	_randomNumberLabel.font = [UIFont fontWithName:@"Helvetica-Bold" size:40];
	[self.view addSubview:_randomNumberLabel];
	
	// "I Feel Lucky" button
	_playGameButton = [[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
	[_playGameButton setTitle:@"Start" forState:UIControlStateNormal];
	[_playGameButton addTarget:self action:@selector(getScorePressed:) forControlEvents:UIControlEventTouchUpInside];
	_playGameButton.frame = CGRectMake(160-75, 130, 150, 100);
	[self.view addSubview:_playGameButton];
	
	// "Difficulty" button
	_difficultyButton = [[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
	[_difficultyButton setTitle:[self getDifficultyString] forState:UIControlStateNormal];
	[_difficultyButton addTarget:self action:@selector(difficultyPressed:) forControlEvents:UIControlEventTouchUpInside];
	_difficultyButton.frame = CGRectMake(160-55, 250, 110, 30);
	[self.view addSubview:_difficultyButton];
	
	// "Show Leaderboards" button
	_showLeaderboardsButton = [[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
	[_showLeaderboardsButton setTitle:@"Leaderboards" forState:UIControlStateNormal];
	[_showLeaderboardsButton addTarget:self action:@selector(showHighscorePressed:) forControlEvents:UIControlEventTouchUpInside];
	_showLeaderboardsButton.frame = CGRectMake(160-55, 290, 110, 30);
	[self.view addSubview:_showLeaderboardsButton];
	
	// "Show Awards" button
	_showAwardsButton = [[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
	[_showAwardsButton setTitle:@"Awards" forState:UIControlStateNormal];
	[_showAwardsButton addTarget:self action:@selector(showAwardsPressed:) forControlEvents:UIControlEventTouchUpInside];
	_showAwardsButton.frame = CGRectMake(160-55, 330, 110, 30);
	[self.view addSubview:_showAwardsButton];
	
	// "Show Friends" button
	_showFriendsButton = [[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
	[_showFriendsButton setTitle:@"Friends" forState:UIControlStateNormal];
	[_showFriendsButton addTarget:self action:@selector(showFriendsPressed:) forControlEvents:UIControlEventTouchUpInside];
	_showFriendsButton.frame = CGRectMake(160-55, 370, 110, 30);
	[self.view addSubview:_showFriendsButton];
	
	// "Show Profile" button
	_showProfileButton = [[UIButton buttonWithType:UIButtonTypeRoundedRect] retain];
	[_showProfileButton setTitle:@"Profile" forState:UIControlStateNormal];
	[_showProfileButton addTarget:self action:@selector(showProfilePressed:) forControlEvents:UIControlEventTouchUpInside];
	_showProfileButton.frame = CGRectMake(160-55, 410, 110, 30);
	[self.view addSubview:_showProfileButton];
	
	// Get the best score or set it to none.
	NSString* bestScore = AgonGetActiveProfileBestDisplayScore(_difficulty);
	if (!bestScore) {
		bestScore = @"None";
	}
	
	// The label holding the best score
	_bestScoreLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 450, 320, 25)];
	_bestScoreLabel.text = [NSString stringWithFormat:@"Best Score: %@", bestScore];
	_bestScoreLabel.textAlignment = UITextAlignmentCenter;
	_bestScoreLabel.font = [UIFont fontWithName:@"Helvetica-Bold" size:16];
	_bestScoreLabel.backgroundColor = [UIColor clearColor];
	[self.view addSubview:_bestScoreLabel];
	
	// Start the game
	[self startGame];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	UIInterfaceOrientation orientations[] = { UIInterfaceOrientationPortrait, UIInterfaceOrientationPortraitUpsideDown, UIInterfaceOrientationLandscapeLeft, UIInterfaceOrientationLandscapeRight };
	return AgonShouldAutorotateToInterfaceOrientation(orientations, 4, interfaceOrientation);
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration {
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:duration];
	[UIView setAnimationCurve: UIViewAnimationCurveEaseInOut];
	
	// From portrait to landscape.
	if(UIInterfaceOrientationIsPortrait(self.interfaceOrientation) && UIInterfaceOrientationIsLandscape(toInterfaceOrientation)) {
		_randomNumberLabel.frame = CGRectMake(0, 70, 480, 70);
		_playGameButton.frame = CGRectMake(240-75, 130, 150, 100);
		_difficultyButton.frame = CGRectMake(240-55, 240, 110, 30);
		_showLeaderboardsButton.frame = CGRectMake(100-55, 240, 110, 30);
		_showAwardsButton.frame = CGRectMake(100-55, 280, 110, 30);
		_showFriendsButton.frame = CGRectMake(380-55, 240, 110, 30);
		_showProfileButton.frame = CGRectMake(380-55, 280, 110, 30);
		_bestScoreLabel.frame = CGRectMake(0, 282, 480, 25);
	}
	// From landscape to portrait.
	else if(UIInterfaceOrientationIsLandscape(self.interfaceOrientation) && UIInterfaceOrientationIsPortrait(toInterfaceOrientation)) {
		_randomNumberLabel.frame = CGRectMake(0, 70, 320, 70);
		_playGameButton.frame = CGRectMake(160-75, 130, 150, 100);
		_difficultyButton.frame = CGRectMake(160-55, 250, 110, 30);
		_showLeaderboardsButton.frame = CGRectMake(160-55, 290, 110, 30);
		_showAwardsButton.frame = CGRectMake(160-55, 330, 110, 30);
		_showFriendsButton.frame = CGRectMake(160-55, 370, 110, 30);
		_showProfileButton.frame = CGRectMake(160-55, 410, 110, 30);
		_bestScoreLabel.frame = CGRectMake(0, 450, 320, 25);
	}
	
	[UIView commitAnimations];
}

- (NSString*)getDifficultyString {
	if(_difficulty == LEADERBOARD_EASY) {
		return @"Easy";
	}
	else if(_difficulty == LEADERBOARD_MEDIUM) {
		return @"Medium";
	}
	else if(_difficulty == LEADERBOARD_HARD) {
		return @"Hard";		
	}
	else {
		assert(_difficulty == LEADERBOARD_EXTREME);
		return @"Extreme";		
	}
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; 
}

@end
