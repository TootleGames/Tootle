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
#import "OFBootstrap.h"
#import "OFBootstrapService.h"
#import "OFResourceDataMap.h"
#import "OFBootstrapService.h"

@implementation OFBootstrap

@synthesize minimumOpenFeintVersionSupported;
@synthesize pollingFrequencyInChat;
@synthesize pollingFrequencyDefault;
@synthesize loggedInUserId;
@synthesize loggedInUserName;
@synthesize loggedInUserHasSetName;
@synthesize loggedInUserIsNewUser;
@synthesize loggedInUserProfilePictureUrl;
@synthesize loggedInUserUsesFacebookProfilePicture;
@synthesize accessToken;
@synthesize accessTokenSecret;
@synthesize loggedInUserHasNonDeviceCredential;
@synthesize clientApplicationId;
@synthesize clientApplicationIconUrl;
@synthesize loggedInUserHadFriendsOnBootup;

- (void)setMinimumOpenFeintVersionSupported:(NSString*)value
{
	minimumOpenFeintVersionSupported = [value integerValue];
}

- (void)setPollingFrequencyInChat:(NSString*)value
{
	pollingFrequencyInChat = [value integerValue];
}

- (void)setPollingFrequencyDefault:(NSString*)value
{
	pollingFrequencyDefault = [value integerValue];
}

- (void)setLoggedInUserId:(NSString*)value
{
	OFSafeRelease(loggedInUserId);
	loggedInUserId = [value retain];
}

- (void)setLoggedInUserProfilePictureUrl:(NSString*)value
{
	OFSafeRelease(loggedInUserProfilePictureUrl);
	loggedInUserProfilePictureUrl = [value retain];
}

- (void)setLoggedInUserUsesFacebookProfilePicture:(NSString*)value
{
	loggedInUserUsesFacebookProfilePicture = [value boolValue];
}

- (void)setLoggedInUserHadFriendsOnBootup:(NSString*)value
{
	loggedInUserHadFriendsOnBootup = [value boolValue];
}

- (void)setLoggedInUserName:(NSString*)value
{
	OFSafeRelease(loggedInUserName);
	loggedInUserName = [value retain];
}

- (void)setLoggedInUserHasSetName:(NSString*)value
{
	loggedInUserHasSetName = [value boolValue];
}

- (void)setLoggedInUserHasNonDeviceCredential:(NSString*)value
{
	loggedInUserHasNonDeviceCredential = [value boolValue];
}

- (void)setLoggedInUserIsNewUser:(NSString*)value
{
	loggedInUserIsNewUser = [value boolValue];
}

- (void)setAccessTokenSecret:(NSString*)value
{
	OFSafeRelease(accessTokenSecret);
	accessTokenSecret = [value retain];
}

- (void)setAccessToken:(NSString*)value
{
	OFSafeRelease(accessToken);
	accessToken = [value retain];
}

- (void)setClientApplicationId:(NSString*)value
{
	OFSafeRelease(clientApplicationId);
	clientApplicationId = [value retain];
}

- (void)setClientApplicationIconUrl:(NSString*)value
{
	OFSafeRelease(clientApplicationIconUrl);
	clientApplicationIconUrl = [value retain];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"minimum_openfeint_version_supported",			@selector(setMinimumOpenFeintVersionSupported:));	
		dataMap->addField(@"polling_frequency_in_chat",						@selector(setPollingFrequencyInChat:));
		dataMap->addField(@"polling_frequency_default",						@selector(setPollingFrequencyDefault:));
		dataMap->addField(@"logged_in_user_id",								@selector(setLoggedInUserId:));
		dataMap->addField(@"logged_in_user_profile_picture_url",			@selector(setLoggedInUserProfilePictureUrl:));
		dataMap->addField(@"logged_in_user_uses_facebook_profile_picture",	@selector(setLoggedInUserUsesFacebookProfilePicture:));
		dataMap->addField(@"logged_in_user_name",							@selector(setLoggedInUserName:));
		dataMap->addField(@"logged_in_user_has_set_name",					@selector(setLoggedInUserHasSetName:));
		dataMap->addField(@"logged_in_user_is_new_user",					@selector(setLoggedInUserIsNewUser:));
		dataMap->addField(@"logged_in_user_has_friends",					@selector(setLoggedInUserHadFriendsOnBootup:));
		dataMap->addField(@"logged_in_user_is_new_user",					@selector(setLoggedInUserIsNewUser:));
		dataMap->addField(@"access_token",									@selector(setAccessToken:));
		dataMap->addField(@"access_token_secret",							@selector(setAccessTokenSecret:));		
		dataMap->addField(@"logged_in_user_has_non_device_credential",		@selector(setLoggedInUserHasNonDeviceCredential:));	
		dataMap->addField(@"client_application_id",							@selector(setClientApplicationId:));
		dataMap->addField(@"client_application_icon_url",					@selector(setClientApplicationIconUrl:));
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"bootstrap";
}

- (void)dealloc
{
	OFSafeRelease(loggedInUserId);
	OFSafeRelease(loggedInUserName);
	OFSafeRelease(loggedInUserProfilePictureUrl);
	OFSafeRelease(accessToken);
	OFSafeRelease(accessTokenSecret);
	OFSafeRelease(clientApplicationId);
	OFSafeRelease(clientApplicationIconUrl);
	[super dealloc];
}

@end