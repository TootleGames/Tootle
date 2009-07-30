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

#import "OFTableSequenceControllerHelper.h"

@class OFChatRoomInstance;

@interface OFSelectChatRoomDefinitionController : OFTableSequenceControllerHelper
{
	BOOL includeGlobalRooms;
	BOOL includeApplicationRooms;
	BOOL includeDeveloperRooms;
}

@property (assign) BOOL includeGlobalRooms;
@property (assign) BOOL includeApplicationRooms;
@property (assign) BOOL includeDeveloperRooms;

- (IBAction)onClickedButton;
@end
