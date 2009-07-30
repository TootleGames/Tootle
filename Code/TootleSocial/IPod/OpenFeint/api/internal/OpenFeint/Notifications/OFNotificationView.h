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

#import "OFCallbackable.h"
#import "OFNotificationStatus.h"

@class MPOAuthAPIRequestLoader;
@class OFNotificationInputResponse;

@interface OFNotificationView : UIView<OFCallbackable>
{
@package
	UILabel* notice;
	UIImageView* statusIndicator;
	UIImageView* backgroundImage;
	OFNotificationInputResponse* mInputResponse;
	BOOL mParentViewIsRotatedInternally;

	UIView* presentationView;
}

+ (void)showNotificationWithRequest:(MPOAuthAPIRequestLoader*)request andNotice:(NSString*)noticeText inView:(UIView*)containerView withInputResponse:(OFNotificationInputResponse*)inputResponse;
+ (void)showNotificationWithText:(NSString*)noticeText andStatus:(OFNotificationStatus*)status inView:(UIView*)containerView withInputResponse:(OFNotificationInputResponse*)inputResponse;

- (void)configureWithRequest:(MPOAuthAPIRequestLoader*)request andNotice:(NSString*)noticeText inView:(UIView*)containerView withInputResponse:(OFNotificationInputResponse*)inputResponse;
- (void)configureWithText:(NSString*)noticeText andStatus:(OFNotificationStatus*)status inView:(UIView*)containerView withInputResponse:(OFNotificationInputResponse*)inputResponse;

- (bool)canReceiveCallbacksNow;

@property (nonatomic, retain) IBOutlet UILabel* notice;
@property (nonatomic, retain) IBOutlet UIImageView* statusIndicator;
@property (nonatomic, retain) IBOutlet UIImageView* backgroundImage;

- (void)_setPresentationView:(UIView*)_presentationView;
- (void)_present;

@end