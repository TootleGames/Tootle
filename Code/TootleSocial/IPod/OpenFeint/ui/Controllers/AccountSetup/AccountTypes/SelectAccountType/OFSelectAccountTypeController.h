////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFAccountSetupBaseController.h"

@interface OFSelectAccountTypeController : OFAccountSetupBaseController<UIActionSheetDelegate>
{
@private
	UILabel* socialNetworkNotice;
	UILabel* openFeintNotice;	
	BOOL neverShowLostAccountWarning;
	OFRetainedPtr<NSString> mControllerToOpen;
}

@property (nonatomic, retain) IBOutlet UILabel* socialNetworkNotice;
@property (nonatomic, retain) IBOutlet UILabel* openFeintNotice;
@property (nonatomic, assign) BOOL neverShowLostAccountWarning;


@end
