/*------------------------------------------------------

	Render node that renders a PathNetwork as lines.

	Subscribes to the asset for changes to update the mesh 
	rather than rebuilding all the time

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"
#include "TRenderNodeDebugMesh.h"
#include <TootleCore/TSubscriber.h>


namespace TLRender
{
	class TRenderNodePathNetwork;
}

namespace TLAsset
{
	class TPathNetwork;
}

namespace TLPath
{
	class TPathNode;
}


//----------------------------------------------------
//	
//----------------------------------------------------
class TLRender::TRenderNodePathNetwork : public TLRender::TRenderNodeDebugMesh
{
public:
	TRenderNodePathNetwork(TRefRef RenderNodeRef,TRefRef TypeRef);

	virtual void			Initialise(TLMessaging::TMessage& Message);
	virtual void			Shutdown()									{	SetPathNetwork( TRef() );	}

	void					SetPathNetwork(TRefRef PathNetworkRef);		//	change the path network asset

protected:
	virtual void			ProcessMessage(TLMessaging::TMessage& Message);
	
	void					InitMeshFromPathNetwork(TLAsset::TPathNetwork& PathNetwork);				//	create the debug mesh for the asset
	s32						GetPathNodeVertex(TLAsset::TPathNetwork& PathNetwork,TLPath::TPathNode& PathNode);		//	return vertex for this path node - if it doesn't exist, create it

public:
	TColour					m_PathColour;		//	colour of vertexes for the lines
	TRef					m_PathNetworkRef;	//	asset ref of path network
	TKeyArray<TRef,u16>		m_PathNodeVertex;	//	there's a vertex for each path node, this keeps track of it
};


