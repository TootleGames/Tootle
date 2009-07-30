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

#import "OFTableControllerHelper.h"

@class OFService;
@class OFTableSectionDescription;
@class OFTableCellHelper;

@interface OFTableControllerHelper (Overridables)

- (OFResource*) getResourceFromSection:(NSArray*)sectionCells atRow:(NSUInteger)row;
- (NSString*) getCellControllerNameFromSection:(NSArray*)sectionCells atRow:(NSUInteger)row;
- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure;
- (void)_onDataLoaded:(OFPaginatedSeries*)resources isIncremental:(BOOL)isIncremental;
- (OFService*) getService;
- (UIViewController*)getNoDataFoundViewController;
- (NSString*)getNoDataFoundMessage;

// These are optional.
- (NSString*)getLeadingCellControllerNameForSection:(OFTableSectionDescription*)section;
- (NSString*)getTrailingCellControllerNameForSection:(OFTableSectionDescription*)section;
- (void)onLeadingCellWasLoaded:(OFTableCellHelper*)leadingCell forSection:(OFTableSectionDescription*)section;
- (void)onTrailingCellWasLoaded:(OFTableCellHelper*)trailingCell forSection:(OFTableSectionDescription*)section;
- (bool)shouldRefreshAfterNotification;
- (NSString*)getNotificationToRefreshAfter;
- (bool)isNewContentShownAtBottom;
- (void)onCellWasClicked:(OFTableSectionCellDescription*)cellResource indexPathInTable:(NSIndexPath*)indexPath;
- (void)onLeadingCellWasClickedForSection:(OFTableSectionDescription*)section;
- (void)onTrailingCellWasClickedForSection:(OFTableSectionDescription*)section;
- (NSString*)getTextToShowWhileLoading;
- (bool)shouldAlwaysRefreshWhenShown;
- (NSString*)getTableHeaderControllerName;
- (void)onTableHeaderCreated:(UIViewController*)tableHeader;

@end
