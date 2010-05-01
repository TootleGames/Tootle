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
/*
		NSString* appdir = TLString::ConvertToUnicharString(applicationdir, FALSE);
		
		// On the ipod/iphone the assets are stored in the bundle root.
		//NSString* path = [appdir stringByAppendingString:@"/Assets/"];
		NSString* path = [appdir stringByAppendingString:@"/"];
		
		// Copy path string
		const char* pAssetDir = (const char*)[path UTF8String];
		AssetDir =  pAssetDir;

		///////////////////////////////////////////////////////////////////////////
		[appdir release];
 */
		
		return TRUE;
	}	
	
	return FALSE;
	
}


Bool TLFileSys::Platform::GetAssetSubDirectory(TTempString& AssetDir, const TTempString& Subdirectory)
{
	// On the iPod we don't have sub directories per se
	// The application bundle/package contains resources in a flat structure so resources
	// should be added specifically for a given project and care must be taken not to add files
	// with the same name and extension.
	// If you *need* a subdirectory you can use folder references within the project that will structure the bundle
	// with a 'directory' but all files within the filesystem folder will be added to the project so may not help
	// Additionally if this route is used then the subdirectory will need to be accessed via another routine other than this one.
	
	// Print a warnign to say that we are looking for a subdirectory that may not be found
	TLDebug_Print("WARNING: GetAssetSubDirectory called which may not find the resource you are looking for");
	return GetAssetDirectory(AssetDir);
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