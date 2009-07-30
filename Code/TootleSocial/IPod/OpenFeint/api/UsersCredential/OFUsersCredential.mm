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

#import "OFDependencies.h"
#import "OFUsersCredential.h"
#import "OFUsersCredentialService.h"
#import "OFResourceDataMap.h"



struct OFKnownCredentialDisplayName
{
	OFKnownCredentialDisplayName(NSString* _credentialType, NSString* _displayName)
	: credentialType(_credentialType)
	, displayName(_displayName)
	{
	}
	
	OFRetainedPtr<NSString> credentialType;
	OFRetainedPtr<NSString> displayName;
};

namespace  
{
	static OFKnownCredentialDisplayName sKnownCredentials[] = 
	{
	OFKnownCredentialDisplayName(@"twitter", @"Twitter"),
	OFKnownCredentialDisplayName(@"fbconnect", @"Facebook"),
	OFKnownCredentialDisplayName(@"http_basic", @"Email & Password"),
	};
}

@implementation OFUsersCredential

@synthesize credentialType;
@synthesize hasGlobalPermissions;
@synthesize isLinked;

- (void)setCredentialType:(NSString*)_value
{
	credentialType = [_value retain];
}

- (void)setCredentialProfilePictureUrl:(NSString*)_value
{
	credentialProfilePictureUrl = [_value retain];
}

- (void)setHasGlobalPermissions:(NSString*)_value
{
	if([_value isEqualToString:@"true"])
	{
		hasGlobalPermissions = YES;
	}
	else
	{
		hasGlobalPermissions = NO;
	}
}

- (void)setIsLinked:(NSString*)_value
{
	isLinked = [_value boolValue];
}

+ (OFService*)getService;
{
	return [OFUsersCredentialService sharedInstance];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"credential_type",	@selector(setCredentialType:));		
		dataMap->addField(@"credential_profile_picture_url", @selector(setCredentialProfilePictureUrl:));
		dataMap->addField(@"has_global_permissions", @selector(setHasGlobalPermissions:));
		dataMap->addField(@"is_linked", @selector(setIsLinked:));
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"users_credential";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return @"openfeint_users_credential_discovered";
}

+ (NSString*)getDisplayNameForCredentialType:(NSString*)credentialType
{
	for (int i = 0; i < sizeof(sKnownCredentials) / sizeof(OFKnownCredentialDisplayName); i++)
	{
		if ([sKnownCredentials[i].credentialType.get() isEqualToString:credentialType])
		{
			return sKnownCredentials[i].displayName;
		}
	}
	return nil;
}

- (BOOL)isFacebook
{
	return [credentialType isEqualToString:@"fbconnect"];
}

- (BOOL)isTwitter
{
	return [credentialType isEqualToString:@"twitter"];
}

- (void) dealloc
{
	[credentialType release];
	[credentialProfilePictureUrl release];
	[super dealloc];
}

@end
