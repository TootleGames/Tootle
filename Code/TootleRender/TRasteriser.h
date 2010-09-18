/*
 
	Base class for a rasteriser and the generic raster data types

*/
#pragma once

#include <TootleCore/TSubscriber.h>
#include <TootleAsset/TMesh.h>
#include <TootleAsset/TAtlas.h>
#include <TootleAsset/TTexture.h>
#include <TootleRender/TEffect.h>



namespace TLRender
{
	class TScreen;
	class TRenderTarget;
}


namespace TLRaster
{
	class TRasteriser;

	class TRasterData;			//	raster data "packet"
	class TRasterSpriteData;	//	raster data specifically for sprites so different rasterisers can take advantage of hardware-specific sprite stuff
	class TMaterial;			//	runtime material data
	class TVertexElement;		//	runtime vertex reference
	
	namespace TVertexElementType
	{
		enum Type
		{
			Position,
			Normal,
			Colour,
			TexCoord,
			
			_Max,
		};
	}
	
	enum TColourMode
	{
		ColourNone,
		ColourF,
		Colour24,
		Colour32,
		Colour64,
	};

	namespace TBlendMode
	{
		enum Type
		{
			Opaque=0,	//	solid rendering
			Alpha,		//	half + half? I forget, but regularly expected blending and uses alpha channel
			Add,		//	one + one	
		};
	}
};
TLCore_DeclareIsDataType(TLRaster::TRasterData);
TLCore_DeclareIsDataType(TLRaster::TRasterSpriteData);
TLCore_DeclareIsDataType(TLRaster::TVertexElement);
TLCore_DeclareIsDataType(TLRaster::TMaterial);


//--------------------------------------------------
//	material. The ==,!= and < operators should all work on the same values
//--------------------------------------------------
class TLRaster::TMaterial
{
public:
	TMaterial()					{	Init();	}

	void						Init();				//	initialise to default setup

	bool						HasAlpha() const	{	return (m_BlendMode != TBlendMode::Opaque) || m_Colour.IsTransparent() || (m_pTexture && m_pTexture->HasAlphaChannel());	}

	FORCEINLINE bool			operator<(const TMaterial& That) const
	{
		//	put non-alpha's first
		if ( this->HasAlpha() && !That.HasAlpha() )
			return true;
		else if ( !this->HasAlpha() && That.HasAlpha() )
			return false;

		//	sort by texture first
		if ( m_pTexture != That.m_pTexture )
			return m_pTexture < That.m_pTexture;

		//	equal materials
		return false;
	}
	
	FORCEINLINE bool			operator==(const TMaterial& That) const
	{
		return ( m_BlendMode == That.m_BlendMode &&
				m_pTexture == That.m_pTexture &&
				m_Colour == That.m_Colour &&
				m_PointSize == That.m_PointSize &&
				m_LineWidth == That.m_LineWidth );
	}
	
	FORCEINLINE bool			operator!=(const TMaterial& That) const
	{
		return ( m_BlendMode != That.m_BlendMode ||
				m_pTexture != That.m_pTexture ||
				m_Colour != That.m_Colour ||
				m_PointSize != That.m_PointSize ||
				m_LineWidth != That.m_LineWidth );
	}
	
public:
	const TLAsset::TTexture*	m_pTexture;		//	texture
	TColour						m_Colour;		//	accumulated scene colour (usually)
	TBlendMode::Type			m_BlendMode;	//	blend mode
	float						m_PointSize;	//	pixel-size of points on the mesh
	float						m_LineWidth;	//	pixel-thickness of lines on mesh
};



//----------------------------------------------------------------------------//
//	raw raster info for one element of some vertexes
//----------------------------------------------------------------------------//
class TLRaster::TVertexElement
{
public:
	bool							IsValid() const			{	return m_pData.m_void!=NULL;	}
	void							SetInvalid()			{	m_pData.m_void = NULL;	}

//	bool							Set(const TLAsset::TVertexDef& VertexDef,const u8* pVertexData,TLVertexElement::Type Element) const;	//	set up data from a vertex definition
	template<typename TYPE> void	Set(const TArray<TYPE>* pData,TVertexElementType::Type ElementType);	//	old style mesh usage
	template<typename TYPE> void	Set(const TArray<TYPE>& Data,TVertexElementType::Type ElementType)		{	Set( &Data, ElementType );	}

public:
	union
	{
		void const*			m_void;
		float const*		m_float;		
		u8 const*			m_u8;		
		s8 const*			m_s8;		
		u16 const*			m_u16;		
		s16 const*			m_s16;		
		u32 const*			m_u32;		
		s32 const*			m_s32;		
		TColour24 const*	m_TColour24;		
		TColour32 const*	m_TColour32;		
		TColour const*		m_TColour;		
	} m_pData;						//	pointer to the data in acceptable pod formats
	
	TVertexElementType::Type	m_ElementType;	//	type of element - position, normal etc
	u8							m_Stride;		//	gap between element-end and next element start
	TRef						m_DataType;		//	type, eg. float3
};


//----------------------------------------------------------------------------//
//	old style mesh usage
//----------------------------------------------------------------------------//
template<typename TYPE>
void TLRaster::TVertexElement::Set(const TArray<TYPE>* pData,TVertexElementType::Type ElementType)
{
	m_pData.m_void = (pData && pData->GetSize()) ? pData->GetData() : NULL;
	m_ElementType = ElementType;
	m_Stride = 0;
	m_DataType = TLBinary::GetDataTypeRef<TYPE>();
}


//----------------------------------------------------------------------------//
//	final raw raster data, rasteriser-specific
//----------------------------------------------------------------------------//
class TLRaster::TRasterData
{
public:
	class Flags
	{
	public:
		enum Type
		{
			None			= 0,
			Default			= None,	//	none is the default as this matches 0

			Wireframe		= 1<<0,	//	render outlines
			NoDepthRead		= 1<<3,	//	read depth
			NoDepthWrite	= 1<<4,	//	write depth
			DrawPoints		= 1<<5,	//	draw vertex positions as regular points
			PointSprites	= 1<<6,	//	draw vertex positions as point sprite
		};
	};

public:
	TRasterData()												{	Init();	}
	void		Init();											//	initialise default setup
	void		SetMaterial(const TMaterial& Material);			//	set material
	void		SetWireframe(bool ResetColour=true);			//	setup raster data to render wireframe data
	void		SetDebug();										//	setup raster data as debug data
//	void		Set(const TLAsset::TVertexDef& VertexDefinition,const u8* pVertexData,u32 ElementMask=0xffffffff);	//	setup geometry from a vertex definition
	void		Set(const TLAsset::TMesh& Mesh,TColourMode DesiredColourType);	//	setup geometry info from mesh
	void		SetDepthRead(bool DepthRead)					{	DepthRead ? m_Flags.Clear(Flags::NoDepthRead) : m_Flags.Set(Flags::NoDepthRead);	}
	void		SetTransformNone()								{	m_Transform.SetInvalid();	}
	void		SetTransform(const TLMaths::TTransform& Transform)	{	m_Transform = Transform;	}
	
	TInPlaceArray<TVertexElement>		GetVertexElements()			{	return TInPlaceArray<TVertexElement>( m_VertexElements, sizeofarray(m_VertexElements), &m_VertexElementCount );	}
	const TInPlaceArray<TVertexElement>	GetVertexElements() const	{	return TInPlaceArray<TVertexElement>( m_VertexElements, sizeofarray(m_VertexElements), &m_VertexElementCount );	}
	
	FORCEINLINE bool			operator<(const TRasterData& That) const
	{
		//	sort by material
		if ( m_Material != That.m_Material )
			return m_Material < That.m_Material;
		
		//	sort by something else, vertex data etc
		return false;
	}
	FORCEINLINE bool			operator==(const TRasterData& That) const	{	return this == &That;	}	//	gr: don't think a proper contents check is required...

	
public:
	TLMaths::TTransform				m_Transform;	//	transform for geometry
	TMaterial						m_Material;		//	material info
	TFlags<Flags::Type>						m_Flags;		//	render flags
	const TPtrArray<TLRender::TEffect>*		m_pEffects;		//	effects

	//	vertex data
	u32								m_VertexCount;
	TVertexElement					m_VertexElements[TLRaster::TVertexElementType::_Max];	
	u32								m_VertexElementCount;

	//	primitive data
	const TArray<TLAsset::TMesh::Triangle>*		m_pTriangles;
	const TArray<TLAsset::TMesh::Tristrip>*		m_pTristrips;
	const TArray<TLAsset::TMesh::Trifan>*		m_pTrifans;
	const TArray<TLAsset::TMesh::Line>*			m_pLines;
	const TArray<TLAsset::TMesh::Linestrip>*	m_pLinestrips;
};




//----------------------------------------------------------------------------//
//	raster data for sprites
//	gr: note: because of the new interleaved vertexes we could use this as the vertex 
//		and just skip over the material info. Though this might be expensive to send all 
//		that additional data around when the vertexes want to be packed as much as possible
//		maybe the material could be a reference or a pointer for storage. but it would save 
//		a load of memcpying when turning them into rasterdata's
//----------------------------------------------------------------------------//
class TLRaster::TRasterSpriteData
{
public:
	inline bool				operator<(const TRasterSpriteData& That) const	{	return m_Material < That.m_Material;	}
	FORCEINLINE bool		operator==(const TRasterSpriteData& That) const	{	return this == &That;	}	//	gr: don't think a proper contents check is required...

public:
	TMaterial				m_Material;
	TLAsset::TSpriteGlyph	m_Sprite;
};


/*
//----------------------------------------------------------------------------//
//	raster data for particles
//	todo: work this out. But should be seperate raster data for platform optimisation
//		maybe make each bit of data a block of N particles and then join them together in
//		a pool like the sprites
//----------------------------------------------------------------------------//
class TRasterParticleData
{
public:
	TInPlaceArray<float3>	GetArray()	{	return TInPlaceArray<float3>( m_ParticlePositions, 256, &m_ParticleCount );	}

private:
	TRasterMaterial		m_Material;
	float3				m_ParticlePositions[256];
	u8					m_ParticleCount;
};
TLDeclare_IsDataType(TRasterParticleData);

*/


class TLRaster::TRasteriser : public TLMessaging::TSubscriber
{
public:
	TRasteriser()			{}
	virtual ~TRasteriser()	{}
	
	virtual bool		Initialise();
	virtual void		Shutdown();
	virtual TRefRef		GetSubscriberRef() const		{	static TRef Ref="Raster";	return Ref;	}

	virtual void		Rasterise(TLRender::TRenderTarget& RenderTarget,const TLRender::TScreen& Screen)=0;		//	real renderer, rasterises everything to this render target

	virtual void		Render(const TArray<TRasterData>& Data)=0;				//	render this data
	virtual void		Render(const TArray<TRasterSpriteData>& Data)=0;		//	render this sprite data
	virtual void		StoreTemporaryMeshes(const TPtrArray<TLAsset::TMesh>& Meshes)=0;	//	hold onto this temporary mesh, just for this render pass

protected:
	virtual void		OnTextureDeleted(TRefRef TextureRef)=0;			//	clean up after a texture has been deleted
	virtual void		OnTextureChanged(TRefRef TextureRef)=0;			//	update graphics of a texture when the asset changes

	virtual void		ProcessMessage(TLMessaging::TMessage& Message);
};


