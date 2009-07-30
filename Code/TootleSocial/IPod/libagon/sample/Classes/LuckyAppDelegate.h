//  LuckyAppDelegate.h

//  Copyright Aptocore ApS 2009. All rights reserved.

#import <UIKit/UIKit.h>

@class LuckyViewController;

@interface LuckyAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    LuckyViewController *viewController;
}

@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) LuckyViewController *viewController;

@end

