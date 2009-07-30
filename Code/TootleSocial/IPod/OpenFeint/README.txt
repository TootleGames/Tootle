********************************************************************************
********************************************************************************
********************************************************************************
********************************************************************************

                             OpenFeint 2.0d

                             build 7.16.2009

                              Release Notes

          Copyright (c) 2009 Aurora Feint Inc. All Rights Reserved.

********************************************************************************

********************************************************************************
********************************************************************************
********************************************************************************
********************************************************************************



********************************************************************************
**** What's New? (7.16.2009)
********************************************************************************
- Improved OpenFeint "Introduction flow"
- User may set their name when first getting an account
- User may at any time import friends from Twitter or Facebook
- Nicer landing page in the dashboard encouraging you to import friends until you have some
- Fixed compatibility issues with using the 3.0 base sdk and 2.x deployment targets


********************************************************************************
**** Getting Started
********************************************************************************
---------------------------------------------
---- Building OpenFeint With Your Project:
---------------------------------------------
- If you have previously used OpenFeint, delete the existing group reference from your project
- Drag and drop the root folder onto your project in XCode. Make sure it's included as a group and not a folder reference.
- With "All Configurations" add to "Other Linker Flags" the value "-ObjC"
- With "All Configurations" set the user defined setting "GCC_OBJC_CALL_CXX_CDTORS" to "YES"
- Ensure the following frameworks are included in your link step: Foundation, UIKit, CoreGraphics, QuartzCore, Security, SystemConfiguration, libsqlite3.0.dylib
- You must have a prefix header. It must have the following line:
	#import "OpenFeintPrefix.pch"
	

---------------------------------------------
---- Releasing your title with OpenFeint:
---------------------------------------------	
- Register an Application on api.openfeint.com
- Use the ProductKey and ProductSecret for your registered application.
  NOTE: If you got keys from beta.openfeint.com, discard them and get new ones from api.openfeint.com
- When launching your app, OpenFeint will print out what servers it is using to the console/log using NSLog. 
  NOTE: Make sure your application is using https://api.openfeint.com/


---------------------------------------------
---- How To Use OpenFeint
---------------------------------------------
#import "OpenFeint.h"

// Initialize OpenFeint on the title screen after you've displayed any splash screens. OpenFeint will present a modal the first time it's 
// initialized to conform with apple regulations.

- (void)initializeOpenfeint
{       
    [OpenFeint initializeWithProductKey:yourProductKey
                              andSecret:yourProductSecret
                         andDisplayName:yourApplicationNameForUsers
                            andSettings:aDictionaryOfOpenFeintSettings    // see OpenFeintSettings.h
                            andDelegate:self];                            // see OpenFeintDelegate.h
                           
    // You probably want to invoke this from a button instead of directly here.
    [OpenFeint launchDashboard];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	[OpenFeint applicationDidBecomeActive];
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	[OpenFeint applicationWillResignActive];
}



********************************************************************************
**** Known Issues
********************************************************************************
- Documentation is sorely lacking. Will be coming with OpenFeint 2.0 final release.
- Delegate parameters from OF Services are not easily exposed yet.
- You must be compiling with Objective-C++ to use the OpenFeint APIs. (Use a .mm file extension)
- There are a small number of non-prefixed symbols in the global namespace.
- Notifications do not perform well on top of OpenGL based game views.
- Occasional non-error logging spew needs to scoped with OpenFeint and togglable
- When resizing the keyboard in chat, the text pops up or down 40 pixels before animating.


********************************************************************************
**** Changelog
********************************************************************************

---------------------------------------------
---- Version 6.29.2009 (2.0)
---------------------------------------------
- Friends:
- 	A player can import friends from twitter and facebook:
- 	A player can see all of his or her friends in one place:
- Feint Library:
-	A player can see all the games they've played in once place
- Social Player Profiles:
- 	A player can see the name and avatar of the profile owner:
- 	A player can see all the games the profile owner has played:
- 	A player can see all the friends the profile owner has:
- Achievements:
- 	A developer can add up to 100 achievements to a game:
- 	Each player has a gamerscore and earns points when unlocking achievements:
- 	Achievements can be compared between friends for a particular game:
- 	If you do not have any achievements to be compared, there is an iPromote Page link with a call to action prominantly visible
- 	Achievements can be unlocked by the game client when on or offline:
- 	Achievements unlocked offline are syncronized when next online:
- Friend Leaderboards:
- 	A leaderboard can be sorted by friends:
- 	Player avatars are visible on the leaderboard:
- Chat Room:
- 	Each chat message has a player's profile avatar next to it:
- 	Each chat message has some kind of visual representation of the game they are using:
- 	Clicking on a person's chat message takes you to their profile:
- Chat Room Moderation:
- 	A player report can optionally include a reason:
- 	A player can click "report this user" on a player's profile:
- 	A developer can give Moderator privileges to up to 5 users from the dashboard:
- 	When a player has been flagged more than a certain number of times, they are not allowed to chat for a relative amount of time:
- 	Moderators reporting a user immediately flags them:
- Fixed iPhone SDK 3.0 compatibility issues
- Lots of bugfixes
- Lots of user interface changes
- Lots of Perforamnce improvements
- Fixed compatibility with iPod Music Picker
- Fixed glitch visual glitch in landscape when running on a 2.0 device and building with the 3.0 SDK

---------------------------------------------
---- Version 5.29.2009 (1.7)
---------------------------------------------
- Simplified account setup
- Users can access OpenFeint without setting up an account
- Login is only required once per device instead of per app
- 3.0 compatibility fixes
- Various bug fixes

---------------------------------------------
---- Version 5.22.2009 (1.7)
---------------------------------------------
- Simplified account setup
- Users can access OpenFeint without setting up an account
- Login is only required once per device instead of per app
- 3.0 compatibility fixes
- Various bug fixes

---------------------------------------------
---- Version 5.13.2009 (1.6b)
---------------------------------------------
- OpenFeint works properly on 3.0 devices.

---------------------------------------------
---- Version 4.29.2009 (1.6)
---------------------------------------------
- Dashboard now supports landscape (interface orientation is a setting when initializing OF).
- OpenFeint can now be compiled against any iPhone SDK version
- Various minor bug-fixes

---------------------------------------------
---- Version 4.21.2009 (1.5)
---------------------------------------------
- One Touch iPromote
- Keyboard can now be toggled in the chat rooms
- Greatly improved performance and memory usage of chat rooms
- Profanity Filter is now even more clean.
- Massive scale improvements
- Improved internal analytics for tracking OF usage
- User conversion rate tracking (view, buy, return)
- Various minor bug-fixes

---------------------------------------------
---- Version 3.26.2009 (1.0)
---------------------------------------------
- Users can login with their Facebook accounts (using FBConnect)
- Every user now has proper account "settings"
- Global "publishing" permissions are now present on account creation screens
- Chat scrolling now works properly in 2.0, 2.1, 2.2, and 2.2.1.
- DashboardDidAppear delegate implemented by request


---------------------------------------------
---- Version 3.20.2009
---------------------------------------------
- Users can login with other account containers (twitter)
- Added global, developer, and game lobbies
- Developer and game rooms can be configured from developer website
- Account error handling improved
- Polling system improvements: remote throttling, disabled when device locks
- Improved versioning support
- Leaderboard values can be 64 bit integers (requested feature!)
- Removed profile screens
- Added Settings tab with Logout button
- Final tab organization and art integration
- Lots of minor bug fixes and tweaks

---------------------------------------------
---- Version 3.15.2009
---------------------------------------------
- Out of dashboard background notifications
- Multiple leaderboards for each title (configurable via web site)
- Landscape keyboard issue addressed
- Startup time significantly reduced
- Multi-threaded API calls now work properly
- Added profanity filter to server
- Basic request based version tracking
- Now using HTTPS for all data communication

---------------------------------------------
---- Version 3.10.2009
---------------------------------------------
- Robust connectivity and server error handling
- Integration protocol no longer requires all callbacks
- Various Bugfixes

---------------------------------------------
---- Version 3.6.2009
---------------------------------------------
- Each game has a dedicated chat room
- First implementation of background alerts
- Framework preparation for future features
- Framework enhancements for table views

---------------------------------------------
---- Version 3.3.2009
---------------------------------------------
- First pass at Leaderboards ("Global" and "Near You")
- Tabbed Dashboard with temporary icons
- OFHighScore API for setting high score
- OpenFeintDelegate now works
- OpenFeint api changed to allow a per-dashboard delegate
- Automatically prompt to setup account before submitting requests
- Placeholder in-game alerts
- Better offline and error support
- Smaller library size (AuroraLib has been mostly removed)

---------------------------------------------
---- Version 2.25.2009
---------------------------------------------
- First draft public API
- Placeholder profile
- Placeholder Dashboard
- Account create, login, and logout 


