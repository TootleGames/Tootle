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

#import "OFTwitterExtendedCredentialController.h"
#import "OFViewDataMap.h"
#import "OFISerializer.h"
#import "OFFormControllerHelper+Overridables.h"
#import "OFControllerLoader.h"

@implementation OFTwitterExtendedCredentialController

@synthesize extendedCredentialController = mExtendedCredentialController;

- (NSString*)getFormSubmissionUrl 
{
	return @"extended_credentials.xml";
}

-(NSString*)singularResourceName
{
	return @"credential";
}

-(void)populateViewDataMap:(OFViewDataMap*)dataMap
{
	dataMap -> addFieldReference(@"password", 1);
}

-(void)addHiddenParameters:(OFISerializer*)parameterStream
{
	[super addHiddenParameters:parameterStream];
	OFRetainedPtr <NSString> credential_type = @"twitter";
	parameterStream->io("credential_type", credential_type);
}


- (void)didReceiveMemoryWarning 
{
	[super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
	// Release anything that's not essential, such as cached data
}

-(void)dismiss
{
	[self dismissModalViewControllerAnimated:YES];
}

-(void)onFormSubmitted
{
	mExtendedCredentialController.twitterCredentialsReceived = true;
	[self dismiss];
}
- (void)registerActionsNow
{
}
- (bool)canReceiveCallbacksNow
{
	return true;
}
- (void)dealloc 
{
	[super dealloc];
}



@end
