#include "TRasteriser.h"
#include <TootleAsset/TLAsset.h>




SyncBool TLRaster::TRasteriser::Initialise()
{
	//	subscribe to the asset manager to catch when assets are deleted
	if ( !TLAsset::g_pManager )
		return SyncWait;

	this->SubscribeTo( TLAsset::g_pManager );

	return SyncTrue;
}


SyncBool TLRaster::TRasteriser::Shutdown()
{
	return SyncTrue;
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

