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
#import "OFTimerHeartbeat.h"

@implementation OFTimerHeartbeat

- (id)initWithTarget:(NSObject*)target andSelector:(SEL)selector
{
	if(self = [super init])
	{
		mTarget = target;
		mSelector = selector;
	}
	return self;
}

- (void)pulse
{
	[mTarget performSelector:mSelector];
}

+ (NSTimer*) scheduledTimerWithInterval:(NSTimeInterval)interval target:(NSObject*)target selector:(SEL)selector
{	
	return [NSTimer scheduledTimerWithTimeInterval:interval
									 target:[[[OFTimerHeartbeat alloc] initWithTarget:target andSelector:selector] autorelease]
								   selector:@selector(pulse)
								   userInfo:nil
									repeats:YES];
}

@end