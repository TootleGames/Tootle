/*------------------------------------------------------
	A render target is something we render to! 
	
	We can have multiple render targets in a screen. These
	appear as "windows" in the screen.

	A render target can be an offscreen render target too. But 
	still needs to be owned by a screen

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TColour.h>
#include <TootleCore/TRef.h>
#include <TootleCore/TFlags.h>
#include "TLRender.h"
#include "TCamera.h"
#include "TRenderNode.h"
#include "TRenderNodeClear.h"


namespace TLRender
{
	class TRenderTarget;
	class TScreen;
	class TCamera;

	namespace Platform
	{
		class RenderTarget;
	}
}



//---------------------------------------------------------
//	base render target type
//---------------------------------------------------------
class TLRender::TRenderTarget
{
public:
	enum Flags
	{
		Flag_ClearColour	= 0,
		Flag_ClearDepth,
		Flag_ClearStencil,	//	
		Flag_Enabled,		//	enable rendering
		Flag_AntiAlias,		//	enable antialiasing if the screen supports it
	};

public:
	TRenderTarget(TRefRef Ref=TRef());
	~TRenderTarget()							{	}

	virtual Bool			BeginDraw(const Type4<s32>& MaxSize);
	virtual void			Draw();	
	virtual void			EndDraw();

	const TRef&				GetRef() const								{	return m_Ref;	}
	inline Bool				operator==(const TRef& Ref) const			{	return GetRef() == Ref;	}

	virtual Bool			SetSize(const Type4<s32>& Size)				{	m_Size = Size;	return TRUE;	}
	virtual void			GetSize(Type4<s32>& Size,const Type4<s32>& MaxSize) const;				//	get the render target's dimensions. we need the screen in case dimensions are max's
	Bool					GetOrthoSize(Type4<float>& OrthoSize,const Type4<s32>& MaxSize);	//	get the orthographic dimensions (0-100 on width). returns FALSE if not using an ortho camera

	void					SetCamera(TPtr<TCamera>& pCamera)			{	m_pCamera = pCamera;	}
	TPtr<TCamera>&			GetCamera()									{	return m_pCamera;	}
	TFlags<Flags>&			GetFlags()									{	return m_Flags;	}
	Bool					GetFlag(TRenderTarget::Flags Flag) const	{	return m_Flags(Flag);	}
	TColour&				GetClearColour()							{	return m_ClearColour;	}
	TPtr<TRenderNode>&		GetRootRenderNode()							{	return m_pRootRenderNode;	}

	TFlags<TRenderNode::RenderFlags::Flags>&	Debug_ForceRenderFlagsOn()	{	return m_Debug_ForceRenderFlagsOn;	}
	TFlags<TRenderNode::RenderFlags::Flags>&	Debug_ForceRenderFlagsOff()	{	return m_Debug_ForceRenderFlagsOff;	}

	//	generic scene rendering controls
	virtual void			BeginScene()										{	}					//	save off current scene (and optionally reset)
	virtual void			BeginSceneReset(Bool ApplyCamera=TRUE)				{	}					//	save off current scene (and optionally reset)
	virtual Bool			GetSceneMatrix(TLMaths::TMatrix& Matrix)			{	return FALSE;	}	//	save off the current scene's camera matrix
	virtual void			EndScene()											{	}					//	restore previous scene
	virtual void			Translate(const TLMaths::TTransform& Transform)		{	}					//	create a transform/rot matrix from the quaternion and apply
	virtual void			TranslateCamera();															//	apply camera transform to the scene
	virtual void			SetSceneColour(const TColour& SceneColour)			{	}					//	set current render colour (glColourf)

	//	clever stuff
	Bool					GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize) const;	//	get world pos from 2d point inside our rendertarget size


protected:
	Bool							DrawNode(TRenderNode* pRenderNode,TRenderNode* pParentRenderNode,const TLMaths::TTransform* pSceneTransform);	//	render a render object
	Bool							DrawMeshWrapper(TLAsset::TMesh* pMesh,TRenderNode* pRenderNode,const TLMaths::TTransform& SceneTransform,TPtrArray<TRenderNode>& PostRenderList);	
	virtual TLRender::DrawResult	DrawMesh(TLAsset::TMesh& Mesh,const TRenderNode* pRenderNode,const TFlags<TRenderNode::RenderFlags::Flags>* pForceFlags)			{	return TLRender::Draw_Error;	}
	template<class SHAPE>
	void							DrawMeshShape(const SHAPE& Shape,const TRenderNode* pRenderNode,const TFlags<TRenderNode::RenderFlags::Flags>& RenderFlags,Bool ResetScene);

	virtual Bool					GetViewportSize(Type4<s32>& ViewportSize,const Type4<s32>& MaxSize);	//	convert our relative size to the opengl viewport size (upside down) - returns FALSE if too small to be seen
	
	virtual Bool					BeginProjectDraw(const Type4<s32>& ViewportSize);	//	setup projection mode
	virtual void					EndProjectDraw();

	virtual Bool					BeginOrthoDraw(const Type4<s32>& ViewportSize);		//	setup ortho projection mode
	virtual void					EndOrthoDraw();

protected:
	TRef							m_Ref;					//	render target ref
	Type4<s32>						m_Size;					//	pos + w + h. negative numbers mean min/max's

	TPtr<TCamera>					m_pCamera;				//	camera 
	s32								m_Debug_SceneCount;		//	to check we do Begin and EndScene in sync
	TColour							m_ClearColour;			//	clear colour
	TFlags<Flags>					m_Flags;				//	render target flags
	TPtr<TLRender::TRenderNode>		m_pRootRenderNode;		//	root render object
	TPtr<TRenderNodeClear>			m_pRenderNodeClear;		//	clear-screen render object

	u32								m_Debug_PolyCount;
	u32								m_Debug_VertexCount;	
	
	TColour							m_SceneColour;			//	current scene colour
	
	TPtrArray<TRenderNode>			m_TempPostRenderList;	//	instead of having a local, we have a member list of post render nodes. performance saving

	TFlags<TLRender::TRenderNode::RenderFlags::Flags>	m_Debug_ForceRenderFlagsOn;	//	force render objects to render with these flags ON - like globally turning on/off debug rendering
	TFlags<TLRender::TRenderNode::RenderFlags::Flags>	m_Debug_ForceRenderFlagsOff;	//	force render objects to render with these flags OFF
};



template<class SHAPE>
void TLRender::TRenderTarget::DrawMeshShape(const SHAPE& Shape,const TLRender::TRenderNode* pRenderNode,const TFlags<TLRender::TRenderNode::RenderFlags::Flags>& RenderFlags,Bool ResetScene)
{
	if ( !Shape.IsValid() )
		return;

	//	save off current render state
	if ( ResetScene )
		BeginSceneReset();
	else
		BeginScene();
		
	//	possible a little expensive... generate a mesh for the bounds...
	TLAsset::TMesh ShapeMesh("Bounds");

	ShapeMesh.GenerateShape( Shape );

	//	then render our temporary mesh
	DrawMesh( ShapeMesh, pRenderNode, &RenderFlags );

	EndScene();
}
