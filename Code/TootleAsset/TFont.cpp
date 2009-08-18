#include "TFont.h"
#include <TootleCore/TBinaryTree.h>

#ifdef _DEBUG
#define IMPORT_CHILDREN_PER_UPDATE	10
#else
#define IMPORT_CHILDREN_PER_UPDATE	100
#endif



TLAsset::TFont::TFont(const TRef& AssetRef) :
	TAsset				( GetAssetType_Static(), AssetRef ),
	m_ImportChildIndex	( 0 )
{
}




//-------------------------------------------------------
//	add a new glyph to the font
//-------------------------------------------------------
TPtr<TLAsset::TMesh>& TLAsset::TFont::AddGlyph(u16 Character)
{
	//	create a new mesh and add
	TPtr<TLAsset::TMesh> pGlyphMesh = new TLAsset::TMesh( TRef() );

	//	add (this sets up the mesh)
	return AddGlyph( Character, pGlyphMesh );
}


//-------------------------------------------------------
//	add a new glyph to the font
//-------------------------------------------------------
TPtr<TLAsset::TMesh>& TLAsset::TFont::AddGlyph(u16 Character,TPtr<TLAsset::TMesh>& pMesh)
{
	if ( !pMesh )
	{
		TLDebug_Break("Mesh expected");
		return TLPtr::GetNullPtr<TLAsset::TMesh>();
	}

	//	make up a better ref
	TRef GlyphMeshRef( (u32)Character );
	pMesh->SetAssetRef( GlyphMeshRef );

	//	add glyph data to the mesh
	TPtr<TBinaryTree> pGlyphData = pMesh->GetData("Glyph", TRUE );
	if ( !pGlyphData )
	{
		TLDebug_Break("Mesh is missing glyph information");
		return TLPtr::GetNullPtr<TLAsset::TMesh>();
	}

	u16 GlyphChar = Character;
	pGlyphData->Empty(FALSE);
	pGlyphData->Write( GlyphChar );

	//	add mesh to the glyph list
	TPtr<TLAsset::TMesh>* pAddedMesh = m_Glyphs.Add( Character, pMesh );

	if ( !pAddedMesh )
	{
		TLDebug_Print( TString("Warning: overwriting existing glyph character %c(0x%04x) in font", (char)Character, Character ) );
		return TLPtr::GetNullPtr<TLAsset::TMesh>();
	}

	return *pAddedMesh;
}




//-------------------------------------------------------
//	load asset data out binary data
//-------------------------------------------------------
SyncBool TLAsset::TFont::ImportData(TBinaryTree& Data)		
{
	TPtrArray<TBinaryTree> DataArray;

	//	find children
	if ( Data.GetChildren("Glyph", DataArray) > 0 )
	{
		u32 ProcessCount = 0;

		//	process next children
		for ( ;	m_ImportChildIndex<DataArray.GetSize();	m_ImportChildIndex++,ProcessCount++ )
		{
			//	only process N at a time
			if ( ProcessCount >= IMPORT_CHILDREN_PER_UPDATE )
				return SyncWait;

			TPtr<TBinaryTree>& pChildData = DataArray[m_ImportChildIndex];
			if ( !pChildData )
			{
				TLDebug_Break("Child data expected");
				continue;
			}

			pChildData->ResetReadPos();

			TRef ChildRef;
			if ( !pChildData->Read( ChildRef ) )
				return SyncFalse;

			//	create a new child mesh
			TPtr<TLAsset::TMesh> pNewChildMesh = new TLAsset::TMesh( ChildRef );
			
			//	import it
			if ( !pNewChildMesh->ImportData( *pChildData.GetObjectPointer() ) )
				return SyncFalse;

			//	read out the glyph character
			TPtr<TBinaryTree> pGlyphData = pNewChildMesh->GetData("glyph");

			//	dont know what glyph this mesh is for..
			if ( !pGlyphData )
				continue;

			pGlyphData->ResetReadPos();
			
			u16 GlyphChar;
			if ( !pGlyphData->Read( GlyphChar ) )
				continue;

			//	we know what char this is for so attach this mesh as it's appropriate character
			AddGlyph( GlyphChar, pNewChildMesh );
		}
	}

	//	store unread data
	ImportUnknownData( Data );

	//	all finished, reset child import index 
	m_ImportChildIndex = 0;

	return SyncTrue;
}


//-------------------------------------------------------
//	save asset data to binary data
//-------------------------------------------------------
SyncBool TLAsset::TFont::ExportData(TBinaryTree& Data)				
{	
	//	export each glyph mesh
	for ( u32 c=0;	c<m_Glyphs.GetSize();	c++ )
	{
		TPtr<TLAsset::TMesh>& pChildMesh = m_Glyphs.ElementAt(c);

		//	ensure each glyph mesh has it's glyph data
		TPtr<TBinaryTree>& pChildMeshGlyph = pChildMesh->GetData("Glyph");
		if ( !pChildMeshGlyph )
		{
			if ( !TLDebug_Break("Glyph child mesh of font is missing Glyph data") )
				continue;
		}

		//	add to exporting data
		TPtr<TBinaryTree>& pChildData = Data.AddChild("Glyph");
		if ( !pChildData )
		{
			TLDebug_Break("Failed to create child");
			continue;
		}

		//	write the ref of the child to the child's data
		pChildData->Write( pChildMesh->GetAssetRef() );

		//	now export the data for that child
		pChildMesh->ExportData( *pChildData.GetObjectPointer() );
	}

	//	export other data
	ExportUnknownData( Data );

	return SyncTrue;
}	
