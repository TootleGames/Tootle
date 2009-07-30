////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFDependencies.h"
#import "OFWhosPlayingCell.h"
#import "OFViewHelper.h"
#import "OFImageView.h"
#import "OFGamePlayer.h"
#import "OFUser.h"

@implementation OFWhosPlayingCell

@synthesize profilePictureView, nameLabel, lastPlayedGameLabel, gamerScoreLabel, appGamerScoreLabel;

- (void)onResourceChanged:(OFResource*)resource
{
	OFGamePlayer* player = (OFGamePlayer*)resource;
	OFUser* user = player.user;
		
	NSString* lastPlayedText = @"Last played %@";
	if ([user isLocalUser])
		lastPlayedText = @"Playing %@";

	nameLabel.text = user.name;
	[profilePictureView useProfilePictureFromUser:user];
	lastPlayedGameLabel.text = [NSString stringWithFormat:lastPlayedText, user.lastPlayedGameName];
	appGamerScoreLabel.text = [NSString stringWithFormat:@"%u", player.applicationGamerscore];
	gamerScoreLabel.text = [NSString stringWithFormat:@"%u", user.gamerScore];
}

- (void)dealloc
{
	self.nameLabel = nil;
	self.profilePictureView = nil;
	self.lastPlayedGameLabel = nil;
	self.appGamerScoreLabel = nil;
	self.gamerScoreLabel = nil;
	[super dealloc];
}

@end
