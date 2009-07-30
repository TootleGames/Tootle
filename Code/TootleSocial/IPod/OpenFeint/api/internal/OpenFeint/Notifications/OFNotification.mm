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
#import "OFNotification.h"
#import "MPOAuthAPIRequestLoader.h"
#import "OFNotificationView.h"
#import "OFAchievementNotificationView.h"
#import "OFAchievement.h"
#import "OpenFeint+Private.h"

@implementation OFNotification

+ (OFNotification*)sharedInstance				
{												
	static OFNotification* sInstance = nil;		
	if(sInstance == nil)						
	{											
		sInstance = [OFNotification new];		
	}											
	
	return sInstance;							
}

- (id)init
{
	if(self = [super init])
	{
	}
	return self;
}

- (void)dealloc
{
	[super dealloc];
}

- (void)showBackgroundNoticeForLoader:(MPOAuthAPIRequestLoader*)request withNotice:(NSString*)noticeText
{
	UIView* topView = [OpenFeint getTopApplicationView];
	if (topView)
	{
		[OFNotificationView showNotificationWithRequest:request andNotice:noticeText inView:topView withInputResponse:nil];
	}
}

- (void)showBackgroundNotice:(NSString*)noticeText andStatus:(OFNotificationStatus*)status andInputResponse:(OFNotificationInputResponse*)inputResponse
{
	UIView* topView = [OpenFeint getTopApplicationView];
	if (topView)
	{
		[OFNotificationView showNotificationWithText:noticeText andStatus:status inView:topView withInputResponse:inputResponse];
	}
}

- (void)showAchievementNotice:(OFAchievement*)unlockedAchievement withInputResponse:(OFNotificationInputResponse*)inputResponse
{
	UIView* topView = [OpenFeint getTopApplicationView];
	if (topView)
	{
		[OFAchievementNotificationView showAchievementNotice:unlockedAchievement inView:topView withInputResponse:inputResponse];
	}
}

@end