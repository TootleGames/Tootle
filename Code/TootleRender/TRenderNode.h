/*------------------------------------------------------

	Render object

-------------------------------------------------------*/
#pragma once

//#include "TLRender.h"
#include <TootleCore/TPtrArray.h>
#include <TootleCore/TKeyArray.h>
#include <TootleAsset/TMesh.h>
#include <TootleCore/TFlags.h>
#include <TootleCore/TLGraph.h>
#include <TootleMaths/TQuadTree.h>

namespace TLRender
{
	class TRenderNode;
	class TRenderTarget;
	class TRenderZoneNode;
}



class TLRender::TRenderNode : public TLGraph::TGraphNode<TLRender::TRenderNode>
{
private:
	enum InvalidateFlags
	{
		InvalidateDummy = 0,
		InvalidateLocalBounds,
		InvalidateWorldBounds,
		InvalidateWorldPos,
		InvalidateParents,
		ForceInvalidateParents,		//	invalidate parents even if nothing has changed - I've needed this to invalidate parent's boxes even though current bounds are invalid
		InvalidateChildren,
	};
	typedef TFlags<InvalidateFlags> TInvalidateFlags;

public:
	//	gr: not actually a class...
	class RenderFlags
	{
	public:
		//	flags for all the parts of the mesh we're drawing/processing
		enum Flags
		{
			Enabled,
			DepthRead,					//	read from depth buffer (off draws over everything)
			DepthWrite,					//	write to depth buffer (off means will get drawn over)
			ResetScene,					//	position and rotation are not inherited
			CalcWorldBoundsBox,			//	always calculate world bounds box (for physics, object picking etc etc)
			CalcWorldBoundsSphere,		//	always calculate world bounds sphere (for physics, object picking etc etc)
			EnableVBO,					//	enable creation & usage of VBO's - remove flag for meshs that are modified often
			UseVertexColours,			//	bind vertex colours of mesh. if not set, when rendering a mesh the colours are not bound
			UseMeshLineWidth,			//	calculates mesh/world line width -> screen/pixel width
	
			Debug_Wireframe,			//	draw in wireframe
			Debug_Points,				//	draw a point at every vertex
			Debug_Outline,				//	render again with wireframe on
			Debug_LocalBoundsBox,		//	render our local bounds box
			Debug_WorldBoundsBox,		//	render our world bounds box
			Debug_LocalBoundsSphere,	//	render our local bounds sphere
			Debug_WorldBoundsSphere,	//	render our world bounds sphere
			Debug_LocalBoundsCapsule,	//	render our local bounds capsule
			Debug_WorldBoundsCapsule,	//	render our world bounds capsule
		};
	};

public:
	TRenderNode(TRefRef RenderNodeRef=TRef(),TRefRef TypeRef=TRef());
	virtual ~TRenderNode()					{};

	virtual void							Initialise(TPtr<TLMessaging::TMessage>& pMessage);	//	generic render node init

	FORCEINLINE const TLMaths::TTransform&	GetTransform() const						{	return m_Transform;	}
	FORCEINLINE void						SetTransform(const TLMaths::TTransform& Transform)	{	m_Transform = Transform;	OnTransformChanged();	}
	FORCEINLINE const float3&				GetTranslate() const						{	return m_Transform.GetTranslate() ;	}
	FORCEINLINE void						SetTranslate(const float3& Translate)		{	m_Transform.SetTranslate( Translate );	OnTransformChanged();	}
	FORCEINLINE const float3&				GetScale() const							{	return m_Transform.GetScale() ;	}
	FORCEINLINE void						SetScale(const float3& Scale)				{	m_Transform.SetScale( Scale );	OnTransformChanged();	}
	FORCEINLINE void						SetScale(float Scale)						{	SetScale( float3( Scale, Scale, Scale ) );	}
	FORCEINLINE const TLMaths::TQuaternion&	GetRotation() const							{	return m_Transform.GetRotation() ;	}
	FORCEINLINE void						SetRotation(const TLMaths::TQuaternion& Rotation)	{	m_Transform.SetRotation( Rotation );	OnTransformChanged();	}
	FORCEINLINE float						GetLineWidth() const						{	return m_LineWidth;	}
	FORCEINLINE void						SetLineWidth(float Width)					{	m_LineWidth = Width;	}
	FORCEINLINE const float3&				GetWorldPos() const							{	return m_WorldPos;	}
	FORCEINLINE const float3&				GetWorldPos(Bool& IsValid) const			{	IsValid = m_WorldPosValid;	return m_WorldPos;	}
	FORCEINLINE Bool						IsWorldPosValid() const						{	return m_WorldPosValid;	}

	FORCEINLINE TFlags<RenderFlags::Flags>&	GetRenderFlags()							{	return m_RenderFlags;	}
	FORCEINLINE const TFlags<RenderFlags::Flags>&	GetRenderFlags() const				{	return m_RenderFlags;	}
	void									ClearDebugRenderFlags();
	FORCEINLINE void						SetAlpha(float Alpha)						{	m_Alpha = Alpha;	}
	FORCEINLINE float						GetAlpha() const							{	return m_Alpha;	}
	FORCEINLINE const TRef&					GetMeshRef() const							{	return m_MeshRef;	}
	FORCEINLINE void						SetMeshRef(TRefRef MeshRef)					{	if ( m_MeshRef != MeshRef )	{	m_MeshRef = MeshRef;	OnMeshRefChanged();	}	}

	virtual void							GetMeshAsset(TPtr<TLAsset::TMesh>& pMesh);	//	default behaviour fetches the mesh from the asset lib with our mesh ref

	FORCEINLINE void						SetRenderNodeRef(TRefRef Ref)				{	SetNodeRef( Ref );	}
	FORCEINLINE const TRef&					GetRenderNodeRef() const					{	return GetNodeRef();	}

	virtual void							OnAdded();
	void									Copy(const TRenderNode& OtherRenderNode);	//	copy render object DOES NOT COPY CHILDREN or parent! just properties

	FORCEINLINE TBinaryTree&				GetData()									{	return m_Data;	}
	FORCEINLINE TPtr<TBinaryTree>			GetData(TRefRef DataRef)					{	return GetData().GetChild( DataRef );	}
	FORCEINLINE TPtr<TBinaryTree>			AddData(TRefRef DataRef)					{	return GetData().AddChild( DataRef );	}

	//	overloaded render routine for generic stuff. if this returns TRUE then continue with default RenderNode rendering - 
	//	if FALSE presumed we are doing psuedo rendering ourselves (creating RenderNodes and rendering them to the render target)
	virtual Bool							Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList);	//	pre-draw routine for a render object

	FORCEINLINE void						OnBoundsChanged()							{	SetBoundsInvalid( TInvalidateFlags( InvalidateLocalBounds, InvalidateWorldBounds, InvalidateParents ) );	}
	FORCEINLINE void						OnTransformChanged()						{	SetBoundsInvalid( TInvalidateFlags( HasChildren() ? InvalidateLocalBounds : InvalidateDummy, InvalidateWorldPos, InvalidateWorldBounds, InvalidateParents, InvalidateChildren ) );	}

	void									CalcWorldPos(const TLMaths::TTransform& SceneTransform);	//	calculate our new world position from the latest scene transform

	const TLMaths::TBox&					CalcWorldBoundsBox(const TLMaths::TTransform& SceneTransform);	//	if invalid calculate our local bounds box (accumulating children) if out of date and return it
	const TLMaths::TBox&					CalcLocalBoundsBox();						//	return our current local bounds box and calculate if invalid
	FORCEINLINE const TLMaths::TBox&		GetWorldBoundsBox() const					{	return m_WorldBoundsBox;	}	//	return our current local bounds box, possibly invalid
	FORCEINLINE const TLMaths::TBox&		GetLocalBoundsBox() const					{	return m_LocalBoundsBox;	}	//	return our current local bounds box, possibly invalid

	const TLMaths::TSphere&					CalcWorldBoundsSphere(const TLMaths::TTransform& SceneTransform);	//	if invalid calculate our local bounds box (accumulating children) if out of date and return it
	const TLMaths::TSphere&					CalcLocalBoundsSphere();					//	return our current local bounds box and calculate if invalid
	FORCEINLINE const TLMaths::TSphere&		GetWorldBoundsSphere() const				{	return m_WorldBoundsSphere;	}	//	return our current local bounds box, possibly invalid
	FORCEINLINE const TLMaths::TSphere&		GetLocalBoundsSphere() const				{	return m_LocalBoundsSphere;	}	//	return our current local bounds box, possibly invalid

	const TLMaths::TCapsule&				CalcWorldBoundsCapsule(const TLMaths::TTransform& SceneTransform);	//	if invalid calculate our local bounds box (accumulating children) if out of date and return it
	const TLMaths::TCapsule&				CalcLocalBoundsCapsule();					//	return our current local bounds box and calculate if invalid
	FORCEINLINE const TLMaths::TCapsule&	GetWorldBoundsCapsule() const				{	return m_WorldBoundsCapsule;	}	//	return our current local bounds box, possibly invalid
	FORCEINLINE const TLMaths::TCapsule&	GetLocalBoundsCapsule() const				{	return m_LocalBoundsCapsule;	}	//	return our current local bounds box, possibly invalid

	FORCEINLINE TPtr<TLMaths::TQuadTreeNode>*	GetRenderZoneNode(TRefRef RenderTargetRef)	{	return m_RenderZoneNodes.Find( RenderTargetRef );	}
	FORCEINLINE TPtr<TLMaths::TQuadTreeNode>*	SetRenderZoneNode(TRefRef RenderTargetRef,TPtr<TLMaths::TQuadTreeNode>& pRenderZoneNode)	{	return m_RenderZoneNodes.Add( RenderTargetRef, pRenderZoneNode );	}

	FORCEINLINE Bool						operator==(TRefRef Ref) const				{	return GetRenderNodeRef() == Ref;	}
	FORCEINLINE Bool						operator<(TRefRef Ref) const				{	return GetRenderNodeRef() < Ref;	}

protected:
	FORCEINLINE void						OnMeshRefChanged()							{	m_pMeshCache = NULL;	OnBoundsChanged();	}
	//void									SetBoundsInvalid(const TInvalidateFlags& InvalidateFlags=TInvalidateFlags(InvalidateLocalBounds,InvalidateWorldBounds,InvalidateWorldPos,InvalidateParents,InvalidateChildren));	//	set all bounds as invalid
	void									SetBoundsInvalid(const TInvalidateFlags& InvalidateFlags);

protected:
	TLMaths::TTransform			m_Transform;				//	local transform 
	float						m_Alpha;					//	alpha of render node
	float						m_LineWidth;				//	this is an overriding line width for rendering lines in the mesh. In pixel width. NOT like the mesh line width which is in a world-size.
	float3						m_WorldPos;					//	we always calc the world position on render, even if we dont calc the bounds box/sphere/etc, it's quick and handy!
	Bool						m_WorldPosValid;			//	if this is not valid then the transform of this node has changed since our last render

	//	gr: todo: almagamate all these bounds shapes into a single bounds type that does all 3 or picks the best or something
	TLMaths::TBox				m_LocalBoundsBox;			//	bounding box of self (without transformation) and children (with transformation, so relative to us)
	TLMaths::TBox				m_WorldBoundsBox;			//	bounding box of self in world space
	TLMaths::TSphere			m_LocalBoundsSphere;		//	bounding sphere Shape of self (without transformation) and children (with transformation, so relative to us)
	TLMaths::TSphere			m_WorldBoundsSphere;		//	bounding sphere Shape of self in world space
	TLMaths::TCapsule			m_LocalBoundsCapsule;		//	bounding capsule Shape of self (without transformation) and children (with transformation, so relative to us)
	TLMaths::TCapsule			m_WorldBoundsCapsule;		//	bounding capsule shape of self in world space

	TFlags<RenderFlags::Flags>	m_RenderFlags;

	TKeyArray<TRef,TPtr<TLMaths::TQuadTreeNode> >	m_RenderZoneNodes;	//	for each render target we can have a Node for Render Zones

	//	todo: turn all these into ref properties in a KeyArray to make it a bit more flexible
	TRef						m_MeshRef;
	TPtr<TLAsset::TMesh>		m_pMeshCache;

	TBinaryTree					m_Data;					//	data attached to render object
};



//---------------------------------------------------------------
//	QuadTreeNode for render nodes
//---------------------------------------------------------------
class TLRender::TRenderZoneNode : public TLMaths::TQuadTreeNode
{
public:
	TRenderZoneNode(TRefRef RenderNodeRef);

	void				CalcWorldBounds(TLRender::TRenderNode* pRenderNode,const TLMaths::TTransform& SceneTransform);	//	calculate all the world bounds we need to to do a zone test
	virtual SyncBool	IsInShape(const TLMaths::TBox2D& Shape);

protected:
	TRef				m_RenderNodeRef;		//	render node that we're linked to
};


