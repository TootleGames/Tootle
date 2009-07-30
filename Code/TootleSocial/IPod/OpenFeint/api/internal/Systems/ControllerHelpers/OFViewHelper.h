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

class OFViewHelper
{
public:
	static UIView* findViewByClass(UIView* rootView, Class viewClass);

	static UIView* findViewByTag(UIView* rootView, int targetTag);
	static void setAsDelegateForAllTextFields(id<UITextFieldDelegate> delegate, UIView* rootView);
	static void setReturnKeyForAllTextFields(UIReturnKeyType lastKey, UIView* rootView);
	
	static UIScrollView* findFirstScrollView(UIView* rootView);
	static bool resignFirstResponder(UIView* rootView);
	static void enableAllControls(UIView* rootView, bool isEnabled);
	static CGSize sizeThatFitsTight(UIView* rootView);
};
