////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Copyright (c) 2009 Aurora Feint Inc.
///
///  This library is free software; you can redistribute it and/or
///  modify it under the terms of the GNU Lesser General Public
///  License as published by the Free Software Foundation; either
///  
///  version 3 of the License, or (at your option) any later version.
///  
///  This library is distributed in the hope that it will be useful,
///  
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
///  Lesser General Public License for more details.
///  
///  
///  You should have received a copy of the GNU Lesser General Public
///  License along with this library; if not, write to the Free Software
///  
///  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFProfileFramedView.h"
#import "OFUser.h"
#import "OFUserService.h"
#import "OFImageView.h"
#import "OFViewHelper.h"
#import "OFProfileController.h"
#import "OpenFeint+UserOptions.h"
#import "OpenFeint+Private.h"

namespace OFProfileFrameTags
{
	static NSInteger const kPicture		= 500;
	static NSInteger const kName		= 501;
	static NSInteger const kGame		= 502;
	static NSInteger const kScore		= 503;
	static NSInteger const kOverlay		= 504;
	static NSInteger const kLeftBorder	= 505;
	static NSInteger const kRightBorder	= 506;
	static NSInteger const kScoreIcon	= 507;
}

@implementation OFProfileFramedView

@synthesize contentView;

- (void)_userChanged:(OFUser*)newUser
{
	OFSafeRelease(user);
	user = [newUser retain];
	NSString* playedGamePrefixText = @"Last played ";
	if ([user.resourceId isEqualToString:[OpenFeint lastLoggedInUserId]])
	{
		playedGamePrefixText = @"Playing ";
		if (user.profilePictureUrl && [[OpenFeint lastLoggedInUserProfilePictureUrl] isEqualToString:@""])
		{
			[OpenFeint setLastLoggedInUserProfilePictureUrl:user.profilePictureUrl];
			[OpenFeint setLastLoggedInUserUsesFacebookProfilePicture:user.usesFacebookProfilePicture];
		}
	}
		
	OFImageView* profilePicture = (OFImageView*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kPicture);
	UILabel* feintName = (UILabel*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kName);
	UILabel* lastPlayedGame = (UILabel*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kGame);
	UILabel* gamerscore = (UILabel*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kScore);
	
	[profilePicture useProfilePictureFromUser:user];
	if (user != nil)
	{
		feintName.text = user.name;
		gamerscore.text = [NSString stringWithFormat:@"%d", user.gamerScore];
		lastPlayedGame.text = [NSString stringWithFormat:@"%@%@", playedGamePrefixText, user.lastPlayedGameName];
	}
	else
	{
		feintName.text = @"OpenFeint User";
		gamerscore.text = @"0";
		lastPlayedGame.text = [NSString stringWithFormat:@"%@Unknown Game", playedGamePrefixText];
	}
	
	if ([framedController respondsToSelector:@selector(onProfileUserLoaded:)])
	{
		[framedController onProfileUserLoaded:user];
	}
	OFSafeRelease(framedController);
}

- (void)_userDownloadFailed
{
	[self _userChanged:nil];
}

- (void)_userDownloadSucceeded:(NSArray*)resources
{
	OFAssert([resources count] == 1, "Must only have one resource!");	
	if ([resources count] == 1)
	{
		OFUser* newUser = [(OFUser*)[resources objectAtIndex:0] retain];
		[self _userChanged:newUser];
	}
	else
	{
		[self _userDownloadFailed];
	}
	
}

- (id)initWithCoder:(NSCoder*)aDecoder
{
	self = [super initWithCoder:aDecoder];
	if (self != nil)
	{
		UILabel* feintName = (UILabel*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kName);
		UILabel* lastPlayedGame = (UILabel*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kGame);
		UIImageView* frameOverlayView = (UIImageView*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kOverlay);

		UIImage* overlayImage = nil;
		CGRect frameOverlayRect = frameOverlayView.frame;
		if ([OpenFeint isInLandscapeMode])
		{
			// OF2.0UI
//			UIImageView* topBorder = (UIImageView*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kLeftBorder);
//			CGRect topBorderFrame = topBorder.frame;
//			topBorderFrame.size.width = self.frame.size.width - frameOverlayRect.size.width;
//			topBorderFrame.origin.x = frameOverlayRect.size.width;
//			topBorder.image = [topBorder.image stretchableImageWithLeftCapWidth:1 topCapHeight:0];
//			[topBorder setFrame:topBorderFrame];

			// OF2.0UI
//			overlayImage = [frameOverlayView.image stretchableImageWithLeftCapWidth:0 topCapHeight:120];
			overlayImage = [frameOverlayView.image stretchableImageWithLeftCapWidth:0 topCapHeight:15];
			frameOverlayRect.size.height = self.frame.size.height;

			// ONLY for old look & feel
			UILabel* gamerscore = (UILabel*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kScore);
			gamerscore.transform = CGAffineTransformMake(0.f, -1.f, 1.f, 0.f, 0.f, 0.f);
			UIImageView* scoreIcon = (UIImageView*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kScoreIcon);
			scoreIcon.transform = CGAffineTransformMake(0.f, -1.f, 1.f, 0.f, 0.f, 0.f);
			
			UIImageView* bottomBorder = (UIImageView*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kRightBorder);
			CGRect bottomBorderFrame = bottomBorder.frame;
			bottomBorderFrame.origin.y = self.frame.size.height - bottomBorderFrame.size.height;
			[bottomBorder setFrame:bottomBorderFrame];

			feintName.transform = CGAffineTransformMake(0.f, -1.f, 1.f, 0.f, 0.f, 0.f);
			lastPlayedGame.transform = CGAffineTransformMake(0.f, -1.f, 1.f, 0.f, 0.f, 0.f);

			contentFrame = self.frame;
			contentFrame.origin.x += 66.0f;
			contentFrame.origin.y += 9.0f;
			contentFrame.size.height -= 18.0f;
			contentFrame.size.width -= 66.0f;
			// OF2.0UI
//			contentFrame.origin.x += 64.0f;
//			contentFrame.origin.y += 5.0f;
//			contentFrame.size.height -= 5.0f;
//			contentFrame.size.width -= 69.0f;
		}
		else
		{
			overlayImage = [frameOverlayView.image stretchableImageWithLeftCapWidth:65 topCapHeight:0];
			frameOverlayRect.size.width = self.frame.size.width;

			contentFrame = self.frame;
			contentFrame.origin.x += 9.0f;
			contentFrame.origin.y += 66.0f;
			contentFrame.size.height -= 66.0f;
			contentFrame.size.width -= 18.0f;
			// OF2.0UI
//			contentFrame.origin.x += 5.0f;
//			contentFrame.origin.y += 64.0f;
//			contentFrame.size.height -= 64.0f;
//			contentFrame.size.width -= 10.0f;
		}

		frameOverlayView.image = overlayImage;
		frameOverlayView.frame = frameOverlayRect;
	}
	
	return self;
}

- (void)dealloc
{
	[contentView removeFromSuperview];
	OFSafeRelease(contentView);
	OFSafeRelease(user);	
	OFSafeRelease(framedController);
	[super dealloc];
}

- (void)setFrame:(CGRect)_frame
{
	[super setFrame:_frame];

	if ([OpenFeint isInLandscapeMode])
	{
		UILabel* feintName = (UILabel*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kName);
		UILabel* lastPlayedGame = (UILabel*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kGame);
		UILabel* gamerscore = (UILabel*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kScore);

		float widthToUse = self.frame.size.height - 58.f;	// 58.f is the offset from the bottom in the frame overlay image

		CGRect bounds;
		CGPoint center;
		
		bounds = feintName.bounds;
		center = feintName.center;
		bounds.size.width = widthToUse;
		center.y = widthToUse * 0.5f;
		feintName.bounds = bounds;
		feintName.center = center;

		bounds = lastPlayedGame.bounds;
		center = lastPlayedGame.center;
		bounds.size.width = widthToUse;
		center.y = widthToUse * 0.5f;
		lastPlayedGame.bounds = bounds;
		lastPlayedGame.center = center;

		widthToUse -= 18.f;	// extra space for the gamerscore icon
		bounds = gamerscore.bounds;
		center = gamerscore.center;
		bounds.size.width = widthToUse;
		center.y = widthToUse * 0.5f;
		gamerscore.bounds = bounds;
		gamerscore.center = center;
	}
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

- (void)setUserId:(NSString*)userId
{
	OFImageView* profilePicture = (OFImageView*)OFViewHelper::findViewByTag(self, OFProfileFrameTags::kPicture);
	if (userId == nil || [userId isEqualToString:[OpenFeint lastLoggedInUserId]])
	{
		isDisplayingLocalUser = YES;
		[profilePicture useLocalPlayerProfilePictureDefault];
	}
	else
	{
		isDisplayingLocalUser = NO;
		[profilePicture useOtherPlayerProfilePictureDefault];
	}

	[OFUserService 
		getUser:userId
		onSuccess:OFDelegate(self, @selector(_userDownloadSucceeded:)) 
		onFailure:OFDelegate(self, @selector(_userDownloadFailed))];		
}

- (void)setController:(UIViewController*)controller
{
	if (![controller conformsToProtocol:@protocol(OFProfileFrame)]) 
		return;
		
	OFSafeRelease(framedController);
	framedController = (UIViewController<OFProfileFrame>*)[controller retain];
	[self setUserId:[framedController getProfileUserId]];
}

- (void)setContentView:(UIView*)newContentView
{
	[contentView removeFromSuperview];
	OFSafeRelease(contentView);

	contentView = [newContentView retain];
	[contentView removeFromSuperview];
	[contentView setFrame:contentFrame];
	if ([contentView isKindOfClass:[UIScrollView class]])
	{
		UIScrollView* scrollView = (UIScrollView*)contentView;
		[scrollView setContentSize:CGSizeMake(contentFrame.size.width, scrollView.contentSize.height)];
	}
	[self addSubview:contentView];
	[self sendSubviewToBack:contentView];
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
	UITouch* touch = [touches anyObject];

	if ([touch tapCount] == 1 && user != nil)
	{
		[OFProfileController showProfileForUser:user.resourceId];
	}	
}

- (void)refreshViewForLocalPlayerOnly
{
	if (isDisplayingLocalUser)
	{
		[self setUserId:[OpenFeint lastLoggedInUserId]];
	}
}

@end