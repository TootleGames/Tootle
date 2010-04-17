/*

	Base class for a rasteriser (make this a manager?)

	gr: currently just adding bits to this as they change in the render/opengl code
	until we make the big change over to rasteriser libs

*/
#pragma once

#include <TootleCore/TSubscriber.h>


namespace TLRaster
{
	class TRasteriser;
};




class TLRaster::TRasteriser : public TLMessaging::TSubscriber
{
public:
	TRasteriser()			{}
	virtual ~TRasteriser()	{}
	
	virtual SyncBool	Initialise();
	virtual SyncBool	Shutdown();

	virtual TRefRef		GetSubscriberRef() const		{	static TRef Ref="Raster";	return Ref;	}

protected:
	virtual void		OnTextureDeleted(TRefRef TextureRef)=0;
	virtual void		OnTextureChanged(TRefRef TextureRef)=0;

private:
	virtual void		ProcessMessage(TLMessaging::TMessage& Message);
};

