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

#import "OFFacebookExtendedCredentialController.h"
#import "FBConnect.h"
#import "OFSettings.h"
#import "OFFormControllerHelper+Overridables.h"
#import "OFFormControllerHelper+Submit.h"
#import "OFISerializer.h"
#import "OFNavigationController.h"
#import "OFFacebookAccountController.h"
#import "OFControllerLoader.h"


@implementation OFFacebookExtendedCredentialController

@synthesize extendedCredentialController = mExtendedCredentialController;


- (NSString*)getFormSubmissionUrl 
{
	return @"extended_credentials.xml";
}

-(NSString*)singularResourceName
{
	return @"credential";
}


- (void)addHiddenParameters:(OFISerializer*)parameterStream
{
	[super addHiddenParameters:parameterStream];
	
	OFRetainedPtr <NSString> hasExtendedCredentials = @"true"; 
	parameterStream->io("credential[has_extended_credentials]", hasExtendedCredentials);	
}

- (void)displayError:(NSString*)errorString
{
	[[[[UIAlertView alloc] 
	   initWithTitle:@"Facebook Connect Error"
	   message:errorString
	   delegate:nil
	   cancelButtonTitle:@"Ok"
	   otherButtonTitles:nil] autorelease] show];
	
	[self dismiss];
}

-(void)populateViewDataMap:(OFViewDataMap*)dataMap
{
}


-(void)dismiss
{
	[mExtendedCredentialController dismissModalViewControllerAnimated:YES];
}
-(void)onFormSubmitted
{
	mExtendedCredentialController.facebookCredentialsReceived = true;
	[self dismiss];
}

- (void)registerActionsNow
{
}

- (void)dialogDidCancel:(FBDialog*)dialog
{
	[self closeLoginDialog];
	[self dismiss];
}

- (void)dialogDidSucceed:(FBDialog*)dialog
{
	[self onSubmitForm:nil];
}

-(void)showExtendedPermissionsDialog
{
	FBPermissionDialog* dialog = [[[FBPermissionDialog alloc] init] autorelease];
	dialog.delegate = self; 
	dialog.permission = @"publish_stream"; 
	[dialog show];
}



- (void)session:(FBSession*)session didLogin:(FBUID)uid
{
	[self closeLoginDialog];
	self.fbuid = uid;
	self.fbSession = session;
	self.fbLoggedInStatusImageView.image = [UIImage imageNamed:@"OpenFeintStatusIconNotificationSuccess.png"];
	[self showExtendedPermissionsDialog];
	
}



@end
