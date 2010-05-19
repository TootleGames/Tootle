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
 \brief AGON Online server environments.
 
 The AgonCreate call accepts a value from this enum as input. The value determines
 whether your AGON client will run against the developer, release candidate or 
 production servers. When you have completed your game and AGON intergration and want
 to move to the production servers, you use the publish menu from your game's web
 management page and request a move to the production servers. We will reply once
 the move is completed and you can then switch to using the AgonProduction enum value
 in your AgonCreate call. At this point you will be ready to submit to Apple.
 
 \see AgonCreate
 */
typedef enum {
	AgonDeveloperServers, /**< Enum representing the AGON developer server environment */
	AgonReleaseServers, /**< Enum representing the AGON release candidate server environment */
	AgonProductionServers, /**< Enum representing the AGON production server environment */
} AgonEnvironment;

/**
 \brief Result codes for renaming profile.
 
 The AgonSetActiveOfflineProfileName call returns one of these values.
 
 \see AgonSetActiveOfflineProfileName
 */
typedef enum {
	AgonSetProfileNameResultOk, /**< Enum representing the name has been changed successfully. */
	AgonSetProfileNameResultInvalidName, /**< Enum representing the name was too short or contained invalid characters. */
	AgonSetProfileNameResultNameTaken, /**< Enum representing the name has been changed successfully. */
	AgonSetProfileNameResultNotOffline,
} AgonSetProfileNameResult;


/**
  \brief Root leaderboard ID value.
 
 You can use this leaderboard ID with AgonShowLeaderboard() to show the root
 of your leaderboards. Alternatively you can just call AgonShow() with
 AgonBladeLeaderboard().
 
 \see AgonShowLeaderboard
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
@class NSData;

#ifdef __cplusplus
extern "C" {
#endif

/// @cond HIDDEN	
void AgonShowDeveloperInfo(NSString* title, NSString* message);
/// @endcond

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
 the library. This function checks that it can find the game's bundle package
 <b>(AgonPackge.bundle)</b>, in the root of the game's .app directory.
 
 Attempting to call any of the other AGON functions before this function 
 has been called will fail, except for AgonShowLogs.
 
 \param gameSecret Game secret for the current <b>AgonPackage.bundle</b>.
 \param serverEnvironment Enum specifying which AGON server environment your AGON
 client will be running against. Use AgonDeveloperServers while integrating AGON.
 AgonReleaseServers if testing your game with an AGON release candidate or 
 AgonProductionServers when your AGON integration has completed and your game has 
 been made available on the production environment: http://developer.agon-online.com/wiki/doku.php?id=docs:publish_game
 \return YES if the <b>AgonPackage.bundle</b> was found in the game's .app directory.
 \n NO if no <b>AgonPackage.bundle</b> is present in the game's .app directory.
 */
BOOL AgonCreate(NSString* gameSecret, AgonEnvironment serverEnvironment);
	
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
 
 \b NOTICE: If your game doesn't use a view controller for the view it adds to the key window
 you need to manually set the status bar orientation ([UIApplication sharedApplication].statusBarOrientation)
 before showing the AGON interface. For example in your applicationDidFinishLaunching callback.
 If your device hasn't been manually rotated after your game has been loaded it will report that its
 orientation is unknown (until it has been rotated for the first time) and AGON will adhere to the
 status bar orientation if shown. If you want your game locked to a certain status bar orientation
 you should set the status bar orientation manually in our callback action as it might have been
 changed while inside AGON in response to the user rotating the device.
 
 Showing the interface will submit any unsubmitted highscores and unlocked awards to the server.
 
 \param blade The blade (tab) you wish the AGON interface to start up in.
 \param allowUserSwitching controls whether the user can log off or switch to another user inside AGON
 \return YES if the interface will be shown. NO is returned otherwise, indicating that the 
 interface is already showing.
 
 \see AgonHide
 \see AgonBlade
 \see AGONDidHideNotification
 */
BOOL AgonShow(AgonBlade blade, BOOL allowUserSwitching);
	
/**
 \brief Show the AGON Online client interface.
 
 Please refer to the documentation for AgonShow.
 
 This function allows you to explicitly provide the UIWindow that AGON will make key and visible
 when it is hidden. By default AGON caches the current UIWindow and makes that key and visible when
 AGON is hidden.
 
 \param appWindow The UIWindow to make key and visible when AGON is hidden.
 
 \see AgonShow
 */
BOOL AgonShowEx(AgonBlade blade, BOOL allowUserSwitching, UIWindow* appWindow);
	
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
 
 \param leaderboardId The id of the leaderboard you wish the AGON interface to start up in.
 \param allowUserSwitching controls whether the user can log off or switch to another user inside AGON
 \return YES if the interface will be shown. NO is returned otherwise, indicating that the 
 interface is already showing.
 
 \see AgonHide
 \see AGONDidHideNotification
 */
BOOL AgonShowLeaderboard(int leaderboardId, BOOL allowUserSwitching);
	
/**
 \brief Show the AGON Online leaderboard interface.
 
 Please refer to the documentation for AgonShowLeaderboardEx.
 
 This function allows you to explicitly provide the UIWindow that AGON will make key and visible
 when it is hidden. By default AGON caches the current UIWindow and makes that key and visible when
 AGON is hidden.
 
 \param appWindow The UIWindow to make key and visible when AGON is hidden.
 
 \see AgonShowLeaderboard
 */
BOOL AgonShowLeaderboardEx(int leaderboardId, BOOL allowUserSwitching, UIWindow* appWindow);
	
/**
 \brief Show a window for selecting users on the device.
 
 \return YES if the interface will be shown. NO is returned otherwise, indicating that the 
 interface is already showing.

 \see AGONDidHideNotification
 \see AGONProfileChangedNotification
 */
BOOL AgonShowProfilePicker();
	
/**
 \brief Show a window for selecting users on the device.
 
 Please refer to the documentation for AgonShowProfilePickerEx.
 
 This function allows you to explicitly provide the UIWindow that AGON will make key and visible
 when it is hidden. By default AGON caches the current UIWindow and makes that key and visible when
 AGON is hidden.
 
 \param appWindow The UIWindow to make key and visible when AGON is hidden.
 
 \see AgonShowProfilePicker
 */
BOOL AgonShowProfilePickerEx(UIWindow* appWindow);
	
/**
 \brief Hide the AGON Online client interface.
 
 Hides the AGON Online client interface. This will typically never be called from the
 game, since the user can manually hide the interface from inside of AGON. The internal
 UIWindow is released and the original window is made key and visible again.
 
 \param animated: Whether the blade should animate when hiding.
 \return YES if the interface will be hidden. NO is returned otherwise, indicating that  
 the interface is already hidden or hiding. 
 \see AgonShow
 \see AGONDidHideNotification
 */
BOOL AgonHide(BOOL animated);

/**
 \brief Is the AGON Online client interface showing.
 
 \return YES if the interface is showing. NO if the interface is not showing.
 \see AGONDidHideNotification
 */
BOOL AgonIsShown();

/**
 \brief Is the AGON Online profile picker showing.

 \return YES if the profile picker is showing. NO if the profile picker is not showing.
 */
BOOL AgonIsProfilePickerShown();
	
/**
 \brief Use this function in your view controllers' shouldAutorotateToInterfaceOrientation
 
 If your application uses view controllers that implements shouldAutorotateToInterfaceOrientation,
 you should use this to ensure that the AGON client interface can respond to rotation notifications
 and change between portrait and landscape dynamically. When AGON is not shown it still respects the 
 orientations set by AgonSetSupportedInterfaceOrientations().
 
 \param interfaceOrientation: The interface orientation from shouldAutorotateToInterfaceOrientation.
 */	
BOOL AgonShouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation);
	
/**
 \brief Use this function to restrict the interface orientations supported by AGON.
 
 If you want to restrict the interface orientations supported by AGON you should call this method
 at startup to indicate which orientations to support.
 
 \param supportedOrientations: Array of orientations that AGON should support.
 \param numSupportedOrientations: Number of entries in the supportedOrientations array.
 
 \see AgonShouldAutorotateToInterfaceOrientation
 */	
void AgonSetSupportedInterfaceOrientations(UIInterfaceOrientation* supportedOrientations, int numSupportedOrientations);

/**
 \brief Use this function to control whether users will have the option to create offline profiles.
 
 This function lets you disable the use of offline profiles which is useful if your game depends 
 on an online profile to be used. If you don't allow offline profiles we recommend that you 
 somehow inform your users of that requirement before opening the AGON interface.
 
 When offline profiles aren't allowed, the option for staying offline isn't available from the
 welcome screen.
 
 Default is for AGON to allow users to decide whether their profile should be online or offline.
 */
void AgonSetAllowOfflineProfiles(BOOL allowOfflineProfiles);

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
 \brief Starts a game session.
 
 A game session is required to be able to submit scores and unlock awards. While the game
 session is active all scores and unlocked awards are stored in memory to avoid
 performing database IO during gameplay. The submitted scores and unlocked awards
 are persisted to the database when AgonEndGameSession() is called. 
 
 If the user closes your game using the home button AGON will automatically flush any cached data before the game closes.

 \b NOTICE If the AGON interface is opened, AgonEndGameSession() is automatically called.
 That means that if you open up AGON mid-game, you have to start another game session after AGON is hidden.
 
 
 \see AgonEndGameSession
 
 */
BOOL AgonStartGameSession();
	
/**
 \brief Ends the current game session.
 
 This will flush submitted scores and unlocked awards to the local db. They will then
 be sync'ed with the backend once the AGON interface is shown.

 \b NOTICE If the AGON interface is opened, AgonEndGameSession() is automatically called.
 That means that if you open up AGON mid-game, you have to start another game session after AGON is hidden.

 
 \see AgonStartGameSession
 */
BOOL AgonEndGameSession();
	
/**
 \brief Submit a new score to the AGON client.
 \hideinitializer

 The submitted score will be submitted to the backend server the next time
 the AGON Online client interface is shown (AgonShow(), AgonShowLeaderboard()).
 
 The web setup page for leaderboards contains an option for sorting either
 descending or ascending. You can use this option to control whether higher
 score values will be interpreted as better scores (sort descending) or whether
 lower scores should be considered best (sort ascending). Note that after a
 leaderboard has been created its sort order cannot be modified. If you decide
 to change the sort order for a leaderboard you will therefore have to delete the
 existing and create a new leaderboard using the desired sort order. This isn't
 allowed for leaderboards that have already gone live as players would otherwise
 loose their scores.
 
 What will actually be shown on the leaderboard is dictated by the supplied 
 scoreMetaData string, not the submitted score value. If you simply want the displayed
 score to match your submitted score, you supply a scoreMetaData string that matches it.
 For example, for a score of 31455 you can provide a scoreMetaData as "31455". If you are
 submitting times as scores, e.g. seconds to complete a race lap, you should format the 
 scoreMetaData string as a more human readable format (e.g. "1:23:456"). For lap times
 as floats with a set number of decimals, e.g. 52,44 seconds, you simply multiply with a 
 multiple of ten to convert the float to int. In this case multiplying all the times by 100 
 will do the trick.
 
 \b NOTICE It is required that this function is called between AgonStartGameSession() and AgonEndGameSession()
 
 \b NOTICE Ascending leaderboards do not support negative scores!

 \b NOTICE SCORES ARE INTEGERS, do not submit scores as floating points they will be truncated
 and you will loose crucial precision in the leaderboards.

 \param score (int) The score to submit.
 \param scoreMetaData (NSString*) The string to display in the leaderboard for the supplied score.
 \param leaderboardId (int) The leaderboard id to submit the score to.
 
 \see AgonStartGameSession
 */
#define AgonSubmitScore( /*int*/ score, /*NSString*/ scoreMetaData, /*int*/ leaderboardId) \
{\
	int agon_test_variable = score; \
	if (agon_test_variable != score) { \
		AgonShowDeveloperInfo(@"Submit score ignored", @"You must submit scores as integers. If you need decimal precision, eg. your score is a time, multiply the score by 1000 to get milisecond precision"); \
	} \
	else { \
		AgonSubmitIntegerScore(score, scoreMetaData, leaderboardId); \
	}\
}

/// @cond HIDDEN
void AgonSubmitIntegerScore(int score, NSString* scoreMetaData, int leaderboardId);
/// @endcond

	
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
 \brief Returns data for the current profile's friends.
 
 An NSArray with friends will be returned.
 \b NOTE: The list of friends is automatically retrieved shortly after the AGON
 interface is shown and it is then cached. Until the user has had AGON opened in
 your game once the list of friends will therefore be empty.
 
 Each element in the arrays is an NSDictionary
 with the following elements:
 
 NSString key: "username", NSString username
 NSString key: "alias", NSString alias
 
 \return An NSArray with NSDictionary elements as described above.
 */
NSArray* AgonGetActiveProfileFriends();
	
/**
 \brief Get the active profile's best local score.
 
 \param leaderboardId The leaderboard id to retrieve the score for.
 \return The user's best local score. Use intValue to get the value. Nil when no best score.
 
 \see AgonSubmitScore
 */	
NSNumber* AgonGetActiveProfileBestScore(int leaderboardId);
	
/**
 \brief Get the display string of the active profile's best local score.
 
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
 
 \see AgonGetAwardImageWithId
 \see AgonGetNumberOfAwards
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
 \return Whether award is unlocked or not. Notice that NO is returned 
 if AGON has not been created or the award ID was invalid.	
 
 \see AgonUnlockAwardWithId
 */
BOOL AgonIsAwardWithIdUnlocked(int id);
	
/**
 \brief Unlock the award with the specified ID for the player
 
 \param id ID of the award to unlock
 \return Whether the award was unlocked or not (same as AgonIsAwardUnlocked 
 returns prior to calling this function). 
 
 \b NOTICE: This function must be called between AgonStartGameSession() and AgonEndGameSession(). 
 Otherwise this function will return NO.
 
 \b NOTICE: If AgonCreate() has not been called or the award ID was invalid the function will return NO.
 
  \see AgonIsAwardUnlocked
  \see AgonStartGameSession
 */
BOOL AgonUnlockAwardWithId(int id);

/** \brief Post a message to a social network about an unlocked award.
 The player gets to choose if he wants the message posted to FaceBook or Twitter.

\b NOTICE: The award must have been unlocked for the message to be posted.
 
 \param id ID of the award to post a message about.
 

*/
void AgonShareAwardWithId(int id);

	
/**
 \brief Get locally unique ID of the signed in profile.
 
 Each profile is given a unique ID on the device. You can use this ID
 if you have profile related files that you need to store locally.
 
 E.g. suffixing this ID to the filename will allow you to recognize the appropriate
 file later on.
 
 \b Note: We recommend you to use the cloud storage feature rather than 
 storing per profile data locally as data stored online will be
 backed up and is accessible for the player from any device. Cloud storage
 is explained in detail here: http://developer.agon-online.com/wiki/doku.php?id=docs:cloud_storage
 
 \return The locally unique ID of the AGON profile as an autoreleased NSNumber (containing
 an int).

 \see AGONProfileChangedNotification
*/
NSNumber* AgonGetActiveProfileLocalId();

/**
 \brief Get globally unique ID of the signed in profile.
 
 If a profile has been online-enabled it has been given a globally unique ID on the server.
 You can use this ID to identify a specific profile across games and devices.
 
 \return The globally unique ID of the AGON profile as an autoreleased NSString.

 \see AGONProfileChangedNotification
 */
NSString* AgonGetActiveProfileGlobalId();	
	
/**
 \brief Get username of the signed in profile.
 
 \return User name of the AGON profile.
 
 \see AGONProfileChangedNotification

 */
NSString* AgonGetActiveProfileUserName();

/**
 \brief Set the name of the signed in offline profile.
 
 \return AgonSetProfileNameResultOk if successfull or one
 of the other AgonSetProfileNameResult enum values if failed
 
 \see AgonSetProfileNameResult
*/
AgonSetProfileNameResult AgonSetActiveOfflineProfileName(NSString* username);
	
/**
 \brief Get the alias of the signed in profile.
 
 \return Alias of the AGON profile.
 
 \see AGONProfileChangedNotification
*/
NSString* AgonGetActiveProfileAlias();
	
/**
 \brief Get the image of the signed in profile.
 
 \return Profile image. You should not modify the UIImage. Treat as Autoreleased, 
 but really managed by internal image manager.
 
 \see AGONProfileChangedNotification
 */
UIImage* AgonGetActiveProfilePicture();

/**
 \brief Set the data for the local blob with the given key.
 
 This function allows you to update the local blob with changes
 made locally, since the blob was last merged with the cloud blob.
 
 \param blobId ID of the blob.
 \param blob The blob of data stored for the specified key.
 \return Whether or not saving succeeded. Saving fails if blob synchronization 
 is in progress or if you have not registered a blob merge handler.

 \see AGONAllDataBlobsLoadedNotification
 */
BOOL AgonSetLocalBlob(int blobId, NSData* blob);

/**
 \brief Get the data for the local blob with the given key.
 
 This function returns the blob for the given key with local changes 
 made since the blob was last synchronized with the cloud.
 
 To get the full state of the blob stored for the given key your game
 must combine the data stored in the local blob with the data stored
 in the cloud blob.
 
 \param blobId ID of the blob.
 \return The blob of data stored for the specified key. The NSData object is autoreleased.  
 Nil is returned if you have not registered a blob merge handler.

 \see AGONAllDataBlobsLoadedNotification
 */
NSData* AgonGetLocalBlob(int blobId);

/**
 \brief Get the data for the cloud blob with the given key.
 
 This function returns the blob that was stored in the cloud the last
 time it was synchronized.
 
 To get the full state of the blob stored for the given key your game
 must combine the data stored in the local blob with the data stored
 in the cloud blob.
 
 \param blobId ID of the blob.
 \return The cloud blob of data stored for the specified key.
 The NSData object is autoreleased. Nil is returned if the user only has 
 an offline profile or if you have not registered a blob merge handler.

 \see AGONAllDataBlobsLoadedNotification
 */
NSData* AgonGetCloudBlob(int blobId);
	
/**
 \brief Register a callback for merging two blobs with a given key.
 
 When you use the data cloud facility in AGON you must register a 
 callback for dealing with merging blobs. AGON will ask you to merge
 2 blobs whenever it synchronizes with the backend. The first blob
 contains the game's local changes and the second blob contains the
 blob currently stored in the cloud. Your callback should return the
 merged blob as an autoreleased NSData object. During synchronization
 the local blob is locked for writing. When the merged blob has been
 successfully saved the local blob will be cleared and the cloud blob
 will hold the merged changes.
 
 \param callbackTarget The object that has the selector that will
 handle the merge.
 \param callbackAction The selector that will handle the merge. The
 signature for the merge selector must be the following:
 - (NSData*)mergeForBlobId:(int)blobId localBlob:(NSData*)localBlob cloudBlob:(NSData*)cloudBlob;

 \see AGONAllDataBlobsLoadedNotification
 */
void AgonRegisterBlobMergeCallback(id callbackTarget, SEL callbackAction);

/**
 \brief Returns the blade (tab) that AGON was showing the last time the OS or
 a user hid AGON. The purpose of this function is to allow games to open up AGON
 to the same blade if a user is e.g. using AGON and a phone call interupts
 the game.
 */
AgonBlade AgonGetLastOpenBlade();

/**
 \brief Tells AGON where to look for the AgonPackage.bundle file.
 Use this if you do not have the AgonPackage.bundle in the root directory.
 This must be called before AgonCreate if you intend to use it.

 \param path The relative path to the AgonPackage.bundle file
 */
void AgonSetAgonPackagePath(NSString* path);

/**
 \brief Utility function used to post messages to a social network.
 When this function is called the user is prompted to choose either FaceBook or Twitter
 and then a preview of the message is shown and the user can choose to post it or cancel the message.
 This function can be called anytime after AgonCreate.
 
 \param actionTitle Set the title of the UIActionSheet where the user can select service
 \param facebookStory The message to post to FaceBook
 \param facebookStoryTitle The title of the message to post to FaceBook
 \param facebookImageURL An URL to an image to include in the FaceBook message
 \param twitterStory Twitter version of the message (Max 140 characters.)
 */
void AgonPostStory(NSString* actionTitle, NSString* facebookStory, NSString* facebookStoryTitle, NSString* facebookImageURL, NSString* twitterStory);
	
/** @name Notifications
 * Various notifications posted by AGON through the default NotificationCenter
 */
//@{
/** \brief Notification posted when the AGON UI is closed.

	The notification object is nil. This notification does not contain a userInfo dictionary.
 */
extern NSString* const AGONDidHideNotification;

/** \brief Notification posted when the active profile has changed.

 The notification object is nil. This notification does not contain a userInfo dictionary.
 */
extern NSString* const AGONProfileChangedNotification;

/** \brief Notification posted when all data blobs has been loaded from the backend.
 
	The notification object is nil. This notification does not contain a userInfo dictionary.
 */
extern NSString* const AGONAllDataBlobsLoadedNotification;
//@}
	
#ifdef __cplusplus
}
#endif

#endif

#endif
