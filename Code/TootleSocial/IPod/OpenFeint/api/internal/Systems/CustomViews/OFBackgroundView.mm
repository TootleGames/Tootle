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

#import "OFBackgroundView.h"

@implementation OFBackgroundView

- (id)initWithFrame:(CGRect)frame andGradientImage:(NSString*)gradientImageName andPatternImage:(NSString*)patternImageName;
{
	self = [super initWithFrame:frame];
	if (self != nil)
	{
		self.opaque = YES;
		self.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

		mBackgroundGradient = [[UIImage imageNamed:gradientImageName] retain];
		mBackgroundPattern = [[UIImage imageNamed:patternImageName] retain];
	}
	
	return self;
}

- (void)dealloc
{
	OFSafeRelease(mBackgroundGradient);
	OFSafeRelease(mBackgroundPattern);
	
	[super dealloc];
}

- (void)drawRect:(CGRect)rect
{
	[mBackgroundGradient drawAsPatternInRect:rect];
	
	CGContextRef ctx = UIGraphicsGetCurrentContext();
	
	CGContextSaveGState(ctx);
	CGContextSetBlendMode(ctx, kCGBlendModeMultiply);
	[mBackgroundPattern drawAsPatternInRect:rect];
	CGContextRestoreGState(ctx);
}

@end