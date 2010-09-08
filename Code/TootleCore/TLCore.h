/*------------------------------------------------------
	Core include header

-------------------------------------------------------*/
#pragma once


#include "TLTypes.h"
#include "TRef.h"

// Forward declarations
class TString;	
class TBinaryTree;


namespace TLCore
{
	Bool			TootInit();						//	engine init
	SyncBool		TootUpdate();					//	manager update invoked from TootLoop
	Bool			TootShutdown(Bool InitResult);	//	engine shutdown
	void			Quit();							//	instigate quitting of the application

	//	use this format!
	namespace Message
	{
		enum
		{
			OnPropertyChanged	= TRef_Static(O,n,P,r,o),
		};
	}
	
	const TRef InitialiseRef	= TRef_Static(I,n,i,t,i);	//"Initialise";
	const TRef UpdateRef		= TRef_Static(U,p,d,a,t);	//"Update";
	const TRef RenderRef		= TRef_Static(R,e,n,d,e);	//"Render";
	const TRef ShutdownRef		= TRef_Static(S,h,u,t,d);	//"Shutdown";
	const TRef TimeStepRef		= TRef_Static(T,i,m,e,s);	//"Timestep";
	const TRef TimeStepModRef	= TRef_Static(T,S,M,o,d);	//"TSMod";
	const TRef QuitRef			= TRef_Static4(Q,u,i,t);	//"Quit";

	const TRef SetPropertyRef	= TRef_Static(S,e,t,P,r);	// "SetProperty"
	const TRef GetPropertyRef	= TRef_Static(G,e,t,P,r);	// "GetProperty"
	const TRef PropertyRef		= TRef_Static(P,r,o,p,e);	// "Property"

	const TRef ManagerRef		= TRef_Static(M,a,n,a,g);	// "Manager"


	namespace Platform
	{
		void				Sleep(u32 Millisecs);	//	platform thread/process sleep

		void				QueryHardwareInformation(TBinaryTree& Data);
		void				QueryLanguageInformation(TBinaryTree& Data);

		void				OpenWebURL(TString& urlstr);
	}
	
};

