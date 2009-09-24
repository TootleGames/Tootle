+----------------------------------------------------------------------------+
                             AGON Online 1.2

            Copyright (c) 2009 Aptocore ApS. All Rights Reserved.
+----------------------------------------------------------------------------+

+----------------------------------------------------------------------------+
+++ Getting Started
+----------------------------------------------------------------------------+
You should integrate the AGON Online DocSet into XCode:
http://devsupport.agon-online.com/faqs/documentation/agon-online-docset

Alternatively you may read the documentation online:
http://devdb.agon-online.com/developer/doc/index.html

For users of the Unity Engine a Quick Start Guide can be found here:
http://devdb.agon-online.com/developer/doc/html/unity_quick_start.html

The documentation includes all of the details required to integrate AGON Online
into your game.

+----------------------------------------------------------------------------+
+++ Support
+----------------------------------------------------------------------------+
We have set up professional developer support at:
http://devsupport.agon-online.com/

If you have any other questions or comments feel free to write us at:
developer@agon-online.com

We are continuously improving Agon based on your feedback, so:
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
- 1.2.5
 * [NEW] A UnityPackage is now provided for very easy Unity integration.
 * [IMPROVED] AGON is now supported on iPhone OS 2.1 devices as well.
 * [CHANGED] Minor API changes in the Unity API compared to the pre-release
             that a few individuals have tried.
 * [CHANGED] All AGON builds now has the deployment target set to iPhone OS 
             2.1. This means you can use any of the SDK versions to build 
             for any OS version greater than 2.1. AGON builds for 
             SDK 2.2 and 2.2.1 will be deprecated in the future. Please
             note that this does NOT mean that you can only support 3.0
             devices; it simply means that you must use the 3.0 SDK
             to build your game and set the iPhone OS Deployment Target
             to the OS version your game supports. Please let us know
             if you think this will be a problem by contacting us at 
             developer@agon-online.com
 * [FIXED] Fixed problem when the profile picker if rotated while shown.
 * [FIXED] Problem with AGON not being shown if the device was put to sleep
           while showing AGON - this only affected 2.x devices.

The full changelog is available in the manual:
http://devdb.agon-online.com/developer/doc/change_log_history.html