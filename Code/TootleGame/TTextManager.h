
#pragma once

#include <TootleCore/TPtr.h>
#include <TootleCore/TManager.h>

namespace TLText
{
	class TTextManager;

	extern TPtr<TTextManager> g_pTextManager;
}


class TLText::TTextManager : public TManager
{
public:
	TTextManager(TRefRef ManagerRef) :
	  TManager(ManagerRef),
		m_LanguageRef("eng")
	{
	}

	// Language access
	void		SetLanguage(TRefRef LanguageRef)	{ m_LanguageRef = LanguageRef; OnLanguageChanged(); }
	TRefRef		GetLanguage()						{ return m_LanguageRef; }
	
	Bool		IsLanguageSupported(TRefRef LanguageRef);

	// Text access
	Bool		GetText(TRefRef TextRef, TString& Text);
	Bool		GetText(TRefRef TextRef, TRefRef TextSection, TString& Text);

protected:
	
	virtual SyncBool	Initialise();

	virtual void		ProcessMessage(TLMessaging::TMessage& Message);
	virtual void		OnEventChannelAdded(TRefRef refPublisherID, TRefRef refChannelID);

	void				OnLanguageChanged();

private:
	TRef			m_LanguageRef;		// Current selected language

	TArray<TRef>	m_TextFiles;		// List of text files
};