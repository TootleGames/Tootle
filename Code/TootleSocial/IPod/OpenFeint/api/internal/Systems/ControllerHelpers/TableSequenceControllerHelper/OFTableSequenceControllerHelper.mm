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
#import "OFTableSequenceControllerHelper.h"
#import "OFService+Overridables.h"
#import "OFResourceControllerMap.h"
#import "OFXmlDocument.h"
#import "OFTableSequenceControllerHelper+Overridables.h"
#import "OFControllerLoader.h"
#import "OFTableSectionDescription+ResourceAdditions.h"
#import "OFTableSequenceControllerHelper+Pagination.h"

@implementation OFTableSequenceControllerHelper

- (void)viewDidLoad
{
	[super viewDidLoad];

	mIsLoadingNextPage = false;
	mNumLoadedPages = 0;
	mResourceMap.reset(new OFResourceControllerMap); 
	[self populateResourceMap:mResourceMap.get()];
}

- (void)_rearrangeViewsWithToolbarOnTopAtBottom:(UIView*)toolbar andNotificationView:(UIView*)notificationView
{
	if (mContainerView)
	{	
		CGRect tableFrame = self.tableView.frame;
		if (mDashboardNotificationView)
		{
			tableFrame.size.height += mDashboardNotificationView.frame.size.height;
			[mDashboardNotificationView removeFromSuperview];
			OFSafeRelease(mDashboardNotificationView);
		}
		if (notificationView)
		{
			mDashboardNotificationView = [notificationView retain];
			CGRect notificationRect = notificationView.frame;
			tableFrame.size.height -= notificationRect.size.height;
			notificationRect.origin.y = tableFrame.size.height;
			notificationView.frame = notificationRect;
			[mContainerView addSubview:notificationView];
		}
		self.tableView.frame = tableFrame;
	}
	else
	{
		mDashboardNotificationView = [notificationView retain];
		UITableView* table = self.tableView;
		CGRect viewFrame = self.view.frame;
		mContainerView = [[UIView alloc] initWithFrame:viewFrame];
		mContainerView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
		
		// jw note: Any translation is now in the container so the child should be placed at the containers origin
		viewFrame.origin = CGPointZero;
		self.view.frame = viewFrame;
		
		[table removeFromSuperview];
		CGRect notificationRect = notificationView ? notificationView.frame : CGRectZero;
		CGRect tableFrame = table.frame;
		tableFrame.origin.y = 0;
		tableFrame.size.height -= toolbar.frame.size.height;
		tableFrame.size.height -= notificationRect.size.height;
		table.frame = tableFrame;
		
		CGRect toolbarFrame = toolbar.frame;
		toolbar.frame = CGRectMake(0.0f, viewFrame.size.height - toolbar.frame.size.height, toolbar.frame.size.width, toolbar.frame.size.height);
		[mContainerView addSubview:table];
		if (notificationView)
		{
			notificationRect.origin.y = tableFrame.size.height;
			notificationView.frame = notificationRect;
			[mContainerView addSubview:notificationView];
		}
		[mContainerView addSubview:toolbar];
		
		mMyTableView = table;
		
		self.view = mContainerView;	
	}
	
}

- (NSArray*)_sectionsFromResources:(OFPaginatedSeries*)resources
{
	OFSafeRelease(mMetaDataObjects);
	if (resources.tableMetaDataObjects)
	{
		mMetaDataObjects = [resources.tableMetaDataObjects retain];
		resources.tableMetaDataObjects = nil;
	}
	if ([resources count] > 0)
	{
		if ([[resources objectAtIndex:0] isKindOfClass:[OFTableSectionDescription class]])
		{
			return resources.objects;
		}
		else
		{
			OFTableSectionDescription* section = [[OFTableSectionDescription new] autorelease];
			section.page = [OFPaginatedSeries paginatedSeriesFromSeries:resources];
			
			return [NSArray arrayWithObject:section];
		}
	}

	return nil;
}

-(void) _appendNumItems:(unsigned int)newResourcesCount toTableSectionIndex:(unsigned int)targetSectionIndex
{
	OFTableSectionDescription* targetSection = [mSections objectAtIndex:targetSectionIndex];
	const unsigned int numCurrentResourcesInSection = [targetSection.page count];
	const unsigned int indexToAppendARow = numCurrentResourcesInSection > 0 ? numCurrentResourcesInSection - 1 : 0;

	[self.tableView beginUpdates];	

	NSMutableArray* indexPaths = [NSMutableArray arrayWithCapacity:newResourcesCount];
	for(unsigned int i = 0; i < newResourcesCount; ++i)
	{
		[indexPaths addObject:[NSIndexPath indexPathForRow:indexToAppendARow inSection:targetSectionIndex]];
	}
	
	[self.tableView insertRowsAtIndexPaths:indexPaths withRowAnimation:UITableViewRowAnimationFade];
	
	[self.tableView endUpdates];
}

- (void)_onDataLoaded:(OFPaginatedSeries*)resources isIncremental:(BOOL)isIncremental
{
	const bool isFirstTimeShowingData = (mSections == nil);
	const bool isScrolledToHead = [self _isScrolledToHead];
	
	NSArray* sections = [self _sectionsFromResources:resources];
	
	if(isFirstTimeShowingData)
	{
		mNumLoadedPages = 1;
	}
	
	bool allowPagination = [self allowPagination] && [mSections count] == [sections count];
	if(!isFirstTimeShowingData && isIncremental && allowPagination)
	{
		OFTableSectionDescription* newResources = [sections lastObject];
		OFTableSectionDescription* existingResources = [mSections lastObject];					
		const bool shouldPrependContents = ([self _isDataPaginated] == false);
		const unsigned int numNewResourcesAdded = [existingResources addContentsOfSectionWhereUnique:newResources areSortedDescending:YES shouldPrependContents:shouldPrependContents];

		// Animating in multiple rows when the current view is not full causes a fade in animation to be played on top of a
	    // from bottom animation and the app crashes. We've found no work arounds in the animations so the solution is to
		// only append rows to a full screen. The cost of reloading is small without a full screen of data anyways.
		// HOWEVER! There are some issue with the contentsize calculation on the SIMULATOR so if more cells than
		// an arbitrary number do the same.
		// ALSO Pagination crashes on page sizes of 10 in OS 3.0 if we try to append so lets not do it.
		const bool currentContentFillsScreen = [self.tableView contentSize].height > self.tableView.frame.size.height; 
		if (![self _isDataPaginated] && (currentContentFillsScreen || [existingResources.page count] > 15))
		{
			const unsigned int lastSectionIndex = [mSections count] - 1;	
			[self _appendNumItems:numNewResourcesAdded toTableSectionIndex:lastSectionIndex];
		}
		else
		{
			[self _reloadTableData]; 
		}
		
	}
	else
	{
		[mSections release];
		mSections = [sections retain];
		
		if([sections count])
		{
			[self onSectionsCreated];
			[self _reloadTableData];
		}
	}
	
	if(isScrolledToHead)
	{
		[self _scrollToTableHead:!isFirstTimeShowingData];
	}
	else
	{
		[self.tableView flashScrollIndicators];
	}
}

- (NSMutableArray*)getMetaDataOfType:(Class)metaDataType
{
	NSMutableArray* objects = [[NSMutableArray new] autorelease];
	for (NSObject* curItem in mMetaDataObjects)
	{
		if ([curItem isKindOfClass:metaDataType])
		{
			[objects addObject:curItem];
		}
	}
	return objects;
}

- (void)_reloadTableData
{
	[self _createAndDisplayPaginationControls];
	[super _reloadTableData];
}


@end