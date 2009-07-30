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

#include "OFViewHelper.h"

UIView* OFViewHelper::findViewByClass(UIView* rootView, Class viewClass)
{
	if ([rootView isKindOfClass:viewClass])
	{
		return rootView;
	}
	
	for (UIView* view in rootView.subviews)
	{
		UIView* viewOfType = findViewByClass(view, viewClass);
		if(viewOfType != nil)
		{
			return viewOfType;
		}
	}
	
	return nil;
}

bool OFViewHelper::resignFirstResponder(UIView* rootView)
{
	if([rootView isKindOfClass:[UIResponder class]])
	{
		UIResponder* responder = (UIResponder*)rootView;
		if([responder isFirstResponder])
		{
			[responder resignFirstResponder];
			return true;
		}
	}
	
	for(UIView* view in rootView.subviews)
	{
		if(resignFirstResponder(view))
		{
			return true;
		}
	}
	
	return false;
}

UIScrollView* OFViewHelper::findFirstScrollView(UIView* rootView)
{
	if([rootView isKindOfClass:[UIScrollView class]])
	{
		return (UIScrollView*)rootView;
	}
	
	for(UIView* view in rootView.subviews)
	{
		UIScrollView* targetView = findFirstScrollView(view);
		if(targetView != nil)
		{
			return targetView;
		}
	}
			
	return nil;
}

void OFViewHelper::setReturnKeyForAllTextFields(UIReturnKeyType lastKey, UIView* rootView)
{
	unsigned int i = 1;
	UITextField* textField = nil;
		
	while(true)
	{
		UIView* view = [rootView viewWithTag:i];
		++i;
				
		if(!view)
		{
			break;
		}
		
		if(![view isKindOfClass:[UITextField class]])
		{			
			textField = nil;
			continue;
		}
		
		textField = (UITextField*)view;
		textField.returnKeyType = UIReturnKeyNext;
	}
	
	if(textField)
	{
		textField.returnKeyType = lastKey;
	}
}

void OFViewHelper::setAsDelegateForAllTextFields(id<UITextFieldDelegate> delegate, UIView* rootView)
{
	for(UIView* view in rootView.subviews)
	{
		if([view isKindOfClass:[UITextField class]])
		{
			UITextField* textField = (UITextField*)view;

			if(textField.delegate == nil)
			{
				textField.delegate = delegate;
			}
		}
		
		setAsDelegateForAllTextFields(delegate, view);
	}
}

CGSize OFViewHelper::sizeThatFitsTight(UIView* rootView)
{
	CGSize sizeThatFits = CGSizeZero;
	
	for(UIView* view in rootView.subviews)
	{
		float right = view.frame.origin.x + view.frame.size.width;
		float bottom = view.frame.origin.y + view.frame.size.height;
		if(right > sizeThatFits.width)
		{
			sizeThatFits.width = right;
		}
		
		if(bottom > sizeThatFits.height)
		{
			sizeThatFits.height = bottom;
		}		
	}
	
	return sizeThatFits;
}

UIView* OFViewHelper::findViewByTag(UIView* rootView, int targetTag)
{
	if(rootView.tag == targetTag)
	{
		return rootView;
	}

	for(UIView* view in rootView.subviews)
	{
		UIView* targetView = findViewByTag(view, targetTag);
		if(targetView != nil)
		{
			return targetView;
		}
	}
	
	return NULL;
}

void OFViewHelper::enableAllControls(UIView* rootView, bool isEnabled)
{
	if([rootView isKindOfClass:[UIControl class]])
	{
		UIControl* control = (UIControl*)rootView;
		control.enabled = isEnabled;
	}
	
	for(UIView* view in rootView.subviews)
	{
		enableAllControls(view, isEnabled);
	}
}
