/*------------------------------------------------------
	
	for TAM files; "tootle asset markup"
	It's like a way of batching files, or taking other 
	assets and turning them into new assets etc
	take a jpg, make it black and white, or take a mesh
	and scale it, or remove polygons etc

-------------------------------------------------------*/
#pragma once

#include "TFile.h"
#include "TFileXml.h"


namespace TLFileSys
{
	class TFileAssetMarkup;
}

namespace TLAsset
{
	class TMesh;
}

//---------------------------------------------------------
//	SVG xml file that converts to a mesh
//---------------------------------------------------------
class TLFileSys::TFileAssetMarkup : public TLFileSys::TFileXml
{
public:
	TFileAssetMarkup(TRefRef FileRef,TRefRef FileTypeRef);

	virtual SyncBool	ExportAsset(TPtr<TLAsset::TAsset>& pAsset,TRefRef ExportAssetType);
	virtual void		GetSupportedExportAssetTypes(TArray<TRef>& SupportedTypes) const	{	SupportedTypes.Add( TRef() );	}

protected:
	SyncBool			ImportMesh(TPtr<TLAsset::TMesh> pMesh,TPtr<TXmlTag>& pTag);	//	generate mesh TAM tag
	SyncBool			ImportMesh_ImportTag(TPtr<TLAsset::TMesh>& pMesh,TPtr<TXmlTag>& pImportTag);	//	generate mesh TAM tag
	SyncBool			ImportMesh_ImportTag_Part(TPtr<TLAsset::TMesh>& pMesh,const TPtr<TLAsset::TMesh>& pImportMesh,TRefRef ImportMeshPartRef,TPtr<TXmlTag>& pImportTag);	//	import part of a mesh into this mesh
};

