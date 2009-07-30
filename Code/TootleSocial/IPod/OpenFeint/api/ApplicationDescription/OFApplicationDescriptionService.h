//
//  OFApplicationDescriptionService.h
//  OpenFeint
//
//  Created by Jason Citron on 4/20/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFService.h"

@interface OFApplicationDescriptionService : OFService

OPENFEINT_DECLARE_AS_SERVICE(OFApplicationDescriptionService);

+ (void) getShowWithId:(NSString*)resourceId onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure;

@end
