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

#import "OFViewDataMap.h"
#import "OFXmlDocument.h"
#import "OFXmlElement.h"
#import "OFViewHelper.h"

void OFViewDataMap::addFieldReference(NSString* fieldName, NSInteger viewTag)
{
	FieldReference newField;
	newField.name = fieldName;
	newField.tag = viewTag;
	mFields.push_back(newField);
}

UIView* OFViewDataMap::findViewByTag(UIView* rootView, int tag) const
{
	return OFViewHelper::findViewByTag(rootView, tag);
}

UIView* OFViewDataMap::findViewByName(UIView* rootView, NSString* fieldName) const
{
	FieldReferenceSeries::const_iterator sit = std::find(mFields.begin(), mFields.end(), fieldName);
	if(sit == mFields.end())
	{
		return NULL;
	}
	
	return findViewByTag(rootView, sit->tag);
}

bool OFViewDataMap::isValidField(NSString* name) const
{
	return std::find(mFields.begin(), mFields.end(), name) != mFields.end();
}

unsigned int OFViewDataMap::getFieldCount() const
{
	return mFields.size();
}

OFPointer<OFViewDataMap> OFViewDataMap::fromXml(OFXmlDocument* xmlData)
{
	OFPointer<OFViewDataMap> viewMap = new OFViewDataMap;
	
	[xmlData pushNextScope:"fields"];
	while(OFPointer<OFXmlElement> nextField = [xmlData readNextElement])
	{			
		if(NSString* name = nextField->getAttributeNamed(@"name"))
		{
			if(NSString* tag = nextField->getAttributeNamed(@"tag"))
			{
				viewMap->addFieldReference(name, [tag intValue]);
			}
			else
			{
				OFAssert(0, "Missing tag attribute for view data map entry %@", name);
			}			
		}
		else
		{
			OFAssert(0, "Missing name attribute for view data map.");
		}
	}

	[xmlData popScope];
	return viewMap;
}

OFViewDataMap::FieldReferenceSeries::const_iterator OFViewDataMap::begin() const
{
	return mFields.begin();
}

OFViewDataMap::FieldReferenceSeries::const_iterator OFViewDataMap::end() const
{
	return mFields.end();
}
