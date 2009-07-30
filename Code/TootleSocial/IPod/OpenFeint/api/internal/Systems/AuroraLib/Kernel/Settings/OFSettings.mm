////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Copyright (c) 2009 Aurora Feint Inc.
///
///  This library is free software; you can redistribute it and/or
///  modify it under the terms of the GNU Lesser General Public
///  License as published by the Free Software Foundation; either
///  
///  version 3 of the License, or (at your option) any later version.
///  
///  This library is distributed in the hope that it will be useful,
///  
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
///  Lesser General Public License for more details.
///  
///  
///  You should have received a copy of the GNU Lesser General Public
///  License along with this library; if not, write to the Free Software
///  
///  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "OFSettings.h"
#include "OFXmlReader.h"

OFDefineSingleton(OFSettings);

OFSettings::OFSettings()
{
	// These are default settings intended for distribution
	mServerUrl = @"https://api.openfeint.com/";
	mFacebookApplicationKey = @"1a2dcd0bdc7ce8056aeb1dac00c2a886"; //Feint
	mFacebookCallbackServerUrl = mServerUrl;
	
	discoverLocalConfiguration();
	
#if !defined(_DISTRIBUTION)
	loadSettingsFile();
#endif
}

void OFSettings::loadSettingsFile()
{
#if !defined(_DISTRIBUTION)
	OFXmlReader OFSettingsReader("openfeint_internal_settings");
	NSMutableString* environmentScope = [NSMutableString stringWithString:@"environment-"];
	
#ifdef _DEBUG
	[environmentScope appendString:@"debug"];
#else
	[environmentScope appendString:@"release"];
#endif

	OFISerializer::Scope sRoot(&OFSettingsReader, "config");	
	OFISerializer::Scope sEnvironment(&OFSettingsReader, [environmentScope cStringUsingEncoding:NSUTF8StringEncoding]);
	OFSettingsReader.io("server-url", mServerUrl);
	OFSettingsReader.io("facebook-callback-url", mFacebookCallbackServerUrl);
	OFSettingsReader.io("facebook-application-key", mFacebookApplicationKey);		
	
#endif
}

void OFSettings::discoverLocalConfiguration()
{
	NSDictionary* infoDict = [[NSBundle mainBundle] infoDictionary];
	NSString* bundleIdentifier = [infoDict valueForKey:@"CFBundleIdentifier"];
	NSString* bundleVersion = [infoDict valueForKey:@"CFBundleVersion"];
	
	mClientBundleIdentifier		= bundleIdentifier;
	mClientBundleVersion		= bundleVersion;
	mClientLocale				= [[NSLocale currentLocale] localeIdentifier];
	mClientDeviceType			= [UIDevice currentDevice].model;
	mClientDeviceSystemName		= [UIDevice currentDevice].systemName;
	mClientDeviceSystemVersion	= [UIDevice currentDevice].systemVersion;	
}