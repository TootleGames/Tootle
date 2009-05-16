#!/bin/sh
# Tootle's automated build script;
# use: ./TootleBuild.sh Seaman YourKeychainPassword

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

PROJECT=$1
PASSWORD=$2

# maybe should force this to be relative to something?
cd ~/TootleBuild/
LOG_DIR=`pwd`
LOG_FILE="${LOG_DIR}/Log_$PROJECT"


if [ "$PROJECT" == "" ]; then
	echo "No project provided. First parameter should be the project name, which is case sensitive. "
	exit 1
fi


# update the tootle repository, if this fails assume it hasn't been checked out
echo "Updating Tootle svn repository..."
svn update Tootle > "${LOG_DIR}/Log_Tootle_svn.txt"
if [ $? != 0 ]; then
	echo "svn update of Tootle failed, the Tootle Repository must have already been checked out (especially for the latest version of this script). See Tootle_svn.txt"
	exit 1
fi

# in case the build script has changed, restore execution permissions
chmod 777 "Tootle/Code/BuildConfigurations/IPod/TootleBuild.sh"


# if the directory doesn't exist, assume the repository hasn't been checked out
if [ ! -d "$PROJECT" ]; then
	echo "$PROJECT directory doesn't exist, checking out repository..."
	svn checkout svn://grahamgrahamreeves.getmyip.com:1983/$PROJECT $PROJECT > ${LOG_FILE}_svn.txt
	
	if [ $? != 0 ]; then
		echo "svn checkout of $PROJECT failed. See ${LOG_FILE}_svn.txt"
		exit 1
	fi
else
	echo "Updating $PROJECT svn..."
	svn update $PROJECT > ${LOG_FILE}_svn.txt
	if [ $? != 0 ]; then
		echo "svn update of $PROJECT failed. See ${LOG_FILE}_svn.txt"
		exit 1
	fi
fi



# make sure password was provided
if [ "$PASSWORD" == "" ]; then
	echo "No keychain password provided. Second parameter should be your keychain/login password to unlock the keychain so we can sign the .app"
	exit 1
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
echo "Building $PROJECT..."
cd $PROJECT/Code/IPod/
xcodebuild -configuration "Release AdHoc" -sdk iphoneos2.2.1 -alltargets clean build > ${LOG_FILE}_build.txt
if [ $? != 0 ]; then
	echo "xcode build failed. read ${LOG_FILE}_build.txt"
	exit 1
fi


# change directory to .app's directory
# this is to make things simple and so we're in our zip's directory
# so when we zip up we don't include a massive directory structure
cd "build/Release AdHoc-iphoneos/"
APP_FILENAME="$PROJECT.app"

# check .app has been built
if [ ! -e "$APP_FILENAME" ]; then
	CURRENT_PATH=`pwd`
	echo ".app doesn't exist - failed to build? ${CURRENT_PATH}/${PRODUCT_FILENAME}"
	exit 1
fi

# date in Fri_15May_1301 format appended to zip filename, after the .app and before the .zip
DATE_STRING=`date +%a_%d%b_%H%M`
ZIP_FILENAME="${APP_FILENAME}.${DATE_STRING}.zip"

# ftp details for curl
FTP_HOST="ftp.grahamreeves.com"
FTP_USER="graham70"
FTP_PASS="a5c59gra533"
FTP_DIR="public_html/tootlegames/beta/"


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

echo "Zip and upload finished. $PROJECT built!"
exit 0

