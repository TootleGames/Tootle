#include "TFileAsset.h"

#ifdef _DEBUG
#define IMPORT_CHILDREN_PER_UPDATE	20
#else
#define IMPORT_CHILDREN_PER_UPDATE	200
#endif

namespace TLFileSys
{
	const TRef		g_TootFileRef = TRef_Static(T,o,o,t,D);	//	serves as version type too - changing this makes old file types incompatible
};



TLFileSys::TFileAsset::TFileAsset(TRefRef FileRef,TRefRef FileTypeRef) :
	TFile			( FileRef, FileTypeRef ),
	m_NeedsImport	( FALSE ),
	m_NeedsExport	( FALSE ),
	m_AssetData		( "Tooot" )	//	gr: this WAS the fileref, but i dont think it's ever referenced as that, i've changed it, just so when you look at it in any way we might know where the data is from. The FileRef is per-game instance anyway so probably shouldnt get saved
{
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
	{
		//	catch data that has no data and no children
		if ( pChild->GetSize() == 0 )
		{
			TTempString Debug_String("Importing data with no children and no data: ");
			pChild->GetDataRef().GetString( Debug_String );
			TLDebug_Warning( Debug_String );
		}

		return TRUE;
	}

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
		TPtr<TBinaryTree> pNewChild = pChild->AddChild( ChildHeader.m_DataRef );
		pNewChild->SetDataTypeHint( ChildHeader.m_DataType, TRUE );

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
	if ( !GetNeedsImport() )
	{
		TLDebug_Break("Warning: redundant import...");
	}

	//	if we need to be exported at this point then the asset file data (tree) has changed
	//	we are going to lose our changes...
	if ( GetNeedsExport() )
	{
		TLDebug_Break("Trying to import asset file, but the Asset file has changed. Will lose changes.");
	}

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

		/*
		TTempString Debug_String("TFileAsset ");
		this->Debug_GetString( Debug_String );
		Debug_String.Append(" imported. Data below.");
		TLDebug_Print( Debug_String );
		this->GetData().Debug_PrintTree();
		*/
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
		TBinaryTree& Child = *Children[c];
		
		TBinary ChildTreeData;
		if ( Child.GetChildren().GetSize() )
		{
			ExportTree( Child.GetChildren(), ChildTreeData );
		}

		//	setup header
		SectionHeader ChildHeader( Child );

		//	write header
		Data.Write( ChildHeader );

		//	write child's data
		Data.Write( Child.GetData() );

		//	write child's children data if we have any children
		if ( Child.GetChildren().GetSize() )
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
	if ( !GetNeedsExport() )
	{
		TLDebug_Break("Warning: redundant export...");
	}

	//	if we need to be imported at this point then the plain TBinary/TFile data has changed since we last imported...
	//	So we're out of sync, the TBinaryTree doesn't reflect what's in the file
	if ( GetNeedsImport() )
	{
		TLDebug_Break("Trying to export asset file, but the file data has changed so we're out of sync");
		return SyncFalse;
	}

	TFile* pFile = this;

	//	clean out existing data in file
	pFile->Empty();

	//	asset file isnt setup
	if ( !m_Header.IsValid() )
	{
		TLDebug_Break("Assetfile isnt setup");
		return SyncFalse;
	}

	/*
	TLDebug_Print("Exporting asset file to file...");
	GetData().Debug_PrintTree();
	*/

	//	init up header
	m_Header.m_DataCheckSum = 0;
	m_Header.m_DataLength = 0;
	
	//	write root data
	pFile->Write( GetAssetData().GetData() );

	//	write all the child data
	ExportTree( GetAssetData().GetChildren(), pFile->GetData() );

	//	update header
	m_Header.m_DataLength = pFile->GetSize();
	m_Header.m_DataCheckSum = pFile->GetChecksum();

	//	try to compress
	if ( pFile->Compress() == SyncTrue )
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
//	copy contents of other asset file into this (note: uses ReferenceDataTree, does NOT duplicate data)
//---------------------------------------------------------
Bool TLFileSys::TFileAsset::CopyAssetFileData(TFileAsset& OtherAssetFile)
{
	//	copy header
	this->m_Header = OtherAssetFile.GetHeader();

	//	copy asset data,
	this->GetAssetData().ReferenceDataTree( OtherAssetFile.GetAssetData() );

	//	as we're overwriting our asset data we can assume the plain data is out of date now
	//	so if the source is up to date, copy it, otherwise it's out of date and needs exporting
	if ( OtherAssetFile.GetNeedsExport() )
	{
		//	the plain file data is out of date
		GetData().Empty();
		SetNeedsImport( FALSE );
		SetNeedsExport( TRUE );
	}
	else
	{
		//	the plain file data is up to date, so copy it
		GetData().Copy( OtherAssetFile.GetData() );
		SetNeedsImport( FALSE );
		SetNeedsExport( FALSE );
	}

	//	copy importer
	//	gr: dont know if we need to do this
	if ( this->m_pImporter || OtherAssetFile.m_pImporter )
	{
		if ( TLDebug_Break("Should we do this?") )
		{
			this->m_pImporter = OtherAssetFile.m_pImporter;
		}
	}

	return TRUE;
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

	if ( m_TootFileRef != TLFileSys::g_TootFileRef )
		return FALSE;

	if ( !m_AssetType.IsValid() )
		return FALSE;

	return TRUE;
}


//---------------------------------------------------------
//	do import 
//	(todo: make async)
//---------------------------------------------------------
TRef TLFileSys::TLFileAssetImporter::Mode_Init::Update(float Timestep)
{
	TLFileSys::TFile* pFile = GetFile();

	//	file missing??
	if ( !pFile )
	{
		TLDebug_Break("file expected");
		return "failed";
	}

	//	reset read pos
	pFile->ResetReadPos();

	//	empty out the existing tree data
	GetAssetFile()->GetAssetData().Empty();

	//	file is empty
	if ( pFile->GetData().GetSize() == 0 )
	{
		TLDebug_Break("File is empty");
		return "Failed";
	}

	//	read file header
	TFileAsset::Header& Header = GetHeader();
	if ( !pFile->Read( Header ) )
	{
		#ifdef _DEBUG
		TTempString Debug_String("Asset file ");
		Debug_String.Append( pFile->GetFilename() );
		Debug_String.Append(" failed to read header data");
		TLDebug_Break( Debug_String );
		#endif
		return "Failed";
	}

	//	validate header ref
	if ( !Header.IsValid() )
	{
		#ifdef _DEBUG
		TTempString Debug_String("Asset file ");
		Debug_String.Append( pFile->GetFilename() );
		Debug_String.Append(" header ref is wrong/old: ");
		GetHeader().m_TootFileRef.GetString( Debug_String );
		Debug_String.Append(". Should be: ");
		TLFileSys::g_TootFileRef.GetString( Debug_String );
		TLDebug_Break( Debug_String );
		#endif
		return "Failed";
	}

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
		pFile->Read( pAssetFile->GetAssetData().GetData() );
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

		#ifdef _DEBUG
		TTempString Debug_String( pFile->GetFilename() );
		Debug_String.Append(" imported child ");
		ChildHeader.m_DataRef.GetString( Debug_String );
		
		if ( ChildHeader.m_ChildCount > 0 )
			Debug_String.Appendf(" (has %d children)", ChildHeader.m_ChildCount );
			
		TLDebug_Print( Debug_String );
		#endif
		
		//	section is corrupt
		if ( !ChildHeader.m_DataRef.IsValid() )
			return "Failed";

		//	create new section and copy data into it
		TPtr<TBinaryTree> pChild = pAssetFile->GetAssetData().AddChild( ChildHeader.m_DataRef );
		if ( !pChild )
			return "Failed";
		pChild->SetDataTypeHint( ChildHeader.m_DataType, TRUE );

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
		pAssetFile->GetAssetData().Empty();
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
