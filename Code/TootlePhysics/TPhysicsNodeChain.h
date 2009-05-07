/*------------------------------------------------------

	Physics node with joints

-------------------------------------------------------*/
#pragma once

#include "TPhysicsNode.h"

/*
namespace TLPhysics
{
	class TPhysicsNodeChain;
};

//------------------------------------------------------
//	
//------------------------------------------------------
class TLPhysics::TPhysicsNodeChain : public TLPhysics::TPhysicsNode
{
public:
	TPhysicsNodeChain(TRefRef NodeRef,TRefRef TypeRef=TRef());

	FORCEINLINE Bool			operator==(TRefRef Ref) const			{	return GetNodeRef() == Ref;	}
	virtual Bool				HasMultipleShapes() const				{	return TRUE;	}

protected:
	virtual void				Initialise(TLMessaging::TMessage& Message);	

	//	box2d interface
	Bool						CreateBody(b2World& World);					//	create the body in the world
	virtual Bool				CreateBodyShape();							//	when our collision shape changes we recreate the shape on the body
	virtual void				GetBodys(TArray<b2Body*>& Bodies) const		{	if ( m_pBody )	Bodies.Add( m_pBody );	}

protected:
	TArray<b2Body*>				m_ChainBodies;				//	box2d bodies for each chain link
};


*/