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
	virtual void	SetProperty(TLMessaging::TMessage& Message);

private:
	bool			CreateEffect();

public:
	TPtr<TEffect_Sprite>	m_pEffect;
};


