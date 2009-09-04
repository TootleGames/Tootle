/*
 *  IPodAlert.mm
 *  TootleCore
 *
 *  Created by Duane Bradbury on 19/05/2009.
 *  Copyright 2009 Tootle. All rights reserved.
 *
 */

#include "IPodAlert.h"

@implementation AlertsViewController


- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	// use "buttonIndex" to decide your action
	//[myTableView deselectRowAtIndexPath:[myTableView indexPathForSelectedRow] animated:NO];
	assert(FALSE);
}

- (void)dialogueOKCancelAction:(NSString *)nsstr
{
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Error" message:nsstr
												   delegate:self cancelButtonTitle:@"Cancel" otherButtonTitles:@"OK", nil];
	[alert show];
	[alert release];	
}

- (SyncBool)dialogueResult
{
	return SyncWait;
}



@end
