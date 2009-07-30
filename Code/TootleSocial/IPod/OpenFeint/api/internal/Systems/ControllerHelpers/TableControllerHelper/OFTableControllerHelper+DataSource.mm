//
//  OFTableControllerHelper+DataSource.mm
//  OpenFeint
//
//  Created by Jason Citron on 4/20/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFDependencies.h"
#import "OFTableControllerHelper+DataSource.h"
#import "OFTableControllerHelper+Overridables.h"
#import "OFTableControllerHelper+ViewDelegate.h"
#import "OFTableSectionDescription.h"

@implementation OFTableControllerHelper (DataSource)

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	return mSections ? [mSections count] : 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	return [self getNumCellsInSection:section];
}

- (NSArray *)sectionIndexTitlesForTableView:(UITableView *)tableView
{
	return nil;
}

- (NSInteger)tableView:(UITableView *)tableView sectionForSectionIndexTitle:(NSString *)title atIndex:(NSInteger)index
{
	return 0;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
	if (section < [mSections count])
	{
		OFTableSectionDescription* ofSection = [mSections objectAtIndex:section];
		return ofSection.title;
	}
	else
	{
		return nil;
	}
}

- (NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section
{
	return nil;
}

- (UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	return [self getCellForIndex:indexPath inTable:tableView useHelperCell:NO];
}

@end
