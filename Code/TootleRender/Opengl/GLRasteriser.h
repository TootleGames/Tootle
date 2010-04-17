/*

	Base class for a rasteriser (make this a manager?)

	gr: currently just adding bits to this as they change in the render/opengl code
	until we make the big change over to rasteriser libs

*/
#pragma once

#include "../TRasteriser.h"


namespace TLRaster
{
	namespace Opengl
	{
		class GLRasteriser;
	}
};


//------------------------------------------------------
//	opengl rasteriser
//------------------------------------------------------
class TLRaster::Opengl::GLRasteriser : public TLRaster::TRasteriser
{
public:
	GLRasteriser()			{}
	virtual ~GLRasteriser()	{}

protected:
	virtual void		OnTextureDeleted(TRefRef TextureRef);
	virtual void		OnTextureChanged(TRefRef TextureRef);
};

