//
//  OFApplicationDescription.h
//  OpenFeint
//
//  Created by Jason Citron on 4/20/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFResource.h"

@class OFService;

@interface OFApplicationDescription : OFResource
{
@package
	NSString* name;
	NSString* iconUrl;
	NSString* itunesId;
	NSString* price;
	NSString* currentVersion;
	NSString* briefDescription;
	NSString* extendedDescription;
	NSString* applicationId;
}

@property (nonatomic, readonly, retain) NSString* name;
@property (nonatomic, readonly, retain) NSString* iconUrl;
@property (nonatomic, readonly, retain) NSString* itunesId;
@property (nonatomic, readonly, retain) NSString* price;
@property (nonatomic, readonly, retain) NSString* currentVersion;
@property (nonatomic, readonly, retain) NSString* briefDescription;
@property (nonatomic, readonly, retain) NSString* extendedDescription;
@property (nonatomic, readonly, retain)	NSString* applicationId;

+ (OFResourceDataMap*)getDataMap;
+ (OFService*)getService;
+ (NSString*)getResourceName;

@end