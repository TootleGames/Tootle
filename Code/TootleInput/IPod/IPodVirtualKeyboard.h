/*
 *  IPodVirtualKeyboard.h
 *  TootleInput
 *
 *  Created by Duane Bradbury on 21/05/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#pragma once

#include <TootleCore/TLTypes.h>

#import <UIKit/UIKit.h>

@interface TextFieldController : UIViewController <UITextFieldDelegate>

{
	UITextField		*m_textField;	
}

@end


namespace TLInput
{
	namespace Platform
	{	
		namespace IPod
		{
			Bool			CreateVirtualKeyboard();
			Bool			DestroyVirtualKeyboard();
			
			static TextFieldController *g_TextFieldViewController = NULL;
		}
	}
}
