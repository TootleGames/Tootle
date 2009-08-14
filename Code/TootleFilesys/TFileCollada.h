/*------------------------------------------------------
	
	Collada file format (simple vector graphics)
	Converts to mesh

-------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include "TFileXml.h"
#include <TootleAsset/TMesh.h>


namespace TLFileSys
{
	class TFileCollada;
};


namespace TLCollada
{
	class TGeometry;
	class TGeometryData;
	class VertexMap;
	class TMaterial;
}



class TLCollada::VertexMap
{
public:
	VertexMap();

	FORCEINLINE Bool	operator==(const VertexMap& Vertex) const		{	return (m_Position == Vertex.m_Position) && (m_TexCoord == Vertex.m_TexCoord);	}

public:
	s32			m_VertexIndex;	//	vertex index
	s32			m_Position;		//	position data index
	s32			m_TexCoord;		//	texcoord data index
};

TLCore_DeclareIsDataType( TLCollada::VertexMap );



class TLCollada::TGeometryData
{
public:
	TGeometryData();
	Bool					Import(TXmlTag& Tag);

	template<typename FTYPE>
	const FTYPE*			GetData(u32 ElementIndex) const				{	return (const FTYPE*)GetFloatData( ElementIndex, FTYPE::GetSize() );	}

	FORCEINLINE Bool		operator==(const TString& DataID) const		{	return (m_DataID == DataID);	}

protected:
	const float*			GetFloatData(u32 ElementIndex,u32 ElementSize) const;	//	fetch real data and check we're using the right data type

public:
	const TString*			m_pDataName;		//	name of the type of data - usually "Position" or "normal" or "texcoord"
	TString					m_DataID;			//	unique name of the data (ID) - copied so we pre-pend with # to make matching id's easier
	
	const TString*			m_pSourceDataID;	//	our data is stored in a different GeometryData object

	TArray<float>			m_Data;				//	array of the data (always assume its floats - or convert to float if its not)
	u32						m_ElementSize;		//	"stride" - number of compoenents per item - (ie. 3 for position, 2 for texcoord)
	u32						m_ElementCount;		//	"count" == datasize/elementsize (ie. N positions)
};



class TLCollada::TGeometry
{
public:
	TGeometry();
	Bool						Import(TXmlTag& Tag,TPtrArray<TLCollada::TMaterial>& Materials);	//	import geometry data

	FORCEINLINE Bool			operator==(const TString& GeometryID) const		{	return (m_GeometryID == GeometryID);	}

public:
	const TString*					m_pGeometryName;	//	name of the geometry - always seems to match ID
	TString							m_GeometryID;		//	unique name of the geometry (ID) - copied so we pre-pend with # to make matching id's easier
	TPtrArray<TGeometryData>		m_GeometryData;		//	array of all the geometry data for this mesh
	TArray<TLCollada::VertexMap>	m_VertexMap;		//	mapping of vertexes we've already added
	TPtr<TLAsset::TMesh>			m_pMesh;			//	mesh created from all the geometry data
};




class TLCollada::TMaterial
{
public:
	Bool						Import(TXmlTag& MaterialTag,TXmlTag& LibraryEffectsTag);	//	import 

	FORCEINLINE Bool			operator==(const TString& MaterialID) const		{	return (m_MaterialID == MaterialID);	}

public:
	TString						m_MaterialID;		//	unique name of the geometry (ID) - copied so we pre-pend with # to make matching id's easier
	TColour						m_Colour;
};




//---------------------------------------------------------
//	collada xml file
//---------------------------------------------------------
class TLFileSys::TFileCollada : public TLFileSys::TFileXml
{
public:
	TFileCollada(TRefRef FileRef,TRefRef FileTypeRef);

	virtual TRef		GetFileExportAssetType() const										{	return TRef_Static4(M,e,s,h);	}
	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported);			//	import the XML and convert from Collada to mesh

protected:
	Bool				ImportGeometryLibraries(TXmlTag& RootTag);				//	import geometry information
	Bool				ImportMaterialLibraries(TXmlTag& RootTag);				//	import material information
	Bool				ImportScene(TLAsset::TMesh& Mesh,TXmlTag& RootTag);		//	
	Bool				CreateDatum(TLAsset::TMesh& Mesh,TRefRef DatumRef,TRefRef DatumShapeType,const TLAsset::TMesh& GeometryMesh);

protected:
	TPtrArray<TLCollada::TGeometry>		m_Geometry;		//	list of geometry (meshes) we've created
	TPtrArray<TLCollada::TMaterial>		m_Materials;	//	list of materials
};

	
	

