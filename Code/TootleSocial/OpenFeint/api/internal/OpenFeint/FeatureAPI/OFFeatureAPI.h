//  Copyright 2009-2010 Aurora Feint, Inc.
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//  	http://www.apache.org/licenses/LICENSE-2.0
//  	
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#pragma once

#import "OFCallbackable.h"

@interface OFFeatureAPI : NSObject<OFCallbackable>
{
	NSMutableArray* outstandingRequests;
}

+ (void)createSharedInstanceWithSettings:(NSDictionary*)settings;
+ (void)createSharedInstance;
+ (void)destroySharedInstance;

@end

#define DEFINE_FEATURE_API_SHARED_INSTANCE(featureAPIName)						\
static featureAPIName* _sharedInstance = nil;

#define IMPLEMENT_FEATURE_API_BOILERPLATE(featureAPIName)						\
+ (void)createSharedInstance													\
{																				\
	if (_sharedInstance == nil)													\
	{																			\
		_sharedInstance = [featureAPIName new];									\
	}																			\
}																				\
																				\
+ (void)destroySharedInstance													\
{																				\
	OFSafeRelease(_sharedInstance);												\
}																				\
																				\
+ (featureAPIName*)sharedInstance												\
{																				\
	NSAssert(_sharedInstance != nil, @"Feature API not initialized!");			\
	return _sharedInstance;														\
}
