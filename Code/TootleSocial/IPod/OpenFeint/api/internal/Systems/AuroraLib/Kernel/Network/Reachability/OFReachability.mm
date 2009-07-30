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

#include "OFReachability.h"
#include "OFSettings.h"
#include "OFLog.h"

OFDefineSingleton(OFReachability);

bool isReachableWithoutRequiringConnection(SCNetworkReachabilityFlags flags)
{
	 // kSCNetworkReachabilityFlagsReachable indicates that the specified nodename or address can
	// be reached using the current network configuration.
	BOOL isReachable = flags & kSCNetworkReachabilityFlagsReachable;
	
	// This flag indicates that the specified nodename or address can
	// be reached using the current network configuration, but a
	// connection must first be established.
	//
	// If the flag is false, we don't have a connection. But because CFNetwork
	// automatically attempts to bring up a WWAN connection, if the WWAN reachability
	// flag is present, a connection is not required.
	BOOL noConnectionRequired = !(flags & kSCNetworkReachabilityFlagsConnectionRequired);
	if ((flags & kSCNetworkReachabilityFlagsIsWWAN)) {
		noConnectionRequired = YES;
	}
	
	return (isReachable && noConnectionRequired) ? YES : NO;
}

NetworkReachability getReachabilityStatus(SCNetworkReachabilityFlags reachabilityFlags)
{
	bool reachable = isReachableWithoutRequiringConnection(reachabilityFlags);
	
	if(!reachable) 
	{
		return NotReachable;
	}

	if(reachabilityFlags & kSCNetworkReachabilityFlagsIsWWAN) 
	{
		return ReachableViaCarrierDataNetwork;
	}
	
	return ReachableViaWiFiNetwork;	
}


void OFReachability::setIsTargetReachableAccordingToFlags(OFReachability* me, SCNetworkReachabilityFlags flags)
{
	NetworkReachability oldReachability = me->mGameServerReachability;
	me->mGameServerReachability = getReachabilityStatus(flags);	
	me->notifyObservers(oldReachability);
	
	OFLogInfo(OFReachability, "Network status changed: %s", 
		me->mGameServerReachability == NotReachable ? "disconnected" : 
		(me->mGameServerReachability == ReachableViaCarrierDataNetwork ? "connected carrier" : "connected wifi"));
}

void OFReachability::notifyObservers(NetworkReachability oldReachbilityFlags)
{
	ObserverSeries::iterator it = mObservers.begin();
	for(; it != mObservers.end(); ++it)
	{
		(*it)->onGameServerReachabilityChanges(oldReachbilityFlags, mGameServerReachability);
	}
}

void OFReachability::networkReachabilityCallBack(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void *info)
{
	OFReachability::setIsTargetReachableAccordingToFlags(OFReachability::Instance(), flags);
}
	
OFReachability::OFReachability() 
: mGameServerReachability(NotReachable)
{
	OFAssert([NSThread isMainThread], "Must be initialized from the main thread");
	
	OFRetainedPtr<NSURL> hostUrl = [NSURL URLWithString:OFSettings::Instance()->getServerUrl()];
	mReachabilityRef = SCNetworkReachabilityCreateWithName(NULL, [[hostUrl.get() host] UTF8String]);
	OFAssert(mReachabilityRef, "Failed creating reachability for address");
	
	SCNetworkReachabilitySetCallback(
		mReachabilityRef, 
		networkReachabilityCallBack,
		NULL
	);	

	SCNetworkReachabilityScheduleWithRunLoop(
		mReachabilityRef, 
		CFRunLoopGetMain(), 
		kCFRunLoopDefaultMode
	);

	updateNetworkReachabilityNow();
}

void OFReachability::updateNetworkReachabilityNow()
{
	SCNetworkReachabilityFlags flags = NULL;
	bool gotFlags = SCNetworkReachabilityGetFlags(mReachabilityRef, &flags);
	if(!gotFlags)
	{
		OFLogError(OFReachability, "Unable to update network status synchronously.");
	}
	
	setIsTargetReachableAccordingToFlags(this, flags);
}

OFReachability::~OFReachability()
{
	SCNetworkReachabilitySetCallback(
		mReachabilityRef, 
		NULL,
		NULL
	);
	
	SCNetworkReachabilityUnscheduleFromRunLoop(
		mReachabilityRef, 
		CFRunLoopGetMain(),
		kCFRunLoopDefaultMode
	);
	
	CFRelease(mReachabilityRef);
}

NetworkReachability OFReachability::gameServerReachability()
{
	return mGameServerReachability;
}

bool OFReachability::isGameServerReachable()
{
	return mGameServerReachability != NotReachable;
}

void OFReachability::addObserver(OFIReachabilityObserver* observer)
{
	mObservers.push_back(observer);
}

void OFReachability::removeObserver(OFIReachabilityObserver* observer)
{
	ObserverSeries::iterator sit = std::find(mObservers.begin(), mObservers.end(), observer);
	if(sit != mObservers.end())
	{	
		mObservers.erase(sit);
	}
}