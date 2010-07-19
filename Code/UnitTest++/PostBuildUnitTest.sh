#	run self as unit test with the unit test command line command 
#	 (handled in TLGui::OnCommandLine() and return the programs error 
#	 code so the build fails if the test fails.

#	This path is built from enviorment variables from XCode and should evaluate
#	 to something like ~/Tootle/TestGame/build/Debug/TestGame.app/Contents/MacOS/TestGame
${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH} "UnitTest"

#	return the last-executed thing's return code
exit $?
