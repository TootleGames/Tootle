#import <UIKit/UIKit.h>

//#define ENABLE_OPENFEINT

#include <TootleCore/TLTypes.h>
#include <TootleCore/TString.h>

namespace TLSocial
{
	namespace Platform
	{
		namespace IPod
		{
			namespace OpenFeint
			{
#ifdef ENABLE_OPENFEINT
				void BeginSession(const TString& APIKey, const TString& APISecret);
				void OpenDashboard();
				void OpenLeaderboard()	{}
				void EndSession();
				
				void SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID);
#else
				void BeginSession(const TString& APIKey, const TString& APISecret)		{}
				void OpenDashboard()	{}
				void OpenLeaderboard()	{}
				void EndSession()		{}
				
				void SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID) {}
#endif
			}
		}
	}
}

