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
#import "OFUserSettingPushController.h"
#import "OFResourceDataMap.h"
#import "OFControllerLoader.h"

@implementation OFUserSettingPushController

@synthesize controllerName;
@synthesize settingTitle;

- (void)setControllerName:(NSString*)_value
{
	controllerName = [_value retain];
}

- (void)setSettingTitle:(NSString*)_value
{
	settingTitle = [_value retain];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"controller_name",					@selector(setControllerName:));
		dataMap->addField(@"setting_title",						@selector(setSettingTitle:));
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"user_setting_push_controller";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return nil;
}

- (UIViewController*)getController
{
	return OFControllerLoader::load(controllerName);
}

- (BOOL)verifyControllerExists
{
	return OFControllerLoader::doesControllerExist(controllerName);
}

- (void) dealloc
{
	[controllerName release];
	[settingTitle release];
	[super dealloc];
}

@end
