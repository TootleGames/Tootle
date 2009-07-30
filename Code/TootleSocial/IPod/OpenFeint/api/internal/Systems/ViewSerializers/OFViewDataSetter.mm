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

#import "OFViewDataSetter.h"
#import "OFViewDataMap.h"
#if defined(_UNITTEST)
#import "UIMockLabel.h"
#endif
#import <objc/runtime.h>

const OFViewDataSetter::UITypeAndSetter OFViewDataSetter::sAvailableSetters[OFViewDataSetter::sNumSetters] = 
{
#if defined(_UNITTEST)
	{ [UIMockLabel class],			&OFViewDataSetter::setValueUIMockLabel		},
#endif
	{ [UILabel class],				&OFViewDataSetter::setValueUILabel			}	
};

OFViewDataSetter::OFViewDataSetter(UIView* targetView, OFViewDataMap* fieldMap)
: mTargetView(targetView)
, mFieldMap(fieldMap)
{
}

void OFViewDataSetter::setField(NSString* fieldName, NSString* value)
{
	UIView* targetView = mFieldMap->findViewByName(mTargetView.get(), fieldName);
	if(!targetView)
	{
		OFAssert(0, "View with name %fieldName not found in target views map", fieldName);
		return;
	}
	
	Class targetViewClass = [targetView class];
	UIViewValueSetter setter = NULL;
	for(unsigned int i = 0; i < sNumSetters; ++i)
	{
		if(sAvailableSetters[i].uiClassType == targetViewClass)
		{
			setter = sAvailableSetters[i].setter;
			break;
		}
	} 
	
	if(!setter)
	{
		OFAssert(0, "Attempting to set a value on an unsupported view class (%s)", class_getName(targetViewClass));
		return;
	}
	
	(this->*setter)(targetView, value);
}

void OFViewDataSetter::setValueUILabel(UIView* targetView, NSString* value) const
{
	UILabel* label = static_cast<UILabel*>(targetView);
	label.text = value;
}

#if defined(_UNITTEST)
void OFViewDataSetter::setValueUIMockLabel(UIView* targetView, NSString* value) const
{
	UIMockLabel* label = static_cast<UIMockLabel*>(targetView);
	label.text = value;
}
#endif

bool OFViewDataSetter::isValidField(NSString* fieldName) const
{
	return mFieldMap->isValidField(fieldName);
}
