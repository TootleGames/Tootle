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
#import "OFFormControllerHelper.h"
#import "OFViewHelper.h"
#import "OFViewDataMap.h"
#import "OFHttpService.h"
#import "OFFormControllerHelper+Overridables.h"
#import "OFProvider.h"
#import "OFViewHelper.h"
#import "OFFormControllerHelper+Submit.h"
#import "OpenFeint+Private.h"

@implementation OFFormControllerHelper

/////////////////////////////////////////////////////////////////////////////////////////////

- (void)viewDidLoad
{	
	mHttpService = [OFProvider createHttpService];
	
	mTagActions.clear();
	[self registerActionsNow];
	
	mViewDataMap = new OFViewDataMap;
	[self populateViewDataMap:mViewDataMap.get()];
	
	OFViewHelper::setAsDelegateForAllTextFields((id)self, self.view);
	OFViewHelper::setReturnKeyForAllTextFields(UIReturnKeySend, self.view);	
	
	mScrollContainerView = [OFViewHelper::findFirstScrollView(self.view) retain];
	
	CGSize contentSize = OFViewHelper::sizeThatFitsTight(mScrollContainerView);
	contentSize.height += 20.f;
	mScrollContainerView.contentSize = contentSize;
}

- (void)viewDidAppear:(BOOL)animated
{
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_KeyboardDidShow:) name:UIKeyboardDidShowNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_KeyboardDidHide:) name:UIKeyboardDidHideNotification object:nil];	
	[super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
	
	mHttpService->cancelAllRequests();
	OFViewHelper::resignFirstResponder(self.view);
}

- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
	[self hideLoadingScreen];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (bool)canReceiveCallbacksNow
{
	return [self navigationController] != nil;
}

- (void)dealloc
{
	[mLoadingScreen release]; mLoadingScreen = NULL;
	[mScrollContainerView release]; mScrollContainerView = NULL;
	[super dealloc];
}


- (IBAction)onTriggerAction:(UIView*)sender
{
	TagActionMap::const_iterator sit = mTagActions.find(sender.tag);
	if(sit == mTagActions.end())
	{
		OFAssert(0, "Attempting to trigger action for tag %d. No action has been registered.", sender.tag); 
		return;
	}
	
	sit->second.invoke();
}
- (IBAction)onTriggerBarAction:(UIBarButtonItem*)sender
{
	[self onTriggerAction:(UIView*)sender]; //UIBarButtonItem isn't a UIView, but we're pretending it is for triggering actions
}

- (void)registerAction:(OFDelegate)action forTag:(int)tag
{
	mTagActions.insert(TagActionMap::value_type(tag, action));
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

@end
