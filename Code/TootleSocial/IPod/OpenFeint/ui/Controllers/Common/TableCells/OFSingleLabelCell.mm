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
#import "OFSingleLabelCell.h"

@implementation OFSingleLabelCell

@synthesize label;

- (void)onResourceChanged:(OFResource*)resource
{
	
}

- (void)dealloc
{
	self.label = nil;
	[super dealloc];
}

@end
