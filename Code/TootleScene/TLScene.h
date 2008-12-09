#pragma once

#include "TScenegraph.h"


namespace TLScene
{
	class TSceneNode;

	TRef	CreateNode(TRefRef InstanceRef,TRefRef TypeRef,TPtr<TLScene::TSceneNode> pParentNode=NULL,TPtr<TLMessaging::TMessage> pInitMessage=NULL);
}

