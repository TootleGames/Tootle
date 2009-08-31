/*
 *  TSocialNetworkingManager.h
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 28/08/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TRef.h>

namespace TLSocial
{	
	// Supported social networking platforms
	const TRef FacebookRef			= TRef_Static(F,a,c,e,b);	//"Facebook";
	//	const TRef TwitterRef			= TRef_Static(T,w,i,t,t);	//"Twitter";
	const TRef OpenFeintRef			= TRef_Static(O,p,e,n,F);	//"OpenFeint";
	const TRef AGONOnlineRef		= TRef_Static(A,G,O,N,O);	//"AGONOnline";
		
}