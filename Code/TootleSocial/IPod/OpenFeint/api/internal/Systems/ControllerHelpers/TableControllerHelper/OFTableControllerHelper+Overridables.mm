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
#import "OFTableControllerHelper+Overridables.h"
#import "OFService.h"
#import "OFControllerHelpersCommon.h"

@implementation OFTableControllerHelper (Overridables)

- (NSString*)getTextToShowWhileLoading
{
	return @"Downloading";
}

- (UIViewController*)getNoDataFoundViewController
{
	// Optional
	return nil;
}

- (NSString*)getNoDataFoundMessage
{
	ASSERT_OVERRIDE_MISSING;
	return @"No data results were found";
}

- (NSString*)getTableHeaderControllerName
{
	return nil;
}

- (void)onTableHeaderCreated:(UIViewController*)tableHeader
{
	// Do Nothing
}

- (OFService*) getService
{
	ASSERT_OVERRIDE_MISSING;
	return nil;
}

- (void)onLeadingCellWasClickedForSection:(OFTableSectionDescription*)section
{
	// Do Nothing
}

- (void)onTrailingCellWasClickedForSection:(OFTableSectionDescription*)section
{
	// Do Nothing
}

- (void)onCellWasClicked:(OFTableSectionCellDescription*)cellResource indexPathInTable:(NSIndexPath*)indexPath
{
	// Do Nothing
}

- (bool)shouldAlwaysRefreshWhenShown
{
	return NO;
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	ASSERT_OVERRIDE_MISSING;
}

- (void)_onDataLoaded:(OFPaginatedSeries*)resources isIncremental:(BOOL)isIncremental
{
	ASSERT_OVERRIDE_MISSING;
}

- (bool)isNewContentShownAtBottom
{
	return false;
}

- (OFResource*) getResourceFromSection:(NSArray*)sectionCells atRow:(NSUInteger)row
{
	ASSERT_OVERRIDE_MISSING;
	return nil;
}

- (NSString*) getCellControllerNameFromSection:(NSArray*)sectionCells atRow:(NSUInteger)row
{
	ASSERT_OVERRIDE_MISSING;
	return @"";
}

- (bool)shouldRefreshAfterNotification
{
	return false;
}

- (NSString*)getNotificationToRefreshAfter
{
	return @"";
}

- (NSString*)getLeadingCellControllerNameForSection:(OFTableSectionDescription*)section
{
	return nil;
}

- (NSString*)getTrailingCellControllerNameForSection:(OFTableSectionDescription*)section
{
	return nil;
}

- (void)onLeadingCellWasLoaded:(OFTableCellHelper*)leadingCell forSection:(OFTableSectionDescription*)section
{
	
}

- (void)onTrailingCellWasLoaded:(OFTableCellHelper*)trailingCell forSection:(OFTableSectionDescription*)section
{
	
}

@end