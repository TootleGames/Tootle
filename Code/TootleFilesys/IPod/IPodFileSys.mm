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



Bool TLFileSys::Platform::GetAssetDirectory(TTempString& AssetDir)
{
	
	TTempString applicationdir;
	
	if(GetApplicationURL(applicationdir))
	{
		NSString* appdir = [NSString stringWithUTF8String:applicationdir.GetData()];
		
		// On the ipod/iphone the assets are stored in the bundle root.
		//NSString* path = [appdir stringByAppendingString:@"/Assets/"];
		NSString* path = [appdir stringByAppendingString:@"/"];
		
		// Copy path string
		const char* pAssetDir = (const char*)[path UTF8String];
		AssetDir =  pAssetDir;
		
		///////////////////////////////////////////////////////////////////////////
		
		
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