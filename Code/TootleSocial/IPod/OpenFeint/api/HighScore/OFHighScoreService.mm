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
#import "OFHighScoreService.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFService+Private.h"
#import "OFHighScore.h"
#import "OFLeaderboard.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFHighScoreService);

@implementation OFHighScoreService

OPENFEINT_DEFINE_SERVICE(OFHighScoreService);

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFHighScore getResourceName], [OFHighScore class]);
}

+ (void) getPage:(NSInteger)pageIndex forLeaderboard:(NSString*)leaderboardId friendsOnly:(BOOL)friendsOnly onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	[OFHighScoreService getPage:pageIndex forLeaderboard:leaderboardId friendsOnly:friendsOnly silently:NO onSuccess:onSuccess onFailure:onFailure];
}

+ (void) getPage:(NSInteger)pageIndex forLeaderboard:(NSString*)leaderboardId friendsOnly:(BOOL)friendsOnly silently:(BOOL)silently onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	params->io("leaderboard_id", leaderboardId);

	params->io("page", pageIndex);
	
	if (friendsOnly)
	{
		bool friendsLeaderboard = true;
		OFRetainedPtr<NSString> followerId = @"me";
		params->io("friends_leaderboard", friendsLeaderboard);
		params->io("follower_id", followerId);
	}
	
	OFActionRequestType requestType = silently ? OFActionRequestSilent : OFActionRequestForeground;
	
	[[self sharedInstance] 
		getAction:@"client_applications/@me/high_scores.xml"
		withParameters:params
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:requestType
		withNotice:@"Downloading High Scores"];

}

+ (void) getPageWithLoggedInUserForLeaderboard:(NSString*)leaderboardId onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	params->io("leaderboard_id", leaderboardId);	
	params->io("near_user_id", @"me");
	
	[[self sharedInstance]
	 getAction:@"client_applications/@me/high_scores.xml"
	 withParameters:params
	 withSuccess:onSuccess
	 withFailure:onFailure
	 withRequestType:OFActionRequestForeground
	 withNotice:@"Downloading High Scores"];
}

+ (void) setHighScore:(int64_t)score forLeaderboard:(NSString*)leaderboardId onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{	
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	OFRetainedPtr<NSString> leaderboardIdString = leaderboardId;
	params->io("leaderboard_id", leaderboardIdString);
	
	{
		OFISerializer::Scope high_score(params, "high_score");
		params->io("score", score);
	}
	
	[[self sharedInstance]
	 postAction:@"client_applications/@me/high_scores.xml"
	 withParameters:params
	 withSuccess:onSuccess
	 withFailure:onFailure
	 withRequestType:OFActionRequestBackground
	 withNotice:@"Submitting High Score"];
}

+ (void) batchSetHighScores:(OFHighScoreBatchEntrySeries&)highScoreBatchEntrySeries onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure optionalMessage:(NSString*)submissionMessage
{
	
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	params->serialize("high_scores", "entry", highScoreBatchEntrySeries);
	
	NSString* notice = submissionMessage ? submissionMessage : @"Submitting High Scores";
	[[self sharedInstance]
	 postAction:@"client_applications/@me/high_scores.xml"
	 withParameters:params
	 withSuccess:onSuccess
	 withFailure:onFailure
	 withRequestType:OFActionRequestForeground
	 withNotice:notice];
}

+ (void) getAllHighScoresForLoggedInUser:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure optionalMessage:(NSString*)submissionMessage
{
	NSString* notice = submissionMessage ? submissionMessage : @"Downloading High Scores";
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	OFRetainedPtr<NSString> me = @"me";
	bool acrossLeaderboards = true;
	params->io("across_leaderboards", acrossLeaderboards);
	params->io("user_id", me);
	
	[[self sharedInstance] 
	 getAction:@"client_applications/@me/high_scores.xml"
	 withParameters:params
	 withSuccess:onSuccess
	 withFailure:onFailure
	 withRequestType:OFActionRequestForeground
	 withNotice:notice];
	
}

@end

