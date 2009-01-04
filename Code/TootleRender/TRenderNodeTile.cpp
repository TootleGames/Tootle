#include "TRenderNodeTile.h"
#include <TootleRender/TCamera.h>
#include <TootleRender/TRenderTarget.h>



TLRender::TRenderNodeTile::TRenderNodeTile(TRefRef RenderNodeRef,TRefRef TypeRef) :
	TRenderNode	( RenderNodeRef, TypeRef )
{
}


//--------------------------------------------------------
//	generate render nodes on the fly to render this mesh as tiled relative to the camera
//--------------------------------------------------------
Bool TLRender::TRenderNodeTile::Draw(TRenderTarget* pRenderTarget,TRenderNode* pParent,TPtrArray<TRenderNode>& PostRenderList)
{
	//	get our mesh's bounding box so we know how big each tile is
	TPtr<TLAsset::TMesh> pTileMesh;
	GetMeshAsset( pTileMesh );
	if ( !pTileMesh )
		return FALSE;

	TLMaths::TBox TileBox = pTileMesh->CalcBoundsBox();
	if ( !TileBox.IsValid() )
		return FALSE;
	
	//	transform the tile box so scaling, rotation etc is applied
	//	gr: DONT transform the box, if this is scaled, it's applied to the children
	//	so their spacing is scaled anyway.
	//	rotation might be needed though? scale certainly unwanted here
//	TileBox.Transform( GetTransform() );

	//	get the render target's camera lookat
	TPtr<TLRender::TCamera>& pCamera = pRenderTarget->GetCamera();
	if ( !pCamera )
		return FALSE;

	float3 NodePos(0,0,0);
	GetTransform().TransformVector( NodePos );
	const float3& CameraLookAt = pCamera->GetLookAt();
	float3 TileSize = TileBox.GetSize();
	float3 TileStep( (CameraLookAt-NodePos) / TileSize );

	Type2<s32> TileOffset;
	TileOffset.x = (s32)( CameraLookAt.x / TileSize.x );
	TileOffset.y = (s32)( CameraLookAt.y / TileSize.y );

	//	todo: change this to expand OUTWARDS from the center tileoffset until
	//		we're outside the viewable area
	float CameraZ = 50.f;

	Bool Widescreen = FALSE;
	float ScreenScale = 1.f;
	if ( pCamera->IsOrtho() )
		ScreenScale = 3.f;

	float2 ScreenScale2 = Widescreen ? float2( ScreenScale * 1.5f, ScreenScale * 1.f ) : float2( ScreenScale * 1.f, ScreenScale * 1.5f );

	s32 TilesWide = ( (ScreenScale2.x*CameraZ) / TileSize.x) / 2.f;
	s32 TilesHigh = ( (ScreenScale2.y*CameraZ) / TileSize.y) / 2.f;
	u32 NodeCount = 0;

	if ( pCamera->IsOrtho() )
	{
		TileOffset.x += TilesWide;
		TileOffset.y += TilesHigh;
	}

	for ( s32 y=-TilesHigh;	y<TilesHigh;	y++ )
	{
		float3 TilePos;
		float TileY = (float)( y+TileOffset.y );
		TilePos.y = (TileY * TileSize.y) ;//+ NodePos.y;	//	dont include node pos as it's inherited from this
		//TilePos.z = NodePos.z;
		for ( s32 x=-TilesWide;	x<TilesWide;	x++ )
		{
			float TileX = (float)( x+TileOffset.x );
			TilePos.x = (TileX * TileSize.x) ;//+ NodePos.x;	//	dont include node pos as it's inherited from this

			//	alloc nodes as required
			if ( NodeCount >= m_TileRenderNodeBuffer.GetSize() )
			{
				TLRender::TRenderNode* pNewNode = new TLRender::TRenderNode("Tile");
				if ( !pNewNode )
					break;
				m_TileRenderNodeBuffer.Add( pNewNode );
			}

			TPtr<TLRender::TRenderNode>& pTileNode = m_TileRenderNodeBuffer[ NodeCount ];
			if ( !pTileNode )
				break;

			//	update node
			pTileNode->SetMeshRef( GetMeshRef() );
			pTileNode->SetTranslate( TilePos );
			
			//	add to render list
			PostRenderList.Add( pTileNode );
			NodeCount++;
		}
	}

	return FALSE;
}

