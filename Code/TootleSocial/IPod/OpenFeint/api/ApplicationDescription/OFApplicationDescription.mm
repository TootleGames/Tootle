//
//  OFApplicationDescription.mm
//  OpenFeint
//
//  Created by Jason Citron on 4/20/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFDependencies.h"
#import "OFApplicationDescription.h"
#import "OFApplicationDescriptionService.h"
#import "OFResourceDataMap.h"

@implementation OFApplicationDescription

@synthesize name, iconUrl, itunesId, price, currentVersion, briefDescription, extendedDescription, applicationId;

- (void)setName:(NSString*)value
{
	OFSafeRelease(name);
	name = [value retain];
}

- (void)setIconUrl:(NSString*)value
{
	OFSafeRelease(iconUrl);
	if (![value isEqualToString:@""])
	{
		iconUrl = [value retain];
	}
}

- (void)setItunesId:(NSString*)value
{
	OFSafeRelease(itunesId);
	itunesId = [value retain];
}

- (void)setPrice:(NSString*)value
{
	OFSafeRelease(price);
	price = [value retain];
}

- (void)setCurrentVersion:(NSString*)value
{
	OFSafeRelease(currentVersion);
	currentVersion = [value retain];
}

- (void)setBriefDescription:(NSString*)value
{
	OFSafeRelease(briefDescription);
	briefDescription = [value retain];
}

- (void)setExtendedDescription:(NSString*)value
{
	OFSafeRelease(extendedDescription);
	extendedDescription = [value retain];
}

- (void)setApplicationId:(NSString*)value
{
	OFSafeRelease(applicationId);
	applicationId = [value retain];
}

+ (OFService*)getService;
{
	return [OFApplicationDescriptionService sharedInstance];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"name",					@selector(setName:));
		dataMap->addField(@"icon_url",				@selector(setIconUrl:));
		dataMap->addField(@"price",					@selector(setPrice:));
		dataMap->addField(@"itunes_id",				@selector(setItunesId:));
		dataMap->addField(@"current_version",		@selector(setCurrentVersion:));
		dataMap->addField(@"description_brief",		@selector(setBriefDescription:));
		dataMap->addField(@"description_extended",	@selector(setExtendedDescription:));
		dataMap->addField(@"client_application_id",	@selector(setApplicationId:));
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"application_description";
}

+ (NSString*)getResourceDiscoveredNotification
{
	return nil;
}

- (void) dealloc
{
	OFSafeRelease(name);
	OFSafeRelease(iconUrl);
	OFSafeRelease(itunesId);
	OFSafeRelease(price);
	OFSafeRelease(currentVersion);
	OFSafeRelease(briefDescription);
	OFSafeRelease(extendedDescription);
	[super dealloc];
}

@end