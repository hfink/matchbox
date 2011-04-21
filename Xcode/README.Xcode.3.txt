Xcode 3 has a few troubles properly choosing the platform of the executable in
the multi-platform project WordMatch.xcodeproj. For example, if you choose to
run the iPhoneTest on the iOS simulator, you will not be able to properly 
execute the OSX unit tests of this same project as Xcode assumes you are still
targeting the simulator platform. 

There is a simple workaround for this: 

	Close the Xcode project
	Open ./Xcode/WordMatch.xcodeproj/*.pbxuser
	Find the line that start with "activeSDKPreference=..."
	Erase that line
	Re-open the project.
