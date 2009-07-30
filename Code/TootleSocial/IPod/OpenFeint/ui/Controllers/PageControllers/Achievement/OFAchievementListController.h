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
#import "OFProfileFrame.h"

@class OFPlayedGame;
@class OFUser;

@interface OFAchievementListController : OFTableSequenceControllerHelper< OFProfileFrame >
{
@package
	NSString* applicationName;
	NSString* applicationId;
	NSString* applicationIconUrl;
	BOOL doesUserHaveApplication;
	NSString* userId;
	OFUser* mUser;
}

@property (nonatomic, retain) NSString* applicationName;
@property (nonatomic, retain) NSString* applicationId;
@property (nonatomic, retain) NSString* applicationIconUrl;
@property (nonatomic) BOOL doesUserHaveApplication;
@property (nonatomic, retain) NSString* userId;

- (IBAction)onGetGame;

- (void)populateContextualDataFromPlayedGame:(OFPlayedGame*)playedGame;

- (NSString*)getProfileUserId;

@end