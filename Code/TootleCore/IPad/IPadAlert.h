/*
 *  IPadAlert.h
 *  TootleCore
 *
 *  Created by Duane Bradbury on 19/05/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include <TLTypes.h>
#import <UIKit/UIKit.h>

@interface AlertsViewController : UIViewController <UIAlertViewDelegate>
{

}

- (void)dialogueOKCancelAction:(NSString *)nsstr;
- (SyncBool)dialogueResult;

@end
