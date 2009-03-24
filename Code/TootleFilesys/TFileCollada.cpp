#include "TFileCollada.h"
#include <TootleAsset/TMesh.h>
#include <TootleMaths/TTessellate.h>
#include "TLFile.h"



namespace TLCollada
{
	class TriangleInput
	{
	public:
		TriangleInput();
		Bool			Import(TXmlTag& Tag,const TPtrArray<TGeometryData>& GeometryData);

	public:
		TRef			m_Semantic;	//	"VERTEX" "NORMAL" "TEXCOORD"
		u32				m_Offset;	//	which index of the data is for this vertex input
		TGeometryData*	m_pData;	//	the actual data for this vertex input (ie. pointer to array of floats for positions)
	};


}

	
TLCollada::VertexMap::VertexMap() :
	m_VertexIndex	( -1 ),
	m_Position		( -1 ),
	m_TexCoord		( -1 )
{
}


TLCollada::TriangleInput::TriangleInput() :
	m_Offset	( 0 ),
	m_pData		( NULL )
{
}


Bool TLCollada::TriangleInput::Import(TXmlTag& Tag,const TPtrArray<TGeometryData>& GeometryData)
{
	m_Semantic = *Tag.GetProperty("semantic");

	//	get data source ID
	const TString* pSourceDataID = Tag.GetProperty("source");
	if ( !pSourceDataID )
		return FALSE;

	//	get data source
	TPtr<TGeometryData> pGeometryData = GeometryData.FindPtr( *pSourceDataID );

	//	source has a source (link, not actual data)
	while ( pGeometryData && pGeometryData->m_pSourceDataID != NULL )
	{
		//	find the link
		pGeometryData = GeometryData.FindPtr( *pGeometryData->m_pSourceDataID );
	}

	//	missing geometry data
	if ( !pGeometryData )
	{
		TLDebug_Break("Triangle input has invalid data reference");
		return FALSE;
	}

	//	save link
	m_pData = pGeometryData;


	//	offset is default to zero
	m_Offset = 0;

	//	get alternative value
	const TString* pOffsetString = Tag.GetProperty("offset");
	if ( pOffsetString )
	{
		s32 Offset;
		if ( !pOffsetString->GetInteger( Offset ) )
			return FALSE;
		if ( Offset < 0 )
		{
			TLDebug_Break("Invalid offset");
			return FALSE;
		}
		m_Offset = Offset;
	}

	return TRUE;
}

	

TLCollada::TGeometry::TGeometry() : 
	m_pGeometryName		( NULL )
{
}

Bool TLCollada::TGeometry::Import(TXmlTag& Tag,TPtrArray<TLCollada::TMaterial>& Materials)
{
	//	import names
	m_pGeometryName = Tag.GetProperty("name");	//	optional

	//	require ID
	const TString* pGeometryID = Tag.GetProperty("id");
	if ( !pGeometryID )
	{
		TLDebug_Break("<geometry> missing ID");
		return FALSE;
	}

	//	copy ID and prefix with # to make it easier to match
	m_GeometryID = "#";
	m_GeometryID.Append( *pGeometryID );

	//	get the "mesh" child
	TXmlTag* pMeshTag = Tag.GetChild("mesh");
	if ( !pMeshTag )
	{
		TLDebug_Break("<mesh> tag expected");
		return FALSE;
	}

	//	import the geometry data first
	TLDebug_Print("Collada: Importing geometry source data...");
	TPtrArray<TXmlTag> DataTags;
	pMeshTag->GetChildren("source", DataTags );
	pMeshTag->GetChildren("vertices", DataTags );
	for ( u32 i=0;	i<DataTags.GetSize();	i++ )
	{
		TXmlTag* pDataTag = DataTags[i];
		TPtr<TGeometryData> pGeometryData = new TGeometryData();
		if ( !pGeometryData->Import( *pDataTag ) )
		{
			TLDebug_Break("failed to import geometry data");
			return FALSE;
		}

		//	add to list
		m_GeometryData.Add( pGeometryData );
	}

	//	create a mesh to import the geometry into
	m_pMesh = new TLAsset::TMesh( TRef(*m_pGeometryName) );

	//	import triangles into the mesh
	TLDebug_Print("Collada: Importing geometry triangles...");
	TPtrArray<TXmlTag> TriangleTags;
	pMeshTag->GetChildren("triangles", TriangleTags );
	for ( u32 t=0;	t<TriangleTags.GetSize();	t++ )
	{
		TXmlTag* pTriangleTag = TriangleTags[t];

		//	get material
		const TString* pMaterialID = pTriangleTag->GetProperty("Material");
		TPtr<TLCollada::TMaterial>* ppMaterial = pMaterialID ? Materials.Find( *pMaterialID ) : NULL;
		TLCollada::TMaterial* pMaterial = ppMaterial ? (*ppMaterial).GetObject() : NULL;

		//	get all the different mappings of triangle data -> vertex data
		u32 TriangleDataStep = 1;	//	step is the biggest offset from the inputs, + 1
		
		//	gr: store relavant inputs - dont need all of them (maybe for future expansion)
		TPtr<TLCollada::TriangleInput> pPositionInput;
		//TPtr<TLCollada::TriangleInput> pNormalInput;
		TPtr<TLCollada::TriangleInput> pTexCoordInput;

		TLDebug_Print("Collada: Importing geometry triangle inputs...");
		TPtrArray<TXmlTag> InputTags;
		pTriangleTag->GetChildren("input",InputTags);
		for ( u32 i=0;	i<InputTags.GetSize();	i++ )
		{
			TPtr<TLCollada::TriangleInput> pInput = new TLCollada::TriangleInput();
			if ( !pInput->Import( *InputTags[i], m_GeometryData ) )
			{
				TLDebug_Break("failed to import <triangles> input tag");
				return FALSE;
			}

			//	gr: make sure we take ALL offsets into account to get the correct stride of the triangle data
			if ( pInput->m_Offset+1 > TriangleDataStep )
				TriangleDataStep = pInput->m_Offset+1;

			//	store input
			if ( pInput->m_Semantic == "Vertex" )
				pPositionInput = pInput;
			//else if ( pInput->m_Semantic == "Normal" )
			//	pNormalInput = pInput;
			else if ( pInput->m_Semantic == "TexCoord" )
				pTexCoordInput = pInput;
			//	else unwanted input
		}

		//	pull out triangle data
		TXmlTag* pDataTag = pTriangleTag->GetChild("p");
		if ( !pDataTag )
		{
			TLDebug_Break("<triangles> tag is missing data tag <p>");
			return FALSE;
		}

		//	get vertex's colour
		TColour* pVertexColour = pMaterial ? &pMaterial->m_Colour : NULL;

		TLDebug_Print("Collada: Importing geometry triangle's triangles...");
		u32 DataIndex = 0;				//	which bit of data are we on (ie. index in the array)
		u32 TriangleComponentIndex = 0;	//	which of the 3 parts of the triangle are we on

		//	keep reading through all the data and get each index of data
		s32 InputIndex = -1;
		u32 CharIndex = 0;
		TLCollada::VertexMap CurrentTriangleVertexMap;
		TLAsset::TMesh::Triangle CurrentTriangle;
		while ( TLString::ReadNextInteger( pDataTag->GetDataString(), CharIndex, InputIndex ) )
		{
			u32 Offset = DataIndex % TriangleDataStep;
			DataIndex++;

			//	set the vertex index for each kind of input at this offset
			if ( pPositionInput && pPositionInput->m_Offset == Offset )
				CurrentTriangleVertexMap.m_Position = InputIndex;

			if ( pTexCoordInput && pTexCoordInput->m_Offset == Offset )
				CurrentTriangleVertexMap.m_TexCoord = InputIndex;

			//	more components to read of the current vertex map
			if ( Offset != TriangleDataStep-1 )
				continue;

			//	all parts of the vertex map have been read, get vertex
			//	see if the vertex map configuration already exists (which means vertex has already been created)
			TLCollada::VertexMap* pExistingVertexMap = m_VertexMap.Find( CurrentTriangleVertexMap );

			//	doesnt already exist, create vertex and entry
			if ( !pExistingVertexMap )
			{
				//	get vertex's data
				const float3* pPosition = pPositionInput ? pPositionInput->m_pData->GetData<float3>( CurrentTriangleVertexMap.m_Position ) : NULL;
				const float2* pTexCoord = pTexCoordInput ? pTexCoordInput->m_pData->GetData<float2>( CurrentTriangleVertexMap.m_TexCoord ) : NULL;
	
				if ( !pPosition )
				{
					TLDebug_Break("Missing vertex position");
					return FALSE;
				}

				//	create vertex
				CurrentTriangleVertexMap.m_VertexIndex = m_pMesh->AddVertex( *pPosition, pVertexColour, pTexCoord );

				//	add to list
				s32 MapIndex = m_VertexMap.Add( CurrentTriangleVertexMap );
				pExistingVertexMap = &m_VertexMap[MapIndex];
			}

			//	add to triangle's data
			if ( pExistingVertexMap->m_VertexIndex == -1 )
			{
				TLDebug_Break("Missing vertex index from mapping");
				return FALSE;
			}

			//	update triangle
			CurrentTriangle[TriangleComponentIndex] = pExistingVertexMap->m_VertexIndex;

			//	more of the triangle to fetch?
			if ( TriangleComponentIndex < 2 )
			{
				TriangleComponentIndex++;
				continue;
			}

			//	triangle complete! add to mesh
			m_pMesh->GetTriangles().Add( CurrentTriangle );
			m_pMesh->OnPrimitivesChanged();

			//	reset temporary triangle for debugging
			CurrentTriangle.Set( 0xffff, 0xffff, 0xffff );
			TriangleComponentIndex = 0;
		}

		//	clean map for next set of triangles
		m_VertexMap.Empty();
	}

	return TRUE;
}






TLCollada::TGeometryData::TGeometryData() :
	m_pDataName			( NULL ),
	m_pSourceDataID		( NULL ),
	m_ElementSize		( 0 ),
	m_ElementCount		( 0 )
{
}



//---------------------------------------------------------------
/*
	    <source id="Box01-texcoords" name="texcoord">
                    <float_array id="Box01-texcoords-array" count="8">0 1 0 0 1 1 1 0</float_array>
                    <technique_common>
                        <accessor source="#Box01-texcoords-array" count="4" stride="2">
                            <param name="S" type="float" />
                            <param name="T" type="float" />
                        </accessor>
                    </technique_common>
                </source>

	or
                <vertices id="Box01-geometry-vertices">
                    <input semantic="POSITION" source="#Box01-positions" />
                </vertices>
*/
//---------------------------------------------------------------
Bool TLCollada::TGeometryData::Import(TXmlTag& Tag)
{
	//	read id (required)
	const TString* pIDString = Tag.GetProperty("id");
	if ( !pIDString )
	{
		TLDebug_Break("expected geometry data's ID");
		return FALSE;
	}
	
	//	pre-pend with # to make matching much easier
	m_DataID = "#";
	m_DataID.Append( *pIDString );

	//	get name (optional)
	m_pDataName = Tag.GetProperty("name");
	
	//	using external data
	TXmlTag* pInputTag = Tag.GetChild("input");
	if ( pInputTag )
	{
		m_pSourceDataID = pInputTag->GetProperty("source");
		if ( !m_pSourceDataID )
		{
			TLDebug_Break("source id expected");
			return FALSE;
		}

		//	dont use the rest of the data as we're using external data
		return TRUE;
	}

	//	import data
	TXmlTag* pFloatArrayTag = Tag.GetChild("float_array");
	if ( !pFloatArrayTag )
	{
		TLDebug_Break("missing geometry data");
		return FALSE;
	}

	TXmlTag* pTechniqueTag = Tag.GetChild("technique_common");
	if ( !pTechniqueTag )
	{
		TLDebug_Break("<technique_common> expected");
		return FALSE;
	}

	TXmlTag* pAccessorTag = pTechniqueTag->GetChild("accessor");
	if ( !pAccessorTag )
	{
		TLDebug_Break("<accessor> expected");
		return FALSE;
	}

	const TString* pCountString = pAccessorTag->GetProperty("count");
	const TString* pStrideString = pAccessorTag->GetProperty("stride");
	s32 Count = 0;
	s32 Stride = 1;
	if ( pCountString )		pCountString->GetInteger( Count );
	if ( pStrideString )	pStrideString->GetInteger( Stride );

	//	pre-alloc data for speed
	m_Data.AddAllocSize( Count * Stride );

	//	gr: currently... dont need process the accessors as we're blindly using it... maybe need to use the stride though
	u32 CharIndex = 0;
	float f;
	while ( TLString::ReadNextFloat( pFloatArrayTag->GetDataString(), CharIndex, f, TRUE ) )
	{
		m_Data.Add( f );
	}

	//	check size against expected size
	if ( m_Data.GetSize() != Count*Stride )
	{
		TLDebug_Break("Read wrong number of elements from <float_array>");
		return FALSE;
	}

	//	set element size
	m_ElementSize = Stride;

	return TRUE;
}


//---------------------------------------------------------------
//	fetch real data and check we're using the right data type
//---------------------------------------------------------------
const float* TLCollada::TGeometryData::GetFloatData(u32 ElementIndex,u32 ElementSize) const
{
	//	wrong element size 
	if ( ElementSize != m_ElementSize )
	{
		TLDebug_Break("Requesting data with wrong size destination");
		return NULL;
	}

	//	get real data index
	u32 Index = ElementIndex * m_ElementSize;

	return &m_Data.ElementAtConst( Index );
}





Bool TLCollada::TMaterial::Import(TXmlTag& MaterialTag,TXmlTag& LibraryEffectsTag)
{
	/*
	<material id="right door" name="right door">
		<instance_effect url="#right door-effect" />
	</material>
	*/
	//	get id
	const TString* pIDString = MaterialTag.GetProperty("id");
	if ( !pIDString )
	{
		TLDebug_Break("<Material> missing id");
		return FALSE;
	}
	m_MaterialID = *pIDString;

	//	get effect id
	TXmlTag* pMaterialEffectTag = MaterialTag.GetChild("Instance_effect");
	const TString* pEffectString = pMaterialEffectTag ? pMaterialEffectTag->GetProperty("url") : NULL;
	if ( !pEffectString )
	{
		TLDebug_Break("Material missing effect link");
		return FALSE;
	}

	//	remove the # from the effect string
	TTempString MaterialEffectID = *pEffectString;
	if ( MaterialEffectID.GetCharAt(0) == '#' )
		MaterialEffectID.RemoveCharAt(0,1);

	//	get the effect info
	TXmlTag* pEffectTag = NULL;
	for ( u32 e=0;	e<LibraryEffectsTag.GetChildren().GetSize();	e++ )
	{
		TXmlTag* pChildEffectTag = LibraryEffectsTag.GetChildren().ElementAt(e);
		const TString* pChildEffectID = pChildEffectTag->GetProperty("id");
		if ( pChildEffectTag->GetTagName() != "effect" || !pChildEffectID )
			continue;

		if ( (*pChildEffectID) != MaterialEffectID )
			continue;

		//	found the effect
		pEffectTag = pChildEffectTag;
		break;
	}

	//	couldnt find effect tag
	if ( !pEffectTag )
	{
		TLDebug_Break("Cannot find effect info for material");
		return FALSE;
	}

	//	get profile tag
	TXmlTag* pProfileTag = pEffectTag->GetChild("profile_COMMON");
	TXmlTag* pTechniqueTag = pProfileTag ? pProfileTag->GetChild("technique") : NULL;
	TXmlTag* pPhongTag = pTechniqueTag ? pTechniqueTag->GetChild("phong") : NULL;
	if ( !pPhongTag )
	{
		TLDebug_Break("Couldnt find the <effect><profile_common><technique><phong> tag. Phong is the only one i know in use at the moment!");
		return FALSE;
	}

	//	just use the ambient colour for our material colour;
	//	http://en.wikipedia.org/wiki/Phong_shading
	TXmlTag* pAmbientTag = pPhongTag->GetChild("ambient");
	TXmlTag* pAmbientColourTag = pAmbientTag ? pAmbientTag->GetChild("color") : NULL;
	if ( !pAmbientColourTag )
	{
		TLDebug_Break("Missing ambient/colour tag from phong effect");
		return FALSE;
	}

	//	read out colour
	u32 CharIndex = 0;
	if ( !TLString::ReadNextFloat( pAmbientColourTag->GetDataString(), CharIndex, m_Colour.GetRgba() ) )
	{
		TLDebug_Break("Failed to read colour from phong ambient colour tag");
		return FALSE;
	}

	return TRUE;	
}









TLFileSys::TFileCollada::TFileCollada(TRefRef FileRef,TRefRef FileTypeRef) :
	TFileXml			( FileRef, FileTypeRef )
{
}



//--------------------------------------------------------
//	import the XML and convert from collada to mesh
//--------------------------------------------------------
SyncBool TLFileSys::TFileCollada::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
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


	//	get the root svg tag (all items are under this and contains scene info)
	TPtr<TXmlTag> pRootTag = m_XmlData.GetChild("collada");

	//	malformed SVG maybe
	if ( !pRootTag )
		return SyncFalse;

	//	import material information
	TLDebug_Print("Collada: Importing material libraries...");
	if ( !ImportMaterialLibraries( *pRootTag ) )
		return SyncFalse;

	//	import geometry information
	TLDebug_Print("Collada: Importing geometry libraries...");
	if ( !ImportGeometryLibraries( *pRootTag ) )
		return SyncFalse;

	//	create mesh asset
	TPtr<TLAsset::TMesh> pNewMesh = new TLAsset::TMesh( GetFileRef() );
	pNewMesh->SetLoadingState( TLAsset::LoadingState_Loaded );

	//	pull out scene information (ie. what geometry is used, what matierals theyre bound to etc)
	TLDebug_Print("Collada: Importing scene...");
	if ( !ImportScene( *pNewMesh, *pRootTag ) )
		return SyncFalse;

	//	assign resulting asset
	pAsset = pNewMesh;
	pNewMesh->CalcBoundsBox();

	return SyncTrue;
}



//--------------------------------------------------------
//	import actual final mesh from nodes in the scene
//--------------------------------------------------------
Bool TLFileSys::TFileCollada::ImportScene(TLAsset::TMesh& Mesh,TXmlTag& RootTag)
{
	//	get the library tag
	TXmlTag* pLibrarySceneTag = RootTag.GetChild("library_visual_scenes");
	if ( !pLibrarySceneTag )
	{
		TLDebug_Break("Collada has no scene library");
		return FALSE;
	}

	//	get visual scene - there may be more than one... maybe for animation keyframes or something?
	TXmlTag* pVisualSceneTag = pLibrarySceneTag->GetChild("visual_scene");
	if ( !pVisualSceneTag )
	{
		TLDebug_Break("Visual scene expected");
		return FALSE;
	}

	//	get all the nodes to combine into one mesh
	TPtrArray<TXmlTag> Nodes;
	pVisualSceneTag->GetChildren("Node", Nodes );
	if ( !Nodes.GetSize() )
	{
		TLDebug_Break("No nodes in scene");
		return FALSE;
	}

	//	merge nodes into one large mesh
	for ( u32 n=0;	n<Nodes.GetSize();	n++ )
	{
		//	get geometry
		TXmlTag* pGeometryTag = Nodes[n]->GetChild("instance_geometry");
		const TString* pGeometryID = pGeometryTag ? pGeometryTag->GetProperty("url") : NULL;
		TPtr<TLCollada::TGeometry>* ppGeometry = pGeometryID ? m_Geometry.Find( *pGeometryID ) : NULL;
		TLCollada::TGeometry* pGeometry = ppGeometry ? (*ppGeometry).GetObject() : (TLCollada::TGeometry*)NULL;
		if ( !pGeometry )
		{
			TLDebug_Break("Cannot find node's geometry");
			return FALSE;
		}

		//	missing mesh
		if ( !pGeometry->m_pMesh )
		{
			TLDebug_Break("Geometry has no mesh");
			continue;
		}

		//	check the name of the node to see if it's labelled as a datum
		const TString* pNodeName = Nodes[n]->GetProperty("Name");
		if ( pNodeName )
		{
			//	is named as a datum
			TRef DatumRef,ShapeRef;
			if ( TLString::IsDatumString( *pNodeName, DatumRef, ShapeRef ) )
			{
				//	create a datum on the mesh... from the geometry mesh['s bounds]
				CreateDatum( Mesh, DatumRef, ShapeRef, *pGeometry->m_pMesh );
				continue;
			}
		}

		//	extract material to get vertex colour info for mesh
		/*
                   <bind_material>
                        <technique_common>
                            <instance_material symbol="body" target="#body" />
                        </technique_common>
                    </bind_material>
		*/

		//	merge this geometry's mesh into our mesh
		Mesh.Merge( *pGeometry->m_pMesh );
	}

	return TRUE;
}


//--------------------------------------------------------
//	import material information
//--------------------------------------------------------
Bool TLFileSys::TFileCollada::ImportMaterialLibraries(TXmlTag& RootTag)
{
	//	get the library tag
	TXmlTag* pLibraryTag = RootTag.GetChild("library_materials");
	TXmlTag* pLibraryEffectsTag = RootTag.GetChild("library_effects");
	if ( !pLibraryTag || !pLibraryEffectsTag )
	{
		TLDebug_Break("Collada has no materials and/or no effects");
		return FALSE;
	}

	//	import all the material children
	for ( u32 c=0;	c<pLibraryTag->GetChildren().GetSize();	c++ )
	{
		TXmlTag* pMaterialTag = pLibraryTag->GetChildren().ElementAt(c);
		if ( pMaterialTag->GetTagName() != "material" )
			continue;

		//	create new geometry
		TLDebug_Print("Collada: Importing material...");
		TPtr<TLCollada::TMaterial> pNewMaterial = new TLCollada::TMaterial;
		if ( !pNewMaterial->Import( *pMaterialTag, *pLibraryEffectsTag ) )
			return FALSE;

		//	add to list of geometries
		m_Materials.Add( pNewMaterial );
	}

	return TRUE;
}

//--------------------------------------------------------
//	import geometry information
//--------------------------------------------------------
Bool TLFileSys::TFileCollada::ImportGeometryLibraries(TXmlTag& RootTag)
{
	//	get the library tag
	TXmlTag* pLibraryGeometryTag = RootTag.GetChild("library_geometries");
	if ( !pLibraryGeometryTag )
	{
		TLDebug_Break("Collada has no geometry");
		return FALSE;
	}

	//	go through all the <geometry> tags
	for ( u32 c=0;	c<pLibraryGeometryTag->GetChildren().GetSize();	c++ )
	{
		TXmlTag* pGeometryTag = pLibraryGeometryTag->GetChildren().ElementAt(c);
		if ( pGeometryTag->GetTagName() != "geometry" )
			continue;

		//	create new geometry
		TLDebug_Print("Collada: Importing geometry...");
		TPtr<TLCollada::TGeometry> pNewGeometry = new TLCollada::TGeometry;
		if ( !pNewGeometry->Import( *pGeometryTag, m_Materials ) )
			return FALSE;

		//	add to list of geometries
		m_Geometry.Add( pNewGeometry );
	}

	return TRUE;
}



//--------------------------------------------------------
//	
//--------------------------------------------------------
Bool TLFileSys::TFileCollada::CreateDatum(TLAsset::TMesh& Mesh,TRefRef DatumRef,TRefRef DatumShapeType,const TLAsset::TMesh& GeometryMesh)
{
	TPtr<TLMaths::TShape> pShape;

	if ( DatumShapeType == TLMaths::TSphere2D::GetTypeRef() )
	{
		//	get box of points for extents
		TLMaths::TBox2D Box;
		Box.Accumulate( GeometryMesh.GetVertexes() );
		pShape = new TLMaths::TShapeSphere2D( Box );
	}
	else if ( DatumShapeType == TLMaths::TSphere::GetTypeRef() )
	{
		//	get box of points for extents
		TLMaths::TBox Box;
		Box.Accumulate( GeometryMesh.GetVertexes() );
		pShape = new TLMaths::TShapeSphere( Box );
	}
	else if ( DatumShapeType == TLMaths::TOblong2D::GetTypeRef() )
	{
		//	gr: currently, simple mehtod just works with 4-point meshes
		if ( GeometryMesh.GetVertexes().GetSize() != 4 )
		{
			TLDebug_Break("Cannot create an oblong datum shape from a polygon/outline with other than 4 points");
			return FALSE;
		}

		TLMaths::TOblong2D Oblong;
		TFixedArray<float2,4>& Corners = Oblong.GetBoxCorners();
		Corners[0] = GeometryMesh.GetVertexes()[0].xy();
		Corners[1] = GeometryMesh.GetVertexes()[1].xy();
		Corners[2] = GeometryMesh.GetVertexes()[2].xy();
		Corners[3] = GeometryMesh.GetVertexes()[3].xy();

		//	set as explicitly valid
		Oblong.SetValid();

		pShape = new TLMaths::TShapeOblong2D( Oblong );
	}
	else if ( DatumShapeType == TLMaths::TCapsule2D::GetTypeRef() )
	{
		//	get box of points for extents
		TLMaths::TBox2D Box;
		Box.Accumulate( GeometryMesh.GetVertexes() );
		pShape = new TLMaths::TShapeCapsule2D( Box );
	}
	else if ( DatumShapeType == TLMaths::TBox2D::GetTypeRef() )
	{
		//	get box of points for extents
		TLMaths::TBox2D Box;
		Box.Accumulate( GeometryMesh.GetVertexes() );
		pShape = new TLMaths::TShapeBox2D( Box );
	}
	else if ( DatumShapeType == TLMaths::TBox::GetTypeRef() )
	{
		//	get box of points for extents
		TLMaths::TBox Box;
		Box.Accumulate( GeometryMesh.GetVertexes() );
		pShape = new TLMaths::TShapeBox( Box );
	}

	//	created okay
	if ( pShape )
	{
		//	add datum
		Mesh.AddDatum( DatumRef, pShape );
		return TRUE;
	}

#ifdef _DEBUG
	TTempString Debug_String("Unknown datum shape type ");
	DatumShapeType.GetString( Debug_String );
	TLDebug_Break(Debug_String);
#endif
	return FALSE;
}


