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

@interface OFChatMessage : OFResource
{
@package
	NSString* playerName;
	NSString* playerCurrentGameId;
	NSString* playerCurrentGame;
	NSString* message;
	NSString* playerId;
	NSString* date;
	NSString* playerProfilePictureUrl;
	NSString* playerCurrentGameIconUrl;
	BOOL doesLocalPlayerOwnGame;
	BOOL playerUsesFacebookProfilePicture;
}

+ (OFResourceDataMap*)getDataMap;
+ (OFService*)getService;
+ (NSString*)getResourceName;
+ (NSString*)getResourceDiscoveredNotification;

@property (nonatomic, readonly) NSString* date;
@property (nonatomic, readonly) NSString* playerName;
@property (nonatomic, readonly) NSString* playerCurrentGameId;
@property (nonatomic, readonly) NSString* playerCurrentGame;
@property (nonatomic, readonly) NSString* message;
@property (nonatomic, readonly)	NSString* playerId;
@property (nonatomic, readonly) NSString* playerProfilePictureUrl;
@property (nonatomic, readonly) NSString* playerCurrentGameIconUrl;
@property (nonatomic, readonly) BOOL playerUsesFacebookProfilePicture;
@property (nonatomic, readonly) BOOL doesLocalPlayerOwnGame;

@end