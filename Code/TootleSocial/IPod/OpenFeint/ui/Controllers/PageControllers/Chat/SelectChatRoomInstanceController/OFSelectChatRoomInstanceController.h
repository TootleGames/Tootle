/*
 *  OFSelectChatRoomInstanceController.h
 *  OpenFeint
 *
 *  Created by jakobwilkenson on 3/17/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#import "OFTableSequenceControllerHelper.h"
@class OFChatRoomInstance;
@interface OFSelectChatRoomInstanceController : OFTableSequenceControllerHelper
{
@package
	NSArray* preLoadedChatRoomInstances;
}

@property (nonatomic, retain) NSArray* preLoadedChatRoomInstances;

+ (void)pushChatRoom:(OFChatRoomInstance*)chatRoom navController:(UINavigationController*)navController;
+ (void)pushRoomFullScreen:(UINavigationController*)navController;
@end
