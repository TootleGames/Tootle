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
#import "OFUsersCredentialService.h"
#import "OFUsersCredential.h"
#import "OFService+Private.h"
#import "OFHttpNestedQueryStringWriter.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFUsersCredentialService);

@implementation OFUsersCredentialService

OPENFEINT_DEFINE_SERVICE(OFUsersCredentialService);

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFUsersCredential getResourceName], [OFUsersCredential class]);
}

+ (void) getIndexOnSuccess:(const OFDelegate&)onSuccess 
				 onFailure:(const OFDelegate&)onFailure 
onlyIncludeNotLinkedCredentials:(bool)onlyIncludeNotLinkedCredentials
onlyIncludeFriendsCredentials:(bool)onlyIncludeFriendsCredentials
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	params->io("only_include_not_linked_credentials", onlyIncludeNotLinkedCredentials);
	params->io("only_include_friends_credentials", onlyIncludeFriendsCredentials);
	OFRetainedPtr<NSString> me = @"me";
	params->io("user_id", me);
	
	[[self sharedInstance]
		getAction:@"users_credentials"
		withParameters:params
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestForeground
		withNotice:@"Downloading"];
}


+ (void) getIndexOnSuccess:(const OFDelegate&)onSuccess 
				 onFailure:(const OFDelegate&)onFailure 
onlyIncludeLinkedCredentials:(bool)onlyIncludeLinkedCredentials
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	params->io("only_include_linked_credentials", onlyIncludeLinkedCredentials);
	OFRetainedPtr<NSString> me = @"me";
	params->io("user_id", me);
	
	[[self sharedInstance]
	 getAction:@"users_credentials"
	 withParameters:params
	 withSuccess:onSuccess
	 withFailure:onFailure
	 withRequestType:OFActionRequestSilent
	 withNotice:@"Downloading"];
}

+ (void)importFriendsFromCredentialType:(NSString*)credentialType 
							  onSuccess:(const OFDelegate&)onSuccess 
							  onFailure:(const OFDelegate&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	params->io("credential_name", credentialType);
	params->io("user_id", @"me");
	
	[[self sharedInstance]
	 getAction:@"users_credentials/import_friends"
	 withParameters:params
	 withSuccess:onSuccess
	 withFailure:onFailure
	 withRequestType:OFActionRequestSilent
	 withNotice:nil];
}

@end
