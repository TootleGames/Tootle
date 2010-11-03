/*------------------------------------------------------

	Simple render node to render a sprite as efficiently as possible

-------------------------------------------------------*/
#pragma once

#include "TRenderNode.h"


namespace TLRender
{
	class TRenderNodeSprite;
}


//----------------------------------------------------
//	
//----------------------------------------------------
class TLRender::TRenderNodeSprite : public TLRender::TRenderNode
{
public:
	TRenderNodeSprite(TRefRef RenderNodeRef,TRefRef TypeRef);

protected:
	virtual void	Initialise(TLMessaging::TMessage& Message);
	virtual void	SetProperty(TLMessaging::TMessage& Message);

	virtual const TLRaster::TRasterData*	Render(TArray<TLRaster::TRasterData>& MeshRasterData,TArray<TLRaster::TRasterSpriteData>& SpriteRasterData,const TColour& SceneColour);	//	Render function. Make up RasterData's and add them to the list. Return a pointer to the main raster data created if applicable

private:
	bool			CreateEffect();

public:
	TPtr<TEffect_Sprite>	m_pEffect;
};


