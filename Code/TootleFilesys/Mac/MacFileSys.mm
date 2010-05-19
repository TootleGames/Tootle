/*
 *  MacFileSys.mm
 *  TootleFilesys
 *
 *  Created by Duane Bradbury on 22/09/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "MacFileSys.h"

#import <Foundation/Foundation.h>

// Filesystem versions
#define FS_DEVELOPMENT 0
#define FS_RELEASE 1

#define FILESYS_VERSION FS_DEVELOPMENT	

#include <TootleCore/TLTypes.h>
#include <TootleCore/Mac/MacString.h>

Bool TLFileSys::Platform::GetAssetDirectory(TTempString& AssetDir)
{
	
	TTempString applicationdir;
	
	if(GetApplicationURL(applicationdir))
	{

#if(FILESYS_VERSION == FS_DEVELOPMENT)	
		///////////////////////////////////////////////////////////////////////////
		// Developer version
		///////////////////////////////////////////////////////////////////////////
		
		//	For a developer build we need to remove components until we find the project 
		// directory and then append 'Asset'
		
		NSString* appdir = TLString::ConvertToUnicharString(applicationdir);

		NSArray* array = [appdir componentsSeparatedByString:@"Code"];

		// Failed to split string into two parts?  If more than two then 'code' appears mutliple times in string
		// and may need concatenating.  If less than then this may be a deployment build?
		if([array count] != 2)
		{
			[appdir release];			
			
			TLDebug_Break("Failed to find 'Code' directory in developer build");
			return FALSE;
		}
		
		// now remove the 'Code' directory and append 'Asset'
		NSString* rawpath = [array objectAtIndex:0];
		NSString* path = [rawpath stringByAppendingString:@"Assets/"];
		
		// Copy path string
		const char* pAssetDir = (const char*)[path UTF8String];
		AssetDir =  pAssetDir;
		
		[appdir release];
				
		///////////////////////////////////////////////////////////////////////////

#else
		
		///////////////////////////////////////////////////////////////////////////
		// Release version
		///////////////////////////////////////////////////////////////////////////

		// For a release build we will need to use the bundle path as-is and append the 'Asset' directory to it

		// On the Mac the assets are stored in the bundle under 'contents/resources/assets'
		applicationdir.Append("/Contents/Resources/Assets");
		AssetDir = applicationdir;
		
		///////////////////////////////////////////////////////////////////////////
#endif
		
		return TRUE;
	}	
	
	return FALSE;
	
}

Bool TLFileSys::Platform::GetAssetSubDirectory(TTempString& UserDir, const TTempString& Subdirectory)
{
	// Get the subdirectory requested
	TTempString tmpassetdir;
	
	if(GetAssetDirectory(tmpassetdir))
	{
		tmpassetdir.Append(Subdirectory);
		tmpassetdir.Append("/");
		
		UserDir = tmpassetdir;
				
		return TRUE;
		
	}
	
	return FALSE;		
}		


Bool TLFileSys::Platform::GetUserDirectory(TTempString& UserDir)
{
#if(FILESYS_VERSION == FS_DEVELOPMENT)	
	///////////////////////////////////////////////////////////////////////////
	// Developer version
	///////////////////////////////////////////////////////////////////////////

	// The developer version uses a 'User' sub directory of the 'Asset' directory
	
	return GetAssetSubDirectory(UserDir, "User");
	///////////////////////////////////////////////////////////////////////////

#else
	
	///////////////////////////////////////////////////////////////////////////
	// Release version
	///////////////////////////////////////////////////////////////////////////
	
	// In release we want to use the users directory on the system rather than our developer user sub directory

	// Get the 'Documents' path for the user directory.
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES); 
	
	NSString *tmppath = [paths objectAtIndex:0];
	
	// Append the seperator (just in case it causes issues)
	NSString *path = [tmppath stringByAppendingString:@"/"];
	
	// Copy path string
	const char* pUserDir = (const char*)[path UTF8String];
	UserDir =  pUserDir;
	
	return TRUE;
	
	///////////////////////////////////////////////////////////////////////////

#endif
}		

Bool TLFileSys::Platform::GetApplicationURL(TTempString& url)
{
	NSBundle* bundle = [NSBundle mainBundle];
	
	if(bundle)
	{
		NSString* bundlepath = [bundle bundlePath];
		
		const char* pApplicationDir = (const char*)[bundlepath UTF8String];
		
		url = pApplicationDir;
		return TRUE;
		
	}
	
	return FALSE;
}