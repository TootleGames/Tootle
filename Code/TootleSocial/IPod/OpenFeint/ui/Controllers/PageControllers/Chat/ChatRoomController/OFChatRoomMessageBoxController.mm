////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFDependencies.h"
#import "OFChatRoomMessageBoxController.h"
#import "OFViewDataMap.h"
#import "OFChatMessage.h"
#import "OFNavigationController.h"
#import "OpenFeint+Private.h"

@implementation OFChatRoomMessageBoxController

@synthesize textEntryField;
@synthesize hideKeyboardButton;

- (void)viewDidLoad
{
	[super viewDidLoad];
	self.view.frame = CGRectMake(0.0f, 0.0f, [OpenFeint getDashboardBounds].size.width, 44.0f);
	mIsKeyboardShown = false;
}

- (void)_KeyboardWillShow:(NSNotification*)notification
{
	if(mIsKeyboardShown)
	{
		return;
	}

	[hideKeyboardButton setImage:[UIImage imageNamed:@"OFKeyboardDown.png"] forState:UIControlStateNormal];
	CGRect keyboardButtonRect = hideKeyboardButton.frame;
	keyboardButtonRect.origin.y += [OpenFeint isInLandscapeMode] ? 6.0f : 5.0f;
	[hideKeyboardButton setFrame:keyboardButtonRect];
	mIsKeyboardShown = true;
}

- (void)_KeyboardWillHide:(NSNotification*)notification
{
	if(!mIsKeyboardShown)
	{
		return;
	}
		
	[hideKeyboardButton setImage:[UIImage imageNamed:@"OFKeyboardUp.png"] forState:UIControlStateNormal];
	CGRect keyboardButtonRect = hideKeyboardButton.frame;
	keyboardButtonRect.origin.y -= [OpenFeint isInLandscapeMode] ? 6.0f : 5.0f;
	[hideKeyboardButton setFrame:keyboardButtonRect];
	mIsKeyboardShown = false;	
}

- (IBAction)toggleKeyboardNow
{
	if(mIsKeyboardShown)
	{
		[textEntryField resignFirstResponder];
	}
	else
	{
		[textEntryField becomeFirstResponder];
	}
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];

	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_KeyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_KeyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];	
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (bool)shouldShowLoadingScreenWhileSubmitting
{
	return false;
}

- (void)registerActionsNow
{
}

- (void)populateViewDataMap:(OFViewDataMap*)dataMap
{
	dataMap->addFieldReference(@"message", 1);
}

- (NSString*)getFormSubmissionUrl
{
	return @"chat_messages.xml";
}

- (NSString*)getTextToShowWhileSubmitting
{
	return @"Sending Chat Message";
}

- (void)onBeforeFormSubmitted
{
	textEntryField.text = @"";
}

- (void)onFormSubmitted
{
	// Do Nothing
}

- (NSString*)singularResourceName
{
	return [OFChatMessage getResourceName];
}

- (void)dealloc 
{
	self.textEntryField = nil;
	self.hideKeyboardButton = nil;
    [super dealloc];
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

- (bool)shouldDismissKeyboardWhenSubmitting
{
	return false;
}
@end
