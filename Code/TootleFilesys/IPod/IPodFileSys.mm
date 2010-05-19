/*
 *  IPodFileSys.mm
 *  TootleFileSys
 *
 *  Created by Duane Bradbury on 23/09/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "IPodFileSys.h"

#import <Foundation/Foundation.h>
#import <TootleCore/IPod/IPodString.h>




Bool TLFileSys::Platform::GetAssetDirectory(TTempString& AssetDir)
{
	
	TTempString applicationdir;
	
	if(GetApplicationURL(applicationdir))
	{
		applicationdir.Append("/Assets/");
		AssetDir =  applicationdir;
		
		return TRUE;
	}	
	
	return FALSE;
	
}


Bool TLFileSys::Platform::GetAssetSubDirectory(TTempString& AssetDir, const TTempString& Subdirectory)
{
	// Get the subdirectory requested.
	TTempString tmpassetdir;
	
	if(GetAssetDirectory(tmpassetdir))
	{
		tmpassetdir.Append(Subdirectory);
		tmpassetdir.Append("/");
		
		AssetDir = tmpassetdir;
		
		return TRUE;
		
	}
	
	return FALSE;
}


Bool TLFileSys::Platform::GetUserDirectory(TTempString& UserDir)
{
	// Get the 'Documents' path for the user directory.
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES); 
	
	NSString *tmppath = [paths objectAtIndex:0];

	// Append the seperator (just in case it causes issues)
	NSString *path = [tmppath stringByAppendingString:@"/"];

	// Copy path string
	const char* pUserDir = (const char*)[path UTF8String];
	UserDir =  pUserDir;
		
	return TRUE;
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