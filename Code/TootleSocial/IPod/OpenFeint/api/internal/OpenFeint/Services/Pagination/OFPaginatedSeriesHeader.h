//
//  OFPaginatedSeriesHeader.h
//  OpenFeint
//
//  Created by Jason Citron on 5/30/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

class OFXmlElement;

@interface OFPaginatedSeriesHeader : NSObject 
{
	NSUInteger currentPage;
	NSUInteger totalPages;
	NSUInteger perPage;
	NSUInteger totalObjects;			
}

+ (NSString*)getElementName;
+ (OFPaginatedSeriesHeader*)paginationHeaderWithXmlElement:(OFXmlElement*)element;
+ (OFPaginatedSeriesHeader*)paginationHeaderClonedFrom:(OFPaginatedSeriesHeader*)otherHeader;

- (OFPaginatedSeriesHeader*)initWithXmlElement:(OFXmlElement*)element;
- (OFPaginatedSeriesHeader*)initWithXmlElement:(OFXmlElement*)element;

- (bool)isLastPageLoaded;

@property (readonly, nonatomic) NSUInteger currentPage;
@property (readonly, nonatomic) NSUInteger totalPages;
@property (readonly, nonatomic) NSUInteger perPage;
@property (readonly, nonatomic) NSUInteger totalObjects;

@end
