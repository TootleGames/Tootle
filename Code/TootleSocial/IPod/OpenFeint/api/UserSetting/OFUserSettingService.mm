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
#import "OFUserSettingService.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFService+Private.h"
#import "OFUserSetting.h"
#import "OFUserSettingPushController.h"
#import "OFLeaderboard.h"

OPENFEINT_DEFINE_SERVICE_INSTANCE(OFUserSettingService);

@implementation OFUserSettingService

OPENFEINT_DEFINE_SERVICE(OFUserSettingService);

- (void) populateKnownResources:(OFResourceNameMap*)namedResources
{
	namedResources->addResource([OFUserSetting getResourceName], [OFUserSetting class]);
	namedResources->addResource([OFUserSettingPushController getResourceName], [OFUserSettingPushController class]);
}

+ (void) getIndexOnSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;
	[[self sharedInstance]
		getAction:@"users/@me/settings.xml"
		withParameters:params
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestForeground
		withNotice:@"Downloading Settings"];
}

+ (void) setUserSettingWithId:(NSString*)settingId toBoolValue:(bool)value onSuccess:(const OFDelegate&)onSuccess onFailure:(const OFDelegate&)onFailure
{	
	OFPointer<OFHttpNestedQueryStringWriter> params = new OFHttpNestedQueryStringWriter;

	{
		OFISerializer::Scope high_score(params, "user_setting");
		OFRetainedPtr<NSString> idString = settingId;
		params->io("id", idString);
		params->io("value", value);		
	}

	[[self sharedInstance]
		postAction:@"users/@me/settings.xml"
		withParameters:params
		withSuccess:onSuccess
		withFailure:onFailure
		withRequestType:OFActionRequestForeground
		withNotice:[NSString stringWithFormat:@"Updating Setting"]];
}

@end
