/*
 *  IPodVibration.h
 *  TootleInput
 *
 *	Vibration handling on the iPhone.  No conrol over intensity or duration. :(
 *
 *	NOTE: This is also iPhone only (does nothing on ipod Touch) and dependent on the user settings so if vibrate is switched off then no vibrate anyway.
 *
 *  Created by Duane Bradbury on 02/06/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

namespace TLInput
{
	namespace Platform
	{
		namespace IPod
		{
			void VibrateDevice();
		}
	}
}