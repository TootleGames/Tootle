////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
///  Copyright 2010 Aurora Feint, Inc.
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

#import <Foundation/NSObject.h>


enum OFCloudStorageStatus_Code {
	CSC_Unspecified			= 0,	// Status is not specified; This code is never expected in a reply.
	//
	// The following codes are loosely based on HTTP status codes [http://en.wikipedia.org/wiki/List_of_HTTP_status_codes].
	CSC_Ok					= 200,	// Status is normal; no error.
	CSC_NotAcceptable		= 406,	// Not acceptable; Request parameters may be may be malformed.
	CSC_NotFound			= 404,	// Specified blob does not exist on server.
	CSC_GatewayTimeout		= 504,	// Expected reply not received; network connection may have gone offline.
	CSC_InsufficientStorage	= 507,	// Server unable to store given blob due to server capacity contstraints.
	//
	// The following codes may be supported in the future.
	// CSC_BadRequest		= 400,	// The request is invalid.
	// CSC_Unauthorized		= 401,	//
	// CSC_RequestTimeout	= 408,	//
	
};


@interface OFCloudStorageStatus_Object : NSObject
{
}
- (OFCloudStorageStatus_Code) getStatusCode;
- (BOOL) isAnyError;
- (BOOL) isNetworkError;
@end


@interface OFCloudStorageStatus_Ok : OFCloudStorageStatus_Object
{
}
- (OFCloudStorageStatus_Code) getStatusCode;
@end


@interface OFCloudStorageStatus_NotAcceptable : OFCloudStorageStatus_Object
{
}
- (OFCloudStorageStatus_Code) getStatusCode;
@end


@interface OFCloudStorageStatus_NotFound : OFCloudStorageStatus_Object
{
}
- (OFCloudStorageStatus_Code) getStatusCode;
@end


@interface OFCloudStorageStatus_GatewayTimeout : OFCloudStorageStatus_Object
{
}
- (OFCloudStorageStatus_Code) getStatusCode;
@end


@interface OFCloudStorageStatus_InsufficientStorage : OFCloudStorageStatus_Object
{
}
- (OFCloudStorageStatus_Code) getStatusCode;
@end

