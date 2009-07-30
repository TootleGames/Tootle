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

#include "OFViewDataGetter.h"
#include "OFViewDataMap.h"
#include "OFOutputSerializer.h"
#if defined(_UNITTEST)
#include "UIMockLabel.h"
#endif
#include "OFViewHelper.h"
#include <objc/runtime.h>

const OFViewDataGetter::UITypeAndGetter OFViewDataGetter::sAvailableGetters[OFViewDataGetter::sNumGetters] = 
{
#if defined(_UNITTEST)
	{ [UIMockLabel class],			&OFViewDataGetter::writeValueUIMockLabel		},
#endif
	{ [UILabel class],				&OFViewDataGetter::writeValueUILabel			},
	{ [UITextField class],			&OFViewDataGetter::writeValueUITextField		},
	{ [UITextView class],			&OFViewDataGetter::writeValueUITextView		},

	{ [UISwitch class],				&OFViewDataGetter::writeValueUISwitch			}
};

OFViewDataGetter::OFViewDataGetter(UIView* rootView, OFViewDataMap* viewData)
: mRootView(rootView)
, mViewData(viewData)
{
}

OFViewDataGetter::~OFViewDataGetter()
{
}

void OFViewDataGetter::serialize(OFOutputSerializer* stream) const
{
	OFViewDataMap::FieldReferenceSeries::const_iterator it = mViewData->begin();
	OFViewDataMap::FieldReferenceSeries::const_iterator itEnd = mViewData->end();	
	for(; it != itEnd; ++it)
	{
		const OFViewDataMap::FieldReference& field = *it;
		UIView* namedView = OFViewHelper::findViewByTag(mRootView.get(), field.tag);
		serializeView(namedView, field.name, stream);
	}
}

void OFViewDataGetter::serializeView(UIView* namedView, NSString* name, OFOutputSerializer* stream) const
{
	Class targetViewClass = [namedView class];
	UIViewValueGetter getter = NULL;
	for(unsigned int i = 0; i < sNumGetters; ++i)
	{
		if(sAvailableGetters[i].uiClassType == targetViewClass)
		{
			getter = sAvailableGetters[i].getter;
			break;
		}
	} 
	
	if(!getter)
	{
		OFAssert(0, "Attempting to get a value on an unsupported view class (%s)", class_getName(targetViewClass));
		return;
	}
	
	(this->*getter)(namedView, name, stream);	
}

void OFViewDataGetter::writeValueUILabel(UIView* targetView, NSString* name, OFOutputSerializer* stream) const
{
	UILabel* label = (UILabel*)targetView;
	OFRetainedPtr<NSString> text = label.text;
	stream->io([name UTF8String], text);
}

void OFViewDataGetter::writeValueUITextField(UIView* targetView, NSString* name, OFOutputSerializer* stream) const
{
	UITextField* textField = (UITextField*)targetView;
	OFRetainedPtr<NSString> text = textField.text;
	stream->io([name UTF8String], text);	
}


void OFViewDataGetter::writeValueUITextView(UIView* targetView, NSString* name, OFOutputSerializer* stream) const
{
	UITextView* textView = (UITextView*)targetView;
	OFRetainedPtr<NSString> text = textView.text;
	stream->io([name UTF8String], text);	
}

void OFViewDataGetter::writeValueUISwitch(UIView* targetView, NSString* name, OFOutputSerializer* stream) const
{
	UISwitch* toggleSwitch = (UISwitch*)targetView;
	bool value = toggleSwitch.on;
	stream->io([name UTF8String], value);
}

#if defined(_UNITTEST)
void OFViewDataGetter::writeValueUIMockLabel(UIView* targetView, NSString* name, OFOutputSerializer* stream) const
{
	UIMockLabel* label = (UIMockLabel*)targetView;
	OFRetainedPtr<NSString> text = label.text;
	stream->io([name UTF8String], text);
}
#endif