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
#import "OFTableSequenceControllerHelper+Overridables.h"
#import "OFControllerHelpersCommon.h"
#import "OFService+Overridables.h"

@implementation OFTableSequenceControllerHelper ( Overridables )

- (void)populateResourceMap:(OFResourceControllerMap*)mapToPopulate
{
	ASSERT_OVERRIDE_MISSING;
}

- (void)populateSectionHeaderFooterResourceMap:(OFResourceControllerMap*)mapToPopulate
{
	// Optional
}

- (void)onSectionsCreated
{
	// Optional
}

- (bool)allowPagination
{
	return true;
}

- (void)doIndexActionOnSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure;
{
	OFService* service = [self getService];
	[[service class] getIndexOnSuccess:success onFailure:failure];
}

- (void)doIndexActionWithPage:(unsigned int)oneBasedPageNumber onSuccess:(const OFDelegate&)success onFailure:(const OFDelegate&)failure
{
	ASSERT_OVERRIDE_MISSING;
}

@end

