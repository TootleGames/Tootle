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
#import "OFLeaderboardService.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFService+Private.h"
#import "OFLeaderboard.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFLeaderboardService);

@implementation OFLeaderboardService

OPENFEINT_DEFINE_SERVICE(OFLeaderboardService);

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFLeaderboard getResourceName], [OFLeaderboard class]);
}

+ (void) getIndexOnSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	[[self sharedInstance] 
		getAction:@"client_applications/@me/leaderboards.xml"
		withParameters:nil
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestForeground
		withNotice:@"Downloading Leaderboards"];
}

+ (void)getLeaderboardsForApplication:(NSString*)applicationId onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	if (applicationId == nil || [applicationId length] == 0)
	{
		applicationId = @"@me";
	}

	[[self sharedInstance] 
		getAction:[NSString stringWithFormat:@"client_applications/%@/leaderboards.xml", applicationId]
		withParameters:nil
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestForeground
		withNotice:@"Downloading Leaderboards"];
}

@end
