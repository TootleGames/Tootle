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

#import "OFImageCache.h"

namespace
{
	static NSInteger const kCacheSize = 50;
}

@implementation OFImageCache

+ (OFImageCache*)sharedInstance
{
	static OFImageCache* sInstance = nil;
	if (sInstance == nil)
		sInstance = [OFImageCache new];
		
	return sInstance;
}

- (id)init
{
	self = [super init];
	if (self != nil)
	{
		mCache = [[NSMutableDictionary dictionaryWithCapacity:kCacheSize] retain];
	}
	
	return self;
}

- (void)dealloc
{
	OFSafeRelease(mCache);
	[super dealloc];
}

- (UIImage*)fetch:(NSString*)identifier
{
	return [mCache objectForKey:identifier];
}

- (void)store:(UIImage*)image withIdentifier:(NSString*)identifier
{
	if ([mCache count] == kCacheSize)
	{
		NSInteger const kElementsToPurge = kCacheSize / 3;
		NSMutableArray* keysToRemove = [NSMutableArray arrayWithCapacity:kElementsToPurge];
		NSEnumerator* cacheEnumerator = [mCache keyEnumerator];
		
		for (int i = 0; i < kElementsToPurge; ++i)
			[keysToRemove addObject:[cacheEnumerator nextObject]];
		
		for (NSString* key in keysToRemove)
			[mCache removeObjectForKey:key];
	}
	
	[mCache setObject:image forKey:identifier];
}

- (void)purge
{
	[mCache removeAllObjects];
}

@end