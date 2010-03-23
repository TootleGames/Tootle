/*
 *  IPodVirtualKeyboard.mm
 *  TootleInput
 *
 *  Created by Duane Bradbury on 21/05/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#import "IPodVirtualKeyboard.h"

#include <TootleCore/TRef.h>
#include <TootleCore/IPod/IPodString.h>

#include "IPodInput.h"

//	gr: need the screen and gui to get hold of the window to attach the keyboard to
#include <TootleRender/TScreen.h>
#include <TootleRender/TScreenManager.h>
#include <TootleGui/IPod/IPodWindow.h>

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
	[m_textField setHidden:YES];
	[m_textField setBorderStyle:UITextBorderStyleRoundedRect];
	[m_textField setAutocorrectionType:UITextAutocorrectionTypeNo];
	[m_textField setAutocapitalizationType:UITextAutocapitalizationTypeWords];
	[m_textField setReturnKeyType:UIReturnKeySend];
	[m_textField setDelegate:self];
	//[m_textField setKeyboardType:UIKeyboardTypeNamePhonePad];	// use the name/phone type input method (alpha-numeric only keyboard)
	[m_textField setReturnKeyType:UIReturnKeyDone];	// set the return key string to 'Done'
	
	
	TString initialText = TLInput::g_pInputSystem->GetVirtualKeyboardText();
	
	NSString *initialString = TLString::ConvertToUnicharString(initialText);
	
	[m_textField setText:initialString];


	[initialString release];

	//	gr: get the window from the default screen
	//	gr: todo; get the window from this [subview]'s parent/view owner - which is a window.
	TLRender::TScreen* pScreen = TLRender::g_pScreenManager->GetDefaultScreen();
	TLGui::TWindow* pWindow = pScreen ? pScreen->GetWindow() : NULL;
	TLGui::Platform::Window* pPlatformWindow = static_cast<TLGui::Platform::Window*>( pWindow );
	UIWindow* pUiWindow = pPlatformWindow ? pPlatformWindow->m_pWindow : NULL;
	
	//	can't get a window to attach to, probably want this function to fail rather than fail silently
	if ( !pUiWindow )
	{
		TLDebug_Break("Failed to find window to attach text field to");
		return;
	}

	// DB - The plan was to add the controller as a subview of the window and then the text as a subview of the controller.
	// Alas for some reaosn that doesn't quite work so now the plan is to simply add the text as a subview of the window.
	//[self.view addSubview:m_textField];
	[pUiWindow addSubview:m_textField];

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
	
	//TODO: do something with [textField text]...??

	// Add RETURN key ref to say keyboard has finished editing
	TRef KeyRef = TRef("return");
	TLInput::Platform::IPod::ProcessVirtualKey(KeyRef);
	
	// Remove the text from the game window
	[m_textField removeFromSuperview];
	
	return YES;
}


/*
 // May not be needed.  Set a key in the info.plist of your project as follows:
 //
 //		<key>UIInterfaceOrientation</key>
 //		<string>UIInterfaceOrientationLandscapeRight</string>
 //
 // for fixed orientations.  Change the UIInterfaceOrientationLandscapeRight accordingly for your projects orientation.
 //
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}
*/



- (void)loadView
{	
	[self createTextField];	
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	// Intercept the key data here and add to the iPod specific keyboard array.
	// TODO: Needs wrapping up into an iPod specific class really
	// TODO: Need to check for special cases like delete, return, etc
	
	s32 Change = (range.length > 0 ? -1 : 1);
	
	// Check for backspace
	if(Change == -1)
	{
		// Delete a character
		TRef KeyRef = TRef("backspace");
		
		TLInput::Platform::IPod::ProcessVirtualKey(KeyRef);
	}
	else if(Change > 0)
	{
		const char *pString = [string UTF8String];
		TTempString TempStr(pString);

		if(!TLInput::g_pInputSystem->IsSupportedInputCharacter(TempStr[0]))
			return FALSE;
		
		TRef KeyRef = TRef(TempStr);

		//if(!TLInput::g_pInputSystem->GetSupportedInputCharacterRef(KeyRef, TempStr[0]))
		//	return FALSE;

		TLInput::Platform::IPod::ProcessVirtualKey(KeyRef);
	}
	
	return TRUE;
}

@end

#define ENABLE_VIRTUAL_KEYBOARD

using namespace TLInput;

Bool TLInput::Platform::IPod::CreateVirtualKeyboard()
{
#ifdef ENABLE_VIRTUAL_KEYBOARD
	if ( !g_TextFieldViewController )
		g_TextFieldViewController = [[TextFieldController alloc] init];
	
	//[g_TextFieldViewController loadView];
	
	// add this a subview of the main window
	TLRender::TScreen* pScreen = TLRender::g_pScreenManager->GetDefaultScreen();
	TLGui::TWindow* pWindow = pScreen ? pScreen->GetWindow() : NULL;
	TLGui::Platform::Window* pPlatformWindow = static_cast<TLGui::Platform::Window*>( pWindow );
	UIWindow* pUiWindow = pPlatformWindow ? pPlatformWindow->m_pWindow : NULL;
	
	//	can't get a window to attach to, probably want this function to fail rather than fail silently
	if ( !pUiWindow )
		return false;
	
	[pUiWindow addSubview: g_TextFieldViewController.view];


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
	g_bVirtualKeyboardActive = TRUE;
	
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
	
	g_bVirtualKeyboardActive = FALSE;
	
	return TRUE;
#else
	return FALSE;
#endif
}
