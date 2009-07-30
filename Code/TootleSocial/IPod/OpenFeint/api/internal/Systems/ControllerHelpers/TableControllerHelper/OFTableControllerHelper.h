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

#import "OFPointer.h"
#import "OFCallbackable.h"
#import "OFDelegate.h"

@class OFTableSectionCellDescription;
@class OFTableSectionDescription;
@class OFDeadEndErrorController;
@class OFResource;
@class OFPaginatedSeries;

@interface OFTableControllerHelper : UITableViewController<OFCallbackable>
{
@package
	UIViewController* mLoadingScreen;
	UIViewController* mTableHeaderController;
	OFDeadEndErrorController* mEmptyTableController;
	NSMutableDictionary* mHelperCellsForHeight;
	NSArray* mSections;
	UITableView* mMyTableView;
	UIView* mContainerView;
	UIView* mDashboardNotificationView;
}

- (bool)canReceiveCallbacksNow;
- (void)showLoadingScreen;
- (void)hideLoadingScreen;
- (OFDelegate)getOnSuccessDelegate;
- (OFDelegate)getOnFailureDelegate;

- (OFTableSectionCellDescription*)getCellAtIndexPath:(NSIndexPath*)indexPath;
- (OFTableSectionDescription*)getSectionForIndexPath:(NSIndexPath*)indexPath;
- (OFTableSectionDescription*)getSectionWithIdentifier:(NSString*)identifier;
- (NSInteger)getNumCellsInTable;
- (NSInteger)getNumCellsInSection:(NSInteger)sectionIndex;
- (bool)isInHiddenTab;

- (void)_reloadTableData;
- (bool)_isScrolledToHead;
- (void)_scrollToTableHead:(BOOL)animate;
- (UITableView*)tableView;


- (void)setView:(UIView*)view;
- (void)reloadDataFromServer;

// private
- (void)_onDataLoadedWrapper:(OFPaginatedSeries*)resources isIncremental:(BOOL)isIncremental;

@end
