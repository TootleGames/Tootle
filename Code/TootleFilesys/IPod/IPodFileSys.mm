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




Bool TLFileSys::Platform::GetAssetDirectory(TString& AssetDir)
{
	if ( !GetApplicationURL(AssetDir) )
		return false;
	
	AssetDir << "/Assets/";
	return true;
}



Bool TLFileSys::Platform::GetUserDirectory(TString& UserDir)
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

Bool TLFileSys::Platform::GetApplicationURL(TString& url)
{
	NSBundle* bundle = [NSBundle mainBundle];
	if ( !bundle )
		return false;

	NSString* bundlePath = [bundle bundlePath];
	url << bundlePath;
		
//		const char* pApplicationDir = (const char*)[bundlepath UTF8String];
//		url = pApplicationDir;
	return true;
}