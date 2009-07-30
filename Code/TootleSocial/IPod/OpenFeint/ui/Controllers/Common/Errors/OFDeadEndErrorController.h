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

@interface OFDeadEndErrorController : UIViewController
{
@private
	NSString* mMessage;
	UILabel* mMessageView;
}

+ (id)deadEndErrorWithMessage:(NSString*)errorMessage;
+ (id)mustBeOnlineErrorWithMessage:(NSString*)errorMessage;

@property (nonatomic, retain) NSString* message;
@property (nonatomic, retain) IBOutlet UILabel* messageView;

@end
