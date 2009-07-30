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

#include "OFResourceLoader.h"
#include "OFXmlDocument.h"
#include "OFXmlElement.h"
#include "OFViewDataSetter.h"

void OFResourceLoader::loadIntoView(NSString* resourceTypeName, OFXmlDocument* resourceXml, OFViewDataSetter* setter)
{
	[resourceXml pushNextScope:[resourceTypeName UTF8String]];
	while(OFPointer<OFXmlElement> nextElement = [resourceXml readNextElement])
	{
		if(nextElement->hasChildren())
		{
			OFAssert(0, "Resource field has child fields. Skipping. This is not currently supported. (%@)\r\n", resourceTypeName);
			continue;
		}

		NSString* fieldName = nextElement->getName();
		
		if(!setter->isValidField(fieldName))
		{
			OFAssert(0, "Skipping invalid field %@.%@", resourceTypeName, fieldName);
			continue;
		}
		
		setter->setField(fieldName, nextElement->getValue());
	}

	[resourceXml popScope];
}