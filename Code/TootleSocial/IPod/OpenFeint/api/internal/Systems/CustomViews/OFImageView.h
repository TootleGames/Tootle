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

#pragma once

#include <UIKit/UIKit.h>

class OFHttpService;
class OFImageViewHttpServiceObserver;
@class OFUser;

@interface OFImageView : UIControl< OFCallbackable >
{
@private
	UIImage* mDefaultImage;
	
	NSString* mImageUrl;
	UIImage* mImage;
	
	UIActivityIndicatorView* mLoadingView;
	
	OFPointer<OFHttpService> mHttpService;
	OFPointer<OFImageViewHttpServiceObserver> mHttpServiceObserver;
	
	CGPathRef mBorderPath;
	BOOL mUseSharpCorners;
	
	OFDelegate mDelegate;
	BOOL mUseFacebookOverlay;
	UIImageView* mFacebookOverlay;
}

- (bool)canReceiveCallbacksNow;

@property (nonatomic, retain) UIImage* image;
@property (nonatomic, retain) NSString* imageUrl;
@property (nonatomic, assign) BOOL useFacebookOverlay;
@property (nonatomic, assign) BOOL useSharpCorners;

- (void)showLoadingIndicator;

- (void)setDefaultImage:(UIImage*)defaultImage;

- (void)useLocalPlayerProfilePictureDefault;
- (void)useOtherPlayerProfilePictureDefault;

- (void)useProfilePictureFromUser:(OFUser*)user;

- (void)setImageDownloadFinishedDelegate:(OFDelegate const&)delegate;

@end