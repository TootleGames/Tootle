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

		
		NSString* appdir = [NSString stringWithUTF8String:applicationdir.GetData()];

		NSArray* array = [appdir componentsSeparatedByString:@"Code"];

		// Failed to split string into two parts?  If more than two then 'code' appears mutliple times in string
		// and may need concatenating.  If less than then this may be a deployment build?
		if([array count] != 2)
			return FALSE;
		
		// now remove the 'Code' directory and append 'Asset'
		NSString* rawpath = [array objectAtIndex:0];
		NSString* path = [rawpath stringByAppendingString:@"Assets/"];
		
		// Copy path string
		const char* pAssetDir = (const char*)[path UTF8String];
		AssetDir =  pAssetDir;
				
		///////////////////////////////////////////////////////////////////////////

#else
		
		///////////////////////////////////////////////////////////////////////////
		// Release version
		///////////////////////////////////////////////////////////////////////////

		// For a release build we will need to use the bundle path as-is and append the 'Asset' directory to it

		
		NSString* appdir = [NSString stringWithUTF8String:applicationdir.GetData()];
		
		// On the ipod/iphone the assets are stored in the bundle root.
		//NSString* path = [appdir stringByAppendingString:@"/Assets/"];
		NSString* path = [appdir stringByAppendingString:@"/Contents/Resources/"];
		
		// Copy path string
		const char* pAssetDir = (const char*)[path UTF8String];
		AssetDir =  pAssetDir;
		
		///////////////////////////////////////////////////////////////////////////
#endif
		
		return TRUE;
	}	
	
	return FALSE;
	
}

Bool TLFileSys::Platform::GetAssetSubDirectory(TTempString& UserDir, const TTempString& Subdirectory)
{
#if(FILESYS_VERSION == FS_DEVELOPMENT)	
	///////////////////////////////////////////////////////////////////////////
	// Developer verison
	///////////////////////////////////////////////////////////////////////////

	// For a developer version we append the subdirectory onto the asset directory
	
	TTempString tmpassetdir;
	
	if(GetAssetDirectory(tmpassetdir))
	{
		NSString* tmpdir = [NSString stringWithUTF8String:tmpassetdir.GetData()];		
		NSString* subdir = [NSString stringWithUTF8String:Subdirectory.GetData()];
		
		// Append the subdirectory to the asset directory
		NSString* tmppath = [tmpdir stringByAppendingString:subdir];
		
		// Add seperator
		NSString* path = [tmppath stringByAppendingString:@"/"];
		
		// Copy path string
		const char* pUserDir = (const char*)[path UTF8String];
		UserDir =  pUserDir;
		
		return TRUE;
		
	}
	
	return FALSE;
	
	///////////////////////////////////////////////////////////////////////////
	
#else

	///////////////////////////////////////////////////////////////////////////
	// Release version
	///////////////////////////////////////////////////////////////////////////

	// For a release version we do not use subdirectories per se.  All assets are stored in the 
	// root of the package so we simply access the root asset directory instead.
	
	// Print a warnign to say that we are looking for a subdirectory that may not be found
	TLDebug_Print("WARNING: GetAssetSubDirectory called which may not find the resource you are looking for");
	return GetAssetDirectory(UserDir);
	
	///////////////////////////////////////////////////////////////////////////

#endif	
	
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