//
//  OFTableControllerHelper+Pagination.mm
//  OpenFeint
//
//  Created by Jason Citron on 5/29/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFTableSequenceControllerHelper+Pagination.h"
#import "OFTableSequenceControllerHelper+Overridables.h"
#import "OFTableSectionDescription.h"
#import "OFTableSequenceControllerLoadMoreCell.h"
#import "OFDelegateChained.h"
#import "OFPaginatedSeriesHeader.h"

@implementation OFTableSequenceControllerHelper (Pagination)

- (bool)_isDataPaginated
{
	if ([self allowPagination])
	{
		OFTableSectionDescription* lastSection = [mSections lastObject];
		return lastSection.page.header != nil;
	}
	else
	{
		return false;
	}	
}

- (void)_createAndDisplayPaginationControls
{
	if([self _isDataPaginated])
	{
		OFTableSectionDescription* lastSection = [mSections lastObject];
		
		NSString* cellControllerName = @"TableSequenceControllerLoadMore";			
		
		NSAssert(lastSection.trailingCellName == nil || [lastSection.trailingCellName isEqualToString:cellControllerName], @"Pagination requires the trailing cell of the desired section be empty");
		if(lastSection.trailingCellName == nil)
		{
			lastSection.trailingCellName = cellControllerName;
		}
	}
}

- (NSIndexPath*)_getPaginationCellIndexPath
{
	const unsigned int lastSectionIndex = [mSections count] - 1;
	const unsigned int paginationCellRowNumber = [self.tableView numberOfRowsInSection:lastSectionIndex] - 1;
	return [NSIndexPath indexPathForRow:paginationCellRowNumber inSection:lastSectionIndex];
}

- (void)_setPaginationCellIsLoading:(BOOL)isLoading
{	
	NSIndexPath* loadMoreButtonIndexPath = [self _getPaginationCellIndexPath];
	if (isLoading)
	{
		[self.tableView deselectRowAtIndexPath:loadMoreButtonIndexPath animated:YES];
	}
	UITableViewCell* lastcell = [self.tableView cellForRowAtIndexPath:loadMoreButtonIndexPath];
	OFTableSequenceControllerLoadMoreCell* paginationCell = (OFTableSequenceControllerLoadMoreCell*)lastcell;
	paginationCell.isLoading = isLoading;
	mIsLoadingNextPage = isLoading;
}

- (void)_paginationRequestSucceeded:(OFPaginatedSeries*)page
{
	NSIndexPath* cellToRefresh = [self _getPaginationCellIndexPath];
	
	[self _setPaginationCellIsLoading:false];
	[self _onDataLoadedWrapper:page isIncremental:YES];
	
	// This causes the old pagination cell to be refreshed by the view
	[self.tableView beginUpdates];
		[self.tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:cellToRefresh] withRowAnimation:UITableViewRowAnimationFade];
		[self.tableView insertRowsAtIndexPaths:[NSArray arrayWithObject:cellToRefresh] withRowAnimation:UITableViewRowAnimationFade];	
	[self.tableView endUpdates];
}

- (void)_paginationRequestFailed:(NSArray*)resources
{
	[self _setPaginationCellIsLoading:false];
		
	[[[[UIAlertView alloc] 
		initWithTitle:nil
		message:@"OpenFeint was unable to load more items. Please try again"
		delegate:nil
		cancelButtonTitle:@"Ok"
		otherButtonTitles:nil] autorelease] show];
		
	[self getOnFailureDelegate].invoke(resources);
}

- (void)onTrailingCellWasClickedForSection:(OFTableSectionDescription*)section
{
	if (section != [mSections lastObject])
	{
		return;
	}

	if(mIsLoadingNextPage || [section.page.header isLastPageLoaded])
	{
		return;
	}
	
	mNumLoadedPages += 1;
	
	[self _setPaginationCellIsLoading:true];
	
	OFDelegate paginationSucceeded(self, @selector(_paginationRequestSucceeded:));
	OFDelegate paginationFailed(self, @selector(_paginationRequestFailed:));
		
	[self doIndexActionWithPage:mNumLoadedPages onSuccess:paginationSucceeded onFailure:paginationFailed];
}

@end
