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
#import "OFService+Overridables.h"
#import "OFControllerHelpersCommon.h"
#import "OFResourceNameMap.h"
#import "OFPointer.h"

@implementation OFService ( Overridables )

+ (void) getIndexOnSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	/// @note	if having a default index action does not make sense for your resource, consider
	///			overriding the location that this request is being made from to invoke your
	///			own custom request.
	
	ASSERT_OVERRIDE_MISSING;
}

+ (void) getShowWithId:(NSString*)resourceId onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	ASSERT_OVERRIDE_MISSING;
}

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	ASSERT_OVERRIDE_MISSING;
}

- (OFService*)sharedInstance
{
	ASSERT_OVERRIDE_MISSING;
	return NULL;
}

- (void)registerPolledResources:(OFPoller*)poller
{
}

@end
