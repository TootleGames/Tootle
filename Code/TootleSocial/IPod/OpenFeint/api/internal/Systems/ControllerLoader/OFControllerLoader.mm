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

#import "OFControllerLoader.h"
#import "OpenFeint+Private.h"
#import <objc/runtime.h>
#import "NSObject+WeakLinking.h"

namespace
{
	NSString* gSuffixString = @"";
	NSString* gClassNamePrefixString = @"";
}

template <typename _T>
static _T* loadControllerFromNib(NSString* nibName, id owner)
{
	if(owner == nil)
	{
		// citron note: This suppresses tons of console spam
		owner = @"";
	}
	
	NSArray* objects = [[NSBundle mainBundle] loadNibNamed:nibName owner:owner options:nil];	
	
	for(unsigned int i = 0; i < [objects count]; ++i)
	{
		NSObject* obj = [objects objectAtIndex:i];
		if([obj isKindOfClass:[_T class]]) 
		{
			return static_cast<_T*>(obj);
		}
	}
	
	return nil;
}

UITableViewCell* OFControllerLoader::loadCell(NSString* cellName, NSObject* filesOwner)
{
	NSString* nibName = [NSString stringWithFormat:@"%@Cell%@", cellName, gSuffixString];
	UITableViewCell* tableCell = loadControllerFromNib<UITableViewCell>(nibName, filesOwner);
	
	if(!tableCell)
	{
		NSString* cellClassName = [NSString stringWithFormat:@"%@%@Cell", gClassNamePrefixString, cellName];
		Class cellClass = (Class)objc_lookUpClass([cellClassName UTF8String]);
		if(cellClass)
		{
			tableCell = (UITableViewCell*)class_createInstance(cellClass, 0);
			
			// 3.0 weak linking compatability
			if ([tableCell respondsToSelector:@selector(initWithStyle:reuseIdentifier:)])
			{
				[tableCell performSelector:@selector(initWithStyle:reuseIdentifier:) withObject:OF_OS_3_ENUM_ARG(UITableViewCellStyleDefault) withObject:cellName]; 
			}
			else
			{
				// The id cast is a hack to get rid of a deprecated function warning
				[(id)tableCell initWithFrame:CGRectZero reuseIdentifier:cellName];
			}
			
			[tableCell autorelease];
			
			SEL setOwner = @selector(setOwner:);
			if([tableCell respondsToSelector:setOwner])
			{
				[tableCell performSelector:setOwner withObject:filesOwner];
			}			
		}

		if(!tableCell)
		{
			OFAssert(0, "Failed trying to load table cell %@ from nib %@", cellName, nibName);
			return nil;
		}
	}
	
	if(![tableCell.reuseIdentifier isEqualToString:cellName])
	{
		OFAssert(0, "Table cell '%@' from nib '%@' has an incorrect reuse identifier. Expected '%@' but was '%@'", cellName, nibName, cellName, tableCell.reuseIdentifier);
	}
		
	return tableCell;
}

UIViewController* OFControllerLoader::load(NSString* name, NSObject* filesOwner)
{
	UIViewController* controller = nil;
	if ([OpenFeint isInLandscapeMode])
	{
		NSString* landscapeNibName = [NSString stringWithFormat:@"%@ControllerLandscape%@", name, gSuffixString];
		controller = loadControllerFromNib<UIViewController>(landscapeNibName, filesOwner);
	}
	
	if (!controller)
	{
		controller = loadControllerFromNib<UIViewController>(OFControllerLoader::getControllerNibName(name), filesOwner);
	}
	
	
	if(!controller)
	{
		Class controllerClass = (Class)objc_lookUpClass([OFControllerLoader::getControllerClassName(name) UTF8String]);
		if(controllerClass)
		{
			controller = (UIViewController*)class_createInstance(controllerClass, 0);
			[controller init];
			[controller autorelease];
		}
	}
	
	OFAssert(controller, "Failed trying to load controller %@", name);
		
	return controller;
}

UIView* OFControllerLoader::loadView(NSString* viewName, NSObject* filesOwner)
{
	UIView* view = nil;
	if ([OpenFeint isInLandscapeMode])
	{
		NSString* landscapeNibName = [NSString stringWithFormat:@"%@Landscape%@", viewName, gSuffixString];
		view = loadControllerFromNib<UIView>(landscapeNibName, filesOwner);
	}
	
	if (!view)
	{
		view = loadControllerFromNib<UIView>([NSString stringWithFormat:@"%@%@", viewName, gSuffixString], filesOwner);
	}	
	
	OFAssert(view, "Failed trying to load view %@", viewName);
		
	return view;
}

bool OFControllerLoader::doesControllerExist(NSString* name)
{
	if ([[NSBundle mainBundle] pathForResource:OFControllerLoader::getControllerNibName(name) ofType:@"nib"])
	{
		return true;
	}
	
	if (objc_lookUpClass([OFControllerLoader::getControllerClassName(name) UTF8String]))
	{
		return true;
	}
	return false;
}

UINavigationController* OFControllerLoader::getNavigationController(UIViewController* requestingController)
{
	UINavigationController* container = nil;
	if([requestingController isKindOfClass:[UINavigationController class]])
	{
		container = (UINavigationController*)requestingController;
	}
	else
	{
		container = [requestingController navigationController];
	}
	
	OFAssert(container, "Attempting to get a navigation controller from a view controller that doesn't have one.");
	
	return container;
}

void OFControllerLoader::push(UIViewController* requestingController, NSString* name, BOOL shouldAnimate, BOOL shouldHaveBackButton)
{
	UIViewController* incomingContoller = load(name);
	incomingContoller.navigationItem.hidesBackButton = !shouldHaveBackButton;	
	[getNavigationController(requestingController) pushViewController:incomingContoller animated:shouldAnimate];
}
	
void OFControllerLoader::replaceMeWith(UIViewController* controllerToReplace, NSString* name)
{
	UIViewController* incomingController = load(name);

	UINavigationController* navController = getNavigationController(controllerToReplace);
	[navController popToRootViewControllerAnimated:NO];
	[navController pushViewController:incomingController animated:NO];
	[navController setNavigationBarHidden:YES animated:NO];
}

NSString* OFControllerLoader::getControllerNibName(NSString* controllerName)
{
	return [NSString stringWithFormat:@"%@Controller%@", controllerName, gSuffixString];
}

NSString* OFControllerLoader::getControllerClassName(NSString* controllerName)
{
	return [NSString stringWithFormat:@"%@%@Controller", gClassNamePrefixString, controllerName];
}

void OFControllerLoader::setAssetFileSuffix(NSString* suffixString)
{
	[gSuffixString release];
	gSuffixString = [suffixString retain];
}

void OFControllerLoader::setClassNamePrefix(NSString* prefixString)
{
	[gClassNamePrefixString release];
	gClassNamePrefixString = [prefixString retain];
}