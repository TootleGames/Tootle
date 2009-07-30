//
//  OFPaginatedSeries.h
//  OpenFeint
//
//  Created by Jason Citron on 5/30/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

@class OFPaginatedSeriesHeader;

@interface OFPaginatedSeries : NSObject
{
	NSMutableArray* tableMetaDataObjects;
	NSMutableArray* objects;
	OFPaginatedSeriesHeader* header;
}

@property (retain, nonatomic) NSMutableArray* tableMetaDataObjects;
@property (retain, nonatomic) NSMutableArray* objects;
@property (retain, nonatomic) OFPaginatedSeriesHeader* header;

+ (OFPaginatedSeries*)paginatedSeries;
+ (OFPaginatedSeries*)paginatedSeriesWithObject:(id)objectToAdd;
+ (OFPaginatedSeries*)paginatedSeriesFromArray:(NSArray*)array;
+ (OFPaginatedSeries*)paginatedSeriesFromSeries:(OFPaginatedSeries*)seriesToCopy;

- (void)addObject:(id)objectToAdd;
- (unsigned int)count;
- (id)objectAtIndex:(unsigned int)index;

@end
