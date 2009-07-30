////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFDependencies.h"
#import "OFMustBeOnlineErrorController.h"

@implementation OFMustBeOnlineErrorController

-(void)_onReachabilityStatusChanged:(NSNumber*)statusAsInt
{
	NetworkReachability status = (NetworkReachability)[statusAsInt intValue];
	if(status != NotReachable)
	{
		[[self navigationController] popViewControllerAnimated:YES];
	}
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	
	mReachabilityObserver.reset(new OFReachabilityObserver(OFDelegate(self, @selector(_onReachabilityStatusChanged:))));
	self.title = @"You're Offline";
	self.navigationItem.hidesBackButton = YES;
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

@end
