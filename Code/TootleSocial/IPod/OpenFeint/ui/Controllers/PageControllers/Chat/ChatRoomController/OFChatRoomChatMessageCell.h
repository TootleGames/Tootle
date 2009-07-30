////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFTableCellHelper.h"

@class OFChatRoomController;

@interface OFChatRoomChatMessageCell : OFTableCellHelper
{
@private
	OFChatRoomController* owner;
}

@property (nonatomic, readwrite, assign) IBOutlet OFChatRoomController* owner;

- (void)onResourceChanged:(OFResource*)resource;
- (IBAction)onClickedFeintName;
- (IBAction)onClickedGameName;

@end
