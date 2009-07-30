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

#import "OFKeyValuePair.h"

@implementation OFKeyValuePair

@synthesize key;
@synthesize value;

+ (OFKeyValuePair*)pairWithKey:(NSString*)key andValue:(NSObject*)value
{
	return [[[OFKeyValuePair alloc] initWithKey:key andValue:value] autorelease];
}
- (id)init
{
	return [self initWithKey:nil andValue:nil];
}

- (id)initWithKey:(NSString*)_key andValue:(NSObject*)_value
{
	self = [super init];
	if (self)
	{
		self.key = _key;
		self.value = _value;
	}
	return self;
}

- (void)dealloc
{
	self.key = nil;
	self.value = nil;
	[super dealloc];
}

@end

