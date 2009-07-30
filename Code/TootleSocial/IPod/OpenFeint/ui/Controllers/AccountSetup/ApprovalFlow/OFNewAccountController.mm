////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Copyright (c) 2009 Aurora Feint Inc.
///
///  This library is free software; you can redistribute it and/or
///  modify it under the terms of the GNU Lesser General Public
///  License as published by the Free Software Foundation; either
///  
///  version 3 of the License, or (at your option) any later version.
///  
///  This library is distributed in the hope that it will be useful,
///  
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
///  Lesser General Public License for more details.
///  
///  
///  You should have received a copy of the GNU Lesser General Public
///  License along with this library; if not, write to the Free Software
///  
///  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "OFNewAccountController.h"

#import "OpenFeint+UserOptions.h"
#import "OpenFeint+Private.h"
#import "OFControllerLoader.h"
#import "OFViewHelper.h"
#import "OFViewDataMap.h"
#import "OFISerializer.h"
#import "OFFormControllerHelper+EditingSupport.h"
#import "OFFormControllerHelper+Submit.h"
#import "OFAccountLoginController.h"

@interface OFNewAccountController ()
- (void)dismiss;
- (void)popBackToMe;
- (void)_showNameLoadingView;
- (void)_hideNameLoadingView;
@end

@implementation OFNewAccountController

@synthesize keepDashboardOpenOnApproval,
			nameEntryField,
			acceptNameButton,
			useDefaultNameButton,
			alreadyHaveAccountButton;

- (void)viewWillAppear:(BOOL)animated
{
	self.navigationItem.hidesBackButton = YES;

	[self _hideNameLoadingView];
	
	[nameEntryField setText:[OpenFeint lastLoggedInUserName]];
	[nameEntryField setPlaceholder:[OpenFeint lastLoggedInUserName]];

	[super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
	[[self navigationController] setNavigationBarHidden:YES animated:YES];
	[super viewDidAppear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
	[[self navigationController] setNavigationBarHidden:NO animated:YES];
	[super viewDidDisappear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

- (bool)canReceiveCallbacksNow
{
	return true;
}

- (void)dealloc
{
	self.nameEntryField = nil;
	self.acceptNameButton = nil;
	self.useDefaultNameButton = nil;
	OFSafeRelease(loadingView);
	[super dealloc];
}

- (void)popBackToMe
{
	[[self navigationController] popToViewController:self animated:YES];
}

- (void)dismiss
{
	if (!hasBeenDismissed)
	{
		[OpenFeint allowErrorScreens:YES];
		[OpenFeint dismissDashboard];
		
		if (keepDashboardOpenOnApproval)
		{
			[OpenFeint presentRootControllerWithTabbedDashboard];
		}

		hasBeenDismissed = YES;		
	}
}

- (void)_showNameLoadingView
{
	CGSize nameEntrySize = nameEntryField.frame.size;

	loadingView = [[UIView alloc] initWithFrame:nameEntryField.frame];
	loadingView.backgroundColor = [UIColor colorWithRed:0.f green:0.f blue:0.f alpha:0.5f];
	loadingView.opaque = NO;
	
	UIActivityIndicatorView* indicator = [[[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite] autorelease];
	[indicator startAnimating];

	UILabel* submitting = [[[UILabel alloc] initWithFrame:CGRectMake(0.0f, 0.0f, nameEntrySize.width, nameEntrySize.height)] autorelease];

	NSString* submittingText = @"Submitting name...";
	CGSize textSize = [submittingText sizeWithFont:submitting.font];

	submitting.backgroundColor = [UIColor clearColor];
	submitting.textColor = [UIColor whiteColor];
	submitting.text = submittingText;

	float const kSpaceBetweenIndicatorAndText = 4.f;
	float totalWidth = indicator.frame.size.width + textSize.width + kSpaceBetweenIndicatorAndText;
	float indicatorX = (nameEntrySize.width - totalWidth) * 0.5f;
	float indicatorY = (nameEntrySize.height - indicator.frame.size.height) * 0.5f;	
	float textX = indicatorX + indicator.frame.size.width + kSpaceBetweenIndicatorAndText;

	[indicator setFrame:CGRectMake(indicatorX, indicatorY, indicator.frame.size.width, indicator.frame.size.height)];
	[submitting setFrame:CGRectMake(textX, 0.0f, nameEntrySize.width, nameEntrySize.height)];
	
	[loadingView addSubview:indicator];
	[loadingView addSubview:submitting];
	[nameEntryField.superview addSubview:loadingView];
}

- (void)_hideNameLoadingView
{
	[loadingView removeFromSuperview];
	OFSafeRelease(loadingView);
}

- (IBAction)onSubmitForm:(UIView*)sender
{
	OFSafeRelease(desiredName);
	desiredName = [nameEntryField.text retain];
	
	[self _hideNameLoadingView];

	if (![desiredName isEqualToString:[OpenFeint lastLoggedInUserName]])
	{
		acceptNameButton.enabled = NO;
		alreadyHaveAccountButton.enabled = NO;
		
		[self _showNameLoadingView];
		[super onSubmitForm:sender];
	}
	else
	{
		[self dismiss];
	}
}

- (IBAction)_useDefaultName
{
	[self dismiss];
}

- (IBAction)_alreadyCreatedAccount
{
	OFAccountLoginController* accountFlowController = (OFAccountLoginController*)OFControllerLoader::load(@"OpenFeintAccountLogin");
	accountFlowController.neverShowLostAccountWarning = YES;
	[accountFlowController setCancelDelegate:OFDelegate(self, @selector(popBackToMe))];
	[accountFlowController setCompletionDelegate:OFDelegate(self, @selector(dismiss))];
	[[self navigationController] pushViewController:accountFlowController animated:YES];
}

// OFFormControllerHelper overrides
- (void)registerActionsNow
{
}

- (NSString*)getFormSubmissionUrl
{
	return @"users/update_name.xml";
}

- (void)onFormSubmitted
{
	[self _hideNameLoadingView];

	[OpenFeint setLastLoggedInUserName:desiredName];
	[OpenFeint setLoggedInUserHasSetName:YES];
	
	nameEntryField.text = desiredName;
	OFSafeRelease(desiredName);
	
	[self dismiss];
}

- (NSString*)singularResourceName
{
	return @"user";
}

- (NSString*)getHTTPMethod
{
	return @"POST";
}

// Optional OFFormControllerHelper overrides
- (void)onPresentingErrorDialog
{
	[self _hideNameLoadingView];	
	acceptNameButton.enabled = YES;
	alreadyHaveAccountButton.enabled = YES;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField
{
	[super textFieldDidBeginEditing:textField];
	[acceptNameButton setBackgroundImage:[UIImage imageNamed:@"OFApprovalSubmit.png"] forState:UIControlStateNormal];
	useDefaultNameButton.hidden = NO;
}

- (void)populateViewDataMap:(OFViewDataMap*)dataMap
{
	dataMap->addFieldReference(@"name", nameEntryField.tag);	
}

- (void)addHiddenParameters:(OFISerializer*)parameterStream
{
	parameterStream->io("id", @"me");
}

- (bool)shouldShowLoadingScreenWhileSubmitting
{
	return false;
}

- (bool)shouldDismissKeyboardWhenSubmitting
{
	return true;
}

@end
