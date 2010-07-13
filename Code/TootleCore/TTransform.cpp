/*
 *  TTransform.cpp
 *  TootleCore
 *
 *  Created by Duane Bradbury on 27/10/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "TTransform.h"
#include "TBinaryTree.h"
#include "TLDebug.h"


//-----------------------------------------------------------
//	import transform data from binary data/message/etc- ChangedBits specifies which parts have changed
//-----------------------------------------------------------
u8 TLMaths::TTransform::ImportData(TBinaryTree& Data)
{
	u8 ChangedBits = 0x0;
	
	
	//	import translate
	float3 OldTranslate = m_Translate;
	Bool ChangedTransform = Data.ImportData( Properties::Translation, m_Translate );
	
	//	gr: now support setting of just the Z
	ChangedTransform |= Data.ImportData( Properties::ZTranslation, m_Translate.z );
	
	if ( ChangedTransform )
	{
		TLDebug_CheckFloat( m_Translate );
		
		//	only consider value changed if different, or was preivously invalid
		if ( !HasTranslate() || m_Translate != OldTranslate )
			ChangedBits |= TLMaths_TransformBitTranslate;
	}
	
	//	import scale
	float3 OldScale = m_Scale;
	if ( Data.ImportData( Properties::Scale, m_Scale ) )
	{
		TLDebug_CheckFloat( m_Scale );
		
		//	only consider value changed if different, was preivously invalid
		if ( !HasScale() || m_Scale != OldScale )
			ChangedBits |= TLMaths_TransformBitScale;
	}
	
	
	//	import rotation
	TQuaternion OldRotation = m_Rotation;
	if ( Data.ImportData( Properties::Rotation, m_Rotation ) )
	{
		TLDebug_CheckFloat( m_Rotation );
		
		//	only consider value changed if different, or was preivously invalid
		if ( !HasRotation() || m_Rotation != OldRotation )
			ChangedBits |= TLMaths_TransformBitRotation;
	}
	
	//	anything we imported is now considered valid...
	m_Valid |= ChangedBits;
	
	return ChangedBits;
}


//-----------------------------------------------------------
//	export all our valid data to this binary data- can optionally make it only write certain attributes. Returns attributes written
//-----------------------------------------------------------
u8 TLMaths::TTransform::ExportData(TBinaryTree& Data,u8 TransformBits) const
{
	//	only write bits that are valid
	TransformBits &= m_Valid;
	
	if ( (TransformBits & TLMaths_TransformBitTranslate) != 0x0 )
	{
		TLDebug_CheckFloat( m_Translate );
		Data.ExportData( Properties::Translation, m_Translate );
	}
	
	if ( (TransformBits & TLMaths_TransformBitScale) != 0x0 )
	{
		TLDebug_CheckFloat( m_Scale );
		Data.ExportData( Properties::Scale, m_Scale );
	}
	
	if ( (TransformBits & TLMaths_TransformBitRotation) != 0x0 )
	{
		TLDebug_CheckFloat( m_Rotation );
		Data.ExportData( Properties::Rotation, m_Rotation );
	}
	
	return TransformBits;
}


//-----------------------------------------------------------
//	export all our valid data to this binary data- can optionally make it only write certain attributes. Returns attributes written
//-----------------------------------------------------------
u8 TLMaths::TTransform::ReplaceData(TBinaryTree& Data,u8 TransformBits) const
{
	//	need to remove some parts of the data
	u8 RemoveBits = m_Valid & ~TransformBits; 

	//	only write bits that are valid
	TransformBits &= m_Valid;
	
	if ( (TransformBits & TLMaths_TransformBitTranslate) != 0x0 )
	{
		TLDebug_CheckFloat( m_Translate );
		Data.ReplaceData( Properties::Translation, m_Translate );
	}
	else if ( ( RemoveBits & TLMaths_TransformBitTranslate ) != 0x0 )
	{
		Data.RemoveChild( Properties::Translation );
	}

	if ( (TransformBits & TLMaths_TransformBitScale) != 0x0 )
	{
		TLDebug_CheckFloat( m_Scale );
		Data.ReplaceData( Properties::Scale, m_Scale );
	}
	else if ( ( RemoveBits & TLMaths_TransformBitScale ) != 0x0 )
	{
		Data.RemoveChild( Properties::Scale );
	}

	if ( (TransformBits & TLMaths_TransformBitRotation) != 0x0 )
	{
		TLDebug_CheckFloat( m_Rotation );
		Data.ReplaceData( Properties::Rotation, m_Rotation );
	}
	else if ( ( RemoveBits & TLMaths_TransformBitRotation ) != 0x0 )
	{
		Data.RemoveChild( Properties::Rotation );
	}
	
	return TransformBits;
}


//-----------------------------------------------------------
//	transform this by Trans.
//	THIS is the parent, Trans is the "child" transform
//-----------------------------------------------------------
void TLMaths::TTransform::Transform(const TLMaths::TTransform& Trans)
{
	if ( Trans.HasTranslate() )
	{
		float3 Translate = Trans.m_Translate;
		
		//	gr: this is from the render code, we transform the translate of the child by ourselves(the parent)
		//		so the translate gets rotated, scaled etc... theres a possibility this might be wrong for other things, but Im not sure if it is or not
		//		it's done BEFORE the merge of the rotation and the matrix of the child
		//		the order of the other parts doesn't matter as they dont interact, where as this does
		Transform( Translate );
		
		SetTranslate( Translate );
		TLDebug_CheckFloat( Translate );
	}
	
	
	if ( Trans.HasScale() )
	{
		if ( HasScale() )
			m_Scale *= Trans.m_Scale;
		else
			SetScale( Trans.m_Scale );
		
		TLDebug_CheckFloat( m_Scale );
	}
	
	if ( Trans.HasRotation() )
	{
		if ( HasRotation() )
		{
			m_Rotation *= Trans.m_Rotation;
		}
		else
		{
			SetRotation( Trans.m_Rotation );
			m_Rotation.Normalise();
		}
		
		TLDebug_CheckFloat( m_Rotation );
	}
}


//-----------------------------------------------------------
//	transform this by another transform. returns elements that have changed 
//	(slightly slower, but if your caller does much LESS work if nothing changed then use this)
//-----------------------------------------------------------
u8 TLMaths::TTransform::Transform_HasChanged(const TLMaths::TTransform& Trans)
{
	u8 ChangedBits = 0x0;
	
	if ( Trans.HasTranslate() )
	{
		if ( Trans.m_Translate.IsNonZero() )
		{
			float3 Translate = Trans.m_Translate;
			
			//	gr: this is from the render code, we transform the translate of the child by ourselves(the parent)
			//		so the translate gets rotated, scaled etc... theres a possibility this might be wrong for other things, but Im not sure if it is or not
			//		it's done BEFORE the merge of the rotation and the matrix of the child
			//		the order of the other parts doesn't matter as they dont interact, where as this does
			Transform( Translate );
			
			ChangedBits |= SetTranslateHasChanged( Translate );
			TLDebug_CheckFloat( Translate );
		}
	}
	
	
	if ( Trans.HasScale() )
	{
		if ( HasScale() )
			m_Scale *= Trans.m_Scale;
		else
			SetScale( Trans.m_Scale );
		
		//	assume scale always changes... can check Trans.m_Scale.LengthSq() == 1 i think to see if its all 1's. Not sure how accurate that is...		
		ChangedBits |= TLMaths_TransformBitScale;
		
		TLDebug_CheckFloat( m_Scale );
	}
	
	if ( Trans.HasRotation() )
	{
		//	gr: needs normalising?
		if ( HasRotation() )
			m_Rotation *= Trans.m_Rotation;
		else
			SetRotation( Trans.m_Rotation );
		
		//	assume rotation always changes... 
		ChangedBits |= TLMaths_TransformBitScale;
		
		TLDebug_CheckFloat( m_Rotation );
	}
	
	return ChangedBits;
}


//-----------------------------------------------------------
//	Modify the transform values by another transform, Translates the translate, scales the scale, rotates the rotation. Doesn't multiply and rotate the translate etc
//-----------------------------------------------------------
void TLMaths::TTransform::AddTransform(const TLMaths::TTransform& Trans)
{
	if ( Trans.HasTranslate() )
	{
		if ( HasTranslate() )
			m_Translate += Trans.GetTranslate();
		else
			SetTranslate( Trans.GetTranslate() );
		
		TLDebug_CheckFloat( m_Translate );
	}
	
	if ( Trans.HasScale() )
	{
		if ( HasScale() )
			m_Scale *= Trans.m_Scale;
		else
			SetScale( Trans.m_Scale );
		
		TLDebug_CheckFloat( m_Scale );
	}
	
	if ( Trans.HasRotation() )
	{
		//	gr: needs normalising?
		if ( HasRotation() )
			m_Rotation *= Trans.m_Rotation;
		else
			SetRotation( Trans.m_Rotation );
		
		TLDebug_CheckFloat( m_Rotation );
	}
}


//-----------------------------------------------------------
//	Modify the transform values by another transform, Translates the translate, scales the scale, rotates the rotation. Doesn't multiply and rotate the translate etc. returns elements that have changed (slightly slower, but if your caller does much LESS work if nothing changed then use this)
//-----------------------------------------------------------
u8 TLMaths::TTransform::AddTransform_HasChanged(const TLMaths::TTransform& Trans)
{
	u8 ChangedBits = 0x0;
	
	if ( Trans.HasTranslate() )
	{
		if ( Trans.m_Translate.IsNonZero() )
		{
			if ( HasTranslate() )
				m_Translate += Trans.GetTranslate();
			else
				SetTranslate( Trans.GetTranslate() );
			
			ChangedBits |= TLMaths_TransformBitTranslate;
			
			TLDebug_CheckFloat( m_Translate );
		}
	}
	
	
	if ( Trans.HasScale() )
	{
		if ( HasScale() )
			m_Scale *= Trans.m_Scale;
		else
			SetScale( Trans.m_Scale );
		
		//	assume scale always changes... can check Trans.m_Scale.LengthSq() == 1 i think to see if its all 1's. Not sure how accurate that is...		
		ChangedBits |= TLMaths_TransformBitScale;
		
		TLDebug_CheckFloat( m_Scale );
	}
	
	if ( Trans.HasRotation() )
	{
		//	gr: needs normalising?
		if ( HasRotation() )
			m_Rotation *= Trans.m_Rotation;
		else
			SetRotation( Trans.m_Rotation );
		
		//	assume rotation always changes... 
		ChangedBits |= TLMaths_TransformBitScale;
		
		TLDebug_CheckFloat( m_Rotation );
	}
	
	return ChangedBits;
}


//-----------------------------------------------------------
//	transform vector
//-----------------------------------------------------------
void TLMaths::TTransform::Transform(float3& Vector) const
{
	TLDebug_CheckFloat( Vector );
	
	//	if we're transforming a vector that is 0,0,0 then scale and rotation will do nothing
	if ( Vector.IsNonZero() )
	{
		if ( HasScale() )
		{
			Vector *= GetScale();
			TLDebug_CheckFloat( Vector );
		}
		
		if ( HasRotation() )
		{
			GetRotation().RotateVector( Vector );
			TLDebug_CheckFloat( Vector );
		}
	}
	
	if ( HasTranslate() )
	{
		Vector += GetTranslate();
		TLDebug_CheckFloat( Vector );
	}
}

//-----------------------------------------------------------
//	transform vector
//-----------------------------------------------------------
void TLMaths::TTransform::Transform(float2& Vector) const
{
	TLDebug_CheckFloat( Vector );
	
	//	if we're transforming a vector that is 0,0,0 then scale and rotation will do nothing
	if ( Vector.IsNonZero() )
	{
		if ( HasScale() )
		{
			Vector *= GetScale();
			TLDebug_CheckFloat( Vector );
		}
		
		if ( HasRotation() )
		{
			GetRotation().RotateVector( Vector );
			TLDebug_CheckFloat( Vector );
		}
	}
	
	if ( HasTranslate() )
	{
		Vector += GetTranslate();
		TLDebug_CheckFloat( Vector );
	}
}

//-----------------------------------------------------------
//	untransform vector
//-----------------------------------------------------------
void TLMaths::TTransform::Untransform(float3& Vector) const
{
	TLDebug_CheckFloat( Vector );
	
	if ( HasTranslate() )
	{
		Vector -= GetTranslate();
		TLDebug_CheckFloat( Vector );
	}
	
	if ( HasRotation() )
	{
		GetRotation().UnrotateVector( Vector );
		TLDebug_CheckFloat( Vector );
	}
	
	if ( HasScale() )
	{
		Vector /= GetScale();
		TLDebug_CheckFloat( Vector );
	}
}


//-----------------------------------------------------------
//	untransform vector
//-----------------------------------------------------------
void TLMaths::TTransform::Untransform(float2& Vector) const
{
	TLDebug_CheckFloat( Vector );
	
	if ( HasTranslate() )
	{
		Vector -= GetTranslate().xy();
		TLDebug_CheckFloat( Vector );
	}
	
	if ( HasRotation() )
	{
		GetRotation().UnrotateVector( Vector );
		TLDebug_CheckFloat( Vector );
	}
	
	if ( HasScale() )
	{
		Vector /= GetScale().xy();
		TLDebug_CheckFloat( Vector );
	}
}

//-----------------------------------------------------------
//	make an "untransform" from this transform. (inverts rotation, scale, trans)
//-----------------------------------------------------------
void TLMaths::TTransform::Invert()
{
	//	negate translate
	if ( HasTranslate() )
	{
		m_Translate.x = -m_Translate.x;
		m_Translate.y = -m_Translate.y;
		m_Translate.z = -m_Translate.z;
	}
	
	//	invert rotation
	if ( HasRotation() )
	{
		m_Rotation.Invert();
	}
	
	//	get reciprocal of scale
	if ( HasScale() )
	{
		m_Scale.x = 1.f/m_Scale.x;
		m_Scale.y = 1.f/m_Scale.y;
		m_Scale.z = 1.f/m_Scale.z;
	}
}

//-----------------------------------------------------------
//	assert handler
//-----------------------------------------------------------
void TLMaths::TTransform::Debug_Assert(const char* pString) const
{
	TLDebug_Break( pString );
}


//-----------------------------------------------------------
//	see if transforms are same
//-----------------------------------------------------------
Bool TLMaths::TTransform::operator==(const TLMaths::TTransform& Transform) const
{
	//	quick check to see if valid elements are different or not...
	//	technically we could have scale=1 or transform=0 and they'd have the same EFFECT, but validity is different..
	if ( GetHasTransformBits() != Transform.GetHasTransformBits() )
		return FALSE;
	
	if ( HasTranslate() && GetTranslate() != Transform.GetTranslate() )
		return FALSE;
	
	if ( HasScale() && GetScale() != Transform.GetScale() )
		return FALSE;
	
	if ( HasRotation() && GetRotation() != Transform.GetRotation() )
		return FALSE;
	
	return TRUE;
}


//-----------------------------------------------------------
//	see if transforms are different 
//-----------------------------------------------------------
Bool TLMaths::TTransform::operator!=(const TLMaths::TTransform& Transform) const
{
	//	quick check to see if valid elements are different or not...
	//	technically we could have scale=1 or transform=0 and they'd have the same EFFECT, but validity is different..
	if ( GetHasTransformBits() == Transform.GetHasTransformBits() )
		return FALSE;
	
	if ( HasTranslate() && GetTranslate() == Transform.GetTranslate() )
		return FALSE;
	
	if ( HasScale() && GetScale() == Transform.GetScale() )
		return FALSE;
	
	if ( HasRotation() && GetRotation() == Transform.GetRotation() )
		return FALSE;
	
	return TRUE;
}
