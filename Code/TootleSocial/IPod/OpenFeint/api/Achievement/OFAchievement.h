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

@interface OFAchievement : OFResource
{
@package
	NSString* title;
	NSString* description;
	NSUInteger gamerscore;
	NSString* iconUrl;
	BOOL isSecret;
	BOOL isUnlocked;
	BOOL isUnlockedByComparedToUser;
	NSString* comparedToUserId;
	NSDate* unlockDate;
}

+ (OFResourceDataMap*)getDataMap;
+ (OFService*)getService;
+ (NSString*)getResourceName;
+ (NSString*)getResourceDiscoveredNotification;

@property (nonatomic, readonly, retain) NSString* title;
@property (nonatomic, readonly, retain) NSString* description;
@property (nonatomic, readonly) NSUInteger gamerscore;
@property (nonatomic, readonly, retain) NSString* iconUrl;
@property (nonatomic, readonly) BOOL isSecret;
@property (nonatomic, readonly) BOOL isUnlocked;
@property (nonatomic, readonly) BOOL isUnlockedByComparedToUser;
@property (nonatomic, readonly, retain) NSString* comparedToUserId;
@property (nonatomic, readonly, retain) NSDate* unlockDate;

@end