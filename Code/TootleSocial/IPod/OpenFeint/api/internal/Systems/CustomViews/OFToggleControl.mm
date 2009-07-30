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

#import "OFToggleControl.h"

@implementation OFToggleControl

@synthesize isLeftSelected = mIsLeftSelected;

- (void)_commonInit
{
	self.backgroundColor = [UIColor clearColor];
	self.opaque = NO;
	
	mIsLeftSelected = YES;
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
	}
	
	return self;
}

- (void)dealloc
{
	OFSafeRelease(mImageLeftSelected);
	OFSafeRelease(mImageRightSelected);
	
	[super dealloc];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	UITouch* touch = [touches anyObject];
	
	if ([touch tapCount] == 1)
	{
		mIsLeftSelected = !mIsLeftSelected;
		[self sendActionsForControlEvents:UIControlEventValueChanged];
	}

	[self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect
{
	CGPoint pos = CGPointMake(0.0f, 0.0f);
	
	if (mIsLeftSelected)
		[mImageLeftSelected drawAtPoint:pos];
	else
		[mImageRightSelected drawAtPoint:pos];
}

- (void)setLeftSelected:(BOOL)isLeftSelected
{
	mIsLeftSelected = isLeftSelected;
	[self sendActionsForControlEvents:UIControlEventValueChanged];
	[self setNeedsDisplay];
}

- (void)setImageForLeftSelected:(UIImage*)selected
{
	OFSafeRelease(mImageLeftSelected);
	mImageLeftSelected = [selected retain];
}

- (void)setImageForRightSelected:(UIImage*)selected
{
	OFSafeRelease(mImageRightSelected);
	mImageRightSelected = [selected retain];
}

@end