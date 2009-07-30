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

#include "OFCallbackable.h"
#include "OFRetainedPtr.h"

/// @note Any interfaces that wish to receive callbacks from an OFDelegate must implement the OFCallbackable protocol.
///		  If you forget, the error will be caught with a runtime assertion.

class OFDelegate
{
public:
	OFDelegate();
	~OFDelegate();

	OFDelegate(OFDelegate const& otherDelegate);
	OFDelegate& operator=(const OFDelegate& otherDelegate);

	OFDelegate(NSObject<OFCallbackable>* target, SEL selector);	
	OFDelegate(NSObject<OFCallbackable>* target, SEL selector, NSObject* userParam);

	/// @warning	when using manual chaining, the invoked selector has a second paramter of type OFDelegateChained*.
	///				you are required to explicitly invoke the chained delegate when ready!
	OFDelegate(NSObject<OFCallbackable>* target, SEL selector, const OFDelegate& manuallyChainedCall);	

	OFDelegate(NSObject<OFCallbackable>* target, SEL selector, NSThread* targetThread);
	OFDelegate(NSObject<OFCallbackable>* target, SEL selector, NSThread* targetThread, NSObject* userParam);
	
	void invoke(NSObject* parameter = 0) const;
	void invoke(NSObject* parameter, NSTimeInterval afterDelay) const;

	bool isValid() const;

private:
	NSObject<OFCallbackable>* mTarget;
	NSObject* mUserParam;
	NSThread* mTargetThread;
	SEL mSelector;
};
