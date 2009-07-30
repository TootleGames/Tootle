////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFResumeView.h"
#import "OpenFeint+Private.h"

namespace 
{
	const float kGameIconSize = 20.f;
	const float kGameIconSizeLandscape = 15.f;
	const float kGameIconRightInset = 20.f;
	const float kStandardRightNavBarItemRightInset = 5.f;
	// Our nav bar has a drop shadow making it non standard size
	const float kNavBarHeightPortrait = 44.f;
	const float kNavBarHeightLandscape = 39.f;
}
@implementation OFResumeView

- (id)initWithAction:(SEL)action andTarget:(id)target
{
	self = [super initWithFrame:CGRectZero];
	if (self)
	{
		mAction = action;
		mTarget = target;
		const float navBarHeight = [OpenFeint isInLandscapeMode] ? kNavBarHeightLandscape : kNavBarHeightPortrait;
		mBackgroundView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"OpenFeintResumeButtonBackground.png"]];
		mBackgroundView.autoresizingMask = UIViewAutoresizingNone;
		self.frame = mBackgroundView.frame;
		mBackgroundView.frame = CGRectMake(mBackgroundView.frame.origin.x + kStandardRightNavBarItemRightInset, 
										  navBarHeight - mBackgroundView.frame.size.height,
										  mBackgroundView.frame.size.width,
										  mBackgroundView.frame.size.height);
		
		self.autoresizingMask = UIViewAutoresizingNone;
		[self addSubview:mBackgroundView];
		NSString* buttonIconPath = [OpenFeint getResumeIconFileName];
		if (buttonIconPath)
		{
			const float iconSize = [OpenFeint isInLandscapeMode] ? kGameIconSizeLandscape : kGameIconSize;
			UIImage* buttonIcon = [UIImage imageNamed:buttonIconPath];
			UIImageView* iconView = [[[UIImageView alloc] initWithImage:buttonIcon] autorelease];
			iconView.frame = CGRectMake(self.frame.size.width - iconSize * 0.5f - kGameIconRightInset, self.frame.size.height * 0.5f - iconSize * 0.5f, iconSize, iconSize);
			iconView.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin;
			iconView.userInteractionEnabled = NO;
			[self addSubview:iconView];
		}		
	}
	return self;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	[mTarget performSelector:mAction];
}

- (void)dealloc
{
	OFSafeRelease(mBackgroundView);
	[super dealloc];
}

@end
