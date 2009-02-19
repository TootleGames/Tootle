/*------------------------------------------------------

	
 
 -------------------------------------------------------*/
#pragma once

#include "../TTessellate.h"


namespace TLMaths
{
	namespace Platform
	{
		TTessellator*	CreateTessellator(TPtr<TLAsset::TMesh>& pMesh);		//	create platform specific tessellator
	}
}


