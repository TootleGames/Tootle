////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFDeadEndErrorController.h"
#import "OFReachabilityObserver.h"
#import "OFPointer.h"
#import "OFCallbackable.h"

@interface OFMustBeOnlineErrorController : OFDeadEndErrorController<OFCallbackable>
{
@private
	OFPointer<OFReachabilityObserver> mReachabilityObserver;
}
@end
