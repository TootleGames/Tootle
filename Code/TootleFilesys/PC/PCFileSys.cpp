/*
 *  PCFileSys.cpp
 *  TootleFilesys
 *
 *  Created by Duane Bradbury on 22/09/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "PCFileSys.h"
#include <TootleCore/TLCore.h>
#include <TootleGui/TLGui.h>


Bool TLFileSys::Platform::GetAssetDirectory(TTempString& AssetDir)
{
	
	TTempString applicationdir;
	
	if(GetApplicationURL(applicationdir))
	{
		// For a developer build we need to remove components until we find the project 
		// directory and then append 'Asset'
		// For a release build we will need to use the installation path and append the 'Asset' directory to it

		///////////////////////////////////////////////////////////////////////////
		// Developer version
		///////////////////////////////////////////////////////////////////////////

		applicationdir.Append( "Assets\\" );

		AssetDir = applicationdir;
		///////////////////////////////////////////////////////////////////////////

		
		return TRUE;
	}	
	
	return FALSE;
	
}

Bool TLFileSys::Platform::GetAssetSubDirectory(TTempString& AssetDir, const TTempString& Subdirectory)
{
	TTempString tmpassetdir;
	
	if(GetAssetDirectory(tmpassetdir))
	{
		// Append the subdirectory
		tmpassetdir.Append(Subdirectory);

		// Append the seperator
		tmpassetdir.Append("\\");

		AssetDir = tmpassetdir;

		return TRUE;
	}

	return FALSE;
}


Bool TLFileSys::Platform::GetUserDirectory(TTempString& UserDir)
{
	return GetAssetSubDirectory(UserDir, "User");
}		

Bool TLFileSys::Platform::GetApplicationURL(TTempString& url)
{
	TString NewDirectory = TLGui::GetAppExe();
	TLFileSys::GetParentDir( NewDirectory );

	url = NewDirectory;

	return TRUE;
}