+----------------------------------------------------------------------------+
                             AGON Online 1.2

            Copyright (c) 2009 Aptocore ApS. All Rights Reserved.
+----------------------------------------------------------------------------+

+----------------------------------------------------------------------------+
+++ Getting Started
+----------------------------------------------------------------------------+
There is a sample game included with libagon that you can use as an initial
reference on how to integrate libagon. The sample will build and run,
allowing you to immediately try out AGON Online on the development server.

To get started with your own game you will need to register your company at 
http://devdb.agon-online.com/developer. From here you can register games for use
with AGON Online. Once you have registered a game you should take note of your 
Game Secret and Build a Client Package. The game secret should be given as a
text string to AgonCreate. The client package should be extracted and the
included AgonPackage.bundle should be added to your game's resources in 
XCode. The AgonPackage.bundle must end up in the root of your game's 
.app directory.

You should look in AGON.h to familiarize yourself with the API. AGON.h
and this README currently acts as the primary documentation source. 

To integrate the library into your project you must link against the relevant 
version of libagon.a and include AGON.h as required.

Additionally libagon dependes on the following frameworks/dylibs:
  - AddressBook.framework
  - CoreGraphics.framework
  - CoreLocation.framework
  - Foundation.framework
  - libstdc++.6.dylib (NOTE: Not required for C++/Objective-C++ apps)
  - libsqlite3.dylib
  - QuartzCore.framework
  - Security.framework
  - SystemConfiguration.framework
  - UIKit.framework

The following linker options must be set:
  - Other Linker Flags: -ObjC

You must also add AgonData.bundle to your project's resources and they 
must end up in the root of your game's .app directory.

+----------------------------------------------------------------------------+
+++ Documentation
+----------------------------------------------------------------------------+
The documentation for libagon has been made available as an XCode compatible 
DocSet feed. The DocSet feed link has been made available from the downloads 
section on AGON Online developer administration page: 
http://devdb.agon-online.com/developer/dashboard/downloads

Additionally there are a number of knowledge base articles available from the
AGON developer support site:
http://devsupport.agon-online.com/faqs

+----------------------------------------------------------------------------+
+++ iPhone SDK
+----------------------------------------------------------------------------+
The current version of the library is compiled against SDK version 2.2, 2.2.1 
and 3.0. Please contact us to get a version compiled against another version
of the SDK.

+----------------------------------------------------------------------------+
+++ Changelog
+----------------------------------------------------------------------------+
- 1.2.3
 * [IMPROVED] Now possible to clear your email by entering an empty value
			  on the edit email view under profile edit.
 * [IMPROVED] Profile information is now communicated to the backend via
              HTTPS.
 * [IMPROVED] Changed page transitions during signup to fade rather than curl.
 * [IMPROVED] Now checks for invalid signature during signup on non-production
              servers.
 * [CHANGED] AGON now explicitly hides the status bar and restores it upon exit
			 if the game had it set to visible.
 * [FIXED] Now correctly handles orientation for AGON in games that don't use a
           view controller.
 * [FIXED] Corrected a spelling error on the edit email view (adress -> address).
 * [FIXED] Now handles unknown device orientation in award view controller.

- 1.2.2
 * [IMPROVED] Now supports transparency in leaderboard icons.
 * [IMPROVED] Now possible to pan and zoom when choosing a profile picture.
 * [IMPROVED] Facebook Connect usage inside AGON is now completely isolated.
              This allows the host application to use Facebook Connect as well
              as AGON. Please notice that the FBConnect code is already linked
              into AGON so you do not have to link your application against
              any of the FBConnect code. Your app still needs to be able to
              find the FBConnect headers to compile correctly.
 * [FIXED] Fixed a bug that caused the friend invite badge to be placed on the 
           profile tab when no award tab existed.
 * [FIXED] Fixed a memory leak when changing away from the scores tab.
 * [FIXED] Fixed a crash that occurred if changing tabs while on a leaderboard 
           that was scrolled further down than it had entries.
 * [FIXED] Fixed a UI issue that caused the last leaderboard to be halfway off 
           the screen when viewing all the available leaderboards.
 * [KNOWN ISSUE] Apple's image picker is broken when in landscape mode on devices 
                 running less than OS 3.0. Users will need to rotate to portrait 
                 when choosing their profile picture for it to work properly.

- 1.2.1
 * [FIXED] Fixed a rotation issue going into a leaderboard in landscape and
           navigating back to list of leaderboards in portrait.
 * [FIXED] Fixed UI error when starting in landscape and navigating between 
           Friends and their comments.

- 1.2
 * [NEW] AGON now supports both portrait and landscape and can respond to
         orientation changes.
 * [NEW] Twitter bragging on leaderboards and awards.
 * [NEW] A welcome screen is now shown the first time an app opens up AGON.
         The user can choose whether to go online or not.
 * [IMPROVED] Now possible to set location directly from the nearby-leaderboard 
              as long as your location hasn't been set.
 * [IMPROVED] Profile edit view for better supporting landscape.
 * [IMPROVED] Added AGON Logo and improved buttons and cells for a more 
              consistent look.
 * [IMPROVED] New PocketScore logo.
 * [FIXED] Fixed minor leaks.
 * [FIXED] Bragging to Facebook from the leaderboards now includes full 
           leaderboard path and image. 
 * [FIXED] AGON now shows an alert view if publishing to Facebook fails because
           of connectivity issues.
 * [FIXED] When selecting Friends Tab, the sub-tab for friends is now always 
           properly selected.
 * [FIXED] Now robustly handles cases where the device goes back and forth
           between being offline / online during profile creation.

- 1.1.9
 * [IMPROVED] AGON no longer requires the application to have a UIWindow.

- 1.1.8
 * [FIXED] Rare problem with simulators being unable to access the servers due
           to an invalid signature. If you have been affected by this bug, 
           then your simulator will put you in offline mode the next time you
           open the AGON interface. You click reconnect and the bug should be
           cleared for you. Notice that only simulators were affected by this.
 * [FIXED] Minor memory leak on leaderboards view.
 * [FIXED] Crash when trying to show AGON from within a UIAlertView.

- 1.1.7
 * [FIXED] Leaderboard cells where not refreshed for display, resulting in the
           first 8 cells showing repeatedly.

- 1.1.6
 * [FIXED] Freezing issue when trying to brag about an award.
 * [FIXED] Graphics corruption when Facebook dialogs appeared - OS 3.0 issue.

- 1.1.5
 * [FIXED] Bug where AGON would crash when trying to reset awards. The 
           precondition for this bug involved deleting an award on the backend
           that had already been earned by a player and then trying to reset
           awards for this player.

- 1.1.4
 * [IMPROVED] Added support for working with awards based on id or index.
              Awards must always be unlocked by ID. The following API 
              methods have been renamed:
               * AgonGetAwardPocketScoreAtIndex -> AgonGetAwardPocketScoreWithIndex
               * AgonGetAwardTitleAtIndex -> AgonGetAwardTitleWithIndex
               * AgonGetAwardDescriptionAtIndex -> AgonGetAwardDescriptionWithIndex
               * AgonGetAwardImageAtIndex -> AgonGetAwardImageWithIndex
               * AgonIsAwardUnlocked -> AgonIsAwardWithIdUnlocked
               * AgonUnlockAwardAtIndex -> AgonUnlockAwardWithId
              The following new methods have been introduced to support working
              with awards based on their ids:
               * AgonGetAwardPocketScoreWithId
               * AgonGetAwardTitleWithId
               * AgonGetAwardDescriptionWithId
               * AgonGetAwardImageWithId
              NOTE: To make your code less error prone we recommend that you
              only use the index based award functions when you are working
              with awards as a collection. E.g. to display a list in-game
              of available awards.
 * [IMPROVED] I Feel Lucky sample was extended to illustrate one way of 
              working with awards and leaderboards based on id.              
 * [FIXED] Problem with award ids being treated as indexes internally in AGON.
 * [FIXED] When trying to open a friend's latest event before while the
           remaining events were loading AGON would crash.

- 1.1.3
 * [FIXED] Moved I Feel Lucky sample to developer server.

- 1.1.2
 * [NEW] AGON is now provided compiled against SDK 2.2, 2.2.1 and 3.0.
 * [NEW] Added support for secret awards. These can now be setup from the
         developer interface at http://devdb.agon-online.com/developer.
 * [IMPROVED] Landscape support has been improved to fix problem with onscreen
              keyboard. Your game's view controllers must now use the new
              API function AgonShouldAutorotateToInterfaceOrientation to
              indicate whether or not to autorotate. Additionally AGON always
              sets the status bar orientation to UIInterfaceOrientationPortrait.
              This is also to ensure that the onscreen keyboards behaves
              correctly. 
 * [IMPROVED] Leaderboards score loading performance (bandwidth usage).
 * [IMPROVED] Better resistance to high backend load.
 * [IMPROVED] User feedback during registration.
 * [IMPROVED] User feedback when modifying profile information.
 * [FIXED] Alias was not used for own score on leaderboards.
 * [FIXED] Bug with some game icons showing a green stripe in the tabbar.
 * [FIXED] OS 3.0 compatibility issues regarding posting of comments.

- 1.1.1
 * [NEW] The I Feel Lucky sample now supports landscape mode.
 * [IMPROVED] Better transitions when opening and closing the AGON interface.
 * [IMPROVED] No longer asserts when the local client package is not in sync  
              with the state on the server (e.g. achievements count changed).
 * [IMPROVED] Now possible to toggle retain count assertions in libagon using
              the new API function AgonEnableRetainCountAsserts. Disabled by
              default, since not all games use a standard event loop, causing
              unexpected retain counts and thus false assertions.
 * [IMPROVED] Finding the user's current position in the leaderboards now only
              queries the server when actually necessary.
 * [IMPROVED] Added return values to AgonShow and AgonHide indicating whether
              they could be performed. They will return NO if the interface is
              already shown or hidden, respectively.
 * [IMPROVED] Now displays alias for received invites.
 * [IMPROVED] Now only loads awards state once per game session.
 * [IMPROVED] Now displays a small overlay above the tabbar when not running 
              on the production servers.
 * [IMPROVED] Leaderboards score loading performance (bandwidth usage).
 * [FIXED] When a user started AGON for the first time AGON would go to the
           leaderboards overview, instead of the specified leaderboard.
 * [FIXED] When loading friends, the load indicator would sometimes disappear
           before the friend list had completed loading.
 * [FIXED] When an application was running in landscape mode and the AGON 
           interface was started, the UI did not behave as expected.
 * [FIXED] Unicode bug when scraping for friends.

- 1.1
 * [NEW] Added support for defining multiple leaderboards from the developer
         interface at http://devdb.agon-online.com/developer. It is now 
         required that at least one leaderboard is defined on the backend to 
         submit any highscores.
 * [NEW] Added support for having multiple leaderboards within the AGON UI.
         This means that the API has changed to accept a leaderboard ID in
         AgonSubmitScore, AgonGetActiveProfileBestScore and 
         AgonGetActiveProfileBestDisplayScore. Additionally a new function
         has been added to show AGON with a specific leaderboard selected:
         AgonShowLeaderboard.
 * [NEW] Added AgonGetActiveProfileBestScore to get the integer score value
         of the user's best score.
 * [NEW] Now replaces the AGON title with the name of the server when not 
         on the production server. This is to help make sure that a game is
         not submitted to the app store with settings for the development
         server.
 * [NEW] AGON now retains the callback target provided by the game in 
         AgonShow.
 * [NEW] Extended the "I Feel Lucky" sample game with a more interesting way 
         for queueing and displaying awards.
 * [NEW] Extended the "I Feel Lucky" sample game with multiple leaderboards.
 * [NEW] Added a web section on the developer site with an example of the 
         type of web integration that will become possible with AGON.
 * [CHANGED] Logging now defaults to being off. To enable logging from within
             AGON you should call AgonShowLogs with YES.
 * [CHANGED] Renamed AgonBladeHighscore enum to AgonBladeLeaderboards.
 * [CHANGED] Now has an explicit ordering concept on awards on the developer
             interface.
 * [IMPROVED] No longer uses the applications Icon.png for the tab bar. 
              Instead it uses the app icon from the AgonPackage.bundle file.              
 * [IMPROVED] Activity indicator now shown when accepting or revoking friend
              requests.
 * [IMPROVED] Back button on detail friends view is now a pointy button.
 * [IMPROVED] Many optimizations on backend code, that should improve the 
              response time on many operations.
 * [FIXED] Bug that caused the screen to go blank when AGON went offline just
           after opening the friend detail view.
 * [FIXED] Bug where accepting a friend request, then changing to a different 
           tab and returning to the friends tab showed an empty friends list.
 * [FIXED] Bug that caused the "New Comment" view to not accept touches, 
           making it impossible to move the cursor and cancel auto-completion 
           suggestions
 * [FIXED] Memcache bug on backend that would, at times, cause a score to
           be missing in a leaderboard.

- 1.0.7
 * [NEW] It is now possible to clear the set location on the profile tab.
 * [IMPROVED] Error messages shown, when unable to get online user name.
 * [FIXED] Images could be incorrect on friend cells on the friends view.
 * [FIXED] User registration could be confusing if entering an already 
           existing user name or an invalid user name.
 * [FIXED] When editing the profile and tabbing away from the editing view,
           without having pressed DONE, the profile would be saved. This
           was not intended. Now any editing made is reverted/cancelled.
 * [FIXED] The UI would lock up when disallowing access to current position
           when editing profile.
 * [FIXED] Entering invalid information on the profile view would throw
           the user offline. E.g. invalid user name.
 * [FIXED] No longer possible to interact with AGON when Facebook dialogs
           are showing. This would cause a crash if the user managed to
           press the "back to game" tab.
 * [FIXED] No longer possible to select external link in Facebook dialogs.
 * [FIXED] Minor memory leaks on profile creation page.
 * [FIXED] Now possible to enter an offline user name.
 * [FIXED] Problem with location information being dropped when saving profile.
 * [FIXED] Data and time is now properly localized where used.
 * [FIXED] Fixed rare bug with post requests made to the server.
 * [FIXED] Problem with location determination failing due to timeout when
           the user has interactively to allow core location to run.

- 1.0.6
 * [NEW] Documentation has been made available as a DocSet from 
         http://devdb.agon-online.com/developer/dashboard/downloads
 * [NEW] Introduced ability to reset awards from client on developer server.
 * [CHANGED] Facebook one line story updated.
 * [CHANGED] Renamed AgonSetBackgroundTint1 to AgonSetStartBackgroundTint
 * [CHANGED] Renamed AgonSetBackgroundTint2 to AgonSetEndBackgroundTint
 * [IMPROVED] A new best highscore should now only result in a single event.
 * [IMPROVED] Now displays an alertview when libagon thinks that you might
              have provided an incorrect game secret for the current 
              AgonPackage.bundle.
 * [IMPROVED] Improved log messages given during initial start up to better 
              indicate first time creation of local db.
 * [IMPROVED] Now detects server changes and resets local database when such 
              a change occurs. E.g. from developer to production server.
 
- 1.0.5
 * [NEW] Added support for disabling logging - void AgonShowLogs(BOOL showLogs);
 * [NEW] Custom awards can be created at http://devdb.agon-online.com/developer
 * [CHANGED] Achievements renamed to Awards.
 * [IMPROVED] AgonCreate now only takes a game secret. The game secret can be found
              on the game's general tab at http://devdb.agon-online.com/developer
 * [IMPROVED] libagon now expects to be able to find an AgonPackage.bundle inside 
              the root of the game's .app folder. AgonPackage bundles can be 
              generated from http://devdb.agon-online.com/developer
 * [IMPROVED] Optimized score submission.
 * [IMPROVED] Improvements to I Feel Lucky sample game.
 * [IMPROVED] Awards blade is only shown if the game has actually defined some 
              awards.
 * [FIXED] Bug with activity indicator on friends view
 * [FIXED] Problem with Facebook publishing in a user's first AGON session.
 * [FIXED] Minor bug with inviting friends based on AGON username or email
 * [FIXED] Minor bug with location detection on profile view.
 
- 1.0.4
 * [FIXED] The development server secure address was pointing to the wrong
           server. This meant that it was impossible to get a device registered 
           on the server.
 
- 1.0.3
 * [CHANGED] C/Objective-C applications must explicitly link against 
             libstdc++.6.dylib
 * [CHANGED] I Feel Lucky sample has been updated to pure Objective-C.
 * [IMPROVED] No longer forces the host application to use C++/Objective-C++ 
              linkage.
- 1.0.2
 * [IMPROVED] Improved I Feel Lucky sample comments.
 
- 1.0.1
 * [NEW] AGON Online Facebook is now live (publishing now works for everybody)
 * [NEW] I Feel Lucky sample game added.
 * [FIXED] Minor friends view bug fixes.
 * [FIXED] Bug fix for event cell images being incorrect.
 
- 1.0.0
 * [NEW] Initial libagon release.

+----------------------------------------------------------------------------+
+++ Version numbering
+----------------------------------------------------------------------------+
The version numbering scheme used by libagon is best described from an 
example. The following is an example of a libagon release zip filename:

libagon1.1.2-0-SDK2.2-g19bf1f19

The above filename can be broken into the following components:
1.1.2: Humanized release number - major.minor.bugfix
0: Number of commits since last release. 0 for stable releases.
SDK2.2: The SDK that libagon was compiled against.
g19bf1f19: Short ID of the changelist that generated the release.

+----------------------------------------------------------------------------+
+++ Support
+----------------------------------------------------------------------------+
We have set up professional developer support at:

http://devsupport.agon-online.com/

If you have any other questions or comments feel free to write us at:

developer@agon-online.com

We are continuously improving Agon based on your feedback, so:

Have your say - write to us!
