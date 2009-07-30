//
//  OFTableSectionCellDescription.mm
//  OpenFeint
//
//  Created by Jason Citron on 4/20/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFDependencies.h"
#import "OFTableSectionCellDescription.h"
#import "OFResource.h"

@implementation OFTableSectionCellDescription

@synthesize resource;
@synthesize controllerName;

+ (id)cellController:(NSString*)controllerName andResource:(OFResource*)resource
{
	OFTableSectionCellDescription* cellDescription = [OFTableSectionCellDescription new];
	cellDescription.resource = resource;
	cellDescription.controllerName = controllerName;
	return cellDescription;
}

- (void)dealloc
{
	self.resource = nil;
	self.controllerName = nil;
	
	[super dealloc];
}

@end
