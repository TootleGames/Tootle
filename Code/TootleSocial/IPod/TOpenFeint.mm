#import "TOpenFeint.h"
#ifdef ENABLE_OPENFEINT

#import "OFHighScoreService.h"
#import "OpenFeint.h"
#import "OFViewHelper.h"


void TLSocial::Platform::IPod::OpenFeint::BeginSession()
{
	[OpenFeint initializeWithProductKey:@"Np73Z3nH4lPmwCGxQCh7A" 
							  andSecret:@"1Vr4ufYKQrdHptYmq6C5LP5dytD0GNNz74o908lQiY" 
						 andDisplayName:@"TestGame" 
							andSettings:nil 
							andDelegate:nil];

}

void TLSocial::Platform::IPod::OpenFeint::OpenDashboard()
{
	[OpenFeint launchDashboard];

}

void TLSocial::Platform::IPod::OpenFeint::EndSession()
{
}

void TLSocial::Platform::IPod::OpenFeint::SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID)
{

	NSString* pFormattedLeaderboard = [NSString stringWithFormat:@"%d", LeaderboardID];
	
	[OFHighScoreService	setHighScore:Score 
					  forLeaderboard:pFormattedLeaderboard
						   onSuccess:OFDelegate() 
						   onFailure:OFDelegate()];

}


#endif