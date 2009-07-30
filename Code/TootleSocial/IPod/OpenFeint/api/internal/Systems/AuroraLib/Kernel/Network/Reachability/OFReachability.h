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

#import <SystemConfiguration/SystemConfiguration.h>

#import <list>
#import "OFBase.h"
#import "OFIReachabilityObserver.h"

class OFReachability
{
OFDeclareSingleton(OFReachability);
public:
	~OFReachability();

	NetworkReachability gameServerReachability();
	bool isGameServerReachable();
	
	// Warning: You cannot invoke these from an OFIReachabilityObserver callback
	void addObserver(OFIReachabilityObserver* observer);
	void removeObserver(OFIReachabilityObserver* observer);
	
private:
	static void setIsTargetReachableAccordingToFlags(OFReachability* me, SCNetworkReachabilityFlags flags);
	static void networkReachabilityCallBack(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void *info);
	void updateNetworkReachabilityNow();
	void notifyObservers(NetworkReachability oldReachbilityFlags);
	
	NetworkReachability mGameServerReachability;
	SCNetworkReachabilityRef mReachabilityRef;
	
	typedef std::list<OFIReachabilityObserver*> ObserverSeries;
	ObserverSeries mObservers;
};