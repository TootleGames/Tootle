/*------------------------------------------------------

	Render node that renders a quad tree

	currently just a dumb single mesh, but could expand into a [render]tree (to match the quadtree) in future

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"
#include "TRenderNodeDebugMesh.h"
#include <TootleMaths/TQuadTree.h>



namespace TLRender
{
	class TRenderNodeQuadTreeZone;
};




//----------------------------------------------------
//	
//----------------------------------------------------
class TLRender::TRenderNodeQuadTreeZone : public TLRender::TRenderNodeDebugMesh
{
public:
	TRenderNodeQuadTreeZone(TRefRef RenderNodeRef,TRefRef TypeRef);

	virtual void			Initialise(TLMessaging::TMessage& Message);
	virtual void			Shutdown()									{	SetQuadTreeZone( TLPtr::GetNullPtr<TLMaths::TQuadTreeZone>() );	}

	void					SetQuadTreeZone(TPtr<TLMaths::TQuadTreeZone>& pZone);

protected:
	virtual void			ProcessMessage(TLMessaging::TMessage& Message);	//	catch changes to the zone we're tracking

public:
	TPtr<TLMaths::TQuadTreeZone>	m_pZone;
};

