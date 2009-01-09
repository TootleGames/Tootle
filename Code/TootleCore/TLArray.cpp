#include "TArray.h"
#include "TLDebug.h"
#include "TString.h"


Bool TLArray::Debug::Break(const char* pErrorString,const char* pSourceFunction)
{
#if defined(_DEBUG)
	if ( !TLDebug::IsEnabled() )
		return FALSE;
	
	TTempString String( pErrorString );
	return TLDebug::Break( String, pSourceFunction );
#else
	return FALSE;
#endif
}

	
void TLArray::Debug::Print(const char* pErrorString,const char* pSourceFunction)
{
#if defined(_DEBUG)
	if ( !TLDebug::IsEnabled() )
		return;
	
	TTempString String( pErrorString );
	TLDebug::Print( String, pSourceFunction );
#endif
}

	

