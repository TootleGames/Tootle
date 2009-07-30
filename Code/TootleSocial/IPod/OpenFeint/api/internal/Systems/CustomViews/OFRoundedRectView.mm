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

#import "OFRoundedRectView.h"

@implementation OFRoundedRectView

- (void)_commonInit
{
	[super setBackgroundColor:[UIColor clearColor]];

	self.opaque = NO;
	
	mRoundedRect = [[[UIImage imageNamed:@"OFRoundedRectBorder.png"] stretchableImageWithLeftCapWidth:10.f topCapHeight:10.f] retain];
}

- (id)initWithCoder:(NSCoder*)aDecoder
{
	self = [super initWithCoder:aDecoder];
	if (self != nil)
	{
		[self _commonInit];
	}

	return self;
}

- (id)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if (self != nil)
	{
		[self _commonInit];
		mBackgroundColor = [UIColor colorWithRed:0.f green:0.f blue:0.f alpha:0.4f];	
	}
	
	return self;
}

- (void)dealloc
{
	OFSafeRelease(mBackgroundColor);
	OFSafeRelease(mRoundedRect);
	[super dealloc];
}

- (void)setBackgroundColor:(UIColor*)backgroundColor
{
	OFSafeRelease(mBackgroundColor);
	mBackgroundColor = [backgroundColor retain];
}

- (void)drawRect:(CGRect)rect
{
	float minx = CGRectGetMinX(rect);
	float miny = CGRectGetMinY(rect);
	float maxx = CGRectGetWidth(rect);
	float midx = maxx * 0.5f;
	float maxy = CGRectGetHeight(rect);
	float midy = maxy * 0.5f;

	// adill note: photoshop radius used to make the rounded rect border image + 0.5f seems to make things work well.
	float const radius = 10.5f;
	
	CGContextRef ctx = UIGraphicsGetCurrentContext();

	CGContextSaveGState(ctx);
	{
		CGContextBeginPath(ctx);
		CGContextMoveToPoint(ctx, minx, midy);
		CGContextAddArcToPoint(ctx, minx, miny, midx, miny, radius);
		CGContextAddArcToPoint(ctx, maxx, miny, maxx, midy, radius);
		CGContextAddArcToPoint(ctx, maxx, maxy, midx, maxy, radius);
		CGContextAddArcToPoint(ctx, minx, maxy, minx, midy, radius);
		CGContextClosePath(ctx);
		CGContextClip(ctx);

		[mBackgroundColor setFill];
		CGContextFillRect(ctx, rect);
	}
	CGContextRestoreGState(ctx);

	[mRoundedRect drawInRect:rect];
}

@end