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

#pragma once

class OFSettings 
{
OFDeclareSingleton(OFSettings);
public:
	NSString* getServerUrl() const						{ return mServerUrl; }
	NSString* getFacebookApplicationKey() const			{ return mFacebookApplicationKey; }
	NSString* getFacebookCallbackServerUrl() const		{ return mFacebookCallbackServerUrl; }
	NSString* getClientBundleIdentifier() const			{ return mClientBundleIdentifier; }
	NSString* getClientBundleVersion() const			{ return mClientBundleVersion; }
	NSString* getClientLocale() const					{ return mClientLocale; }
	NSString* getClientDeviceType() const				{ return mClientDeviceType; }
	NSString* getClientDeviceSystemName() const			{ return mClientDeviceSystemName; }
	NSString* getClientDeviceSystemVersion() const		{ return mClientDeviceSystemVersion; }	
	
private:
	void discoverLocalConfiguration();
	void loadSettingsFile();
	
	OFRetainedPtr<NSString> mServerUrl;
	OFRetainedPtr<NSString> mFacebookCallbackServerUrl;
	OFRetainedPtr<NSString> mFacebookApplicationKey;
	OFRetainedPtr<NSString> mClientBundleIdentifier;
	OFRetainedPtr<NSString> mClientBundleVersion;
	OFRetainedPtr<NSString> mClientLocale;
	OFRetainedPtr<NSString> mClientDeviceType;
	OFRetainedPtr<NSString> mClientDeviceSystemName;
	OFRetainedPtr<NSString> mClientDeviceSystemVersion;	
};