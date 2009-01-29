#include "TVirtualFileSys.h"
#include <TootleAsset/TMesh.h>
#include <TootleFileSys/TFileAsset.h>


namespace TLDebugFile
{
	SyncBool		LoadDebugFile_Asset(TPtr<TLFileSys::TFile>& pFile,TPtr<TLAsset::TAsset> pAsset);	//	create a cube mesh
	SyncBool		LoadDebugFile_MeshCube(TPtr<TLFileSys::TFile>& pFile);		//	create a cube mesh
	SyncBool		LoadDebugFile_MeshQuad(TPtr<TLFileSys::TFile>& pFile);		//	create a cube mesh
	SyncBool		LoadDebugFile_MeshSphere(TPtr<TLFileSys::TFile>& pFile);	//	create a cube mesh
};




TLFileSys::TVirtualFileSys::TVirtualFileSys(TRefRef FileSysRef,TRefRef FileSysTypeRef) :
	TFileSys			( FileSysRef, FileSysTypeRef )
{
}


//-------------------------------------------------------
//
//-------------------------------------------------------
SyncBool TLFileSys::TVirtualFileSys::LoadFileList()
{
	//	
	CreateFileInstance("d_sphere.asset");
	CreateFileInstance("d_cube.asset");
	CreateFileInstance("d_quad.asset");

	//	update time stamp of file list
	FinaliseFileList();

	return SyncTrue;
}


//-------------------------------------------------------
//	
//-------------------------------------------------------
SyncBool TLFileSys::TVirtualFileSys::LoadFile(TPtr<TLFileSys::TFile>& pFile)
{
	//	file is already loaded (if not broken)
	if ( pFile->IsLoaded() == SyncTrue )
		return SyncTrue;

	if ( pFile->GetFileRef() == TRef("d_cube") )
	{
		return TLDebugFile::LoadDebugFile_MeshCube( pFile );
	}

	if ( pFile->GetFileRef() == TRef("d_sphere") )
	{
		return TLDebugFile::LoadDebugFile_MeshSphere( pFile );
	}

	if ( pFile->GetFileRef() == TRef("d_quad") )
	{
		return TLDebugFile::LoadDebugFile_MeshQuad( pFile );
	}

	//	unknown debug file name
	TLDebug_Break( TString("VirtualFileSys: Dont know how to create the file %s", pFile->GetFilename().GetData() ) );

	return SyncFalse;
}


//---------------------------------------------------------
//	create a cube mesh file
//---------------------------------------------------------
SyncBool TLDebugFile::LoadDebugFile_Asset(TPtr<TLFileSys::TFile>& pFile,TPtr<TLAsset::TAsset> pAsset)
{	
	//	is "loaded" now
	pAsset->SetLoadingState( TLAsset::LoadingState_Loaded );

	TPtr<TLFileSys::TFileAsset> pAssetFile;
	if ( pFile->GetFileTypeRef() == "Asset" )
	{
		pAssetFile = pFile;
	}
	else
	{
		pAssetFile = new TLFileSys::TFileAsset( pAsset->GetAssetRef(), "Asset" );
	}

	//	export asset to asset file
	SyncBool ExportResult = pAsset->Export( pAssetFile );
	if ( ExportResult != SyncTrue )
		return ExportResult;

	//	need to export the asset file data to a normal file
	ExportResult = pAssetFile->Export();
	if ( ExportResult != SyncTrue )
		return ExportResult;

	//	if pFile isnt the asset file, then we need to copy all our asset file data into it
	if ( pAssetFile != pFile )
	{
		//	copy file data and attributes (flags, timestamp etc)
		TPtr<TLFileSys::TFile> pAssetFileAsFile = pAssetFile;
		pFile->Copy( pAssetFileAsFile );
		pFile->OnFileLoaded();
	}
	else
	{
		pFile->OnFileLoaded();
		pAssetFile->SetNeedsImport(FALSE);
	}

	//	reset read pos to begging ready for something to read it
	//	gr: dont do this any more, we use the read pos to determine if we've attempted to read data post-import
	//pFile->ResetReadPos();

	return SyncTrue;
}


//---------------------------------------------------------
//	create a cube mesh file
//---------------------------------------------------------
SyncBool TLDebugFile::LoadDebugFile_MeshCube(TPtr<TLFileSys::TFile>& pFile)
{
	//	create a mesh asset at runtime, then turn that into a file. 
	//	Very convuluted :) but there's reasons for it
	TPtr<TLAsset::TMesh> pMesh = new TLAsset::TMesh( pFile->GetFileRef() );
	
	//	generate a cube
	TLMaths::TBox Box( float3(0,0,0), float3(1,1,1) );
	pMesh->GenerateCube( Box );

	//	give it some pretty colours
	//pMesh->GenerateRainbowColours();

	return LoadDebugFile_Asset( pFile, pMesh );
}



//---------------------------------------------------------
//	create a quad mesh file
//---------------------------------------------------------
SyncBool TLDebugFile::LoadDebugFile_MeshQuad(TPtr<TLFileSys::TFile>& pFile)
{
	//	create a mesh asset at runtime, then turn that into a file. 
	//	Very convuluted :) but there's reasons for it
	TPtr<TLAsset::TMesh> pMesh = new TLAsset::TMesh( pFile->GetFileRef() );
	
	//	generate a cube
	TFixedArray<float3,4> QuadOutline;
	QuadOutline[0] = float3( 0, 0, 0 );
	QuadOutline[1] = float3( 1, 0, 0 );
	QuadOutline[2] = float3( 1, 1, 0 );
	QuadOutline[3] = float3( 0, 1, 0 );
	pMesh->GenerateQuad( QuadOutline, TColour(1.f, 1.f, 1.f, 1.f ) );

	return LoadDebugFile_Asset( pFile, pMesh );
}


//---------------------------------------------------------
//	create a sphere mesh file
//---------------------------------------------------------
SyncBool TLDebugFile::LoadDebugFile_MeshSphere(TPtr<TLFileSys::TFile>& pFile)
{
	TPtr<TLAsset::TMesh> pMesh = new TLAsset::TMesh( pFile->GetFileRef() );
	
	//	generate a cube
	pMesh->GenerateSphere( 1.f );

	return LoadDebugFile_Asset( pFile, pMesh );
}

//---------------------------------------------------------
//	create a new empty file into file system if possible - if the filesys is read-only we cannot add external files and this fails
//---------------------------------------------------------
TPtr<TLFileSys::TFile> TLFileSys::TVirtualFileSys::CreateFile(const TString& Filename,TRefRef FileTypeRef)
{
	//	create/get existing file
	TPtr<TLFileSys::TFile> pNewFile = CreateFileInstance( Filename, FileTypeRef );

	//	error creating file
	if ( !pNewFile )
		return pNewFile;

	//	update timestamp
	pNewFile->SetTimestampNow();

	return pNewFile;
}


//---------------------------------------------------------
//	add this file into the file system if it's not there
//---------------------------------------------------------
SyncBool TLFileSys::TVirtualFileSys::WriteFile(TPtr<TFile>& pFile)
{
	if ( !pFile )
	{
		TLDebug_Break("File expected");
		return SyncFalse;
	}

	//	create/get existing file
	TPtr<TFile> pNewFile = CreateFileInstance( pFile->GetFilename() );

	//	error creating file
	if ( !pNewFile )
		return SyncFalse;

	//	only write data if the data is different
	if ( pFile != pNewFile )
	{
		//	copy data across
		pNewFile->GetData().Copy( pFile->GetData() );
	}

	//	update timestamp
	pNewFile->SetTimestampNow();

	return SyncTrue;
}


//---------------------------------------------------------
//	delete file from file sys
//---------------------------------------------------------
SyncBool TLFileSys::TVirtualFileSys::DeleteFile(TRefRef FileRef)
{
	//	just remove from file list
	if ( !RemoveInstance( FileRef ) )
		return SyncFalse;

	return SyncTrue;
}
