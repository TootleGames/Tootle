#import <UIKit/UIKit.h>

// Quick efine to enable AGON Online.
// If you enable this you will also need to link wiht the AGON library.
// To do that add the following to your project build settings:
//		Other Linker Flags 
//			Debug	- "-lagond"
//			Release - "-lagon"
//		Library Search Paths
//			All - "../../../Tootle/Code/TootleSocial/IPod/libagon/lib/$(SDK_NAME)"
//
// Additionally you will need to add some bundles to your project resources:
//	AgonData.bundle		- found in the libagon/resources directory
//	AgonPackage.bundle	- project specific and is downloaded from the AGON online developer site.
//#define ENABLE_AGONONLINE


#include <TootleCore/TLTypes.h>
#include <TootleCore/TString.h>

namespace TLSocial
{
	namespace Platform
	{
		namespace IPod
		{
			namespace AGONOnline
			{
#ifdef ENABLE_AGONONLINE
				void BeginSession();
				void OpenDashboard();
				void EndSession();
				
				void SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID);

#else
				void BeginSession()		{}
				void OpenDashboard()	{}
				void EndSession()		{}
				
				void SubmitScore(const s32& Score, const TString& Format, const s32& LeaderboardID)	{}
#endif
			}
		}
	}
}

#ifdef ENABLE_AGONONLINE

#import <AGON.h>


#endif