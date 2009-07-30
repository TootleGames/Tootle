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
#import "OFChatMessageService.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFService+Private.h"
#import "OFChatMessage.h"
#import "OFPoller.h"
#import "OpenFeint+Private.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFChatMessageService);

@implementation OFChatMessageService

OPENFEINT_DEFINE_SERVICE(OFChatMessageService);

- (void)populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFChatMessage getResourceName], [OFChatMessage class]);
}

- (void)registerPolledResources:(OFPoller*)poller
{
	[poller registerResourceClass:[OFChatMessage class]];
}

- (void) _onChatMessagesDiscovered:(NSNotification*)notification
{
	// citron note: Do Nothing. The table controller handles this on its own now
}

+ (void) getIndexOnSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	// citron note: The table controller will automagically get updates from the poller for the resource.
	//				This is redundent.
	onSuccess.invoke(nil);
}

+ (void) clearCacheAndPollNow
{
	[OpenFeint clearPollingCacheForClassType:[OFChatMessage class]];
	[OpenFeint forceImmediatePoll];
}

@end
