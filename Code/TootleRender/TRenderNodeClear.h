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

	virtual TPtr<TLAsset::TMesh>&	GetMeshAsset() 											{	return m_pClearMesh;	}
	virtual Bool					Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)	{	return TRUE;	}

	void							SetSize(const TLMaths::TBox2D& ClearBox,float NearZ);	//	resize the mesh (also creates as required)

protected:
	virtual void					InitMesh(TLAsset::TMesh& ClearMesh);

public:
	TPtr<TLAsset::TMesh>			m_pClearMesh;		//	just a quad
};