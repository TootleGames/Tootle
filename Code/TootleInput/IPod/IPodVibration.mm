/*
 *  IPodVibration.cpp
 *  TootleInput
 *
 *  Created by Duane Bradbury on 02/06/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "IPodVibration.h"

#import <AudioToolbox/AudioToolbox.h>

#import <UIKit/UIKit.h>

/*
 Vibrate the iPhone
 NOTE: This is iPhone only (does nothing on ipod Touch) and dependent on the user settings so if vibrate is switched off then no vibrate anyway.

*/
void TLInput::Platform::IPod::VibrateDevice()
{
	AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);

}



/*
 //There is no way to manipulate how long or how intense the vibration is on the iPhone.  
 
Source: https://devforums.apple.com/message/54195#54195

 [NSThread detachNewThreadSelector:@selector(vibrationCycle:) toTarget:self withObject:[NSNumber numberWithInt:3]];  
 
 
 - (void)vibrationCycle:(NSNumber *)iterations 
 {
	for(int i = 0; i < [iterations intValue]; i++) 
	{
		AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
		sleep(0.7);
	}
}

*/