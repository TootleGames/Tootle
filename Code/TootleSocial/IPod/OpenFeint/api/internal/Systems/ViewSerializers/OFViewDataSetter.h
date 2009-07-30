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

#include "OFSmartObject.h"
class OFViewDataMap;

class OFViewDataSetter : public OFSmartObject
{
public:
	OFViewDataSetter(UIView* targetView, OFViewDataMap* fieldMap);
	
	void setField(NSString* fieldName, NSString* value);
	bool isValidField(NSString* fieldName) const;
	
private:
	typedef void (OFViewDataSetter::*UIViewValueSetter)(UIView* targetView, NSString* value) const;
	
	struct UITypeAndSetter
	{
		Class uiClassType;
		UIViewValueSetter setter;
	};

#if defined(_UNITTEST)
	static const unsigned int sNumSetters = 2;
#else
	static const unsigned int sNumSetters = 1;
#endif

	static const UITypeAndSetter sAvailableSetters[sNumSetters];

	void setValueUILabel(UIView* targetView, NSString* value) const;
#if defined(_UNITTEST)
	void setValueUIMockLabel(UIView* targetView, NSString* value) const;
#endif
	
	OFRetainedPtr<UIView> mTargetView;
	OFPointer<OFViewDataMap> mFieldMap;
};