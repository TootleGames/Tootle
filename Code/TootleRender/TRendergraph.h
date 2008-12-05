
#pragma once

#include <TootleCore/TLGraph.h>
#include "TRenderNode.h"



namespace TLRender
{
	namespace Platform
	{
		extern SyncBool Init();
		extern SyncBool Update();
		extern SyncBool Shutdown();
	}


	class TRendergraph;

	extern TPtr<TRendergraph> g_pRendergraph;
};

/*
	TRendergraph class
*/
class TLRender::TRendergraph : public TLGraph::TGraph<TLRender::TRenderNode>
{
public:
	TRendergraph(TRefRef refManagerID) :
		TLGraph::TGraph<TLRender::TRenderNode>	(refManagerID)
	{
	}
protected:

	virtual SyncBool Initialise();
	virtual SyncBool Update(float /*fTimeStep*/);
	virtual SyncBool Shutdown();
};