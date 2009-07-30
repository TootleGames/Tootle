////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFFormControllerHelper.h"

@interface OFChatRoomMessageBoxController : OFFormControllerHelper
{
@private
	UITextField* textEntryField;
	UIButton* hideKeyboardButton;
}

@property (nonatomic, retain) IBOutlet UITextField* textEntryField;
@property (nonatomic, retain) IBOutlet UIButton* hideKeyboardButton;

- (IBAction)toggleKeyboardNow;

@end
