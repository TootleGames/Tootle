/*------------------------------------------------------

	Render object

-------------------------------------------------------*/
#pragma once

//#include "TLRender.h"
#include <TootleCore/TPtrArray.h>
#include <TootleCore/TKeyArray.h>
#include <TootleAsset/TMesh.h>
#include <TootleAsset/TTexture.h>
#include <TootleCore/TFlags.h>
#include <TootleCore/TLGraph.h>
#include <TootleMaths/TQuadTree.h>
#include <TootleMaths/TCapsule.h>



namespace TLRender
{
	class TRenderNode;
	class TRenderTarget;
	class TRenderZoneNode;
}

namespace TLMaths
{
	template<class SHAPETYPE>
	class TBoundsShape;
}


template<class SHAPETYPE>
class TLMaths::TBoundsShape
{
public:
	typedef const SHAPETYPE& (TLRender::TRenderNode::*TCalcLocalBoundsFunc)();
public:
	TBoundsShape() : m_Valid ( SyncFalse )	{}

	const SHAPETYPE&	CalcWorldShape(TLRender::TRenderNode& RenderNode,TCalcLocalBoundsFunc pCalcLocalBoundsFunc);	//	if invalid calculate our local bounds box (accumulating children) if out of date and return it
	FORCEINLINE void	SetShapeOld()			{	if ( m_Valid == SyncTrue )	m_Valid = SyncWait;	}	//	if up-to-date (TRUE) then make old (WAIT)
	FORCEINLINE Bool	SetLocalShapeInvalid()	{	if ( !m_LocalShape.IsValid() )	return FALSE;	m_LocalShape.SetInvalid();	return TRUE;	}

public:
	SyncBool		m_Valid;		//	Validity of bounds box - SyncWait if valid, but old
	SHAPETYPE		m_LocalShape;	//	
	SHAPETYPE		m_WorldShape;	//	
};



class TLRender::TRenderNode : public TLGraph::TGraphNode<TLRender::TRenderNode>
{
public:
	friend class TLMaths::TBoundsShape<TLMaths::TBox>;
	friend class TLMaths::TBoundsShape<TLMaths::TBox2D>;
	friend class TLMaths::TBoundsShape<TLMaths::TSphere>;
	friend class TLMaths::TBoundsShape<TLMaths::TSphere2D>;
	friend class TRendergraph;

private:
	enum InvalidateFlags
	{
		InvalidateDummy = 0,
		InvalidateLocalBounds,
		InvalidateWorldBounds,
		InvalidateWorldPos,
		InvalidateParentLocalBounds,
		ForceInvalidateParentsLocalBounds,		//	invalidate parents even if nothing has changed - I've needed this to invalidate parent's boxes even though current bounds are invalid
		InvalidateChildWorldBounds,				//	
		InvalidateChildWorldPos,				//	
		InvalidateParentsChildrenWorldBounds,	//	explicitly invalidate our parents' children - by default do not (otherwise we end up invalidating the whole tree as it goes up to the root down)
		InvalidateParentsChildrenWorldPos,		//	as above, but for pos
		FromChild,								//	child has instigated this invalidation
		FromParent,								//	parent has instigated this invalidation
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
			Enabled,					//	when not enabled, this (and tree) is not rendered
			DepthRead,					//	read from depth buffer (off draws over everything)
			DepthWrite,					//	write to depth buffer (off means will get drawn over)
			ResetScene,					//	position and rotation are not inherited
			UseVertexColours,			//	bind vertex colours of mesh. if not set when rendering, a mesh the colours are not bound
			UseVertexUVs,				//	bind vertex UVs of mesh. if not set when rendering we have no texture mapping
			UseMeshLineWidth,			//	calculates mesh/world line width -> screen/pixel width
			UseNodeColour,				//	set when colour is something other than 1,1,1,1 to save some processing (off by default!)
			EnableCull,					//	enable camera/frustum/zone culling. if disabled, the whole tree below is disabled
			ForceCullTestChildren,		//	if we are not culled, still do a cull test with children. By default, when not culled, we don't test children as they should be encapsulated within our bounds
			InvalidateBoundsByChildren,	//	default behaviour is to invalidate our bounding box when child CHANGES. we can disable this for certain cases - eg. root objects, which when invalidated cause the entire tree to recalculate stuff - still invalidated when a child is ADDED to the tree (this may have to change to "first-calculation of bounds")
			AddBlending,					//	will ADD when rendering colours/textures etc
	
			Debug_Wireframe,			//	draw in wireframe (all white outlines)
			Debug_ColourWireframe,		//	when drawing in wireframe keep colours
			Debug_Points,				//	draw a point at every vertex
			Debug_Outline,				//	render again with wireframe on
			Debug_Position,				//	draws a 3axis cross at 0,0,0 on the render node
			Debug_LocalBoundsBox,		//	render our local bounds box
			Debug_WorldBoundsBox,		//	render our world bounds box
			Debug_LocalBoundsSphere,	//	render our local bounds sphere
			Debug_WorldBoundsSphere,	//	render our world bounds sphere
		};
	};

public:
	TRenderNode(TRefRef RenderNodeRef=TRef(),TRefRef TypeRef=TRef());

	virtual void							Initialise(TLMessaging::TMessage& Message);	//	generic render node init
	virtual void 							Update(float Timestep);	
	virtual void							Shutdown();									//	clean-up any TPtrs back to us so we will be deallocated

	FORCEINLINE const TLMaths::TTransform&	GetTransform() const						{	return m_Transform;	}
	FORCEINLINE const float3&				GetTranslate() const						{	return m_Transform.GetTranslate() ;	}
	FORCEINLINE const float3&				GetScale() const							{	return m_Transform.GetScale() ;	}
	FORCEINLINE const TLMaths::TQuaternion&	GetRotation() const							{	return m_Transform.GetRotation() ;	}

	FORCEINLINE void						SetTransform(const TLMaths::TTransform& Transform);
	FORCEINLINE void						SetTranslate(const float3& Translate,Bool Invalidate=TRUE);
	FORCEINLINE void						SetScale(const float3& Scale,Bool Invalidate=TRUE);
	FORCEINLINE void						SetScale(float Scale,Bool Invalidate=TRUE)	{	SetScale( float3( Scale, Scale, Scale ), Invalidate );	}
	FORCEINLINE void						SetRotation(const TLMaths::TQuaternion& Rotation,Bool Invalidate=TRUE);
	
	FORCEINLINE float						GetLineWidth() const						{	return m_LineWidth;	}
	FORCEINLINE void						SetLineWidth(float Width)					{	m_LineWidth = Width;	}

	FORCEINLINE TFlags<RenderFlags::Flags>&	GetRenderFlags()							{	return m_RenderFlags;	}
	FORCEINLINE const TFlags<RenderFlags::Flags>&	GetRenderFlags() const				{	return m_RenderFlags;	}
	void									ClearDebugRenderFlags();
	FORCEINLINE Bool						IsEnabled() const							{	return m_RenderFlags( RenderFlags::Enabled );	}
	FORCEINLINE void						SetEnabled(Bool Enabled)					{	m_RenderFlags.Set( RenderFlags::Enabled, Enabled );	}

	FORCEINLINE void						SetAlpha(float Alpha)						{	if ( m_Colour.GetAlpha() != Alpha )	{	m_Colour.GetAlpha() = Alpha;	OnColourChanged();	}	}
	FORCEINLINE float						GetAlpha() const							{	return m_Colour.GetAlpha();	}
	FORCEINLINE void						SetColour(const TColour& Colour)			{	m_Colour = Colour;	OnColourChanged();	}
	FORCEINLINE const TColour&				GetColour() const							{	return m_Colour;	}
	FORCEINLINE Bool						IsColourValid() const						{	return m_RenderFlags( RenderFlags::UseNodeColour );	}
	FORCEINLINE void						OnColourChanged();							//	enable node colour if non-white
	FORCEINLINE const TRef&					GetMeshRef() const							{	return m_MeshRef;	}
	FORCEINLINE void						SetMeshRef(TRefRef MeshRef)					{	if ( m_MeshRef != MeshRef )	{	m_MeshRef = MeshRef;	OnMeshRefChanged();	}	}
	FORCEINLINE const TRef&					GetTextureRef() const						{	return m_TextureRef;	}
	FORCEINLINE void						SetTextureRef(TRefRef TextureRef)			{	if ( m_TextureRef != TextureRef )	{	m_TextureRef = TextureRef;	OnTextureRefChanged();	}	}

	virtual TPtr<TLAsset::TMesh>&			GetMeshAsset();								//	default behaviour fetches the mesh from the asset lib with our mesh ref
	virtual TPtr<TLAsset::TTexture>&		GetTextureAsset();							//	default behaviour fetches the mesh from the asset lib with our mesh ref

	FORCEINLINE void						SetRenderNodeRef(TRefRef Ref)				{	SetNodeRef( Ref );	}
	FORCEINLINE TRefRef						GetRenderNodeRef() const					{	return GetNodeRef();	}
	FORCEINLINE TRefRef						GetOwnerSceneNodeRef() const				{	return m_OwnerSceneNode;	}

	virtual void							OnAdded();
	void									Copy(const TRenderNode& OtherRenderNode);	//	copy render object DOES NOT COPY CHILDREN or parent! just properties

	FORCEINLINE TBinaryTree&				GetData()									{	return m_Data;	}
	FORCEINLINE TPtr<TBinaryTree>			GetData(TRefRef DataRef)					{	return GetData().GetChild( DataRef );	}
	FORCEINLINE TPtr<TBinaryTree>			AddData(TRefRef DataRef)					{	return GetData().AddChild( DataRef );	}

	//	overloaded render routine for generic stuff. if this returns TRUE then continue with default RenderNode rendering - 
	//	if FALSE presumed we are doing psuedo rendering ourselves (creating RenderNodes and rendering them to the render target)
	virtual Bool							Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList);	//	pre-draw routine for a render object

	FORCEINLINE void						OnTransformChanged();						//	invalidate bounds
	FORCEINLINE void						OnMeshChanged();							//	invalidate bounds
	FORCEINLINE void						OnBoundsChanged()							{	OnMeshChanged();	}

	void									SetWorldTransform(const TLMaths::TTransform& SceneTransform);	//	set new world transform
	void									SetWorldTransformOld();						//	downgrade all world shape/transform states from valid to old
	FORCEINLINE SyncBool					IsWorldTransformValid() const				{	return m_WorldTransformValid;	}

	const float3&							GetWorldPos();								//	calculate our new world position from the latest scene transform
	FORCEINLINE const float3&				GetWorldPos(SyncBool& IsValid) 				{	GetWorldPos();	IsValid = m_WorldPosValid;	return m_WorldPos;	}

	//	get world shapes/datums - recalculates if old
	const TLMaths::TBox&					GetWorldBoundsBox()		{	return m_BoundsBox.CalcWorldShape( *this, &TLRender::TRenderNode::CalcLocalBoundsBox );	}
	const TLMaths::TBox2D&					GetWorldBoundsBox2D()		{	return m_BoundsBox2D.CalcWorldShape( *this, &TLRender::TRenderNode::CalcLocalBoundsBox2D );	}
	const TLMaths::TSphere&					GetWorldBoundsSphere()		{	return m_BoundsSphere.CalcWorldShape( *this, &TLRender::TRenderNode::CalcLocalBoundsSphere );	}
	const TLMaths::TSphere2D&				GetWorldBoundsSphere2D()	{	return m_BoundsSphere2D.CalcWorldShape( *this, &TLRender::TRenderNode::CalcLocalBoundsSphere2D );	}

	//	get world shape/datum - no recalcualtion, also returns age (False=Invalid, Wait=Old, True=UpToDate)
	const TLMaths::TBox&					GetWorldBoundsBox(SyncBool& Validity) const			{	Validity = m_BoundsBox.m_Valid;			return m_BoundsBox.m_WorldShape;	}
	const TLMaths::TBox2D&					GetWorldBoundsBox2D(SyncBool& Validity) const		{	Validity = m_BoundsBox2D.m_Valid;		return m_BoundsBox2D.m_WorldShape;	}
	const TLMaths::TSphere&					GetWorldBoundsSphere(SyncBool& Validity) const		{	Validity = m_BoundsSphere.m_Valid;		return m_BoundsSphere.m_WorldShape;	}
	const TLMaths::TSphere2D&				GetWorldBoundsSphere2D(SyncBool& Validity) const	{	Validity = m_BoundsSphere2D.m_Valid;	return m_BoundsSphere2D.m_WorldShape;	}

	const TLMaths::TBox&					CalcLocalBoundsBox();						//	return our current local bounds box and calculate if invalid
	const TLMaths::TBox2D&					CalcLocalBoundsBox2D();						//	return our current local bounds box and calculate if invalid
	const TLMaths::TSphere&					CalcLocalBoundsSphere();					//	return our current local bounds box and calculate if invalid
	const TLMaths::TSphere2D&				CalcLocalBoundsSphere2D();					//	return our current local bounds box and calculate if invalid

	FORCEINLINE TPtr<TLMaths::TQuadTreeNode>*	GetRenderZoneNode(TRefRef RenderTargetRef)	{	return m_RenderZoneNodes.Find( RenderTargetRef );	}
	FORCEINLINE TPtr<TLMaths::TQuadTreeNode>*	SetRenderZoneNode(TRefRef RenderTargetRef,TPtr<TLMaths::TQuadTreeNode>& pRenderZoneNode)	{	return m_RenderZoneNodes.Add( RenderTargetRef, pRenderZoneNode );	}

	FORCEINLINE Bool						operator==(TRefRef Ref) const				{	return GetRenderNodeRef() == Ref;	}
	FORCEINLINE Bool						operator<(TRefRef Ref) const				{	return GetRenderNodeRef() < Ref;	}

protected:
	FORCEINLINE void						OnMeshRefChanged()							{	m_pMeshCache = NULL;	OnMeshChanged();	}
	FORCEINLINE void						OnTextureRefChanged()						{	m_pTextureCache = NULL;	}
	//void									SetBoundsInvalid(const TInvalidateFlags& InvalidateFlags=TInvalidateFlags(InvalidateLocalBounds,InvalidateWorldBounds,InvalidateWorldPos,InvalidateParents,InvalidateChildren));	//	set all bounds as invalid
	void									SetBoundsInvalid(const TInvalidateFlags& InvalidateFlags);

	virtual void							ProcessMessage(TLMessaging::TMessage& Message);

protected:
	TLMaths::TTransform			m_Transform;				//	local transform of node
	TLMaths::TTransform			m_WorldTransform;			//	last SceneTransform
	SyncBool					m_WorldTransformValid;		//	m_WorldTransform is... false - invalid (never calculated before). Wait - Last one is valid, but needs updating on next render. True - most update to date and doesnt need updating again

	TColour						m_Colour;					//	colour of render node - only works if UseNodeColour is set
	float						m_LineWidth;				//	this is an overriding line width for rendering lines in the mesh. In pixel width. NOT like the mesh line width which is in a world-size.
	float3						m_WorldPos;					//	we always calc the world position on render, even if we dont calc the bounds box/sphere/etc, it's quick and handy!
	SyncBool					m_WorldPosValid;			//	if this is not valid then the transform of this node has changed since our last render

	//	gr: todo: almagamate all these bounds shapes into a single bounds type that does all 3 or picks the best or something
	TLMaths::TBoundsShape<TLMaths::TBox>		m_BoundsBox;
	TLMaths::TBoundsShape<TLMaths::TBox2D>		m_BoundsBox2D;
	TLMaths::TBoundsShape<TLMaths::TSphere>		m_BoundsSphere;
	TLMaths::TBoundsShape<TLMaths::TSphere2D>	m_BoundsSphere2D;

	TFlags<RenderFlags::Flags>	m_RenderFlags;

	TKeyArray<TRef,TPtr<TLMaths::TQuadTreeNode> >	m_RenderZoneNodes;	//	for each render target we can have a Node(TRenderZoneNode) for Render Zones

	//	todo: turn all these into ref properties in a KeyArray to make it a bit more flexible
	TRef						m_MeshRef;
	TPtr<TLAsset::TMesh>		m_pMeshCache;
	TRef						m_TextureRef;
	TPtr<TLAsset::TTexture>		m_pTextureCache;

	TBinaryTree					m_Data;					//	data attached to render object

	TRef						m_OwnerSceneNode;		//	"Owner" scene node - if this is set then we automaticcly process some stuff (ie. catching OnTransform of a scene node we set our transform)
};



//---------------------------------------------------------------
//	QuadTreeNode for render nodes
//---------------------------------------------------------------
class TLRender::TRenderZoneNode : public TLMaths::TQuadTreeNode
{
public:
	TRenderZoneNode(TRefRef RenderNodeRef);

	virtual SyncBool	IsInShape(const TLMaths::TBox2D& Shape);

protected:
	TRef				m_RenderNodeRef;		//	render node that we're linked to
	TPtr<TRenderNode>	m_pRenderNode;			//	cache of render node
};






//---------------------------------------------------------------
//	enable node colour if non-white
//---------------------------------------------------------------
FORCEINLINE void TLRender::TRenderNode::OnColourChanged()
{
	if ( m_Colour.GetRed() > TLMaths::g_NearOne &&
		m_Colour.GetGreen() > TLMaths::g_NearOne &&
		m_Colour.GetBlue() > TLMaths::g_NearOne &&
		m_Colour.GetAlpha() > TLMaths::g_NearOne )
	{
		m_RenderFlags.Clear( RenderFlags::UseNodeColour );
	}
	else
	{
		m_RenderFlags.Set( RenderFlags::UseNodeColour );
	}
}



FORCEINLINE void TLRender::TRenderNode::SetTransform(const TLMaths::TTransform& Transform)
{
	//	gr: note, no checks using this function atm...
	m_Transform = Transform;	
	OnTransformChanged();
}

FORCEINLINE void TLRender::TRenderNode::SetTranslate(const float3& Translate,Bool Invalidate)				
{	
	m_Transform.SetTranslate( Translate );

	if ( Invalidate )	
		OnTransformChanged();	
}

FORCEINLINE void TLRender::TRenderNode::SetScale(const float3& Scale,Bool Invalidate)						
{	
	m_Transform.SetScale( Scale );	

	if ( Invalidate )	
		OnTransformChanged();	
}

FORCEINLINE void TLRender::TRenderNode::SetRotation(const TLMaths::TQuaternion& Rotation,Bool Invalidate)	
{	
	m_Transform.SetRotation( Rotation );	

	if ( Invalidate )	
		OnTransformChanged();	
}

//---------------------------------------------------------------
//	invalidate bounds when pos/rot/scale
//---------------------------------------------------------------
FORCEINLINE void TLRender::TRenderNode::OnTransformChanged()						
{	
	SetBoundsInvalid( TInvalidateFlags( 
						//HasChildren() ? InvalidateLocalBounds : InvalidateDummy, //	gr: not needed?
						InvalidateWorldPos,				//	world pos must have changed - may be able to reduce this to just Translate changes
						InvalidateWorldBounds,			//	shape of mesh must have changed
						InvalidateParentLocalBounds,	//	our parent's LOCAL bounds has now changed as it's based on it's children (this)
						InvalidateChildWorldBounds,		//	invalidate the children's world bounds
						InvalidateChildWorldPos		//	invalidate the children's world pos too
					) );	
}


//---------------------------------------------------------------
//	invalidate bounds when mesh has changed
//---------------------------------------------------------------
FORCEINLINE void TLRender::TRenderNode::OnMeshChanged()								
{	
	SetBoundsInvalid( TInvalidateFlags( 
						InvalidateLocalBounds,			//	our local shape has probbaly changed
						InvalidateWorldBounds,			//	this also affects world bounds
						InvalidateParentLocalBounds		//	and so shape of parents may have changed as it encapsulates us
						) );	
}






template<class SHAPETYPE>
const SHAPETYPE& TLMaths::TBoundsShape<SHAPETYPE>::CalcWorldShape(TLRender::TRenderNode& RenderNode,TCalcLocalBoundsFunc pCalcLocalBoundsFunc)
{
	//	... world transform must be valid (or old/wait) and our bounds are not this new, so recalculate
	SyncBool IsWorldTransformValid = RenderNode.m_WorldTransformValid;

	//	check validity synchronoisation is correct
	if ( m_Valid == SyncTrue && IsWorldTransformValid != SyncTrue )
	{
		TLDebug_Break("Mis-match in age of world shape - shape should always be same or older as the transform");
		m_Valid = IsWorldTransformValid;
	}

	//	world bounds is up to date
	if ( m_Valid == IsWorldTransformValid )
		return m_WorldShape;

	//	world transform isn't valid at all
	if ( IsWorldTransformValid == SyncFalse )
	{
		//	gr: shouldn't be valid...
		m_WorldShape.SetInvalid();
		return m_WorldShape;
	}

	//	get/recalc local bounds box
	const SHAPETYPE& LocalShape = ((&RenderNode)->*pCalcLocalBoundsFunc)();
	if ( !LocalShape.IsValid() )
	{
		//	gr: shouldn't be valid...
		m_WorldShape.SetInvalid();
		return m_WorldShape;
	}

	//	tranform our local bounds into the world bounds
	m_WorldShape = LocalShape;
	m_WorldShape.Transform( RenderNode.m_WorldTransform );
	
	//	update state (matches world transform state)
	m_Valid = IsWorldTransformValid;

	return m_WorldShape;
}


