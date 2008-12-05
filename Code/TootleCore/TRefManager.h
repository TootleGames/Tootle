#pragma once
#include "TRef.h"
#include "TManager.h"

namespace TLRef
{
	class TRefManager;

	extern void GenerateCharLookupTable();
};

class TLRef::TRefManager : public TManager
{
public:
	TRefManager(TRef refManagerID) :
	  TManager(refManagerID)
	{
	}

protected:

	// Generates the look up table - cant do it at bootup/globally
	SyncBool	Initialise()
	{
		TLRef::GenerateCharLookupTable();

		return SyncTrue;

	}
};