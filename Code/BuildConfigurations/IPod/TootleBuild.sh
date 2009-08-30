#!/bin/sh
# Tootle's automated build script;
# use: ./TootleBuild.sh <SVNREPO> <PROJECT> <TARGET> <KEYCHAINPASSWORD>
# e.g ./TootleBuild.sh Seaman Seaman Demo YourKeychainPassword

# assumes you've already checked out the repositories.
# Structure like
# ~/TootleBuild/
# add a symbolic link to this script in ~/TootleBuild/
# checkout Tootle, PROJECT etc to ~/TootleBuild/Tootle and ~/TootleBuild/PROJECT
# then call ./TootleBuild.sh PROJECT PASSWORD and it does the following

# Update svn
# unlocks your keychain so xcode can sign the app
# Build 
# Zip
# Upload

#SVN_REPO=$1
PROJECT=$1
PASSWORD=$2
TARGET=$3

# Use the project for the SVN repo for now - assumes the project and repository are the same name
SVN_REPO=$PROJECT

# maybe should force this to be relative to something?
cd ~/TootleBuild/
LOG_DIR=`pwd`
LOG_FILE="${LOG_DIR}/Logs/$PROJECT"


# make sure svn repo was provided
if [ "$SVN_REPO" == "" ]; then
	echo "No svn repository provided. First parameter should be the svn repository name, which is case sensitive. "
	exit 1
fi

# make sure project was provided
if [ "$PROJECT" == "" ]; then
	echo "No project provided. Second parameter should be the project name, which is case sensitive. "
	exit 1
fi


# make sure target was provided
# gr: if no target supplied, assume the target name is the same as the project name
if [ "$TARGET" == "" ]; then
	TARGET="$PROJECT";
fi

# make sure password was provided
if [ "$PASSWORD" == "" ]; then
	echo "No keychain password provided. Fourth parameter should be your keychain/login password to unlock the keychain so we can sign the .app"
	exit 1
fi



# update the tootle repository, if this fails assume it hasn't been checked out
echo "Updating Tootle svn repository..."
svn update Tootle > "${LOG_FILE}_Tootle_svn.txt"
if [ $? != 0 ]; then
	echo "svn update of Tootle failed, the Tootle Repository must have already been checked out (especially for the latest version of this script). See Tootle_svn.txt"
	exit 1
fi

# in case the build script has changed, restore execution permissions
# gr: dont auto allow execution.
#chmod 777 "Tootle/Code/BuildConfigurations/IPod/TootleBuild.sh"


# if the directory doesn't exist, assume the repository hasn't been checked out
if [ ! -d "$PROJECT" ]; then
	echo "$PROJECT directory doesn't exist, checking out repository..."
	svn checkout svn://grahamgrahamreeves.getmyip.com:1983/$SVN_REPO $PROJECT > ${LOG_FILE}_svn.txt
	
	if [ $? != 0 ]; then
		echo "svn checkout of $SVN_REPO repository for project $PROJECT failed. See ${LOG_FILE}_svn.txt"
		exit 1
	fi
else
	echo "Updating $PROJECT svn repository ($SVN_REPO)..."
	svn update $SVN_REPO > ${LOG_FILE}_svn.txt
	if [ $? != 0 ]; then
		echo "svn update of $SVN_REPO repository for project $PROJECT failed. See ${LOG_FILE}_svn.txt"
		exit 1
	fi
fi




# unlock keychain to sign project
echo "Unlocking keychain with password..."
security unlock-keychain -p $PASSWORD > ${LOG_FILE}_unlock.txt

# error
if [ $? != 0 ]; then
	echo "Failed to unlock keychain. See ${LOG_FILE}_unlock.txt"
	exit 1
fi

# need to change directory for xcodebuild to work :(
echo "Building $TARGET for the $PROJECT project..."
cd $PROJECT/Code/IPod/
xcodebuild -target $TARGET -configuration "Release AdHoc" -sdk iphoneos2.2.1 clean build > ${LOG_FILE}_build.txt
if [ $? != 0 ]; then
	echo "xcode build failed. read ${LOG_FILE}_build.txt"
	exit 1
fi


# change directory to .app's directory
# this is to make things simple and so we're in our zip's directory
# so when we zip up we don't include a massive directory structure
cd "build/Release AdHoc-iphoneos/"
APP_FILENAME="$TARGET.app"

# check .app has been built
if [ ! -e "$APP_FILENAME" ]; then
	CURRENT_PATH=`pwd`
	echo ".app doesn't exist - failed to build? ${CURRENT_PATH}/${PRODUCT_FILENAME}"
	exit 1
fi

# date in Year_Month_Day_Time format appended to zip filename, after the .app and before the .zip
DATE_STRING=`date +%Y_%b_%d_%H%M`
ZIP_FILENAME="${APP_FILENAME}.${DATE_STRING}.zip"

# ftp details for curl
FTP_HOST="ftp.grahamreeves.com"
FTP_USER="graham70"
FTP_PASS="a5c59gra533"
FTP_DIR="public_html/tootlegames/beta/$TARGET/"


echo "Zipping up ${APP_FILENAME}..."
echo "Zip filename: ${ZIP_FILENAME}"
zip "${ZIP_FILENAME}" -r "${APP_FILENAME}" > ${LOG_FILE}_zip.txt

# abort if we failed to zip
if [ $? != 0 ]; then
	echo "Failed to zip ${ZIP_FILENAME}; see ${LOG_FILE}_zip.txt"
	exit 1
fi

# upload zip with curl
echo "uploading ${ZIP_FILENAME} to ${FTP_HOST}/${FTP_DIR}..."
curl -T "${ZIP_FILENAME}" -u ${FTP_USER}:${FTP_PASS} ftp://${FTP_HOST}/${FTP_DIR} > "${LOG_FILE}_ftp.txt"

# failed
if [ $? != 0 ]; then
	echo "Failed to upload ${ZIP_FILENAME}; see ${LOG_FILE}_ftp.txt"
	exit 1
fi

echo "Zip and upload finished. $TARGET for project $PROJECT built!"
exit 0

