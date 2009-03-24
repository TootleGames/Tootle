
#pragma once

#include <TootleCore/TRef.h>
#include <TootleCore/TString.h>
#include <TootleCore/TBinary.h>
#include <TootleCore/TPtr.h>

#include "TXml.h"


//----------------------------------------------------
//	string reading routines for importing arrays of data
//----------------------------------------------------
namespace TLString
{
	Bool				ReadNextLetter(const TString& String,u32& CharIndex,char& Char);
	Bool				ReadNextInteger(const TString& String,u32& CharIndex,s32& Integer);

	template<typename FLOATTYPE>
	FORCEINLINE Bool	ReadNextFloat(const TString& String,u32& CharIndex,FLOATTYPE& FloatType,Bool ReturnInvalidFloatZero=FALSE);
	template<>
	FORCEINLINE Bool	ReadNextFloat(const TString& String,u32& CharIndex,float& FloatType,Bool ReturnInvalidFloatZero);
	Bool				ReadNextFloatArray(const TString& String,u32& CharIndex,float* pFloats,u32 FloatSize,Bool ReturnInvalidFloatZero=FALSE);

	Bool				IsDatumString(const TString& String,TRef& DatumRef,TRef& ShapeType);	//	check if string marked as a datum
};




//----------------------------------------------------
//	generic tootle <Data> xml data formatting -> TBinary data conversion
//----------------------------------------------------
namespace TLFile
{
	TRef		GetDataTypeFromString(const TString& String);
	SyncBool	ImportBinaryData(TPtr<TXmlTag>& pTag,TBinary& BinaryData,TRefRef DataType);
}






template<typename FLOATTYPE>
FORCEINLINE Bool TLString::ReadNextFloat(const TString& String,u32& CharIndex,FLOATTYPE& FloatType,Bool ReturnInvalidFloatZero)
{
	return ReadNextFloatArray( String, CharIndex, FloatType.GetData(), FloatType.GetSize(), ReturnInvalidFloatZero );
}

template<>
FORCEINLINE Bool TLString::ReadNextFloat(const TString& String,u32& CharIndex,float& FloatType,Bool ReturnInvalidFloatZero)
{
	return ReadNextFloatArray( String, CharIndex, &FloatType, 1, ReturnInvalidFloatZero );
}
