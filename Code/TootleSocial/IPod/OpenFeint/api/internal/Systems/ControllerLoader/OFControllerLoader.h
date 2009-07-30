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

class OFControllerLoader
{
public:
	static UIViewController* load(NSString* name, NSObject* filesOwner = nil);
	static UITableViewCell* loadCell(NSString* cellName, NSObject* filesOwner = nil);
	static UIView* loadView(NSString* viewName, NSObject* filesOwner = nil);
	static bool doesControllerExist(NSString* name);
	
	static void replaceMeWith(UIViewController* controllerToReplace, NSString* name);
	static void push(UIViewController* requestingController, NSString* name, BOOL shouldAnimate = YES, BOOL shouldHaveBackButton = YES);
		
	static void setAssetFileSuffix(NSString* suffixString);
	static void setClassNamePrefix(NSString* prefixString);
	
private:
	static UINavigationController* getNavigationController(UIViewController* requestingController);
	static NSString* getControllerNibName(NSString* controllerName);
	static NSString* getControllerClassName(NSString* controllerName);
};