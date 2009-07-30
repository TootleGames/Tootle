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

class OFOutputSerializer;
class OFViewDataMap;

class OFViewDataGetter
{
public:
	OFViewDataGetter(UIView* rootView, OFViewDataMap* viewData);
	~OFViewDataGetter();

	void serialize(OFOutputSerializer* stream) const;
	
private:
	void serializeView(UIView* namedView, NSString* name, OFOutputSerializer* stream) const;

	typedef void (OFViewDataGetter::*UIViewValueGetter)(UIView* targetView, NSString* name, OFOutputSerializer* stream) const;
	
	struct UITypeAndGetter
	{
		Class uiClassType;
		UIViewValueGetter getter;
	};

#if defined(_UNITTEST)
	static const unsigned int sNumGetters = 5;
#else
	static const unsigned int sNumGetters = 4;
#endif

	static const UITypeAndGetter sAvailableGetters[sNumGetters];

	void writeValueUILabel(UIView* targetView, NSString* name, OFOutputSerializer* stream) const;
	void writeValueUITextField(UIView* targetView, NSString* name, OFOutputSerializer* stream) const;
	void writeValueUITextView(UIView* targetView, NSString* name, OFOutputSerializer* stream) const;

	void writeValueUISwitch(UIView* targetView, NSString* name, OFOutputSerializer* stream) const;
#if defined(_UNITTEST)
	void writeValueUIMockLabel(UIView* targetView, NSString* name, OFOutputSerializer* stream) const;
#endif
	
	OFRetainedPtr<UIView> mRootView;
	OFPointer<OFViewDataMap> mViewData;
};