/*
 *  TFacebookSession.h
 *  TootleSocial
 *
 *  Created by Duane Bradbury on 05/06/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#import "FBConnect/FBConnect.h"

@class FBSession;

@interface SessionViewController : UIViewController
<FBDialogDelegate, FBSessionDelegate, FBRequestDelegate> {
	IBOutlet UILabel* _label;
	IBOutlet UIButton* _permissionButton;
	IBOutlet UIButton* _feedButton;
	IBOutlet FBLoginButton* _loginButton;
	FBSession* _session;
}

@property(nonatomic,readonly) UILabel* label;

- (void)askPermission:(id)target;
- (void)publishFeed:(id)target;

@end
