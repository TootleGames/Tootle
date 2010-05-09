#pragma once
#include "TSceneNode_Object.h"
#include <TootleGame/TTimeline.h>


namespace TLScene
{
	class TSceneNode_Timeline;
}

class TLScene::TSceneNode_Timeline : public TSceneNode_Object
{
public:
	TSceneNode_Timeline(TRefRef NodeRef,TRefRef TypeRef) :
		TSceneNode_Object(NodeRef, TypeRef),
		m_pTimelineInstance(NULL),
		m_bAutoDelete(FALSE)
	{
	}

	virtual ~TSceneNode_Timeline()
	{
		// Safeguard destructor removal of the timeline instance
		DeleteTimelineInstance();
	}

protected:
	virtual void			Initialise(TLMessaging::TMessage& Message);	
	virtual void 			Update(float Timestep);					
	virtual void			Shutdown();							

	virtual void			SetProperty(TLMessaging::TMessage& Message);	
	virtual void			UpdateNodeData();
	virtual void			ProcessMessage(TLMessaging::TMessage& Message);

	virtual void			OnRenderNodeAdded(TPtr<TLRender::TRenderNode>& pRenderNode);

	TLAnimation::TTimelineInstance*	GetTimelineInstance()	{	return m_pTimelineInstance;	}
	virtual void			OnTimelineComplete(TLAnimation::TTimelineInstance& Timeline);	//	timeline has finished (this won't occur if the timeline loops)
	virtual void			OnTimelineJump()			{}									//	timeline has jumped, probably looped

private:
	void				CreateTimelineInstance();
	void				DeleteTimelineInstance();

private:
	TRef								m_TimelineAssetRef;
	TLAnimation::TTimelineInstance*		m_pTimelineInstance;

	Bool								m_bAutoDelete;
};