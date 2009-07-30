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
#import "OFExtendedCredentialController.h"
#import "OFTwitterExtendedCredentialController.h"
#import "OFFacebookExtendedCredentialController.h"
#import "OFControllerLoader.h"
#import "OpenFeint+Private.h"
#import "OFNavigationController.h"

@implementation OFExtendedCredentialController
@synthesize requireTwitterCredentials = mRequireTwitterCredentials;
@synthesize requireFacebookCredentials = mRequireFacebookCredentials;
@synthesize twitterCredentialsReceived = mTwitterCredentialsReceived;
@synthesize facebookCredentialsReceived = mFacebookCredentialsReceived;
@synthesize twitterCredentialsButton = mTwitterCredentialsButton;
@synthesize twitterCredentialsStatusImage = mTwitterCredentialsStatusImage;
@synthesize facebookCredentialsButton = mFacebookCredentialsButton;
@synthesize facebookCredentialsStatusImage = mFacebookCredentialsStatusImage;
@synthesize request = mRequest;

- (void)_setupViewForNetworkWithCredentialButton:(UIButton*)credentialButton statusImage:(UIImageView*)statusImage credentialsRequired:(BOOL)credentialsRequired credentialsReceived:(BOOL)credentialsReceived
{
	if(credentialsRequired)
	{
		credentialButton.hidden = NO;
		statusImage.hidden = NO;
		if(credentialsReceived)
		{
			credentialButton.enabled = NO;
			statusImage.image = [UIImage imageNamed:@"OpenFeintStatusIconNotificationSuccess.png"];
		}
	}
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
	
	if ((mTwitterCredentialsReceived || !mRequireTwitterCredentials) && (mFacebookCredentialsReceived || !mRequireFacebookCredentials))
	{
		[self performSelector:@selector(dismiss) withObject:nil afterDelay:0.01f];
		return;
	}
	[self _setupViewForNetworkWithCredentialButton:mTwitterCredentialsButton 
									   statusImage:mTwitterCredentialsStatusImage 
							   credentialsRequired:mRequireTwitterCredentials 
							   credentialsReceived:mTwitterCredentialsReceived];
	
	[self _setupViewForNetworkWithCredentialButton:mFacebookCredentialsButton 
									   statusImage:mFacebookCredentialsStatusImage 
							   credentialsRequired:mRequireFacebookCredentials 
							   credentialsReceived:mFacebookCredentialsReceived];
	
	if ([OpenFeint isInLandscapeMode] && mTwitterCredentialsButton.hidden && !mFacebookCredentialsButton.hidden)
	{
		CGRect facebookButtonRect = mFacebookCredentialsButton.frame;
		facebookButtonRect.origin = mTwitterCredentialsButton.frame.origin;
		mFacebookCredentialsButton.frame = facebookButtonRect;
		
		CGRect facebookStatusRect = mFacebookCredentialsStatusImage.frame;
		facebookStatusRect.origin = mTwitterCredentialsStatusImage.frame.origin;
		mFacebookCredentialsStatusImage.frame = facebookStatusRect;
	}
}

- (IBAction) twitterCredentialsButtonClicked:(UIButton*)sender
{
	OFTwitterExtendedCredentialController* modal = (OFTwitterExtendedCredentialController*)OFControllerLoader::load(@"TwitterExtendedCredential");
	modal.extendedCredentialController = self;
	OFNavigationController* ofNavController = [[[OFNavigationController alloc] initWithRootViewController:modal] autorelease];
	UIBarButtonItem* cancelButton = [[[UIBarButtonItem alloc] initWithTitle:@"Cancel" style:UIBarButtonItemStylePlain target:modal action:@selector(dismiss)] autorelease];
	[ofNavController.navigationBar.topItem setLeftBarButtonItem:cancelButton];
	[ofNavController.navigationBar.topItem setTitle:@"Twitter"];
	[self presentModalViewController:ofNavController animated:YES];
}

- (IBAction) facebookCredentialsButtonClicked:(UIButton*)sender
{
	OFFacebookExtendedCredentialController* modal = (OFFacebookExtendedCredentialController*)OFControllerLoader::load(@"FacebookExtendedCredential");
	modal.extendedCredentialController = self;
	OFNavigationController* ofNavController = [[[OFNavigationController alloc] initWithRootViewController:modal] autorelease];
	[self presentModalViewController:ofNavController animated:YES];
}

- (IBAction) resubmitRequest:(UIButton*)sender
{
	[mRequest dispatch];
	[self dismiss];
}

- (void) dismiss
{
	[OpenFeint dismissDashboard];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

- (void)dealloc {
	self.twitterCredentialsButton = nil;
	self.twitterCredentialsStatusImage = nil;
	self.facebookCredentialsButton = nil;
	self.facebookCredentialsStatusImage = nil;
	self.request = nil;
	
    [super dealloc];
}


@end
