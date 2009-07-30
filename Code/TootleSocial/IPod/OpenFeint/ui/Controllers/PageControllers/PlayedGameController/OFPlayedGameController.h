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

@interface OFPlayedGameController : OFTableSequenceControllerHelper<OFProfileFrame>
{
	NSString* userId;
	OFUser* mUser;
}

@property (nonatomic, retain) NSString* userId;

- (NSString*)getProfileUserId;

- (void)onCellWasClicked:(OFResource*)cellResource indexPathInTable:(NSIndexPath*)indexPath;

@end
