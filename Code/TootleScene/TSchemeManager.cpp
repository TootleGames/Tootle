/*
 *  TSchemeManager.cpp
 *  TootleScene
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TSchemeManager.h"

namespace TLScheme
{
	TPtr<TSchemeManager> g_pSchemeManager = NULL;
}


using namespace TLScheme;

SyncBool TSchemeManager::Initialise() 
{	
	return SyncTrue; 
}

SyncBool TSchemeManager::Shutdown()
{ 
	return SyncTrue; 
}

SyncBool TSchemeManager::Update(float /*fTimeStep*/)		
{
	return SyncTrue;
}


Bool TSchemeManager::IsSchemeLoaded(TRefRef SchemeRef)
{
	return FALSE;
}

Bool TSchemeManager::RequestLoadScheme(TRefRef SchemeRef)
{
	return FALSE;
}

Bool TSchemeManager::RequestUnloadScheme(TRefRef SchemeRef)
{
	return FALSE;
}
