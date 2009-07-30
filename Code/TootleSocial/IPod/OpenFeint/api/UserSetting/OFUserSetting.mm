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
#import "OFUserSetting.h"
#import "OFUserSettingService.h"
#import "OFResourceDataMap.h"

@implementation OFUserSetting

@synthesize name;
@synthesize value;
@synthesize valueType;

- (void)setName:(NSString*)_value
{
	name = [_value retain];
}

- (void)setValueType:(NSString*)_value
{
	valueType = [_value retain];
}

+ (OFService*)getService;
{
	return [OFUserSettingService sharedInstance];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"name",			@selector(setName:));
		dataMap->addField(@"value",			@selector(setValue:));
		dataMap->addField(@"value_type",	@selector(setValueType:));		
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"user_setting";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return @"openfeint_user_setting_discovered";
}

- (void) dealloc
{
	[name release];
	[value release];
	[valueType release];
	[super dealloc];
}

@end
