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

#import "OFInputResponsePushController.h"
#import "OpenFeint.h"
#import "OFControllerLoader.h"

@implementation OFInputResponsePushController

- (id)initWithControllerName:(NSString*)controllerName andNavController:(UINavigationController*)navController andShowAsModal:(BOOL)showAsModal
{
	self = [super init];
	if (self)
	{
		mControllerName = [controllerName retain];
		mNavController = navController;
		mShowAsModal = showAsModal;
	}
	return self;
}

+ (OFNotificationInputResponse*)responseWithControllerName:(NSString*)controllerName 
										  andNavController:(UINavigationController*)navController
											andShowAsModal:(BOOL)showAsModal
{
	return [[[OFInputResponsePushController alloc] initWithControllerName:controllerName andNavController:navController andShowAsModal:showAsModal] autorelease];
}

- (void)respondToInput
{
	UIViewController* controllerToPush = OFControllerLoader::load(mControllerName);
	if (controllerToPush)
	{
		if (mShowAsModal)
		{
			[mNavController presentModalViewController:controllerToPush animated:YES];
		}
		else
		{
			[mNavController pushViewController:controllerToPush animated:YES];
		}
	}
}

- (void)dealloc
{
	OFSafeRelease(mControllerName);
	[super dealloc];
}
@end
