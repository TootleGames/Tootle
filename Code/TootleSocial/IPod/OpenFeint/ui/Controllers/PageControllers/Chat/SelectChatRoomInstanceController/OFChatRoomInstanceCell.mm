/*
 *  OFChatRoomInstanceCell.mm
 *  OpenFeint
 *
 *  Created by jakobwilkenson on 3/17/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#import "OFDependencies.h"
#import "OFChatRoomInstanceCell.h"
#import "OFChatRoomDefinitionCell.h"
#import "OFViewHelper.h"
#import "OFChatRoomInstance.h"

@implementation OFChatRoomInstanceCell

- (void)onResourceChanged:(OFResource*)resource
{
	OFChatRoomInstance* chatRoomInstance = (OFChatRoomInstance*)resource;
	
	UILabel* nameLabel = (UILabel*)OFViewHelper::findViewByTag(self, 1);
	nameLabel.text = chatRoomInstance.roomName;
	
	UILabel* numUsersLabel = (UILabel*)OFViewHelper::findViewByTag(self, 2);
	numUsersLabel.text = [NSString stringWithFormat:@"%d/%d", chatRoomInstance.numUsersInRoom, chatRoomInstance.maxNumUsersInRoom];
	if (chatRoomInstance.numUsersInRoom == chatRoomInstance.maxNumUsersInRoom)
	{
		numUsersLabel.textColor = [UIColor redColor];
	}
	else
	{
		numUsersLabel.textColor = [UIColor blackColor];
	}
	
	UIImageView* typeIcon = (UIImageView*)OFViewHelper::findViewByTag(self, 4);
	typeIcon.image = [OFChatRoomDefinitionCell getChatIconForChatType:chatRoomInstance.roomType];
}

@end
