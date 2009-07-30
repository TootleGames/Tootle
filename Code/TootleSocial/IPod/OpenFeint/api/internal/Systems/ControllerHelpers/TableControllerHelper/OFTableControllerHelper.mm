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
#import "OFTableControllerHelper.h"
#import "OFTableControllerHelper+Overridables.h"
#import "OFLoadingController.h"
#import "OFViewHelper.h"
#import "OFDeadEndErrorController.h"
#import "OFControllerLoader.h"
#import <objc/runtime.h>
#import "OFTableSectionDescription.h"
#import "OFPoller.h"
#import "OFNavigationController.h"
#import "OFTableControllerHeader.h"
#import "OFProfileFramedView.h"
#import "OpenFeint+Private.h"

@interface OFTableControllerHelper ()
- (void)_refreshData;
- (void)_displayEmptyDataSetView;
- (void)_removeEmptyDataSetView;
- (void)_createAndDisplayTableHeader;
- (void)_refreshDataIncrementally:(NSNotification*)notification;
- (void)_createSectionLeadingAndTrailingCells;
@end

@implementation OFTableControllerHelper

- (OFDelegate)getOnSuccessDelegate
{
	return OFDelegate(self, @selector(_onDataLoadedWrapper:));
}

- (OFDelegate)getOnFailureDelegate
{
	return OFDelegate(self, @selector(_onDataFailedLoading));
}

- (bool)isInHiddenTab
{
	OFNavigationController* navController = (OFNavigationController*)self.navigationController;
	OFAssert([navController isKindOfClass:[OFNavigationController class]], @"only use OFNavigationControllers");
	return navController.isInHiddenTab;
}

- (void) hideLoadingScreen
{
	[mLoadingScreen.view removeFromSuperview];
	OFSafeRelease(mLoadingScreen);
}

- (void)_refreshDataIncrementally:(NSNotification*)notification
{
	NSArray* resources = [notification.userInfo objectForKey:OFPollerNotificationKeyForResources];	
	[self _onDataLoadedWrapper:[OFPaginatedSeries paginatedSeriesFromArray:resources] isIncremental:YES];
}

- (void) showLoadingScreen
{
	if(mLoadingScreen && mLoadingScreen.view.superview == self.view)
	{
		return;
	}
	
	[self hideLoadingScreen];
	mLoadingScreen = [[OFLoadingController loadingControllerWithText:[self getTextToShowWhileLoading]] retain];
	[self.view.superview addSubview:mLoadingScreen.view];
}

- (void)viewDidLoad
{
	[super viewDidLoad];
	self.tableView.dataSource = self;
	if (self.tableView.style == UITableViewStyleGrouped)
	{
		self.view.backgroundColor = [UIColor clearColor];
	}
	
	mHelperCellsForHeight = [[NSMutableDictionary dictionaryWithCapacity:1] retain];
}

- (UITableView*)tableView
{
	UITableView* table = [super tableView];
	if(!table)
	{
		return mMyTableView;
	}
	
	return table;
}

- (void)setView:(UIView*)view
{
	if (![view isKindOfClass:[UITableView class]] && mMyTableView == nil)
	{
		mMyTableView = self.tableView;
	}
	
	[super setView:view];
}

- (void)reloadDataFromServer
{
	[self _refreshData];
}

- (void)viewWillAppear:(BOOL)animated
{
// -----
//	adill: don't invoke the super here because we DO NOT want the keyboard notifications to be registered.
//		   instead we're just going to do what the header file says UITableViewController does.
//	[super viewWillAppear:animated];
	if ([self.tableView numberOfSections] == 0)
	{
		[self.tableView reloadData];
	}
	else
	{
		NSIndexPath* selection = [self.tableView indexPathForSelectedRow];
		if (selection)
		{
			[self.tableView deselectRowAtIndexPath:selection animated:animated];
		}
	}
// -----

	if([self shouldRefreshAfterNotification])
	{
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_refreshDataIncrementally:) name:[self getNotificationToRefreshAfter] object:nil];
	}
}

- (void)viewDidAppear:(BOOL)animated
{
	[super viewDidAppear:animated];
	
	if(mSections == nil || [self shouldAlwaysRefreshWhenShown])
	{
		[self _refreshData];
	}		
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
	OFViewHelper::resignFirstResponder(self.view);
	[self hideLoadingScreen];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) _refreshData
{
	[self showLoadingScreen];
	[self doIndexActionOnSuccess:[self getOnSuccessDelegate] onFailure:[self getOnFailureDelegate]];
}

- (void)_displayEmptyDataSetView
{
	OFSafeRelease(mEmptyTableController);
	mEmptyTableController = [[self getNoDataFoundViewController] retain];
	if (mEmptyTableController == nil)
	{
		mEmptyTableController = (OFDeadEndErrorController*)[OFControllerLoader::load(@"EmptyDataSet") retain];
		mEmptyTableController.message = [self getNoDataFoundMessage];
	}
	
	[mEmptyTableController viewWillAppear:NO];
	CGRect noDataRect = mEmptyTableController.view.frame;
	noDataRect.size.width = self.tableView.frame.size.width;
	noDataRect.size.height = self.tableView.frame.size.height;
	if (mTableHeaderController)
	{
		noDataRect.size.height -= mTableHeaderController.view.frame.size.height;
	}
	mEmptyTableController.view.frame = noDataRect;
	self.tableView.tableFooterView = mEmptyTableController.view;
	[self.tableView reloadData];
}

- (void)_removeEmptyDataSetView
{
	if (mEmptyTableController)
	{
		OFSafeRelease(mEmptyTableController);
		self.tableView.tableFooterView = nil;
	}
}

- (void)_onDataLoadedWrapper:(OFPaginatedSeries*)resources
{
	[self _onDataLoadedWrapper:resources isIncremental:NO];
}

- (void)_onDataLoadedWrapper:(OFPaginatedSeries*)resources isIncremental:(BOOL)isIncremental
{
	[self hideLoadingScreen];
	
	[self _onDataLoaded:resources isIncremental:isIncremental];

	[self _createAndDisplayTableHeader];
	
	if([mSections count] > 0)
	{
		[self _removeEmptyDataSetView];
	}
	else
	{
		[self _displayEmptyDataSetView];
		[self _reloadTableData];
	}
}

- (void) _reloadTableData
{
	[self _createSectionLeadingAndTrailingCells];
	[self.tableView reloadData];
}

- (bool)canReceiveCallbacksNow
{
	return [self navigationController] != nil;
}

- (void)_createAndDisplayTableHeader
{	
	NSString* tableHeaderControllerName = [self getTableHeaderControllerName];
	if(tableHeaderControllerName)
	{
		if(!mTableHeaderController)
		{
			mTableHeaderController = [OFControllerLoader::load(tableHeaderControllerName, self) retain];
			[self onTableHeaderCreated:mTableHeaderController];
			
			if ([mTableHeaderController conformsToProtocol:@protocol(OFTableControllerHeader)])
			{
				UIViewController<OFTableControllerHeader>* headerController = mTableHeaderController;
				UIView* contentView = self.view;
				if ([self.view isKindOfClass:[OFProfileFramedView class]])
				{
					OFProfileFramedView* profileView = (OFProfileFramedView*)self.view;
					contentView = profileView.contentView;
				}
				[headerController resizeView:contentView];
			}
			
			self.tableView.tableHeaderView = mTableHeaderController.view;
		}
	}
	
	// citron note: Setting a tableHeaderView to nil may cause the table to fill in blank space as if it
	//				had a full screen empty header. We have no idea why, but this seems to fix the issue. 
	else if(self.tableView.tableHeaderView != nil) 
	{
		self.tableView.tableHeaderView = nil;
	}
}

- (void)_createSectionLeadingAndTrailingCells
{	
	for(OFTableSectionDescription* section in mSections)
	{
		if(section.leadingCellName == nil)
		{
			section.leadingCellName = [self getLeadingCellControllerNameForSection:section];
		}
		
		if(section.trailingCellName == nil)
		{
			section.trailingCellName = [self getTrailingCellControllerNameForSection:section];
		}
	}
}

- (bool)_isScrolledToHead
{
	NSIndexPath* lastIndexPath = [[self.tableView indexPathsForVisibleRows] lastObject];
	
	const unsigned int section = lastIndexPath.section;
	const unsigned int row = lastIndexPath.row;
	
	if(mSections == nil)
	{
		return true;
	}
	
	if([self isNewContentShownAtBottom])
	{
		return (section == [mSections count] - 1 && row == [self getNumCellsInSection:lastIndexPath.section] - 1);
	}

	return (section == 0 && row == 0);
}

- (void)_scrollToTableHead:(BOOL)animate
{	
	if(mSections == nil)
	{
		return;
	} 
	
	NSIndexPath* firstCell = nil;
	CGRect targetRect = CGRectZero;
	
	if([self isNewContentShownAtBottom])
	{
		if(self.tableView.tableFooterView)
		{
			targetRect = self.tableView.tableFooterView.frame;
		}
		else
		{
			int lastSectionIndex = [mSections count] - 1;
			int numCellsInSection = [self getNumCellsInSection:lastSectionIndex];
			if (numCellsInSection > 0)
			{
				firstCell = [NSIndexPath indexPathForRow:numCellsInSection - 1 inSection:lastSectionIndex];
			}
			if (firstCell)
			{
				targetRect = [self.tableView rectForRowAtIndexPath:firstCell];
			}
		}
	}
	else
	{
		if(self.tableView.tableHeaderView)
		{
			targetRect = self.tableView.tableHeaderView.frame;
		}
		else
		{
			if ([self getNumCellsInSection:0] > 0)
			{
				firstCell = [NSIndexPath indexPathForRow:0 inSection:0];
			}
			if (firstCell)
			{
				targetRect = [self.tableView rectForRowAtIndexPath:firstCell];
			}
		}
	}	
	
	if(!CGRectEqualToRect(CGRectZero, targetRect))
	{
		[self.tableView scrollRectToVisible:targetRect animated:animate];
	}
}

- (void)_onDataFailedLoading
{
	[self hideLoadingScreen];
}

- (void)dealloc
{
	[self.tableView setDelegate:nil];
	[self.tableView setDataSource:nil];
	[self.tableView setTableHeaderView:nil];
	
	OFSafeRelease(mEmptyTableController);
	[mTableHeaderController.view removeFromSuperview];
	[mTableHeaderController release];
	mTableHeaderController = nil;
	
	[mHelperCellsForHeight release];
	mHelperCellsForHeight = nil;
	
	[mSections release];
	mSections = nil;
	
	OFSafeRelease(mContainerView);
	OFSafeRelease(mDashboardNotificationView);
	
	[self setTableView:nil];

	[super dealloc];
}

- (OFTableSectionCellDescription*)getCellAtIndexPath:(NSIndexPath*)indexPath
{
	int row = indexPath.row;
	NSArray* cellsInSection = [self getSectionForIndexPath:indexPath].page.objects;
	return [cellsInSection objectAtIndex:row];
}

- (OFTableSectionDescription*)getSectionForIndexPath:(NSIndexPath*)indexPath
{
	if (mSections)
	{
		NSAssert2(indexPath.section < [mSections count], @"invalid section index %d in class %s", indexPath.section, class_getName([self class]));
		
		if (indexPath.section < [mSections count])
		{
			return [mSections objectAtIndex:indexPath.section];
		}
		else
		{
			return nil;
		}
	}
	else
	{
		return nil;
	}
}

- (OFTableSectionDescription*)getSectionWithIdentifier:(NSString*)identifier
{
	if (mSections)
	{
		for (OFTableSectionDescription* section in mSections)
		{
			if ([section.identifier isEqualToString:identifier])
			{
				return section;
			}
		}
	}
	return nil;
}

- (NSInteger)getNumCellsInTable
{
	int numResources = 0;
	for (OFTableSectionDescription* section in mSections)
	{
		numResources += [section countPageItems];
	}
	return numResources;
}

- (NSInteger)getNumCellsInSection:(NSInteger)sectionIndex
{
	if (mSections)
	{
		NSAssert2(sectionIndex < [mSections count], @"invalid section index %d in class %s", sectionIndex, class_getName([self class]));
		if (sectionIndex < [mSections count])
		{
			OFTableSectionDescription* section = [mSections objectAtIndex:sectionIndex];
			return [section countPageItems];
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

@end
