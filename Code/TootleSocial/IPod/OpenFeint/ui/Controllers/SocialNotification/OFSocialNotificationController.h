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

#import <UIKit/UIKit.h>
#import "OFSocialNotification.h"

@class OFImageView;

@interface OFSocialNotificationController : UIViewController< OFCallbackable >
{
	OFSocialNotification* mSocialNotification;
	
	UILabel* mNotificationText;
	UILabel* mApplicationLabel;
	OFImageView* mNotificationImage;
	
	UISwitch* mRememberSwitch;

	UIImageView* mNetworkIcon1;
	UIImageView* mNetworkIcon2;
}

@property(nonatomic,retain) OFSocialNotification* socialNotification;
@property(nonatomic,retain) IBOutlet UILabel* notificationText;
@property(nonatomic,retain) IBOutlet UILabel* applicationLabel;
@property(nonatomic,retain) IBOutlet OFImageView* notificationImage;
@property(nonatomic,retain) IBOutlet UISwitch* rememberSwitch;
@property(nonatomic,retain) IBOutlet UIImageView* networkIcon1;
@property(nonatomic,retain) IBOutlet UIImageView* networkIcon2;

-(void)addSocialNetworkIcon:(UIImage*)networkIcon;

-(IBAction)yesButtonClicked:(UIButton*)sender;
-(IBAction)noButtonClicked:(UIButton*)sender;
-(void)dismiss;

-(bool)canReceiveCallbacksNow;

@end