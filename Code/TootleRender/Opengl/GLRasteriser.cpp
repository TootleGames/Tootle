#include "GLRasteriser.h"
#include "../TLRender.h"



void TLRaster::Opengl::GLRasteriser::OnTextureDeleted(TRefRef TextureRef)
{
	TLRender::Opengl::DestroyTexture( TextureRef );
}



void TLRaster::Opengl::GLRasteriser::OnTextureChanged(TRefRef TextureRef)
{
	//	gr: delete the texture so it's re-uploaded on next bind.
	//	todo: just update contents of the texture!
	TLRender::Opengl::DestroyTexture( TextureRef );
}


