/*
 *  IPodVirtualKeyboard.mm
 *  TootleInput
 *
 *  Created by Duane Bradbury on 21/05/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#import "IPodVirtualKeyboard.h"

#include <TootleCore/IPod/IPodApp.h>



// the amount of vertical shift upwards keep the text field in view as the keyboard appears
#define kOFFSET_FOR_KEYBOARD					150.0

#define kTextFieldWidth							100.0	// initial width, but the table cell will dictact the actual width
#define kTextFieldHeight						20.0	// initial width, but the table cell will dictact the actual width

// the duration of the animation for the view shift
#define kVerticalOffsetAnimationDuration		0.30

#define kUITextField_Section					0
#define kUITextField_Rounded_Custom_Section		1
#define kUITextField_Secure_Section				2

// Private interface for TextFieldController - internal only methods.
@interface TextFieldController (Private)
- (void)setViewMovedUp:(BOOL)movedUp;
@end

@implementation TextFieldController

- (id)init
{
	self = [super init];
	if (self)
	{
		// this title will appear in the navigation bar
		self.title = NSLocalizedString(@"TextFieldTitle", @"");
	}
	
	return self;
}

- (void)dealloc
{
	[m_textField release];
	[super dealloc];
}

#pragma mark
#pragma mark UITextField
#pragma mark
- (void)createTextField
{
	CGRect Position = CGRectMake(0, 0, 320, 32);
	m_textField  = [[UITextField alloc] initWithFrame:Position];
	[m_textField setHidden:NO];
	[m_textField setBorderStyle:UITextBorderStyleRoundedRect];
	[m_textField setAutocorrectionType:UITextAutocorrectionTypeNo];
	[m_textField setAutocapitalizationType:UITextAutocapitalizationTypeWords];
	[m_textField setReturnKeyType:UIReturnKeySend];
	[m_textField setDelegate:self];
	
	
	// DB - The plan was to add the controller as a subview of the window and then the text as a subview of the controller.
	// Alas for some reaosn that doesn't quite work so now the plan is to simply add the text as a subview of the window.
	//[self.view addSubview:m_textField];
	[TLCore::Platform::g_pIPodApp.window addSubview:m_textField];

	[m_textField becomeFirstResponder];
	
	/*
	CGRect frame = CGRectMake(0.0, 0.0, kTextFieldWidth, kTextFieldHeight);
	UITextField *returnTextField = [[UITextField alloc] initWithFrame:frame];
	
	returnTextField.borderStyle = UITextBorderStyleBezel;
    returnTextField.textColor = [UIColor blackColor];
	returnTextField.font = [UIFont systemFontOfSize:17.0];
    returnTextField.placeholder = @"<enter text>";
    returnTextField.backgroundColor = [UIColor whiteColor];
	returnTextField.autocorrectionType = UITextAutocorrectionTypeNo;	// no auto correction support
	
	returnTextField.keyboardType = UIKeyboardTypeDefault;	// use the default type input method (entire keyboard)
	returnTextField.returnKeyType = UIReturnKeyDone;
	
	returnTextField.clearButtonMode = UITextFieldViewModeWhileEditing;	// has a clear 'x' button to the right
	
	return returnTextField;
	 */
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	// remove keyboard
	[m_textField resignFirstResponder];
	
	// do something with [textField text]...
	
	// Remove the text from the game window
	[m_textField removeFromSuperview];
	
	return YES;
}



- (void)loadView
{	
	[self createTextField];	
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	//TODO: Intercept key string here and send out as a messgae via the input system
	
	return TRUE;
}

@end

//#define ENABLE_VIRTUAL_KEYBOARD

using namespace TLInput;

Bool Platform::IPod::CreateVirtualKeyboard()
{
#ifdef ENABLE_VIRTUAL_KEYBOARD	
	g_TextFieldViewController = [[TextFieldController alloc] init];
	
	//[g_TextFieldViewController loadView];
	
	// add this a subview of the main window
	[TLCore::Platform::g_pIPodApp.window addSubview: g_TextFieldViewController.view];


	/*
	CGRect Position = CGRectMake(0, 400, 320, 32);
	UITextField* inputTextField  = [[UITextField alloc] initWithFrame:Position];
	[inputTextField setHidden:NO];
	[inputTextField setBorderStyle:UITextBorderStyleRoundedRect];
	[inputTextField setAutocorrectionType:UITextAutocorrectionTypeNo];
	[inputTextField setAutocapitalizationType:UITextAutocapitalizationTypeWords];
	[inputTextField setReturnKeyType:UIReturnKeySend];
	//[inputTextField setDelegate:self];
	[TLCore::Platform::g_pIPodApp.window addSubview:inputTextField];
	[inputTextField becomeFirstResponder];
	 */
	
	return TRUE;
#else
	return FALSE;
#endif
}

Bool Platform::IPod::DestroyVirtualKeyboard()
{
#ifdef ENABLE_VIRTUAL_KEYBOARD	
	if(g_TextFieldViewController != NULL)
	{
		//[g_TextFieldViewController removeFromSuperview];

		[g_TextFieldViewController release];
		g_TextFieldViewController = NULL;
	}
	
	return TRUE;
#else
	return FALSE;
#endif
}
