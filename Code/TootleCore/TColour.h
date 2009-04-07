/*------------------------------------------------------
	Generic "colour" type with conversions to/from other
	colour formats. If we stick with using this colour
	object everywhere then we keep conversions to a minimum
	and do little hacks here and there to speed it up 
	(eg. cache a u32 RGBA value whenever the colours change)

	All colour components are stored as 0-1 floats

-------------------------------------------------------*/
#pragma once
#include "TLTypes.h"
#include "TLMaths.h"
#include "TFixedArray.h"

class TColour;			//	128bit float RGBA colour
class TColour24;		//	24 bit RGB colour
class TColour32;		//	32 bit RGBA colour


namespace TLColour
{
	//	colour conversions
	FORCEINLINE u8				GetComponent8(u32 rgba,u32 ComponentIndex)	{	return (u8)( (rgba >> (ComponentIndex*8)) & 0xff );	}
	FORCEINLINE u32				GetComponent32(u8 c,u32 ComponentIndex)		{	return ((u32)c) << (ComponentIndex*8);	}
	
	FORCEINLINE u32				MakeRgba32(u8 r,u8 g,u8 b,u8 a)				{	return GetComponent32(r,0) | GetComponent32(g,1) | GetComponent32(b,2) |GetComponent32(a,3);	}
	FORCEINLINE u8				MakeComponent8(const float& c)				{	return (u8)( c * 255.f);	}
	FORCEINLINE float			MakeComponentf(const u8& c)					{	return ((float)c) / 255.f;	}

	FORCEINLINE TColour			Debug_GetColour(u32 ColourIndex);			//	get a debug colour. provide an index to cycle through colours 
}




class TColour
{
	friend class TColour24;
	friend class TColour32;
public:
	TColour() : m_Rgba ( 0.f,0.f,0.f,1.f )								{	}	
	TColour(float r, float g,float b,float a=1.f) : m_Rgba ( r,g,b,a )	{	}	
	TColour(u8 r, u8 g,u8 b,u8 a=255)									{	Set( r,g,b,a );	}
	TColour(const TColour& Colour);
	TColour(const TColour24& Colour);
	TColour(const TColour32& Colour);

	FORCEINLINE float4&			GetRgbaf()								{	return m_Rgba;	}
	FORCEINLINE float&			GetRedf() 								{	return m_Rgba.x;	}
	FORCEINLINE float&			GetGreenf() 							{	return m_Rgba.y;	}
	FORCEINLINE float&			GetBluef() 								{	return m_Rgba.z;	}
	FORCEINLINE float&			GetAlphaf() 							{	return m_Rgba.w;	}
	FORCEINLINE const float4&	GetRgbaf() const						{	return m_Rgba;	}
	FORCEINLINE const float&	GetRedf() const							{	return m_Rgba.x;	}
	FORCEINLINE const float&	GetGreenf() const						{	return m_Rgba.y;	}
	FORCEINLINE const float&	GetBluef() const						{	return m_Rgba.z;	}
	FORCEINLINE const float&	GetAlphaf() const						{	return m_Rgba.w;	}

	//FORCEINLINE u32				GetRgba32() const						{	return TLColour::MakeRgba32( GetRed8(), GetGreen8(), GetBlue8(), GetAlpha8() );	}	//	convert colours to u32 RGBA colour
	FORCEINLINE u8				GetRed8() const							{	return TLColour::MakeComponent8( GetRedf() );		}
	FORCEINLINE u8				GetGreen8() const						{	return TLColour::MakeComponent8( GetGreenf() );	}
	FORCEINLINE u8				GetBlue8() const						{	return TLColour::MakeComponent8( GetBluef() );	}
	FORCEINLINE u8				GetAlpha8() const						{	return TLColour::MakeComponent8( GetAlphaf() );	}

	FORCEINLINE Bool			IsVisible() const						{	return GetAlphaf() > TLMaths_NearZero;	}
	FORCEINLINE Bool			IsTransparent() const					{	return GetAlphaf() < TLMaths_NearOne;	}
	FORCEINLINE const float*	GetData() const							{	return m_Rgba.GetData();	}
	FORCEINLINE float4&			GetData4()								{	return m_Rgba;	}
	FORCEINLINE const float4&	GetData4() const						{	return m_Rgba;	}
	FORCEINLINE float3&			GetData3()								{	return m_Rgba.xyz();	}
	FORCEINLINE const float3&	GetData3() const						{	return m_Rgba.xyz();	}

	FORCEINLINE void			Set(float r,float g,float b,float a)	{	m_Rgba.Set( r,g,b,a );	Limit();	}
	FORCEINLINE void			Set(u8 r,u8 g,u8 b,u8 a)				{	m_Rgba.Set( TLColour::MakeComponentf(r), TLColour::MakeComponentf(g), TLColour::MakeComponentf(b), TLColour::MakeComponentf(a) );	}

	FORCEINLINE void			operator=(const u32& rgba)				{	Set( TLColour::GetComponent8(rgba,0), TLColour::GetComponent8(rgba,1), TLColour::GetComponent8(rgba,2), TLColour::GetComponent8(rgba,3) );	}
	FORCEINLINE void			operator=(const TColour& Colour);
	FORCEINLINE void			operator=(const TColour24& Colour);
	FORCEINLINE void			operator=(const TColour32& Colour);

	FORCEINLINE					operator const float4&() const			{	return m_Rgba;	}
	FORCEINLINE					operator float4&()						{	return m_Rgba;	}

	FORCEINLINE void			operator+=(const float4& v)					{	m_Rgba += v;	Limit();	}
	FORCEINLINE void			operator-=(const float4& v)					{	m_Rgba -= v;	Limit();	}
	FORCEINLINE void			operator*=(const float4& v)					{	m_Rgba *= v;	Limit();	}
	FORCEINLINE void			operator/=(const float4& v)					{	m_Rgba /= v;	Limit();	}

	FORCEINLINE void			operator+=(const float v)					{	m_Rgba += v;	Limit();	}
	FORCEINLINE void			operator-=(const float v)					{	m_Rgba -= v;	Limit();	}
	FORCEINLINE void			operator*=(const float v)					{	m_Rgba *= v;	Limit();	}
	FORCEINLINE void			operator/=(const float v)					{	m_Rgba /= v;	Limit();	}

	FORCEINLINE Bool			operator==(const TColour& Colour) const		{	return m_Rgba == Colour.m_Rgba;	}	//	required for sorting in arrays... maybe we'll have a need for this in future
	FORCEINLINE Bool			operator!=(const TColour& Colour) const		{	return m_Rgba != Colour.m_Rgba;	}	//	required for sorting in arrays... maybe we'll have a need for this in future
	FORCEINLINE Bool			operator<(const TColour& Colour) const		{	return FALSE;	}	//	required for sorting in arrays... maybe we'll have a need for this in future

protected:
	FORCEINLINE void			Limit()										{	TLMaths::Limit( m_Rgba.x, 0.f, 1.f );	TLMaths::Limit( m_Rgba.y, 0.f, 1.f );	TLMaths::Limit( m_Rgba.z, 0.f, 1.f );	TLMaths::Limit( m_Rgba.w, 0.f, 1.f );	}

protected:
	float4						m_Rgba;
};



class TColour24
{
	friend class TColour;
	friend class TColour32;
public:
	TColour24() : m_Rgb ( 0,0,0 )										{	}
	TColour24(u8 r, u8 g,u8 b) : m_Rgb ( r,g,b )						{	}
	TColour24(const TColour& Colour);
	TColour24(const TColour24& Colour);
	TColour24(const TColour32& Colour);

	FORCEINLINE float4			GetRgbaf() const						{	return float4( GetRedf(), GetGreenf(), GetBluef(), GetAlphaf() );	}
	FORCEINLINE float			GetRedf() const							{	return TLColour::MakeComponentf( GetRed8() );	}
	FORCEINLINE float			GetGreenf() const						{	return TLColour::MakeComponentf( GetBlue8() );	}
	FORCEINLINE float			GetBluef() const						{	return TLColour::MakeComponentf( GetGreen8() );	}
	FORCEINLINE float			GetAlphaf() const						{	return 1.f;	}
	FORCEINLINE Type4<u8>		GetRgba32(u8 Alpha=255) const			{	return m_Rgb.xyzw( Alpha );	}
	FORCEINLINE const Type3<u8>&	GetRgb24() const					{	return m_Rgb;	}
	FORCEINLINE const u8&		GetRed8() const							{	return m_Rgb.x;	}
	FORCEINLINE const u8&		GetGreen8() const						{	return m_Rgb.y;	}
	FORCEINLINE const u8&		GetBlue8() const						{	return m_Rgb.z;	}
	FORCEINLINE const u8		GetAlpha8() const						{	return 255;	}

	FORCEINLINE Bool			IsVisible() const						{	return GetAlpha8() > 0;	}
	FORCEINLINE Bool			IsTransparent() const					{	return GetAlpha8() < 255;	}
	FORCEINLINE const u8*		GetData() const							{	return m_Rgb.GetData();	}
	FORCEINLINE const Type3<u8>&	GetData3() const						{	return m_Rgb;	}

	FORCEINLINE void			Set(u8 r,u8 g,u8 b)						{	m_Rgb.Set( r, g, b );	}

	FORCEINLINE void			operator=(const TColour& Colour);
	FORCEINLINE void			operator=(const TColour24& Colour);
	FORCEINLINE void			operator=(const TColour32& Colour);

	FORCEINLINE Bool			operator==(const TColour24& Colour) const	{	return m_Rgb == Colour.m_Rgb;	}
	FORCEINLINE Bool			operator!=(const TColour24& Colour) const	{	return m_Rgb != Colour.m_Rgb;	}

protected:
	Type3<u8>					m_Rgb;
};



class TColour32
{
	friend class TColour;
	friend class TColour24;
public:
	TColour32() : m_Rgba ( 0,0,0,255 )									{	}
	TColour32(u8 r, u8 g,u8 b,u8 a=255) : m_Rgba ( r,g,b,a )			{	}
	TColour32(u32 rgba)													{	Set( rgba );	}
	TColour32(const TColour& Colour);
	TColour32(const TColour24& Colour);
	TColour32(const TColour32& Colour);

	FORCEINLINE float4			GetRgbaf() const						{	return float4( GetRedf(), GetGreenf(), GetBluef(), GetAlphaf() );	}
	FORCEINLINE float			GetRedf() const							{	return TLColour::MakeComponentf( GetRed8() );	}
	FORCEINLINE float			GetGreenf() const						{	return TLColour::MakeComponentf( GetBlue8() );	}
	FORCEINLINE float			GetBluef() const						{	return TLColour::MakeComponentf( GetGreen8() );	}
	FORCEINLINE float			GetAlphaf() const						{	return TLColour::MakeComponentf( GetAlpha8() );	}
	FORCEINLINE u32				GetRgba32() const						{	return *GetData32();	}
	FORCEINLINE const u8&		GetRed8() const							{	return m_Rgba.x;	}
	FORCEINLINE const u8&		GetGreen8() const						{	return m_Rgba.y;	}
	FORCEINLINE const u8&		GetBlue8() const						{	return m_Rgba.z;	}
	FORCEINLINE const u8&		GetAlpha8() const						{	return m_Rgba.w;	}

	FORCEINLINE Bool			IsVisible() const						{	return GetAlpha8() > 0;	}
	FORCEINLINE Bool			IsTransparent() const					{	return GetAlpha8() < 255;	}
	FORCEINLINE const u8*		GetData() const							{	return m_Rgba.GetData();	}
	FORCEINLINE const u32*		GetData32() const						{	return (u32*)m_Rgba.GetData();	}
	FORCEINLINE const Type4<u8>&	GetData4() const					{	return m_Rgba;	}
	FORCEINLINE const Type3<u8>&	GetData3() const					{	return m_Rgba.xyz();	}

	FORCEINLINE void			Set(u32 rgba)							{	*GetData32() = rgba;	}

	FORCEINLINE void			operator=(const TColour& Colour);
	FORCEINLINE void			operator=(const TColour24& Colour);
	FORCEINLINE void			operator=(const TColour32& Colour);

	FORCEINLINE Bool			operator==(const TColour32& Colour) const	{	return m_Rgba == Colour.m_Rgba;	}	//	required for sorting in arrays... maybe we'll have a need for this in future
	FORCEINLINE Bool			operator!=(const TColour32& Colour) const	{	return m_Rgba != Colour.m_Rgba;	}	//	required for sorting in arrays... maybe we'll have a need for this in future

protected:
	FORCEINLINE u32*			GetData32()									{	return (u32*)m_Rgba.GetData();	}

protected:
	Type4<u8>					m_Rgba;
};








namespace TLColour
{
	extern TFixedArray<TColour,8>	g_Debug_Colours;		//	static list of debug colours - initialised in TLMaths::Init
}


//-------------------------------------------------------------------
//	get a debug colour. provide an index to cycle through colours 
//-------------------------------------------------------------------
FORCEINLINE TColour TLColour::Debug_GetColour(u32 ColourIndex)
{
	return (TLColour::g_Debug_Colours.GetSize()==0) ? TColour() : TLColour::g_Debug_Colours[ ColourIndex % TLColour::g_Debug_Colours.GetSize() ];
}


//	constructors/= operators for when we can't define them before the other class
FORCEINLINE TColour::TColour(const TColour& Colour) : m_Rgba( Colour.m_Rgba )			{	}
FORCEINLINE TColour::TColour(const TColour24& Colour) : m_Rgba( Colour.GetRedf(), Colour.GetGreenf(), Colour.GetBluef(), Colour.GetAlphaf() )			{	}
FORCEINLINE TColour::TColour(const TColour32& Colour) : m_Rgba( Colour.GetRedf(), Colour.GetGreenf(), Colour.GetBluef(), Colour.GetAlphaf() )			{	}
FORCEINLINE void TColour::operator=(const TColour& Colour)		{	m_Rgba = Colour.m_Rgba;	}
FORCEINLINE void TColour::operator=(const TColour24& Colour)	{	Set( Colour.GetRed8(), Colour.GetGreen8(), Colour.GetBlue8(), Colour.GetAlpha8() );	}
FORCEINLINE void TColour::operator=(const TColour32& Colour)	{	Set( Colour.GetRed8(), Colour.GetGreen8(), Colour.GetBlue8(), Colour.GetAlpha8() );	}

FORCEINLINE TColour24::TColour24(const TColour& Colour) : m_Rgb( Colour.GetRed8(), Colour.GetGreen8(), Colour.GetBlue8() )	{	}
FORCEINLINE TColour24::TColour24(const TColour24& Colour) : m_Rgb( Colour.m_Rgb )			{	}
FORCEINLINE TColour24::TColour24(const TColour32& Colour) : m_Rgb( Colour.m_Rgba.xyz() )	{	}
FORCEINLINE void TColour24::operator=(const TColour& Colour)		{	m_Rgb.Set( Colour.GetRed8(), Colour.GetGreen8(), Colour.GetBlue8() );	}
FORCEINLINE void TColour24::operator=(const TColour24& Colour)		{	m_Rgb = Colour.m_Rgb;	}
FORCEINLINE void TColour24::operator=(const TColour32& Colour)	{	m_Rgb.Set( Colour.GetRed8(), Colour.GetGreen8(), Colour.GetBlue8() );	}

FORCEINLINE TColour32::TColour32(const TColour& Colour) : m_Rgba ( Colour.GetRed8(), Colour.GetGreen8(), Colour.GetBlue8(), Colour.GetAlpha8() )		{	}
FORCEINLINE TColour32::TColour32(const TColour24& Colour) : m_Rgba ( Colour.m_Rgb.xyzw(255) )	{	}
FORCEINLINE TColour32::TColour32(const TColour32& Colour) : m_Rgba ( Colour.m_Rgba )		{	}
FORCEINLINE void TColour32::operator=(const TColour& Colour)		{	m_Rgba.Set( Colour.GetRed8(), Colour.GetGreen8(), Colour.GetBlue8(), Colour.GetAlpha8() );	}
FORCEINLINE void TColour32::operator=(const TColour24& Colour)		{	m_Rgba.Set( Colour.GetRed8(), Colour.GetGreen8(), Colour.GetBlue8(), Colour.GetAlpha8() );	}
FORCEINLINE void TColour32::operator=(const TColour32& Colour)		{	*GetData32() = *Colour.GetData32();	}









TLCore_DeclareIsDataType( TColour );
TLCore_DeclareIsDataType( TColour24 );
TLCore_DeclareIsDataType( TColour32 );
