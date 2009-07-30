////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  This is beta software and is subject to changes without notice.
///
///  Do not distribute.
///
///  Copyright (c) 2009 Aurora Feint Inc. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#import "OFFullscreenImportFriendsMessage.h"
#import "OFControllerLoader.h"
#import "OFUsersCredential.h"
#import "OpenFeint+Private.h"

@implementation OFFullScreenImportFriendsMessage

@synthesize owner;
@synthesize messageLabel;

- (IBAction)onImportFriendsPressed
{
	[self.owner.navigationController pushViewController:OFControllerLoader::load(@"ImportFriends") animated:YES];
}

- (void)setMissingCredentials:(NSArray*)missingUsersCredentials withNotice:(NSString*)notice
{
	const unsigned int numMissingCredentials = [missingUsersCredentials count];

	NSMutableArray* networkNames = [NSMutableArray arrayWithCapacity:2];		
	for(unsigned int i = 0; i < numMissingCredentials; ++i)
	{	
		OFUsersCredential* credential = [missingUsersCredentials objectAtIndex:i];
		
		NSString* humanReadableName = [OFUsersCredential getDisplayNameForCredentialType:credential.credentialType];

		if(i == numMissingCredentials - 1 && numMissingCredentials > 1)
		{
			humanReadableName = [NSString stringWithFormat:@"or %@", humanReadableName];
		}
		
		[networkNames addObject:humanReadableName];
	}
		
	NSString* socialNetworkNames = @"your social networkNames";	
	if([missingUsersCredentials count] == 2)
	{
		socialNetworkNames = [networkNames componentsJoinedByString:@" "];
	}
	else if(numMissingCredentials > 0)
	{
		socialNetworkNames = [networkNames componentsJoinedByString:@", "];
	}
	
	NSString* maybeASpace = @"";
	if (notice && [notice length] >= 0)
		maybeASpace = @" ";
	
	NSString* formatString = @"%@%@Import your friends from %@ to see what OpenFeint games they're playing, compare progress, and more.";	
	messageLabel.text = [NSString stringWithFormat:formatString, notice, maybeASpace, socialNetworkNames];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return toInterfaceOrientation == [OpenFeint getDashboardOrientation];
}

@end