/*

	opengl rasteriser implementation.
	
*/
#pragma once
#include <TootleRender/TRasteriser.h>


namespace TLRender
{
	class TOrthoCamera;
	class TProjectCamera;
}

namespace TLRaster
{
	class OpenglRasteriser;
}

namespace Opengl
{
	class TShaderAssetFactory;		//	asset factory for fixed-function shaders
}


//------------------------------------------------------
//	generic opengl rasteriser
//------------------------------------------------------
class TLRaster::OpenglRasteriser : public TLRaster::TRasteriser
{
public:
	OpenglRasteriser()				{}

	virtual bool		Initialise();		//	init rasteriser and opengl system
	virtual void		Shutdown();			//	

	virtual void		Rasterise(TLRender::TRenderTarget& RenderTarget,const TLRender::TScreen& Screen);		//	real renderer, rasterises everything to this render target

	virtual void		Render(const TArray<TRasterData>& Data)					{	m_RasterData.Add( Data );	}
	virtual void		Render(const TArray<TRasterSpriteData>& Data)			{	m_RasterSpriteData.Add( Data );	}
	virtual void		StoreTemporaryMeshes(const TPtrArray<TLAsset::TMesh>& Meshes)	{	m_TempMeshes.Add( Meshes );	}

protected:
	virtual void		OnTextureDeleted(TRefRef TextureRef);
	virtual void		OnTextureChanged(TRefRef TextureRef);

private:
	bool				BeginRasterise(TLRender::TRenderTarget& RenderTarget,const TLRender::TScreen& Screen);	//	setup viewports etc
	void				EndRasterise(TLRender::TRenderTarget& RenderTarget,const TLRender::TScreen& Screen);	//	

	void				SetFrustum(const TLRender::TOrthoCamera& Camera,const TLRender::TScreen& Screen);
	void				SetFrustum(const TLRender::TProjectCamera& Camera,const TLRender::TScreen& Screen);
	void				SetCamera(TLRender::TRenderTarget& RenderTarget,TLRender::TProjectCamera& Camera);
	void				SetCamera(TLRender::TRenderTarget& RenderTarget,TLRender::TOrthoCamera& Camera);
	
	void				Flush();							//	post-rasterisation flushes
	void				ResolveSprites();					//	turn sprites into normal raster data
	bool				AllocSpriteTriangles(u32 MaxSpriteCount);
	void				Rasterise(const TRasterData& Data);	//	this is the dumb, but final actual rendering function

private:
	TPtr<TLAsset::TAssetFactory>		m_pShaderAssetFactory;

	THeapArray<TRasterData,100,TSortPolicySorted<TRasterData> >				m_RasterData;
	THeapArray<TRasterSpriteData,100,TSortPolicySorted<TRasterSpriteData> >	m_RasterSpriteData;
	TPtrArray<TLAsset::TMesh>			m_TempMeshes;

	THeapArray<TLAsset::TSpriteGlyph>	m_Sprites;			//	list of all sprites (technically just their vertexes)
	THeapArray<Type3<u16> >				m_SpriteTriangles;	//	sprite triangle buffer
};




//	fixed functions opengl shaders
namespace TLAsset
{
	class TShader_TextureMatrix;
}


//------------------------------------------------
//	asset factory for fixed-function shaders
//------------------------------------------------
class Opengl::TShaderAssetFactory : public TLAsset::TAssetFactory
{
protected:
	virtual TLAsset::TAsset*	CreateObject(TRefRef InstanceRef,TRefRef TypeRef);	
};

//------------------------------------------------
//	Fixed function shader which modifies the texture matrix. This allows us to modify 
//	ALL the uv's on a mesh in one go. Useful for sprites for animated textures.
//	Possible that a software shader which modifies the UV's might be faster (no state change).
//	Certainly a vertex shader would be faster.
//	
//	shader data expects to find a TTransform
//------------------------------------------------
class TLAsset::TShader_TextureMatrix : public TLAsset::TShader
{
public:
	TShader_TextureMatrix(TRefRef AssetRef);

	static TRef			GetShaderInstanceRef()		{	return TRef_Static(F,F,T,x,M);	}	//	FixedFunctionTeXtureMatrix

	virtual Bool		PreRender(TBinaryTree& ShaderData);
	virtual void		PostRender();
};


