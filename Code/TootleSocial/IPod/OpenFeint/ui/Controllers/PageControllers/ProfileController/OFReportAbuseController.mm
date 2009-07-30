//
//  OFReportAbuseController.m
//  OpenFeint
//
//  Created by Aurora Feint on 5/22/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OFReportAbuseController.h"
#import "OFViewDataMap.h"
#import "OFISerializer.h"
#import "OFFormControllerHelper+Overridables.h"
#import "OFControllerLoader.h"
#import "OFNavigationController.h"

@implementation OFReportAbuseController

@synthesize flaggedUserId = mFlaggedUserId;

+(UIViewController*)OFReportAbuseControllerForUserId:(NSString*)userId
{
	OFReportAbuseController* reportAbuseController = (OFReportAbuseController*)OFControllerLoader::load(@"ReportAbuse");	
	reportAbuseController.flaggedUserId = userId;
	OFNavigationController* ofNavController = [[[OFNavigationController alloc] initWithRootViewController:reportAbuseController] autorelease];
	UIBarButtonItem* cancelButton = [[[UIBarButtonItem alloc] initWithTitle:@"Cancel" style:UIBarButtonItemStylePlain target:reportAbuseController action:@selector(_dismissModalViewController)] autorelease];
	[ofNavController.navigationBar.topItem setLeftBarButtonItem:cancelButton];
	return ofNavController;
	
}
-(void)_dismissModalViewController
{
	[self.navigationController dismissModalViewControllerAnimated:true];
}

- (void)registerActionsNow
{
	[self registerAction:OFDelegate(self, @selector(_dismissModalViewController)) forTag:2];
}

- (NSString*)getFormSubmissionUrl 
{
	return [NSString stringWithFormat:@"users/%@/abuse_flags.xml", mFlaggedUserId];
}

-(NSString*)singularResourceName
{
	return @"abuse_flag";
}

-(void)populateViewDataMap:(OFViewDataMap*)dataMap
{
	dataMap -> addFieldReference(@"reason", 1);
}

-(void)addHiddenParameters:(OFISerializer*)parameterStream
{
	[super addHiddenParameters:parameterStream];
	OFRetainedPtr <NSString> abuse_type = @"chat";
	parameterStream->io("abuse_flag[abuse_type]", abuse_type);
}

- (void)didReceiveMemoryWarning 
{
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}

-(void)onFormSubmitted
{
	[[[[UIAlertView alloc] initWithTitle:@"Successfully Reported" 
						message:@"Abuse has been successfully reported. Thank you." 
					   delegate:self 
			  cancelButtonTitle:@"OK" 
			  otherButtonTitles: nil] autorelease] show];
}

- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
	[self _dismissModalViewController];
}

- (void)dealloc 
{
	self.flaggedUserId = nil;
    [super dealloc];
}



@end
