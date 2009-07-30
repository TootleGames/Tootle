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
#import "OFFormControllerHelper+Submit.h"
#import "OFFormControllerHelper+Overridables.h"

#import "OFViewDataGetter.h"
#import "OFHttpNestedQueryStringWriter.h"
#import "OFControllerLoader.h"
#import "OFViewHelper.h"
#import "OFXmlDocument.h"
#import "OFXmlElement.h"
#import "OFProvider.h"
#import "OpenFeint+Private.h"
#import "MPOAuthAPIRequestLoader.h"
#import "OFLoadingController.h"

static NSString* parseErrorXml(NSData* errorXml)
{
	NSMutableString* theWholeReason = [NSMutableString string];
	
	OFXmlDocument* doc = [OFXmlDocument xmlDocumentWithData:errorXml];
	[doc pushNextScope:"errors"];
	while(OFPointer<OFXmlElement> nextError = [doc readNextElement])
	{
		NSString* field = nextError->getAttributeNamed(@"field");
		NSString* reason = nextError->getAttributeNamed(@"reason");
		
		if([field isEqualToString:@"base"])
		{
			[theWholeReason appendFormat:@"%@\n", reason];
		}
		else
		{
			[theWholeReason appendFormat:@"- %@: %@\n", field, reason];
		}
	}
	[doc popScope];
	
	return theWholeReason;
}

@interface OFFormControllerHelper ()
- (void) submitForm;
@end

@implementation OFFormControllerHelper ( Submit )

- (void)submitForm
{
	OFPointer<OFHttpNestedQueryStringWriter> queryStream = new OFHttpNestedQueryStringWriter;
	{
		OFISerializer::Scope resource(queryStream, [[self singularResourceName] UTF8String]);
		
		OFViewDataGetter getter(self.view, mViewDataMap);
		getter.serialize(queryStream);
	}
	
	[self addHiddenParameters:queryStream.get()];

	[self onBeforeFormSubmitted];

	[[OpenFeint provider] performAction:[self getFormSubmissionUrl]
					  withParameters:queryStream->getQueryParametersAsMPURLRequestParameters()
						withHttpMethod:[self getHTTPMethod]
						  withSuccess:OFDelegate(self, @selector(_requestRespondedBehavior:))
						   withFailure:OFDelegate(self, @selector(_requestErroredBehavior:))
						   withRequestType:OFActionRequestForeground
						   withNotice:[self getTextToShowWhileSubmitting]
						   requiringAuthentication:[self shouldUseOAuth]];
}

- (void) showLoadingScreenWithMessage:(NSString*)message
{
	OFSafeRelease(mLoadingScreen);
	mLoadingScreen = [[OFLoadingController loadingControllerWithText:message] retain];
	[self.view addSubview:mLoadingScreen.view];
}

- (void) showLoadingScreen
{
	[self showLoadingScreenWithMessage:[self getTextToShowWhileSubmitting]];
}

- (void) hideLoadingScreen
{
	[mLoadingScreen.view removeFromSuperview];
	OFSafeRelease(mLoadingScreen);
}

- (void)_requestRespondedBehavior:(MPOAuthAPIRequestLoader*)response
{
 	[self hideLoadingScreen];
	[self onFormSubmitted];
}

- (void)_requestErroredBehavior:(MPOAuthAPIRequestLoader*)response
{
	[self hideLoadingScreen];
	[self onPresentingErrorDialog];

	NSString* message = parseErrorXml(response.data);
	NSString* okButtonTitle = @"Ok. I'll correct it.";
	if ([message length] == 0)
	{
		NSError* error = response.error;						
		message = [NSString stringWithFormat:@"%@ (%d[%d])", [error localizedDescription], error.domain, error.code];
		okButtonTitle = @"Ok.";
	}
	
	[[[[UIAlertView alloc] initWithTitle:@"Oops! There was a problem:" 
								message:message
								delegate:nil
								cancelButtonTitle:okButtonTitle
								otherButtonTitles:nil] autorelease] show];
}

- (IBAction)onSubmitForm:(UIView*)sender
{
	if([self shouldShowLoadingScreenWhileSubmitting])
	{
		[self showLoadingScreen];
	}
	
	if([self shouldDismissKeyboardWhenSubmitting])
	{
		OFViewHelper::resignFirstResponder(self.view);			
	}

	[self submitForm];
}

@end
