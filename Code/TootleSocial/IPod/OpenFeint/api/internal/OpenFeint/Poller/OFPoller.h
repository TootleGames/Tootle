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

#pragma once

#include "OFResourceNameMap.h"

@protocol OFProviderProtocol;

@interface OFPoller : NSObject<OFCallbackable>
{
@private
	NSObject<OFProviderProtocol>* mProvider;
	OFResourceNameMap mRegisteredResources;
	NSMutableDictionary* mChronologicalResourceTypes;
	NSTimer* mHeartbeat;
	bool mInPoll;
	bool mQueuedPoll;
}

- (id)initWithProvider:(NSObject<OFProviderProtocol>*)provider;

- (Class)getRegisteredResourceClassWithName:(NSString*)resourceName;
- (void)registerResourceClass:(Class)resourceClassType;
- (void)clearCacheForResourceClass:(Class)resourceClassType;

- (void)pollNow;

- (void)resetToDefaultPollingFrequency;
- (void)changePollingFrequency:(NSTimeInterval)pollingFrequency;
- (void)stopPolling;
- (NSTimeInterval)getPollingFrequency;

- (bool)canReceiveCallbacksNow;

@end


extern NSString* OFPollerNotificationKeyForResources;