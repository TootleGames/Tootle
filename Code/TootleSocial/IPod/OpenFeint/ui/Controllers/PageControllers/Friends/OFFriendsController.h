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
#import "OFProfileFrame.h"

@class OFUser;

@interface OFFriendsController : OFTableSequenceControllerHelper<OFProfileFrame>
{
	NSString* userId;
	OFUser* mUser;
	BOOL alwaysRefresh;
}

@property (nonatomic, retain) NSString* userId;


@end
