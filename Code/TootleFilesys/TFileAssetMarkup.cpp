#include "TFileAssetMarkup.h"
#include <TootleAsset/TMesh.h>
#include <TootleMaths/TTessellate.h>
#include "TLFile.h"

namespace TLFileAssetMarkup
{
	//	get list of bits of the geometry to import
	enum PartFlags
	{
		Part_Vertexes = 0,
		Part_Colours,
		Part_Polygons,
		Part_Lines,
	};
}



TLFileSys::TFileAssetMarkup::TFileAssetMarkup(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml				( FileRef, FileTypeRef )
{
}



//--------------------------------------------------------
//	import the XML and do all the commands
//--------------------------------------------------------
SyncBool TLFileSys::TFileAssetMarkup::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
{
	if ( pAsset )
	{
		TLDebug_Break("Async export not supported yet. asset should be NULL");
		return SyncFalse;
	}

	Supported = TRUE;

	//	import xml
	SyncBool ImportResult = TFileXml::Import();
	if ( ImportResult != SyncTrue )
		return ImportResult;

	//	get the root tag
	TPtr<TXmlTag> pTamTag = m_XmlData.GetChild("tam");

	//	malformed TAM
	if ( !pTamTag )
	{
		TLDebug_Print("TAM file missing root <TAM> tag");
		return SyncFalse;
	}

	//	get asset type
	const TString* pAssetTypeString = pTamTag->GetProperty("AssetType");
	if ( !pAssetTypeString )
	{
		TLDebug_Print("TAM tag missing asset type, e.g: <TAM AssetType=\"Mesh\">");
		return SyncFalse;
	}

	//	create asset
	TRef AssetType( *pAssetTypeString );

	//	do specific importing
	ImportResult = SyncFalse;
	TPtr<TLAsset::TAsset> pNewAsset;

	if ( AssetType == "Mesh" )
	{
		pNewAsset = new TLAsset::TMesh( GetFileRef() );
		ImportResult = ImportMesh( pNewAsset, pTamTag );
	}
	else
	{
		TLDebug_Break("Unsupported asset type in TAM");
	}

	//	failed to import
	if ( ImportResult != SyncTrue )
	{
		return SyncFalse;
	}

	//	assign resulting asset
	pAsset = pNewAsset;

	return SyncTrue;
}


//--------------------------------------------------------
//	generate mesh TAM tag
//--------------------------------------------------------
SyncBool TLFileSys::TFileAssetMarkup::ImportMesh(TPtr<TLAsset::TMesh> pMesh,TPtr<TXmlTag>& pTag)
{
	/*
	<tam assettype="mesh">
		<import assetref="m_ball">
			<all scale="2" />
		</import>
	</tam>
	*/
	//	deal with child tags
	for ( u32 c=0;	c<pTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pTag->GetChildren().ElementAt(c);

		SyncBool TagImportResult = SyncFalse;
		if ( pChildTag->GetTagName() == "import" )
		{
			TagImportResult = ImportMesh_ImportTag( pMesh, pChildTag );
		}
		else
		{
			TLDebug_Break("Unsupported tag in TAM mesh import");
		}

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async TAM import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}


//--------------------------------------------------------
//	generate mesh TAM tag
//--------------------------------------------------------
SyncBool TLFileSys::TFileAssetMarkup::ImportMesh_ImportTag(TPtr<TLAsset::TMesh>& pMesh,TPtr<TXmlTag>& pImportTag)
{
	/*
	<import assetref="m_ball">
		<all scale="2" />
	</import>
	*/
	//	get mesh to import
	TRef ImportMeshRef;
	const TString* pImportMeshRefString = pImportTag->GetProperty("AssetRef");
	if ( pImportMeshRefString )
		ImportMeshRef.Set( *pImportMeshRefString );
	if ( !ImportMeshRef.IsValid() )
	{
		TLDebug_Print("TAM mesh import: Invalid mesh ref");
		return SyncFalse;
	}

	//	get the asset we're importing from
	TPtr<TLAsset::TMesh> pImportMesh = TLAsset::GetAssetPtr<TLAsset::TMesh>( ImportMeshRef );
	if ( !pImportMesh )
	{
		TLDebug_Print("TAM mesh import: Unknown asset");
		return SyncFalse;
	}

	//	find out what we need to do with this mesh
	for ( u32 c=0;	c<pImportTag->GetChildren().GetSize();	c++ )
	{
		TPtr<TXmlTag>& pChildTag = pImportTag->GetChildren().ElementAt(c);
		TRef ImportMeshPartRef( pChildTag->GetTagName() );

		SyncBool TagImportResult = ImportMesh_ImportTag_Part( pMesh, pImportMesh, ImportMeshPartRef, pChildTag );

		//	failed
		if ( TagImportResult == SyncFalse )
			return SyncFalse;

		//	async
		if ( TagImportResult == SyncWait )
		{
			TLDebug_Break("todo: async TAM import");
			return SyncFalse;
		}
	}
	
	return SyncTrue;
}


template<class ARRAYTYPE>
void IncrementVertexIndexes(ARRAYTYPE& Indexes,s32 VertexIncrement)
{
	if ( VertexIncrement < 0 )
	{
		TLDebug_Break("Invalid vertex increment");
		return;
	}

	for ( u32 i=0;	i<Indexes.GetSize();	i++ )
		Indexes[i] += VertexIncrement;
}

template<class ARRAYTYPE>
void AppendGeometry(ARRAYTYPE& GeometryTo,const ARRAYTYPE& GeometryFrom,s32 VertexOffset)
{
	s32 FirstPoly = GeometryTo.Add( GeometryFrom );

	//	nothing added
	if ( FirstPoly == -1 )
		return;

	for ( u32 i=FirstPoly;	i<GeometryTo.GetSize();	i++ )
	{
		IncrementVertexIndexes( GeometryTo[i], VertexOffset );
	}
}



//--------------------------------------------------------
//	import part of a mesh into this mesh
//--------------------------------------------------------
SyncBool TLFileSys::TFileAssetMarkup::ImportMesh_ImportTag_Part(TPtr<TLAsset::TMesh>& pMesh,const TPtr<TLAsset::TMesh>& pImportMesh,TRefRef ImportMeshPartRef,TPtr<TXmlTag>& pImportTag)
{
	/*
		<all scale="2" />
	*/

	//	store off the range of vertexes so if we manipulate them, we don't affect existing geometry
	s32 FirstVertexIndex = -1;
	s32 LastVertexIndex = -1;

	TFlags<TLFileAssetMarkup::PartFlags> ImportPartFlags;

	if ( ImportMeshPartRef == "all" )
	{
		ImportPartFlags.Set( TLFileAssetMarkup::Part_Vertexes );
		ImportPartFlags.Set( TLFileAssetMarkup::Part_Colours );
		ImportPartFlags.Set( TLFileAssetMarkup::Part_Polygons );
		ImportPartFlags.Set( TLFileAssetMarkup::Part_Lines );
	}
	else if ( ImportMeshPartRef == "polygons" )
	{
		ImportPartFlags.Set( TLFileAssetMarkup::Part_Vertexes );
		ImportPartFlags.Set( TLFileAssetMarkup::Part_Colours );
		ImportPartFlags.Set( TLFileAssetMarkup::Part_Polygons );
	}
	else if ( ImportMeshPartRef == "lines" )
	{
		ImportPartFlags.Set( TLFileAssetMarkup::Part_Vertexes );
		ImportPartFlags.Set( TLFileAssetMarkup::Part_Colours );
		ImportPartFlags.Set( TLFileAssetMarkup::Part_Lines );
	}
	else
	{
		TLDebug_Print("TAM mesh import: unknown part of geometry to import from mesh");
		return SyncFalse;
	}

	//	invalidate flags we dont need
	const TArray<float3>& ImportMeshVerts = pImportMesh->GetVertexes();
	const TArray<TColour>& ImportMeshColours = pImportMesh->GetColours();
	if ( !ImportMeshColours.GetSize() )
		ImportPartFlags.Clear( TLFileAssetMarkup::Part_Colours );

	//	import verts
	if ( ImportPartFlags( TLFileAssetMarkup::Part_Vertexes ) )
	{
		for ( u32 v=0;	v<ImportMeshVerts.GetSize();	v++ )
		{
			s32 NewVertIndex;
			if ( ImportPartFlags( TLFileAssetMarkup::Part_Colours ) )
				NewVertIndex = pMesh->AddVertex( ImportMeshVerts[v], ImportMeshColours[v] );
			else
				NewVertIndex = pMesh->AddVertex( ImportMeshVerts[v] );

			if ( FirstVertexIndex == -1 )	
				FirstVertexIndex = NewVertIndex;
			LastVertexIndex = NewVertIndex;
		}
	}

	//	no vertexes to import - dont import geometry
	if ( FirstVertexIndex == -1 || LastVertexIndex == -1 )
	{
		ImportPartFlags.Clear( TLFileAssetMarkup::Part_Polygons );
		ImportPartFlags.Clear( TLFileAssetMarkup::Part_Lines );
	}

	//	import geometry
	if ( ImportPartFlags( TLFileAssetMarkup::Part_Polygons ) )
	{
		AppendGeometry( pMesh->GetTriangles(), pImportMesh->GetTriangles(), FirstVertexIndex );
		AppendGeometry( pMesh->GetTristrips(), pImportMesh->GetTristrips(), FirstVertexIndex );
		AppendGeometry( pMesh->GetTrifans(), pImportMesh->GetTrifans(), FirstVertexIndex );
	}

	if ( ImportPartFlags( TLFileAssetMarkup::Part_Lines ) )
	{
		AppendGeometry( pMesh->GetLines(), pImportMesh->GetLines(), FirstVertexIndex );
	}

	//	manipulate geometry using the properties
	//	apply them in order so that we can do shader-like things
	for ( u32 p=0;	p<pImportTag->GetPropertyCount();	p++)
	{
		const TXmlTag::TProperty& Property = pImportTag->GetPropertyAt(p);

		const TStringLowercase<TTempString>& PropertyName = Property.m_Key;
		const TString& PropertyData = Property.m_Item;

		if ( PropertyName == "Scale" )
		{
			float Scale;
			if ( !PropertyData.GetFloat( Scale ) )
				continue;

			pMesh->ScaleVerts( Scale, FirstVertexIndex, LastVertexIndex );
		}
		else if ( PropertyName == "ColourMult" )
		{
			float4 ColourChange;
			u32 CharIndex = 0;
			if ( !TLString::ReadNextFloatArray( PropertyData, CharIndex, ColourChange.GetData(), ColourChange.GetSize() ) )
				continue;

			//	go through all the colours and multiply them
			u32 LastColourIndex = (u32)LastVertexIndex < pMesh->GetColours().GetSize() ? (u32)LastVertexIndex : pMesh->GetColours().GetSize();
			for ( u32 v=(s32)FirstVertexIndex;	v<LastColourIndex;	v++ )
			{
				TColour& MeshColour = pMesh->GetColours().ElementAt(v);
				MeshColour *= ColourChange;
			}
		}
		else if ( PropertyName == "ColourAdd" )
		{
			float4 ColourChange;
			u32 CharIndex = 0;
			if ( !TLString::ReadNextFloatArray( PropertyData, CharIndex, ColourChange.GetData(), ColourChange.GetSize() ) )
				continue;

			//	go through all the colours and multiply them
			u32 LastColourIndex = (u32)LastVertexIndex < pMesh->GetColours().GetSize() ? (u32)LastVertexIndex : pMesh->GetColours().GetSize();
			for ( u32 v=FirstVertexIndex;	v<LastColourIndex;	v++ )
			{
				TColour& MeshColour = pMesh->GetColours().ElementAt(v);
				MeshColour += ColourChange;
			}
		}
		else if ( PropertyName == "ColourSub" )
		{
			float4 ColourChange;
			u32 CharIndex = 0;
			if ( !TLString::ReadNextFloatArray( PropertyData, CharIndex, ColourChange.GetData(), ColourChange.GetSize() ) )
				continue;

			//	go through all the colours and multiply them
			u32 LastColourIndex = (u32)LastVertexIndex < pMesh->GetColours().GetSize() ? (u32)LastVertexIndex : pMesh->GetColours().GetSize();
			for ( u32 v=FirstVertexIndex;	v<LastColourIndex;	v++ )
			{
				TColour& MeshColour = pMesh->GetColours().ElementAt(v);
				MeshColour -= ColourChange;
			}
		}
		else
		{
			//	unhandled property
			TLDebug_Print( TString("Unknown property \"%s\" in asset data import in TAM file.", PropertyName.GetData() ) );
		}
	}
	
	return SyncTrue;
}

