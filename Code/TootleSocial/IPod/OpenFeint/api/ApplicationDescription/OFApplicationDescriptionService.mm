//
//  OFApplicationDescriptionService.mm
//  OpenFeint
//
//  Created by Jason Citron on 4/20/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFDependencies.h"
#import "OFApplicationDescriptionService.h"
#import "OFApplicationDescription.h"
#import "OFService+Private.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFApplicationDescriptionService);

@implementation OFApplicationDescriptionService

OPENFEINT_DEFINE_SERVICE(OFApplicationDescriptionService);

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFApplicationDescription getResourceName], [OFApplicationDescription class]);
}

+ (void) getShowWithId:(NSString*)resourceId onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	[[self sharedInstance] getAction:[NSString stringWithFormat:@"client_applications/%@/application_descriptions.xml", resourceId]
		 withParameters:nil
		 withSuccess:onSuccess
		 withFailure:onFailure
		 withRequestType:OFActionRequestForeground
		 withNotice:@"Downloading application description"];
}

@end
