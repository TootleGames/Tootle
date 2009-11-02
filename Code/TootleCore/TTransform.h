/*
 *  TTransform.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 27/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TLTypes.h"
#include "TLMaths.h"
#include "TArray.h" 

namespace TLMaths
{
	class TTransform;	//	Transform class - encapsulates common usage of position, rotation and scale
}


class TLMaths::TTransform
{
public:
	TTransform() : m_Valid ( 0x0 )			{	}
	
	FORCEINLINE void				SetScale(const float3& Scale)							{	m_Scale = Scale;			SetScaleValid();	}
	FORCEINLINE void				SetTranslate(const float3& Translate)					{	m_Translate = Translate;	SetTranslateValid();	}
	FORCEINLINE void				SetRotation(const TQuaternion& Rotation)				{	m_Rotation = Rotation;		SetRotationValid();	}
	
	FORCEINLINE u8					SetHasChanged(const TLMaths::TTransform& NewTransform);					//	set all elements and return what bits have changed
	FORCEINLINE u8					SetScaleHasChanged(const float3& Scale);								//	set scale, return's the Transform bit that has changed
	FORCEINLINE u8					SetTranslateHasChanged(const float3& Translate);						//	set translation, return's the Transform bit that has changed
	FORCEINLINE u8					SetTranslateHasChanged(const float3& Translate,float MinChange);		//	set translation, return's the Transform bit that has changed
	FORCEINLINE u8					SetRotationHasChanged(const TQuaternion& Rotation);						//	set rotation, return's the Transform bit that has changed
	FORCEINLINE u8					SetRotationHasChanged(const TQuaternion& Rotation,float MinChange);		//	set rotation, return's the Transform bit that has changed
	
	FORCEINLINE float3&				GetTranslate() 			{	return m_Translate;	}	//	only use if HasTranslate()
	FORCEINLINE float3&				GetScale()				{	return m_Scale;	}		//	only use if HasScale()
	FORCEINLINE TQuaternion&		GetRotation() 			{	return m_Rotation;	}	//	only use if HasRotation()
	FORCEINLINE const float3&		GetTranslate() const	{	Debug_Assert( HasTranslate(), "Translate accessed but is invalid");	return m_Translate;	}
	FORCEINLINE const float3&		GetScale() const		{	Debug_Assert( HasScale(), "Scale accessed but is invalid");			return m_Scale;	}
	FORCEINLINE const TQuaternion&	GetRotation() const		{	Debug_Assert( HasRotation(), "Rotation accessed but is invalid");	return m_Rotation;	}
	
	FORCEINLINE void				SetInvalid()			{	m_Valid = 0x0;	}
	FORCEINLINE void				SetTranslateInvalid()	{	m_Valid &= ~TLMaths_TransformBitTranslate;	/*m_Translate.Set( 0.f, 0.f, 0.f );*/	}
	FORCEINLINE void				SetScaleInvalid()		{	m_Valid &= ~TLMaths_TransformBitScale;		/*m_Scale.Set( 1.f, 1.f, 1.f );*/	}
	FORCEINLINE void				SetRotationInvalid()	{	m_Valid &= ~TLMaths_TransformBitRotation;		/*m_Rotation.SetIdentity();*/	}
	
	FORCEINLINE void				SetTranslateValid()				{	m_Valid |= TLMaths_TransformBitTranslate;	}
	FORCEINLINE void				SetTranslateValid(Bool Valid)	{	if ( Valid )	SetTranslateValid();	else SetTranslateInvalid();	}
	FORCEINLINE void				SetScaleValid()					{	m_Valid |= TLMaths_TransformBitScale;		}
	FORCEINLINE void				SetRotationValid()				{	m_Valid |= TLMaths_TransformBitRotation;	}
	
	FORCEINLINE Bool				HasAnyTransform() const			{	return (m_Valid != 0x0);	}
	FORCEINLINE Bool				HasTranslate() const			{	return (m_Valid & TLMaths_TransformBitTranslate) != 0x0;	}
	FORCEINLINE Bool				HasScale() const				{	return (m_Valid & TLMaths_TransformBitScale) != 0x0;	}
	FORCEINLINE Bool				HasRotation() const				{	return (m_Valid & TLMaths_TransformBitRotation) != 0x0;	}
	FORCEINLINE u8					GetHasTransformBits() const		{	return m_Valid;	}
	
	//	these Transform()'s are like matrix multiplies
	void				Transform(const TLMaths::TTransform& Trans);			//	transform this by another transform, this is like a local tranform, if Trans says "move right", it will move right, relative to the rotation. dumb faster method which doesn't do checks
	u8					Transform_HasChanged(const TLMaths::TTransform& Trans);	//	transform this by another transform, this is like a local tranform, if Trans says "move right", it will move right, relative to the rotation. returns elements that have changed (slightly slower, but if your caller does much LESS work if nothing changed then use this)
	void				Transform(float3& Vector) const;				//	transform vector
	void				Transform(float2& Vector) const;				//	transform vector
	void				Untransform(float3& Vector) const;				//	untransform vector
	void				Untransform(float2& Vector) const;				//	untransform vector
	void				Invert();										//	make an "untransform" from this transform. (inverts rotation, scale, trans)
	
	template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>	void	Transform(TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>& VectorArray) const		{	for ( u32 i=0;	i<VectorArray.GetSize();	i++ )	Transform( VectorArray[i] );	}
	template<typename TYPE, class SORTPOLICY, class ALLOCATORPOLICY>	void	Untransform(TArray<TYPE, SORTPOLICY, ALLOCATORPOLICY>& VectorArray) const	{	for ( u32 i=0;	i<VectorArray.GetSize();	i++ )	Untransform( VectorArray[i] );	}
	
	//	matrix style "adds"
	void				AddTransform(const TLMaths::TTransform& Trans);				//	Modify the transform values by another transform, Translates the translate, scales the scale, rotates the rotation. Doesn't multiply and rotate the translate etc
	u8					AddTransform_HasChanged(const TLMaths::TTransform& Trans);	//	Modify the transform values by another transform, Translates the translate, scales the scale, rotates the rotation. Doesn't multiply and rotate the translate etc. returns elements that have changed (slightly slower, but if your caller does much LESS work if nothing changed then use this)
	
	u8					ImportData(TBinaryTree& Data);				//	import transform data from binary data/message/etc- returns bitmask of the attributes that have changed
	u8					ExportData(TBinaryTree& Data,u8 TransformBits=TLMaths_TransformBitAll);	//	export all our valid data to this binary data- can optionally make it only write certain attributes. Returns bits of the attributes written.
	
	Bool				operator==(const TLMaths::TTransform& Transform) const;		//	see if transforms are same
	Bool				operator!=(const TLMaths::TTransform& Transform) const;		//	see if transforms are different 
	
private:
#ifdef _DEBUG
	FORCEINLINE void	Debug_Assert(Bool Condition,const char* pString) const		{	if ( !Condition )	Debug_Assert( pString );	}
#else
	FORCEINLINE void	Debug_Assert(Bool Condition,const char* pString) const		{	}
#endif
	
	void				Debug_Assert(const char* pString) const;
	
protected:
	float3				m_Translate;		//	simple translation
	float3				m_Scale;			//	scale
	TQuaternion			m_Rotation;			//	human-usable rotation
	u8					m_Valid;			//	bit mask of validity.	TRANSFORM_BIT_XXX - last for byte alignment
};



//--------------------------------------------------
//	set all elements and return what bits have changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetHasChanged(const TLMaths::TTransform& NewTransform)
{
	u8 Changes = 0x0;
	
	if ( NewTransform.HasScale() )
		Changes |= SetScaleHasChanged( NewTransform.GetScale() );
	
	if ( NewTransform.HasRotation() )
		Changes |= SetRotationHasChanged( NewTransform.GetRotation() );
	
	if ( NewTransform.HasTranslate() )
		Changes |= SetTranslateHasChanged( NewTransform.GetTranslate() );
	
	return Changes;
}


//--------------------------------------------------
//	set scale, return's the Transform bit that has changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetScaleHasChanged(const float3& Scale)
{
	if ( !HasScale() || Scale != m_Scale )
	{
		SetScale( Scale );
		return TLMaths_TransformBitScale;
	}
	else
		return 0x0;
}


//--------------------------------------------------
//	set translation, return's the Transform bit that has changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetTranslateHasChanged(const float3& Translate)
{
	if ( !HasTranslate() || Translate != m_Translate )
	{
		SetTranslate( Translate );
		return TLMaths_TransformBitTranslate;
	}
	else
		return 0x0;
}


//--------------------------------------------------
//	set translation, return's the Transform bit that has changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetTranslateHasChanged(const float3& Translate,float MinChange)
{
	if ( !HasTranslate() )
	{
		SetTranslate( Translate );
		return TLMaths_TransformBitTranslate;
	}
	else
	{
		if ( m_Translate.HasDifferenceMin( Translate, MinChange ) )
		{
			SetTranslate( Translate );
			return TLMaths_TransformBitTranslate;
		}
		else
		{
			//	very minor change - don't apply
			return 0x0;
		}
	}
}




//--------------------------------------------------
//	set rotation, return's the Transform bit that has changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetRotationHasChanged(const TLMaths::TQuaternion& Rotation,float MinChange)
{
	if ( !HasRotation() )
	{
		SetRotation( Rotation );
		return TLMaths_TransformBitRotation;
	}
	else
	{
		if ( m_Rotation.GetData().HasDifferenceMin( Rotation.GetData(), MinChange ) )
		{
			SetRotation( Rotation );
			return TLMaths_TransformBitRotation;
		}
		else
		{
			//	very minor change - don't apply
			return 0x0;
		}
	}
}




//--------------------------------------------------
//	set rotation, return's the Transform bit that has changed
//--------------------------------------------------
FORCEINLINE u8 TLMaths::TTransform::SetRotationHasChanged(const TLMaths::TQuaternion& Rotation)
{
	if ( !HasRotation() || Rotation != m_Rotation )
	{
		SetRotation( Rotation );
		return TLMaths_TransformBitRotation;
	}
	else
		return 0x0;
}




