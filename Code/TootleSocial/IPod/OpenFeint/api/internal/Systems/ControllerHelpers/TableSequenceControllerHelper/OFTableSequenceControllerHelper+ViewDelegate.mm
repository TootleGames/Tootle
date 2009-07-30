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
#import "OFTableSequenceControllerHelper+ViewDelegate.h"
#import "OFTableSequenceControllerHelper+Overridables.h"
#import "OFTableSectionDescription.h"
#import "OFControllerLoader.h"
#import "OFResourceViewHelper.h"

@implementation OFTableSequenceControllerHelper (ViewDelegate)

- (OFResource*) getResourceFromSection:(NSArray*)sectionCells atRow:(NSUInteger)row
{
	return (OFResource*)[sectionCells objectAtIndex:row];
}

- (NSString*) getCellControllerNameFromSection:(NSArray*)sectionCells atRow:(NSUInteger)row
{
	OFResource* resource = [self getResourceFromSection:sectionCells atRow:row];
	return mResourceMap->getControllerName([resource class]);
}

- (UIView*)getHeaderForSection:(NSInteger)section
{
	OFTableSectionDescription* tableDescription = (OFTableSectionDescription*)[mSections objectAtIndex:section];
	return tableDescription.headerView;
}

- (UIView*)getFooterForSection:(NSInteger)section
{
	OFTableSectionDescription* tableDescription = (OFTableSectionDescription*)[mSections objectAtIndex:section];
	return tableDescription.footerView;
}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section
{
	return [self getHeaderForSection:section];
}

- (UIView *)tableView:(UITableView *)tableView viewForFooterInSection:(NSInteger)section
{
	return [self getFooterForSection:section];
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
	UIView* header = [self getHeaderForSection:section];
	if (header)
	{
		return header.frame.size.height;
	}
	else
	{
		OFTableSectionDescription* tableDescription = (OFTableSectionDescription*)[mSections objectAtIndex:section];
		return tableDescription.title ? 30.f : 0.f;
	}
}

- (CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section
{
	UIView* footer = [self getFooterForSection:section];
	return footer ? footer.frame.size.height : 0.f;
}

@end
