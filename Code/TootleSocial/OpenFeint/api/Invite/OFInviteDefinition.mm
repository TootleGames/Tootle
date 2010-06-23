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

#import "OFInviteDefinition.h"
#import "OFInviteService.h"
#import "OFResourceDataMap.h"

@implementation OFInviteDefinition

@synthesize clientApplicationName;
@synthesize clientApplicationID;
@synthesize inviteIdentifier;
@synthesize senderParameter;
@synthesize receiverParameter;
@synthesize inviteIconURL;
@synthesize developerMessage;
@synthesize receiverNotification;
@synthesize senderNotification;
@synthesize senderIncentiveText;
@synthesize suggestedSenderMessage;

+ (OFService*)getService;
{
	return [OFInviteService sharedInstance];
}

+ (OFResourceDataMap*)getDataMap
{
	static OFPointer<OFResourceDataMap> dataMap;
	
	if(dataMap.get() == NULL)
	{
		dataMap = new OFResourceDataMap;
		dataMap->addField(@"client_application_name", @selector(setClientApplicationName:), @selector(clientApplicationName));
		dataMap->addField(@"client_application_id", @selector(setClientApplicationID:), @selector(clientApplicationID));
		dataMap->addField(@"invite_identifier", @selector(setInviteIdentifier:), @selector(inviteIdentifier));
		dataMap->addField(@"sender_parameter", @selector(setSenderParameter:), @selector(senderParameter));
		dataMap->addField(@"receiver_parameter", @selector(setReceiverParameter:), @selector(receiverParameter));
		dataMap->addField(@"invite_icon_url", @selector(setInviteIconURL:), @selector(inviteIconURL));
		dataMap->addField(@"developer_message", @selector(setDeveloperMessage:), @selector(developerMessage));
		dataMap->addField(@"receiver_notification", @selector(setReceiverNotification:), @selector(receiverNotification));
		dataMap->addField(@"sender_notification", @selector(setSenderNotification:), @selector(senderNotification));
		dataMap->addField(@"sender_incentive_text", @selector(setSenderIncentiveText:), @selector(senderIncentiveText));
		dataMap->addField(@"suggested_sender_message", @selector(setSuggestedSenderMessage:), @selector(suggestedSenderMessage));
	}
	
	return dataMap.get();
}

+ (NSString*)getResourceName
{
	return @"invite_definition";
}

- (void) dealloc
{
	OFSafeRelease(clientApplicationName);
	OFSafeRelease(clientApplicationID);
	OFSafeRelease(inviteIdentifier);
	OFSafeRelease(senderParameter);
	OFSafeRelease(receiverParameter);
	OFSafeRelease(inviteIconURL);
	OFSafeRelease(developerMessage);
	OFSafeRelease(receiverNotification);
	OFSafeRelease(senderNotification);
	OFSafeRelease(senderIncentiveText);
	OFSafeRelease(suggestedSenderMessage);
	[super dealloc];
}

@end
