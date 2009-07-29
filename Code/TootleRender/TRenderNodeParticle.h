/*------------------------------------------------------

	Render node that renders a particle system;

	Particle system is implemented as an embedded mesh on
	the render node that renders point-sprites.

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"
#include "TRenderNodeDebugMesh.h"



namespace TLRender
{
	class TRenderNodeParticle;
}


//----------------------------------------------------
//	
//----------------------------------------------------
class TLRender::TRenderNodeParticle : public TLRender::TRenderNodeDebugMesh
{
public:
	TRenderNodeParticle(TRefRef NodeRef,TRefRef TypeRef);

protected:
	virtual void			Initialise(TLMessaging::TMessage& Message);
	virtual void			SetProperty(TLMessaging::TMessage& Message);
	virtual void			ProcessMessage(TLMessaging::TMessage& Message);
	
	void					Update(float Timestep);
	void					SpawnParticles(s32 Count,TLAsset::TMesh& Mesh,TArray<u16>* pReuseParticles);		//	spawn N particles

	TLMaths::TShape*		GetRandomEmitShape();					//	pick a random emitter shape
	u16						GetParticleCount()						{	TLAsset::TMesh* pMesh = GetMeshAsset();	return pMesh ? GetParticleCount( *pMesh ) : 0;	}
	u16						GetParticleCount(TLAsset::TMesh& Mesh)	{	return Mesh.GetVertexCount();	}

protected:
	float		m_SizeMin;			//	min size of a particle in world space
	float		m_SizeMax;			//	max size of a particle in world space
	float		m_EmitRate;			//	emit X particles every second
	u32			m_MaxParticleCount;	//	maximum number of particles
	TRef		m_ParticleAsset;	//	particle system asset

	float		m_EmitCountdown;	//	countdown until another particle is released
	u32			m_EmitQueue;		//	number of particles we explicitly want to spawn on next (probably first) update

	TPtrArray<TLMaths::TShape>	m_EmitShapes;		//	emission shapes localised to this render node
	TPtrArray<TLMaths::TShape>	m_ExcludeShapes;	//	when we emit particles make sure they're outside these shapes. Die when in here?
};


