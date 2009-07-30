////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface OFProfileNoFriendsHeaderView : UIView
{
	UILabel* messageLabel;
	id target;
	SEL importFriendsCallback;
}

@property (nonatomic, retain) IBOutlet UILabel* messageLabel;
@property (nonatomic, assign) IBOutlet id target;
@property (nonatomic, assign) IBOutlet SEL importFriendsCallback;

- (IBAction)onImportFriendsPressed;

@end
