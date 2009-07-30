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

////////////////////////////////////////////////////////////
///
/// @type		NSNumber bool
/// @default	false
/// @behavior	When false, OpenFeint will attempt to acquire access keys in the background. On success
///				or failure, the user will be notified via a non-interrupting pop-up notification.
///
///				When true, OpenFeint will attempt to acquire access keys in the foreground. If your game
///				has not been authorized by the user, he will be prompted to authorize your game. If a user 
///				does not have an OpenFeint account, he will be required to create one. This authorization process
///				will not be skippable. Once authorization is complete, the OpenFeint dashboard will dismiss itself.
///
///	@warning	Setting this value to true will effectively require a user have an OpenFeint account and be 
///				online when using your game.
///
////////////////////////////////////////////////////////////
extern const NSString* OpenFeintSettingRequireAuthorization;

////////////////////////////////////////////////////////////
///
/// @type		NSNumber UIInterfaceOrientation
/// @default	UIInterfaceOrientationPortrait
/// @behavior	Defines what orientation the OpenFeint dashboard launches in. The dashboard does not auto rotate.
///
////////////////////////////////////////////////////////////
extern const NSString* OpenFeintSettingDashboardOrientation;

/// @param resumeIconFileName is copied. Image is used as the resume button on the nav bar. Will get scaled as needed.

////////////////////////////////////////////////////////////
///
/// @type		NSString 
/// @default	OpenFeintDefaultResumeIcon.png
/// @behavior	Determines what file to use as the icon on the nav bar resume button
///
////////////////////////////////////////////////////////////
extern const NSString* OpenFeintSettingResumeIcon;

////////////////////////////////////////////////////////////
///
/// @type		NSString 
/// @default	Your application icon from iPromote, if provided, otherwise none.
/// @behavior	Determines what file to use as the banner image on the game page in portrait mode
///
////////////////////////////////////////////////////////////
extern const NSString* OpenFeintSettingGamePageBannerPortrait;

////////////////////////////////////////////////////////////
///
/// @type		NSString 
/// @default	Your application icon from iPromote, if provided, otherwise none.
/// @behavior	Determines what file to use as the banner image on the game page in landscape mode
///
////////////////////////////////////////////////////////////
extern const NSString* OpenFeintSettingGamePageBannerLandscape;

////////////////////////////////////////////////////////////
///
/// @type		NSString 
/// @default	Your application's (short) display name.
/// @behavior	Used as the game tab's title
///
////////////////////////////////////////////////////////////
extern const NSString* OpenFeintSettingShortDisplayName;
