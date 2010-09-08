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


Bool TLFileSys::Platform::GetAssetDirectory(TString& AssetDir)
{
	TTempString applicationdir;
	if ( !GetApplicationURL(applicationdir) )
		return false;

	// For a developer build we need to remove components until we find the project 
	// directory and then append 'Asset'
	// For a release build we will need to use the installation path and append the 'Asset' directory to it

	// Developer version
	applicationdir.Append( "Assets/" );

	AssetDir = applicationdir;
	
	return true;
}



Bool TLFileSys::Platform::GetUserDirectory(TString& UserDir)
{
	return GetAssetSubDirectory(UserDir, "User");
}		

Bool TLFileSys::Platform::GetApplicationURL(TString& url)
{
	url = TLGui::GetAppPath();
	return true;
}