/*
 *  TLanguage.h
 *  TootleCore
 *
 *	Defines the language ID's
 *
 *  Created by Duane Bradbury on 06/02/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once


#define TLLanguage_RefEnglish		TRef_Static3(e,n,g)
#define TLLanguage_RefUsa			TRef_Static3(u,s,a)
#define TLLanguage_RefFrench		TRef_Static3(f,r,e)
#define TLLanguage_RefGerman		TRef_Static3(g,e,r)
#define TLLanguage_RefSpanish		TRef_Static3(s,p,a)
#define TLLanguage_RefItalian		TRef_Static3(i,t,a)
#define TLLanguage_RefDutch			TRef_Static3(n,e,d)
#define TLLanguage_RefJapanese		TRef_Static3(j,a,p)

namespace TLLanguage
{
	const TRef	g_LanguageRef_English	= TLLanguage_RefEnglish;
	const TRef	g_LanguageRef_Usa		= TLLanguage_RefUsa;
	const TRef	g_LanguageRef_French	= TLLanguage_RefFrench;	
	const TRef	g_LanguageRef_German	= TLLanguage_RefGerman;	
	const TRef	g_LanguageRef_Spanish	= TLLanguage_RefSpanish;	
	const TRef	g_LanguageRef_Italian	= TLLanguage_RefItalian;	
	const TRef	g_LanguageRef_Dutch		= TLLanguage_RefDutch;	
	const TRef	g_LanguageRef_Japanese	= TLLanguage_RefJapanese;	
}
