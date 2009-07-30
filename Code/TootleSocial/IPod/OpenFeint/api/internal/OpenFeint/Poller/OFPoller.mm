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
#import "OFPoller.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFProviderProtocol.h"
#import "MPOAuthAPIRequestLoader.h"
#import "OFResource.h"
#import "OFXmlDocument.h"
#import "OFPollerResourceType.h"
#import <objc/runtime.h>
#import "OFHttpNestedQueryStringWriter.h"
#import "OFTimerHeartbeat.h"
#import "OpenFeint+Private.h"
#import "OpenFeint+Settings.h"
#import "OFPaginatedSeries.h"

NSString* OFPollerNotificationKeyForResources = @"OFPollerNotificationKeyResources";

@implementation OFPoller

- (id)initWithProvider:(NSObject<OFProviderProtocol>*)provider
{
	if(self = [super init])
	{
		mProvider = [provider retain];
		mChronologicalResourceTypes = [[NSMutableDictionary dictionary] retain];
	}
	return self;
}

- (void) dealloc
{
	[mHeartbeat invalidate];
	[mHeartbeat release];
	[mChronologicalResourceTypes release];
	[mProvider release];
	[super dealloc];
}

- (Class)getRegisteredResourceClassWithName:(NSString*)resourceName
{
	return mRegisteredResources.getTypeNamed(resourceName);
}

- (void)resetToDefaultPollingFrequency
{
	NSUInteger defaultPollingFrequency = [OpenFeint getPollingFrequencyDefault];
	[self changePollingFrequency:defaultPollingFrequency];
}

- (void)changePollingFrequency:(NSTimeInterval)pollingFrequency
{
	[self stopPolling];
	
	if(pollingFrequency == 0.0f)
	{
		return;
	}
	
	mHeartbeat = [[OFTimerHeartbeat scheduledTimerWithInterval:pollingFrequency target:self selector:@selector(pollNow)] retain];
	OFLog(@"Polling every %f seconds", pollingFrequency);
}

- (void)stopPolling
{
	[mHeartbeat invalidate];
	[mHeartbeat release];
	mHeartbeat = nil;
	OFLog(@"Stopped polling");
}

- (void)registerResourceClass:(Class)resourceClassType;
{	
	if(![resourceClassType isSubclassOfClass:[OFResource class]])
	{
		NSAssert1(0, @"'%s' must derive from OFResource to be work with the Polling system.", class_getName(resourceClassType));
		return;
	}
	
	NSString* name = [resourceClassType performSelector:@selector(getResourceName)];
	NSString* notificationName = [resourceClassType performSelector:@selector(getResourceDiscoveredNotification)];
		
	mRegisteredResources.addResource(name, resourceClassType);
	
	OFPollerResourceType* resourceType = [[[OFPollerResourceType alloc] initWithName:name andDiscoveryNotification:notificationName] autorelease];
	[mChronologicalResourceTypes setObject:resourceType forKey:resourceClassType];
}

- (void)clearCacheForResourceClass:(Class)resourceClassType
{
	OFPollerResourceType* resourceType = [mChronologicalResourceTypes objectForKey:resourceClassType];
	[resourceType markNewResourcesOld];
	[resourceType clearLastSeenId];
}

- (void)_onPollComplete
{
	mInPoll = false;
	if (mQueuedPoll)
	{
		mQueuedPoll = false;
		[self pollNow];
	}
}

- (void)_onSucceededDownloading:(MPOAuthAPIRequestLoader*)request
{
	OFPaginatedSeries* incomingResources = [OFResource resourcesFromXml:[OFXmlDocument xmlDocumentWithData:request.data] withMap:&mRegisteredResources];
	
	NSMutableSet* incomingTypes = [NSMutableSet set];
	for(OFResource* currentResource in incomingResources.objects)
	{
		OFPollerResourceType* resourceType = [mChronologicalResourceTypes objectForKey:[currentResource class]];
		[resourceType addResource:currentResource];
		[incomingTypes addObject:resourceType];
	}

	for(OFPollerResourceType* resourceType in incomingTypes)
	{
		NSDictionary* resourcesDictionary = [NSDictionary dictionaryWithObject:resourceType.newResources forKey:OFPollerNotificationKeyForResources];
		[[NSNotificationCenter defaultCenter] postNotificationName:resourceType.discoveryNotification object:nil userInfo:resourcesDictionary];
		[resourceType markNewResourcesOld];
	}
	[self _onPollComplete];
}

- (void)_onFailedDownloading
{
	[self _onPollComplete];
}

- (void)pollNow
{		
	if(![mProvider isAuthenticated])
	{
		return;
	}
	
	if (mInPoll)
	{
		mQueuedPoll = true;
		return;
	}
	
	mQueuedPoll = false;
	mInPoll = true;
	
	OFLog(@"Polling Now");
	
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	for(Class resourceClass in mChronologicalResourceTypes)
	{
		OFPollerResourceType* resourceType = [mChronologicalResourceTypes objectForKey:resourceClass];
		OFRetainedPtr<NSString> theId = [[NSNumber numberWithLongLong:resourceType.lastSeenId] stringValue];
		params->io([resourceType.idParameterName UTF8String], theId);
	}
		
	[mProvider performAction:@"/users/@me/activities.xml"
			  withParameters:params->getQueryParametersAsMPURLRequestParameters()
			  withHttpMethod:@"GET"
			     withSuccess:OFDelegate(self, @selector(_onSucceededDownloading:))
				 withFailure:OFDelegate(self, @selector(_onFailedDownloading))
				withRequestType:OFActionRequestSilent
				  withNotice:nil];
}

- (NSTimeInterval)getPollingFrequency
{
	return mHeartbeat.timeInterval;
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

@end
