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

#import "OFDependencies.h"
#import "OFFormControllerHelper+EditingSupport.h"
#import "OFFormControllerHelper+Submit.h"
#import "OFViewHelper.h"
#import "OFFormControllerHelper+Overridables.h"
#import "OpenFeint+Private.h"

@implementation OFFormControllerHelper ( EditingSupport )

- (void)_KeyboardDidShow:(NSNotification*)notification
{
    if (mIsKeyboardShown)
	{
		return;
	}
		
    NSDictionary* info = [notification userInfo];
 
    NSValue* aValue = [info objectForKey:UIKeyboardBoundsUserInfoKey];
    CGSize keyboardSize = [aValue CGRectValue].size;

    CGRect viewFrame = [mScrollContainerView frame];
	CGPoint screenSpaceOrigin = [mScrollContainerView convertPoint:viewFrame.origin toView:[OpenFeint getTopLevelView]];
	mViewHeightWithoutKeyboard = viewFrame.size.height;
	float bottomOfView = [OpenFeint getDashboardBounds].size.height - keyboardSize.height;
	viewFrame.size.height = bottomOfView - screenSpaceOrigin.y;
	
	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:0.3f];		
    mScrollContainerView.frame = viewFrame;
	[UIView commitAnimations];

	CGRect rectToMakeVisible = mActiveTextField.frame;
	if ([mActiveTextField superview] != mScrollContainerView)
	{
		rectToMakeVisible.origin.x += [[mActiveTextField superview] frame].origin.x;
		rectToMakeVisible.origin.y += [[mActiveTextField superview] frame].origin.y;
//		rectToMakeVisible.origin = [mActiveTextField convertPoint:rectToMakeVisible.origin toView:mScrollContainerView];
	}
	[mScrollContainerView scrollRectToVisible:rectToMakeVisible animated:YES];
 
    mIsKeyboardShown = YES;
}

- (void)_KeyboardDidHide:(NSNotification*)notification
{
    if (!mIsKeyboardShown)
	{
		return;
	}

    CGRect viewFrame = [mScrollContainerView frame];
    viewFrame.size.height = mViewHeightWithoutKeyboard;

	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:0.3f];		
    mScrollContainerView.frame = viewFrame;
	[UIView commitAnimations];
	
    mIsKeyboardShown = NO;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField
{
	mActiveTextField = textField;

	CGRect rectToMakeVisible = mActiveTextField.frame;
	if ([mActiveTextField superview] != mScrollContainerView)
	{
		rectToMakeVisible.origin.x += [[mActiveTextField superview] frame].origin.x;
		rectToMakeVisible.origin.y += [[mActiveTextField superview] frame].origin.y;
//		rectToMakeVisible.origin = [mActiveTextField convertPoint:rectToMakeVisible.origin toView:mScrollContainerView];
	}
	[mScrollContainerView scrollRectToVisible:rectToMakeVisible animated:YES];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	UIResponder* nextView = OFViewHelper::findViewByTag(self.view, textField.tag + 1);

	if(nextView != nil)
	{
		if([nextView isKindOfClass:[UITextField class]])
		{
			[nextView becomeFirstResponder];
		}
		else
		{
			[textField resignFirstResponder];
		}
	}
	else
	{		
		[self onSubmitForm:textField];
	}
	
	return YES;
}

@end
