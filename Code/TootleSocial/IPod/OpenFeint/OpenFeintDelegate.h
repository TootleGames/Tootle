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

@protocol OpenFeintDelegate<NSObject>

@optional

////////////////////////////////////////////////////////////
///
/// @note This is where you should pause your game.
///
////////////////////////////////////////////////////////////
- (void)dashboardWillAppear;

////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////
- (void)dashboardDidAppear;

////////////////////////////////////////////////////////////
///
/// @note This is where Cocoa based games should unpause and resume playback. 
///
/// @warning	Since an exit animation will play, this will cause negative performance if your game 
///				is rendering on an EAGLView. For OpenGL games, you should refresh your view once and
///				resume your game in dashboardDidDisappear
///
////////////////////////////////////////////////////////////
- (void)dashboardWillDisappear;

////////////////////////////////////////////////////////////
///
/// @note This is where OpenGL games should unpause and resume playback.
///
////////////////////////////////////////////////////////////
- (void)dashboardDidDisappear;

////////////////////////////////////////////////////////////
///
/// @note This is called whenever the game successfully connects to OpenFeint with a logged in user, not just when a new user logs in
///
////////////////////////////////////////////////////////////
- (void)userLoggedIn:(NSString*)userId;

////////////////////////////////////////////////////////////
///
/// @note Developers can override this method to customize the OpenFeint approval screen. Returning YES will prevent OpenFeint from
///       displaying it's default approval screen. If a developer chooses to override the approval screen they MUST call 
///       [OpenFeint userDidApproveFeint:(BOOL)approved] before OpenFeint will function.
///
////////////////////////////////////////////////////////////
- (BOOL)showCustomOpenFeintApprovalScreen;

@end
