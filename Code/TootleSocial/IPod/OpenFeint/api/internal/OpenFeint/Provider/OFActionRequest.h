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

#import "OFCallbackable.h"
#import "OFActionRequestType.h"

@class MPOAuthAPIRequestLoader;

@interface OFActionRequest : NSObject<OFCallbackable>
{
	MPOAuthAPIRequestLoader* mLoader;
	OFActionRequestType mRequestType;
	NSString* mNoticeText;
	int mPreviousHttpStatusCode;
	bool mRequiresAuthentication;
}

@property (nonatomic, readonly) NSString* notice;
@property (nonatomic, readonly) bool failedNotAuthorized;
@property (nonatomic, readonly) bool requiresAuthentication;

+ (id)actionRequestWithLoader:(MPOAuthAPIRequestLoader*)loader withRequestType:(OFActionRequestType)requestType withNotice:(NSString*)notice requiringAuthentication:(bool)requiringAuthentication;

- (id)initWithLoader:(MPOAuthAPIRequestLoader*)loader withRequestType:(OFActionRequestType)requestType withNotice:(NSString*)notice requiringAuthentication:(bool)requiringAuthentication;
- (void)dispatch;
- (bool)canReceiveCallbacksNow;

@end

extern const int OpenFeintHttpStatusCodeForServerMaintanence;
