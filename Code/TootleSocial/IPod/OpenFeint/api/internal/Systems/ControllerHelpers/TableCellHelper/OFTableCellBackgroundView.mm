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

#import "OFTableCellBackgroundView.h"

@implementation OFTableCellBackgroundView

- (id)initWithFrame:(CGRect)frame andImageName:(NSString*)imageName andRoundedEdge:(OFTableCellRoundedEdge)roundedEdge
{
	self = [super initWithFrame:frame];
	if (self != nil)
	{
		self.backgroundColor = [UIColor clearColor];
		self.opaque = NO;
		
		mImage = [[UIImage imageNamed:imageName] retain];
		mEdgeType = roundedEdge;
	}
	
	return self;
}

- (void)dealloc
{
	OFSafeRelease(mImage);
	[super dealloc];
}

- (void)drawRect:(CGRect)rect
{
	float minx = CGRectGetMinX(rect);
	float miny = CGRectGetMinY(rect);
	float maxx = CGRectGetWidth(rect);
	float midx = maxx * 0.5f;
	float maxy = CGRectGetHeight(rect);
	float midy = maxy * 0.5f;

	float const radius = 10.0f;
	
	CGContextRef ctx = UIGraphicsGetCurrentContext();

	switch (mEdgeType)
	{
		case kRoundedEdge_Top:
		{
			CGContextBeginPath(ctx);
			CGContextMoveToPoint(ctx, minx, midy);
			CGContextAddArcToPoint(ctx, minx, miny, midx, miny, radius);
			CGContextAddArcToPoint(ctx, maxx, miny, maxx, midy, radius);
			CGContextAddLineToPoint(ctx, maxx, maxy);
			CGContextAddLineToPoint(ctx, minx, maxy);
			CGContextAddLineToPoint(ctx, minx, midy);
			CGContextClosePath(ctx);
			CGContextClip(ctx);
		} break;
		
		case kRoundedEdge_Bottom:
		{
			CGContextBeginPath(ctx);
			CGContextMoveToPoint(ctx, minx, midy);
			CGContextAddLineToPoint(ctx, minx, miny);
			CGContextAddLineToPoint(ctx, maxx, miny);
			CGContextAddLineToPoint(ctx, maxx, midy);
			CGContextAddArcToPoint(ctx, maxx, maxy, midx, maxy, radius);
			CGContextAddArcToPoint(ctx, minx, maxy, minx, midy, radius);
			CGContextClosePath(ctx);
			CGContextClip(ctx);
		} break;
		
		case kRoundedEdge_TopAndBottom:
		{
			CGContextBeginPath(ctx);
			CGContextMoveToPoint(ctx, minx, midy);
			CGContextAddArcToPoint(ctx, minx, miny, midx, miny, radius);
			CGContextAddArcToPoint(ctx, maxx, miny, maxx, midy, radius);
			CGContextAddArcToPoint(ctx, maxx, maxy, midx, maxy, radius);
			CGContextAddArcToPoint(ctx, minx, maxy, minx, midy, radius);
			CGContextClosePath(ctx);
			CGContextClip(ctx);
		} break;
		
		default: break;
	}

	[mImage drawInRect:rect];
}

@end