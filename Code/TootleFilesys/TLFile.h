
#pragma once

#include <TootleCore/TRef.h>
#include <TootleCore/TString.h>
#include <TootleCore/TBinary.h>
#include <TootleCore/TPtr.h>

#include "TXml.h"


namespace TLString
{
	Bool	ReadNextLetter(const TString& String,u32& CharIndex, char& Char);
	Bool	ReadNextFloatArray(const TString& String,u32& CharIndex,float* pFloats,u32 FloatSize);

	/*
	template<typename FLOATTYPE>
	Bool	ReadNextFloat(const TString& String,u32& CharIndex,FLOATTYPE& FloatType);
	template<>
	Bool	ReadNextFloat(const TString& String,u32& CharIndex,float& FloatType);
	*/

	Bool	ReadNextFloatArray(const TString& String,u32& CharIndex,float* pFloats,u32 FloatSize);
};

/*
template<typename FLOATTYPE>
Bool TLString::ReadNextFloat(const TString& String,u32& CharIndex,FLOATTYPE& FloatType)
{
	return ReadNextFloatArray( String, CharIndex, FloatType.GetData(), FloatType.GetSize() );
}

template<>
Bool TLString::ReadNextFloat(const TString& String,u32& CharIndex,float& FloatType)
{
	return ReadNextFloatArray( String, CharIndex, &FloatType, 1 );
}
*/



namespace TLFile
{
	TRef		GetDataTypeFromString(const TString& String);
	SyncBool	ImportBinaryData(TPtr<TXmlTag>& pTag,TBinary& BinaryData,TRefRef DataType);
}
