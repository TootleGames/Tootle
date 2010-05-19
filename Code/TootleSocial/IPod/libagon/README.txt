+----------------------------------------------------------------------------+
                             AGON Online

            Copyright (c) 2009 Aptocore ApS. All Rights Reserved.
+----------------------------------------------------------------------------+

+----------------------------------------------------------------------------+
+++ Getting Started
+----------------------------------------------------------------------------+
There is a developer wiki available with detailed information about AGON integration: 
http://developer.agon-online.com/wiki/

The AGON API reference documentation is available online here:
http://developer.agon-online.com/documentation/latest

The AGON reference documentation is provided as an Online DocSet and can be integrated into XCode:
http://developer.agon-online.com/wiki/doku.php?id=docs:xcode_documentation_integration

For Objective-C developers a Quick Start Guide can be found here:
http://developer.agon-online.com/wiki/doku.php?id=docs:quick_start

For Unity Engine developers a Quick Start Guide can be found here:
http://developer.agon-online.com/wiki/doku.php?id=docs:unity_quick_start

For Cocos2D developers a Quick Start Guide can be found here:
http://www.cocos2d-iphone.org/wiki/doku.php/prog_guide:social_networks#integrating_agon_online_into_cocos2d-iphone

The wiki and reference documentation include all of the details required to 
integrate AGON Online into your game.

+----------------------------------------------------------------------------+
+++ Upgrading
+----------------------------------------------------------------------------+
To upgrade to the latest version of AGON you should make sure that everything
inside the existing libagon folder is overwritten. Please make sure you do not
merge the old folder with the new folder, since you might then have files that
are no longer needed.

If you have an AgonData.bundle folder added to your Xcode project's resource
then you should delete the folder from the project. The AgonData.bundle is now
provided as part of the AgonPackage.bundle.

Make sure you re-download the AgonPackage.bundle to ensure that the 
AgonPackage.bundle is compatible with the latest version.

Finally you need to update your code to work with the API interface changes
for the latest version. It is imperative that you get AGON.h updated so that
you discover interface changes.

+----------------------------------------------------------------------------+
+++ Support
+----------------------------------------------------------------------------+
We have set up professional developer support at:
http://devsupport.agon-online.com/

If you have any other questions or comments feel free to write us at:
developer@agon-online.com

We are continuously improving AGON based on your feedback, so:
Have your say - write to us!

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
+++ Changelog
+----------------------------------------------------------------------------+
- 1.5.1
 * [FIXED] Crash when a memory warning was issued after the "profile" tab had been activated inside AGON.
 * [FIXED] A number of memory leaks.

The full changelog is available in the manual:
http://developer.agon-online.com/documentation/latest/change_log_history.html
