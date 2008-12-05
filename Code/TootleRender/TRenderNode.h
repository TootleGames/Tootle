/*------------------------------------------------------

	Render object

-------------------------------------------------------*/
#pragma once

//#include "TLRender.h"
#include <TootleCore/TPtrArray.h>
#include <TootleAsset/TMesh.h>
#include <TootleCore/TFlags.h>

#include <TootleCore/TLGraph.h>

namespace TLRender
{
	class TRenderNode;
	class TRenderTarget;
}


class TLRender::TRenderNode : public TLGraph::TGraphNode<TLRender::TRenderNode>
{
private:
	enum InvalidateFlags
	{
		InvalidateDummy = 0,
		InvalidateLocal,
		InvalidateWorld,
		InvalidateParents,
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
			HasShader,
			HasNormals,
			HasTexture,
			HasTexture2,
			UseVertexColours,
			HasAlpha,
			HasTextureAlpha,
			HasTexture2Alpha,
			MergeColour,		//	merge colour from parent node when rendering
			DepthRead,			//	read from depth buffer (off draws over everything)
			DepthWrite,			//	write to depth buffer (off means will get drawn over)
			ResetScene,			//	position and rotation are not inherited
			AffectsParentBounds,		//	if not set bounds of parent[s] are not invalidated or accumulated by this object (debug items usually)
			CalcWorldBoundsBox,			//	always calculate world bounds box (for physics, object picking etc etc)
			CalcWorldBoundsSphere,		//	always calculate world bounds sphere (for physics, object picking etc etc)
			EnableVBO,					//	enable creation & usage of VBO's
			EnableFixedVerts,			//	enable "fixed" verts (interleaved)
			Debug_Wireframe,
			Debug_Points,	
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
	TRenderNode(TRefRef RenderNodeRef=TRef());
	virtual ~TRenderNode()												{};

	virtual void							UpdateAll(float Timestep)					{	};	//	gr: render graph does no updates through the tree

	FORCEINLINE const TLMaths::TTransform&	GetTransform() const						{	return m_Transform;	}
	FORCEINLINE void						SetTransform(const TLMaths::TTransform& Transform)	{	m_Transform = Transform;	SetLocalMatrixInvalid();	}
	FORCEINLINE const float3&				GetTranslate() const						{	return m_Transform.GetTranslate() ;	}
	FORCEINLINE void						SetTranslate(const float3& Translate)		{	m_Transform.SetTranslate( Translate );	SetLocalMatrixInvalid();	}
	FORCEINLINE const float3&				GetScale() const							{	return m_Transform.GetScale() ;	}
	FORCEINLINE void						SetScale(const float3& Scale)				{	m_Transform.SetScale( Scale );	SetLocalMatrixInvalid();	}
	FORCEINLINE const TLMaths::TQuaternion&	GetRotation() const							{	return m_Transform.GetRotation() ;	}
	FORCEINLINE void						SetRotation(const TLMaths::TQuaternion& Rotation)	{	m_Transform.SetRotation( Rotation );	SetLocalMatrixInvalid();	}
	FORCEINLINE float						GetLineWidth() const						{	return m_LineWidth;	}
	FORCEINLINE void						SetLineWidth(float Width)					{	m_LineWidth = Width;	}

	FORCEINLINE TFlags<RenderFlags::Flags>&	GetRenderFlags()							{	return m_RenderFlags;	}
	FORCEINLINE const TFlags<RenderFlags::Flags>&	GetRenderFlags() const						{	return m_RenderFlags;	}
	void									ClearDebugRenderFlags();
	FORCEINLINE void						SetColour(const TColour& Colour)			{	m_Colour = Colour;	}
	FORCEINLINE const TColour&				GetColour() const							{	return m_Colour;	}
	FORCEINLINE const TRef&					GetMeshRef() const							{	return m_MeshRef;	}
	FORCEINLINE void						SetMeshRef(TRefRef MeshRef)					{	if ( m_MeshRef != MeshRef )	{	m_MeshRef = MeshRef;	m_pMeshCache = NULL;	OnBoundsChanged();	}	}

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

	FORCEINLINE void						OnBoundsChanged()							{	SetBoundsInvalid( TInvalidateFlags( InvalidateLocal, InvalidateWorld, InvalidateParents ) );	}
	FORCEINLINE void						SetLocalMatrixInvalid()						{	SetBoundsInvalid( TInvalidateFlags( HasChildren() ? InvalidateLocal : InvalidateDummy, InvalidateWorld, InvalidateParents, InvalidateChildren ) );	}

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

	FORCEINLINE Bool						operator==(TRefRef Ref) const				{	return GetRenderNodeRef() == Ref;	}
	FORCEINLINE Bool						operator<(TRefRef Ref) const				{	return GetRenderNodeRef() < Ref;	}

protected:
	void						SetBoundsInvalid(const TInvalidateFlags& InvalidateFlags=TInvalidateFlags(InvalidateLocal,InvalidateWorld,InvalidateParents,InvalidateChildren));	//	set all bounds as invalid

protected:
	TLMaths::TTransform			m_Transform;
	TColour						m_Colour;
	float						m_LineWidth;				//	temporary until moved into mesh asset... thickness of the lines we render

	//	gr: todo: almagamate all these bounds shapes into a single bounds type that does all 3 or picks the best or something
	TLMaths::TBox				m_LocalBoundsBox;			//	bounding box of self (without transformation) and children (with transformation, so relative to us)
	TLMaths::TBox				m_WorldBoundsBox;			//	bounding box of self in world space
	TLMaths::TSphere			m_LocalBoundsSphere;		//	bounding sphere Shape of self (without transformation) and children (with transformation, so relative to us)
	TLMaths::TSphere			m_WorldBoundsSphere;		//	bounding sphere Shape of self in world space
	TLMaths::TCapsule			m_LocalBoundsCapsule;		//	bounding capsule Shape of self (without transformation) and children (with transformation, so relative to us)
	TLMaths::TCapsule			m_WorldBoundsCapsule;		//	bounding capsule shape of self in world space

	TFlags<RenderFlags::Flags>	m_RenderFlags;

	//	todo: turn all these into ref properties in a KeyArray to make it a bit more flexible
	TRef						m_MeshRef;
	TPtr<TLAsset::TMesh>		m_pMeshCache;

	TBinaryTree					m_Data;					//	data attached to render object
};