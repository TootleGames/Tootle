//  AwardViewController.m

//  Copyright Aptocore ApS 2009. All rights reserved.

#import "AwardViewController.h"
#import "AGON.h"

// The vertical position for the award bar when it isn't shown.
const float AWARD_TARGET_FOR_HIDDEN = -100.0f;
// The vertical position for the award bar when it is shown.
const float AWARD_TARGET_FOR_SHOWN = 0.0f;
// The duration (in seconds) in which the award will remain in view when it is unlocked.
const float AWARD_TIME_TO_SHOW = 2.5f;

// Internal methods
@interface AwardViewController (Private)
- (CGRect)getAwardBarFrameWithYpos:(CGFloat)yPos;
- (void)showPendingAwards;
- (void)hideAward:(id)sender;
@end

@implementation AwardViewController

- (id)init {
	self = [super init];
	if (self != nil) {	
		// Create the list that holds the awards that have just been unlocked and needs to be displayed.
		_pendingAwards = [NSMutableArray new];
		_awardBar = nil;
		_awardIcon = nil;
		_awardLabel = nil;
   	    _awardTitle = nil;
		
		[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	}
	
	return self;
}

- (void)dealloc {	
	// Release the container for the awards that were queued to be displayed.
	[_pendingAwards release];
	
	[_awardBar release];
	[_awardIcon release];
	[_awardLabel release];
	[_awardTitle release];
	
	[[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
	
	[super dealloc];
}

- (CGRect)getAwardBarFrameWithYpos:(CGFloat)yPos {
	// The simulator initially reports UIDeviceOrientationUnknown - we assume it to in portrait mode initially.
	UIDeviceOrientation orient = [UIDevice currentDevice].orientation;
	if(orient == UIDeviceOrientationUnknown || UIDeviceOrientationIsPortrait(orient)) {
		return CGRectMake(160-150, yPos, 300, 74);
	}
	else {
		return CGRectMake(240-150, yPos, 300, 74);
	}
}

- (void)loadView {
	// Simple white background
	self.view = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, 320, 80)] autorelease];
	self.view.backgroundColor = [UIColor whiteColor];
	
	// Setup the award Bar.
	_awardBar = [[UIImageView alloc] initWithFrame:[self getAwardBarFrameWithYpos:AWARD_TARGET_FOR_HIDDEN]];
	_awardBar.image = [UIImage imageNamed:@"Awardbar.png"];
	[self.view addSubview:_awardBar];
	
	// Setup the award icon.
	_awardIcon = [[UIImageView alloc] initWithFrame:CGRectMake(15, 5, 48, 48)];
	[_awardBar addSubview:_awardIcon];
	
	// Setup the award label.
	_awardLabel = [[UILabel alloc] initWithFrame:CGRectMake(70, 5, 200, 15)];
	_awardLabel.text = @"Award Earned!";
	_awardLabel.textColor = [UIColor darkGrayColor];
	_awardLabel.backgroundColor = [UIColor clearColor];
	_awardLabel.font = [UIFont fontWithName:@"Verdana" size:14];
	[_awardBar addSubview:_awardLabel];
	
	// Setup the award title.
	_awardTitle = [[UILabel alloc] initWithFrame:CGRectMake(70, 25, 200, 25)];
	_awardTitle.backgroundColor = [UIColor clearColor];
	_awardTitle.font = [UIFont fontWithName:@"Verdana-Bold" size:16];
	[_awardBar addSubview:_awardTitle];
	
	// Initially hidden.
	self.view.hidden = YES;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (void)unlockAwardWithId:(AgonAwardId)awardId {
	// Try to unlock the award. This will fail if the award has already been unlocked.
	if (!AgonUnlockAwardWithId(awardId)) return;
	
	// Queue the award to be displayed (Lucky's award ids go from [0-9]).
	NSNumber* num = [NSNumber numberWithInt:awardId];
	[_pendingAwards addObject:num];
	
	// Show the player that he/she has just unlocked a cool award.
	[self showPendingAwards];
}

// Starts displaying any queued, unlocked awards.
- (void)showPendingAwards {
	// If no awards are ready to be displayed, early out.
	if ([_pendingAwards count] == 0) return;
	
	// If an award is currently being displayed, abort.
	if (self.view.hidden == NO) return;
	
	// Pop the index for the award that is about to be displayed from the front of the queue.
	NSNumber* num = [_pendingAwards objectAtIndex:0];
	[_pendingAwards removeObjectAtIndex:0];
	int awardId = [num intValue];
	
	// Setup the UI elements to reflect the data for the current award.
	_awardTitle.text = AgonGetAwardTitleWithId(awardId);
	_awardIcon.image = AgonGetAwardImageWithId(awardId);
	
	// Show the award bar.
	self.view.hidden = NO;
	
	// Start the animation of the award bar.
	_awardBar.frame = [self getAwardBarFrameWithYpos:AWARD_TARGET_FOR_HIDDEN];
	[UIView beginAnimations:nil context:nil];
	_awardBar.frame = [self getAwardBarFrameWithYpos:AWARD_TARGET_FOR_SHOWN];
	[UIView commitAnimations];
	[self performSelector:@selector(hideAward:) withObject:_awardBar afterDelay:AWARD_TIME_TO_SHOW];
}

// Callback which is fired when an award has completed its animation and is no longer visible.
- (void)hideAwardDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context {
	self.view.hidden = YES;
	
	// Show any awards that were unlocked while this award was being displayed.
	[self showPendingAwards];
}

// Hides the award bar by moving it out of the screen.
- (void)hideAward:(id)sender {
	_awardBar.frame = [self getAwardBarFrameWithYpos:AWARD_TARGET_FOR_SHOWN];
	[UIView beginAnimations:nil context:nil];
	_awardBar.frame = [self getAwardBarFrameWithYpos:AWARD_TARGET_FOR_HIDDEN];
	[UIView setAnimationDidStopSelector:@selector(hideAwardDidStop:finished:context:)];
	[UIView setAnimationDelegate:self];
	[UIView commitAnimations];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; 
}

@end
