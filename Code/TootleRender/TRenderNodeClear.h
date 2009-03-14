/*------------------------------------------------------

	Simple render object used to clear screen when hardware
	can't do what we want

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"

namespace TLRender
{
	class TRenderNodeClear;
}


class TLRender::TRenderNodeClear : public TLRender::TRenderNode
{
public:
	TRenderNodeClear(TRefRef NodeRef,TRefRef TypeRef=TRef());

	virtual TPtr<TLAsset::TMesh>&	GetMeshAsset() 		{	return m_pClearMesh;	}

	void							SetSize(const TLMaths::TBox2D& ClearBox,float NearZ);	//	resize the mesh (also creates as required)

public:
	TPtr<TLAsset::TMesh>			m_pClearMesh;		//	just a quad
};