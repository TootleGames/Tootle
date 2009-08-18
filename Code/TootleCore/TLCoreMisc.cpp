#include "TLCoreMisc.h"


//---------------------------------------------------
//	convert a pointer to an integer
//---------------------------------------------------
u32 TLCore::PointerToInteger(void* pAddress)
{
	u32 Address = 0;

	//	ignore pointer truncation warning (void* to integer)
#pragma warning(push)
#pragma warning(disable : 4311) 
	Address = reinterpret_cast<u32>( pAddress );
#pragma warning(pop)

	return Address;
}

//---------------------------------------------------
//	convert an integer to a pointer
//---------------------------------------------------
void* TLCore::IntegerToPointer(u32 Integer)
{
	void* pAddress = 0;

	//	ignore pointer truncation warning (void* to integer)
#pragma warning(push)
#pragma warning(disable : 4311) 
	pAddress = reinterpret_cast<void*>( Integer );
#pragma warning(pop)

	return pAddress;
}


