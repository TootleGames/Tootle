# ------------------------------------------------------------
#
#	This is our custom script to build QT's MOC files
#	which we require in order to use the Signals and Slots system
#
#	In order to make automated building easier, this file's contents should be copied
#	into a run script in the target of TootleGui, rather than using this script as
#	it requires setting up execution permissions, which SVN cannot do.
#
#	This script works for XCode (mac), a seperate batch file (which does pretty much the 
#	same thing) should be made for windows. It may be possible to wrap all this into
#	a cross platform makefile....
#
#	Add any additional QT type headers into this file, and make sure the output .Moc.Cpp files
#	are built with the project. For this reason, this script needs to be run before sources
#	are compiled!
#
# ------------------------------------------------------------

# QT Moc executable
MOC="/Developer/Tools/Qt/moc"

# files to parse
HEADERFILES=("Qt/Tree.h" "Qt/Window.h" "Qt/ModelTest.h")

# QT includes
INCPATH="-I/usr/local/Qt4.6/mkspecs/macx-xcode -I. -I/Library/Frameworks/QtCore.framework/Versions/4/Headers -I/usr/include/QtCore -I/Library/Frameworks/QtGui.framework/Versions/4/Headers -I/usr/include/QtGui -I/Library/Frameworks/QtOpenGL.framework/Versions/4/Headers -I/usr/include/QtOpenGL -I/usr/include -I/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers -I/System/Library/Frameworks/AGL.framework/Headers -I. -I. -I/usr/local/include -I/System/Library/Frameworks/CarbonCore.framework/Headers -F/Library/Frameworks"

# moc platform specific defines
DEFINES="-DQT_OPENGL_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED"

PATH_IN=${SRCROOT}/../
PATH_OUT=${SRCROOT}/../

# loop through files
for HEADERFILE in ${HEADERFILES[@]}; do
	# execute moc
	FILE_IN=${PATH_IN}${HEADERFILE}
	FILE_OUT=${PATH_OUT}${HEADERFILE}.Moc.cpp 
	echo "Moc building $FILE_IN to $FILE_OUT"
	$MOC $DEFINES $INCPATH -D__APPLE__ -D__GNUC__ $FILE_IN -o $FILE_OUT 
	# result in, $? use this?
done

# always exit with success even if there are warnings about no output generated
exit 0

