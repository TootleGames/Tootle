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
#import "OFActionRequest.h"
#import "OpenFeint+Private.h"
#import "OFProvider.h"
#import "MPOAuthAPIRequestLoader.h"
#import "OFDelegateChained.h"
#import "MPOAuthURLResponse.h"
#import "OFServerMaintenanceNoticeController.h"
#import "OFNotification.h"
#import "OFReachability.h"

// If this changes, the server configrations must be updated. 
const int OpenFeintHttpStatusCodeForServerMaintanence = 450;
const int OpenFeintHttpStatusCodeNotAuthorized = 401;
const int OpenFeintHttpStatusCodeUpdateRequired = 426;
const int OpenFeintHttpStatusCodePermissionsRequired = 430;

@implementation OFActionRequest

@synthesize notice = mNoticeText;
@synthesize requiresAuthentication = mRequiresAuthentication;
@dynamic failedNotAuthorized;

- (bool)failedNotAuthorized
{
	return mPreviousHttpStatusCode == OpenFeintHttpStatusCodeNotAuthorized;
}

+ (id)actionRequestWithLoader:(MPOAuthAPIRequestLoader*)loader withRequestType:(OFActionRequestType)requestType withNotice:(NSString*)notice requiringAuthentication:(bool)requiringAuthentication
{
	return [[[self alloc] initWithLoader:loader withRequestType:requestType withNotice:notice requiringAuthentication:requiringAuthentication] autorelease];
}

- (bool) _checkAndHandleHttpErrors:(MPOAuthAPIRequestLoader*)request
{
	bool errorsEnabled = [OpenFeint areErrorScreensAllowed];
	
	if([request.oauthResponse.urlResponse isKindOfClass:[NSHTTPURLResponse class]])
	{
		NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse*)request.oauthResponse.urlResponse;
		
		mPreviousHttpStatusCode = httpResponse.statusCode;		
		
		switch(mPreviousHttpStatusCode)
		{
			case OpenFeintHttpStatusCodeUpdateRequired:
			{
				[OpenFeint displayUpgradeRequiredErrorMessage:request.data];
				return errorsEnabled;
			}			
			case OpenFeintHttpStatusCodeForServerMaintanence:
			{
				[OpenFeint displayServerMaintenanceNotice:request.data]; 
				return errorsEnabled;
			}
			case OpenFeintHttpStatusCodeNotAuthorized:
			{
				if(self.requiresAuthentication)
				{
					[[OpenFeint provider] destroyLocalCredentials];
					[OpenFeint launchLoginFlowForRequest:self];
					// adill Note: -dispatch already released the loader but in this case
					// we want to offer the user another login opportunity, and should it be successful 
					// we'll re-dispatch the request. So we need to hold onto it until then.
					mLoader = [request retain];
					return true;
				}
				
				return false;
			}
			case OpenFeintHttpStatusCodePermissionsRequired:
			{
				[OpenFeint launchGetExtendedCredentialsFlowForRequest:self withData:request.data];
				mLoader = [request retain];
				return true;
			}
		}		
	}
	
	return false;
}

- (void) _onFailure:(MPOAuthAPIRequestLoader*)request nextCall:(OFDelegateChained*)nextCall
{
	if([self _checkAndHandleHttpErrors:request])
	{
		// Handled
	}
	else if(request.error)
	{
		NSError* error = request.error;						
		NSString* errorMessage = [NSString stringWithFormat:@"%@ (%d[%d])", [error localizedDescription], error.domain, error.code];			
		[OpenFeint displayErrorMessage:errorMessage];
	}

	[nextCall invokeWith:request];
}

- (id)initWithLoader:(MPOAuthAPIRequestLoader*)loader withRequestType:(OFActionRequestType)requestType withNotice:(NSString*)notice requiringAuthentication:(bool)requiringAuthentication
{
	if(self = [super init])
	{
		// adill Note: We retain the loader, then proceed to set it's failure delegate with an OFDelegate (that is retaining us)
		//		thus we have a circular reference. To remedy this situation we're releasing the loader in dispatch (which will
		//		deallocate the failure delegate which releases the reference to ourself)
		mLoader = [loader retain];
		[mLoader setOnFailure:OFDelegate(self, @selector(_onFailure:nextCall:), [mLoader getOnFailure])];
		mRequestType = requestType;
		mNoticeText = [notice retain];
		mRequiresAuthentication = requiringAuthentication;
	}
	
	return self;
}

- (void)setRequestDoesNotExpectAuthentication
{
	mRequiresAuthentication = false;
}

- (void)dealloc 
{
	OFSafeRelease(mLoader);
	OFSafeRelease(mNoticeText);
    [super dealloc];
}

- (void)dispatch
{
	OFAssert(mLoader != nil, "Cannot dispatch a request twice!");
		
	if (!OFReachability::Instance()->isGameServerReachable() ||
		!self.requiresAuthentication || 
		(self.requiresAuthentication && [[OpenFeint provider] isAuthenticated]))
	{
		if([OpenFeint isShowingFullScreen] == false)
		{
			NSAssert(OFActionRequestCount == 3, @"Number of action request types changed. Make sure this if statement is up to date");
			
			if(mRequestType == OFActionRequestBackground)
			{
				[[OFNotification sharedInstance] showBackgroundNoticeForLoader:mLoader withNotice:mNoticeText];
			}
			else if(mRequestType == OFActionRequestForeground)
			{
				[[OFNotification sharedInstance] showBackgroundNoticeForLoader:mLoader withNotice:mNoticeText];
			}
			else if(mRequestType == OFActionRequestSilent)
			{
				[mLoader loadSynchronously:NO];
			}
		}
		else
		{
			// citron note: for now, all dashboard screens are managing their own "loading" state. Should this change?
			[mLoader loadSynchronously:NO];
		}
		
		OFSafeRelease(mLoader);
	}
	else
	{
		[OpenFeint launchLoginFlowForRequest:self];
	}
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

@end
