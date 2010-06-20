
#pragma once

#include <TootleCore/TLDebug.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TManager.h>
#include <TootleCore/TLanguage.h>
#include <TootleAsset/TText.h>


namespace TLText
{
	class TTextManager;

	extern TPtr<TTextManager> g_pTextManager;
}


class TLText::TTextManager : public TLCore::TManager
{
public:
	TTextManager(TRefRef ManagerRef) :
		TLCore::TManager	( ManagerRef ),
		m_LanguageRef		( TLLanguage_RefEnglish )
	{
	}

	// Language access
	void				SetLanguage(TRefRef LanguageRef)	{ m_LanguageRef = LanguageRef; OnLanguageChanged(); }
	TRefRef				GetLanguage()						{ return m_LanguageRef; }
	
	FORCEINLINE Bool	IsLanguageSupported(TRefRef LanguageRef)	{ return m_LanguagesSupported.Exists(LanguageRef); }
	TArray<TRef>&		GetSupportedLanguages()						{ return m_LanguagesSupported; }

	// Text access
	Bool				GetText(TRefRef TextRef, TString& Text,TLAsset::TText::TTextReplaceTable* pReplaceTable=NULL);						//	
	Bool				GetText(TRefRef TextRef, TRefRef TextSection, TString& Text,TLAsset::TText::TTextReplaceTable* pReplaceTable=NULL);	//	gr: text section is a text asset!

protected:
	
	virtual SyncBool	Initialise();

	virtual void		ProcessMessage(TLMessaging::TMessage& Message);
	virtual void		OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

	void				OnLanguageChanged();

private:
	TRef				m_LanguageRef;		// Current selected language

	THeapArray<TRef>	m_TextFiles;			// List of text files
	THeapArray<TRef>	m_LanguagesSupported;	// List of supported languages as Tref's
};