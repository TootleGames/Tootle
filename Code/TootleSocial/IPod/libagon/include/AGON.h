/**
 \file AGON.h
 
 \brief Header for the AGON Online client library.
*/
#ifndef AGON_H
#define AGON_H

/** 
 \mainpage AGON Online Library
 \section intro_sec Introduction
 
 This is the documentation set for the AGON Online Library. The AGON Online Library is very simple, 
 yet it provides very powerful additional functionality for your game. To get started with integrating 
 AGON Online into your game is a simple matter of registering your game on http://devdb.agon-online.com,
 including the library into your project and then including the required 5 lines of code. A sample
 project is included with the library to make it even easier for you to see how the integration can
 be done. You can find the library changelog and library download links at the bottom of this page.
 
 \section overview_sec Technical Overview
 
 AGON Online consists of a client frontend library and a server backend. The client fronted is what
 you get to integrate into your game. The client speaks using requests with the server backend, which
 is running on Google App Engine. Google App Engine enables request based applications to run on 
 Google's infrastructure. This infrastructure provides us with an automatically scaling backend, where
 we do not need to spend energy on the inherent server scalability problems that comes with providing
 a social network for games to a growing potential user base of over 37 million users.
 
 \image html diagram_small.png 
 <CENTER>\ref diagram_page "Click to see large diagram"</CENTER>
 
 \section downloads_sec Downloads
 
 All availabled downloads can be found from the downloads section on the AGON Online developer portal:\n
 
 http://devdb.agon-online.com/developer/dashboard/downloads
 
 \section other_docs_sec Other Documentation
 
 <a href="http://devsupport.agon-online.com/faqs/documentation/add-integrate-and-publish-an-agon-enabled-game">Add, Integrate and Publish an AGON enabled game</a><br/>
 <a href="http://devsupport.agon-online.com/faqs/documentation/agon-online-leaderboards">AGON Online leaderboards</a><br/>
 <a href="http://devsupport.agon-online.com/faqs/documentation/agon-online-awards">AGON Online awards</a><br/>
 
 \section changelog_sec Changelog
 
 \verbatim
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
           to an invalid signature. If your simulator starts receiving invalid
           signature message boxes please write to us at 
           developer@agon-online.com
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
 * [IMPROVED] AgonCreate now only takes a game secret. The game secret can be 
              found on the game's general tab at http://devdb.agon-online.com/developer
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
 \endverbatim
 
 \section support_sec Support
 
 We have set up professional developer support at:\n
 
 http://devsupport.agon-online.com/\n
 
 If you have any other questions or comments feel free to write us at:\n
 
 developer@agon-online.com\n
 
 We are continuously improving AGON based on your feedback, so:\n
 
 Have your say - write to us!
 
 - \subpage diagram_page
*/

/**
 \page diagram_page AGON Online Diagram
 
 \image html diagram.png
 
*/

/**
 \brief AGON Online client interface blade types. 
 
 Each blade enum corresponds to a tab in the AGON Online client interface.
 
 \see AgonShow
 */
typedef enum {
	AgonBladeLeaderboards, /**< Enum representing the leaderboards blade. */
	AgonBladeAwards, /**< Enum representing the awards blade */
	AgonBladeFriends, /**< Enum representing the friends blade */
	AgonBladeProfile, /**< Enum representing the profile blade */
} AgonBlade;

typedef enum {
	kLeaderboardRootId = -1,
} AgonConstants;

#ifdef __OBJC__ 

#import <objc/objc.h>
#import <UIKit/UIApplication.h>

@class NSString;
@class UIViewController;
@class UIColor;
@class UIImage;
@class NSNumber;

#ifdef __cplusplus
extern "C" {
#endif

/** 
 \example LuckyAppDelegate.m
 Example code from the included I Feel Lucky sample showing creation, configuration and destruction of libagon.
 */
	
/**
 \example LuckyViewController.m
 Example code from the included I Feel Lucky sample showing submission of scores, unlocking of awards and 
 general AGON Online game integration.
 */
	
/**
 \example AwardViewController.m
 Example code from the included I Feel Lucky sample showing a simple example of how to display awards that have been unlocked.
 Unlocked awards are stored in a queue and showed using a small UIView dropping in from the top of the screen.
 You are free to use and modify that example code if it is useful for your scenario.
 */
	
/**
 \brief Create the AGON Online client library.
 
 This function should be called once for a game upon initilization. This will
 initialize the internal state of the library allowing calls to other functions in
 the library. This function assumes that it can find the game's bundle package
 <b>(AgonPackge.bundle)</b>, in the root of the game's .app directory. 
 
 Attempting to call any of the other AGON functions before this function 
 has been called will fail, except for AgonShowLogs.
 
 \param gameSecret Game secret for the current <b>AgonPackage.bundle</b>.
 \return YES if the <b>AgonPackage.bundle</b> was found in the game's .app directory.
 \n NO if no <b>AgonPackage.bundle</b> is present in the game's .app directory.

 \see AgonDestroy
*/
BOOL AgonCreate(NSString* gameSecret);
	
/**
 \brief Destroy the AGON Online client library.
 
 This function should be called once for a game when shutting down to free any
 resources held internally by the library.
 
 \see AgonCreate
*/
void AgonDestroy();

	
/**
 \brief Toggles whether logs from AGON are output to the console.
 
 Call before calling #AgonCreate to include / exclude logs from within #AgonCreate.
 Useful for providing info when submitting support requests.
 
 \param showLogs YES if logs should be output, NO if not.
*/
void AgonShowLogs(BOOL showLogs);
	
	
/**
 \brief AGON has internal assertions in debug mode on retain counts allowing detection of
 unexpected retain counts. Retain count assertions are disabled by default. Retain count
 assertions are only valid if you use normal event loop processing. If you are not doing
 normal event loop processing then the event loop might not be given enough time to 
 process all events, which means that the retain counts might not have the expected value.
 This function has no effect in the release build of libagon.
 
 \param enabled YES enables retain count asserts. NO disables retain count asserts.
 */
void AgonEnableRetainCountAsserts(BOOL enabled);	
	
/**
 \brief Show the AGON Online client interface.
 
 When the client interface is shown the user is presented with a tab view with easy access
 to all of the functionality within the AGON Online client. The user exits the interface
 by pressing the game icon on the tab furthest to the right. Internally the interface is
 shown by creating a UIWindow for the AGON interface and making that window key and visible.
 The interface is animated in similarly to using presentModalViewController.
 
 \b NOTICE: If your game doesn't use a view controller for the view it adds to the key window
 you need to manually set the status bar orientation ([UIApplication sharedApplication].statusBarOrientation)
 before showing the AGON interface. For example in your applicationDidFinishLaunching callback.
 If your device hasn't been manually rotated after your game has been loaded it will report that its
 orientation is unknown (until it has been rotated for the first time) and AGON will adhere to the
 status bar orientation if shown. If you want your game locked to a certain status bar orientation
 you should set the status bar orientation manually in our callback action as it might have been
 changed while inside AGON in response to the user rotating the device.
 
 Showing the interface will submit any unsubmitted highscores and unlocked awards to the server.
 
 \param callbackTarget An optional parameter for an object to give a callback when 
 AGON returns control to the game. AGON retains this object while the interface showing.
 \param callbackAction Optional selector to call on the target when AGON returns  
 control to the game.
 \param blade The blade (tab) you wish the AGON interface to start up in.
 \return YES if the interface will be shown. NO is returned otherwise, indicating that the 
 interface is already showing.
 
 \see AgonHide
 \see AgonBlade
*/
BOOL AgonShow(id callbackTarget, SEL callbackAction, AgonBlade blade);

/**
 \brief Show the AGON Online leaderboard interface.
 
 Shows the client interface and takes the user directly to the leaderboard identified by
 the \a leaderboardId.
 
 When the client interface is shown the user is presented with a tab view with easy access
 to all of the functionality within the AGON Online client. The user exits the interface
 by pressing the game icon on the tab furthest to the right. Internally the interface is
 shown by creating a UIWindow for the AGON interface and making that window key and visible.
 The interface is animated in similarly to using presentModalViewController.
 
 \b NOTICE: If your game doesn't use a view controller for the view it adds to the key window
 you need to manually set the status bar orientation ([UIApplication sharedApplication].statusBarOrientation)
 before showing the AGON interface. For example in your applicationDidFinishLaunching callback.
 If your device hasn't been manually rotated after your game has been loaded it will report that its
 orientation is unknown (until it has been rotated for the first time) and AGON will adhere to the
 status bar orientation if shown. If you want your game locked to a certain status bar orientation
 you should set the status bar orientation manually in our callback action as it might have been
 changed while inside AGON in response to the user rotating the device.
 
 Showing the interface will submit any unsubmitted highscores and unlocked awards to the server.
 
 \param callbackTarget An optional parameter for an object to give a callback when 
 AGON returns control to the game. AGON retains this object while the interface is showing.
 \param callbackAction Optional selector to call on the target when AGON returns  
 control to the game.
 \param leaderboardId The id of the leaderboard you wish the AGON interface to start up in.
 \return YES if the interface will be shown. NO is returned otherwise, indicating that the 
 interface is already showing.
 
 \see AgonHide
 */
BOOL AgonShowLeaderboard(id callbackTarget, SEL callbackAction, int leaderboardId);	
	
/**
 \brief Hide the AGON Online client interface.
 
 Hides the AGON Online client interface. This will typically never be called from the
 game, since the user can manually hide the interface from inside of AGON. The internal
 UIWindow is released and the original window is made key and visible again.
 
 \param animated: Whether the blade should animate when hiding.
 \return YES if the interface will be hidden. NO is returned otherwise, indicating that  
 the interface is already hidden or hiding. 
 \see AgonShow
*/
BOOL AgonHide(BOOL animated);

/**
 \brief Is the AGON Online client interface showing.
 
 \return YES if the interface is showing. NO if the interface is not showing.
 */
BOOL AgonIsShown();

/**
 \brief Use this function in your view controllers' shouldAutorotateToInterfaceOrientation
 
 If your application uses view controllers that implements shouldAutorotateToInterfaceOrientation,
 you should use this to ensure that AGON client interface can respond to rotation notifications
 and change between portrait and landscape dynamically. When AGON is not shown
 all of the orientation provided to this function will be returned as supported.
 
 \param supportedOrientations: Array of orientations supported by your application.
 \param numSupportedOrientations: Number of entries in the supportedOrientations array.
 \param interfaceOrientation: The interface orientation from shouldAutorotateToInterfaceOrientation.
 */	
BOOL AgonShouldAutorotateToInterfaceOrientation(UIInterfaceOrientation* supportedOrientations, int numSupportedOrientations, UIInterfaceOrientation interfaceOrientation);
	
	
/**
 \brief Sets the start color of the background gradient.
 
 The background is painted using a linear gradient. This provides
 a means for basic configuration of the AGON Online client interface.
 
 \param color The start color of the background gradient.
 
 \see AgonSetEndBackgroundTint
*/
void AgonSetStartBackgroundTint(UIColor* color);
	
/**
 \brief Sets the end color of the background gradient.
 
 The background is painted using a linear gradient. This provides
 a means for basic configuration of the AGON Online client interface.
 
 \param color The end color of the background gradient.
 
 \see AgonSetStartBackgroundTint
 */
void AgonSetEndBackgroundTint(UIColor* color);

/**
 \brief Submit a new score to the AGON client.
 
 The submitted score will be submitted to the backend server the next time
 the AGON Online client interface is shown (#AgonShow). 
 
 Higher scores are always better than lower scores in AGON. This means that
 if you have a game where it is better to get a lower score than a higher one, 
 you need to perform the transformation of the submitted score. E.g. a racing 
 game would typically have highscores based on the time it takes to race a
 number of laps. Here lower times are better than higher times. To make AGON 
 show lower times as the better times, the following transformation can be used 
 \code int score = 0x7fffffff-lapTime; \endcode What will actually be shown in 
 the leaderboard is dictated by the supplied scoreMetaData string (e.g. "1:23:456").
 
 \b NOTICE: Apple might require you to notify the user before submitting scores online.
 All though the score is not actually submitted online when this method is called
 then it will be submitted the next time the AGON interface is shown, so you might
 want to notify the user before calling this function. However, Apple is not 
 consistent in their Technical Requirements so you might not be required to do this.
 
 \param score The score to submit. Higher is always better.
 \param scoreMetaData The string to display in the leaderboard for the supplied score.
 \param leaderboardId The leaderboard id to submit the score to.
*/
void AgonSubmitScore(int score, NSString* scoreMetaData, int leaderboardId);

/**
 \brief Get the active profile's best local score.
 
 \param leaderboardId The leaderboard id to retrieve the score for.
 \return The user's best local score. Use intValue to get the value. Nil when no best score.
 
 \see AgonSubmitScore
 */	
NSNumber* AgonGetActiveProfileBestScore(int leaderboardId);
	
/**
 \brief Get display string of the active profile's best local score.
 
 \param leaderboardId The leaderboard id to retrieve the score for.
 \return Meta data string for the user's best score. Nil when no best score.
 
 \see AgonSubmitScore
*/
NSString* AgonGetActiveProfileBestDisplayScore(int leaderboardId);
	
/**
 \brief Get the number of awards registered for the game.
 
 \return The number of awards.
 
 \see AgonGetAwardTitleAtIndex
 \see AgonGetAwardDescriptionAtIndex
 \see AgonGetAwardImageAtIndex
 \see AgonGetAwardPocketScoreAtIndex
*/	
unsigned int AgonGetNumberOfAwards();

/**
 \brief Get the PocketScore of the award at the given index.
 
 \b Notice: To make your code less error prone we recommend that 
 you only use the index based award functions when you are working
 with awards as a collection. E.g. to display a list in-game
 of available awards.
 
 \param index Index of the award to get the title for.
 \return the award's PocketScore
 
 \see AgonGetNumberOfAwards
 */
int AgonGetAwardPocketScoreWithIndex(int index);
	
/**
 \brief Get the PocketScore of the award at the given ID.
 
 \param id ID of the award to get the title for.
 \return the award's PocketScore
 
 \see AgonGetAwardPocketScoreAtIndex
 */
int AgonGetAwardPocketScoreWithId(int id);
	
/**
 \brief Get the title of the award at the given index.
 
 \b Notice: To make your code less error prone we recommend that 
 you only use the index based award functions when you are working
 with awards as a collection. E.g. to display a list in-game
 
 \param index Index of the award to get the title for.
 \return Award title. Autoreleased.

 \see AgonGetNumberOfAwards
*/
NSString* AgonGetAwardTitleWithIndex(int index);
	
/**
 \brief Get the title of the award at the given ID.
 
 \param id ID of the award to get the title for.
 \return Award title. Autoreleased.
 
 \see AgonGetAwardTitleAtIndex
 */
NSString* AgonGetAwardTitleWithId(int id);

/**
 \brief Get the description of the award at the given index.
 
 \b Notice: To make your code less error prone we recommend that 
 you only use the index based award functions when you are working
 with awards as a collection. E.g. to display a list in-game
 
 \param index Index of the award to get the description for.
 \return Award description. Autoreleased.
 
 \see AgonGetNumberOfAwards
*/
NSString* AgonGetAwardDescriptionWithIndex(int index);
	
/**
 \brief Get the description of the award at the given ID.
 
 \param id ID of the award to get the description for.
 \return Award description. Autoreleased.
 
 \see AgonGetAwardDescriptionAtIndex 
 */
NSString* AgonGetAwardDescriptionWithId(int id);
	
/**
 \brief Get the image of the award at the given index.
 
 AGON uses an internal image manager with a predefined cache size,
 which means that the image may or may not be held in the memory cache.
 This means that calling this function can result in a disk read to
 fetch the image data into memory. Thus, if you need the image during
 gameplay it is a good idea to call this function prior to starting the
 game and then retaining the UIImage to ensure that your game does not
 suffer from a frame spike when the award image is required.
 
 \b Notice: To make your code less error prone we recommend that 
 you only use the index based award functions when you are working
 with awards as a collection. E.g. to display a list in-game
 
 \param index Index of the award to get the image for.
 \return Award image. You should not modify the UIImage. Treat as Autoreleased, 
		 but really managed by internal image manager.
 
 \see AgonGetAwardImageWithId
 \see AgonGetNumberOfAwards
*/
UIImage* AgonGetAwardImageWithIndex(int index);
	
/**
 \brief Get the image of the award at the given ID.
 
 See description for AgonGetAwardImageAtIndex
 
 \param id ID of the award to get the image for.
 \return Award image. You should not modify the UIImage. Treat as Autoreleased, 
 but really managed by internal image manager.
 
 \see AgonGetAwardImageAtIndex
 */
UIImage* AgonGetAwardImageWithId(int id);
	
/**
 \brief Query if the award at the given ID has been unlocked.
 
 \param id ID of the award to query.
 \return Whether award is unlocked or not.
 
 \see AgonUnlockAwardAtIndex
 */
BOOL AgonIsAwardWithIdUnlocked(int id);
	
/**
 \brief Unlock the award at the specified ID for the player
 \param id ID of the award to unlock
 \return Whether the award was unlocked or not (same as AgonIsAwardUnlocked 
		 returns prior to calling this function)
 
  \see AgonIsAwardUnlocked
*/
BOOL AgonUnlockAwardWithId(int id);

/**
 \brief Get profile name of the signed in user. Currently AGON does not support
 signing in / out with different profiles.
 
 \return User name of the AGON profile.
 */
NSString* AgonGetActiveProfileUserName();


	
#ifdef __cplusplus
}
#endif

#endif

#endif
