/*
 *  TSchemeNode.h
 *  TootleScene
 *
 * The scheme node will be used to segeragate the scenegraph for different 'sections' of a 'map' and for identifying what primary schemes have been instanced.
 *
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include "TSceneNode_Transform.h"

namespace TLScene
{
	class TSceneNode_Scheme;
};


class TLScene::TSceneNode_Scheme : public TLScene::TSceneNode_Transform
{
public:
	TSceneNode_Scheme(TRefRef NodeRef,TRefRef TypeRef) :
	  TSceneNode_Transform(NodeRef,TypeRef)
	{
	}
};