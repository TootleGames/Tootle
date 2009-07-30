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
#import "OFUserCell.h"
#import "OFViewHelper.h"
#import "OFImageView.h"
#import "OFUser.h"

@implementation OFUserCell

@synthesize profilePictureView, nameLabel, lastPlayedGameLabel, gamerScoreLabel;

- (void)onResourceChanged:(OFResource*)resource
{
	OFUser* user = (OFUser*)resource;
	
	nameLabel.text = user.name;
	[profilePictureView useProfilePictureFromUser:user];
	lastPlayedGameLabel.text = [NSString stringWithFormat:@"Last played %@", user.lastPlayedGameName];
	gamerScoreLabel.text = [NSString stringWithFormat:@"%u", user.gamerScore];
}

- (void)dealloc
{
	self.nameLabel = nil;
	self.profilePictureView = nil;
	self.lastPlayedGameLabel = nil;
	self.gamerScoreLabel = nil;
	[super dealloc];
}

@end
