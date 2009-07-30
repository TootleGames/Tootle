////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#import "OFFormControllerHelper.h"


@interface OFReportAbuseController : OFFormControllerHelper <UIAlertViewDelegate> {
	NSString* mFlaggedUserId;
}
@property(nonatomic,  retain) NSString* flaggedUserId;
+(UIViewController*) OFReportAbuseControllerForUserId:(NSString*)userId;

@end
