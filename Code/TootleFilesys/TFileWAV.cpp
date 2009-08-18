#include "TFileWAV.h"

#include <TootleAsset/TAudio.h>

using namespace TLFileSys;


//---------------------------------------------------------
//	
//---------------------------------------------------------
TFileWAV::TFileWAV(TRefRef FileRef,TRefRef FileTypeRef) :
	TFile	( FileRef, FileTypeRef ),
	m_bHeaderLoaded(FALSE)
{
}



TFileWAV::~TFileWAV()
{
	ShutdownExport(TRUE);
}


//--------------------------------------------------------------
//	turn this file into a audio asset then turn that to a asset file
//--------------------------------------------------------------	
SyncBool TFileWAV::ExportAsset(TPtr<TLAsset::TAsset>& pAsset,Bool& Supported)
{
	Supported = TRUE;
	
	if( !InitExport(pAsset) )
	{
		ShutdownExport(TRUE);
		return SyncFalse;
	}
	
	// Do main export
	SyncBool ExportResult = UpdateExport(pAsset);
	if ( ExportResult == SyncWait )
		return SyncWait;
	
	//	failed, shutdown and return
	if ( ExportResult == SyncFalse )
	{
		ShutdownExport(TRUE);
		return SyncFalse;
	}
	
	//	print out some debug info
	TString FontRefString;
	pAsset->GetAssetRef().GetString( FontRefString );
	TLDebug_Print( TString("%d in the %s WAV", pAsset.GetObjectPointer<TLAsset::TAudio>()->GetNumberOfChannels(), FontRefString.GetData() ) );
	
	//	cleanup, but dont delete audio
	ShutdownExport( FALSE );
	
	return SyncTrue;
	
}	


//--------------------------------------------------------------
//	init export
//--------------------------------------------------------------
Bool TFileWAV::InitExport(TPtr<TLAsset::TAsset>& pAsset)
{
	if ( !pAsset )
	{
		//	create a new audio asset
		pAsset = new TLAsset::TAudio( this->GetFileRef() );
		if ( !pAsset )
			return FALSE;
	}
	
	return TRUE;
}

//--------------------------------------------------------------
//	continue export
//--------------------------------------------------------------
SyncBool TFileWAV::UpdateExport(TPtr<TLAsset::TAsset>& pAsset)
{
	TPtr<TLAsset::TAudio> pAudio = pAsset;
	if ( !pAudio )
	{
		TLDebug_Break("Audio expected");
		return SyncFalse;
	}

	// Header loaded?  Load header if not
	if(!m_bHeaderLoaded)
	{
		if(!LoadHeader(pAsset))
		{
			TLDebug_Print("Failed to load WAV file header");
			return SyncFalse;
		}
		
		TLDebug_Print("WAV file header loaded");
		
		m_bHeaderLoaded = TRUE;
	}

			
	// Load the audio data
	// TODO: Make Async by copying a set number of bytes each frame until complete
	TBinary DataBinary;
	ReadAll( DataBinary ); 
	pAudio->RawAudioDataBinary().Copy( DataBinary );
	
	/*
	s32 CharactersToExport = 3;
	
	//	loop through all the characters in the font
	while ( m_NextGlyphIndex != 0 && --CharactersToExport > 0 )                                            
	{
		u16 Character = m_NextGlyphCharacter & 0xffff;
		u32 GlyphIndex = m_NextGlyphIndex;
		
		//	get next glyph now so we can easily use continue; in the loop
		m_NextGlyphCharacter = FT_Get_Next_Char( m_pFace, m_NextGlyphCharacter, &m_NextGlyphIndex );
		
		//	if this is a character we dont support... skip
		if ( !TLFileFreetype::IsCharRequired( m_NextGlyphCharacter ) )
			continue;
		
		//	load a glyph into the face's current-glyph
		//	always load vectors, never using bitmap fonts
		u32 LoadFlags = FT_LOAD_NO_BITMAP;
		u32 Error = FT_Load_Glyph( m_pFace, GlyphIndex, LoadFlags );
		if ( Error )
			continue;
		
		//	get current-glyph
		TLFreetype::FT_GlyphSlot& pGlyph = m_pFace->glyph;
		if ( !pGlyph )
			continue;
		
		//	add this glyph mesh to the font
		TPtr<TLAsset::TMesh> pGlyphMesh = pFont->AddGlyph( Character );
		if ( !pGlyphMesh )
			continue;
		
		//	get a leadin/out bounding box from the glyph info
		float3 LeadIn( 0.f, 0.f, 0.f );
		float3 LeadOut( (float)pGlyph->advance.x * ScaleDown, (float)pGlyph->advance.y * ScaleDown, 0.f );
		TLMaths::TBox LeadInOutBoundsBox( LeadIn, LeadOut );
		
		//	write into the mesh as arbirtry data
		TPtr<TBinaryTree> pLeadInOutBoxData = pGlyphMesh->GetData("LeadBox",TRUE);
		if ( pLeadInOutBoxData )
			pLeadInOutBoxData->Write( LeadInOutBoundsBox );
		
		//	vectorise glyph
		//		if ( !pVectoriser->MakeMesh( pGlyphMesh, TLGlutTessellator::OutsetType_Front, 1.0f, 0.f ) )
		if ( !VectoriseGlyph( pGlyphMesh, pGlyph->outline ) )
		{
			//	if the glyph is empty, we add it anyway  for spacing (eg. space)
			TLDebug_Print( TString("Added empty character %c(0x%04x) to font", (char)Character, Character ) );
			
			//	the lead in/out bounding box is set, but we need to set an empty (but valid) bounds box
			//pGlyphMesh->GetBoundsBox().Set( float3(0,0,0), float3(0,0,0) );
			//pGlyphMesh->GetBoundsSphere().Set( float3(0,0,0), 0.f );
			continue;
		}
		
		//	 some debugging
		//TLDebug_Print( TString("Added character %c(0x%04x) to font", (char)Character, Character ) );
	}
	
	//	still got glyphs to import
	if ( m_NextGlyphIndex != 0 )
		return SyncWait;
	*/
	
	return SyncTrue;
}


//--------------------------------------------------------------
// Reads and checks the header data to ensure we are dealing with a valid audio WAV file.
//--------------------------------------------------------------
Bool TFileWAV::LoadHeader(TPtr<TLAsset::TAsset>& pAsset)
{
	TLDebug_Print("Loading audio header");

	TPtr<TLAsset::TAudio> pAudio = pAsset;

	ResetReadPos();
	
	descriptor	WAVDescriptor;
	
	Read(WAVDescriptor);
	
	TTempString TempStr;
	
	TempStr.Append(WAVDescriptor.riff, 4);
	
	TLDebug_Print(TempStr);
	
	if(TempStr != "RIFF")
		return FALSE;

	// We have a valid wave file header
	// Double check the wave portion in the descriptor	
	TempStr.Empty();
	TempStr.Append(WAVDescriptor.wave, 4);
	
	if(TempStr != "WAVE")
		return FALSE;
	
	// Read the WAV format header info
	formatdataheader WAVFormatHeader;
	
	Read(WAVFormatHeader);

	// Check the size is correct.  If not bail out
	if(WAVFormatHeader.uSize != sizeof(formatdata))
		return FALSE;
	
	formatdata WAVFormatData;
	
	// Read teh Format info
	Read(WAVFormatData);
	
	// Check the format.  Should be 1 for PCM i.e. no compression.
	// TODO: Handle other forms of compression
	
	if(WAVFormatData.uFormat != 1)
		return FALSE;
	
	// Debug print the header info

	// Everything OK so proceed to copy the actual audio header data to the asset.
	pAudio->SetSampleRate(WAVFormatData.uSampleBitrate);
	pAudio->SetNumberOfChannels(WAVFormatData.uNumChannels);
	pAudio->SetBitsPerSample(WAVFormatData.uBitsPerSample);

#ifdef _DEBUG
	TLDebug_Print( TString("Bitrate: %d", WAVFormatData.uSampleBitrate) );
	TLDebug_Print( TString("Channel Count: %d", WAVFormatData.uNumChannels) );
	TLDebug_Print( TString("BitsPerSample: %d", WAVFormatData.uBitsPerSample) );
#endif

	// Now read the header for the actual audio data
	audiodataheader WAVAudioHeader;
	Read(WAVAudioHeader);
	
	// Set the size of the audio data
	pAudio->SetSize(WAVAudioHeader.uSize);
	
	TLDebug_Print("Audio header loaded successfully");

	return TRUE;
}

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
void TFileWAV::ShutdownExport(Bool bDeleteAudio)
{
	
}


