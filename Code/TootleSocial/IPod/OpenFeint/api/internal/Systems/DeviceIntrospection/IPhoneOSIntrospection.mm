/*
 *  IPhoneOSIntrospection.cpp
 *  MyOpenFeintSample
 *
 *  Created by Jason Citron on 7/10/09.
 *  Copyright 2009 Aurora Feint Inc.. All rights reserved.
 *
 */

#import "OFDependencies.h"
#import "IPhoneOSIntrospection.h"

bool is2PointOhSystemVersion()
{
	NSArray* versionComponents = [[UIDevice currentDevice].systemVersion componentsSeparatedByString:@"."];
	NSString* majorVersionNumber = (NSString*)[versionComponents objectAtIndex:0];
	return [majorVersionNumber isEqualToString:@"2"];
}

bool is3PointOhSystemVersion()
{
	NSArray* versionComponents = [[UIDevice currentDevice].systemVersion componentsSeparatedByString:@"."];
	NSString* majorVersionNumber = (NSString*)[versionComponents objectAtIndex:0];
	return [majorVersionNumber isEqualToString:@"3"];
}