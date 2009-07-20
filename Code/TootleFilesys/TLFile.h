
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
	Bool				ReadIntegerInRange(const TString& String,s32& Integer,s32 Min,s32 Max);	//	reads an integer out of a string, and does a min/max CheckInRange check. returns FALSE if out of range (in debug only, uses TLDebug_CHeckInRange)

	template<typename FLOATTYPE>
	FORCEINLINE Bool	ReadNextFloat(const TString& String,u32& CharIndex,FLOATTYPE& FloatType,Bool ReturnInvalidFloatZero=FALSE);
	template<>
	FORCEINLINE Bool	ReadNextFloat(const TString& String,u32& CharIndex,float& FloatType,Bool ReturnInvalidFloatZero);
	Bool				ReadNextFloatArray(const TString& String,u32& CharIndex,float* pFloats,u32 FloatSize,Bool ReturnInvalidFloatZero=FALSE);

	Bool				IsDatumString(const TString& String,TRef& DatumRef,TRef& ShapeType,Bool& IsJustDatum);	//	check if string marked as a datum. if IsOnlyDatum is FALSE then create geometry as well as a datum

	FORCEINLINE Bool	IsStringDirty(const TString& String)	{	return TRUE;	}	//	gr: for now assume all strings need cleaning up
	Bool				CleanString(TString& String);									//	cleanup string. Convert "\n" to a linefeed, convert tabs, do other generic string-replace's etc, returns if any changes are made
};




//----------------------------------------------------
//	generic tootle <Data> xml data formatting -> TBinary data conversion
//----------------------------------------------------
namespace TLFile
{
	TRef		GetDataTypeFromString(const TString& String);
	SyncBool	ImportBinaryData(TPtr<TXmlTag>& pTag,TBinary& BinaryData,TRefRef DataType);
	Bool		ParseXMLDataTree(TPtr<TXmlTag>& pTag,TBinaryTree& Data);	//	parse XML tag to Binary data[tree]
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
