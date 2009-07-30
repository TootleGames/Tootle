//
//  OFTableControllerHelper+ViewDelegate.mm
//  OpenFeint
//
//  Created by Jason Citron on 4/20/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#import "OFDependencies.h"
#import "OFTableControllerHelper+ViewDelegate.h"
#import "OFTableControllerHelper+Overridables.h"
#import "OFTableCellHelper.h"
#import "OFResourceControllerMap.h"
#import "OFResource.h"
#import "OFControllerLoader.h"
#import "OFTableCellHelper.h"
#import "OFTableSectionDescription.h"
#import <objc/runtime.h>

@implementation OFTableControllerHelper (ViewDelegate)

- (OFTableCellHelper*)loadCell:(NSString*)cellName
{
	OFTableCellHelper* cell = (OFTableCellHelper*)OFControllerLoader::loadCell(cellName, self);
	CGRect cellRect = cell.frame;
	cellRect.size.width = self.tableView.frame.size.width;
	cell.frame = cellRect;
	return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	OFTableSectionDescription* section = [self getSectionForIndexPath:indexPath];
	
	if(indexPath.row == 0 && section.leadingCellName)
	{
		[self onLeadingCellWasClickedForSection:section];
	}
	else if(indexPath.row == [section countPageItems] - 1 && section.trailingCellName)
	{
		[self onTrailingCellWasClickedForSection:section];
	}
	else
	{	
		NSIndexPath* resourceIndexPath = indexPath;
		if(section.leadingCellName)
		{
			resourceIndexPath = [NSIndexPath indexPathForRow:indexPath.row - 1 inSection:indexPath.section];
		}
		
		[self onCellWasClicked:[self getCellAtIndexPath:resourceIndexPath] indexPathInTable:indexPath];
	}
}

- (void)tableView:(UITableView *)tableView accessoryButtonTappedForRowWithIndexPath:(NSIndexPath *)indexPath
{
	[self tableView:tableView didSelectRowAtIndexPath:indexPath];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	return [self getCellForIndex:indexPath inTable:tableView useHelperCell:YES].frame.size.height;
}

- (UITableViewCell*)getCellForIndex:(NSIndexPath*)indexPath inTable:(UITableView*)tableView useHelperCell:(BOOL)useHelperCell
{	
	unsigned int row = indexPath.row;	

	OFTableSectionDescription* section = [self getSectionForIndexPath:indexPath];
	
	const unsigned int sectionCellCount = [section countPageItems];
	
	OFTableCellRoundedEdge roundedEdge = kRoundedEdge_None;
	if (row == 0)
	{
		roundedEdge = kRoundedEdge_Top;
	}
	if (row == sectionCellCount - 1)
	{
		if (roundedEdge == kRoundedEdge_Top)
		{
			roundedEdge = kRoundedEdge_TopAndBottom;
		}
		else
		{
			roundedEdge = kRoundedEdge_Bottom;
		}
	}
	
	if([self isNewContentShownAtBottom])
	{	
		row = (sectionCellCount - 1) - row;
	}
	
	NSAssert2(row < sectionCellCount, @"Somehow we are requesting a cell outside of the number of rows we have. (expecting %d but only have %d)", row, sectionCellCount);

	bool isLeadingCell = false;
	bool isTrailingCell = false;
	NSString* tableCellName;
	id resource;
	{
		if(row == 0 && section.leadingCellName)
		{
			tableCellName = section.leadingCellName;
			resource = section;
			isLeadingCell = true;
		}
		else if(row == sectionCellCount - 1 && section.trailingCellName)
		{
			tableCellName = section.trailingCellName;
			resource = section;
			isTrailingCell = true;
		}
		else
		{
			if(section.leadingCellName)
			{
				row -= 1;
			}
			
			tableCellName = [self getCellControllerNameFromSection:section.page.objects atRow:row];
			resource = [self getResourceFromSection:section.page.objects atRow:row];
		}
	}
	
	if(useHelperCell)
	{
		OFTableCellHelper* cellHelper = [mHelperCellsForHeight objectForKey:tableCellName];
		if(!cellHelper)
		{
			cellHelper = [self loadCell:tableCellName];
			[mHelperCellsForHeight setObject:cellHelper forKey:tableCellName];
		}
		cellHelper.isTrailingCell = isTrailingCell;
		cellHelper.isLeadingCell = isLeadingCell;
		[cellHelper changeStyle:self.tableView.style withRoundedEdge:roundedEdge];
		[cellHelper changeResource:resource];
		return cellHelper;
	}

	OFTableCellHelper* cell = (OFTableCellHelper*)[tableView dequeueReusableCellWithIdentifier:tableCellName];
	if(!cell)
	{
		cell = [self loadCell:tableCellName];
		
		if(![cell isKindOfClass:[OFTableCellHelper class]])
		{
			NSAssert3(0, @"Expected UITableCellView named '%@' to be derived from '%s' but was '%s'", tableCellName, class_getName([OFTableCellHelper class]), class_getName([cell class]));			
		}
	}
	
	cell.isTrailingCell = isTrailingCell;
	cell.isLeadingCell = isLeadingCell;
	[cell changeStyle:self.tableView.style withRoundedEdge:roundedEdge];
	[cell changeResource:resource];
	
	if (isLeadingCell)
	{
		[self onLeadingCellWasLoaded:cell forSection:section];
	}
	else if (isTrailingCell)
	{
		[self onTrailingCellWasLoaded:cell forSection:section];
	}
	return cell;
}

@end
