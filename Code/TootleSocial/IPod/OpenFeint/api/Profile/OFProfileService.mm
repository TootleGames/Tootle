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
#import "OFProfileService.h"
#import "OFService+Private.h"
#import "OpenFeint+UserOptions.h"
#import "OFHttpNestedQueryStringWriter.h"

#import "OFPlayedGame.h"
#import "OFGamerscore.h"
#import "OFUserGameStat.h"
#import "OFUser.h"
#import "OFUsersCredential.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFProfileService)

@implementation OFProfileService

OPENFEINT_DEFINE_SERVICE(OFProfileService);

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFPlayedGame getResourceName], [OFPlayedGame class]);
	namedResources->addResource([OFGamerscore getResourceName], [OFGamerscore class]);
	namedResources->addResource([OFUserGameStat getResourceName], [OFUserGameStat class]);
	namedResources->addResource([OFUser getResourceName], [OFUser class]);
	namedResources->addResource([OFUsersCredential getResourceName], [OFUsersCredential class]);
}

+ (void) getLocalPlayerProfileOnSuccess:(OFDelegate const&)onSuccess onFailure:(OFDelegate const&)onFailure
{
	[OFProfileService getProfileForUser:nil onSuccess:onSuccess onFailure:onFailure];
}

+ (void) getProfileForUser:(NSString*)userId onSuccess:(OFDelegate const&)onSuccess onFailure:(OFDelegate const&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	if (userId == nil || [userId isEqualToString:[OpenFeint lastLoggedInUserId]])
	{
		userId = @"me";
	}
	else
	{
		OFRetainedPtr<NSString> comparedToUserId = @"me";
		params->io("compared_to_user_id", comparedToUserId);
	}
	
	[[self sharedInstance] 
		getAction:[NSString stringWithFormat:@"profiles/%@/", userId]
		withParameters:params
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestSilent
		withNotice:@"Downloading Profile"];
}

+ (void) getGamerscoreForUser:(NSString*)userId onSuccess:(OFDelegate const&)onSuccess onFailure:(OFDelegate const&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	if (userId == nil)
		userId = @"me";

	[[self sharedInstance] 
		getAction:[NSString stringWithFormat:@"profiles/%@/gamerscore", userId]
		withParameters:params
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestSilent
		withNotice:@"Downloading Gamerscore"];
}

+ (void) getPlayedGamesForUser:(NSString*)userId withPage:(NSInteger)pageIndex andCountPerPage:(NSInteger)perPage onSuccess:(OFDelegate const&)onSuccess onFailure:(OFDelegate const&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	params->io("page", pageIndex);
	params->io("per_page", perPage);
	
	if (userId == nil || [userId isEqualToString:[OpenFeint lastLoggedInUserId]])
	{
		userId = @"me";
	}
	else
	{
		params->io("compared_to_user_id", @"me");
	}

	[[self sharedInstance] 
		getAction:[NSString stringWithFormat:@"profiles/%@/list_games", userId]
		withParameters:params
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestForeground
		withNotice:@"Downloading Game Information"];
}

@end