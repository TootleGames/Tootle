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
#import "OFLoadingController.h"
#import "OFControllerLoader.h"
#import "OFViewHelper.h"
#import "OpenFeint+Private.h"

namespace 
{
	const int kNoticeTag = 1;
	const int kContentContainerTag = 3;
}

@interface OFLoadingView : UIView
{	
}
@end

@implementation OFLoadingView

- (void)willMoveToSuperview:(UIView *)newSuperview
{
	if (newSuperview)
	{
		self.frame = CGRectMake(0.f, 0.f, newSuperview.frame.size.width, newSuperview.frame.size.height);
		[self setNeedsLayout];
	}
}

- (void)layoutSubviews
{
	UIView* contentContainer = OFViewHelper::findViewByTag(self, kContentContainerTag);
	NSAssert(contentContainer != nil, @"Missing view with tag kContentContainerTag from SubmittingForm controller.");
	float midX = CGRectGetMidX(self.superview.frame) - self.superview.frame.origin.x;
	float midY = CGRectGetMidY(self.superview.frame) - self.superview.frame.origin.y;
	contentContainer.frame = CGRectMake(midX - contentContainer.frame.size.width * 0.5f, 
										midY - contentContainer.frame.size.height * 0.5f,
										contentContainer.frame.size.width,
										contentContainer.frame.size.height);
	[super layoutSubviews];
}

@end


@implementation OFLoadingController

+ (OFLoadingController*)loadingControllerWithText:(NSString*)loadingText
{
	OFLoadingController* controller = (OFLoadingController*)OFControllerLoader::load(@"Loading");
	[controller setLoadingText:loadingText];
	return controller;
}

- (void)setLoadingText:(NSString*)loadingText
{
	UILabel* submittingText = (UILabel*)OFViewHelper::findViewByTag(self.view, kNoticeTag);
	NSAssert(submittingText != nil, @"Missing UILabel view with tag kNoticeTag from SubmittingForm controller.");
	NSAssert([submittingText isKindOfClass:[UILabel class]], @"View with tag kNoticeTag is not of type UILabel in SubmittingForm controller.");
	
	submittingText.text = loadingText;

	[self.view setNeedsDisplay];
}

- (void)loadView
{
	CGRect viewFrame = [UIScreen mainScreen].bounds;
	self.view = [[[OFLoadingView alloc] initWithFrame:viewFrame] autorelease];
	self.view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	self.view.backgroundColor = [UIColor colorWithRed:0.0f green:0.0f blue:0.0f alpha:0.75f];
	self.view.contentMode = UIViewContentModeCenter;
	
	CGSize arbitraryPaddedContentSize = CGSizeMake(200.f, 100.f);
	UIView* contentContainer = [[[UIView alloc] initWithFrame:CGRectMake(CGRectGetMidX(viewFrame) - arbitraryPaddedContentSize.width * 0.5f, 
																		 CGRectGetMidY(viewFrame) - arbitraryPaddedContentSize.height * 0.5f, 
																		 arbitraryPaddedContentSize.width,
																		 arbitraryPaddedContentSize.height)] autorelease];
	contentContainer.opaque = NO;
	contentContainer.backgroundColor = [UIColor clearColor];
	contentContainer.tag = kContentContainerTag;
	
	UIActivityIndicatorView* indicator = [[[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge] autorelease];
	[indicator startAnimating];
	indicator.center = CGPointMake(arbitraryPaddedContentSize.width * 0.5f, arbitraryPaddedContentSize.height * 0.5f);
	[contentContainer addSubview:indicator];
	
	UILabel* notice = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
	notice.tag = kNoticeTag;
	const float arbitraryHeightThatLooksGood = 40.0f;
	notice.frame = CGRectMake(0.0f, indicator.frame.origin.y + indicator.frame.size.height, arbitraryPaddedContentSize.width, arbitraryHeightThatLooksGood);
	notice.text = @"Loading";
	notice.textColor = [UIColor whiteColor];
	notice.backgroundColor = [UIColor clearColor];
	notice.textAlignment = UITextAlignmentCenter;
	notice.numberOfLines = 0;
	[contentContainer addSubview:notice];

	[self.view addSubview:contentContainer];
	
	self.view.alpha = 0.0f;
	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:0.25f];
	self.view.alpha = 1.0f;
	[UIView commitAnimations];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

@end