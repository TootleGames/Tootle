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
#import "OFTableCellHelper.h"
#import "OFTableCellHelper+Overridables.h"
#import "OFTableCellBackgroundView.h"

@implementation OFTableCellHelper

@synthesize isLeadingCell = mIsLeadingCell;
@synthesize isTrailingCell = mIsTrailingCell;

- (void)setSelected:(BOOL)selected animated:(BOOL)animated 
{
    [super setSelected:selected animated:animated];
}

- (void)dealloc 
{
	[mResource release];
	[super dealloc];
}

- (void)changeStyle:(UITableViewStyle)style withRoundedEdge:(OFTableCellRoundedEdge)edge
{
	// OF2.0UI - For pre-release!
	if (mIsTrailingCell || mIsLeadingCell)
	{
		NSString* bgImage = mIsTrailingCell ? @"OFTrailingCellBackground.png" : @"OFLeadingCellBackground.png";
		OFTableCellRoundedEdge roundedEdge = (style == UITableViewStylePlain) ? kRoundedEdge_None : edge;
		self.backgroundView = [[[OFTableCellBackgroundView alloc] initWithFrame:self.frame andImageName:bgImage andRoundedEdge:roundedEdge] autorelease];
	}

	// OF2.0UI - Table cell backgrounds -- still needs to be fleshed out
//	NSString* bgImage = @"OFTableGroupedBackground.png";
//	NSString* bgSelectedImage = @"OFTableGroupedBackgroundSelected.png";
//	OFTableCellRoundedEdge roundedEdge = edge;
//	if (style == UITableViewStylePlain)
//	{
//		roundedEdge = kRoundedEdge_None;
//		bgImage = @"OFTablePlainBackground.png";
//		bgSelectedImage = @"OFTablePlainBackgroundSelected.png";
//	}
//
//	self.backgroundView = [[[OFTableCellBackgroundView alloc] initWithFrame:self.frame andImageName:bgImage andRoundedEdge:roundedEdge] autorelease];
//	self.selectedBackgroundView = [[[OFTableCellBackgroundView alloc] initWithFrame:self.frame andImageName:bgSelectedImage andRoundedEdge:roundedEdge] autorelease];
}

- (void)changeResource:(OFResource*)resource
{
	[mResource release];
	mResource = [resource retain];
	[self onResourceChanged:mResource];
}

@end