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
#import "OFGamePlayer.h"
#import "OFProfileService.h"
#import "OFResourceDataMap.h"
#import "OFUser.h"

@implementation OFGamePlayer

@synthesize user, applicationId, applicationGamerscore;

- (void)setApplicationId:(NSString*)value
{
	applicationId = [value retain];
}

- (void)setApplicationGamerscore:(NSString*)value
{
	applicationGamerscore = [value intValue];
}

- (void)setUser:(OFResource*)value
{
	user = (OFUser*)[value retain];
}

+ (OFService*)getService;
{
	return [OFProfileService sharedInstance];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"app_id", @selector(setApplicationId:));
		dataMap->addField(@"app_gamerscore", @selector(setApplicationGamerscore:));
		dataMap->addNestedResourceField(@"user", @selector(setUser:), [OFUser class]);
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"game_player";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return nil;
}

- (void) dealloc
{
	OFSafeRelease(user);
	OFSafeRelease(applicationId);
	[super dealloc];
}

@end