#include "TLOpengl.h"
#include <TootleCore/TKeyArray.h>
#include <TootleCore/TMatrix.h>
#include <TootleAsset/TTexture.h>
#include <TootleCore/TBinary.h>


//	include platform specific render code for glTranslate etc
//	this kinda code will move to the rasteriser (as will these includes)
#include PLATFORMHEADER(Opengl.h)

#ifdef _DEBUG
	#define DEBUG_CHECK_PRIMITIVE_INDEXES	//	check indexes of all the primitives are within bound vertex range
#endif


//	gr: added this to check if it was the cause of a crash on iphone, but it's not. Colour format was the problem
//#define REBIND_DATA	//	if defined then we re-bind data even if its the same pointer as was used before


namespace Opengl
{
	u32							g_BoundTextureIndex = 0;						//	currently bound texture
	TKeyArray<TRef,u32>			g_TextureIndexes;								//	Texture Asset Ref -> gl Texture index lookup table

	void				GetOpenglFilterTypes(TArray<u32>& OpenglFilters,Bool MinLinearFilter,Bool MagLinearFilter,Bool MipMapEnabled);	//	get opengl filter type from asset filter type
	void				BindTextureIndex(u32 TextureIndex);					//	set current texture - no change if same as currently bound texture
	void				UnbindTextureIndex(u32 TextureIndex);				//	unbind texture index if it's currently bound
	u32					UploadTexture(const TLAsset::TTexture& Texture);		//	generate opengl texture ID for this asset - if a texture already has an entry it will be destroyed and re-created
	Bool				DestroyTexture(TRefRef TextureRef);					//	destroy opengl texture by texture reference
	Bool				DestroyTexture(u32& TextureIndex);					//	destroy opengl texture
	u32					GetTextureIndex(TRefRef TextureRef);				//	search the texture lookup table for the correct texture ID for this asset
	bool				GetDataType(TRefRef DataTypeRef,u16& OpenglType,u16& OpenglElementCount);	//	get opengl data type (eg. GL_FLOAT) from Data Type Ref
	u16					GetVertexElementType(TLRaster::TVertexElementType::Type ElementType);
}



//---------------------------------------------------
//	
//---------------------------------------------------
bool Opengl::Init()
{
	//	gr: should check for an active context here (the caller should have created 
	//		a context and made it current. Otherwise the commands below will fail
	
	//	init opengl state
	glDisable( GL_CULL_FACE );
	glEnable( GL_SCISSOR_TEST );
	glDisable( GL_TEXTURE_2D );
	
	//	setup opengl stuff
	glDisable( GL_LIGHTING );
	
	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	
	//	fastest for debug line/points
	glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	
	//	make perspective correction good (mostly for textures)
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	//	setup the texture alpha blending mode
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
	
	if ( !Opengl::Platform::Init() )
		return false;
	
	//	print out version
	const char* pDriver = reinterpret_cast<const char*>( glGetString( GL_RENDERER ) );
	const char* pDriverVendor = reinterpret_cast<const char*>( glGetString( GL_VENDOR ) );
	const char* pDriverVersion = reinterpret_cast<const char*>( glGetString( GL_VERSION ) );
	Debug_CheckForError();		

	TDebugString Debug_String;
	Debug_String << "Opengl Driver: \"" << pDriver << "\", Vender: \"" << pDriverVendor << "\", Version: \"" << pDriverVersion << "\"";
	TLDebug_Print( Debug_String );

	return true;
}

//---------------------------------------------------
//	
//---------------------------------------------------
void Opengl::Shutdown()
{
	//	unbind owt
	Unbind();
	
	//	platform shutdown
	Opengl::Platform::Shutdown();
	
	//	dealloc our textures
	g_TextureIndexes.Empty(true);
}



//-----------------------------------------------------------
//	draw vertexes as points
//-----------------------------------------------------------
void Opengl::DrawPrimitivePoints(const TArray<float3>* pVertexes)
{
	//	have a static array of indexes and grow it as required
	static THeapArray<u16> g_AllPoints;

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
void Opengl::Unbind()
{
	//	unbind attribs
	BindVertexes( NULL );
	BindColoursNull();
	BindUVs( NULL );
	BindTexture( NULL );
}

//---------------------------------------------------
//	reset scene
//---------------------------------------------------
void Opengl::SceneReset()
{
	glLoadIdentity();
	
	//	gr: do we need to do camera transform here?
}


//---------------------------------------------------
//	transform scene
//---------------------------------------------------
void Opengl::SceneTransform(const TLMaths::TTransform& Transform,const TLMaths::TMatrix* pMatrix)
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
void Opengl::SceneRotate(const TLMaths::TAngle& Rotation,const float3& Axis)
{
	glRotatef( Rotation.GetDegrees(), Axis.x, Axis.y, Axis.z );
}



//---------------------------------------------------
//	get opengl filter type from asset filter type
//---------------------------------------------------
void Opengl::GetOpenglFilterTypes(TArray<u32>& OpenglFilters,Bool MinLinearFilter,Bool MagLinearFilter,Bool MipMapEnabled)
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
void Opengl::BindTextureIndex(u32 TextureIndex)
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
void Opengl::UnbindTextureIndex(u32 TextureIndex)
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
u32 Opengl::GetTextureIndex(TRefRef TextureRef)
{
	u32* pTextureIndex = g_TextureIndexes.Find( TextureRef );
	return pTextureIndex ? *pTextureIndex : 0;
}


//---------------------------------------------------
//	generate opengl texture ID for this asset - if a texture already has an entry it will be destroyed and re-created
//---------------------------------------------------
u32 Opengl::UploadTexture(const TLAsset::TTexture& Texture)
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

	TDebugString Debug_String;
	Debug_String << "Uploading texture " << Texture.GetAssetAndTypeRef();
	TLDebug_Print( Debug_String );

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
//	destroy opengl texture by texture reference
//---------------------------------------------------
Bool Opengl::DestroyTexture(TRefRef TextureRef)
{
	u32 TextureIndex = GetTextureIndex( TextureRef );
	if ( TextureIndex == 0 )
		return false;

	//	destroy texture
	DestroyTexture( TextureIndex );

	return true;
}

//---------------------------------------------------
//	destroy opengl texture by its internal index
//---------------------------------------------------
Bool Opengl::DestroyTexture(u32& TextureIndex)
{
	if ( TextureIndex == 0 )
	{
		TLDebug_Break("Invalid texture index provided to delete");
		return false;
	}

	//	gr: in debug, we should probably check that this texture index actually exists
	s32 TextureIndexIndex = g_TextureIndexes.FindKeyIndex( TextureIndex );
	if ( TextureIndexIndex == -1 )
	{
		TLDebug_Break("Trying to delete texture index that hasn't been created (or not entered into the index!)");
		return false;
	}

	//	make sure it's not bound
	UnbindTextureIndex( TextureIndex );

	//	delete texture
	glDeleteTextures( 1, &TextureIndex );

	//	remove from the texture index
	g_TextureIndexes.RemoveAt( TextureIndexIndex );

	//	overwrite the original data just in case it's not invalidated by the caller
	TextureIndex = 0;

	return true;
}


//---------------------------------------------------
//	bind texture - returns FALSE if no texture is bound (either fail or expected)
//---------------------------------------------------
Bool Opengl::BindTexture(const TLAsset::TTexture* pTexture)
{
	u32 TextureIndex = 0;

	//	get texture Index for texture
	if ( pTexture )
	{
		TextureIndex = GetTextureIndex( pTexture->GetAssetRef() );

		//	isn't one... upload texture
		if ( TextureIndex == 0 )
		{
			TextureIndex = UploadTexture( *pTexture );
		}
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
Bool Opengl::BindUVs(const TArray<float2>* pUVs)
{
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
Bool Opengl::BindColours(const TArray<TColour>* pColours)
{
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
Bool Opengl::BindColours(const TArray<TColour24>* pColours)
{
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
Bool Opengl::BindColours(const TArray<TColour32>* pColours)
{
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
Bool Opengl::BindColours(const TArray<TColour64>* pColours)
{
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
Bool Opengl::BindVertexes(const TArray<float3>* pVertexes)
{
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


u16 Opengl::GetVertexElementType(TLRaster::TVertexElementType::Type ElementType)
{
	switch ( ElementType )
	{
		case TLRaster::TVertexElementType::Position:	return GL_VERTEX_ARRAY;
		case TLRaster::TVertexElementType::Normal:		return GL_NORMAL_ARRAY;
		case TLRaster::TVertexElementType::Colour:		return GL_COLOR_ARRAY;
		case TLRaster::TVertexElementType::TexCoord:	return GL_TEXTURE_COORD_ARRAY;
		default:
			TLDebug_Break("Unknwon vertex element type");
			return 0;
	}
}	

//-----------------------------------------------------------
//	unbind this vertex element
//-----------------------------------------------------------
void Opengl::Unbind(const TLRaster::TVertexElementType::Type& Element)
{
	u16 ElementTypegl = GetVertexElementType( Element );
	if ( ElementTypegl == 0x0 )
		return;
	
	glDisableClientState( ElementTypegl );
	Debug_CheckForError();
}


//-----------------------------------------------------------
//	bind this vertex element
//-----------------------------------------------------------
bool Opengl::BindVertexElement(const TLRaster::TVertexElement& Element)
{
	u16 ElementTypegl = GetVertexElementType( Element.m_ElementType );
	if ( ElementTypegl == 0x0 )
		return false;
	
	//	disable this element
	if ( !Element.IsValid() )
	{
		glDisableClientState( ElementTypegl );
		Debug_CheckForError();
		return false;
	}

	//	get the opengl type
	u16 ElementDataTypegl = 0x0;
	u16 ElementCountgl = 0;
	if ( !GetDataType( Element.m_DataType, ElementDataTypegl, ElementCountgl ) )
	{
		glDisableClientState( ElementTypegl );
		Debug_CheckForError();
		return false;
	}
	
	//	enable and bind element
	glEnableClientState( ElementTypegl );
		
	//	bind
	switch ( Element.m_ElementType )
	{
		case TLRaster::TVertexElementType::Position:
			glVertexPointer( ElementCountgl, ElementDataTypegl, Element.m_Stride, Element.m_pData.m_void );
			break;
			
		case TLRaster::TVertexElementType::Normal:
			if ( ElementCountgl != 3 )
				TLDebug_Break("Normal vertex element must have elements parts");
			glNormalPointer( ElementDataTypegl, Element.m_Stride, Element.m_pData.m_void );
			break;
			
		case TLRaster::TVertexElementType::Colour:
			glColorPointer( ElementCountgl, ElementDataTypegl, Element.m_Stride, Element.m_pData.m_void );
			break;
			
		case TLRaster::TVertexElementType::TexCoord:
			glTexCoordPointer( ElementCountgl, ElementDataTypegl, Element.m_Stride, Element.m_pData.m_void );
			break;
			
		default:
			TLDebug_Break("Unknwon vertex element type");
			return false;
	}
	
	Debug_CheckForError();
	return true;
}



//------------------------------------------------------------
//	get opengl data type (eg. GL_FLOAT) from Data Type Ref
//------------------------------------------------------------
bool Opengl::GetDataType(TRefRef DataTypeRef,u16& OpenglType,u16& OpenglElementCount)
{
#define case_BinaryTypeToOpenglType(TYPE)	\
	case TLBinary_TypeNRef( Type4, TYPE ):	OpenglElementCount = 4;	OpenglType = GetDataType<TYPE>();	return (OpenglType!=0);		\
	case TLBinary_TypeNRef( Type3, TYPE ):	OpenglElementCount = 3;	OpenglType = GetDataType<TYPE>();	return (OpenglType!=0);		\
	case TLBinary_TypeNRef( Type2, TYPE ):	OpenglElementCount = 2;	OpenglType = GetDataType<TYPE>();	return (OpenglType!=0);		\
	case TLBinary_TypeRef( TYPE ):			OpenglElementCount = 1;	OpenglType = GetDataType<TYPE>();	return (OpenglType!=0);		\

	switch ( DataTypeRef.GetData() )
	{
		case_BinaryTypeToOpenglType( float );
		case_BinaryTypeToOpenglType( u8 );
		case_BinaryTypeToOpenglType( s8 );
		case_BinaryTypeToOpenglType( u16 );
		case_BinaryTypeToOpenglType( s16 );
		case_BinaryTypeToOpenglType( u32 );
		case_BinaryTypeToOpenglType( s32 );
		case_BinaryTypeToOpenglType( u64 );
		case_BinaryTypeToOpenglType( s64 );
			
		case TLBinary_TypeRef( TColour ):		OpenglElementCount = 4;	OpenglType = GetDataType<float>();	return (OpenglType!=0);	return true;
		case TLBinary_TypeRef( TColour24 ):		OpenglElementCount = 3;	OpenglType = GetDataType<u8>();		return (OpenglType!=0);	return true;
		case TLBinary_TypeRef( TColour32 ):		OpenglElementCount = 4;	OpenglType = GetDataType<u8>();		return (OpenglType!=0);	return true;
			
		default:
		{
			TDebugString Debug_String;
			Debug_String << "Don't know how to convert type " << DataTypeRef << " to opengl data type";
			TLDebug_Break( Debug_String );
			return false;
		}
	}
	
#undef case_BinaryTypeToOpenglType

}


//-----------------------------------------------------------
//	main renderer, just needs primitive type, and the data
//-----------------------------------------------------------
void Opengl::DrawPrimitives(u16 GLPrimType,u32 IndexCount,const u16* pIndexData)
{
	//	no data to render
#ifdef _DEBUG
	if ( !IndexCount || !pIndexData )
	{
		TLDebug_Break("Shouldn't have null/zero data in primitive data");
		return;
	}
#endif

/*
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
	*/
	
	//	draw
	glDrawElements( GLPrimType, IndexCount, GL_UNSIGNED_SHORT, pIndexData );

	Debug_CheckForError();
}


//----------------------------------------------------------------------------//
//	transform the texture matrix
//----------------------------------------------------------------------------//
void Opengl::TextureTransform(const TLMaths::TTransform& Transform)
{
	//	switch to texture matrix mode
	glMatrixMode(GL_TEXTURE);

	//	gr: identity should be redundant...
	glLoadIdentity();

	//	apply the transform to the scene (currently the texture mode)
	SceneTransform( Transform );

	//	gr: assuming we were in modelview mode before here
	glMatrixMode(GL_MODELVIEW);
}

