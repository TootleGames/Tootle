//
//  main.m

//  Copyright Aptocore ApS 2009. All rights reserved.

#import <UIKit/UIKit.h>

int main(int argc, char *argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, @"LuckyAppDelegate");
    [pool release];
    return retVal;
}
