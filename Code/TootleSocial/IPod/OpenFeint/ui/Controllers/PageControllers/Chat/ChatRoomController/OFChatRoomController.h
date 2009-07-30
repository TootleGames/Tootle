////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFTableSequenceControllerHelper.h"

@class OFChatRoomMessageBoxController;
@class OFChatRoomInstance;

@interface OFChatRoomController : OFTableSequenceControllerHelper<UIAlertViewDelegate, UITextFieldDelegate>
{
@private
	OFChatRoomMessageBoxController* mChatRoomMessageBoxController;
	bool mIsKeyboardShown;	
	bool mHasAppearedBefore;
	bool mRoomIsFull;
	OFChatRoomInstance* roomInstance;
}
@property (nonatomic, retain) OFChatRoomInstance* roomInstance;
@end
