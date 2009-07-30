/*
 *  OFImageView.mm
 *  OpenFeint
 *
 *  Created by Andy Dill on 5/4/09.
 *  Copyright 2009 Aurora Feint. All rights reserved.
 *
 */

#include "OFImageView.h"

#include "OFHttpService.h"
#include "OFHttpServiceObserver.h"
#include "OFProvider.h"
#include "OFImageCache.h"
#import "OFUser.h"

class OFImageViewHttpServiceObserver : public OFHttpServiceObserver
{
public:
	OFImageViewHttpServiceObserver(OFDelegate const& onSuccess, OFDelegate const& onFailure);
	
	void onFinishedDownloading(OFHttpServiceRequestContainer* info);
	void onFailedDownloading(OFHttpServiceRequestContainer* info);
	
private:
	OFDelegate mSuccessDelegate;
	OFDelegate mFailedDelegate;
};

OFImageViewHttpServiceObserver::OFImageViewHttpServiceObserver(OFDelegate const& onSuccess, OFDelegate const& onFailure)
: mSuccessDelegate(onSuccess)
, mFailedDelegate(onFailure)
{
}

void OFImageViewHttpServiceObserver::onFinishedDownloading(OFHttpServiceRequestContainer* info)
{
	NSData* data = info->getData();
	if ([data length] > 0)
		mSuccessDelegate.invoke(data);
	else
		mFailedDelegate.invoke();
}

void OFImageViewHttpServiceObserver::onFailedDownloading(OFHttpServiceRequestContainer* info)
{
	mFailedDelegate.invoke();
}

@implementation OFImageView

@synthesize imageUrl = mImageUrl;
@synthesize image = mImage;
@synthesize useFacebookOverlay = mUseFacebookOverlay;
@synthesize useSharpCorners = mUseSharpCorners;


- (void)_destroyHttpService
{
	if (mHttpService.get() != NULL)
	{
		mHttpService->cancelAllRequests();
		mHttpService.reset(NULL);
	}
}

- (void)removeFacebookOverlay
{
	if (mFacebookOverlay)
	{
		[mFacebookOverlay removeFromSuperview];
		OFSafeRelease(mFacebookOverlay);
	}
}

- (void)addFacebookOverlay
{
	[self removeFacebookOverlay];
	mFacebookOverlay = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"OFProfilePictureFacebookIcon.png"]];
	[self addSubview:mFacebookOverlay];
	CGRect facebookRect = mFacebookOverlay.frame;
	static float kInset = 2.f;
	facebookRect.origin.x = self.frame.size.width - facebookRect.size.width - kInset;
	facebookRect.origin.y = self.frame.size.height - facebookRect.size.height - kInset;
	mFacebookOverlay.frame = facebookRect;
}

- (void)setUseFacebookOverlay:(BOOL)useOverlay
{
	[self removeFacebookOverlay];
	mUseFacebookOverlay = useOverlay;
	if (mUseFacebookOverlay && mImage && mImage != mDefaultImage)
	{
		[self addFacebookOverlay];
	}
}

- (void)_resetView:(UIImage*)image
{
	[self removeFacebookOverlay];
	
	[mLoadingView stopAnimating];
	[mLoadingView removeFromSuperview];
	OFSafeRelease(mLoadingView);

	OFSafeRelease(mImage);
	mImage = [image retain];
	
	if (mUseFacebookOverlay && mImage && mImage != mDefaultImage)
	{
		[self addFacebookOverlay];
	}
}

- (void)setImageUrl:(NSString*)imageUrl
{
	OFSafeRelease(mImageUrl);
	mImageUrl = [imageUrl retain];

	UIImage* image = [[OFImageCache sharedInstance] fetch:mImageUrl];
	[self _destroyHttpService];
	[self _resetView:image];
	[self setNeedsDisplay];

	if (mImageUrl != nil && ![mImageUrl isEqualToString:@""])
	{
		if (image == nil)
		{
			[self showLoadingIndicator];
						
			OFDelegate success(self, @selector(_imageDownloaded:));
			OFDelegate failure(self, @selector(_imageDownloadFailed));

			bool absoluteUrl = [imageUrl hasPrefix:@"http"] || [imageUrl hasPrefix:@"www"];
			mHttpService = absoluteUrl ? OFPointer<OFHttpService>(new OFHttpService(@"")) : [OFProvider createHttpService];
			mHttpServiceObserver = new OFImageViewHttpServiceObserver(success, failure);
			mHttpService->startRequest(mImageUrl, HttpMethodGet, NULL, mHttpServiceObserver.get());
		}
		else
		{
			mDelegate.invoke();
		}
	}
	else if (mDefaultImage)
	{
		[self _resetView:mDefaultImage];		
	}
}

- (void)setImage:(UIImage*)image
{
	OFSafeRelease(mImageUrl);
	[self _destroyHttpService];
	
	if (mDefaultImage != nil && image == nil)
		image = mDefaultImage;
	
	[self _resetView:image];
	[self setNeedsDisplay];
}

- (void)dealloc
{
	CGPathRelease(mBorderPath);
	OFSafeRelease(mImageUrl);
	[self _destroyHttpService];
	[self _resetView:nil];
	OFSafeRelease(mImage);
	OFSafeRelease(mDefaultImage);
	[super dealloc];
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

- (void)_recreateBorderPath
{
	if (mBorderPath)
	{
		CGPathRelease(mBorderPath);
		mBorderPath = nil;
	}

	if (mUseSharpCorners)
	{
		return;
	}

	CGRect rect = self.frame;
	
	float const radius = 5.0f;

	float maxx = CGRectGetWidth(rect);
	float midx = maxx * 0.5f;
	float maxy = CGRectGetHeight(rect);
	float midy = maxy * 0.5f;

	CGMutablePathRef path = CGPathCreateMutable();
	CGPathMoveToPoint(path, NULL, 0.0f, midy);
	CGPathAddArcToPoint(path, NULL, 0.0f, 0.0f, midx, 0.0f, radius);
	CGPathAddArcToPoint(path, NULL, maxx, 0.0f, maxx, midy, radius);
	CGPathAddArcToPoint(path, NULL, maxx, maxy, midx, maxy, radius);
	CGPathAddArcToPoint(path, NULL, 0.0f, maxy, 0.0f, midy, radius);
	CGPathCloseSubpath(path);

	mBorderPath = path;
}

- (void)setFrame:(CGRect)frame
{
	[super setFrame:frame];
	[self _recreateBorderPath];
}

- (void)_imageDownloaded:(NSData*)imageData
{
	UIImage* image = [UIImage imageWithData:imageData];
	if (image != nil)
		[[OFImageCache sharedInstance] store:image withIdentifier:mImageUrl];

	[self _resetView:image];
	[self setNeedsDisplay];
	
	mDelegate.invoke();
}

- (void)_imageDownloadFailed
{
	UIImage* imageToUse = mDefaultImage;
	if (imageToUse == nil)
		OFLog(@"OFImageView download failed but doesn't have a default image!");
		
	[self _resetView:imageToUse];
	[self setNeedsDisplay];

	mDelegate.invoke();
}

- (void)drawRect:(CGRect)rect
{
	CGContextRef ctx = UIGraphicsGetCurrentContext();

	if (mImage != nil)
	{
		if (mBorderPath)
		{
			CGContextBeginPath(ctx);
			CGContextAddPath(ctx, mBorderPath);
			CGContextClosePath(ctx);
			CGContextClip(ctx);
		}

		[mImage drawInRect:rect];
	}
	else
	{
		CGContextSetGrayFillColor(ctx, 0.0f, 0.0f);
		CGContextFillRect(ctx, rect);
	}
}

- (void)showLoadingIndicator
{
	mLoadingView = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray];
	mLoadingView.hidesWhenStopped = YES;

	CGRect loadingFrame = mLoadingView.frame;
	loadingFrame.origin.x = (self.frame.size.width - mLoadingView.frame.size.width) * 0.5f;
	loadingFrame.origin.y = (self.frame.size.height - mLoadingView.frame.size.height) * 0.5f;
	[mLoadingView setFrame:loadingFrame];

	[mLoadingView startAnimating];
	[self addSubview:mLoadingView];	
}

- (void)setDefaultImage:(UIImage*)defaultImage
{
	bool needsToShowImage = false;
	if (mImage == mDefaultImage)
		needsToShowImage = true;
		
	OFSafeRelease(mDefaultImage);
	mDefaultImage = [defaultImage retain];
	[self _resetView:mDefaultImage];
	[self setNeedsDisplay];
}

- (void)setImageDownloadFinishedDelegate:(OFDelegate const&)delegate
{
	mDelegate = delegate;
}

- (void)useLocalPlayerProfilePictureDefault
{
	[self setDefaultImage:[UIImage imageNamed:@"OFProfileIconDefaultSelf.png"]];
}

- (void)useOtherPlayerProfilePictureDefault
{
	[self setDefaultImage:[UIImage imageNamed:@"OFProfileIconDefault.png"]];
}

- (void)useProfilePictureFromUser:(OFUser*)user
{
	if (user)
	{
		if ([user isLocalUser])
		{
			[self useLocalPlayerProfilePictureDefault];
		}
		else
		{
			[self useOtherPlayerProfilePictureDefault];
		}
		self.useFacebookOverlay = user.usesFacebookProfilePicture;
		self.imageUrl = user.profilePictureUrl;
	}
	else
	{
		[self useOtherPlayerProfilePictureDefault];
		self.imageUrl = nil;
	}
	
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
	UITouch* touch = [touches anyObject];
	
	if ([touch tapCount] == 1)
	{
		[self sendActionsForControlEvents:UIControlEventTouchUpInside];
	}
}

- (void)setUseSharpCorners:(BOOL)useSharpCorners
{
	mUseSharpCorners = useSharpCorners;
	[self _recreateBorderPath];
}

@end