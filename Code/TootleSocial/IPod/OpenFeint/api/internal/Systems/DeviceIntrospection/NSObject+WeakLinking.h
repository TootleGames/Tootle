//
//  NSObject+WeakLinking.h
//  MyOpenFeintSample
//
//  Created by Jason Citron on 7/10/09.
//  Copyright 2009 Aurora Feint Inc.. All rights reserved.
//

#pragma once

#ifdef __IPHONE_3_0
	#define OF_OS_3_ENUM_ARG(enumName) (id)enumName
#else
	#define OF_OS_3_ENUM_ARG(enumName) (id)0
#endif

@interface NSObject (WeakLinking)

- (void)trySet:(NSString*)firstDottedProperty elseSet:(NSString*)elseDottedProperty with:(id)value;
- (id)  tryGet:(NSString*)firstDottedProperty elseGet:(NSString*)elseDottedProperty;

- (void)trySet:(NSString*)firstDottedProperty with:(id)firstValue elseSet:(NSString*)elseDottedProperty with:(id)secondValue;
		
@end
