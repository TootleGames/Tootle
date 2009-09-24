/**
 \file AGON.h
 
 \brief Header for the AGON Online client library.
*/
#ifndef AGON_H
#define AGON_H

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

/**
  \brief Root leaderboard ID value.
 
 You can use this leaderboard ID with AgonShowLeaderboard() to show the root
 of your leaderboards. Alternatively you can just call AgonShow() with
 AgonBladeLeaderboards.
 
 \see AgonShowLeaderboards
 */
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
 the library. This function checks that it can find the included AGON resources
 bundle <b>(AgonData.bundle)</b> and the game's bundle package
 <b>(AgonPackge.bundle)</b>, in the root of the game's .app directory.
 
 Attempting to call any of the other AGON functions before this function 
 has been called will fail, except for AgonShowLogs.
 
 \param gameSecret Game secret for the current <b>AgonPackage.bundle</b>.
 \return YES if the required bundles were found in the game's .app directory.
 \n NO if one of the bundles were missing from the game's .app directory.

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
 \brief Enable internal retain count assertions in the library.
 
 AGON has internal assertions in debug mode on retain counts allowing detection of
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
 
 The first time the user is presented for AGON online he is asked whether he wants to Go Online
 or Stay Offline. This is done to comply with Apple's requirement of notifying users before
 submitting data to an online datastore. 
 
 If the user selects Stay Offline AGON will still function with personal leaderboards and awards,
 but all of the online features will be disabled.
 
 If the user selects Go Online the full functionality of AGON will be available when an Internet
 connection is available. If the Internet connection is lost AGON reverts to offline mode, but
 still records scores and awards. Any scores and awards submitted while being offline are 
 synchronized to the servers when the user comes back online.
 
 When the client interface is shown the user is presented with a tab view with easy access
 to all of the functionality within the AGON Online client. The user exits the interface
 by pressing the game icon on the tab furthest to the right. Internally the interface is
 shown by creating a UIWindow for the AGON interface and making that window key and visible.
 The interface is animated in similarly to using presentModalViewController.
 
 \b NOTICE: If your game doesn't use a view controller for the view it adds to the key window
 you should manually set the status bar orientation ([UIApplication sharedApplication].statusBarOrientation)
 to control the orientation of AGON before showing the interface.
 
 Showing the interface will submit any unsubmitted highscores and unlocked awards to the server.
 
 \param callbackTarget An optional parameter for an object to give a callback when 
 AGON returns control to the game. AGON retains this object while the interface showing.
 \param callbackAction Optional selector to call on the target when AGON returns  
 control to the game.
 \param blade The blade (tab) you wish the AGON interface to start up in.
 \return YES if the interface will be shown. NO is returned otherwise, indicating that the 
 interface is already showing.
 
 \see AgonHide, AgonBlade
*/
BOOL AgonShow(id callbackTarget, SEL callbackAction, AgonBlade blade);

/**
 \brief Show the AGON Online leaderboard interface.
 
 Shows the client interface and takes the user directly to the leaderboard identified by
 the \a leaderboardId.
 
 Please refer to the documentation for AgonShow() for additional details.
 
 \param callbackTarget An optional parameter for an object to give a callback when 
 AGON returns control to the game. AGON retains this object while the interface is showing.
 \param callbackAction Optional selector to call on the target when AGON returns  
 control to the game.
 \param leaderboardId The id of the leaderboard you wish the AGON interface to start up in.
 \return YES if the interface will be shown. NO is returned otherwise, indicating that the 
 interface is already showing.
 
 \see AgonHide, AgonShow

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
 \brief Use this function in your game's view controllers' shouldAutorotateToInterfaceOrientation
 
 If your application uses view controllers that implements shouldAutorotateToInterfaceOrientation,
 you should use this to ensure that the AGON client interface can respond to rotation notifications
 and change between portrait and landscape dynamically. When AGON is not shown all of the orientations
 provided to this function will be returned as supported. When AGON is shown all interface 
 orientations are supported unless you have restricted the supported orientations using 
 AgonSetSupportedInterfaceOrientations
 
 \param supportedOrientations: Array of orientations supported by your application.
 \param numSupportedOrientations: Number of entries in the supportedOrientations array.
 \param interfaceOrientation: The interface orientation from shouldAutorotateToInterfaceOrientation.
 
 \see AgonSetSupportedInterfaceOrientations
 */	
BOOL AgonShouldAutorotateToInterfaceOrientation(UIInterfaceOrientation* supportedOrientations, int numSupportedOrientations, UIInterfaceOrientation interfaceOrientation);
	
/**
 \brief Use this function to restrict the interface orientations supported by AGON.
 
 If you want to restrict the interface orientations supported by AGON you should call this method
 at startup to indicate which orientations to support.
 
 \param supportedOrientations: Array of orientations that AGON should support.
 \param numSupportedOrientations: Number of entries in the supportedOrientations array.
 
 \see AgonSetSupportedInterfaceOrientations
 */	
void AgonSetSupportedInterfaceOrientations(UIInterfaceOrientation* supportedOrientations, int numSupportedOrientations);
	
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
 the AGON Online client interface is shown (AgonShow()).
 
 The sorting order depends on the configuration made on the developer 
 management site for the leaderboard.
  
 \param score The score to submit. Higher is always better.
 \param scoreMetaData The string to display in the leaderboard for the supplied score.
 \param leaderboardId The leaderboard id to submit the score to.
*/
void AgonSubmitScore(int score, NSString* scoreMetaData, int leaderboardId);

/**
 \brief Get the number of scores stored locally for the currently signed in user.
 
 Use this to query AGON about how many scores the currently signed in user has
 stored locally for the specified leaderboard ID.
 
 \param leaderboardId ID of the leaderboard to the return the number of scores for.
 
 \see AgonGetActiveProfileScores
*/
int AgonGetActiveProfileNumberOfScores(int leaderboardId);

/**
 \brief Get the scores for the currently signed in user.
 
 Use AgonGetActiveProfileNumberOfScores() to get the number of scores available. An
 NSArray with scores will be returned. Each element in the arrays is an NSDictionary
 with the following elements:
 
 NSString key: "position", NSNumber position
 NSString key: "score", NSNumber score
 NSString key: "metaData", NSString metaDataString
 NSString key: "date", NSDate date
 
 The scores are returned sorted by score, with the best score first (depending on 
 ascending or descending sort order for the leaderboard).
 
 \param leaderboardId ID of the leaderboard to get the scores from.
 \param fromIdx The index of the first score to retrieve.
 \param numberOfScores The number of scores to retrieve after fromIdx.
 \return An NSArray with NSDictionary elements as described above.
 
 \see AgonGetActiveProfileNumberOfScores
*/
NSArray* AgonGetActiveProfileScores(int leaderboardId, int fromIdx, int numberOfScores);
	
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
 
 \see AgonGetAwardTitleWithIndex, AgonGetAwardDescriptionWithIndex, AgonGetAwardImageWithIndex, AgonGetAwardPocketScoreWithIndex
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
 
 \see AgonGetAwardPocketScoreWithIndex
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
 
 \see AgonGetAwardTitleWithIndex
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
 
 \see AgonGetAwardDescriptionWithIndex 
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
 
 \see AgonGetAwardImageWithId, AgonGetNumberOfAwards
*/
UIImage* AgonGetAwardImageWithIndex(int index);
	
/**
 \brief Get the image of the award at the given ID.
 
 See description for AgonGetAwardImageWithIndex
 
 \param id ID of the award to get the image for.
 \return Award image. You should not modify the UIImage. Treat as Autoreleased, 
 but really managed by internal image manager.
 
 \see AgonGetAwardImageWithIndex
 */
UIImage* AgonGetAwardImageWithId(int id);
	
/**
 \brief Query if the award at the given ID has been unlocked.
 
 \param id ID of the award to query.
 \return Whether award is unlocked or not.
 
 \see AgonUnlockAwardWithId
 */
BOOL AgonIsAwardWithIdUnlocked(int id);
	
/**
 \brief Unlock the award with the specified ID for the player
 
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
