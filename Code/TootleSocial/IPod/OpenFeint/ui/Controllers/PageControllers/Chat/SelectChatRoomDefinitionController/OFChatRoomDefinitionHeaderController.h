////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#import "OFCallbackable.h"

@class OFSelectChatRoomDefinitionController;

@interface OFChatRoomDefinitionHeaderController : UIViewController<OFCallbackable>
{
@private
	OFSelectChatRoomDefinitionController* chatRoomDefinitionController;
	UIImageView* feintLogoView;
}

@property (nonatomic, readwrite, assign) IBOutlet OFSelectChatRoomDefinitionController* chatRoomDefinitionController;
@property (nonatomic, readwrite, assign) IBOutlet UIImageView* feintLogoView;

- (void)updateBeforeDisplaying;
- (void)hideFeintLogo;

@end