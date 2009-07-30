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

#import "OFReachabilityObserver.h"
#import "OpenFeint+Private.h"
#import "OFReachability.h"

OFReachabilityObserver::OFReachabilityObserver(const OFDelegate& onStatusChanged)
: mOnStatusChanged(onStatusChanged)
{
	OFReachability::Instance()->addObserver(this);
}

OFReachabilityObserver::~OFReachabilityObserver()
{
	OFReachability::Instance()->removeObserver(this);
}

void OFReachabilityObserver::onGameServerReachabilityChanges(NetworkReachability previousStatus, NetworkReachability newStatus)
{
	mOnStatusChanged.invoke([NSNumber numberWithInt:newStatus]);
}