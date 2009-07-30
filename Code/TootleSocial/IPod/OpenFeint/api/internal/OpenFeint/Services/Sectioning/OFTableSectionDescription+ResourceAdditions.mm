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
#import "OFTableSectionDescription+ResourceAdditions.h"
#import "OFResource.h"

@implementation OFTableSectionDescription (ResourceAdditions)

- (NSMutableArray*)arrayWithoutDuplicates:(OFTableSectionDescription*)otherSection areSortedDescending:(BOOL)areSortedDescending
{
	NSMutableArray* adjustedResources = [NSMutableArray arrayWithCapacity:[otherSection.page count]];
	
	for(OFResource* newResource in otherSection.page.objects)
	{
		bool isDuplicate = false;
		
		const unsigned long long newResourceId = [newResource.resourceId longLongValue];
		if (newResourceId != 0)
		{
			for(OFResource* existingResource in self.page.objects)
			{
				const unsigned long long existingResourceId = [existingResource.resourceId longLongValue];
			
				if(areSortedDescending && newResourceId > existingResourceId)
				{
					break;
				}
				else if(newResourceId == existingResourceId)
				{
					isDuplicate = true;
					break;
				}
			}
		}
		
		if(isDuplicate)
		{
			break;
		}
		
		[adjustedResources addObject:newResource];
	}
	
	return adjustedResources;
}

- (unsigned int)addContentsOfSectionWhereUnique:(OFTableSectionDescription*)otherSection areSortedDescending:(BOOL)areSortedDescending shouldPrependContents:(BOOL)shouldPrependContents
{
	unsigned int numNewUniqueResources = 0;
	
	if(!self.page)
	{
		self.page = otherSection.page;
		numNewUniqueResources = [otherSection.page count];
	}
	else
	{
		NSMutableArray* adjustedResources = [self arrayWithoutDuplicates:otherSection areSortedDescending:areSortedDescending];
		numNewUniqueResources = [adjustedResources count];
		if(numNewUniqueResources)
		{
			if(shouldPrependContents)
			{
				[adjustedResources addObjectsFromArray:self.page.objects];
				self.page.objects = adjustedResources;
			}
			else
			{
				[self.page.objects addObjectsFromArray:adjustedResources];
			}			

			// citron note: This allows us to display data about this section with
			//				regards to the most recently added/loaded page.
			self.page.header = otherSection.page.header;
		}
	}
	
	return numNewUniqueResources;	
}

@end