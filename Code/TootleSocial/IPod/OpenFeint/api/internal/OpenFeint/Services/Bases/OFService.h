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

#import "OFCallbackable.h"
#import "OFPointer.h"
#import "OFResourceNameMap.h"
#import "OFDelegate.h"

@interface OFService : NSObject<OFCallbackable>
{
@private
	OFPointer<OFResourceNameMap> mKnownResources;
}

- (bool) canReceiveCallbacksNow;
- (OFResourceNameMap*) getKnownResources;

@end

#define OPENFEINT_DECLARE_AS_SERVICE(interfaceName) \
+ (interfaceName*)sharedInstance;					\
+ (void)initializeService;							\
+ (void)shutdownService;

#define OPENFEINT_DEFINE_SERVICE_INSTANCE(interfaceName)	\
namespace													\
{															\
	static interfaceName* interfaceName##Instance = nil;	\
}

#define OPENFEINT_DEFINE_SERVICE(interfaceName)									\
+ (interfaceName*)sharedInstance												\
{																				\
	return interfaceName##Instance;												\
}																				\
																				\
+ (void)initializeService														\
{																				\
	if (interfaceName##Instance == nil)											\
	{																			\
		interfaceName##Instance = [interfaceName new];							\
	}																			\
}																				\
																				\
+ (void)shutdownService															\
{																				\
	int retainCount = [interfaceName##Instance retainCount];					\
	if (retainCount > 1)														\
	{																			\
		OFLog(@#interfaceName " has outstanding references during shtudown!");	\
	}																			\
																				\
	[interfaceName##Instance release];											\
	interfaceName##Instance = nil;												\
}
