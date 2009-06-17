
#include "TTextManager.h"

#include <TootleCore/TEventChannel.h>
#include <TootleCore/TLanguage.h>
#include <TootleAsset/TLAsset.h>
#include <TootleAsset/TText.h>
#include <TootleCore/TCoreManager.h>

namespace TLText
{
	TPtr<TTextManager> g_pTextManager = NULL;
}


using namespace TLText;

// Get the text from *any* of the text sections.  First one found is returned, if successfully found.
Bool TTextManager::GetText(TRefRef TextRef, TString& Text,TLAsset::TText::TTextReplaceTable* pReplaceTable)
{
	for(u32 uIndex = 0; uIndex < m_TextFiles.GetSize(); uIndex++)
	{
		// Text section is the same as the text file ref
		TRefRef TextSection = m_TextFiles.ElementAtConst(uIndex);

		// Get the text asset file from the asset system
		TPtr<TLAsset::TText> pAsset = TLAsset::GetAsset(TextSection, TRUE);

		if ( !pAsset )
			continue;
		
		//	find matching text
		if ( pReplaceTable )
		{
			if( pAsset->GetText(m_LanguageRef, TextRef, Text, *pReplaceTable ) )
				return TRUE;
		}
		else
		{
			if( pAsset->GetText(m_LanguageRef, TextRef, Text ) )
				return TRUE;
		}
	}

	// Not found
	return FALSE;
}


Bool TTextManager::GetText(TRefRef TextRef, TRefRef TextSection, TString& Text,TLAsset::TText::TTextReplaceTable* pReplaceTable) 
{
	// Is the section loaded?  The section is the same ref as a text file ref
	if(!m_TextFiles.Exists(TextSection))
		return FALSE;

	// Get the asset file and return the GetText routine form that
	TPtr<TLAsset::TText> pAsset = TLAsset::GetAsset(TextSection, TRUE);

	if(!pAsset.IsValid())
		return FALSE;

	if ( pReplaceTable )
		return pAsset->GetText(m_LanguageRef, TextRef, Text, *pReplaceTable );
	else
		return pAsset->GetText(m_LanguageRef, TextRef, Text );
}

void TTextManager::OnLanguageChanged()	
{ 
	// TODO: Send out a language changed message
	// so things can update themselves such as text dialogues
}


SyncBool TTextManager::Initialise()
{
	// Add some languages that we will support
	//TODO: This will need updating so that it is either generated from
	// the game or lists all possibly supported languages.
	m_LanguagesSupported.Add( TLLanguage::g_LanguageRef_Usa );
	m_LanguagesSupported.Add( TLLanguage::g_LanguageRef_English );
	//m_LanguagesSupported.Add( TLLanguage::g_LanguageRef_French );
	//m_LanguagesSupported.Add( TLLanguage::g_LanguageRef_German );
	//m_LanguagesSupported.Add( TLLanguage::g_LanguageRef_Spanish );
	//m_LanguagesSupported.Add( TLLanguage::g_LanguageRef_Italian );
	//m_LanguagesSupported.Add( TLLanguage::g_LanguageRef_Dutch );
	//m_LanguagesSupported.Add( TLLanguage::g_LanguageRef_Japanese );

	// Get the language from the core manager as specified on the hardware
	//	gr: should get this from the TApplication::GetLanguage ?
	TRef LanguageRef = TLCore::g_pCoreManager->GetHardwareLanguage();
	
	if(IsLanguageSupported(LanguageRef))
	{
		SetLanguage(LanguageRef);
	}
	else
	{
		TLDebug_Break("What do we do here?");
	}	

	return TManager::Initialise();
}

void TTextManager::OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID)
{
	if(refPublisherID == "ASSET")
	{
		TLMessaging::g_pEventChannelManager->SubscribeTo(this, refPublisherID, refChannelID); 
	}
	
	// Super event channel routine
	TManager::OnEventChannelAdded(refPublisherID, refChannelID);
}

void TTextManager::ProcessMessage(TLMessaging::TMessage& Message)
{
	if(Message.GetMessageRef() == "OnAssetChanged")
	{
		TRef AssetRef, AssetType, State;
		Bool bSuccess;

		// Is the asset a text type?
		Message.Read(AssetRef);
		Message.Read(AssetType);
		Message.Read(State);
		Message.Read(bSuccess);

		if(AssetType == TRef("Text"))
		{
			if(State == TRef("Load"))
			{
				// Load successful?
				if(bSuccess == TRUE)
				{
					// Add the asset ref to our list of loaded assets
					m_TextFiles.Add(AssetRef);
				}
			}
			else if(State == TRef("Unload"))
			{
				// Remove asset from the file list
				m_TextFiles.Remove(AssetRef);
			}
		}

		return;
	}

	TManager::ProcessMessage(Message);
}


