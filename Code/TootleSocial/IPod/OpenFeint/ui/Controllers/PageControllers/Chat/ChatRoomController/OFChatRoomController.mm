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
#import "OFChatRoomController.h"
#import "OFService+Private.h"
#import "OFChatMessageService.h"
#import "OFChatMessage.h"
#import "OFResourceControllerMap.h"
#import "OpenFeint+Private.h"
#import "OFTableSequenceControllerHelper+Overridables.h"
#import "OFControllerLoader.h"
#import "OFProfileController.h"
#import "OFChatRoomMessageBoxController.h"
#import "OFChatRoomInstance.h"
#import "OFDashboardNotificationView.h"
#import "OFInputResponsePerformSelector.h"
#import "OFChangeNameController.h"
#import "OFViewHelper.h"
#import "OpenFeint+Settings.h"
#import "OpenFeint+UserOptions.h"
#import "OFChatRoomInstanceService.h"

// citron note: using hidesBottomBarWhenPushed is not working for some reason when pushing *another* view after this one.
//				to make sure that it works properly, instead of actually hiding the tab bar we are just going to overlay
//				the keyboard on top of it.
static const unsigned int gTabBarHeight = 0;

@implementation OFChatRoomController

@synthesize roomInstance;

- (BOOL)isChangingName
{
	return self.modalViewController != nil;
}

- (void)arrangeToolbarAndSetNameBar
{
	OFDashboardNotificationView* notificationView = nil;
	if (![OpenFeint lastLoggedInUserHasSetName])
	{
		NSString* userName = [OpenFeint lastLoggedInUserName];
		
		OFNotificationInputResponse* inputResponse = [OFInputResponsePerformSelector responseWithSelector:@selector(changeNameClicked) 
																								andTarget:self];
		
		notificationView = [OFDashboardNotificationView notificationWithText:[NSString stringWithFormat:@"Hey %@. Set your name.", userName]
															andInputResponse:inputResponse];
	}
	
	[self _rearrangeViewsWithToolbarOnTopAtBottom:mChatRoomMessageBoxController.view andNotificationView:notificationView];
}

- (void)setTopBarsVisible:(BOOL)visible
{
	if (!visible == self.navigationController.navigationBarHidden)
	{
		return;
	}
	
	if (visible)
	{
		// jw note: hiding/showing the nav bar sets it to portrait width which messes up the animation etc so we need to animate it manually
		[self.navigationController setNavigationBarHidden:NO animated:NO];
		
		CGAffineTransform endTransform = self.navigationController.navigationBar.transform;
		CGAffineTransform startTransform = CGAffineTransformTranslate(endTransform, 0.f, -self.navigationController.navigationBar.frame.size.height);
		[self.navigationController.navigationBar setTransform:startTransform];
		
		[UIView beginAnimations:nil context:nil];
		const float kAnimDuration = 0.3f;
		[UIView setAnimationDuration:kAnimDuration];
		self.navigationController.navigationBar.transform = endTransform;
		[UIView commitAnimations];		
	}
	else
	{		
		// jw note: animating out works fine so we let it handle itself so it ends up in the correct "hidden" state
		[self.navigationController setNavigationBarHidden:YES animated:YES];
	}
	
	CGRect barFrame = self.navigationController.navigationBar.frame;
	barFrame.size.width = self.view.frame.size.width;
	self.navigationController.navigationBar.frame = barFrame;
}

- (void)_KeyboardWillShow:(NSNotification*)notification
{
    if (mIsKeyboardShown)
        return;
		
	if ([OpenFeint isInLandscapeMode])
	{
		[self setTopBarsVisible:NO];
	}
	
    NSValue* aValue = [[notification userInfo] objectForKey:UIKeyboardBoundsUserInfoKey];
    CGSize keyboardSize = [aValue CGRectValue].size;

	CGRect tableRect = [self.tableView frame];
	CGRect messageBoxRect = [mChatRoomMessageBoxController.view frame];
	
	float scrollOffset = (keyboardSize.height - gTabBarHeight);
	if (mDashboardNotificationView)
	{
		mDashboardNotificationView.hidden = YES;
		tableRect.size.height += mDashboardNotificationView.frame.size.height;
	}
	tableRect.size.height -= scrollOffset;
	messageBoxRect.origin.y -= scrollOffset;

	CGPoint offsetBeforeResizing = self.tableView.contentOffset;

	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:0.3f];		
	[self.tableView setFrame:tableRect];
	[mChatRoomMessageBoxController.view setFrame:messageBoxRect];
	[UIView commitAnimations];

	float offsetAfterResizing = offsetBeforeResizing.y + scrollOffset;
	const float largsetScrollableOffset = (self.tableView.contentSize.height - self.tableView.frame.size.height);
	if(offsetAfterResizing > largsetScrollableOffset)
	{
		offsetAfterResizing = largsetScrollableOffset; 
	}	
	[self.tableView setContentOffset:CGPointMake(offsetBeforeResizing.x, offsetAfterResizing) animated:NO];	  
	
    mIsKeyboardShown = YES;
}

- (void)_KeyboardWillHide:(NSNotification*)notification
{
    if (!mIsKeyboardShown)
        return;
	
	float additionalContentOffset = 0.0f;
	if ([OpenFeint isInLandscapeMode])
	{
		[self setTopBarsVisible:YES];
		additionalContentOffset = self.navigationController.navigationBar.frame.size.height;
	}

    NSValue* aValue = [[notification userInfo] objectForKey:UIKeyboardBoundsUserInfoKey];
    CGSize keyboardSize = [aValue CGRectValue].size;
	
	CGRect tableRect = [self.tableView frame];
	CGRect messageBoxRect = [mChatRoomMessageBoxController.view frame];
	
	float scrollOffset = (keyboardSize.height - gTabBarHeight);
	float notificationOffset = 0.f;
	tableRect.size.height += scrollOffset;
	if (mDashboardNotificationView)
	{
		mDashboardNotificationView.hidden = NO;
		notificationOffset = mDashboardNotificationView.frame.size.height;
		tableRect.size.height -= notificationOffset;
	}
	messageBoxRect.origin.y += scrollOffset;

	CGPoint offsetBeforeResizing = self.tableView.contentOffset;

	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:0.3f];		
	[self.tableView setFrame:tableRect];
	[mChatRoomMessageBoxController.view setFrame:messageBoxRect];
	[UIView commitAnimations];

	float offsetAfterResizing = offsetBeforeResizing.y - scrollOffset + additionalContentOffset + notificationOffset;
	if(offsetAfterResizing < 0)
	{
		offsetAfterResizing = 0; 
	}			
	[self.tableView setContentOffset:CGPointMake(offsetBeforeResizing.x, offsetAfterResizing) animated:NO];	 
	
    mIsKeyboardShown = NO;
}

- (void)dealloc
{
	[mChatRoomMessageBoxController release];
	[roomInstance release];
	[super dealloc];
}

- (void)viewDidLoad
{	
	[super viewDidLoad];
	mChatRoomMessageBoxController = [OFControllerLoader::load(@"ChatRoomMessageBox") retain];
}

- (void)closeChangeNameModal
{
	[self dismissModalViewControllerAnimated:YES];
}

- (void)changeNameClicked
{
	OFChangeNameController* changeNameController = (OFChangeNameController*)OFControllerLoader::load(@"ChangeName");
	UINavigationController* modalNavController = [[[UINavigationController alloc] initWithRootViewController:changeNameController] autorelease];
	
	NSString* backgroundImageName = [OpenFeint isInLandscapeMode] ? @"OpenFeintBackgroundWhiteLandscape.png" : @"OpenFeintBackgroundWhite.png";
	UIView* backgroundView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:backgroundImageName]];
	[modalNavController.view addSubview:backgroundView];
	[modalNavController.view sendSubviewToBack:backgroundView];
	
	modalNavController.navigationBar.barStyle = UIBarStyleBlackOpaque;
	UIBarButtonItem* cancelButton = [[[UIBarButtonItem alloc] initWithTitle:@"Cancel" style:UIBarButtonItemStylePlain target:self action:@selector(closeChangeNameModal)] autorelease];
	[modalNavController.navigationBar.topItem setLeftBarButtonItem:cancelButton];
	
	[self presentModalViewController:modalNavController animated:YES];
}

- (void)rejoinRoom
{
	OFDelegate success(self, @selector(onRejoinedRoom));
	OFDelegate failure(self, @selector(onFailedToRejoinRoom));
	[OFChatRoomInstanceService attemptToJoinRoom:roomInstance rejoining:YES onSuccess:success onFailure:failure];
}

- (void)onRejoinedRoom
{
	[OpenFeint setPollingFrequency:[OpenFeint getPollingFrequencyInChat]];
}

- (void)onFailedToRejoinRoom
{
	mRoomIsFull = true;
	[[[[UIAlertView alloc] initWithTitle:@"Room Full!" message:@"Sorry, you've lost your spot due to inactivity." delegate:self cancelButtonTitle:@"OK" otherButtonTitles:nil] autorelease] show];
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
	alertView.delegate = nil;
	if (mRoomIsFull)
	{
		[self.navigationController popViewControllerAnimated:YES];
	}
	
}
- (void)viewWillAppear:(BOOL)animated
{
	if ([self isChangingName])
	{
		[self arrangeToolbarAndSetNameBar];
		return;
	}
	[super viewWillAppear:animated];

	self.title = self.roomInstance.roomName;
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_KeyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_KeyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];	
	
	[mChatRoomMessageBoxController viewWillAppear:animated];
	if (mHasAppearedBefore)
	{
		[self rejoinRoom];
	}
	else
	{
		[OpenFeint setPollingFrequency:[OpenFeint getPollingFrequencyInChat]];
	}

	[self arrangeToolbarAndSetNameBar];
	mHasAppearedBefore = true;
}

- (void)viewDidAppear:(BOOL)animated
{
	if ([self isChangingName])
	{
		return;
	}
	[super viewDidAppear:animated];
	[mChatRoomMessageBoxController viewDidAppear:animated];
}

-(void)viewWillDisappear:(BOOL)animated
{
	if ([self isChangingName])
	{
		return;
	}
	[super viewWillDisappear:animated];
	if ([OpenFeint isInLandscapeMode])
	{
		[self setTopBarsVisible:YES];
	}
	[mChatRoomMessageBoxController viewWillDisappear:animated];
	[OpenFeint stopPolling];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)viewDidDisappear:(BOOL)animated
{
	if ([self isChangingName])
	{
		return;
	}

	[super viewDidDisappear:animated];
	[mChatRoomMessageBoxController viewDidDisappear:animated];
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
	if (self.interfaceOrientation == UIInterfaceOrientationPortrait ||
		self.interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown)
	{
		[self setTopBarsVisible:YES];
	}
	[super didRotateFromInterfaceOrientation:fromInterfaceOrientation];
}

- (void)populateResourceMap:(OFResourceControllerMap*)resourceMap
{
	resourceMap->addResource([OFChatMessage class], @"ChatRoomChatMessage");
}

- (OFService*)getService
{
	return [OFChatMessageService sharedInstance];
}

- (NSString*)getTableHeaderControllerName
{
	return nil;
}

- (NSString*)getNoDataFoundMessage
{
	return [NSString stringWithFormat:@"We're downloading messages in the background or no one has said anything yet."];
}

- (bool)shouldRefreshAfterNotification
{
	return true;
}

- (NSString*)getNotificationToRefreshAfter
{
	return [OFChatMessage getResourceDiscoveredNotification];
}

- (bool)isNewContentShownAtBottom
{
	return true;
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	success.invoke(nil);
}

@end