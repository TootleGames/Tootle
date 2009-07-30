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
#import "OFTableSectionDescription.h"
#import "OFTableSectionCellDescription.h"
#import "OFPaginatedSeries.h"
#import "OFResourceViewHelper.h"

@implementation OFTableSectionDescription

@synthesize title;
@synthesize identifier;
@synthesize page;
@synthesize leadingCellName;
@synthesize trailingCellName;
@synthesize headerView;
@synthesize footerView;

- (void)setHeaderView:(UIView*)newHeaderView
{
	OFSafeRelease(headerView);
	headerView = [newHeaderView retain];
}

- (void)setFooterView:(UIView*)newFooterView
{
	OFSafeRelease(footerView);
	footerView = [newFooterView retain];
}

+ (id)sectionWithTitle:(NSString*)title andPage:(OFPaginatedSeries*)page
{
	return [[[OFTableSectionDescription alloc] initWithTitle:title andPage:page] autorelease];
}

+ (id)sectionWithTitle:(NSString*)title andCell:(OFTableSectionCellDescription*)cellDescription
{
	OFPaginatedSeries* page = [OFPaginatedSeries paginatedSeriesWithObject:cellDescription];
	return [[[OFTableSectionDescription alloc] initWithTitle:title andPage:page] autorelease];
}

- (id)initWithTitle:(NSString*)_title andPage:(OFPaginatedSeries*)_page
{
	if(self = [super init])
	{
		self.title = _title;
		self.page = _page;
	}
	return self;
	
}

- (void)dealloc
{
	self.title = nil;
	self.identifier = nil;
	self.page = nil;
	self.leadingCellName = nil;
	self.trailingCellName = nil;
	self.headerView = nil;
	self.footerView = nil;
	[super dealloc];
}

- (unsigned int)countPageItems
{
	unsigned int count = [self.page count];
	
	if(self.leadingCellName)
	{
		++count;
	}
	
	if(self.trailingCellName)
	{
		++count;
	}
	
	return count;
}

@end