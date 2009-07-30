////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Copyright (c) 2009 Aurora Feint Inc.
///
///  This library is free software; you can redistribute it and/or
///  modify it under the terms of the GNU Lesser General Public
///  License as published by the Free Software Foundation; either
///  
///  version 3 of the License, or (at your option) any later version.
///  
///  This library is distributed in the hope that it will be useful,
///  
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
///  Lesser General Public License for more details.
///  
///  
///  You should have received a copy of the GNU Lesser General Public
///  License along with this library; if not, write to the Free Software
///  
///  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFDependencies.h"
#import "OFUserService.h"
#import "OFService+Private.h"
#import "OFHttpNestedQueryStringWriter.h"

#import "OFUser.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFUserService)

@implementation OFUserService

OPENFEINT_DEFINE_SERVICE(OFUserService);

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFUser getResourceName], [OFUser class]);
}

+ (void) getUser:(NSString*)userId onSuccess:(OFDelegate const&)onSuccess onFailure:(OFDelegate const&)onFailure
{
	if (userId == nil)
	{
		userId = @"@me";
	}
	
	[[self sharedInstance] 
	 getAction:[NSString stringWithFormat:@"users/%@/", userId]
	 withParameters:nil
	 withSuccess:onSuccess
	 withFailure:onFailure
	 withRequestType:OFActionRequestSilent
	 withNotice:@"Downloading Profile"];
}


@end