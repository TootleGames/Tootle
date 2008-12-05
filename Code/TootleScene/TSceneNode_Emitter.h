
#pragma once

#include "TSceneNode_Object.h"

namespace TLScene
{
	class TSceneNode_Emitter;
}


class TLScene::TSceneNode_Emitter : public TLScene::TSceneNode_Object
{
public:
	TSceneNode_Emitter(TRef refNodeID,TRefRef TypeRef);

protected:
	virtual void				ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

	virtual void 				DoUpdate(float fTimestep);

private:
	Bool						EmitObject();

	float3						GetEmissionPosition();

private:

	TRef		m_refNodeIDBase;			// Base ID to use when emitting objects		
	TRef		m_refNodeIDCurrent;			// Current ID to use for the next instance		
	TRef		m_refNodeTypeToEmit;		// Type of object to emit

	TRef		m_refMeshToEmitFrom;		// Reference to a mesh to emit the nodes from

	float		m_fEmissionRate;			// Amount of objects to emit per second
	float		m_fEmissionTime;			// Internal emission timer
};