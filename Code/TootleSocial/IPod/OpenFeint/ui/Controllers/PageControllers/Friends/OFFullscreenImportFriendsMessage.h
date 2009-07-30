////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

@interface OFFullScreenImportFriendsMessage : UIViewController
{
	UIViewController* owner;
	UILabel* messageLabel;
}

@property (nonatomic, assign) IBOutlet UIViewController* owner;
@property (nonatomic, assign) IBOutlet UILabel* messageLabel;

- (void)setMissingCredentials:(NSArray*)missingUsersCredentials withNotice:(NSString*)notice;

- (IBAction)onImportFriendsPressed;

@end