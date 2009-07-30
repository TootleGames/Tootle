/*
 *  OFChatRoomDefinitionCell.mm
 *  OpenFeint
 *
 *  Created by jakobwilkenson on 3/17/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#import "OFDependencies.h"
#import "OFChatRoomDefinitionCell.h"
#import "OFViewHelper.h"
#import "OFChatRoomDefinition.h"

@implementation OFChatRoomDefinitionCell

- (void)onResourceChanged:(OFResource*)resource
{
	OFChatRoomDefinition* chatRoom = (OFChatRoomDefinition*)resource;
	
	UILabel* nameLabel = (UILabel*)OFViewHelper::findViewByTag(self, 1);
	nameLabel.text = chatRoom.roomName;
	
	UIImageView* typeIcon = (UIImageView*)OFViewHelper::findViewByTag(self, 5);
	typeIcon.image = [OFChatRoomDefinitionCell getChatIconForChatType:chatRoom.roomType];
	
}

+ (UIImage*)getChatIconForChatType:(NSString*)chatType
{	
	if ([chatType isEqualToString:[OFChatRoomDefinition getDeveloperRoomTypeId]])
	{
		return [UIImage imageNamed:@"OpenFeintChatIconDeveloperRoom.png"];
	}
	else if ([chatType isEqualToString:[OFChatRoomDefinition getApplicationRoomTypeId]])
	{
		return [UIImage imageNamed:@"OpenFeintChatIconApplicationRoom.png"];
	}
	else
	{
		return [UIImage imageNamed:@"OpenFeintChatIconGlobalRoom.png"];
	}
}

@end
