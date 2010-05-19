#include "TRenderNodeParticle.h"
#include <TootleAsset/TParticle.h>



TLRender::TRenderNodeParticle::TRenderNodeParticle(TRefRef NodeRef,TRefRef TypeRef) :
	TRenderNodeDebugMesh	( NodeRef, TypeRef ),
	m_SizeMin				( 0.f ),
	m_SizeMax				( 0.f ),
	m_EmitRate				( 0.f ),
	m_MaxParticleCount		( 0 ),
	m_EmitCountdown			( 0.f ),
	m_EmitQueue				( 0 )
{
}


//----------------------------------------------------
//	
//----------------------------------------------------
void TLRender::TRenderNodeParticle::Initialise(TLMessaging::TMessage& Message)
{
	//	setup specific flags
	GetRenderFlags().Set( TLRender::TRenderNode::RenderFlags::UsePointSprites );

	//	subscribe to updates
	//	gr: change this so the game or something dictates updates - not the core
	this->SubscribeTo( TLCore::g_pCoreManager );

	//	do inherited init
	TLRender::TRenderNodeDebugMesh::Initialise( Message );
}


//----------------------------------------------------
//	setup properties
//----------------------------------------------------
void TLRender::TRenderNodeParticle::SetProperty(TLMessaging::TMessage& Message)
{
	//	set new particle system asset
	if ( Message.ImportData("Particle", m_ParticleAsset ) )
	{
		//	load particle system
		TLAsset::TParticle* pParticleAsset = TLAsset::GetAsset<TLAsset::TParticle>( m_ParticleAsset );
		if ( pParticleAsset )
		{
			//	read properties out of particle system
			TLMessaging::TMessage DummyMessage;
			DummyMessage.ReferenceDataTree( pParticleAsset->GetData() );
			SetProperty( DummyMessage );
		}
	}

	//	set render node's texture
	TRef TextureRef;
	if ( Message.ImportData("Texture", TextureRef ) )
		SetTextureRef( TextureRef );

	Bool SizeChanged = Message.ImportData("SzMin", m_SizeMin );
	SizeChanged |= Message.ImportData("SzMax", m_SizeMax );

	//	gr: currently we only have 1 size per render node, not implemented random size per point sprite yet
	if ( SizeChanged )
	{
		float Size = TLMaths::Interp( m_SizeMin, m_SizeMax, 0.5f );
		SetPointSpriteSize( Size );
	}

	Message.ImportData("EmitRate", m_EmitRate );
	Message.ImportData("CntMax", m_MaxParticleCount );

	//	if there is an initial particle count, we set a value in the emit queue
	//	so on next update will with have at least the inital emit count
	u32 InitialCount = 0;
	if ( Message.ImportData("CntInit", InitialCount ) )
	{
		s32 InitialEmit = InitialCount - GetParticleCount();
		m_EmitQueue = (InitialEmit < 0) ? 0 : InitialEmit;
	}

	//	import emit shapes
	TPtrArray<TBinaryTree> ShapeDatas;
	if ( Message.GetChildren("EmitShape", ShapeDatas) )
	{
		for ( u32 s=0;	s<ShapeDatas.GetSize();	s++ )
		{
			TPtr<TLMaths::TShape> pShape = TLMaths::ImportShapeData( ShapeDatas[s] );
			if ( pShape )
				m_EmitShapes.Add( pShape );
		}
	}

	//	import exlusion shapes
	ShapeDatas.Empty();
	if ( Message.GetChildren("ExcShape", ShapeDatas) )
	{
		for ( u32 s=0;	s<ShapeDatas.GetSize();	s++ )
		{
			TPtr<TLMaths::TShape> pShape = TLMaths::ImportShapeData( ShapeDatas[s] );
			if ( pShape )
				m_ExcludeShapes.Add( pShape );
		}
	}

	//	do inherited init
	TLRender::TRenderNodeDebugMesh::SetProperty( Message );
}


//----------------------------------------------------
//	
//----------------------------------------------------
void TLRender::TRenderNodeParticle::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	update from core
	if ( Message.GetMessageRef() == TLCore::UpdateRef )
	{
		float Timestep = 0.f;
		Message.ResetReadPos();
		if ( Message.Read( Timestep ) )
		{
			Update( Timestep );
		}
	}

	TLRender::TRenderNodeDebugMesh::ProcessMessage( Message );
}


//----------------------------------------------------
//	
//----------------------------------------------------
void TLRender::TRenderNodeParticle::Update(float Timestep)
{
	//	get mesh
	TLAsset::TMesh* pMesh = GetMeshAsset();
	if ( !pMesh )
		return;

	//	keep a list of dead particles we can re-use before removing
	TFixedArray<u16,100> DeadParticles;
	
	u16 ParticleCount = GetParticleCount( *pMesh );

	//	update existing particles
	for ( u16 i=0;	i<ParticleCount;	i++ )
	{
		//	decrease life
		float Life = 10000.f;
		Life -= Timestep;
		if ( Life < TLMaths_NearZero )
		{
			DeadParticles.Add( i );
			continue;
		}

		//	update colour
		//	update scale
		//	update movement
		//	die if in exclusion shape
	}

	//	emit new particles - check emit rate is valid otherwise we might get stuck
	if ( m_EmitRate > TLMaths_NearZero )
	{
		m_EmitCountdown -= Timestep;

		//	keep re-increasing the countdown with a new particle until countdown is ready again 
		while ( m_EmitCountdown < 0.f )
		{
			m_EmitCountdown += m_EmitRate;
			m_EmitQueue++;
		}
	}

	//	have some particles to spawn!
	if ( m_EmitQueue > 0 )
	{
		SpawnParticles( m_EmitQueue, *pMesh, &DeadParticles );
		m_EmitQueue = 0;
	}

	//	remove dead particles
	while ( DeadParticles.GetSize() > 0 )
	{
		pMesh->RemoveVertex( DeadParticles.ElementLast(), FALSE );
		DeadParticles.RemoveLast();
	}
}


//----------------------------------------------------
//	spawn N particles
//----------------------------------------------------
void TLRender::TRenderNodeParticle::SpawnParticles(s32 Count,TLAsset::TMesh& Mesh,TArray<u16>* pReuseParticles)
{
	//	limit the number of particles we can spawn
	s32 MaxToSpawn = m_MaxParticleCount - GetParticleCount();
	if ( Count > MaxToSpawn )
		Count = MaxToSpawn;
	if ( Count <= 0 )
		return;

	//	create particles
	for ( s32 i=0;	i<Count;	i++ )
	{
		//	pick random emitter shape
		TLMaths::TShape* pShape = GetRandomEmitShape();
		if ( !pShape )
		{
			TLDebug_Warning("Failed to get emission shape - none set?");
			break;
		}

		//	init pos
		float3 SpawnPos = pShape->GetRandomPosition();

		//	todo:
		//	init size
		//float Size = TLMaths::Randf( m_MinSize, m_MaxSize );
		//	init life
		//	init colour

		//	re-use particle if possible
		if ( pReuseParticles && pReuseParticles->GetSize() > 0 )
		{
			u16 VertexIndex = pReuseParticles->ElementLast();
			Mesh.GetVertex(VertexIndex) = SpawnPos;

			//	remove particle we used from list
			pReuseParticles->RemoveLast();
			if ( pReuseParticles->GetSize() == 0 )
				pReuseParticles = NULL;
		}
		else
		{
			//	add new vertex
			Mesh.AddVertex( SpawnPos );
		}
	}
}


//----------------------------------------------------
//	pick a random emitter shape
//----------------------------------------------------
TLMaths::TShape* TLRender::TRenderNodeParticle::GetRandomEmitShape()
{
	//	gr: todo: weight this towards larger shapes so particles are evenly distributed
	return m_EmitShapes.GetRandomElement();
}

