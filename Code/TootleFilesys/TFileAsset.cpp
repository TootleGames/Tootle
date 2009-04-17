#include "TFileAsset.h"

#ifdef _DEBUG
#define IMPORT_CHILDREN_PER_UPDATE	20
#else
#define IMPORT_CHILDREN_PER_UPDATE	200
#endif

namespace TLFileSys
{
	const TRef		g_TootFileRef = TRef_Static(T,o,o,t,C);	//	serves as version type too - changing this makes old file types incompatible
};



TLFileSys::TFileAsset::TFileAsset(TRefRef FileRef,TRefRef FileTypeRef) :
	TFile	( FileRef, FileTypeRef ),
	m_Data	( FileRef )
{
	//	initially needs importing
	SetNeedsImport( TRUE );
}


//---------------------------------------------------------
//	recursive import of child tree
//---------------------------------------------------------
Bool TLFileSys::TFileAsset::ImportTree(TPtr<TBinaryTree>& pChild,SectionHeader& Header,TBinary& Data)
{
	//	read child's data
	if ( !Data.Read( pChild->GetData() ) )
		return FALSE;

	//	reset read pos ready for import later
	//pChild->GetData().ResetReadPos();

	//	no children, no more data
	if ( Header.m_ChildCount == 0 )
		return TRUE;

	//	if we have children read the data out
	TBinary ChildTreeData;
	if ( !Data.Read( ChildTreeData ) )
		return FALSE;

	//	reset read pos
	ChildTreeData.ResetReadPos();

	//	if child has children, import them too
	for ( u32 c=0;	c<Header.m_ChildCount;	c++ )
	{
		//	read new child's header
		SectionHeader ChildHeader;

		//	failed to read section header info? then fail entirely. sections should still match up
		if ( !ChildTreeData.Read( ChildHeader ) )
			return FALSE;

		//	create new child
		TPtr<TBinaryTree> pNewChild = pChild->AddChild( ChildHeader.m_SectionRef );

		//	import that child
		if ( !ImportTree( pNewChild, ChildHeader, ChildTreeData ) )
			return FALSE;
	}

	return TRUE;
}


//---------------------------------------------------------
//	turn this TFile into a binary tree
//---------------------------------------------------------
SyncBool TLFileSys::TFileAsset::Import()
{
	//	create importer if we havent got one
	if ( !m_pImporter )
	{
		//	file is broken
		if ( m_Header.m_Flags( TFileAsset::Broken ) )
			return SyncFalse;

		//	if we dont need to import, dont
		if ( GetNeedsImport() == FALSE )
			return SyncTrue;

		//	create importer
		m_pImporter = new TFileAssetImporter(this);
	}

	//	update importer
	m_pImporter->Update(0.f);
	SyncBool ImportResult = m_pImporter->GetResult();

	//	finished processing, cleanup importer
	if ( ImportResult != SyncWait )
	{
		m_pImporter = NULL;
		OnImportFinished(ImportResult);
	}

	return ImportResult;
}	




//---------------------------------------------------------
//	recursive import of child tree
//---------------------------------------------------------
Bool TLFileSys::TFileAsset::ExportTree(TPtrArray<TBinaryTree>& Children,TBinary& Data)
{
	//	write children
	for ( u32 c=0;	c<Children.GetSize();	c++ )
	{
		//	create data for child
		TPtr<TBinaryTree>& pChild = Children[c];
		
		TBinary ChildTreeData;
		if ( pChild->GetChildren().GetSize() )
		{
			ExportTree( pChild->GetChildren(), ChildTreeData );
		}

		//	setup header
		SectionHeader ChildHeader;
		ChildHeader.m_ChildCount = pChild->GetChildren().GetSize();
		ChildHeader.m_Length = pChild->GetData().GetSize();
		ChildHeader.m_SectionRef = pChild->GetDataRef();

		//	write header
		Data.Write( ChildHeader );

		//	write child's data
		Data.Write( pChild->GetData() );

		//	write child's children data if we have any children
		if ( pChild->GetChildren().GetSize() )
		{
			Data.Write( ChildTreeData );
		}
	}

	return TRUE;
}


//---------------------------------------------------------
//	export the binary tree to this TFile
//---------------------------------------------------------
SyncBool TLFileSys::TFileAsset::Export()
{
	TFile* pFile = this;

	//	clean out existing data in file
	pFile->Empty();

	//	asset file isnt setup
	if ( !m_Header.IsValid() )
	{
		TLDebug_Break("Assetfile isnt setup");
		return SyncFalse;
	}

	//	init up header
	m_Header.m_DataCheckSum = 0;
	m_Header.m_DataLength = 0;
	
	//	write root data
	pFile->Write( GetData().GetData() );

	//	write all the child data
	ExportTree( GetData().GetChildren(), pFile->GetData() );

	//	update header
	m_Header.m_DataLength = pFile->GetSize();
	m_Header.m_DataCheckSum = pFile->GetChecksum();

	//	try to compress
	if ( pFile->Compress() )
		m_Header.m_Flags.Set( TFileAsset::Compressed );

	//	inject header to start of the file
	pFile->WriteToStart( m_Header );

	//	the file is "laoded" now... the assetfile binary tree is now binary in the TFile
	pFile->SetTimestampNow();
	pFile->SetIsLoaded( SyncTrue );
	pFile->GetFlags().Clear( TLFileSys::TFile::OutOfDate );
	SetNeedsExport(FALSE);

	return SyncTrue;
}	



//---------------------------------------------------------
//	write this section to binary[file]
//---------------------------------------------------------
TLFileSys::TFileAsset::Header::Header() :
	m_DataLength		( 0 ),
	m_DataCheckSum		( 0 )
{
}


//---------------------------------------------------------
//	check if the header has been setup
//---------------------------------------------------------
Bool TLFileSys::TFileAsset::Header::IsValid() const
{
	if ( !m_TootFileRef.IsValid() )
		return FALSE;

	if ( !m_AssetType.IsValid() )
		return FALSE;

	return TRUE;
}




//---------------------------------------------------------
//	
//---------------------------------------------------------
TRef TLFileSys::TLFileAssetImporter::Mode_Init::Update(float Timestep)
{
	TLFileSys::TFile* pFile = GetFile();

	//	check file hasn't been parsed
	if ( pFile->GetReadPos() != -1 )
	{
		if ( TLDebug_Break("Asset file read pos not at begining") )
			return "Failed";
	}

	//	file is empty
	if ( pFile->GetData().GetSize() == 0 )
		return "Failed";

	//	empty out the existing tree data
	GetAssetFile()->GetData().Empty();

	pFile->ResetReadPos();
		
	//	read file header
	TFileAsset::Header& Header = GetHeader();
	if ( !pFile->Read( Header ) )
		return "Failed";

	//	validate header ref
	if ( GetHeader().m_TootFileRef != TLFileSys::g_TootFileRef )
		return "Failed";

	//	decompress file
	return "Decompress";
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
TRef TLFileSys::TLFileAssetImporter::Mode_Decompress::Update(float Timestep)
{
	//	no decompression needed, skip!
	if ( !GetHeader().m_Flags( TFileAsset::Compressed ) )
		return "ImportChild";
	
	SyncBool DecompressResult = GetFile()->Decompress();
	if ( DecompressResult == SyncWait )
		return TRef();

	//	failed to decompress
	if ( DecompressResult == SyncFalse )
		return "Failed";

	//	decompressed, update header flags
	GetHeader().m_Flags.Clear( TFileAsset::Compressed );
	return "ImportChild";
}


//---------------------------------------------------------
//	
//---------------------------------------------------------
TRef TLFileSys::TLFileAssetImporter::Mode_ImportChild::Update(float Timestep)
{
	TLFileSys::TFile* pFile = GetFile();
	TLFileSys::TFileAsset* pAssetFile = GetAssetFile();

	//	read out root data if first time
	if ( m_CurrentChild == -1 )
	{
		pFile->Read( pAssetFile->GetData().GetData() );
		m_CurrentChild = 0;
	}

	s32 ParseCount = IMPORT_CHILDREN_PER_UPDATE;

	while ( ParseCount-- > 0 )
	{
		//	no more data to parse - done
		if ( pFile->GetSizeUnread() <= 0 )
			return "Finished";

		//	parse another child(secion)

		//	read section header
		TFileAsset::SectionHeader ChildHeader;

		//	failed to read section header info? then fail entirely. sections should still match up
		if ( !pFile->Read( ChildHeader ) )
			return "Failed";
		
		TTempString FileRefString;
		pFile->GetFileRef().GetString( FileRefString );
		TTempString ChildSectionRefString;
		ChildHeader.m_SectionRef.GetString( ChildSectionRefString );
		TLDebug_Print( TString("%s: Imported child %s. Len: %d. Children: %d", FileRefString.GetData(), ChildSectionRefString.GetData(), ChildHeader.m_Length, ChildHeader.m_ChildCount ) );
		

		//	section is corrupt
		if ( !ChildHeader.m_SectionRef.IsValid() )
			return "Failed";

		//	create new section and copy data into it
		TPtr<TBinaryTree> pChild = pAssetFile->GetData().AddChild( ChildHeader.m_SectionRef );
		if ( !pChild )
			return "Failed";

		//	import child data
		if ( !pAssetFile->ImportTree( pChild, ChildHeader, pFile->GetData() ) )
			return "Failed";

		m_CurrentChild++;
	}

	//	loop to import again
	return TRef();
}


//---------------------------------------------------------
//	finished okay - set flags on asset file
//---------------------------------------------------------
Bool TLFileSys::TLFileAssetImporter::Mode_Finished::OnBegin(TRefRef PreviousMode)
{
	//	all done
	GetAssetFile()->SetNeedsImport(FALSE);
	GetHeader().m_Flags.Clear( TFileAsset::Broken );
	return TRUE;
}

//---------------------------------------------------------
//	failed - set flags on asset file
//---------------------------------------------------------
Bool TLFileSys::TLFileAssetImporter::Mode_Failed::OnBegin(TRefRef PreviousMode)
{
	//	failed during parsing
	GetHeader().m_Flags.Set( TFileAsset::Broken );
	
	TFileAsset*	pAssetFile = GetAssetFile();
	if ( pAssetFile )
	{
		pAssetFile->GetData().Empty();
	}
	
	return TRUE;
}


//---------------------------------------------------------
//
//---------------------------------------------------------
TLFileSys::TFileAssetImporter::TFileAssetImporter(TFileAsset* pAssetFile) :
	m_pAssetFile	( pAssetFile )
{
	//	setup jobs
	AddMode<TLFileAssetImporter::Mode_Init>("Init");
	AddMode<TLFileAssetImporter::Mode_Decompress>("Decompress");
	AddMode<TLFileAssetImporter::Mode_ImportChild>("ImportChild");
	AddMode<TLFileAssetImporter::Mode_Finished>("Finished");
	AddMode<TLFileAssetImporter::Mode_Failed>("Failed");
}

//---------------------------------------------------------
//	work out what state of the process we're in
//---------------------------------------------------------
SyncBool TLFileSys::TFileAssetImporter::GetResult()
{
	if ( !m_pAssetFile )
	{
		TLDebug_Break("Asset file expect");
		return SyncFalse;
	}

	TRef CurrentModeRef = GetCurrentModeRef();
	
	//	moved into no-mode
	if ( !CurrentModeRef.IsValid() )
		return SyncFalse;

	//	failed 
	if ( CurrentModeRef == "Failed" )
		return SyncFalse;

	//	finished okay
	if ( CurrentModeRef == "Finished" )
		return SyncTrue;

	//	still processing
	return SyncWait;
}
