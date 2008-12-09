
#pragma once

#include "TScenegraph.h"


namespace TLScene
{
	class TSchemeEditor;

	extern TPtr<TSchemeEditor> g_pSchemeEditor;
}


class TLScene::TSchemeEditor : public TManager
{
	// Internal transform tool type
	typedef enum TransformMode
	{
		Translate = 0,
		Rotate,
		Scale,
	};

	typedef enum TransformAxis
	{
		AXIS_Y = 0,
		AXIS_X,
		AXIS_Z,
	};

public:
	TSchemeEditor(TRef refManagerID);


	// Enabled flag
	FORCEINLINE void			SetEnabled(Bool bEnabled)		{ m_bEnabled = bEnabled; }
	FORCEINLINE void			ToggleEnabled()					{ m_bEnabled = (m_bEnabled ? 0 : 1); }
	FORCEINLINE Bool			IsEnabled()				const	{ return m_bEnabled; }

protected:

	SyncBool		Initialise();
	SyncBool		Shutdown();

	virtual void	ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage);

	virtual void	OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

	
private:

	void			PickObjects(Type2<s32>, TRef refRenderTargetID);
	void			AddRemoveObjects(Type2<s32>, TRef refRenderTargetID);

	void			CreateNodeTransformTools();
	void			RemoveNodeTransformTools();

	void			TransformSelectedNodes(TPtr<TLMessaging::TMessage>& pMessage, TransformAxis uAxis);
	void			TransformSelectedNodes(float fAmount, TransformAxis uAxis);

	void			TranslateSelectedNodes(float fAmount, TransformAxis uAxis);
	void			RotateSelectedNodes(float fAmount, TransformAxis uAxis);
	void			ScaleSelectedNodes(float fAmount, TransformAxis uAxis);

	void			DeleteSelectedNodes();

private:
	TPtrArray<TLScene::TSceneNode>	m_pSelectedNodes;

	TransformMode	m_uTransformMode;

	Bool			m_bEnabled;			// Enabled - on/off setting
};


