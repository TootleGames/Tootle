#include "TEffect.h"
#include <TootleCore/TCoreManager.h>
#include <TootleCore/TLCore.h>
#include <TootleAsset/TAtlas.h>


//	globals
TPtr<TLRender::TEffectFactory> TLRender::g_pEffectFactory;


//-------------------------------------------------------------
//	create an effect
//-------------------------------------------------------------
TLRender::TEffect* TLRender::TEffectFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	if ( TypeRef == "TxAnim" )
		return new TEffect_TextureAnimate();

	return NULL;
}


//---------------------------------------------------------------
//	
//---------------------------------------------------------------
TLRender::TEffect::TEffect(const TArray<TRef>& ShaderCandidates)
{
	//	save shader candidates
	m_ShaderCandidates.Copy( ShaderCandidates );

	//	load most prefered shader
	for ( u32 s=0;	s<m_ShaderCandidates.GetSize() && !m_pShader;	s++ )
	{
		m_pShader = TLAsset::GetAssetPtr<TLAsset::TShader>( m_ShaderCandidates[s], SyncTrue );
	}

	//	warning
	if ( !m_pShader )
	{
		TLDebug_Break("Warning: didn't find any shaders for this effect");
	}
}


//---------------------------------------------------------------
//	set our data pointer and initialise vars from that data
//---------------------------------------------------------------
bool TLRender::TEffect::Initialise(TPtr<TBinaryTree>& pData)
{
	m_pShaderData = pData;
	if ( !m_pShaderData )
	{
		TLDebug_Break("Shader data expected");
		return false;
	}

	return true;
}

//---------------------------------------------------------------
//	pre-render this shader. return false to abort render (eg. if the shader makes the object non-visible)
//---------------------------------------------------------------
Bool TLRender::TEffect::PreRender()
{
	if ( !IsValid() )
		return true;

	//	pre-render the shader
	return m_pShader->PreRender( GetShaderData() );
}

//---------------------------------------------------------------
//	post-render of the shader
//---------------------------------------------------------------
void TLRender::TEffect::PostRender()
{
	if ( !IsValid() )
		return;

	m_pShader->PostRender();
}



TLRender::TEffect_TextureAnimate::TEffect_TextureAnimate() :
	TLRender::TEffect	( TFixedArray<TRef,3>() << "FFTxM" ),
	m_FrameRate			( 0.f ),
	m_Time				( 0.f )
{
}


bool TLRender::TEffect_TextureAnimate::Initialise(TPtr<TBinaryTree>& pData)
{
	//	do super initialise
	if ( !TEffect::Initialise( pData ) )
		return false;

	//	read atlas asset
	TRef AtlasRef;
	if ( !pData->ImportData("Atlas", AtlasRef ) )
	{
		TLDebug_Break("Atlas ref expected");
		return false;
	}

	//	read-in any params
	u32 InitialFrame = 0;
	pData->ImportData("FrInit", InitialFrame );
	pData->ImportData("FrRate", m_FrameRate );
	pData->ImportArrays("Frames", m_FrameIndexes );

	float InitialTime = 0.f;
	pData->ImportData("Time", InitialTime );

	//	add initial frame onto current time
	InitialTime += m_FrameRate * (float)InitialFrame;

	//	init frame data
	TLAsset::TAtlas* pAtlas = TLAsset::GetAsset<TLAsset::TAtlas>( AtlasRef );
	if ( !pAtlas )
	{
		TLDebug_Break("Failed to find atlas asset");
		return false;
	}
	OnAtlasChanged( *pAtlas );

	//	set initial state
	if ( !Update( InitialTime ) )
	{
		//	update didn't change so initialise frame
		SetFrame( InitialFrame );
	}

	//	get update messages
	this->SubscribeTo( TLCore::g_pCoreManager );

	return true;
}


//------------------------------------------------
//	update frame transforms
//------------------------------------------------
void TLRender::TEffect_TextureAnimate::OnAtlasChanged(const TLAsset::TAtlas& Atlas)
{
	//	clean out old frames
	m_Frames.Empty();

	//	fetch glyph array
	const TKeyArray<u16,TLAsset::TAtlasGlyph>& Glyphs = Atlas.GetGlyphs();

	//	if no frames then add a default
	if ( m_FrameIndexes.GetSize() == 0 )
	{
		TLMaths::TTransform* pTransform = m_Frames.AddNew();
		pTransform->SetTranslate( float3( 0.f, 0.f, 0.f ) );
		pTransform->SetScale( float3( 1.f, 1.f, 1.f ) );
	}

	for ( u32 f=0;	f<m_FrameIndexes.GetSize();	f++ )
	{
		u16 FrameKey = m_FrameIndexes[f];
		const TLAsset::TAtlasGlyph* pGlyph = Glyphs.Find( FrameKey );
		
		//	no such glyph for this key in this atlas
		if ( !pGlyph )
			continue;

		//	alloc transform
		TLMaths::TTransform* pTransform = m_Frames.AddNew();
		if ( !pTransform )
			continue;

		//	set transform
		pTransform->SetTranslate( pGlyph->GetTopLeft().xyz(0.f) );
		pTransform->SetScale( pGlyph->GetSize().xyz(1.f) );
	}
}


//------------------------------------------------
//	catch update messages
//------------------------------------------------
void TLRender::TEffect_TextureAnimate::ProcessMessage(TLMessaging::TMessage& Message)
{
	if ( Message.GetMessageRef() == TLCore::UpdateRef )
	{
		float Timestep = 0.f;
		Message.ResetReadPos();
		if ( Message.Read( Timestep ) )
		{
			Update( Timestep );
		}
	}
}


//------------------------------------------------
//	animate - returns true if frame changed
//------------------------------------------------
bool TLRender::TEffect_TextureAnimate::Update(float Timestep)
{
	s32 PreviousFrameIndex = TLMaths::FloatToS32( m_Time / m_FrameRate );

	//	increase time
	m_Time += Timestep;

	//	get frame index
	s32 FrameIndex = TLMaths::FloatToS32(m_Time / m_FrameRate);

	//	no change in frame
	if ( PreviousFrameIndex == FrameIndex )
		return false;

	//	store remainder
	float FrameRemainder = TLMaths::Modf( m_Time, m_FrameRate );

	//	if we've overflowed the frame rate loop around
	int FrameCount = GetFrameCount();
	if ( FrameCount == 0 )
		return false;

	bool Loop = false;
	while ( FrameIndex >= FrameCount )
	{
		FrameIndex -= FrameCount;
		Loop = true;
	}
	while ( FrameIndex <= -FrameCount )
	{
		FrameIndex += FrameCount;
		Loop = true;
	}

	//	reset time if we looped to help floating point inaccuracies
	if ( Loop )
	{
		m_Time = (m_FrameRate * TLMaths::S32ToFloat(FrameIndex) ) + FrameRemainder;
	}

	//	calc new frame in the texture atlas
	SetFrame( FrameIndex );

	return true;
}

