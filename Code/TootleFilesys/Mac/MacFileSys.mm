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



Bool TLFileSys::Platform::GetAssetDirectory(TTempString& AssetDir)
{
	
	TTempString applicationdir;
	
	if(GetApplicationURL(applicationdir))
	{
		//	For a developer build we need to remove components until we find the project 
		// directory and then append 'Asset'
		// For a release build we will need to use the bundle path as-is and append the 'Asset' directory to it

		///////////////////////////////////////////////////////////////////////////
		// Developer version
		///////////////////////////////////////////////////////////////////////////
		
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

		
		return TRUE;
	}	
	
	return FALSE;
	
}

Bool TLFileSys::Platform::GetAssetSubDirectory(TTempString& UserDir, const TTempString& Subdirectory)
{
	
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
}		


Bool TLFileSys::Platform::GetUserDirectory(TTempString& UserDir)
{
	return GetAssetSubDirectory(UserDir, "User");
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