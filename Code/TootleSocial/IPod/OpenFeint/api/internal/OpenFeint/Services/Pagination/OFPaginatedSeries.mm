//
//  OFPaginatedSeries.mm
//  OpenFeint
//
//  Created by Jason Citron on 5/30/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFDependencies.h"
#import "OFPaginatedSeries.h"
#import "OFPaginatedSeriesHeader.h"

@implementation OFPaginatedSeries

@dynamic objects;
@synthesize tableMetaDataObjects;
@synthesize header;

- (NSMutableArray*)objects
{
	return objects;
}

- (void)setObjects:(NSMutableArray*)value
{
	self.header = nil;
	[objects release];
	objects = [value retain];
}

+ (OFPaginatedSeries*)paginatedSeries
{
	return [[[OFPaginatedSeries alloc] init] autorelease];
}

+ (OFPaginatedSeries*)paginatedSeriesWithObject:(id)object
{
	OFPaginatedSeries* page = [OFPaginatedSeries paginatedSeries];
	[page addObject:object];
	return page;
}

- (OFPaginatedSeries*) init
{
	if(self = [super init])
	{
		self.header = nil;
		self.objects = [NSMutableArray arrayWithCapacity:8];
		self.tableMetaDataObjects = nil;
	}
	return self;	
}

- (void)addObject:(id)object
{
	[self.objects addObject:object];
}

- (void)dealloc
{
	self.header = nil;
	self.objects = nil;
	self.tableMetaDataObjects = nil;
	[super dealloc];
}

- (unsigned int)count
{
	return [self.objects count];
}

- (id)objectAtIndex:(unsigned int)index
{
	return [self.objects objectAtIndex:index];
}

+ (OFPaginatedSeries*)paginatedSeriesFromSeries:(OFPaginatedSeries*)seriesToCopy
{
	OFPaginatedSeries* copiedSeries = [OFPaginatedSeries paginatedSeries];

	copiedSeries.objects = [NSMutableArray arrayWithCapacity:[seriesToCopy.objects count]];
	[copiedSeries.objects addObjectsFromArray:seriesToCopy.objects];

	copiedSeries.header = [OFPaginatedSeriesHeader paginationHeaderClonedFrom:seriesToCopy.header];

	return copiedSeries;
}

+ (OFPaginatedSeries*)paginatedSeriesFromArray:(NSArray*)array
{
	OFPaginatedSeries* series = [OFPaginatedSeries paginatedSeries];
	[series.objects addObjectsFromArray:array];
	return series;
}

@end
