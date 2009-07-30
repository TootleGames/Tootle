//
//  OFTableSequenceControllerLoadMoreCell.mm
//  OpenFeint
//
//  Created by Jason Citron on 5/29/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFDependencies.h"
#import "OFTableSequenceControllerLoadMoreCell.h"
#import "OFViewHelper.h"
#import "OFPaginatedSeries.h"
#import "OFPaginatedSeriesHeader.h"
#import "OFTableSectionDescription.h"

@implementation OFTableSequenceControllerLoadMoreCell

@dynamic isLoading;
@synthesize lastLoadedPageHeader;

- (void)onResourceChanged:(OFResource*)resource
{
	self.lastLoadedPageHeader = ((OFTableSectionDescription*)resource).page.header;
	[self setIsLoading:false];
}

- (void)setIsLoading:(bool)value
{
	UIActivityIndicatorView* loadingIcon = (UIActivityIndicatorView*)OFViewHelper::findViewByTag(self, 1);
	if(value)
	{
		[loadingIcon startAnimating];
	}
	else
	{
		[loadingIcon stopAnimating];
	}
	
	UILabel* loadingText = (UILabel*)OFViewHelper::findViewByTag(self, 2);
	UILabel* showingText = (UILabel*)OFViewHelper::findViewByTag(self, 3);
		
	if(value)
	{
		loadingText.text = @"Loading";
		showingText.text = @"";
	}
	else
	{				
		unsigned int amountShowing = self.lastLoadedPageHeader.currentPage * self.lastLoadedPageHeader.perPage;

		if([self.lastLoadedPageHeader isLastPageLoaded])
		{
			amountShowing = self.lastLoadedPageHeader.totalObjects;
			
			loadingText.text = [NSString stringWithFormat:@"No More To Load", self.lastLoadedPageHeader.perPage];
			loadingText.textColor = [UIColor darkGrayColor];
						
			self.selectionStyle = UITableViewCellSelectionStyleNone;
		}
		else
		{
			unsigned int amountToLoad = self.lastLoadedPageHeader.perPage;
			if(self.lastLoadedPageHeader.currentPage == self.lastLoadedPageHeader.totalPages - 1)
			{
				amountToLoad = self.lastLoadedPageHeader.totalObjects - amountShowing;
			}
		
			loadingText.text = [NSString stringWithFormat:@"Load %d More...", amountToLoad];
		}		
		
		showingText.text = [NSString stringWithFormat:@"Showing %d of %d", amountShowing, self.lastLoadedPageHeader.totalObjects];		
	}
			
	[self setSelected:NO animated:YES];
}

@end
