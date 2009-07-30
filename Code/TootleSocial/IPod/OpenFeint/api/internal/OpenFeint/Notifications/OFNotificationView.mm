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

#import "OFDependencies.h"
#import "OFNotificationView.h"
#import "OFDelegate.h"
#import "MPOAuthAPIRequestLoader.h"
#import "OFDelegateChained.h"
#import "OpenFeint+Private.h"
#import "OFNotificationInputResponse.h"
#import "OFControllerLoader.h"
#import <QuartzCore/QuartzCore.h>

static const float gNotificationWaitSeconds = 2.5f; 

@interface OFNotificationView()

- (void)_calcFrameAndTransform;
- (CGPoint)_calcOffScreenPosition:(CGPoint)onScreenPosition;

@end

@implementation OFNotificationView

@synthesize notice;
@synthesize statusIndicator;
@synthesize backgroundImage;

- (void)animationDidStop:(CABasicAnimation *)theAnimation finished:(BOOL)flag
{
	[[self layer] removeAnimationForKey:[theAnimation keyPath]];
	[self removeFromSuperview];
}

- (void)_animateKeypath:(NSString*)keyPath 
			  fromValue:(float)startValue 
				toValue:(float)endValue 
			   overTime:(float)duration
	  animationDelegate:(UIView*)animDelegate
	 removeOnCompletion:(BOOL)removeOnCompletion
			   fillMode:(NSString*)fillMode
{
	CABasicAnimation* animation = [CABasicAnimation animationWithKeyPath:keyPath];
	animation.fromValue = [NSNumber numberWithFloat:startValue];
	animation.toValue = [NSNumber numberWithFloat:endValue];
	animation.duration = duration;
	animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
	animation.delegate = animDelegate;
	animation.removedOnCompletion = removeOnCompletion;
	animation.fillMode = fillMode;
	[[self layer] addAnimation:animation forKey:nil];
}

- (void)_animateFromPosition:(CGPoint)startPos 
				  toPosition:(CGPoint)endPos 
					overTime:(float)duration
		   animationDelegate:(UIView*)animDelegate
		  removeOnCompletion:(BOOL)removeOnCompletion
					fillMode:(NSString*)fillMode

{
	if (startPos.x != endPos.x)
	{
		[self _animateKeypath:@"position.x" 
					fromValue:startPos.x 
					  toValue:endPos.x 
					 overTime:duration 
			animationDelegate:animDelegate 
		   removeOnCompletion:removeOnCompletion 
					 fillMode:fillMode];
	}
	if (startPos.y != endPos.y)
	{
		[self _animateKeypath:@"position.y" 
					fromValue:startPos.y
					  toValue:endPos.y 
					 overTime:duration 
			animationDelegate:animDelegate 
		   removeOnCompletion:removeOnCompletion 
					 fillMode:fillMode];
	}
}

- (void)_dismiss
{
	CGPoint onScreenPosition = self.layer.position;
	[self _animateFromPosition:onScreenPosition
					toPosition:[self _calcOffScreenPosition:onScreenPosition]
					  overTime:0.5f
			 animationDelegate:self
			removeOnCompletion:NO
					  fillMode:kCAFillModeForwards];
}

- (void)_present
{
	CGPoint onScreenPosition = self.layer.position;
	[self _animateFromPosition:[self _calcOffScreenPosition:onScreenPosition]
					toPosition:onScreenPosition
					  overTime:0.25f
			 animationDelegate:nil
			removeOnCompletion:YES
					  fillMode:kCAFillModeRemoved];

	[presentationView addSubview:self];
	OFSafeRelease(presentationView);
}

- (void)_makeStatusIconActiveAndDismiss:(OFNotificationStatus*)status
{
	[self _present];

	self.statusIndicator.image = [UIImage imageNamed:status];	
	self.statusIndicator.hidden = NO;
	[self performSelector:@selector(_dismiss) withObject:nil afterDelay:gNotificationWaitSeconds];
}

- (void)_requestSucceeded:(MPOAuthAPIRequestLoader*)request nextCall:(OFDelegateChained*)nextCall
{
	[self _makeStatusIconActiveAndDismiss:OFNotificationStatusSuccess];					
	[nextCall invokeWith:request];
}

- (void)_requestFailed:(MPOAuthAPIRequestLoader*)request nextCall:(OFDelegateChained*)nextCall
{
	[self _makeStatusIconActiveAndDismiss:OFNotificationStatusFailure];
	[nextCall invokeWith:request];
}

+ (void)showNotificationWithText:(NSString*)noticeText andStatus:(OFNotificationStatus*)status inView:(UIView*)containerView withInputResponse:(OFNotificationInputResponse*)inputResponse
{
	OFNotificationView* view = (OFNotificationView*)OFControllerLoader::loadView(@"NotificationView");
	[view configureWithText:noticeText andStatus:status inView:containerView withInputResponse:inputResponse];
}

+ (void)showNotificationWithRequest:(MPOAuthAPIRequestLoader*)request andNotice:(NSString*)noticeText inView:(UIView*)containerView withInputResponse:(OFNotificationInputResponse*)inputResponse
{
	OFNotificationView* view = (OFNotificationView*)OFControllerLoader::loadView(@"NotificationView");
	[view configureWithRequest:request andNotice:noticeText inView:containerView withInputResponse:inputResponse];
}

- (BOOL)isParentViewRotatedInternally:(UIView*)parentView
{
	CGRect parentBounds = parentView.bounds;
	bool parentIsLandscape = parentBounds.size.width > ([UIScreen mainScreen].bounds.size.height + [UIScreen mainScreen].bounds.size.width) * 0.5f;
	bool dashboardIsLandscape = [OpenFeint isInLandscapeMode];
	return dashboardIsLandscape && !parentIsLandscape;
}

- (CGPoint)_calcOffScreenPosition:(CGPoint)onScreenPosition
{
	CGSize notificationSize = self.bounds.size;
	if (mParentViewIsRotatedInternally)
	{
		UIInterfaceOrientation dashboardOrientation = [OpenFeint getDashboardOrientation];
		float offScreenOffsetX = 0.f;
		float offScreenOffsetY = 0.f;
		switch (dashboardOrientation)
		{
			case UIInterfaceOrientationLandscapeRight:		offScreenOffsetX = -notificationSize.height;	break;
			case UIInterfaceOrientationLandscapeLeft:		offScreenOffsetX = notificationSize.height;		break;
			case UIInterfaceOrientationPortraitUpsideDown:	offScreenOffsetY = -notificationSize.height;	break;
			case UIInterfaceOrientationPortrait:			offScreenOffsetY = notificationSize.height;		break;
		}
		return CGPointMake(onScreenPosition.x + offScreenOffsetX, onScreenPosition.y + offScreenOffsetY);
	}
	else
	{
		return CGPointMake(onScreenPosition.x, onScreenPosition.y + notificationSize.height);
	}
}

- (void)_calcFrameAndTransform
{
	OFAssert(presentationView != nil, "You must have called [self _setPresentationView:] before this!");
	
	CGRect parentBounds = presentationView.bounds;
	const float kNotificationHeight = self.frame.size.height;
	CGRect notificationRect = CGRectZero;
	UIInterfaceOrientation dashboardOrientation = [OpenFeint getDashboardOrientation];
	mParentViewIsRotatedInternally = [self isParentViewRotatedInternally:presentationView];
	if (mParentViewIsRotatedInternally)
	{	
		CGSize notificationSize = CGSizeMake([OpenFeint isInLandscapeMode] ? parentBounds.size.height : parentBounds.size.width, kNotificationHeight);
		notificationRect = CGRectMake(
									  -notificationSize.width * 0.5f,
									  -notificationSize.height * 0.5f, 
									  notificationSize.width, 
									  notificationSize.height
									  );
		
		CGAffineTransform newTransform = CGAffineTransformIdentity;
		
		if (dashboardOrientation == UIInterfaceOrientationLandscapeRight) 
		{
			newTransform = CGAffineTransformMake(0, 1, -1, 0, 
												 notificationSize.height * 0.5f, 
												 parentBounds.size.height * 0.5f);
		}
		else if (dashboardOrientation == UIInterfaceOrientationLandscapeLeft)
		{
			newTransform = CGAffineTransformMake(0, -1, 1, 0, 
												 parentBounds.size.width - notificationSize.height * 0.5f, 
												 parentBounds.size.height * 0.5f);
		}
		else if (dashboardOrientation == UIInterfaceOrientationPortraitUpsideDown)
		{
			newTransform = CGAffineTransformMake(-1, 0, 0, -1, parentBounds.size.width * 0.5f, notificationSize.height * 0.5f);
		}
		else
		{
			newTransform = CGAffineTransformTranslate(newTransform, 0, parentBounds.size.height - notificationSize.height * 0.5f);
		}
		
		self.frame = notificationRect;
		[self setTransform:newTransform];
	}
	else
	{
		CGSize notificationSize = CGSizeMake(parentBounds.size.width, kNotificationHeight);
		notificationRect = CGRectMake(
									  (parentBounds.size.width - notificationSize.width) * 0.5f, 
									  parentBounds.size.height - notificationSize.height, 
									  notificationSize.width, 
									  notificationSize.height
									  );
		self.frame = notificationRect;
	}
}

- (void)_setPresentationView:(UIView*)_presentationView
{
	OFSafeRelease(presentationView);
	presentationView = [_presentationView retain];	
	[self _calcFrameAndTransform];
}

- (void)_buildViewWithText:(NSString*)noticeText
{
	statusIndicator.hidden = YES;
	notice.text = noticeText;
	[backgroundImage setContentMode:UIViewContentModeScaleToFill];
	[backgroundImage setImage:[backgroundImage.image stretchableImageWithLeftCapWidth:(backgroundImage.image.size.width - 2) topCapHeight:0]];
}

- (void)configureWithText:(NSString*)noticeText andStatus:(OFNotificationStatus*)status inView:(UIView*)containerView withInputResponse:(OFNotificationInputResponse*)inputResponse
{
	mInputResponse = [inputResponse retain];
	[self _setPresentationView:containerView];
	[self _buildViewWithText:noticeText];
	[self _makeStatusIconActiveAndDismiss:status];			
}

- (void)configureWithRequest:(MPOAuthAPIRequestLoader*)request andNotice:(NSString*)noticeText inView:(UIView*)containerView withInputResponse:(OFNotificationInputResponse*)inputResponse
{
	mInputResponse = [inputResponse retain];
	[self _setPresentationView:containerView];
	[self _buildViewWithText:noticeText];
	
	[request setOnSuccess:OFDelegate(self, @selector(_requestSucceeded:nextCall:), [request getOnSuccess])]; 
	[request setOnFailure:OFDelegate(self, @selector(_requestFailed:nextCall:), [request getOnFailure])]; 		
	[request loadSynchronously:NO];
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
	UIView* hitView = [super hitTest:point withEvent:event];
	if (hitView == self)
	{
		[mInputResponse respondToInput];
	}
	return hitView;
}

- (void)dealloc 
{
	self.statusIndicator = nil;
	self.backgroundImage = nil;
	self.notice = nil;
	OFSafeRelease(presentationView);
	OFSafeRelease(mInputResponse);
    [super dealloc];
}

@end
