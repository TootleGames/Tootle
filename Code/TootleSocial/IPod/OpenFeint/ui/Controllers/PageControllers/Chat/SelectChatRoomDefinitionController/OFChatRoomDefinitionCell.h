/*
 *  OFChatRoomDefinitionCell.h
 *  OpenFeint
 *
 *  Created by jakobwilkenson on 3/17/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#import "OFTableCellHelper.h"

@interface OFChatRoomDefinitionCell : OFTableCellHelper

- (void)onResourceChanged:(OFResource*)resource;

+ (UIImage*)getChatIconForChatType:(NSString*)chatType;
@end