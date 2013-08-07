all: jni

jni: jni/jni_part.cpp
	OPENCVROOT=~/install/OpenCV-2.4.3.2-android-sdk /home/clayton/install/android-ndk-r8d/ndk-build all	
