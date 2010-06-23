////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
///  Copyright 2009 Aurora Feint, Inc.
/// 
///  Licensed under the Apache License, Version 2.0 (the "License");
///  you may not use this file except in compliance with the License.
///  You may obtain a copy of the License at
///  
///  	http://www.apache.org/licenses/LICENSE-2.0
///  	
///  Unless required by applicable law or agreed to in writing, software
///  distributed under the License is distributed on an "AS IS" BASIS,
///  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
///  See the License for the specific language governing permissions and
///  limitations under the License.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFResource.h"
#import "OFSqlQuery.h"
#import "math.h"

@class OFService;
@class OFUser;
@class OFS3UploadParameters;

@interface OFHighScore : OFResource
{
@package
	OFUser* user;
	int64_t score;
	NSInteger rank;
	NSUInteger leaderboardId;
	NSString* displayText;
	NSString* customData;
	NSData* blob;
	NSString* blobUrl;
	NSString* toHighRankText;
	OFS3UploadParameters* blobUploadParameters;
	double latitude;
	double longitude;
	double distance;
}

- (id)initWithLocalSQL:(OFSqlQuery*)queryRow forUser:(OFUser*)hsUser rank:(NSUInteger)scoreRank;
- (BOOL)hasBlob;
- (void)_setBlob:(NSData*)_blob;
+ (OFResourceDataMap*)getDataMap;
+ (OFService*)getService;
+ (NSString*)getResourceName;
+ (NSString*)getResourceDiscoveredNotification;

@property (nonatomic, readonly, retain)	OFUser* user;
@property (nonatomic, readonly)			int64_t score;
@property (nonatomic, readonly)			NSInteger rank;
@property (nonatomic, readonly)			NSUInteger leaderboardId;
@property (nonatomic, readonly, retain)	NSString* displayText;
@property (nonatomic, readonly, retain)	NSString* toHighRankText;
@property (nonatomic, readonly, retain)	NSString* customData;
@property (nonatomic, readonly, retain)	NSData* blob;
@property (nonatomic, readonly, retain)	NSString* blobUrl;
@property (nonatomic, readonly, retain)	OFS3UploadParameters* blobUploadParameters;
@property (nonatomic, readonly)			double latitude;
@property (nonatomic, readonly)			double longitude;
@property (nonatomic, readonly)			double distance;

@end

