////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFTableCellHelper.h"
#import "OFCallbackable.h"

@class OFTableSequenceControllerHelper;

@interface OFUserSettingActionCell : OFTableCellHelper<OFCallbackable>
{
@private
	OFTableSequenceControllerHelper* owner;
}

- (void)onResourceChanged:(OFResource*)resource;

@property (nonatomic, readonly, assign) IBOutlet OFTableSequenceControllerHelper* owner;

@end
