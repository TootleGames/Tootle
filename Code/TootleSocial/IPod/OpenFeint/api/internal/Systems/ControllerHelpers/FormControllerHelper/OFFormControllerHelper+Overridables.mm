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
#import "OFFormControllerHelper+Overridables.h"
#import "OFControllerHelpersCommon.h"

@implementation OFFormControllerHelper ( Overridables )

- (void)onBeforeFormSubmitted
{
	// do nothing
}

- (void)onPresentingErrorDialog
{
	// do nothing
}

- (NSString*)getFormSubmissionUrl
{
	ASSERT_OVERRIDE_MISSING;
	return @"";
}

- (void)registerActionsNow
{
	ASSERT_OVERRIDE_MISSING;
}

- (NSString*)singularResourceName
{
	ASSERT_OVERRIDE_MISSING;
	return @"";
}

- (void)onFormSubmitted
{
	ASSERT_OVERRIDE_MISSING;
}

- (void)populateViewDataMap:(OFViewDataMap*)dataMap
{
}

- (NSString*)getTextToShowWhileSubmitting
{
	return @"Submitting";
}

- (bool)shouldUseOAuth
{
	return true;
}

- (void)addHiddenParameters:(OFISerializer*)parameterStream
{
}

- (bool)shouldShowLoadingScreenWhileSubmitting
{
	return true;
}

- (bool)shouldDismissKeyboardWhenSubmitting
{
	return true;
}

- (NSString*)getHTTPMethod
{
	return @"POST";
}

@end