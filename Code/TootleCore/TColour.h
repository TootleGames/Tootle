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

class TColour;



class TColour
{
public:
	TColour(float r=0.f, float g=0.f,float b=0.f,float a=1.f) : m_Rgba ( r,g,b,a )	{	}	
	TColour(const TColour& Colour) : m_Rgba ( Colour )								{	}
	TColour(u8 r, u8 g,u8 b,u8 a=255)												{	Set( r,g,b,a );	}
	TColour(u32 rgba)																{	Set( rgba );	}

	FORCEINLINE float4&			GetRgba()								{	return m_Rgba;	}
	FORCEINLINE float&			GetRed() 								{	return m_Rgba.x;	}
	FORCEINLINE float&			GetGreen() 								{	return m_Rgba.y;	}
	FORCEINLINE float&			GetBlue() 								{	return m_Rgba.z;	}
	FORCEINLINE float&			GetAlpha() 								{	return m_Rgba.w;	}
	FORCEINLINE const float4&	GetRgba() const							{	return m_Rgba;	}
	FORCEINLINE const float&	GetRed() const							{	return m_Rgba.x;	}
	FORCEINLINE const float&	GetGreen() const						{	return m_Rgba.y;	}
	FORCEINLINE const float&	GetBlue() const							{	return m_Rgba.z;	}
	FORCEINLINE const float&	GetAlpha() const						{	return m_Rgba.w;	}
	FORCEINLINE u32				GetRgba32() const						{	return TColour::MakeRgba32( GetRed8(), GetGreen8(), GetBlue8(), GetAlpha8() );	}	//	convert colours to u32 RGBA colour
	FORCEINLINE u8				GetRed8() const							{	return MakeComponent8( GetRed() );		}
	FORCEINLINE u8				GetGreen8() const						{	return MakeComponent8( GetGreen() );	}
	FORCEINLINE u8				GetBlue8() const						{	return MakeComponent8( GetBlue() );	}
	FORCEINLINE u8				GetAlpha8() const						{	return MakeComponent8( GetAlpha() );	}

	FORCEINLINE Bool			IsVisible() const						{	return GetAlpha() >= TLMaths::g_NearZero;	}
	FORCEINLINE Bool			IsTransparent() const					{	return GetAlpha() <= (1.f - TLMaths::g_NearZero);	}
	FORCEINLINE const float*	GetData() const							{	return GetRgba().GetData();	}

	FORCEINLINE void			Set(float r,float g,float b,float a)	{	m_Rgba.Set( r,g,b,a );	}
	FORCEINLINE void			Set(const TColour& Colour)				{	m_Rgba.Set( Colour.GetRgba() );	}
	FORCEINLINE void			Set(u8 r,u8 g,u8 b,u8 a)				{	m_Rgba.Set( MakeComponentf(r), MakeComponentf(g), MakeComponentf(b), MakeComponentf(a) );	}
	FORCEINLINE void			Set(u32 rgba)							{	Set( GetComponent8(rgba,0), GetComponent8(rgba,1), GetComponent8(rgba,2), GetComponent8(rgba,3) );	}
	FORCEINLINE void			Limit()									{	TLMaths::Limit( m_Rgba.x, 0.f, 1.f );	TLMaths::Limit( m_Rgba.y, 0.f, 1.f );	TLMaths::Limit( m_Rgba.z, 0.f, 1.f );	TLMaths::Limit( m_Rgba.w, 0.f, 1.f );	}

	FORCEINLINE TColour&		operator = (const TColour& Colour)		{	m_Rgba.Set( Colour.GetRgba() );	return *this;	}
	FORCEINLINE TColour&		operator = (u32 rgba)					{	Set( GetComponent8(rgba,0), GetComponent8(rgba,1), GetComponent8(rgba,2), GetComponent8(rgba,3) );	return *this;	}

	FORCEINLINE					operator const float4&() const			{	return m_Rgba;	}
	FORCEINLINE					operator float4&()						{	return m_Rgba;	}

	FORCEINLINE void			operator+=(const float4& v)					{	m_Rgba += v;	}
	FORCEINLINE void			operator-=(const float4& v)					{	m_Rgba -= v;	}
	FORCEINLINE void			operator*=(const float4& v)					{	m_Rgba *= v;	}
	FORCEINLINE void			operator/=(const float4& v)					{	m_Rgba /= v;	}

	FORCEINLINE void			operator+=(const float v)					{	m_Rgba += v;	}
	FORCEINLINE void			operator-=(const float v)					{	m_Rgba -= v;	}
	FORCEINLINE void			operator*=(const float v)					{	m_Rgba *= v;	}
	FORCEINLINE void			operator/=(const float v)					{	m_Rgba /= v;	}

	//	colour conversions
	static u32					MakeRgba32(u8 r,u8 g,u8 b,u8 a)				{	return GetComponent32(r,0) | GetComponent32(g,1) | GetComponent32(b,2) |GetComponent32(a,3);	}
	static u8					MakeComponent8(const float& c)				{	return (u8)( c * 255.f);	}
	static float				MakeComponentf(const u8& c)					{	return ((float)c) / 255.f;	}
	static u8					GetComponent8(u32 rgba,u32 ComponentIndex)	{	return (u8)( (rgba >> (ComponentIndex*8)) & 0xff );	}
	static u32					GetComponent32(u8 c,u32 ComponentIndex)		{	return ((u32)c) << (ComponentIndex*8);	}

	FORCEINLINE Bool			operator==(const TColour& Colour) const		{	return GetRgba() == Colour.GetRgba();	}	//	required for sorting in arrays... maybe we'll have a need for this in future
	FORCEINLINE Bool			operator!=(const TColour& Colour) const		{	return GetRgba() != Colour.GetRgba();	}	//	required for sorting in arrays... maybe we'll have a need for this in future
	FORCEINLINE Bool			operator<(const TColour& Colour) const		{	return FALSE;	}	//	required for sorting in arrays... maybe we'll have a need for this in future

	static TColour				Debug_GetColour(u32 ColourIndex);			//	get a debug colour. provide an index to cycle through colours 

private:
	float4						m_Rgba;
};



namespace TLColour
{
	extern TFixedArray<TColour,8>	g_Debug_Colours;		//	static list of debug colours - initialised in TLMaths::Init
}



//-------------------------------------------------------------------
//	get a debug colour. provide an index to cycle through colours 
//-------------------------------------------------------------------
inline TColour TColour::Debug_GetColour(u32 ColourIndex)
{
	return (TLColour::g_Debug_Colours.GetSize()==0) ? TColour() : TLColour::g_Debug_Colours[ ColourIndex % TLColour::g_Debug_Colours.GetSize() ];
}