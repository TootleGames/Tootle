#include "OpenglRasteriser.h"
#include "TLOpengl.h"
#include <TootleRender/TCamera.h>
#include <TootleRender/TRenderTarget.h>
#include <TootleRender/TScreen.h>


//----------------------------------------------------------------------------//
//	init rasteriser and opengl system
//----------------------------------------------------------------------------//
bool TLRaster::OpenglRasteriser::Initialise()
{
	//	base init
	if ( !TRasteriser::Initialise() )
		return false;

	//	init opengl system
	if ( !Opengl::Init() )
		return false;

	//	create fixed function shader asset factory
	//	gr: todo: move to global opengl
	if ( !m_pShaderAssetFactory )
	{
		//	need to wait for the asset manager
		if ( !TLAsset::g_pManager )
			return false;
	
		m_pShaderAssetFactory = new Opengl::TShaderAssetFactory;
		TLAsset::g_pManager->AddAssetFactory( m_pShaderAssetFactory );

		//	pre-create assets
		//	gr: change this to actually create the assets and then add them to the asset directory
		TPtr<TLAsset::TAsset>& pShader = TLAsset::CreateAsset( TLAsset::TShader_TextureMatrix::GetShaderInstanceRef(), TLAsset::TShader::GetAssetType_Static() );
		if ( pShader )
			pShader->SetLoadingState( TLAsset::LoadingState_Loaded );
	}
	
	return true;
}


//----------------------------------------------------------------------------//
//	
//----------------------------------------------------------------------------//
void TLRaster::OpenglRasteriser::Shutdown()
{
	m_pShaderAssetFactory = NULL;
	Opengl::Shutdown();
	
	TRasteriser::Shutdown();
}


void TLRaster::OpenglRasteriser::OnTextureDeleted(TRefRef TextureRef)
{
	Opengl::DestroyTexture( TextureRef );
}



void TLRaster::OpenglRasteriser::OnTextureChanged(TRefRef TextureRef)
{
	//	gr: delete the texture so it's re-uploaded on next bind.
	//	todo: just update contents of the texture!
	Opengl::DestroyTexture( TextureRef );
}




//----------------------------------------------------------------------------//
//	setup projection camera
//----------------------------------------------------------------------------//
void TLRaster::OpenglRasteriser::SetCamera(TLRender::TRenderTarget& RenderTarget,TLRender::TProjectCamera& Camera)
{
	//	setup camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//	setup camera
	TLMaths::TTransform CameraTransform;
	CameraTransform.SetTranslate( Camera.GetPosition() * -1.f );
	
	//	apply look at matrix (rotate)
	const TLMaths::TMatrix& LookAtMatrix = Camera.GetCameraLookAtMatrix();	
	Opengl::SceneTransform( CameraTransform, &LookAtMatrix );
}


//----------------------------------------------------------------------------//
//	setup orthographic camera
//----------------------------------------------------------------------------//
void TLRaster::OpenglRasteriser::SetCamera(TLRender::TRenderTarget& RenderTarget,TLRender::TOrthoCamera& Camera)
{
	//	setup camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//	translate
	TLMaths::TTransform CameraTransform;
	CameraTransform.SetTranslate( Camera.GetPosition() );
	
	//	apply look at matrix (rotate)
	//	const TLMaths::TMatrix& LookAtMatrix = Camera.GetCameraLookAtMatrix();
	//	m_CameraTransform.SetMatrix( LookAtMatrix );
	Opengl::SceneTransform( CameraTransform );
}


//----------------------------------------------------------------------------//
//	setup the projection/frustum for a projection camera
//----------------------------------------------------------------------------//
void TLRaster::OpenglRasteriser::SetFrustum(const TLRender::TProjectCamera& Camera,const TLRender::TScreen& Screen)
{
	//	get the camera
	//	init projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	//	get view box
	const TLMaths::TBox2D& ScreenViewBox = Camera.GetScreenViewBox();
	
	//	set projection matrix
	Opengl::Platform::SetFrustumProjection( ScreenViewBox, Camera.GetNearZ(), Camera.GetFarZ() );
	
	//	rotate the view matrix so that UP is properly relative to the new screen
	//	gr: another "thing what is backwards" - as is the -/+ of the shape rotation....
	float ProjectionRotationDeg = -Camera.GetCameraRoll().GetDegrees();
	
	if ( Screen.GetScreenShape() == TLRender::ScreenShape_WideRight )
		ProjectionRotationDeg -= 90.f;
	else if ( Screen.GetScreenShape() == TLRender::ScreenShape_WideLeft )
		ProjectionRotationDeg += 90.f;
	
	//	roll around z AFTER setting the frustm
	Opengl::SceneRotate( TLMaths::TAngle(ProjectionRotationDeg), float3( 0.f, 0.f, 1.f ) );
}


//----------------------------------------------------------------------------//
//	setup the projection/frustum for a projection camera
//----------------------------------------------------------------------------//
void TLRaster::OpenglRasteriser::SetFrustum(const TLRender::TOrthoCamera& Camera,const TLRender::TScreen& Screen)
{
	//	setup ortho projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	//	get ortho coordinates box (same as world extents)
	const TLMaths::TBox2D& OrthoBox = Camera.GetOrthoCoordinateBox();
	
	//	rotate the view matrix so that UP is properly relative to the new screen
	//	gr: another "thing what is backwards" - as is the -/+ of the shape rotation....
	float ProjectionRotationDeg = -Camera.GetCameraRoll().GetDegrees();
	
	if ( Screen.GetScreenShape() == TLRender::ScreenShape_WideRight )
		ProjectionRotationDeg -= 90.f;
	else if ( Screen.GetScreenShape() == TLRender::ScreenShape_WideLeft )
		ProjectionRotationDeg += 90.f;
	
	//	roll around z BEFORE we set the frustum
	Opengl::SceneRotate( TLMaths::TAngle(ProjectionRotationDeg), float3( 0.f, 0.f, 1.f ) );
	
	//	set the world coordinates
	Opengl::Platform::SetFrustumOrthographic( OrthoBox, Camera.GetNearZ(), Camera.GetFarZ() );
	
	Opengl::Debug_CheckForError();		
}	

//----------------------------------------------------------------------------//
//	real renderer, rasterises everything to this render target. 
//----------------------------------------------------------------------------//
void TLRaster::OpenglRasteriser::Rasterise(TLRender::TRenderTarget& RenderTarget,const TLRender::TScreen& Screen)
{
	//	setup viewports/scissoring etc
	if ( !BeginRasterise( RenderTarget, Screen ) )
		return;

	//	turn sprites into raster data
	ResolveSprites();

	//	sort the raster data for batch-friendlyness, z sorting etc
	m_RasterData.Sort();

	//	render!
	for ( u32 i=0;	i<m_RasterData.GetSize();	i++ )
	{
		Rasterise( m_RasterData[i] );
	}

	//	cleanup
	Flush();

	//	end viewport/scissoring stuff
	EndRasterise( RenderTarget, Screen );
}


//----------------------------------------------------------------------------//
//	setup viewports etc
//----------------------------------------------------------------------------//
bool TLRaster::OpenglRasteriser::BeginRasterise(TLRender::TRenderTarget& RenderTarget,const TLRender::TScreen& Screen)
{
	TLRender::TCamera& Camera = *RenderTarget.GetCamera();

	//	get viewport
	const TLMaths::TBox2D& ViewportBox = Camera.GetViewportBox();
	const TLMaths::TBox2D& ScissorBox = Camera.GetScissorBox();
	if ( !ViewportBox.IsValid() || !ScissorBox.IsValid() )
	{
		TLDebug_Break("Viewport and scissor boxes should be valid");
		return false;
	}

	//	setup viewport
	glViewport( (s32)ViewportBox.GetLeft(), (s32)ViewportBox.GetTop(), (s32)ViewportBox.GetWidth(), (s32)ViewportBox.GetHeight() );
	Opengl::Debug_CheckForError();

	Opengl::EnableScissor( true );
	Opengl::SetScissor( (u32)ScissorBox.GetLeft(), (u32)ScissorBox.GetTop(), (u32)ScissorBox.GetWidth(), (u32)ScissorBox.GetHeight() );
	Opengl::Debug_CheckForError();

	//	do projection vs orthographic setup
	if ( Camera.IsOrtho() )
	{
		SetFrustum( static_cast<TLRender::TOrthoCamera&>(Camera), Screen );
		SetCamera( RenderTarget, static_cast<TLRender::TOrthoCamera&>(Camera) );
	}
	else
	{
		SetFrustum( static_cast<TLRender::TProjectCamera&>(Camera), Screen );
		SetCamera( RenderTarget, static_cast<TLRender::TProjectCamera&>(Camera) );
	}

	//	enable/disable antialiasing
	Opengl::EnableAntiAliasing( RenderTarget.GetFlag( TLRender::TRenderTarget::Flag_AntiAlias ) );

	/*	gr: unneccesary?
	 //	set the clear depth
	 float ClearDepth = GetCamera()->GetFarZ();
	 TLMaths::Limit( ClearDepth, 0.f, 1.f );
	 glClearDepth( ClearDepth );
	 glDepthFunc( GL_LEQUAL );
	 */	 
	
	//	clear render target (viewport has been set)
	bool ClearColour = RenderTarget.GetFlag( TLRender::TRenderTarget::Flag_ClearColour );
	bool ClearDepth = RenderTarget.GetFlag( TLRender::TRenderTarget::Flag_ClearDepth );
	bool ClearStencil = RenderTarget.GetFlag( TLRender::TRenderTarget::Flag_ClearStencil );
	Opengl::Clear( RenderTarget.GetClearColour(), ClearColour, ClearDepth, ClearStencil ); 

	Opengl::Debug_CheckForError();

	return true;	
}


//----------------------------------------------------------------------------//
//	
//----------------------------------------------------------------------------//
void TLRaster::OpenglRasteriser::EndRasterise(TLRender::TRenderTarget& RenderTarget,const TLRender::TScreen& Screen)
{
}


//----------------------------------------------------------------------------//
//	clear all the data we used for rendering
//----------------------------------------------------------------------------//
void TLRaster::OpenglRasteriser::Flush()
{
	m_RasterData.Empty(false);	
	m_RasterSpriteData.Empty(false);	
	m_TempMeshes.Empty(true);	
}


//----------------------------------------------------------------------------//
//	rasterise a single item
//----------------------------------------------------------------------------//
void TLRaster::OpenglRasteriser::Rasterise(const TRasterData& Data)
{
	//	no vertexes... maybe allow this to render if the effect produces a vertex buffer
	if ( Data.GetVertexCount() == 0 )
		return;
	
	//	pre-process effects
	u32 EffectCount = Data.m_pEffects ? Data.m_pEffects->GetSize() : 0;
	for ( u32 e=0;	e<EffectCount;	e++ )
	{
		const TPtrArray<TLRender::TEffect>& Effects = *Data.m_pEffects;
		TLRender::TEffect& Effect = *Effects[e];
		if ( !Effect.PreRender() )
			return;
	}
	
//#define DEBUG_WIREFRAME

	//	set scene transform
	glPushMatrix();
	Opengl::SceneTransform( Data.m_Transform );
	Opengl::Debug_CheckForError();
	
	//	bind material
#if defined(DEBUG_WIREFRAME)
	Opengl::BindTexture( TRef() );
#else
	Opengl::BindTexture( Data.m_Material.m_Texture );
#endif
	Opengl::SetLineWidth( Data.m_Material.m_LineWidth );
	Opengl::Debug_CheckForError();

	//	bind flags
	Opengl::EnableWireframe( (Data.m_Flags & TRasterData::Flags::Wireframe ) != 0x0 );
#if defined(DEBUG_WIREFRAME)
	Opengl::EnableWireframe( true );
#endif
	Opengl::EnableDepthRead( (Data.m_Flags & TRasterData::Flags::NoDepthRead ) == 0x0 );
	Opengl::EnableDepthWrite( (Data.m_Flags & TRasterData::Flags::NoDepthWrite ) == 0x0 );
	Opengl::Debug_CheckForError();

	//	hacky method to unbind elements for now
	bool BoundPositionElement = false;
	bool BoundNormalElement = false;
	bool BoundColourElement = false;
	bool BoundTexCoordElement = false;

	//	bind vertex data
	TInPlaceArray<TVertexElement> VertexElements = Data.GetVertexElements();
	for ( u32 v=0;	v<VertexElements.GetSize();	v++ )
	{
		TVertexElement& VertexElement = VertexElements[v];
		if ( !Opengl::BindVertexElement( VertexElement ) )
			continue;
		
		BoundPositionElement |= ( VertexElement.m_Member.m_Ref == TLRaster::TVertexElementType::Position );
		BoundNormalElement |= ( VertexElement.m_Member.m_Ref == TLRaster::TVertexElementType::Normal );
		BoundColourElement |= ( VertexElement.m_Member.m_Ref == TLRaster::TVertexElementType::Colour );
		BoundTexCoordElement |= ( VertexElement.m_Member.m_Ref == TLRaster::TVertexElementType::TexCoord );
	}

#if defined(DEBUG_WIREFRAME)
	BoundColourElement = false;
	BoundTexCoordElement = false;
#endif
		
	//	unbind elements we didn't bind
	if ( !BoundPositionElement )	
		Opengl::Unbind( TLRaster::TVertexElementType::Position );

	if ( !BoundNormalElement )	
		Opengl::Unbind( TLRaster::TVertexElementType::Normal );
	
	if ( !BoundColourElement )	
		Opengl::Unbind( TLRaster::TVertexElementType::Colour );
	
	if ( !BoundTexCoordElement )	
		Opengl::Unbind( TLRaster::TVertexElementType::TexCoord );

	//	bind colour and alpha settings
	Opengl::SetBlendMode( Data.m_Material.m_BlendMode );
	Opengl::SetSceneColour( Data.m_Material.m_Colour );
	Opengl::Debug_CheckForError();


	//	draw primitives
	if ( Data.m_pTriangles )	Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeTriangle(), Data.m_pTriangles.GetArray() );
	if ( Data.m_pTristrips )	Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeTristrip(), Data.m_pTristrips.GetArray() );
	if ( Data.m_pTrifans )		Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeTrifan(), Data.m_pTrifans.GetArray() );
	if ( Data.m_pLinestrips )	Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeLinestrip(), Data.m_pLinestrips.GetArray() );
	if ( Data.m_pLines )		Opengl::DrawPrimitives( Opengl::Platform::GetPrimTypeLine(), Data.m_pLines.GetArray() );
	Opengl::Debug_CheckForError();

#if defined(DEBUG_WIREFRAME)
	if ( true )
#else
	if ( Data.m_Flags & TRasterData::Flags::DrawPoints )
#endif
	{
		Opengl::EnablePointSprites(false);
	//	Opengl::SetPointSize( Data.m_Material.m_PointSize );
#if defined(DEBUG_WIREFRAME)
		Opengl::SetPointSize( 6.f );
#endif
		Opengl::Debug_CheckForError();
		Opengl::DrawPrimitivePoints( Data.GetVertexCount() );
		Opengl::Debug_CheckForError();
	}

	if ( Data.m_Flags & TRasterData::Flags::PointSprites && Data.m_Material.m_Texture.IsValid() )
	{
		Opengl::EnablePointSprites(true);
		Opengl::EnablePointSizeUVMapping(true);
		float PointSize = Data.m_Material.m_PointSize;
		Opengl::ClampPointSpriteSize( PointSize );
		Opengl::SetPointSize( PointSize );
		Opengl::DrawPrimitivePoints( Data.GetVertexCount() );
		Opengl::EnablePointSizeUVMapping(false); 
		Opengl::Debug_CheckForError();
	}

	//	finish effects
	for ( u32 e=0;	e<EffectCount;	e++ )
	{
		const TPtrArray<TLRender::TEffect>& Effects = *Data.m_pEffects;
		TLRender::TEffect& Effect = *Effects[e];
		Effect.PostRender();
	}
	
	glPopMatrix();
	Opengl::Debug_CheckForError();
}



bool TLRaster::OpenglRasteriser::AllocSpriteTriangles(u32 MaxSpriteCount)
{
	//	need this many triangles
	u32 TriangleCount = MaxSpriteCount * 2;

	//	have enough already
	if ( TriangleCount <= m_SpriteTriangles.GetSize() )
		return true;

	//	pre-alloc
	if ( !m_SpriteTriangles.SetAllocSize( TriangleCount ) )
	{
		TDebugString Debug_String;
		Debug_String << "Not enough memory for " << MaxSpriteCount << " sprites!";
		TLDebug_Break(Debug_String);
		return false;
	}

	//	add triangles by-sprite
	for ( u32 s=m_SpriteTriangles.GetSize()/2;	s<MaxSpriteCount;	s++ )
	{
		//	vertex index of the sprite's first vertex
		u32 VertexIndex = s * 4;

		//	add first triangle
		Type3<u16>& TriangleTop = *m_SpriteTriangles.AddNew();
		TriangleTop.x = VertexIndex+0;	//	top left
		TriangleTop.y = VertexIndex+1;	//	top right
		TriangleTop.z = VertexIndex+2;	//	bottom right

		Type3<u16>& TriangleBottom = *m_SpriteTriangles.AddNew();
		TriangleBottom.x = VertexIndex+2;	//	bottom right
		TriangleBottom.y = VertexIndex+3;	//	bottom left
		TriangleBottom.z = VertexIndex+0;	//	top left
	}

	return true;
}



//----------------------------------------------------------------------------//
//	turn sprites into normal raster data
//----------------------------------------------------------------------------//
void TLRaster::OpenglRasteriser::ResolveSprites()
{
	m_Sprites.Empty();
	u32 SpriteCount = m_RasterSpriteData.GetSize();
	if ( SpriteCount == 0 )
		return;

	//	pre-alloc the triangle buffer. As we share this with all the raster datas we cannot allow
	//	the allocated address to change, so we have to pre-alloc ALL the potential triangles early
	if ( !AllocSpriteTriangles(SpriteCount) )
	{
		//	not enough mem for this many sprites
		return;
	}

	//	pre-alloc the sprite buffer. As we share this with all the raster datas we cannot allow
	//	the allocated address to change
	if ( !m_Sprites.SetAllocSize( SpriteCount ) )
	{
		//	not enough mem for this many sprites
		return;
	}

	//	sort sprites by material so they batch better
	m_RasterSpriteData.Sort();

	//	get vertex definition for sprites
	const TLStruct::TDef& SpriteVertexDefinition = TLAsset::TSpriteVertex::GetVertexDef();

	//	store prev material so we know when to split a new batch
	TMaterial* pLastMaterial = NULL;
	TRasterData* pRasterData = NULL;


	for ( u32 s=0;	s<SpriteCount;	s++ )
	{
		TRasterSpriteData& RasterSprite = m_RasterSpriteData[s];
		
		//	add sprite to the array
		s32 NewSpriteDataIndex = m_Sprites.Add( RasterSprite.m_Sprite );

		//	if this is a new batch then make a new raster data
		if ( !pLastMaterial || !(*pLastMaterial == RasterSprite.m_Material) )
		{
			pRasterData = m_RasterData.AddNew();
			//	out of mem
			if ( !pRasterData )
				break;
			
			//	setup raster data
			pRasterData->Init();
			const TLAsset::TSpriteGlyph& FirstSprite = m_Sprites[NewSpriteDataIndex];
			pRasterData->Set( SpriteVertexDefinition, reinterpret_cast<const u8*>( &FirstSprite ) );
			pRasterData->SetMaterial( RasterSprite.m_Material );
			pLastMaterial = &RasterSprite.m_Material;
			
			//	point the triangle geometry at the first triangle for batch... which is always zero
			u32 TriangleIndex = 0;
			const TLAsset::TMesh::Triangle& SpriteTriangle = m_SpriteTriangles[TriangleIndex];
			//const TLAsset::TMesh::Triangle& SpriteTriangle2 = m_SpriteTriangles[TriangleIndex + 1];
			pRasterData->m_pTriangles = TArrayInPlace<TLAsset::TMesh::Triangle>( &SpriteTriangle, 0 ); 
		}
		
		//	increase number of vertexes used in this raster packet
		pRasterData->m_VertexCount += RasterSprite.GetVertexCount();

		//	and number of triangles
		pRasterData->m_pTriangles.m_Size += 2;
	}

}






//-------------------------------------------------------
//	custom shader creation
//-------------------------------------------------------
TLAsset::TAsset* Opengl::TShaderAssetFactory::CreateObject(TRefRef InstanceRef,TRefRef TypeRef)
{
	//	specifically create texture matrix shader
	if ( InstanceRef == TLAsset::TShader_TextureMatrix::GetShaderInstanceRef() && TypeRef == TLAsset::TShader::GetAssetType_Static() )
		return new TLAsset::TShader_TextureMatrix( InstanceRef );
	
	return NULL;
}


TLAsset::TShader_TextureMatrix::TShader_TextureMatrix(TRefRef AssetRef) :
	TLAsset::TShader	( AssetRef )
{
}


//-------------------------------------------------------
//	pre-render, setup the texture matrix as dictated by data
//-------------------------------------------------------
Bool TLAsset::TShader_TextureMatrix::PreRender(TBinaryTree& ShaderData)	
{
	//	read transform from shader data
	TLMaths::TTransform Transform;
	if ( Transform.ImportData( ShaderData ) != TLMaths_TransformBitNone )
		Opengl::TextureTransform( Transform );

	return true;	
}


//-------------------------------------------------------
//	post-render, restore default texture matrix
//-------------------------------------------------------
void TLAsset::TShader_TextureMatrix::PostRender()
{
	TLMaths::TTransform Transform;
	Opengl::TextureTransform( Transform );
}


