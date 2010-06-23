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

@class OFService;

@interface OFInviteDefinition : OFResource
{
	@package
	// These get copied into an invite (modulo string interpolation) at the time the invite is sent.
	NSString* clientApplicationName;		// <client_application_name>
	NSString* clientApplicationID;			// <client_application_id>
	NSString* inviteIdentifier;				// <invite_identifier>
	NSString* senderParameter;				// <sender_parameter>
	NSString* receiverParameter;			// <receiver_parameter>
	NSString* inviteIconURL;				// <invite_icon_url>
	NSString* developerMessage;				// <developer_message>
	NSString* receiverNotification;			// <receiver_notification>
	NSString* senderNotification;			// <sender_notification>

	// These don't get copied, because they're only used at send time.
	NSString* senderIncentiveText;			// <sender_incentive_text>
	NSString* suggestedSenderMessage;		// <suggested_sender_message>
}

+ (OFResourceDataMap*)getDataMap;
+ (OFService*)getService;
+ (NSString*)getResourceName;

@property (nonatomic, retain) NSString* clientApplicationName;
@property (nonatomic, retain) NSString* clientApplicationID;
@property (nonatomic, retain) NSString* inviteIdentifier;
@property (nonatomic, retain) NSString* senderParameter;
@property (nonatomic, retain) NSString* receiverParameter;
@property (nonatomic, retain) NSString* inviteIconURL;
@property (nonatomic, retain) NSString* developerMessage;
@property (nonatomic, retain) NSString* receiverNotification;
@property (nonatomic, retain) NSString* senderNotification;
@property (nonatomic, retain) NSString* senderIncentiveText;
@property (nonatomic, retain) NSString* suggestedSenderMessage;

@end
