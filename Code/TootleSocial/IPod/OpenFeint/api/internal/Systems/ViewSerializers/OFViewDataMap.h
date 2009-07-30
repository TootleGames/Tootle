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

#pragma once

#import "OFSmartObject.h"
#import "OFPointer.h"
#import <vector>

@class OFXmlDocument;
@protocol UITextFieldDelegate;

class OFViewDataMap : public OFSmartObject
{
public:
	struct FieldReference
	{
		OFRetainedPtr<NSString> name;
		NSInteger tag;
		SEL resourceGetter;
		
		bool operator==(NSString* nameString) const
		{
			return [nameString isEqualToString:name.get()];
		}
	};
	
	typedef std::vector<FieldReference> FieldReferenceSeries;
	
	static OFPointer<OFViewDataMap> fromXml(OFXmlDocument* xmlData);
	
	void addFieldReference(NSString* fieldName, NSInteger viewTag);
	
	UIView* findViewByName(UIView* rootView, NSString* fieldName) const;
	UIView* findViewByTag(UIView* rootView, int tag) const;
	
	FieldReferenceSeries::const_iterator begin() const;
	FieldReferenceSeries::const_iterator end() const;	
	
	bool isValidField(NSString* name) const;
	unsigned int getFieldCount() const;
		
private:
	FieldReferenceSeries mFields;
};
