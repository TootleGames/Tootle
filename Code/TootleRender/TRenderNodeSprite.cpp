#include "TRenderNodeSprite.h"




TLRender::TRenderNodeSprite::TRenderNodeSprite(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNode	( RenderNodeRef, TypeRef )
{
}

	
//---------------------------------------------------------
//
//---------------------------------------------------------
bool TLRender::TRenderNodeSprite::CreateEffect()
{
	if ( m_pEffect )
		return true;

	//	add sprite effect
	TBinaryTree EffectData( TRef_Invalid );
	EffectData.ExportData("Type", TRef("Sprite") );
	m_pEffect = AddEffect( EffectData );

	return m_pEffect;
}

//---------------------------------------------------------
//	init
//---------------------------------------------------------
void TLRender::TRenderNodeSprite::Initialise(TLMessaging::TMessage& Message)
{
	//	if the mesh hasn't been set, default to a d_quad
	if ( !m_MeshRef.IsValid() )
	{
		SetMeshRef( TRef_Static(d,UNDERSCORE,q,u,a) );
	}

	TRenderNode::Initialise( Message );
}


//---------------------------------------------------------
//	set properties
//---------------------------------------------------------
void TLRender::TRenderNodeSprite::SetProperty(TLMessaging::TMessage& Message)
{
	TRef Atlas;
	if ( Message.ImportData("Atlas", Atlas ) )
	{
		if ( CreateEffect() )
			m_pEffect->SetAtlas( Atlas );
	}

	u16 Glyph;
	if ( Message.ImportData("Glyph", Glyph ) )
	{
		if ( CreateEffect() )
			m_pEffect->SetGlyph( Glyph );
	}

	TLRender::TRenderNode::SetProperty(Message);
}


const TLRaster::TRasterData* TLRender::TRenderNodeSprite::Render(TArray<TLRaster::TRasterData>& MeshRasterData,TArray<TLRaster::TRasterSpriteData>& SpriteRasterData,const TColour& SceneColour)
{
	TLRaster::TRasterSpriteData& Sprite = *SpriteRasterData.AddNew();
	Sprite.Init();
	
	Sprite.m_Material.m_Texture = GetTextureRef();
	Sprite.m_Material.m_Colour = GetColour();
	
	Sprite.m_Sprite.Transform( GetWorldTransform() );
	
	return NULL;
}

