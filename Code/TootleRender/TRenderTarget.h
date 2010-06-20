/*------------------------------------------------------
	A render target is something we render to! 
	
	We can have multiple render targets in a screen. These
	appear as "windows" in the screen.

	A render target can be an offscreen render target too. But 
	still needs to be owned by a screen

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TLDebug.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TColour.h>
#include <TootleCore/TRef.h>
#include <TootleCore/TFlags.h>
#include "TScreenShape.h"
#include "TCamera.h"
#include "TRenderNode.h"
#include "TRenderNodeClear.h"



//#define TLRENDER_DISABLE_CLEAR




namespace TLRender
{
	class TRenderTarget;
	class TScreen;
	class TCamera;
	class TRenderNode;

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

	virtual Bool			BeginDraw(const Type4<s32>& RenderTargetMaxSize,const Type4<s32>& ViewportMaxSize,const TScreen& Screen);
	virtual void			Draw();	
	virtual void			EndDraw();

	const TRef&				GetRef() const								{	return m_Ref;	}

	Bool					SetSize(const Type4<s32>& Size)				{	m_Size = Size;	OnSizeChanged();	return TRUE;	}
	void					GetSize(Type4<s32>& Size,const Type4<s32>& RenderTargetMaxSize) const;			//	get the render target's dimensions. we need the screen in case dimensions are max's
	void					OnScreenSizeChanged()						{	OnSizeChanged();	}

	void					SetCamera(TPtr<TCamera>& pCamera)			{	m_pCamera = pCamera;	OnSizeChanged();	}	//	gr: call OnSizeChanged to do camera some initialisation - specficcly for the ortho
	void					OnCameraChanged()							{	OnSizeChanged();	}	//	recalculate some camera stuff - only required when viewport, orthoscale etc change, not pos/lookat/roll/up etc
	TPtr<TCamera>&			GetCamera()									{	return m_pCamera;	}
	const TColour&			GetClearColour() const						{	return m_ClearColour;	}
	void					SetClearColour(const TColour& Colour);		//	set new clear colour
	TFlags<Flags>&			GetFlags()									{	return m_Flags;	}
	Bool					GetFlag(TRenderTarget::Flags Flag) const	{	return m_Flags(Flag);	}
	FORCEINLINE void		SetEnabled(Bool Enabled)					{	m_Flags.Set( Flag_Enabled, Enabled );	}
	FORCEINLINE Bool		IsEnabled() const							{	return m_Flags( Flag_Enabled );	}
	TFlags<TRenderNode::RenderFlags::Flags>&	Debug_ForceRenderFlagsOn()	{	return m_Debug_ForceRenderFlagsOn;	}
	TFlags<TRenderNode::RenderFlags::Flags>&	Debug_ForceRenderFlagsOff()	{	return m_Debug_ForceRenderFlagsOff;	}
	void					SetScreenZ(u8 NewZ);						//	set z and resort screen order
	FORCEINLINE u8			GetScreenZ() const							{	return m_ScreenZ;	}

	void					SetRootRenderNode(TRefRef NodeRef)			{	m_RootRenderNodeRef = NodeRef;	}
	void					SetRootRenderNode(TRenderNode& RenderNode)	{	SetRootRenderNode( RenderNode.GetNodeRef() );	}
	TRefRef					GetRootRenderNodeRef() const				{	return m_RootRenderNodeRef;	}
	TRenderNode*			GetRootRenderNode() const;					//	gets the render node at the root

	void					SetRootQuadTreeZone(TLMaths::TQuadTreeZone* pQuadTreeZone);
	TLMaths::TQuadTreeZone*	GetRootQuadTreeZone()						{	return m_pRootQuadTreeZone;	}

	//	generic scene rendering controls
	virtual void			BeginScene()										{	}					//	save off current scene (and optionally reset)
	virtual void			BeginSceneReset(Bool ApplyCamera=TRUE)				{	}					//	save off current scene (and optionally reset)
	virtual void			EndScene()											{	}					//	restore previous scene

	//	clever screen<->world calculation stuff
	Bool					GetWorldRay(TLMaths::TLine& WorldRay,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const;	//	get world pos from 2d point inside our rendertarget size
	Bool					GetWorldPos(float3& WorldPos,float WorldDepth,const Type2<s32>& RenderTargetPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const;	//	get world pos from 2d point inside our rendertarget size

	Bool					GetRenderTargetPos(Type2<s32>& RenderTargetPos, const float3& WorldPos,const Type4<s32>& RenderTargetSize,TScreenShape ScreenShape) const; // get 2d point from 3d world pos

	const TLMaths::TBox2D&	GetWorldViewBox(float WorldDepth=0.f) const			{	return m_pCamera->GetWorldViewBox( WorldDepth );	}	//	the world-space box for the extents at the edges of the screen.
	const TLMaths::TBox2D&	GetWorldViewBox(TPtr<TScreen>& pScreen,float WorldDepth=0.f);		//	same as GetWorldViewBox but can be used before a render

	FORCEINLINE Bool		operator<(const TSorter<TLRender::TRenderTarget,u8>& RenderTarget) const	{	return GetScreenZ() < RenderTarget->GetScreenZ();	}
	FORCEINLINE Bool		operator<(const TRef& Ref) const					{	return GetRef() < Ref;	}
	FORCEINLINE Bool		operator==(const TRef& Ref) const					{	return GetRef() == Ref;	}
	FORCEINLINE Bool		operator<(const u8& ZOrder) const					{	return GetScreenZ() < ZOrder;	}
	FORCEINLINE Bool		operator==(const u8& ZOrder) const					{	return GetScreenZ() == ZOrder;	}

protected:
	Bool							DrawNode(TRenderNode& RenderNode,TRenderNode* pParentRenderNode,const TLMaths::TTransform* pSceneTransform,TColour SceneColour,TLMaths::TQuadTreeNode* pCameraZoneNode);	//	render a render object
	void							DrawMeshWrapper(const TLAsset::TMesh* pMesh,TRenderNode& RenderNode, TColour SceneColour,TPtrArray<TRenderNode>& PostRenderList);	
	void							DrawMesh(const TLAsset::TMesh& Mesh,const TLAsset::TTexture* pTexture,TRenderNode& RenderNode,const TFlags<TRenderNode::RenderFlags::Flags>& RenderFlags,Bool HasAlpha);
	void							DrawMeshShape(const TLMaths::TShape& Shape,TRenderNode& RenderNode,const TFlags<TRenderNode::RenderFlags::Flags>& RenderFlags,Bool ResetScene);

	virtual Bool					BeginProjectDraw(TLRender::TProjectCamera& Camera,TScreenShape ScreenShape)	{	return TRUE;	}	//	setup projection mode
	virtual void					EndProjectDraw()																{	}
	virtual Bool					BeginOrthoDraw(TLRender::TOrthoCamera& Camera,TScreenShape ScreenShape)		{	return TRUE;	}	//	setup ortho projection mode
	virtual void					EndOrthoDraw()																	{	}

	SyncBool						IsRenderNodeVisible(TRenderNode& RenderNode,TLMaths::TQuadTreeNode*& pRenderZoneNode,TLMaths::TQuadTreeNode* pCameraZoneNode,const TLMaths::TTransform* pSceneTransform,Bool& RenderNodeIsInsideCameraZone);	//	check zone of node against camera's zone to determine visibility. if no scene transform is provided then we only do quick tests with no calculations. This can result in a SyncWait returned which means we need to do calculations to make sure of visibility
	Bool							IsZoneVisible(TLMaths::TQuadTreeNode* pCameraZoneNode,TLMaths::TQuadTreeZone* pZone,TLMaths::TQuadTreeNode* pZoneNode,Bool& RenderNodeIsInsideCameraZone);

	void							OnSizeChanged();		//	do any recalculations we need to when the render target's size changes

	void							Debug_DrawZone(TLMaths::TQuadTreeZone& Zone,float z,TLMaths::TQuadTreeNode* pCameraZoneNode);

protected:
	TRef							m_Ref;					//	render target ref
	Type4<s32>						m_Size;					//	pos + w + h. negative numbers mean min/max's
	TPtr<TCamera>					m_pCamera;				//	camera 
	TLMaths::TTransform				m_CameraTransform;		//	camera transform applied on every scene reset - so its the modelview transform
	const TLMaths::TMatrix*			m_pCameraMatrix;		//	now matrix is out of TTransform - we might still need this matrix for the look-at orientation. This pointer should be used if valid, it's a pointer to the camera's lookat matrix

	TColour							m_ClearColour;			//	clear colour
	TFlags<Flags>					m_Flags;				//	render target flags
	u8								m_ScreenZ;				//	order of render targets in screen

	TLMaths::TQuadTreeZone*			m_pRootQuadTreeZone;	//	root visibility zone

	TRef							m_RootRenderNodeRef;	//	root render node
	TPtr<TRenderNodeClear>			m_pRenderNodeClear;		//	clear-screen render object
	s32								m_Debug_SceneCount;		//	to check we do Begin and EndScene in sync
	TPtrArray<TRenderNode>			m_TempPostRenderList;	//	instead of having a local, we have a member list of post render nodes. performance saving

	u32								m_Debug_PolyCount;
	u32								m_Debug_VertexCount;	
	u32								m_Debug_NodeCount;			//	number of nodes we've rendered
	u32								m_Debug_NodeCulledCount;	//	number of nodes we almost rendered, but culled
	
	TFlags<TLRender::TRenderNode::RenderFlags::Flags>	m_Debug_ForceRenderFlagsOn;	//	force render objects to render with these flags ON - like globally turning on/off debug rendering
	TFlags<TLRender::TRenderNode::RenderFlags::Flags>	m_Debug_ForceRenderFlagsOff;	//	force render objects to render with these flags OFF
};


