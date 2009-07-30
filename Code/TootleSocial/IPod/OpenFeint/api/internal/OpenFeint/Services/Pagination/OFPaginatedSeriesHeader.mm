//
//  OFPaginatedSeriesHeader.mm
//  OpenFeint
//
//  Created by Jason Citron on 5/30/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFDependencies.h"
#import "OFPaginatedSeriesHeader.h"
#import "OFXmlElement.h"

namespace 
{
	NSUInteger readValueAsInteger(const char* name, OFXmlElement* element)
	{
		OFPointer<OFXmlElement> elementToRead = element->dequeueNextUnreadChild(name);
		if (elementToRead)
		{
			return [elementToRead->getValue() integerValue];
		}
		return 0;
	}
}

@implementation OFPaginatedSeriesHeader

@synthesize currentPage;
@synthesize totalPages;
@synthesize perPage;
@synthesize totalObjects;

+ (NSString*)getElementName
{
	return @"pagination_header";
}

+ (OFPaginatedSeriesHeader*)paginationHeaderWithXmlElement:(OFXmlElement*)element
{
	return [[[OFPaginatedSeriesHeader alloc] initWithXmlElement:element] autorelease];
}

- (OFPaginatedSeriesHeader*)initWithPaginationSeriesHeader:(OFPaginatedSeriesHeader*)otherHeader
{
	if(self = [super init])
	{
		currentPage = otherHeader.currentPage;
		totalPages = otherHeader.totalPages;
		perPage = otherHeader.perPage;
		totalObjects = otherHeader.totalObjects;		
	}
	return self;
	
}

- (OFPaginatedSeriesHeader*)initWithXmlElement:(OFXmlElement*)element
{
	if(self = [super init])
	{
		currentPage		= readValueAsInteger("current_page", element);
		totalPages		= readValueAsInteger("total_pages", element);
		perPage			= readValueAsInteger("per_page", element);
		totalObjects	= readValueAsInteger("total_entries", element);					
	}
	return self;
}


+ (OFPaginatedSeriesHeader*)paginationHeaderClonedFrom:(OFPaginatedSeriesHeader*)otherHeader
{
	if(otherHeader == nil)
	{
		return nil;
	}
	
	return [[[OFPaginatedSeriesHeader alloc] initWithPaginationSeriesHeader:otherHeader] autorelease];
}

- (bool)isLastPageLoaded
{
	return totalPages == currentPage;
}

@end
