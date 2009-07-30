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

@interface OFUserSettingCell : OFTableCellHelper<OFCallbackable>
{
@private
	UILabel* name;
	UISwitch* booleanSwitch;
	OFTableSequenceControllerHelper* owner;
}

- (void)onResourceChanged:(OFResource*)resource;
- (IBAction)switchToggled;

@property (nonatomic, readonly, assign) IBOutlet UILabel* name;
@property (nonatomic, readonly, assign) IBOutlet UISwitch* booleanSwitch;
@property (nonatomic, readonly, assign) IBOutlet OFTableSequenceControllerHelper* owner;
	
@end
