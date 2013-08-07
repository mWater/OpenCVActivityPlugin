LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=off
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=STATIC

include $(OPENCVROOT)/sdk/native/jni/OpenCV.mk

LOCAL_MODULE    := opencvactivity
LOCAL_SRC_FILES := jni_part.cpp ec-plates/algorithm.cpp ec-plates/Circle.cpp ec-plates/CircleFinder.cpp ec-plates/ColonyCounter.cpp 
LOCAL_LDLIBS +=  -llog -ldl -ljnigraphics


include $(BUILD_SHARED_LIBRARY)
