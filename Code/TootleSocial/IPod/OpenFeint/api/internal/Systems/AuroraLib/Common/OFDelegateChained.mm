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
#include "OFDelegateChained.h"

@implementation OFDelegateChained

+ (id)delegateWith:(const OFDelegate&)delegate
{
	return [[[OFDelegateChained alloc] initWith:delegate] autorelease];
}

- (id)initWith:(const OFDelegate&)delegate
{
	if(self = [super init])
	{
		mChainedDelegate = delegate;
	}
	return self;
}

- (void)invoke
{
	mChainedDelegate.invoke();
}

- (void)invokeWith:(NSObject*)param
{
	mChainedDelegate.invoke(param);
}

- (void)invokeWith:(NSObject*)param andDelay:(NSTimeInterval)delay
{
	mChainedDelegate.invoke(param, delay);
}

@end
