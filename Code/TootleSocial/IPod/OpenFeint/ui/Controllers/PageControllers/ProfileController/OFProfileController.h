////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFProfileFrame.h"
#import "OFTableSequenceControllerHelper.h"

@class OFProfile;

@interface OFProfileController : OFTableSequenceControllerHelper<OFProfileFrame >
{
@package
	NSString* mUserId;
	OFUser* mUser;
	bool alwaysRefresh;
}

@property (nonatomic, retain) NSString* userId;

+ (void)showProfileForUser:(NSString*)userId;

- (NSString*)getProfileUserId;
- (IBAction) onFlag;

@end
