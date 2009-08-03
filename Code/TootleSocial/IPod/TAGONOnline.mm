#import "TAGONOnline.h"

#ifdef ENABLE_AGONONLINE

void TLSocial::Platform::IPod::AGONOnline::BeginSession(const TString& APIKey, const TString& APISecret)
{
	// Enable logging from inside of AGON.
	AgonShowLogs(YES);

	//NSString* pAPIKey = [NSString stringWithUTF8String:APIKey.GetData()];
	NSString* pAPISecret = [NSString stringWithUTF8String:APISecret.GetData()];
	
	// Test App Secret - @"F6F99887E71B210A7DE512050EAD22894B72952A"
	
	AgonCreate(pAPISecret); // App secret that matches the DevDB environment.
	
	// Tint AGON backgrounds to match app
	AgonSetStartBackgroundTint([UIColor colorWithRed:118/255.0 green:118/255.0 blue:118/255.0 alpha:1]);
	AgonSetEndBackgroundTint([UIColor colorWithRed:121/255.0 green:163/255.0 blue:164/255.0 alpha:1]);
}

void TLSocial::Platform::IPod::AGONOnline::OpenDashboard()
{
	//AgonShow(self, @selector(agonDidHide), AgonBladeProfile);	
	AgonShow(nil, nil, AgonBladeProfile);

}

void TLSocial::Platform::IPod::AGONOnline::OpenLeaderboard()
{
	//AgonShow(self, @selector(agonDidHide), AgonBladeProfile);	
	AgonShow(nil, nil, AgonBladeLeaderboards);	
}



void TLSocial::Platform::IPod::AGONOnline::SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID)
{
	// Covert our TString into an NSString
	NSString* pFormatString = [NSString stringWithUTF8String:Format.GetData()];

	AgonSubmitScore(Score, pFormatString, LeaderboardID);
}


void TLSocial::Platform::IPod::AGONOnline::EndSession()
{
	AgonDestroy();
}


#endif