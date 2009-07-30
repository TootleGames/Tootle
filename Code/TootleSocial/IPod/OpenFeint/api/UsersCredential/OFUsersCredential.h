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

#import "OFResource.h"

@class OFService;

@interface OFUsersCredential : OFResource
{
@private
	NSString* credentialType;
	NSString* credentialProfilePictureUrl;
	BOOL hasGlobalPermissions;
	BOOL isLinked;
}

+ (OFResourceDataMap*)getDataMap;
+ (OFService*)getService;
+ (NSString*)getResourceName;
+ (NSString*)getResourceDiscoveredNotification;
+ (NSString*)getDisplayNameForCredentialType:(NSString*)credentialType;

- (BOOL)isFacebook;
- (BOOL)isTwitter;

@property (nonatomic, readonly,  retain)	NSString* credentialType;
@property(assign, readonly) BOOL hasGlobalPermissions;
@property(assign, readonly) BOOL isLinked;

@end

