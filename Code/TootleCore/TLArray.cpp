#include "TArray.h"
#include "TLDebug.h"
#include "TString.h"


Bool TLArray::Debug::Break(const char* pErrorString,const char* pSourceFile,const int SourceLine)
{
#if defined(_DEBUG)
	if ( !TLDebug::IsEnabled() )
		return FALSE;
	
	TTempString String( pErrorString );
	return TLDebug::Break( String, pSourceFile, SourceLine );
#else
	return FALSE;
#endif
}

	
void TLArray::Debug::Print(const char* pErrorString,const char* pSourceFile,const int SourceLine)
{
#if defined(_DEBUG)
	if ( !TLDebug::IsEnabled() )
		return;
	
	TTempString String( pErrorString );
	TLDebug::Print( String, pSourceFile, SourceLine );
#endif
}

	

