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
#import "OFChatRoomChatMessageCell.h"
#import "OFViewHelper.h"
#import "OFChatMessage.h"
#import "OFStringUtility.h"
#import "OFApplicationDescriptionController.h"
#import "OFControllerLoader.h"
#import "OFProfileController.h"
#import "OFImageView.h"
#import "OFAchievementListController.h"
#import "OFChatRoomController.h"
#import "OFChatRoomInstance.h"
#import "OpenFeint+UserOptions.h"
#import "OpenFeint+Private.h"
#import "NSObject+WeakLinking.h"

@implementation OFChatRoomChatMessageCell

@synthesize owner;

- (bool)shouldGameNameLink:(OFChatMessage*)message
{
	if(message.playerCurrentGameId != nil && ![message.playerCurrentGameId isEqualToString:@"null"] && ![message.playerCurrentGameId isEqualToString:@""])
	{
		if (message.doesLocalPlayerOwnGame || message.playerCurrentGameIconUrl != nil)
		{
			return true;
		}
	}
	return false;
}

- (IBAction)onClickedFeintName
{
	OFChatMessage* message = (OFChatMessage*)mResource;
	
	if(message.playerId != nil && ![message.playerId isEqualToString:@"null"] && ![message.playerId isEqualToString:@""])
	{
		OFProfileController* profileController = (OFProfileController*)OFControllerLoader::load(@"Profile");
		profileController.userId = message.playerId;
		[[owner navigationController] pushViewController:profileController animated:YES];
	}
}

- (IBAction)onClickedGameName
{
	OFChatMessage* message = (OFChatMessage*)mResource;

	if([self shouldGameNameLink:message])
	{
		UIViewController* nextController = nil;
		if (message.doesLocalPlayerOwnGame)	// go to achievement list since our user owns the game
		{
			OFAchievementListController* achievementController = (OFAchievementListController*)OFControllerLoader::load(@"AchievementList");
			achievementController.applicationName = message.playerCurrentGame;
			achievementController.applicationId = message.playerCurrentGameId;
			achievementController.applicationIconUrl = message.playerCurrentGameIconUrl;
			achievementController.doesUserHaveApplication = YES;
			achievementController.userId = message.playerId;
			nextController = achievementController;
		}
		else if (message.playerCurrentGameIconUrl != nil) // game has an ipromote page, so go there
		{
			nextController = [OFApplicationDescriptionController applicationDescriptionForId:message.playerCurrentGameId];
		}
		
		if (nextController)
		{
			[[owner navigationController] pushViewController:nextController animated:YES];
		}
	}
}

- (void)setButtonTitleColor:(UIButton*)button color:(UIColor*)color
{
	[button setTitleColor:color forState:UIControlStateNormal];
	[button setTitleColor:color forState:UIControlStateHighlighted];
	[button setTitleColor:color forState:UIControlStateDisabled];
	[button setTitleColor:color forState:UIControlStateSelected];
}

- (void)onResourceChanged:(OFResource*)resource
{
	OFChatMessage* message = (OFChatMessage*)resource;
	
	self.contentView.autoresizesSubviews = NO;
	
	OFImageView* profilePicture = (OFImageView*)OFViewHelper::findViewByTag(self.contentView, 1);
	UIImageView* pictureFrame = (UIImageView*)OFViewHelper::findViewByTag(self.contentView, 2);
	UIButton* feintUser = (UIButton*)OFViewHelper::findViewByTag(self.contentView, 3);
	UIButton* feintGame = (UIButton*)OFViewHelper::findViewByTag(self.contentView, 4);
	UILabel* chatText = (UILabel*)OFViewHelper::findViewByTag(self.contentView, 5);
	OFImageView* gamePicture = (OFImageView*)OFViewHelper::findViewByTag(self.contentView, 6);
	gamePicture.useSharpCorners = YES;
	UIImageView* chatCellOverlay = (UIImageView*)OFViewHelper::findViewByTag(self.contentView, 7);
	
	NSString* defaultProfilePictureName = @"OFProfileIconDefault.png";
	if (message.playerId == nil || [message.playerId isEqualToString:[OpenFeint lastLoggedInUserId]])
		defaultProfilePictureName = @"OFProfileIconDefaultSelf.png";
	[profilePicture setDefaultImage:[UIImage imageNamed:defaultProfilePictureName]];
	profilePicture.useFacebookOverlay = message.playerUsesFacebookProfilePicture;
	profilePicture.imageUrl = message.playerProfilePictureUrl;
	
	float const kSpaceBetweenProfilePicureAndText = 4.0f;
	float const kSpaceBetweenTextAndGamePicture = 2.0f;
	
	BOOL shouldGameNameLink = [self shouldGameNameLink:message];

	float maxWidth = [OpenFeint getDashboardBounds].size.width - pictureFrame.frame.size.width - gamePicture.frame.size.width;
	maxWidth -= kSpaceBetweenProfilePicureAndText + kSpaceBetweenTextAndGamePicture;

	if ([owner.roomInstance isApplicationRoom] || !shouldGameNameLink)
	{
		gamePicture.hidden = YES;
		maxWidth += gamePicture.frame.size.width;
	}
	else
	{
		[gamePicture setDefaultImage:[UIImage imageNamed:@"OFDefaultGameIconInChat.png"]];
		gamePicture.imageUrl = message.playerCurrentGameIconUrl;
		gamePicture.hidden = NO;
	}
	
	CGRect gamePictureFrame = gamePicture.frame;
	gamePictureFrame.origin.x = [OpenFeint getDashboardBounds].size.width - gamePictureFrame.size.width;
	[gamePicture setFrame:gamePictureFrame];

	UIFont* gameButtonFont = [feintGame tryGet:@"titleLabel.font" elseGet:@"font"];
	UIFont* userButtonFont = [feintUser tryGet:@"titleLabel.font" elseGet:@"font"];

	UIColor* blueColor = [UIColor colorWithRed:0.f green:0.f blue:0.8f alpha:1.f];
	
	CGRect feintUserFrame = feintUser.frame;
	OFRetainedPtr<NSString> playerName = OFStringUtility::convertFromValidParameter(message.playerName);
	if([playerName.get() length])
	{
		feintUserFrame.size.width = [playerName.get() sizeWithFont:userButtonFont constrainedToSize:CGSizeMake(maxWidth, FLT_MAX)].width;
	}
	
	
	[feintUser setFrame:feintUserFrame];
	[feintUser setTitle:playerName.get() forState:UIControlStateNormal];
	if (message.playerId != nil && ![message.playerId isEqualToString:@""])
	{
		[self setButtonTitleColor:feintUser color:blueColor];
	}
	else
	{
		[self setButtonTitleColor:feintUser color:[UIColor blackColor]];
	}
	
	CGRect feintGameFrame = feintGame.frame;
	feintGameFrame.origin.x = feintUserFrame.origin.x + feintUserFrame.size.width + 5.f;
	OFRetainedPtr<NSString> gameName = OFStringUtility::convertFromValidParameter([NSString stringWithFormat:@"(%@)", message.playerCurrentGame]);
	if([gameName.get() length])
	{
		feintGameFrame.size.width = [gameName.get() sizeWithFont:gameButtonFont constrainedToSize:CGSizeMake(maxWidth - feintUserFrame.size.width, FLT_MAX)].width;
	}
	[feintGame setFrame:feintGameFrame];
	[feintGame setTitle:gameName.get() forState:UIControlStateNormal];
	
	if (shouldGameNameLink)
	{
		[self setButtonTitleColor:feintGame color:blueColor];
	}
	else
	{
		[self setButtonTitleColor:feintGame color:[UIColor blackColor]];
	}
	
	
	CGRect chatTextFrame = chatText.frame;
	chatText.text = OFStringUtility::convertFromValidParameter(message.message).get();
	if([chatText.text length])
	{
		chatTextFrame.size = [chatText.text sizeWithFont:chatText.font constrainedToSize:CGSizeMake(maxWidth, FLT_MAX)];
	}
	[chatText setFrame:chatTextFrame];
	
	float const kMinHeight = 54.0f;
	
	CGRect resizedFrame = self.frame;
	resizedFrame.size.height = MAX(kMinHeight, chatText.frame.size.height + chatText.frame.origin.y);
	self.frame = resizedFrame;

	CGRect pictureFrameRect = pictureFrame.frame;
	pictureFrameRect.size.height = resizedFrame.size.height;
	[pictureFrame setFrame:pictureFrameRect];
	[pictureFrame setImage:[pictureFrame.image stretchableImageWithLeftCapWidth:0 topCapHeight:2]];
	
	CGRect chatCellOverlayFrame = chatCellOverlay.frame;
	chatCellOverlayFrame.origin.x = 0.0f;
	chatCellOverlayFrame.size.width = [OpenFeint getDashboardBounds].size.width;
	if ([owner.roomInstance isApplicationRoom] || !shouldGameNameLink)
	{
		chatCellOverlayFrame.size.width += chatCellOverlay.image.size.width;
	}
	chatCellOverlayFrame.size.height = resizedFrame.size.height;
	[chatCellOverlay setFrame:chatCellOverlayFrame];
	[chatCellOverlay setImage:[chatCellOverlay.image stretchableImageWithLeftCapWidth:1 topCapHeight:53]];
}

@end
