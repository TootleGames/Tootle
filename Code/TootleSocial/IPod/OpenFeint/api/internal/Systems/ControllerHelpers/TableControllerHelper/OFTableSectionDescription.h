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

#pragma once
#import "OFPaginatedSeries.h"

@class OFTableSectionCellDescription;
@class OFResource;
@class OFResourceViewHelper;

@interface OFTableSectionDescription : NSObject
{
	NSString* title;
	NSString* identifier;
	OFPaginatedSeries* page;
	NSString* leadingCellName;
	NSString* trailingCellName;
	UIView* headerView;
	UIView* footerView;
}

@property (nonatomic, retain) NSString* title;
@property (nonatomic, retain) NSString* identifier;
@property (nonatomic, retain) OFPaginatedSeries* page;
@property (nonatomic, retain) NSString* leadingCellName;
@property (nonatomic, retain) NSString* trailingCellName;
@property (nonatomic, retain) UIView* headerView;
@property (nonatomic, retain) UIView* footerView;

+ (id)sectionWithTitle:(NSString*)title andPage:(OFPaginatedSeries*)page;
+ (id)sectionWithTitle:(NSString*)title andCell:(OFTableSectionCellDescription*)cellDescription;

- (id)initWithTitle:(NSString*)title andPage:(OFPaginatedSeries*)page;
- (unsigned int)countPageItems;

@end
