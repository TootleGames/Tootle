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
	TRenderNodeClear(TRefRef NodeRef,TRefRef TypeRef);

	virtual void				GetMeshAsset(TPtr<TLAsset::TMesh>& pMesh) 	{	pMesh = m_pClearMesh;	}

	void						SetSize(const Type4<s32>& ClearSize,float NearZ);	//	resize the mesh (also creates as required)

public:
	TPtr<TLAsset::TMesh>		m_pClearMesh;		//	just a quad
};