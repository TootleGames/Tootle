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

#import <UIKit/UIKit.h>
#import "OFActionRequest.h"

@interface OFExtendedCredentialController : UIViewController {
	BOOL mRequireTwitterCredentials;
	BOOL mRequireFacebookCredentials;
	BOOL mTwitterCredentialsReceived;
	BOOL mFacebookCredentialsReceived;
	UIButton* mTwitterCredentialsButton;
	UIImageView* mTwitterCredentialsStatusImage;
	UIButton* mFacebookCredentialsButton;
	UIImageView* mFacebookCredentialsStatusImage;
	OFActionRequest* mRequest;
}


@property(assign) BOOL requireTwitterCredentials;
@property(assign) BOOL requireFacebookCredentials; 
@property(assign) BOOL twitterCredentialsReceived; 
@property(assign) BOOL facebookCredentialsReceived; 
@property(nonatomic, retain) IBOutlet UIButton* twitterCredentialsButton;
@property(nonatomic, retain) IBOutlet UIImageView* twitterCredentialsStatusImage;
@property(nonatomic, retain) IBOutlet UIButton* facebookCredentialsButton;
@property(nonatomic, retain) IBOutlet UIImageView* facebookCredentialsStatusImage;
@property(nonatomic, retain) OFActionRequest* request;

- (IBAction) twitterCredentialsButtonClicked:(UIButton*)sender;
- (IBAction) facebookCredentialsButtonClicked:(UIButton*)sender;
- (IBAction) resubmitRequest:(UIButton*)sender;
- (void) dismiss;

@end
