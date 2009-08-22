

#pragma once

namespace TLPhysics
{
	class TCollisionInfo;

	// Forward declaration
	class TPhysicsNode;
}

//------------------------------------------------------
//	collision info which is sent to subscribers - merge with intersection info?	
//------------------------------------------------------
class TLPhysics::TCollisionInfo
{
public:
	TCollisionInfo() : m_OtherNodeStatic ( FALSE ), m_IsNewCollision ( TRUE )	{}

	void			Set(const TLPhysics::TPhysicsNode& OtherNode,const TLMaths::TIntersection& Intersection);
	void			SetIsNewCollision(Bool NewCollision)	{	m_IsNewCollision = NewCollision;	}
	void			SetIsEndOfCollision(TRefRef ShapeRef,const TLPhysics::TPhysicsNode& OtherNode,TRefRef OtherShapeRef);	//	set up end-of-collision with this node

	void			ExportData(TBinaryTree& Data);		//	export this collision info into a BinaryData
	Bool			ImportData(TBinaryTree& Data);		//	get collision info from a BinaryData

	Bool			IsEndOfCollision() const			{	return !m_IsNewCollision;	}
	Bool			HasNormal() const					{	return m_IntersectionNormal.IsNonZero();	}
	const float2&	GetIntersectionNormal() const		{	return m_IntersectionNormal;	}		//	gr: USE THIS FUNCTION as normal computation may change in future

public:
	Bool		m_IsNewCollision;		//	is a new collision, if false then it's notification of an end-of-collision/contact
	TRef		m_OtherNode;			//	ref of other physics node
	TRef		m_OtherNodeOwner;		//	ref of other physics node's owner (ie. what scene node we collided with)
	Bool		m_OtherNodeStatic;		//	other node is static
	float3		m_Intersection;			//	collision point in world space on node that this has come from
	float3		m_OtherIntersection;	//	collision point in world space on other object
	float2		m_IntersectionNormal;	//	this is the direction from this node to the othernode. The direction from intersection to other intersection might be better...
	TRef		m_Shape;				//	ref of the shape that collided on this node
	TRef		m_OtherShape;			//	ref of the shape on the other node we collided with
};

TLCore_DeclareIsDataType( TLPhysics::TCollisionInfo );
