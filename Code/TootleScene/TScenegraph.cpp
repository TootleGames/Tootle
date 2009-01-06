
#include "TScenegraph.h"

#include "TSceneNode_Transform.h"


////////////////////////////////////////////////////
// include files for the node factory
////////////////////////////////////////////////////
#include "TSceneNode_Camera.h"
#include "TSceneNode_Emitter.h"
////////////////////////////////////////////////////


namespace TLScene
{
	TPtr<TScenegraph> g_pScenegraph = NULL;
};

using namespace TLScene;



TSceneNode* TSceneNodeFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	// Create engine/middleware side scene nodes
	if(TypeRef == "Camera")
		return new TSceneNode_Camera(InstanceRef,TypeRef);

	if(TypeRef == "Emitter")
		return new TSceneNode_Emitter(InstanceRef,TypeRef);

	return NULL;
}


SyncBool TScenegraph::Initialise()
{

	// Attach the base scene node factory by default
	TPtr<TClassFactory<TSceneNode,FALSE> > pFactory = new TSceneNodeFactory();

	if(pFactory)
		AddFactory(pFactory);

	return TLGraph::TGraph<TSceneNode>::Initialise();
}



Bool TScenegraph::GetNearestNodes(const TLMaths::TLine& Line, const float& fDistance, TPtrArray<TSceneNode_Transform>& pArray)
{
	TPtr<TSceneNode> pRootNode = GetRootNode();

	if(pRootNode.IsValid())
		GetNearestNodes(pRootNode, Line, fDistance, pArray);

	// Return TRUE if we found any nodes ith range of the line
	return (pArray.GetSize() > 0);
}

void TScenegraph::GetNearestNodes(TPtr<TSceneNode>& pNode, const TLMaths::TLine& Line, const float& fDistance, TPtrArray<TSceneNode_Transform>& pArray)
{
	// Check the node itself - if within range add to the array
	if(IsNodeWithinRange(pNode, Line, fDistance))
		pArray.Add(pNode);

#ifdef TLGRAPH_OWN_CHILDREN

	TPtrArray<TSceneNode>& NodeChildren = pNode->GetChildren();
	for ( u32 c=0;	c<NodeChildren.GetSize();	c++ )
	{
		TPtr<TSceneNode>& pChild = NodeChildren[c];
		GetNearestNodes( pChild, Line, fDistance, pArray );
	}

#else

	// Check the nodes children - the child will propagate the check to all other children too
	TPtr<TSceneNode> pNextNode = pNode->GetChildFirst();
	if(pNextNode.IsValid())
		GetNearestNodes(pNextNode, Line, fDistance, pArray);

	// Check the nodes siblings - the next one will check the next sibling and so on...
	pNextNode = pNode->GetNext();
	if(pNextNode.IsValid())
		GetNearestNodes(pNextNode, Line, fDistance, pArray);

#endif
}

Bool TScenegraph::IsNodeWithinRange(TPtr<TSceneNode>& pNode, const TLMaths::TLine& Line, const float& fDistance)
{
	if(pNode->HasTransform())
	{
		// If the node has transform then we can assume it is the transform node type
		// Cast to a transform node
		TSceneNode_Transform* pTransformNode = reinterpret_cast<TSceneNode_Transform*>(pNode.GetObject());

		// Do distance check from node to line
		float fDistanceToLine = pTransformNode->GetDistanceTo(Line);

		// Is the distance to the line less than the threshold?
		if(fDistanceToLine < fDistance)
			return TRUE;
	}

	return FALSE;
}

TPtr<TSceneNode> TScenegraph::CreateInstance(TRefRef NodeRef,TRefRef TypeRef)
{
	TRef NewNodeRef = CreateNode( NodeRef, TypeRef, "Root");

	return FindNode( NewNodeRef );
}





