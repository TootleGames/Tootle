#include "TLPhysics.h"
#include "TPhysicsNode.h"
#include <TootleMaths/TShape.h>




//------------------------------------------------------
//	set collision info against this node
//------------------------------------------------------
void TLPhysics::TCollisionInfo::Set(const TLPhysics::TPhysicsNode& OtherNode,const TLMaths::TIntersection& Intersection)
{
	m_OtherNode = OtherNode.GetNodeRef();
	m_OtherNodeOwner = OtherNode.GetOwnerSceneNodeRef();
	m_OtherNodeStatic = OtherNode.IsStatic();

	m_Intersection = Intersection.m_Intersection;
}



//------------------------------------------------------
//	export this collision info into a BinaryData
//------------------------------------------------------
void TLPhysics::TCollisionInfo::ExportData(TBinaryTree& Data)
{
	Data.Write( m_OtherNode );
	Data.Write( m_OtherNodeOwner );
	Data.Write( m_OtherNodeStatic );
	Data.Write( m_Intersection );
}


//------------------------------------------------------
//	get collision info from a BinaryData
//------------------------------------------------------
Bool TLPhysics::TCollisionInfo::ImportData(TBinaryTree& Data)
{
	Data.ResetReadPos();

	if ( !Data.Read( m_OtherNode ) )		return FALSE;
	if ( !Data.Read( m_OtherNodeOwner ) )	return FALSE;
	if ( !Data.Read( m_OtherNodeStatic ) )	return FALSE;
	if ( !Data.Read( m_Intersection ) )		return FALSE;

	return TRUE;
}

