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

#include "TSceneNode.h"

namespace TLScene
{
	class TSchemeNode;
};


class TLScene::TSchemeNode
{
public:
	
private:
	TRef	m_SchemeRef;		// The scheme ID
};