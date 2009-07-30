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

#include "OFDelegate.h"
#include "OFDelegateChained.h"

static const SEL gShouldInvokeCallback = @selector(shouldReceiveAsyncCallbacks);

@interface OFDeferredThreadedCallback : NSObject
{
	OFRetainedPtr<NSObject<OFCallbackable> > mTarget;
	OFRetainedPtr<NSObject> mParamOne;
	OFRetainedPtr<NSObject> mParamTwo;	
	SEL mSelector;	
	float mTimeDelay;
}

+ (id)deferredThreadedCallbackWith:(NSObject<OFCallbackable>*)target selector:(SEL)selector paramOne:(NSObject*)paramOne paramTwo:(NSObject*)paramTwo onThread:(NSThread*)onThread withDelay:(NSTimeInterval)delay;
- (id)initThreadedCallbackWith:(NSObject<OFCallbackable>*)target selector:(SEL)selector paramOne:(NSObject*)paramOne paramTwo:(NSObject*)paramTwo onThread:(NSThread*)onThread withDelay:(NSTimeInterval)delay;

- (void)_invokeCallback;
- (void)_invokeCallbackWithDelay;

@end

@implementation OFDeferredThreadedCallback

+ (id)deferredThreadedCallbackWith:(NSObject<OFCallbackable>*)target selector:(SEL)selector paramOne:(NSObject*)paramOne paramTwo:(NSObject*)paramTwo onThread:(NSThread*)onThread withDelay:(NSTimeInterval)delay
{
	return [[[OFDeferredThreadedCallback alloc] initThreadedCallbackWith:target selector:selector paramOne:paramOne paramTwo:paramTwo onThread:onThread withDelay:delay] autorelease];
}

- (id)initThreadedCallbackWith:(NSObject<OFCallbackable>*)target selector:(SEL)selector paramOne:(NSObject*)paramOne paramTwo:(NSObject*)paramTwo onThread:(NSThread*)onThread withDelay:(NSTimeInterval)delay
{
	self = [super init];
	if(self)
	{
		mTarget = target;
		mParamOne = paramOne;
		mParamTwo = paramTwo;
		mSelector = selector;
		mTimeDelay = delay;
		
		if([NSThread currentThread] == onThread)
		{
			[self _invokeCallbackWithDelay];
		}
		else
		{			
			[self performSelector:@selector(_invokeCallbackWithDelay) onThread:onThread withObject:nil waitUntilDone:NO];
		}
	}	
	return self;
}

- (void)_invokeCallbackWithDelay
{	
	if(mTimeDelay)
	{
		[self performSelector:@selector(_invokeCallback) withObject:nil afterDelay:mTimeDelay];
	}
	else
	{
		[self _invokeCallback];
	}
}

- (void)_invokeCallback
{
	NSObject< OFCallbackable >* target = mTarget.get();
	if([target canReceiveCallbacksNow])
	{
		[target performSelector:mSelector withObject:mParamOne.get() withObject:mParamTwo.get()];
	}
}

@end

OFDelegate::OFDelegate()
: mTarget(nil)
, mTargetThread(nil)
, mUserParam(nil)
{
}

OFDelegate::~OFDelegate()
{
	OFSafeRelease(mTarget);
	OFSafeRelease(mTargetThread);
	OFSafeRelease(mUserParam);
}

OFDelegate::OFDelegate(OFDelegate const& otherDelegate)
{
	mSelector = otherDelegate.mSelector;
	mTarget = [otherDelegate.mTarget retain];
	mTargetThread = [otherDelegate.mTargetThread retain];
	mUserParam = [otherDelegate.mUserParam retain];
}

OFDelegate& OFDelegate::operator=(const OFDelegate& otherDelegate)
{
	OFSafeRelease(mTarget);
	OFSafeRelease(mTargetThread);
	OFSafeRelease(mUserParam);

	mSelector = otherDelegate.mSelector;
	mTarget = [otherDelegate.mTarget retain];
	mTargetThread = [otherDelegate.mTargetThread retain];
	mUserParam = [otherDelegate.mUserParam retain];

	return *this;
}

OFDelegate::OFDelegate(NSObject<OFCallbackable>* target, SEL selector, const OFDelegate& manuallyChainedCall)
: mSelector(selector)
{
	mTarget = [target retain];
	mTargetThread = [[NSThread mainThread] retain];
	mUserParam = [[OFDelegateChained delegateWith:manuallyChainedCall] retain];

	OFAssert([mTarget conformsToProtocol:@protocol(OFCallbackable)], "");
}
	
OFDelegate::OFDelegate(NSObject<OFCallbackable>* target, SEL selector)
: mSelector(selector)
, mUserParam(nil)
{
	mTarget = [target retain];
	mTargetThread = [[NSThread mainThread] retain];

	OFAssert([mTarget conformsToProtocol:@protocol(OFCallbackable)], "");
}

OFDelegate::OFDelegate(NSObject<OFCallbackable>* target, SEL selector, NSThread* targetThread)
: mSelector(selector)
, mUserParam(nil)
{
	mTarget = [target retain];
	mTargetThread = [targetThread retain];

	OFAssert([mTarget conformsToProtocol:@protocol(OFCallbackable)], "");
}

OFDelegate::OFDelegate(NSObject<OFCallbackable>* target, SEL selector, NSObject* userParam)
: mSelector(selector)
{
	mTarget = [target retain];
	mTargetThread = [[NSThread mainThread] retain];
	mUserParam = [userParam retain];

	OFAssert([mTarget conformsToProtocol:@protocol(OFCallbackable)], "");
}

OFDelegate::OFDelegate(NSObject<OFCallbackable>* target, SEL selector, NSThread* targetThread, NSObject* userParam)
: mSelector(selector)
{
	mTarget = [target retain];
	mTargetThread = [targetThread retain];
	mUserParam = [userParam retain];
	
	OFAssert([mTarget conformsToProtocol:@protocol(OFCallbackable)], "");
}
		
void OFDelegate::invoke(NSObject* parameter, NSTimeInterval afterDelay) const
{
	if (!mTarget || !mTargetThread)
		return;
	
	[OFDeferredThreadedCallback 
		deferredThreadedCallbackWith:mTarget
									selector:mSelector 
									paramOne:parameter
									paramTwo:mUserParam
									onThread:mTargetThread
									withDelay:afterDelay];
}
	
void OFDelegate::invoke(NSObject* parameter) const
{
	if (!mTarget || !mTargetThread)
		return;
	
	[OFDeferredThreadedCallback 
		deferredThreadedCallbackWith:mTarget
									selector:mSelector 
									paramOne:parameter
									paramTwo:mUserParam
									onThread:mTargetThread
									withDelay:0.0f];
}

bool OFDelegate::isValid() const
{
	return mTarget != nil && mTargetThread != nil;
}