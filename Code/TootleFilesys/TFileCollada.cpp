#include "TFileCollada.h"
#include <TootleAsset/TMesh.h>
#include <TootleMaths/TTessellate.h>


namespace TLString
{
	Bool	ReadNextLetter(const TString& String,u32& CharIndex, char& Char);
	Bool	ReadNextFloatArray(const TString& String,u32& CharIndex,float* pFloats,u32 FloatSize);

	template<typename FLOATTYPE>
	Bool	ReadNextFloat(const TString& String,u32& CharIndex,FLOATTYPE& FloatType);
	template<>
	Bool	ReadNextFloat(const TString& String,u32& CharIndex,float& FloatType);
};







TLFileSys::TFileCollada::TFileCollada(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml			( FileRef, FileTypeRef )
{
}



//--------------------------------------------------------
//	import the XML and convert from SVG to mesh
//--------------------------------------------------------
SyncBool TLFileSys::TFileCollada::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	Supported = TRUE;


	//	import xml
	SyncBool ImportResult = TFileXml::Import();
	if ( ImportResult != SyncTrue )
		return ImportResult;

	//	todo
	return SyncFalse;
}


