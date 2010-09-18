#include "TRasteriser.h"
#include <TootleAsset/TLAsset.h>
#include <TootleCore/TLMemory.h>
#include <stdio.h>	//	for memset... need to sort this!
#include <string.h>	//	for memset... need to sort this!



bool TLRaster::TRasteriser::Initialise()
{
	//	subscribe to the asset manager to catch when assets are deleted
	if ( !TLAsset::g_pManager )
		return false;

	this->SubscribeTo( TLAsset::g_pManager );

	return true;
}


void TLRaster::TRasteriser::Shutdown()
{
}

//----------------------------------------------------
//	catch when a texture asset changes
//----------------------------------------------------
void TLRaster::TRasteriser::ProcessMessage(TLMessaging::TMessage& Message)
{
	switch ( Message.GetMessageRef().GetData() )
	{
		//	Asset removed
		case TRef_Static(A,s,s,D,e):
		{
			//	get asset
			TTypedRef AssetRef;
			Message.ResetReadPos();
			if ( Message.Read( AssetRef ) )
			{
				//	check for texture changes
				if ( AssetRef.GetTypeRef() == TRef_Static(T,e,x,t,u) )
				{
					OnTextureDeleted( AssetRef.GetRef() );
				}
			}
		}
		break;
		
		//	Asset Changed
		case TRef_Static(A,s,s,C,h):
		{
			//	get asset
			TTypedRef AssetRef;
			Message.ResetReadPos();
			if ( Message.Read( AssetRef ) )
			{
				//	check for texture changes
				if ( AssetRef.GetTypeRef() == TRef_Static(T,e,x,t,u) )
				{
					OnTextureChanged( AssetRef.GetRef() );
				}
			}
		}
		break;
	}
}


//----------------------------------------------------------------------------//
//	initialise default setup
//----------------------------------------------------------------------------//
void TLRaster::TRasterData::Init()
{
	//	yuck! but fast! and simple!
	memset( this, 0, sizeof(*this) );
}


//----------------------------------------------------------------------------//
//	set material
//----------------------------------------------------------------------------//
void TLRaster::TRasterData::SetMaterial(const TLRaster::TMaterial& Material)
{
	m_Material = Material;
}


//----------------------------------------------------------------------------//
//	setup raster data to render wireframe data
//----------------------------------------------------------------------------//
void TLRaster::TRasterData::SetWireframe(bool ResetColour)
{
	m_Flags.Set( Flags::Wireframe );
	m_Material.m_LineWidth = 1.f;
	m_Material.m_BlendMode = TBlendMode::Opaque;

	if ( ResetColour )
		m_Material.m_Colour = TColour( 1.f, 1.f, 1.f, 1.f );
	else
		m_Material.m_Colour.SetOpaque();
	
	/*
	GetVertexElements().Remove( TLVertexElement::UV );
	GetVertexElements().Remove( TLVertexElement::NORM );
	GetVertexElements().Remove( TLVertexElement::COL24 );
	GetVertexElements().Remove( TLVertexElement::COL32 );
	GetVertexElements().Remove( TLVertexElement::COL64 );
	GetVertexElements().Remove( TLVertexElement::COLF );
	*/
	m_Material.m_pTexture = NULL;
}



//----------------------------------------------------------------------------//
//	setup raster data as debug data
//----------------------------------------------------------------------------//
void TLRaster::TRasterData::SetDebug()
{
	//	later to render last, no depth etc
}

/*
//----------------------------------------------------------------------------//
//	setup geometry from a vertex definition
//----------------------------------------------------------------------------//
void TLRaster::TRasterData::Set(const TLAsset::TVertexDef& VertexDefinition,const u8* pVertexData,u32 ElementMask=0xffffffff)
{
	//	loop through elements and add
	TArray<TVertexElement::Type>& DefElements = VertexDefinition.GetElements();
	TInPlaceArray<TRasterVertexElements> RasterElements = GetVertexElements();

	//	convert the mask to a slightly more friendly one
	TBits<TVertexElement::Type> ElementMaskBits;
	ElementMaskBits.SetBits( ElementMask );

	//	rebuild array of raste elements
	RasterElements.Empty();
	for ( u32 i=0;	i<DefElements.GetSize();	i++ )
	{
		//	don't include this element if it's not in the mask
		if ( !ElementMaskBits[DefElements[i]] )
			continue;

		//	alloc a new element
		TRasterVertexElements* pRasterElement = RasterElements.AddNew();
		if ( !pRasterElement )
			break;

		//	set it from the definition
		if ( !VertexDefinition.GetRasterElement(*pRasterElement,pVertexData,DefElements[i]) )
		{
			RasterElements.RemoveLast();
			continue;
		}
	}
}
*/

//----------------------------------------------------------------------------//
//	setup geometry info from mesh
//----------------------------------------------------------------------------//
void TLRaster::TRasterData::Set(const TLAsset::TMesh& Mesh,TColourMode DesiredColourType)
{
	//	set flags
	if ( Mesh.HasAlpha() )
	{
		if ( m_Material.m_BlendMode == TBlendMode::Opaque )
			m_Material.m_BlendMode = TBlendMode::Alpha;
	}

	//	bind vertex info
	m_VertexCount = Mesh.GetVertexCount();
	
	//	always bind positions
	TInPlaceArray<TVertexElement> VertexElements = GetVertexElements();
	TVertexElement* pVertexElement = VertexElements.AddNew();
	pVertexElement->Set( Mesh.GetVertexes(), TVertexElementType::Position );

	//	alloc elements for uvs
	const TArray<float2>* pUVs = Mesh.GetUVsNotEmpty();
	if ( pUVs && (!pVertexElement || pVertexElement->IsValid()) )	//	alloc another element if the last one was used
		pVertexElement = VertexElements.AddNew();
	if ( pUVs && pVertexElement )
		pVertexElement->Set( *pUVs, TVertexElementType::TexCoord );

	//	alloc colour elements if desired
	if ( DesiredColourType != ColourNone && (!pVertexElement || pVertexElement->IsValid()) )
		pVertexElement = VertexElements.AddNew();
	if ( DesiredColourType != ColourNone && pVertexElement )
	{
		//	try to get desired colour data
		if ( DesiredColourType == Colour24 )		pVertexElement->Set( Mesh.GetColours24NotEmpty(), TVertexElementType::Colour );
		else if ( DesiredColourType == Colour32 )	pVertexElement->Set( Mesh.GetColours32NotEmpty(), TVertexElementType::Colour );
		else if ( DesiredColourType == Colour64 )	pVertexElement->Set( Mesh.GetColours64NotEmpty(), TVertexElementType::Colour );
		else if ( DesiredColourType == ColourF )	pVertexElement->Set( Mesh.GetColoursNotEmpty(), TVertexElementType::Colour );

		//	last chance, just get any colour data
		if ( !pVertexElement->IsValid() )	pVertexElement->Set( Mesh.GetColours24NotEmpty(), TVertexElementType::Colour );
		if ( !pVertexElement->IsValid() )	pVertexElement->Set( Mesh.GetColours32NotEmpty(), TVertexElementType::Colour );
		if ( !pVertexElement->IsValid() )	pVertexElement->Set( Mesh.GetColours64NotEmpty(), TVertexElementType::Colour );
		if ( !pVertexElement->IsValid() )	pVertexElement->Set( Mesh.GetColoursNotEmpty(), TVertexElementType::Colour );
	}

	//	if the last-allocated vertex element is invalid, remove it
	if ( !pVertexElement->IsValid() )
		VertexElements.RemoveLast();

	//	bind primitives
	m_pTriangles	= Mesh.GetTrianglesNotEmpty();
	m_pTristrips	= Mesh.GetTristripsNotEmpty();
	m_pTrifans		= Mesh.GetTrifansNotEmpty();
	m_pLines		= Mesh.GetLinesNotEmpty();
	m_pLinestrips	= Mesh.GetLinestripsNotEmpty();
}

/*

bool TLRaster::TVertexElement::Set(const TLAsset::TVertexDef& VertexDef,const u8* pVertexData,TVertexElement::Type Element)
{
	//	don't have this element
	if ( !VertexDef.HasElement(Element) )
	{
		SetInvalid();
		return false;
	}

	//	calc data offset
	m_pData = pVertexData + VertexDef.m_Info[Element].m_Offset;
	m_Stride = VertexDef.m_Info[Element].m_Stride;
	m_Elements = VertexDef.m_Info[Element].m_Elements;
	m_DataType = VertexDef.m_Info[Element].m_Type;
	return true;
}
*/


//----------------------------------------------------------------------------//
//	initialise default setup
//----------------------------------------------------------------------------//
void TLRaster::TMaterial::Init()
{
	//	yuck! but fast! and simple!
	memset( this, 0, sizeof(*this) );

	//	this is unrequired seeing as the material is a member
	//	m_Material.Init();
}