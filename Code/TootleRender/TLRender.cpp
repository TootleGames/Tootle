#include "TLRender.h"



#ifdef _DEBUG
	#define DEBUG_CHECK_PRIMITIVE_INDEXES	//	check indexes of all the primitives are within bound vertex range
#endif


//	gr: added this to check if it was the cause of a crash on iphone, but it's not. Colour format was the problem
//#define REBIND_DATA	//	if defined then we re-bind data even if its the same pointer as was used before

namespace TLRender
{
	u32							g_PolyCount;
	u32							g_VertexCount;

	namespace Opengl
	{
		const TArray<float3>*		g_pBoundVertexes = NULL;
		//const TArray<TColour>*		g_pBoundColours = NULL;
		const void*					g_pBoundColours = NULL;
		const TArray<float2>*		g_pBoundUVs = NULL;
		u32							g_BoundTextureIndex = 0;						//	currently bound texture
		TKeyArray<TRef,u32>			g_TextureIndexes;								//	Texture Asset Ref -> gl Texture index lookup table

		void				GetOpenglFilterTypes(TArray<u32>& OpenglFilters,Bool MinLinearFilter,Bool MagLinearFilter,Bool MipMapEnabled);	//	get opengl filter type from asset filter type
		void				BindTextureIndex(u32 TextureIndex);					//	set current texture - no change if same as currently bound texture
		void				UnbindTextureIndex(u32 TextureIndex);				//	unbind texture index if it's currently bound
		u32					UploadTexture(const TLAsset::TTexture& Texture);		//	generate opengl texture ID for this asset - if a texture already has an entry it will be destroyed and re-created
		void				DestroyTexture(u32& TextureIndex);					//	destroy opengl texture
		u32					GetTextureIndex(TRefRef TextureRef);				//	search the texture lookup table for the correct texture ID for this asset
	}
};


//-----------------------------------------------------------
//	draw vertexes as points
//-----------------------------------------------------------
void TLRender::Opengl::DrawPrimitivePoints(const TArray<float3>* pVertexes)
{
	//	have a static array of indexes and grow it as required
	static TArray<u16> g_AllPoints;

	//	grow index array as required
	if ( g_AllPoints.GetSize() < pVertexes->GetSize() )
	{
		u32 OldSize = g_AllPoints.GetSize();
		
		//	alloc points
		g_AllPoints.SetSize( pVertexes->GetSize() );

		//	set new entries
		for ( u32 i=OldSize;	i<g_AllPoints.GetSize();	i++ )
			g_AllPoints[i] = i;
	}

	//	draw points
	DrawPrimitives( Platform::GetPrimTypePoint(), pVertexes->GetSize(), g_AllPoints.GetData() );
}



//---------------------------------------------------
//	unbind everything
//---------------------------------------------------
void TLRender::Opengl::Unbind()
{
	//	unbind attribs
	BindVertexes( NULL );
	BindColoursNull();
	BindUVs( NULL );
	BindTexture( NULL );
}


//---------------------------------------------------
//	transform scene
//---------------------------------------------------
void TLRender::Opengl::SceneTransform(const TLMaths::TTransform& Transform,const TLMaths::TMatrix* pMatrix)
{
	//	gr: do this in the same order as the Transform() functions?
	//		currently in old-render-code-order
	if ( Transform.HasTranslate() )
	{
		const float3& Trans = Transform.GetTranslate();
		glTranslatef( Trans.x, Trans.y, Trans.z );
	}

	if ( Transform.HasScale() )
	{
		const float3& Scale = Transform.GetScale();
		glScalef( Scale.x, Scale.y, Scale.z );
	}

	if ( pMatrix )
	{
		glMultMatrixf( pMatrix->GetData() );
	}

	if ( Transform.HasRotation() )
	{
		TLMaths::TMatrix RotMatrix;
		TLMaths::QuaternionToMatrix( Transform.GetRotation(), RotMatrix );
		glMultMatrixf( RotMatrix.GetData() );

		// Test using getting the Euler angles from the Quaternion rather than converting the quaternion to a matrix
		//float3 vRot = Transform.GetRotation().GetEuler();
		//glRotatef(vRot.x, 1, 0, 0);
		//glRotatef(vRot.y, 0, 1, 0);
		//glRotatef(vRot.z, 0, 0, 1);
		
		// Alternative version using angle axis from quaternion
		// float4 vRot = Transform.GetRotation().GetAxisAngle();
		//glRotate(vRot.w, vRot.x, vRot.y, vRot.z);
	}
}


//---------------------------------------------------
//	eular rotation on the scene - wrapper for glRotatef
//---------------------------------------------------
void TLRender::Opengl::SceneRotate(const TLMaths::TAngle& Rotation,const float3& Axis)
{
	glRotatef( Rotation.GetDegrees(), Axis.x, Axis.y, Axis.z );
}


//---------------------------------------------------
//	get render target's viewport size from the size and the screen size
//---------------------------------------------------
void TLRender::Opengl::GetViewportSize(Type4<s32>& ViewportSize,const Type4<s32>& ViewportTargetMaxSize,const Type4<s32>& RenderTargetSize,const Type4<s32>& RenderTargetMaxSize,TScreenShape ScreenShape)
{
	//	rotate render target size to be in "viewport" space
	Type4<s32> RotatedRenderTargetSize = RenderTargetSize;

	if ( ScreenShape == ScreenShape_WideLeft )
	{
		//	gr: rendertarget is rotated left, so to get viewport, rotate it right again
		//	rotate right
		RotatedRenderTargetSize.x = RenderTargetSize.Top();
		RotatedRenderTargetSize.y = RenderTargetMaxSize.Width() - RenderTargetSize.Right();
		RotatedRenderTargetSize.Width() = RenderTargetSize.Height();
		RotatedRenderTargetSize.Height() = RenderTargetSize.Width();
	}
	else if ( ScreenShape == ScreenShape_WideRight )
	{
		//	gr: rendertarget is rotated right, so to get viewport, rotate it left again
		//	rotate left
		RotatedRenderTargetSize.x = RenderTargetMaxSize.Height() - RenderTargetSize.Bottom();
		RotatedRenderTargetSize.y = RenderTargetSize.Left();
		RotatedRenderTargetSize.Width() = RenderTargetSize.Height();
		RotatedRenderTargetSize.Height() = RenderTargetSize.Width();
	}

	//	position for opengl viewport
	//	0,0 is bottom left, next two sizes in Scissor() and Viewport() are still widht and height, just upside down
	ViewportSize.Left() = RotatedRenderTargetSize.Left();
	ViewportSize.Top() = ViewportTargetMaxSize.Height() - RotatedRenderTargetSize.Top() - RotatedRenderTargetSize.Height();

	//	no change in dimensions
	ViewportSize.Width() = RotatedRenderTargetSize.Width();
	ViewportSize.Height() = RotatedRenderTargetSize.Height();
}



//---------------------------------------------------
//	get opengl filter type from asset filter type
//---------------------------------------------------
void TLRender::Opengl::GetOpenglFilterTypes(TArray<u32>& OpenglFilters,Bool MinLinearFilter,Bool MagLinearFilter,Bool MipMapEnabled)
{
	//	min is simple case
	OpenglFilters.Add( MinLinearFilter ? GL_LINEAR : GL_NEAREST );

	//	if mip map is enabled then use special case mip map filters for mag
	if ( MipMapEnabled )
	{
		TLDebug_Break("work out mip map filters");
		//	GL_NEAREST_MIPMAP_NEAREST
		//	GL_NEAREST_MIPMAP_LINEAR
		//	GL_LINEAR_MIPMAP_NEAREST
		//	GL_LINEAR_MIPMAP_LINEAR
	}

	//	simple mag case
	OpenglFilters.Add( MagLinearFilter ? GL_LINEAR : GL_NEAREST );
}


//---------------------------------------------------
//	set current texture - no change if same as currently bound texture
//---------------------------------------------------
void TLRender::Opengl::BindTextureIndex(u32 TextureIndex)
{
	if ( TextureIndex != g_BoundTextureIndex )
	{
		glBindTexture( GL_TEXTURE_2D, TextureIndex );
		g_BoundTextureIndex = TextureIndex;
	}
}

//---------------------------------------------------
//	unbind texture index if it's currently bound
//---------------------------------------------------
void TLRender::Opengl::UnbindTextureIndex(u32 TextureIndex)
{
	if ( TextureIndex == g_BoundTextureIndex )
	{
		glBindTexture( GL_TEXTURE_2D, 0 );
		g_BoundTextureIndex = 0;
	}
}


//---------------------------------------------------
//	get the existing texture ID for a texture
//---------------------------------------------------
u32 TLRender::Opengl::GetTextureIndex(TRefRef TextureRef)
{
	u32* pTextureIndex = g_TextureIndexes.Find( TextureRef );
	return pTextureIndex ? *pTextureIndex : 0;
}


//---------------------------------------------------
//	generate opengl texture ID for this asset - if a texture already has an entry it will be destroyed and re-created
//---------------------------------------------------
u32 TLRender::Opengl::UploadTexture(const TLAsset::TTexture& Texture)
{
	//	find existing entry - if it exists destroy it and create another
	u32* pTextureIndex = g_TextureIndexes.Find( Texture.GetAssetRef() );

	//	already exists, destroy
	if ( pTextureIndex )
	{
		DestroyTexture( *pTextureIndex );
		g_TextureIndexes.Remove( Texture.GetAssetRef() );
		pTextureIndex = NULL;
	}

	//	Create texture
	u32 NewTextureIndex = 0;

	//	allocate index
	glGenTextures( 1, &NewTextureIndex );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	BindTextureIndex( NewTextureIndex );
	
	//	get filter modes
	TFixedArray<u32,2> OpenglFilters;
	GetOpenglFilterTypes( OpenglFilters, Texture.IsMinFilterLinear(), Texture.IsMagFilterLinear(), Texture.IsMipMapEnabled() );

	//	set filter params
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenglFilters[0] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenglFilters[1] );

	//	clamp
	if ( Texture.IsClampEnabled() )
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	}	
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	}	

	//	generate texture
	u32 SourceColourFormat = Texture.HasAlphaChannel() ? GL_RGBA : GL_RGB;
	u32 DestinationColourFormat = Texture.HasAlphaChannel() ? GL_RGBA : GL_RGB;
	glTexImage2D( GL_TEXTURE_2D, 0, SourceColourFormat, Texture.GetWidth(), Texture.GetHeight(), 0, DestinationColourFormat, GL_UNSIGNED_BYTE, Texture.GetPixelData(0) );

	//	generate mip maps
	if ( Texture.IsMipMapEnabled() )
	{
		TLDebug_Break("todo: generate mip maps");
		/*
		u32 SizeX = GNearestPower( m_Size.x );
		u32 SizeY = GNearestPower( m_Size.y );
		int Lod = 0;
		while ( SizeX > 4 && SizeY > 4 )
		{
			glTexImage2D( TextureType(), Lod, glFormat(), SizeX, SizeY, 0, glFormat(), GL_UNSIGNED_BYTE, m_Data.Data() );
			SizeX >>= 1;
			SizeY >>= 1;
			Lod++;
		}
		*/
	}

	//	add to indexing
	g_TextureIndexes.Add( Texture.GetAssetRef(), NewTextureIndex );

	return NewTextureIndex;
}


//---------------------------------------------------
//	destroy opengl texture
//---------------------------------------------------
void TLRender::Opengl::DestroyTexture(u32& TextureIndex)
{
	if ( TextureIndex == 0 )
	{
		TLDebug_Break("Invalid texture index provided to delete");
		return;
	}

	//	make sure it's not bound
	UnbindTextureIndex( TextureIndex );

	//	delete texture
	glDeleteTextures( 1, &TextureIndex );

	//	overwrite the original data just in case it's not invalidated by the caller
	TextureIndex = 0;
}


//---------------------------------------------------
//	bind texture - returns FALSE if no texture is bound (either fail or expected)
//---------------------------------------------------
Bool TLRender::Opengl::BindTexture(const TLAsset::TTexture* pTexture)
{
	u32 TextureIndex = 0;

	//	get texture Index for texture
	if ( pTexture )
	{
		TextureIndex = GetTextureIndex( pTexture->GetAssetRef() );

		//	isn't one... upload texture
		if ( TextureIndex == 0 )
			TextureIndex = UploadTexture( *pTexture );
	}

	//	no change to binding
	if ( TextureIndex == g_BoundTextureIndex )
		return (TextureIndex!=0);

	//	unbinding texture
	if ( TextureIndex == 0 )
	{
		//	unbind
		BindTextureIndex(0);

		//	disable texturing
		glDisable( GL_TEXTURE_2D );
		return FALSE;
	}

	//	binding texture
	BindTextureIndex( TextureIndex );

	//	enable texturing
	glEnable( GL_TEXTURE_2D );

	return TRUE;
}




//---------------------------------------------------
//	bind UV's
//---------------------------------------------------
Bool TLRender::Opengl::BindUVs(const TArray<float2>* pUVs)
{
	//	already bound to this
#ifndef REBIND_DATA
	if ( pUVs == g_pBoundUVs )
		return TRUE;
#endif

	//	check vertex count before binding
#ifdef _DEBUG
	if ( pUVs )
	{
		u32 BoundVertCount = g_pBoundVertexes ? g_pBoundVertexes->GetSize() : 0;
		if ( pUVs->GetSize() != BoundVertCount )
		{
			TLDebug_Break("Binding more/less uv's than vertex positions - corrupt mesh?");
			pUVs = NULL;
		}
	}
#endif

	//	assign as new bound data
	g_pBoundUVs = pUVs;

	//	unbind
	if ( !pUVs )
	{
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		Opengl::Debug_CheckForError();
		return TRUE;
	}

	//	enable texcoord array
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	
	//	bind
	glTexCoordPointer( 2, GL_FLOAT, 0, pUVs->GetData() );
	
	Debug_CheckForError();
	
	return TRUE;
}



//---------------------------------------------------
//	bind colours
//---------------------------------------------------
Bool TLRender::Opengl::BindColours(const TArray<TColour>* pColours)
{
	//	already bound to this
#ifndef REBIND_DATA
	if ( pColours == g_pBoundColours )
		return TRUE;
#endif
	
	//	check vertex count before binding
#ifdef _DEBUG
	if ( pColours )
	{
		u32 BoundVertCount = g_pBoundVertexes ? g_pBoundVertexes->GetSize() : 0;
		if ( pColours->GetSize() != BoundVertCount )
		{
			TLDebug_Break("Binding more/less colours than vertex positions - corrupt mesh?");
			pColours = NULL;
		}
	}
#endif

	//	assign as new bound data	
	g_pBoundColours = pColours;

	//	unbind
	if ( !pColours )
	{
		glDisableClientState( GL_COLOR_ARRAY );
		Opengl::Debug_CheckForError();
		return TRUE;
	}
	
	//	enable texcoord array
	glEnableClientState( GL_COLOR_ARRAY );
	
	//	bind
	glColorPointer( 4, GL_FLOAT, 0, pColours->GetData() );
	
	Debug_CheckForError();
	
	return TRUE;
}



//---------------------------------------------------
//	bind colours
//---------------------------------------------------
Bool TLRender::Opengl::BindColours(const TArray<TColour24>* pColours)
{
	//	already bound to this
#ifndef REBIND_DATA
	if ( pColours == g_pBoundColours )
		return TRUE;
#endif
	
	//	check vertex count before binding
#ifdef _DEBUG
	if ( pColours )
	{
		u32 BoundVertCount = g_pBoundVertexes ? g_pBoundVertexes->GetSize() : 0;
		if ( pColours->GetSize() != BoundVertCount )
		{
			TLDebug_Break("Binding more/less colours than vertex positions - corrupt mesh?");
			pColours = NULL;
		}
	}
#endif

	//	assign as new bound data	
	g_pBoundColours = pColours;

	//	unbind
	if ( !pColours )
	{
		glDisableClientState( GL_COLOR_ARRAY );
		Opengl::Debug_CheckForError();
		return TRUE;
	}
	
	//	enable texcoord array
	glEnableClientState( GL_COLOR_ARRAY );
	
	//	bind
	glColorPointer( 3, GL_UNSIGNED_BYTE, 0, pColours->GetData() );
	
	Debug_CheckForError();
	
	return TRUE;
}

//---------------------------------------------------
//	bind colours
//---------------------------------------------------
Bool TLRender::Opengl::BindColours(const TArray<TColour32>* pColours)
{
#ifndef REBIND_DATA
	//	already bound to this
	if ( pColours == g_pBoundColours )
		return TRUE;
#endif

	//	check vertex count before binding
#ifdef _DEBUG
	if ( pColours )
	{
		u32 BoundVertCount = g_pBoundVertexes ? g_pBoundVertexes->GetSize() : 0;
		if ( pColours->GetSize() != BoundVertCount )
		{
			TLDebug_Break("Binding more/less colours than vertex positions - corrupt mesh?");
			pColours = NULL;
		}
	}
#endif

	//	assign as new bound data	
	g_pBoundColours = pColours;

	//	unbind
	if ( !pColours )
	{
		glDisableClientState( GL_COLOR_ARRAY );
		Opengl::Debug_CheckForError();
		return TRUE;
	}
	
	//	enable texcoord array
	glEnableClientState( GL_COLOR_ARRAY );
	
	//	bind
	glColorPointer( 4, GL_UNSIGNED_BYTE, 0, pColours->GetData() );
	
	Debug_CheckForError();
	
	return TRUE;
}

//---------------------------------------------------
//	bind colours
//---------------------------------------------------
Bool TLRender::Opengl::BindColours(const TArray<TColour64>* pColours)
{
#ifndef REBIND_DATA
	//	already bound to this
	if ( pColours == g_pBoundColours )
		return TRUE;
#endif

	//	check vertex count before binding
#ifdef _DEBUG
	if ( pColours )
	{
		u32 BoundVertCount = g_pBoundVertexes ? g_pBoundVertexes->GetSize() : 0;
		if ( pColours->GetSize() != BoundVertCount )
		{
			TLDebug_Break("Binding more/less colours than vertex positions - corrupt mesh?");
			pColours = NULL;
		}
	}
#endif

	//	assign as new bound data	
	g_pBoundColours = pColours;

	//	unbind
	if ( !pColours )
	{
		glDisableClientState( GL_COLOR_ARRAY );
		Opengl::Debug_CheckForError();
		return TRUE;
	}
	
	//	enable texcoord array
	glEnableClientState( GL_COLOR_ARRAY );
	
	//	bind
	glColorPointer( 4, GL_UNSIGNED_SHORT, 0, pColours->GetData() );
	
	Debug_CheckForError();
	
	return TRUE;
}


//---------------------------------------------------
//	bind verts
//---------------------------------------------------
Bool TLRender::Opengl::BindVertexes(const TArray<float3>* pVertexes)
{
#ifndef REBIND_DATA
	//	already bound to this
	if ( pVertexes == g_pBoundVertexes )
		return TRUE;
#endif
	
	g_pBoundVertexes = pVertexes;
	
	//	unbind
	if ( !pVertexes )
	{
		glDisableClientState( GL_VERTEX_ARRAY );
		Debug_CheckForError();		
		return TRUE;
	}
	
	//	enable texcoord array
	glEnableClientState( GL_VERTEX_ARRAY );

	//	bind
	glVertexPointer( 3, GL_FLOAT, 0, pVertexes->GetData() );
	Debug_CheckForError();		
	
	return TRUE;
}



//-----------------------------------------------------------
//	main renderer, just needs primitive type, and the data
//-----------------------------------------------------------
void TLRender::Opengl::DrawPrimitives(u16 GLPrimType,u32 IndexCount,const u16* pIndexData)
{
	//	no data to render
#ifdef _DEBUG
	if ( !IndexCount || !pIndexData )
	{
		TLDebug_Break("Shouldn't have null/zero data in primitive data");
		return;
	}
#endif

	//	make sure we don't attempt to render vertexes that aren't bound
#ifdef DEBUG_CHECK_PRIMITIVE_INDEXES
	if ( g_pBoundVertexes )
	{
		u32 MaxVertexIndex = g_pBoundVertexes->GetLastIndex();
		for ( u32 i=0;	i<IndexCount;	i++ )
		{
			if ( pIndexData[i] > MaxVertexIndex )
			{
				TLDebug_Break("Primitive contains vertex index out of the range of vertexes that were bound");
				return;
			}
		}
	}
#endif //DEBUG_CHECK_PRIMITIVE_INDEXES
	
	//	draw
	glDrawElements( GLPrimType, IndexCount, GL_UNSIGNED_SHORT, pIndexData );

	//	inc poly counter
	TLRender::g_VertexCount += IndexCount;
	TLRender::g_PolyCount += IndexCount / 3;

	Debug_CheckForError();
}

