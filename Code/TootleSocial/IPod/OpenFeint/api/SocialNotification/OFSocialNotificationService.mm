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

#import "OFDependencies.h"
#import "OFSocialNotificationService.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFService+Private.h"
#import "OpenFeint+UserOptions.h"
#import "OpenFeint+Private.h"
#import "OFSocialNotificationService+Private.h"
#import "OFImageUrl.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFSocialNotificationService);

@implementation OFSocialNotificationService

OPENFEINT_DEFINE_SERVICE(OFSocialNotificationService);

- (void)populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFImageUrl getResourceName], [OFImageUrl class]);
}

+ (void)getImageUrlForNotificationImageNamed:(NSString*)imageName onSuccess:(OFDelegate const&)onSuccess onFailure:(OFDelegate const&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	params->io("image_name", imageName);

	[[self sharedInstance] 
		getAction:[NSString stringWithFormat:@"client_applications/%@/notification_images/show.xml", [OpenFeint clientApplicationId]]
		withParameters:params
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestSilent
		withNotice:@"Getting Notification Image Url"];
}

+ (void)sendWithText:(NSString *)text
{
	[self sendWithSocialNotification:[[[OFSocialNotification alloc] initWithText:text] autorelease]];
}

+ (void)sendWithText:(NSString*)text imageNamed:(NSString*)imageName
{
	[self sendWithSocialNotification:[[[OFSocialNotification alloc] initWithText:text imageNamed:imageName] autorelease]];
}



@end